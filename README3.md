# Inline Hooks

Hook arbitrary code locations with full register context access

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Inline hooks allow you to intercept execution at any point in the code, not just at function boundaries. Your callback receives a complete snapshot of all CPU registers, enabling inspection and modification of the execution state.## 

A64InlineHook

Installs a hook at an arbitrary code address with full register context.### 

Signature

```
extern "C" void A64InlineHook(
    void* const symbol,
    void* const replace
);
```

### Parameters


symbol

void*

required

Pointer to the code address where the hook should be installed. This can be:* Any valid instruction address in executable memory

* The middle of a function
* The start of a basic block
* After a function prologue

The address must be 4-byte aligned (aligned to ARM instruction boundaries).


replace

void*

required

Pointer to your callback function. The callback must have this signature:

```
void MyInlineCallback(InlineCtx* ctx);
```

The callback receives a pointer to an `InlineCtx` structure containing all CPU registers at the time of the hook.

### Return Value

This function returns `void`. If the hook installation fails, it will trigger a fatal error:* `SkylineError_InlineHookHandlerSizeInvalid` - Handler size mismatch

* `SkylineError_InlineHookPoolExhausted` - No more inline hooks available

Inline hook failures are fatal. Always ensure you have sufficient hooks available (max 4096 inline hooks).

## InlineCtx Structure

Provides complete CPU register state at the hook point.### 

Definition

```
struct InlineCtx {
    nn::os::CpuRegister registers[31];    // X0-X30 general-purpose registers
    nn::os::CpuRegister sp;               // Stack pointer
    nn::os::FpuRegister registers_f[32];  // Q0-Q31 floating-point/SIMD registers
};
```

### Fields


registers

nn::os::CpuRegister[31]

Array of general-purpose registers X0-X30.* `registers[0]` = X0 (first argument / return value)

* `registers[1]` = X1 (second argument)
* `registers[29]` = X29 (frame pointer)
* `registers[30]` = X30 (link register)


sp

nn::os::CpuRegister

Stack pointer (SP) at the time of the hook.


registers_f

nn::os::FpuRegister[32]

Array of 128-bit SIMD/floating-point registers Q0-Q31.These are used for:* Floating-point operations

* SIMD/vector operations
* First 8 also used for floating-point arguments (Q0-Q7)

### Register Layout

The `InlineCtx` structure has a total size of 0x300 bytes:| Offset        | Size      | Description              |
| --------------- | ----------- | -------------------------- |
| 0x00 - 0xF0   | 248 bytes | X0-X30 general registers |
| 0xF0 - 0x100  | 16 bytes  | SP (stack pointer)       |
| 0x100 - 0x300 | 512 bytes | Q0-Q31 SIMD registers    |

## Initialization

### A64HookInit

Initializes the inline hook system. Must be called before using any hook functions.

```
void A64HookInit();
```

Call this function once during your plugin’s initialization, before installing any hooks.

#### What It Does

1. Initializes a mutex for thread-safe hook operations
2. Allocates JIT memory for function hook trampolines (2048 slots)
3. Allocates JIT memory for inline hook handlers (4096 slots)
4. Finds suitable memory regions near the main executable

#### Example

```
void skyline_main() {
    // Initialize hooking system
    A64HookInit();

    // Now you can install hooks
    A64HookFunction(/* ... */);
    A64InlineHook(/* ... */);
}
```

## Usage Examples

### Basic Inline Hook

Hook a specific code location and log register values:

```
void MyInlineCallback(InlineCtx* ctx) {
    // Access general-purpose registers
    u64 x0 = ctx->registers[0].x;
    u64 x1 = ctx->registers[1].x;

    skyline::logger::s_Instance->LogFormat(
        "Hook triggered! X0=0x%llx, X1=0x%llx\n",
        x0, x1
    );
}

void InstallHook() {
    // Hook at a specific address
    void* hookAddress = reinterpret_cast<void*>(0x71000ABCD0);
    A64InlineHook(hookAddress, reinterpret_cast<void*>(MyInlineCallback));
}
```

### Modifying Registers

Change register values to alter execution flow:

```
void ForceReturnValue(InlineCtx* ctx) {
    // Override return value (X0)
    ctx->registers[0].x = 1;  // Force success

    skyline::logger::s_Instance->Log("Forced return value to 1\n");
}

void InstallHook() {
    // Hook right before a function returns
    void* beforeReturn = reinterpret_cast<void*>(0x71000ABCD0);
    A64InlineHook(beforeReturn, reinterpret_cast<void*>(ForceReturnValue));
}
```

### Inspecting Function Arguments

Hook at the start of a function to see its arguments:

```
void LogFunctionArgs(InlineCtx* ctx) {
    // AArch64 calling convention:
    // X0-X7 = first 8 arguments
    void* arg1 = reinterpret_cast<void*>(ctx->registers[0].x);
    u64 arg2 = ctx->registers[1].x;
    u64 arg3 = ctx->registers[2].x;

    skyline::logger::s_Instance->LogFormat(
        "Function called: arg1=%p, arg2=0x%llx, arg3=0x%llx\n",
        arg1, arg2, arg3
    );
}

void InstallHook() {
    // Hook after the function prologue
    void* afterPrologue = reinterpret_cast<void*>(0x71000ABCD8);
    A64InlineHook(afterPrologue, reinterpret_cast<void*>(LogFunctionArgs));
}
```

### Accessing Floating-Point Registers

Inspect SIMD/floating-point state:

```
void LogFloatArgs(InlineCtx* ctx) {
    // Q0-Q7 used for floating-point arguments
    // Access as different types:
    float f32_value = ctx->registers_f[0].v[0];  // First float in Q0
    double f64_value = ctx->registers_f[0].d[0]; // First double in Q0

    skyline::logger::s_Instance->LogFormat(
        "Float args: Q0.s[0]=%f, Q0.d[0]=%f\n",
        f32_value, f64_value
    );
}
```

### Conditional Hook Behavior

Execute logic based on register state:

```
void ConditionalHook(InlineCtx* ctx) {
    u64 flags = ctx->registers[0].x;

    if (flags & 0x1) {
        skyline::logger::s_Instance->Log("Flag bit 0 is set\n");
    }

    if (ctx->registers[1].x == 0) {
        skyline::logger::s_Instance->Log("Warning: X1 is NULL\n");
        // Optionally modify behavior
        ctx->registers[0].x = 0;  // Set error flag
    }
}
```

### Stack Inspection

Access the stack through the stack pointer:

```
void InspectStack(InlineCtx* ctx) {
    u64* stack = reinterpret_cast<u64*>(ctx->sp.x);

    // Read values from stack
    u64 stack_value1 = stack[0];
    u64 stack_value2 = stack[1];

    skyline::logger::s_Instance->LogFormat(
        "Stack: SP=%p, [SP]=%llx, [SP+8]=%llx\n",
        stack, stack_value1, stack_value2
    );
}
```

## When to Use Inline Hooks

Inline hooks are ideal for:✅ **Mid-function inspection** - Hook in the middle of a function to inspect intermediate state✅ **Conditional breakpoints** - Execute code only when certain conditions are met✅ **Register manipulation** - Modify register values to change execution flow✅ **Precise timing** - Hook at exact instruction addresses for timing-sensitive operations✅ **Analyzing proprietary code** - Reverse engineer unknown functions by observing register state**Use function hooks instead when:*** You want to replace an entire function

* You need to call the original function
* You only care about function entry/exit points

## Performance Considerations

### Hook Overhead

Each inline hook adds overhead:1. **Register backup** - ~50 instructions to save all registers (0x300 bytes stack)

1. **Callback execution** - Your callback function
2. **Register restore** - ~50 instructions to restore all registers
3. **Trampoline branch** - Jump to original code

Inline hooks are more expensive than function hooks due to the full register save/restore.Avoid hooking code in tight loops or performance-critical paths.

### Pool Limits

* **Maximum inline hooks** : 4096 (`inline_hook_count`)
* **Handler size** : 12 bytes per hook
* **Total pool size** : ~192 KB

If you exceed this limit, `A64InlineHook` will trigger a fatal error.## 

Technical Details

### Hook Installation Process

1. **Allocate handler entry** - Get next available slot in inline hook pool
2. **Create trampoline** - Use `A64HookFunction` to redirect execution
3. **Setup handler** - Copy inline handler assembly into JIT memory
4. **Link callback** - Store callback pointer in handler entry
5. **Make executable** - Transition JIT memory to executable mode

From `And64InlineHook.cpp:677`:

```
extern "C" void A64InlineHook(void* const address, void* const callback) {
    // Validate handler size
    if (inline_hook_handler_size != handler_end_addr - handler_start_addr) {
        R_ERRORONFAIL(MAKERESULT(Module_Skyline, 
            SkylineError_InlineHookHandlerSizeInvalid));
    }

    // Check pool availability
    if (inline_hook_curridx >= inline_hook_count) {
        R_ERRORONFAIL(MAKERESULT(Module_Skyline, 
            SkylineError_InlineHookPoolExhausted));
    }

    // Hook to call the handler
    void* trampoline;
    A64HookFunction(address, &rx.handler, &trampoline);

    // Populate handler entry
    memcpy(rw.handler.data(), (void*)handler_start_addr, 
        inline_hook_handler_size);
    rw.cur_handler = &inlineHandlerImpl;
    rw.callback = callback;
    rw.trampoline = trampoline;

    inline_hook_curridx++;
}
```

### Handler Assembly

The inline hook handler is implemented in ARM assembly (`armutils.s:94`):

```
inlineHandlerImpl:
    armBackupRegisters      ; Save all CPU state (0x300 bytes)

    mov x0, sp              ; Pass InlineCtx* as argument
    ldr x16, [x17, #8]      ; Load callback address
    blr x16                 ; Call user callback

    armRecoverRegisters     ; Restore all CPU state

    ldr x16, [x17, #0x10]   ; Load trampoline address
    br x16                  ; Jump to original code
```

### Memory Layout

Each inline hook entry has this structure:

```
struct inline_hook_entry {
    uint8_t handler[12];        // Offset 0x00: Handler assembly
    const void* cur_handler;    // Offset 0x0C: Handler implementation
    const void* callback;       // Offset 0x14: User callback
    const void* trampoline;     // Offset 0x1C: Original code
};
```

The inline hook pool is allocated near the main executable (within ±128MB) to enable efficient branching.## 

Best Practices

Keep Callbacks Fast

Your callback is executed with all registers saved. Minimize work to reduce overhead:

```
// ✅ Good: Fast logging
void FastCallback(InlineCtx* ctx) {
    if (g_debug_enabled) {
        log_buffer[log_index++] = ctx->registers[0].x;
    }
}

// ❌ Bad: Expensive work in callback
void SlowCallback(InlineCtx* ctx) {
    std::string msg = format_message(ctx);  // Allocation
    send_over_network(msg);                 // I/O
}
```

### Don’t Modify Stack Pointer

The stack pointer should not be modified:

```
void BadCallback(InlineCtx* ctx) {
    ctx->sp.x += 0x100;  // ❌ DON'T DO THIS - will corrupt stack
}
```

### Be Careful with Register Modifications

Modifying registers can break assumptions in the original code:

```
void SafeCallback(InlineCtx* ctx) {
    // ✅ Safe: Modify return value before function returns
    if (ctx->registers[30].x == return_address) {
        ctx->registers[0].x = 0;  // OK to modify
    }

    // ⚠️ Dangerous: Modifying mid-function may break invariants
    ctx->registers[5].x = 0;  // May break if code relies on X5
}
```

### Verify Hook Addresses

Ensure hook addresses are valid and aligned:

```
void InstallSafeHook(uintptr_t address) {
    // Check alignment
    if (address % 4 != 0) {
        skyline::logger::s_Instance->Log("ERROR: Unaligned address\n");
        return;
    }

    // Verify address is in executable region
    if (!is_executable_address(address)) {
        skyline::logger::s_Instance->Log("ERROR: Not executable\n");
        return;
    }

    A64InlineHook(reinterpret_cast<void*>(address), MyCallback);
}
```

## Related APIs

* [Function Hooks](https://mintlify.wiki/skyline-dev/skyline/api/hooks/function-hooks) - For hooking entire functions
* [Architecture](https://mintlify.wiki/skyline-dev/skyline/core/architecture) - Initialization and system setup

## See Also

* `A64HookFunction()` - Function hooking API
* `A64HookInit()` - Initialize hooking system
* Source: `/source/skyline/inlinehook/And64InlineHook.cpp:677`
* Assembly: `/source/skyline/utils/armutils.s:94`



Hooking API

# Function Hooks

Hook and replace entire functions with A64HookFunction

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Function hooks allow you to intercept and replace entire functions in the game’s code. When you hook a function, the original function is redirected to your replacement, and you optionally receive a trampoline to call the original implementation.## 

A64HookFunction

Replaces a function at runtime by redirecting execution to your replacement function.### 

Signature

```
extern "C" void A64HookFunction(
    void* const symbol,
    void* const replace,
    void** result
);
```

### Parameters


symbol

void*

required

Pointer to the function to hook. This can be:* A direct function pointer (e.g., `reinterpret_cast<void*>(MyFunction)`)

* An address obtained via `nn::ro::LookupSymbol`
* Any valid executable memory address


replace

void*

required

Pointer to your replacement function. This function will be called instead of the original.The replacement function should have the same signature as the original function to maintain ABI compatibility.


result

void**

Output parameter that receives a pointer to the trampoline function. Pass `NULL` if you don’t need to call the original function.The trampoline contains the original instructions and allows you to call the original function from within your hook.

### Return Value

This function returns `void`. The trampoline pointer is written to the `result` parameter.

If hooking fails and `result` is not `NULL`, the value at `result` will be set to `NULL`.

### How It Works

1. **Initialization Required** : You must call `A64HookInit()` before using any hook functions (typically in your entry point).
2. **Trampoline Creation** : If `result` is non-NULL, the function creates a trampoline containing:

* The original instructions that were overwritten
* Fixed-up PC-relative instructions
* A jump back to continue the original function

1. **Instruction Patching** : The original function is patched with either:

* A single branch instruction (if within ±128MB range)
* A 5-instruction sequence for absolute jumps (if beyond branch range)

1. **Thread Safety** : The function uses a mutex internally to ensure thread-safe hooking operations.

### Usage Examples

#### Basic Hook Without Trampoline

If you want to completely replace a function without calling the original:

```
void MyCustomFunction() {
    // Your implementation
}

// Hook the function
A64HookFunction(
    reinterpret_cast<void*>(OriginalFunction),
    reinterpret_cast<void*>(MyCustomFunction),
    nullptr  // No trampoline needed
);
```

#### Hook With Trampoline

When you need to call the original function:

```
Result (*originalMountRom)(char const*, void*, unsigned long) = nullptr;

Result MyMountRomHook(char const* path, void* buffer, unsigned long size) {
    // Pre-processing
    skyline::logger::s_Instance->LogFormat("Mounting ROM: %s\n", path);

    // Call original function
    Result rc = originalMountRom(path, buffer, size);

    // Post-processing
    if (R_SUCCEEDED(rc)) {
        skyline::logger::s_Instance->Log("Mount successful\n");
    }

    return rc;
}

// Install the hook
A64HookFunction(
    reinterpret_cast<void*>(nn::fs::MountRom),
    reinterpret_cast<void*>(MyMountRomHook),
    reinterpret_cast<void**>(&originalMountRom)
);
```

Real-World Example from Skyline

From `main.cpp:120`:

```
Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = nnFsMountRomImpl(path, buffer, size);

    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    g_MountRomInit.call_once([]() {
        // Initialize systems after ROM is mounted
        skyline::utils::SafeTaskQueue* taskQueue =
            new skyline::utils::SafeTaskQueue(100);
        taskQueue->startThread(20, 3, 0x10000);
        taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));
        nn::os::WaitEvent(&after_romfs_task->completionEvent);
    });

    return rc;
}

void skyline_main() {
    A64HookInit();

    A64HookFunction(
        reinterpret_cast<void*>(nn::fs::MountRom),
        reinterpret_cast<void*>(handleNnFsMountRom),
        reinterpret_cast<void**>(&nnFsMountRomImpl)
    );
}
```

#### Hooking Functions by Symbol Name

```
Result (*nnRoInitializeImpl)();

Result MyRoInitHook() {
    Result ret = nnRoInitializeImpl();
    skyline::logger::s_Instance->LogFormat(
        "nn::ro::Initialize returned: 0x%x\n", ret
    );
    return ret;
}

// Hook using function pointer
A64HookFunction(
    reinterpret_cast<void*>(nn::ro::Initialize),
    reinterpret_cast<void*>(MyRoInitHook),
    reinterpret_cast<void**>(&nnRoInitializeImpl)
);
```

## A64HookFunctionV

Low-level variant of `A64HookFunction` with manual memory management.### 

Signature

```
void* A64HookFunctionV(
    void* const symbol,
    void* const replace,
    void* const rxtr,
    void* const rwtr,
    const uintptr_t rwx_size
);
```

### Parameters


symbol

void*

required

Pointer to the function to hook.


replace

void*

required

Pointer to the replacement function.


rxtr

void*

required

Executable memory region for the trampoline.


rwtr

void*

required

Writable memory region for the trampoline (must map to same physical memory as `rxtr`).


rwx_size

uintptr_t

required

Size of the trampoline buffer. Must be at least `A64_MAX_INSTRUCTIONS * 10 * sizeof(uint32_t)` bytes.

### Return Value

Returns a pointer to the executable trampoline (`rxtr`) on success, or `NULL` on failure.

Most users should use `A64HookFunction` instead, which handles memory allocation automatically.Use `A64HookFunctionV` only when you need fine-grained control over trampoline memory placement.

## Best Practices

Function Signature Matching

Ensure your replacement function has the exact same signature as the original:

```
// Original function
Result OriginalFunction(int arg1, void* arg2);

// ✅ Correct: Matching signature
Result MyHook(int arg1, void* arg2) { /* ... */ }

// ❌ Wrong: Mismatched signature will cause crashes
void MyHook(void* arg2) { /* ... */ }
```

### Calling Convention

On AArch64, the calling convention is:* Arguments 1-8: `x0-x7` registers

* Return value: `x0` register
* Floating-point args: `q0-q7` registers

The hook framework preserves the calling convention automatically.### 

Hook Ordering

Hooks are installed immediately and affect all subsequent calls:

```
// Install hooks during initialization
void skyline_main() {
    A64HookInit();

    // Hook order matters if functions call each other
    A64HookFunction(functionA, hookA, &trampolineA);
    A64HookFunction(functionB, hookB, &trampolineB);
}
```

### Error Handling

Always verify hook installation by checking the trampoline pointer:

```
void* trampoline = nullptr;
A64HookFunction(symbol, replace, &trampoline);

if (trampoline == nullptr) {
    // Hook failed - handle error
    skyline::logger::s_Instance->Log("Hook installation failed!\n");
}
```

### Memory Constraints

The hook system has a limited pool of trampolines:* Maximum of 2048 function hooks (`A64_MAX_BACKUPS`)

* Each trampoline can use up to `A64_MAX_INSTRUCTIONS * 10` instructions

If you exhaust the pool, subsequent hooks will fail.## 

Technical Details

### Instruction Relocation

The hooking system automatically fixes PC-relative instructions in the trampoline:* **Branch instructions** (`B`, `BL`, `B.cond`)

* **Compare and branch** (`CBZ`, `CBNZ`, `TBZ`, `TBNZ`)
* **Load literal** (`LDR`, `LDRSW`, `LDR` vector)
* **PC-relative address** (`ADR`, `ADRP`)

### Hook Patching Strategies

**Short-range patch** (±128MB):

```
B <replace>    ; Single branch instruction (4 bytes)
```

**Long-range patch** (beyond ±128MB):

```
NOP            ; Alignment if needed
LDR X17, #8    ; Load target address
BR X17         ; Branch to address
.quad <replace> ; 64-bit target address
```

## Related APIs

* [Inline Hooks](https://mintlify.wiki/skyline-dev/skyline/api/hooks/inline-hooks) - For mid-function hooking
* [Architecture](https://mintlify.wiki/skyline-dev/skyline/core/architecture) - Initialization and system setup

## See Also

* `A64HookInit()` - Initialize the hooking system
* `InlineCtx` structure - For inline hooks with register context
* Source: `/source/skyline/inlinehook/And64InlineHook.cpp:645`


# Logger Overview

Base logger interface and architecture for Skyline

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Overview

The Skyline logger system provides a flexible logging framework with multiple implementations for debugging and monitoring your hooks and mods. All loggers inherit from the abstract `Logger` base class defined in `skyline/logger/Logger.hpp`.## 

Global Instance

The logger system uses a global singleton instance that can be accessed from anywhere in your code:

```
namespace skyline::logger {
    extern Logger* s_Instance;
}
```

**Usage Example:**

```
#include "skyline/logger/Logger.hpp"

// Log a simple message
skyline::logger::s_Instance->Log("Hello from Skyline!\n");

// Log formatted output
skyline::logger::s_Instance->LogFormat("Value: %d, Address: 0x%llx\n", value, address);
```

## Logger Base Class

All logger implementations inherit from the `Logger` abstract class and must implement the following pure virtual methods:### 

Pure Virtual Methods

#### Initialize

```
virtual void Initialize() = 0
```

Called when the logger thread starts. Override this to set up network connections, open files, or perform other initialization tasks.#### 

SendRaw

```
virtual void SendRaw(void* data, size_t size) = 0
```


data

void*

required

Pointer to the raw data buffer to send


size

size_t

required

Number of bytes to send from the buffer

Sends raw data directly to the logger output. This is called by the logger thread when flushing the message queue.#### 

FriendlyName

```
virtual std::string FriendlyName() = 0
```

Returns a human-readable name for the logger implementation (e.g., “TcpLogger”, “SdLogger”).#### 

ShouldFlush

```
virtual bool ShouldFlush() = 0
```

Returns `true` if the logger is ready to flush messages. For example, TcpLogger returns `true` only when a client is connected.---

## Public Methods

When `NOLOG` is defined at compile time, all public logging methods become empty inline functions, allowing you to disable logging without changing your code.

### StartThread

```
void StartThread()
```

Starts the background logger thread that processes the message queue. This should be called after initializing sockets or other dependencies.**Usage:**

```
// From main.cpp after socket initialization
skyline::logger::s_Instance->StartThread();
```

### Log

```
void Log(const char* data, size_t size = UINT32_MAX)
void Log(std::string str)
```


data

const char*

required

Null-terminated string or raw data buffer to log


size

size_t

**default:**"UINT32_MAX"

Number of bytes to log. If `UINT32_MAX` (default), uses `strlen(data)`


str

std::string

required

String object to log (alternate overload)

Adds a message to the internal queue for asynchronous logging. Messages are processed by the logger thread.**Examples:**

```
// Log a C string
skyline::logger::s_Instance->Log("Initialization complete\n");

// Log specific number of bytes
skyline::logger::s_Instance->Log(buffer, 256);

// Log a std::string
std::string message = "Status: OK";
skyline::logger::s_Instance->Log(message);
```

### LogFormat

```
void LogFormat(const char* format, ...)
```


format

const char*

required

printf-style format string


...

variadic

Variable arguments matching the format specifiers

Logs a formatted message using printf-style format strings. Automatically appends a newline character.**Example:**

```
// From main.cpp exception handler
skyline::logger::s_Instance->LogFormat("Error description: %x\n", info->ErrorDescription);
skyline::logger::s_Instance->LogFormat("PC: %" PRIx64 "\n", info->PC.x);
skyline::logger::s_Instance->LogFormat("[%s] Loaded %d plugins", subsystem, count);
```

### SendRaw (Overload)

```
void SendRaw(const char* data)
```


data

const char*

required

Null-terminated string to send immediately

Sends data directly without queueing. Uses `strlen()` to determine size.### 

SendRawFormat

```
void SendRawFormat(const char* format, ...)
```


format

const char*

required

printf-style format string


...

variadic

Variable arguments matching the format specifiers

Formats and sends data directly without queueing. Limited to 4096 bytes (0x1000).### 

Flush

```
void Flush()
```

Manually flushes all queued messages by calling `SendRaw()` on each. The logger thread calls this automatically every 100ms.---

## Logger Architecture

### Message Queue

Messages logged via `Log()` and `LogFormat()` are added to an internal queue (`std::queue<char*>`). A background thread processes this queue periodically:1. User calls `Log()` or `LogFormat()` - message is copied and queued

1. Logger thread wakes every 100ms
2. If `ShouldFlush()` returns `true`, all queued messages are sent via `SendRaw()`
3. Memory for processed messages is freed

### Thread Safety

The logger uses a message queue to avoid blocking the main thread. Messages are queued quickly and processed asynchronously by the logger thread.---

## Available Implementations

Skyline provides several logger implementations for different use cases:[TcpLoggerNetwork-based logging over TCP socket (port 6969)](https://mintlify.wiki/skyline-dev/skyline/api/logger/tcp-logger)

[SdLoggerFile-based logging to SD card](https://mintlify.wiki/skyline-dev/skyline/api/logger/sd-logger)

## KernelLogger

Low-level kernel debug output via svcOutputDebugString

## DualLogger

Combines TCP and SD logging for redundancy

## DummyLogger

No-op logger for production builds

**TcpLogger** and **SdLogger** have full documentation pages. Other logger types follow the same base interface but are typically used in specialized scenarios.

---

## NOLOG Build Flag

When `NOLOG` is defined during compilation, all logging methods become no-op inline functions:

```
#ifdef NOLOG
    inline void StartThread() {}
    inline void Log(const char* data, size_t size = UINT32_MAX) {}
    inline void Log(std::string str) {}
    inline void LogFormat(const char* format, ...) {}
    inline void SendRaw(const char*) {}
    inline void SendRawFormat(const char*, ...) {}
    inline void Flush() {}
#endif
```

This allows you to completely disable logging overhead in production builds without modifying your code.**Build without logging:**

```
make NOLOG=1
```


# TcpLogger

Network-based logger for real-time debugging over TCP

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Overview

`TcpLogger` provides real-time network-based logging over a TCP socket connection. It’s the default logger used in Skyline and is ideal for interactive debugging and monitoring.**Header:** `skyline/logger/TcpLogger.hpp`## 

Class Definition

```
namespace skyline::logger {
    class TcpLogger : public Logger {
    public:
        virtual void Initialize();
        virtual bool ShouldFlush() override;
        virtual void SendRaw(void* data, size_t size);
        virtual std::string FriendlyName() { return "TcpLogger"; }
    };

    void setup_socket_hooks();
}
```

---

## Setup and Initialization

### Basic Setup

From `main.cpp`, the typical initialization sequence is:

```
#include "skyline/logger/TcpLogger.hpp"

void skyline_main() {
    // Initialize hooking
    A64HookInit();

    // Setup socket hooks (required before creating TcpLogger)
    skyline::logger::setup_socket_hooks();

    // Mount SD card for debug
    nn::fs::MountSdCardForDebug("sd");

    // Create and assign logger instance
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();

    // Start logging immediately (queued until socket connects)
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // Later, after socket initialization in your code:
    // skyline::logger::s_Instance->StartThread();
}
```

### Socket Initialization

TcpLogger requires socket initialization before the logger thread can accept connections. The `setup_socket_hooks()` function must be called early to prevent double initialization crashes.

From the actual usage in `main.cpp`:

```
static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    // Allocate socket pool
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);

    // Initialize sockets
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    // Now start the logger thread
    skyline::logger::s_Instance->StartThread();
}};
```

---

## Network Configuration

### Connection Details

TcpLogger binds to port **6969** and accepts incoming connections on any network interface.

```
#define PORT 6969
```

**Connection Flow:**1. TcpLogger creates a TCP socket using `nn::socket::Socket(AF_INET, SOCK_STREAM, 0)`

1. Binds to `INADDR_ANY:6969` to accept connections from any IP
2. Listens for incoming connections with `nn::socket::Listen()`
3. Accepts the first client connection
4. Sends confirmation message: `"TCP Socket Connnected.\n"`

### Connecting to TcpLogger

From your development machine, connect using any TCP client:netcat

telnet

Python

```
# Using netcat
nc <switch-ip-address> 6969
```

---

## Class Methods

### Initialize

```
virtual void Initialize()
```

Empty implementation - TcpLogger initialization is handled by `setup_socket_hooks()` and the background thread.### 

ShouldFlush

```
virtual bool ShouldFlush() override
```

**Returns:** `true` if the logger is initialized and has an active client connection, `false` otherwise.This prevents attempting to send data before a client connects. Messages remain queued until a connection is established.**Implementation:**

```
bool TcpLogger::ShouldFlush() {
    return g_loggerInit && (g_tcpSocket != -1);
}
```

### SendRaw

```
virtual void SendRaw(void* data, size_t size)
```


data

void*

required

Pointer to data buffer to send over the network


size

size_t

required

Number of bytes to send

Sends raw data over the TCP socket using `nn::socket::Send()`.**Implementation:**

```
void TcpLogger::SendRaw(void* data, size_t size) {
    nn::socket::Send(g_tcpSocket, data, size, 0);
}
```

### FriendlyName

```
virtual std::string FriendlyName()
```

**Returns:** `"TcpLogger"`---

## Socket Hooks

### setup_socket_hooks

```
void setup_socket_hooks()
```

**Must be called before any socket initialization!** This function hooks the socket initialization functions to prevent double-initialization crashes.

This function installs hooks for:* `nn::socket::Initialize(void*, ulong, ulong, int)` - Standard pool initialization

* `nn::socket::Initialize(nn::socket::Config const&)` - Config-based initialization
* `nn::socket::Finalize()` - Prevents socket deinitialization

**Purpose:** Some games initialize sockets themselves. These hooks ensure:1. Sockets are only initialized once (prevents crash)

1. Logger thread starts after socket initialization
2. Sockets remain active (Finalize is stubbed)

**Usage:**

```
void skyline_main() {
    A64HookInit();

    // Call this BEFORE creating TcpLogger
    skyline::logger::setup_socket_hooks();

    skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    // ...
}
```

---

## Advanced Usage

### External C Function

TcpLogger exports a C-compatible function for external use:

```
extern "C" void skyline_tcp_send_raw(char* data, size_t size);
```


data

char*

required

Raw data buffer to send


size

size_t

required

Number of bytes to send (uses `u64` internally)

This allows logging from C code or external plugins:

```
// From C code
extern void skyline_tcp_send_raw(char* data, size_t size);

void my_c_function() {
    skyline_tcp_send_raw("Message from C code\n", 21);
}
```

### Connection Configuration

The logger uses SO_KEEPALIVE to maintain persistent connections:

```
int flags = 1;
nn::socket::SetSockOpt(g_tcpSocket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));
```

---

## Common Issues

### Socket Already Initialized Error

**Problem:** Game crashes when creating TcpLogger**Solution:** Ensure `setup_socket_hooks()` is called before any socket initialization:

```
// Correct order:
skyline::logger::setup_socket_hooks();  // First
skyline::logger::s_Instance = new skyline::logger::TcpLogger();
```

### No Connection / Logs Not Appearing

**Problem:** Cannot connect or no logs appear**Checklist:**1. ✅ Called `setup_socket_hooks()` early

1. ✅ Called `StartThread()` after socket initialization
2. ✅ Switch and PC are on the same network
3. ✅ Port 6969 is not blocked by firewall
4. ✅ Using correct Switch IP address

### Delayed Initialization Games

Some games (Fire Emblem Three Houses, Pokémon Sword/Shield) initialize sockets using `Config` variant early. The hooks handle both initialization methods.

---

## Example: Complete Setup

```
#include "skyline/logger/TcpLogger.hpp"
#include "skyline/utils/cpputils.hpp"

void skyline_main() {
    // 1. Initialize hooking framework
    A64HookInit();

    // 2. Setup socket hooks (prevents double-init crashes)
    skyline::logger::setup_socket_hooks();

    // 3. Mount SD card (required for some operations)
    nn::fs::MountSdCardForDebug("sd");

    // 4. Create TcpLogger instance
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();

    // 5. Start logging (messages queued until socket connects)
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // 6. Later: Initialize sockets and start logger thread
    //    (typically done after romfs mount in a task)
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    skyline::logger::s_Instance->StartThread();

    // 7. Now logging is fully active
    skyline::logger::s_Instance->LogFormat("[%s] Logger initialized.", 
                                           skyline::logger::s_Instance->FriendlyName().c_str());
}
```

---

## See Also

* [Logger Overview](https://mintlify.wiki/skyline-dev/skyline/api/logger/overview) - Base logger interface and architecture
* [SdLogger](https://mintlify.wiki/skyline-dev/skyline/api/logger/sd-logger) - File-based logging alternative



# SdLogger

File-based logger that writes to SD card storage

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Overview

`SdLogger` writes log messages to a file on the Nintendo Switch SD card. This is useful for capturing logs when network connectivity is unavailable or when you need persistent log storage.**Header:** `skyline/logger/SdLogger.hpp`## 

Class Definition

```
namespace skyline::logger {
    class SdLogger : public Logger {
    public:
        SdLogger(std::string path);

        virtual void Initialize();
        virtual bool ShouldFlush() override;
        virtual void SendRaw(void* data, size_t size);
        virtual std::string FriendlyName() { return "SdLogger"; }
    };
}
```

---

## Constructor

### SdLogger

```
SdLogger(std::string path)
```


path

std::string

required

Absolute path to the log file on the SD card (e.g., `"sd:/skyline.log"`)

Creates an SdLogger instance that writes to the specified file path.**Behavior:**1. Checks if the path exists using `nn::fs::GetEntryType()`

1. If path doesn’t exist (result `0x202`), creates a new file with `nn::fs::CreateFile()`
2. If path exists but is a directory, fails silently
3. Opens the file in read-write + append mode: `OpenMode_ReadWrite | OpenMode_Append`

**Example:**

```
#include "skyline/logger/SdLogger.hpp"

void skyline_main() {
    // Mount SD card first
    nn::fs::MountSdCardForDebug("sd");

    // Create logger writing to sd:/skyline/logs/output.log
    skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/skyline/logs/output.log");

    // Start logger thread
    skyline::logger::s_Instance->StartThread();

    // Begin logging
    skyline::logger::s_Instance->Log("[skyline] Initialization complete\n");
}
```

---

## Class Methods

### Initialize

```
virtual void Initialize()
```

Empty implementation - all initialization is done in the constructor. Called by the logger thread when it starts.### 

ShouldFlush

```
virtual bool ShouldFlush() override
```

**Returns:** Always returns `false`

SdLogger always returns `false` from `ShouldFlush()` but messages are still written. Each `SendRaw()` call writes directly to the file with the flush flag enabled.

This design avoids unnecessary buffering and ensures logs are immediately written to disk, which is important for debugging crashes.### 

SendRaw

```
virtual void SendRaw(void* data, size_t size)
```


data

void*

required

Pointer to data buffer to write to the file


size

size_t

required

Number of bytes to write

Writes raw data directly to the log file with immediate flush.**Implementation Details:**1. Expands file size to accommodate new data: `nn::fs::SetFileSize(fileHandle, offset + size)`

1. Writes data at current offset: `nn::fs::WriteFile(fileHandle, offset, data, size, ...)`
2. Uses `WriteOptionFlag_Flush` to ensure data is written immediately to SD card
3. Increments internal offset for next write

```
void SdLogger::SendRaw(void* data, size_t size) {
    nn::fs::SetFileSize(fileHandle, offset + size);
    nn::fs::WriteFile(fileHandle, offset, data, size,
                      nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));
    offset += size;
}
```

### FriendlyName

```
virtual std::string FriendlyName()
```

**Returns:** `"SdLogger"`---

## SD Card Requirements

### Mounting the SD Card

 **You must mount the SD card before creating an SdLogger instance** , otherwise file operations will fail.

```
// Mount SD card with debug access
Result rc = nn::fs::MountSdCardForDebug("sd");
if (R_FAILED(rc)) {
    // Handle mount failure
}

// Now create SdLogger
skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/logs/game.log");
```

### File Path Format

Paths must use the Nintendo Switch filesystem format:Valid Paths

Invalid Paths

```
// Root of SD card
"sd:/skyline.log"

// Subdirectory
"sd:/skyline/logs/debug.log"

// Nested directories (must exist or be created first)
"sd:/atmosphere/contents/01006A800016E000/romfs/skyline/logs/output.log"
```

---

## Usage Examples

### Basic Usage

```
#include "skyline/logger/SdLogger.hpp"

void setup_sd_logging() {
    // Mount SD card
    nn::fs::MountSdCardForDebug("sd");

    // Create logger
    skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/skyline_debug.log");

    // Start logger thread
    skyline::logger::s_Instance->StartThread();

    // Log messages
    skyline::logger::s_Instance->Log("=== Skyline Debug Log ===\n");
    skyline::logger::s_Instance->LogFormat("Game TID: %016llX\n", titleId);
}
```

### Creating Log Directories

If your log file is in a subdirectory, create the directory structure first:

```
#include "nn/fs.h"

void setup_logging_with_dir() {
    nn::fs::MountSdCardForDebug("sd");

    // Create directory if it doesn't exist
    nn::fs::CreateDirectory("sd:/skyline");
    nn::fs::CreateDirectory("sd:/skyline/logs");

    // Create logger in subdirectory
    skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/skyline/logs/debug.log");
    skyline::logger::s_Instance->StartThread();
}
```

### Logging Game Events

```
void log_game_state() {
    auto logger = skyline::logger::s_Instance;

    logger->LogFormat("=== Frame %d ===", frameCount);
    logger->LogFormat("Player position: (%.2f, %.2f, %.2f)", x, y, z);
    logger->LogFormat("Health: %d/%d", currentHealth, maxHealth);
    logger->Log("\n");
}
```

---

## Advantages and Limitations

### Advantages ✅

* **No network required** - Works without WiFi or network configuration
* **Persistent storage** - Logs survive crashes and can be retrieved later
* **Immediate writes** - Each message is flushed immediately to SD card
* **Simple setup** - Only requires SD card mounting
* **Post-crash analysis** - Can examine logs after the game crashes

### Limitations ⚠️

* **No real-time viewing** - Must remove SD card or use FTP to view logs
* **SD card wear** - Frequent writes may reduce SD card lifespan
* **Performance impact** - Synchronous file writes are slower than network logging
* **Storage space** - Large logs can fill SD card space
* **No timestamping** - Base implementation doesn’t add timestamps (you must add them manually)

---

## Common Issues

### SD Card Not Mounted

**Problem:** SdLogger constructor fails silently**Solution:** Always mount SD card before creating SdLogger:

```
Result rc = nn::fs::MountSdCardForDebug("sd");
if (R_SUCCEEDED(rc)) {
    skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/log.txt");
} else {
    // Fallback or error handling
}
```

### Directory Doesn’t Exist

**Problem:** Log file path includes non-existent directories**Solution:** Create directories before creating the logger:

```
nn::fs::CreateDirectory("sd:/skyline");
nn::fs::CreateDirectory("sd:/skyline/logs");  // Create parent dirs first
skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/skyline/logs/output.log");
```

### Path is a Directory

**Problem:** Specified path is a directory, not a file**Solution:** Ensure the path points to a file:

```
// Wrong - this is a directory
new skyline::logger::SdLogger("sd:/skyline/");

// Correct - this is a file
new skyline::logger::SdLogger("sd:/skyline/output.log");
```

---

## Advanced Usage

### Adding Timestamps

The base SdLogger doesn’t add timestamps. You can add them manually:

```
void log_with_timestamp(const char* message) {
    nn::time::PosixTime currentTime;
    nn::time::StandardUserSystemClock::GetCurrentTime(&currentTime);

    skyline::logger::s_Instance->LogFormat("[%lld] %s", currentTime.time, message);
}
```

### Rotating Log Files

```
void rotate_log_if_needed(const char* log_path) {
    s64 file_size;
    nn::fs::GetFileSize(&file_size, log_path);

    // If log exceeds 10MB, rotate it
    if (file_size > 10 * 1024 * 1024) {
        nn::fs::RenameFile(log_path, "sd:/skyline_old.log");
        // Next SdLogger creation will create a new file
    }
}
```

### Conditional Logging

```
// Use SdLogger for production, TcpLogger for development
void setup_logger(bool is_development) {
    nn::fs::MountSdCardForDebug("sd");

    if (is_development) {
        skyline::logger::setup_socket_hooks();
        skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    } else {
        skyline::logger::s_Instance = new skyline::logger::SdLogger("sd:/skyline_release.log");
    }

    // Start logger thread (works for both)
    skyline::logger::s_Instance->StartThread();
}
```

---

## Retrieving Logs

### Method 1: Remove SD Card

1. Power off the Switch
2. Remove the SD card
3. Insert SD card into PC
4. Navigate to the log file location
5. Copy or view the log file

### Method 2: FTP Access

```
# Connect via FTP (requires FTP homebrew on Switch)
ftp <switch-ip>
cd /
get skyline.log
```

### Method 3: ftpd Homebrew

Use ftpd or similar homebrew to access SD card over network without removing it.---

## See Also

* [Logger Overview](https://mintlify.wiki/skyline-dev/skyline/api/logger/overview) - Base logger interface and architecture
* [TcpLogger](https://mintlify.wiki/skyline-dev/skyline/api/logger/tcp-logger) - Network-based logging for real-time debugging




# IPC Utilities

Inter-process communication utilities for service calls and HIPC protocol

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

The IPC utilities provide a wrapper around Nintendo’s HIPC (Horizon IPC) protocol for communicating with system services on the Nintendo Switch.## 

Overview

These utilities simplify the process of creating service connections, dispatching requests, and parsing responses when communicating with Nintendo Switch system services.## 

Functions

### nnServiceCreate

Creates a new service connection by name.

```
Result nnServiceCreate(Service* srv, const char* name)
```


srv

Service*

required

Pointer to a Service structure that will be populated with the connection details


name

const char*

required

Name of the service to connect to (e.g., “pm:dmnt”, “fs-srv”)


Result

Result

Returns 0 on success, or an error code on failure

**Implementation Details:*** Initializes HIPC service resolution

* Connects to the named service
* Queries the pointer buffer size via control request 3
* Populates the Service structure with session handle and configuration

**Example:**

```
Service srv;
Result rc = skyline::utils::nnServiceCreate(&srv, "pm:dmnt");
if (R_SUCCEEDED(rc)) {
    // Service connected successfully
}
```

---

### nnServiceClose

Closes a service connection and cleans up resources.

```
void nnServiceClose(Service* s)
```


s

Service*

required

Pointer to the Service structure to close

**Implementation Details:*** Sends a close request to the service

* Closes the session handle if owned
* Resets the Service structure to zero

**Example:**

```
Service srv;
nnServiceCreate(&srv, "pm:dmnt");
// ... use service ...
nnServiceClose(&srv);
```

---

### nnServiceMakeRequest

Constructs an IPC request in the TLS message buffer.

```
void* nnServiceMakeRequest(
    Service* s,
    u32 request_id,
    u32 context,
    u32 data_size,
    bool send_pid,
    const SfBufferAttrs buffer_attrs,
    const SfBuffer* buffers,
    u32 num_objects,
    const Service* const* objects,
    u32 num_handles,
    const Handle* handles
)
```


s

Service*

required

Service to send the request to


request_id

u32

required

Command ID for the service request


context

u32

required

Request context value


data_size

u32

required

Size of the input data in bytes


send_pid

bool

required

Whether to send the process ID with the request


buffer_attrs

SfBufferAttrs

required

Buffer attributes for up to 8 buffers


buffers

const SfBuffer*

required

Array of buffer descriptors


num_objects

u32

required

Number of service objects to send


objects

const Service* const*

required

Array of service objects


num_handles

u32

required

Number of handles to send


handles

const Handle*

required

Array of handles to send


return

void*

Pointer to the data section of the request buffer

This is a low-level function typically used internally by the dispatch macros.

---

### nnServiceParseResponse

Parses an IPC response from the TLS message buffer.

```
Result nnServiceParseResponse(
    Service* s,
    u32 out_size,
    void** out_data,
    u32 num_out_objects,
    Service* out_objects,
    const SfOutHandleAttrs out_handle_attrs,
    Handle* out_handles
)
```


s

Service*

required

Service that sent the response


out_size

u32

required

Expected size of output data in bytes


out_data

void**

Pointer to receive the output data pointer


num_out_objects

u32

required

Number of service objects expected in response


out_objects

Service*

Array to receive output service objects


out_handle_attrs

SfOutHandleAttrs

required

Handle attributes for up to 8 output handles


out_handles

Handle*

Array to receive output handles


Result

Result

Returns 0 on success, or an error code on failure

---

### nnServiceDispatchImpl

Dispatches a complete IPC request and parses the response.

```
Result nnServiceDispatchImpl(
    Service* s,
    u32 request_id,
    const void* in_data,
    u32 in_data_size,
    void* out_data,
    u32 out_data_size,
    SfDispatchParams disp
)
```


s

Service*

required

Service to dispatch the request to


request_id

u32

required

Command ID for the service request


in_data

const void*

Pointer to input data


in_data_size

u32

required

Size of input data in bytes


out_data

void*

Pointer to buffer for output data


out_data_size

u32

required

Size of output data buffer in bytes


disp

SfDispatchParams

required

Dispatch parameters including buffers, handles, and objects


Result

Result

Returns 0 on success, or an error code on failure

**Example:**

```
Service srv;
nnServiceCreate(&srv, "pm:dmnt");

u64 pid = 0x12345;
struct {
    u64 location;
    u8 status;
} out;

Handle handle;
Result rc = nnServiceDispatchImpl(
    &srv,
    65000,
    &pid,
    sizeof(pid),
    &out,
    sizeof(out),
    (SfDispatchParams){
        .out_handle_attrs = {SfOutHandleAttr_HipcCopy},
        .out_handles = &handle
    }
);
```

---

## Dispatch Macros

For convenience, several macros are provided for common dispatch patterns:### 

nnServiceDispatch

Dispatch a request with no input or output data.

```
#define nnServiceDispatch(_s, _rid, ...)
```

### nnServiceDispatchIn

Dispatch a request with input data only.

```
#define nnServiceDispatchIn(_s, _rid, _in, ...)
```

### nnServiceDispatchOut

Dispatch a request with output data only.

```
#define nnServiceDispatchOut(_s, _rid, _out, ...)
```

### nnServiceDispatchInOut

Dispatch a request with both input and output data.

```
#define nnServiceDispatchInOut(_s, _rid, _in, _out, ...)
```

**Example:**

```
u64 input = 100;
u32 output;
Result rc = nnServiceDispatchInOut(&srv, 42, input, output);
```

---

## Ipc Class

The `Ipc` class provides high-level IPC utility methods.### 

getOwnProcessHandle

Retrieves a handle to the current process.

```
static Result Ipc::getOwnProcessHandle(Handle* handleOut)
```


handleOut

Handle*

required

Pointer to receive the process handle


Result

Result

Returns 0 on success, or an error code on failure

**Implementation:*** Gets the current process ID via `svcGetProcessId`

* Connects to the “pm:dmnt” service
* Uses command 65000 to get a copy of the process handle
* Closes the service connection

**Example:**

```
Handle procHandle;
Result rc = skyline::utils::Ipc::getOwnProcessHandle(&procHandle);
if (R_SUCCEEDED(rc)) {
    // Use process handle
}
```

The returned handle is a copy and must be closed with `svcCloseHandle` when no longer needed.

---

## Usage in Skyline

The IPC utilities are primarily used internally by Skyline for:* Getting the process handle during initialization

* Communicating with system services
* Creating custom service wrappers

**Example from main.cpp:**

```
// Populate our own process handle
envSetOwnProcessHandle(skyline::proc_handle::Get());
```

The `proc_handle::Get()` function uses `Ipc::getOwnProcessHandle` internally to obtain the handle.




# Memory Utilities

Virtual memory management and memory manipulation utilities

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline provides memory utilities for virtual memory management, JIT code execution, and low-level memory operations.## 

Virtual Memory Management

The virtual memory (virtmem) utilities provide address space reservation and management for JIT compilation and dynamic memory mapping.### 

virtmemReserve

Reserves a slice of general-purpose address space.

```
void* virtmemReserve(size_t size)
```


size

size_t

required

Size of the address space to reserve (rounded up to page alignment)


return

void*

Pointer to the reserved address space, or NULL on failure

**Use Cases:*** Allocating space for JIT-compiled code

* Reserving contiguous virtual address ranges
* Setting up custom memory mappings

**Example:**

```
size_t codeSize = 0x100000;  // 1 MB
void* codeRegion = virtmemReserve(codeSize);
if (codeRegion) {
    // Use the reserved region for JIT code
}
```

The size is automatically rounded up to page alignment (typically 0x1000 bytes).

---

### virtmemFree

Relinquishes a slice of address space reserved with virtmemReserve.

```
void virtmemFree(void* addr, size_t size)
```


addr

void*

required

Pointer to the address space slice to free


size

size_t

required

Size of the address space slice

Currently a no-op, but should still be called for future compatibility.

**Example:**

```
void* region = virtmemReserve(0x100000);
// ... use region ...
virtmemFree(region, 0x100000);
```

---

### virtmemReserveStack

Reserves a slice of address space inside the stack memory mapping region.

```
void* virtmemReserveStack(size_t size)
```


size

size_t

required

Size of the address space to reserve (rounded up to page alignment)


return

void*

Pointer to the reserved stack region, or NULL on failure

**Purpose:** This function is specifically for use with `svcMapMemory`, which requires addresses within the stack mapping region.**Example:**

```
size_t stackSize = 0x10000;  // 64 KB
void* stackRegion = virtmemReserveStack(stackSize);
if (stackRegion) {
    // Use with svcMapMemory
    Result rc = svcMapMemory(stackRegion, physicalAddr, stackSize);
}
```

Use this instead of `virtmemReserve` when you need to map memory with `svcMapMemory`.

---

### virtmemFreeStack

Relinquishes a slice of address space reserved with virtmemReserveStack.

```
void virtmemFreeStack(void* addr, size_t size)
```


addr

void*

required

Pointer to the stack region to free


size

size_t

required

Size of the stack region

Currently a no-op, but should still be called for future compatibility.

---

### virtmemSetup

Initializes the virtual memory subsystem.

```
void virtmemSetup()
```

**Purpose:*** Initializes internal state for virtmem functions

* Must be called before using any virtmem functions
* Required for libnx JIT functionality

**Usage in Skyline:**

```
extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // needed for libnx JIT
    skyline_main();
}
```

This must be called early during initialization, before any JIT code generation or virtmem operations.

---

## Memory Manipulation Functions

Low-level memory manipulation functions for copying, setting, and searching memory.### 

memset

Fills a block of memory with a specific value.

```
void* memset(void* src, int val, u64 num)
```


src

void*

required

Pointer to the memory block to fill


val

int

required

Value to set (converted to unsigned char)


num

u64

required

Number of bytes to set


return

void*

Returns the src pointer

**Example:**

```
char buffer[256];
memset(buffer, 0, sizeof(buffer));  // Zero out buffer
```

---

memcpy

Copies bytes from source to destination.

```
void* memcpy(void* dest, void const* src, u64 count)
```


dest

void*

required

Destination pointer


src

void const*

required

Source pointer


count

u64

required

Number of bytes to copy


return

void*

Returns the dest pointer

Source and destination must not overlap. Use `memmove` if they might overlap.

**Example:**

```
char src[10] = "hello";
char dest[10];
memcpy(dest, src, 6);  // Copy "hello\0"
```

### memmove

Copies bytes from source to destination, handling overlapping regions.

```
void* memmove(void* dest, const void* src, u64 count)
```


dest

void*

required

Destination pointer


src

const void*

required

Source pointer


count

u64

required

Number of bytes to copy


return

void*

Returns the dest pointer

**Example:**

```
char buffer[20] = "hello world";
// Shift data within the same buffer
memmove(buffer + 2, buffer, 11);
```

Use this instead of `memcpy` when source and destination regions might overlap.

---

memalign

Allocates aligned memory.

```
void* memalign(size_t alignment, size_t size)
```


alignment

size_t

required

Alignment requirement (must be a power of 2)


size

size_t

required

Number of bytes to allocate


return

void*

Pointer to aligned memory, or NULL on failure

**Example:**

```
// Allocate 64 KB stack with 0x1000 byte alignment
void* stack = memalign(0x1000, 0x10000);
if (stack) {
    // Use aligned memory
    free(stack);
}
```

**Usage in Skyline:**

```
// From SafeQueue.cpp
void SafeTaskQueue::startThread(s32 priority, s32 core, u64 stackSize) {
    void* stack = memalign(0x1000, stackSize);
    Result rc = nn::os::CreateThread(&thread, entrypoint, this, stack, stackSize, priority, core);
    nn::os::StartThread(&thread);
}
```

---

### memmem

Searches for a sequence of bytes within a memory region.

```
void* memmem(void* needle, size_t needleLen, void* haystack, size_t haystackLen)
```


needle

void*

required

Pointer to the byte sequence to search for


needleLen

size_t

required

Length of the needle in bytes


haystack

void*

required

Pointer to the memory region to search in


haystackLen

size_t

required

Length of the haystack in bytes


return

void*

Pointer to the first occurrence of needle in haystack, or NULL if not found

**Example:**

```
const char haystack[] = "Hello, World!";
const char needle[] = "World";
void* result = memmem((void*)needle, 5, (void*)haystack, 13);
if (result) {
    // Found at result
}
```

---

## Memory Safety Considerations

**Important Safety Guidelines:*** Always check return values from allocation functions (`virtmemReserve`, `memalign`)

* Ensure proper alignment for platform requirements (typically 0x1000 for code regions)
* Be careful with memory barriers when writing executable code
* Use `memmove` instead of `memcpy` when regions might overlap
* Free all allocated memory to prevent leaks

## JIT Code Generation Example

```
// Reserve memory for JIT code
size_t codeSize = 0x10000;
void* codeRegion = virtmemReserve(codeSize);

if (codeRegion) {
    // Make the region executable
    Result rc = svcSetProcessMemoryPermission(
        currentProcessHandle,
        (u64)codeRegion,
        codeSize,
        Perm_Rx | Perm_W
    );

    if (R_SUCCEEDED(rc)) {
        // Write JIT code to the region
        u32* code = (u32*)codeRegion;
        code[0] = 0xD65F03C0;  // ret instruction

        // Flush instruction cache
        armDCacheFlush(codeRegion, codeSize);
        armICacheInvalidate(codeRegion, codeSize);

        // Execute JIT code
        void (*jitFunc)() = (void(*)())codeRegion;
        jitFunc();
    }

    virtmemFree(codeRegion, codeSize);
}
```

---

## Integration with Skyline

Memory utilities are fundamental to Skyline’s operation:**Initialization (main.cpp):**

```
extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // Initialize virtmem for JIT
    skyline_main();
}
```

**Thread Stack Allocation (SafeQueue.cpp):**

```
void SafeTaskQueue::startThread(s32 priority, s32 core, u64 stackSize) {
    void* stack = memalign(0x1000, stackSize);
    Result rc = nn::os::CreateThread(&thread, entrypoint, this, 
                                      stack, stackSize, priority, core);
    nn::os::StartThread(&thread);
}
```

**Socket Buffer Allocation (main.cpp):**

```
const size_t poolSize = 0x600000;
void* socketPool = memalign(0x4000, poolSize);
nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);
```






# SafeQueue & SafeTaskQueue

Thread-safe queue implementations for task scheduling and concurrent operations

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline provides thread-safe queue implementations for managing concurrent operations and asynchronous task execution.## 

Task Class

The `Task` class encapsulates a unit of work with completion signaling.### 

Constructor (Default)

Creates a new task with no function.

```
Task()
```

**Implementation:*** Initializes a completion event with auto-clear mode

* Event starts in non-signaled state

---

### Constructor (With Function)

Creates a new task with a function to execute.

```
Task(std::function<void()> taskFunc)
```


taskFunc

std::function<void()>

required

Function to execute when the task runs

**Example:**

```
auto task = new skyline::utils::Task([]() {
    // Task code here
    skyline::logger::s_Instance->Log("Task running\n");
});
```

---

### Public Members


taskFunc

std::function<void()>

The function to execute for this task


completionEvent

nn::os::EventType

Event that is signaled when the task completes execution

---

### Destructor

```
~Task()
```

**Implementation:*** Finalizes the completion event

* Cleans up event resources

Do not destroy a Task while it is being executed by a SafeTaskQueue.

---

## SafeQueue Template

A thread-safe generic queue built on Nintendo’s message queue primitives.### 

Template Declaration

```
template <typename T>
class SafeQueue
```


T

typename

required

Type of elements stored in the queue (must be pointer-sized)

---

### Constructor

Creates a new SafeQueue with a fixed capacity.

```
SafeQueue(u64 count)
```


count

u64

required

Maximum number of elements the queue can hold

**Implementation:*** Allocates a buffer for queue storage

* Initializes Nintendo’s message queue with the buffer
* Queue capacity is fixed at creation time

**Example:**

```
// Create a queue that can hold 100 items
auto queue = new skyline::utils::SafeQueue<MyData>(100);
```

---

### push (Blocking)

Adds an element to the queue, blocking if full.

```
void push(std::unique_ptr<T>* ptr)
```


ptr

std::unique_ptr`<T>`*

required

Pointer to unique_ptr to push onto the queue

**Behavior:*** Blocks indefinitely if the queue is full

* Thread-safe, can be called from multiple threads

**Example:**

```
auto item = std::make_unique<MyData>();
queue->push(&item);
```

---

### push (Timed)

Adds an element to the queue with a timeout.

```
bool push(std::unique_ptr<T>* ptr, nn::TimeSpan span)
```


ptr

std::unique_ptr`<T>`*

required

Pointer to unique_ptr to push onto the queue


span

nn::TimeSpan

required

Maximum time to wait for space in the queue


return

bool

Returns true if the item was pushed, false if timeout occurred

**Example:**

```
auto item = std::make_unique<MyData>();
if (queue->push(&item, nn::TimeSpan::FromSeconds(5))) {
    // Successfully pushed within 5 seconds
} else {
    // Timeout - queue was full
}
```

---

### pop (Blocking)

Removes an element from the queue, blocking if empty.

```
void pop(std::unique_ptr<T>** ptr)
```


ptr

std::unique_ptr`<T>`**

required

Pointer to receive the popped unique_ptr

**Behavior:*** Blocks indefinitely if the queue is empty

* Thread-safe, can be called from multiple threads

**Example:**

```
std::unique_ptr<MyData>* item;
queue->pop(&item);
// Use item->get() to access the data
```

---

### pop (Timed)

Removes an element from the queue with a timeout.

```
bool pop(std::unique_ptr<T>** ptr, nn::TimeSpan span)
```


ptr

std::unique_ptr`<T>`**

required

Pointer to receive the popped unique_ptr


span

nn::TimeSpan

required

Maximum time to wait for an item


return

bool

Returns true if an item was popped, false if timeout occurred

**Example:**

```
std::unique_ptr<MyData>* item;
if (queue->pop(&item, nn::TimeSpan::FromMilliSeconds(100))) {
    // Got an item
} else {
    // Timeout - queue was empty
}
```

---

### tryPush

Attempts to add an element without blocking.

```
bool tryPush(std::unique_ptr<T>* ptr)
```


ptr

std::unique_ptr`<T>`*

required

Pointer to unique_ptr to push onto the queue


return

bool

Returns true if the item was pushed, false if queue was full

**Example:**

```
auto item = std::make_unique<MyData>();
if (!queue->tryPush(&item)) {
    // Queue is full, handle overflow
}
```

---

### tryPop

Attempts to remove an element without blocking.

```
bool tryPop(std::unique_ptr<T>** ptr)
```


ptr

std::unique_ptr`<T>`**

required

Pointer to receive the popped unique_ptr


return

bool

Returns true if an item was popped, false if queue was empty

**Example:**

```
std::unique_ptr<MyData>* item;
if (queue->tryPop(&item)) {
    // Process item
}
```

---

### Destructor

```
~SafeQueue()
```

**Implementation:*** Finalizes the message queue

* Frees the internal buffer

Ensure all threads have stopped using the queue before destroying it.

---

## SafeTaskQueue Class

A specialized queue for executing tasks on a dedicated worker thread.### 

Class Definition

```
class SafeTaskQueue : public SafeQueue<Task>
```

Inherits from `SafeQueue<Task>`, adding thread management for automatic task execution.---

### Constructor

Creates a new task queue with specified capacity.

```
SafeTaskQueue(u64 count)
```


count

u64

required

Maximum number of tasks the queue can hold

**Example:**

```
// Create a task queue that can hold 100 tasks
auto taskQueue = new skyline::utils::SafeTaskQueue(100);
```

---

### startThread

Starts the worker thread that processes tasks.

```
void startThread(s32 priority, s32 core, u64 stackSize)
```


priority

s32

required

Thread priority (lower numbers = higher priority)


core

s32

required

CPU core to run the thread on (0-3, or -2 for any core)


stackSize

u64

required

Size of the thread’s stack in bytes

**Implementation:*** Allocates an aligned stack with `memalign(0x1000, stackSize)`

* Creates a thread using `nn::os::CreateThread`
* Starts the thread immediately
* Logs success/failure to the logger

**Example:**

```
taskQueue->startThread(
    20,        // Priority 20
    3,         // Run on core 3
    0x10000    // 64 KB stack
);
```

Thread priorities on Nintendo Switch range from 0 (highest) to 31 (lowest). System threads typically use 16-31.

---

### Public Members


thread

nn::os::ThreadType

The worker thread that processes tasks from the queue

---

### _threadEntrypoint (Internal)

The worker thread’s main loop (internal implementation).

```
void _threadEntrypoint()
```

**Implementation:**1. Logs that the thread has started

1. Enters an infinite loop:
   * Waits for a task with 10ms timeout using `pop(&taskptr, nn::TimeSpan::FromNanoSeconds(10000000))`
   * If a task is received:
     * Executes the task’s function
     * Signals the task’s completion event
     * Releases and deletes the task
   * Otherwise, continues waiting

This is an internal method. Do not call directly.

---

## Usage Examples

### Basic Task Execution

```
// Create and start a task queue
auto taskQueue = new skyline::utils::SafeTaskQueue(100);
taskQueue->startThread(20, 3, 0x10000);

// Create a task
auto task = new skyline::utils::Task([]() {
    skyline::logger::s_Instance->Log("Hello from task!\n");
});

// Push the task (wrap in unique_ptr)
auto taskPtr = new std::unique_ptr<skyline::utils::Task>(task);
taskQueue->push(taskPtr);

// Wait for completion
nn::os::WaitEvent(&task->completionEvent);
```

---

### Real-World Example from Skyline

From `main.cpp`, showing plugin loading on a background thread:

```
// Create a task for post-romfs initialization
static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    // Initialize socket pool
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    // Start logger thread
    skyline::logger::s_Instance->StartThread();

    // Load plugins
    auto manager = new skyline::plugin::Manager();
    manager->LoadPluginsImpl();
}};

// Later, when romfs is mounted:
Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = nnFsMountRomImpl(path, buffer, size);

    g_MountRomInit.call_once([]() {
        // Create and start task queue
        skyline::utils::SafeTaskQueue* taskQueue =
            new skyline::utils::SafeTaskQueue(100);
        taskQueue->startThread(20, 3, 0x10000);

        // Queue the initialization task
        taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));

        // Wait for it to complete
        nn::os::WaitEvent(&after_romfs_task->completionEvent);
    });

    return rc;
}
```

---

### Multiple Tasks

```
auto taskQueue = new skyline::utils::SafeTaskQueue(10);
taskQueue->startThread(20, 3, 0x10000);

// Queue multiple tasks
for (int i = 0; i < 5; i++) {
    auto task = new skyline::utils::Task([i]() {
        skyline::logger::s_Instance->LogFormat("Task %d executing\n", i);
    });
    taskQueue->push(new std::unique_ptr<skyline::utils::Task>(task));
}
```

---

### Task with Completion Waiting

```
// Create a long-running task
auto task = new skyline::utils::Task([]() {
    // Simulate work
    nn::os::SleepThread(nn::TimeSpan::FromSeconds(2));
    skyline::logger::s_Instance->Log("Work complete!\n");
});

// Queue the task
auto taskPtr = new std::unique_ptr<skyline::utils::Task>(task);
taskQueue->push(taskPtr);

// Do other work...

// Wait for task to complete before proceeding
nn::os::WaitEvent(&task->completionEvent);
skyline::logger::s_Instance->Log("Task finished, continuing...\n");
```

---

## Thread Safety Notes

**Thread Safety Guarantees:*** All queue operations (`push`, `pop`, `tryPush`, `tryPop`) are thread-safe

* Multiple threads can safely push and pop simultaneously
* Built on Nintendo’s `nn::os::MessageQueueType` primitives
* Uses proper synchronization internally

**Important Considerations:*** Queue capacity is fixed at creation - cannot grow dynamically

* Blocking operations (`push`, `pop`) will wait indefinitely if queue is full/empty
* Always use timed variants (`push` with timeout, `pop` with timeout) if you need responsiveness
* Don’t destroy a Task until its completion event has been signaled
* The worker thread runs forever - plan your shutdown sequence accordingly

---

## Best Practices

1. **Queue Sizing** : Choose a queue size based on expected task load:

```
   // For occasional tasks
   new SafeTaskQueue(10);

   // For high-frequency tasks
   new SafeTaskQueue(100);
```

1. **Thread Priority** : Use appropriate priority for your workload:

```
   // High priority (use sparingly)
   taskQueue->startThread(16, 3, 0x10000);

   // Normal background work
   taskQueue->startThread(20, 3, 0x10000);

   // Low priority
   taskQueue->startThread(24, 3, 0x10000);
```

1. **Stack Size** : Allocate enough stack for your tasks:

```
   // Small tasks
   taskQueue->startThread(20, 3, 0x4000);   // 16 KB

   // Normal tasks (recommended)
   taskQueue->startThread(20, 3, 0x10000);  // 64 KB

   // Heavy tasks with deep call stacks
   taskQueue->startThread(20, 3, 0x20000);  // 128 KB
```

1. **Error Handling** : Always check for queue full conditions:

```
   auto task = new std::unique_ptr<skyline::utils::Task>(
       new skyline::utils::Task([]() { /* work */ })
   );

   if (!taskQueue->tryPush(task)) {
       skyline::logger::s_Instance->Log("Queue full!\n");
       // Handle overflow
   }
```
