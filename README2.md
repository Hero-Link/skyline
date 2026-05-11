# Introduction

Learn about Skyline, a runtime hooking and code patching framework for Super Smash Bros. Ultimate

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

What is Skyline?

Skyline is an environment for **linking, runtime hooking, and code patching** in Super Smash Bros. Ultimate on Nintendo Switch. It enables developers to create plugins that can modify game behavior, hook functions, and extend functionality without modifying the game’s original code.Skyline runs as a layer between the game and the Atmosphere custom firmware, intercepting function calls and loading custom code at runtime.## 

Architecture

Skyline operates through several key components:### 

Plugin Loading System

The **PluginManager** (`skyline/plugin/PluginManager.hpp:13`) manages the entire plugin lifecycle:* Recursively walks the `romfs:/skyline/plugins` directory to discover NRO plugin files

* Validates plugin files and calculates SHA256 hashes for registration
* Uses Nintendo’s `nn::ro` (relocatable object) module system to load plugins dynamically
* Executes each plugin’s `main()` function as an entry point after loading

Plugins are loaded from: `atmosphere/contents/<game titleid>/romfs/skyline/plugins/`### 

Runtime Hooking

Skyline uses **And64InlineHook** (`skyline/inlinehook/And64InlineHook.hpp:58`) for ARM64 function hooking:* `A64HookFunction()` - Redirects function calls to custom implementations while preserving the original

* `A64InlineHook()` - Injects code directly into the instruction stream
* Supports up to 2048 simultaneous hooks (`A64_MAX_BACKUPS`)

Hooks are installed during initialization in `source/main.cpp:106` after calling `A64HookInit()`.### 

Logger System

Skyline provides multiple logging backends (`skyline/logger/Logger.hpp:14`):* **TcpLogger** - Network logging over TCP sockets (default)

* **SdLogger** - Writes logs to SD card files
* **KernelLogger** - Low-level kernel debug output
* **DummyLogger** - No-op logger for production builds

The logger is initialized early in `source/main.cpp:112` and is available globally via `skyline::logger::s_Instance`.## 

Key Features

## Dynamic Plugin Loading

Load NRO plugins at runtime without rebuilding the game. Plugins are automatically discovered and executed.

## Function Hooking

Intercept and replace any game function using ARM64 inline hooks. Access original function implementations.

## Memory Patching

Direct memory access and patching with controlled page management and JIT compilation support.

## Exception Handling

Custom exception handler captures crashes and dumps register states to the logger for debugging.

## Nintendo NN SDK

Full access to Nintendo’s nn SDK including filesystem, networking, graphics, and system services.

## Development Tools

Built-in logging, debugging utilities, and task queue system for asynchronous operations.

## Relationship to OdysseyReversed and Starlight

Skyline is  **derived from OdysseyReversed and Starlight** , projects originally created for Super Mario Odyssey modding. The framework was adapted and extended by contributors including:* **shibbo** - Original OdysseyReversed implementation

* **shadowninja108** - Core Skyline development
* **3096** - Game-specific ports and compatibility
* **Thog** - rtld (runtime loader) expertise
* **jakibaki** - Runtime hooking advice

Skyline inherits the plugin architecture and hooking system from these projects while adding Super Smash Bros. Ultimate specific features.## 

Game Compatibility

While Skyline was designed for **Super Smash Bros. Ultimate** (Title ID: `01006A800016E000`), it has been ported to other games:

Skyline’s initialization method is not compatible with every game out of the box. Different games may require modified versions of Skyline.

**Known Ports:*** [Animal Crossing: New Horizons](https://github.com/3096/skyline)

* [Pokémon Sword/Shield](https://github.com/3096/skyline/tree/sword)
* [Xenoblade Chronicles: Definitive Edition](https://github.com/3096/skyline/tree/xde)
* [Dragon Quest XI S](https://github.com/3096/skyline/tree/jack)
* [Persona 5 Royal](https://github.com/Raytwo/p5rcbt)
* [Persona 5 Strikers](https://github.com/Raytwo/masquerade-rs)
* [Fire Emblem: Three Houses](https://github.com/three-houses-research-team/aldebaran-rs)

## Technical Requirements

* **Nintendo Switch** running Atmosphere CFW
* **Super Smash Bros. Ultimate** (or compatible game)
* **devkitA64** toolchain for building plugins
* **libnx** for Switch development libraries

The `main.npdm` file in Skyline’s exefs directory is specifically configured for Super Smash Bros. Ultimate. Other games require their own npdm files created with HACTool and npdmtool.

## Next Steps

[InstallationInstall Skyline on your Nintendo Switch](https://mintlify.wiki/skyline-dev/skyline/installation)

[Quick StartCreate your first Skyline plugin](https://mintlify.wiki/skyline-dev/skyline/quickstart)



# Installation

Install Skyline on your Nintendo Switch with Atmosphere CFW

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

# Installing Skyline

This guide will walk you through installing Skyline on your Nintendo Switch running Atmosphere custom firmware.

Skyline requires a Nintendo Switch with Atmosphere CFW installed. This guide assumes you already have a working CFW setup. Modifying your Switch may void your warranty.

## Prerequisites

Before installing Skyline, ensure you have:* Nintendo Switch with Atmosphere CFW installed

* SD card with sufficient space (at least 500MB free)
* Super Smash Bros. Ultimate installed (or another compatible game)
* USB or FTP access to your Switch’s SD card

## Installation Steps

1

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Download Skyline

Head to the [Skyline releases page](https://github.com/skyline-dev/skyline/releases) and download the latest `skyline.zip` file.The release archive contains:* `exefs/` directory with Skyline’s NSO and NPDM files

* Subsdk libraries
* Runtime initialization code

2

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Locate Your Game's Title ID

Find your game’s Title ID. For Super Smash Bros. Ultimate:

```
01006A800016E000
```

Other common Title IDs:* Animal Crossing: `01006F8002326000`

* Pokémon Sword: `0100ABF008968000`
* Pokémon Shield: `01008DB008C2C000`

You can find Title IDs using tools like nxdumptool or by checking SwitchBrew’s game list.

3

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Extract to Atmosphere Directory

Copy the `exefs` directory from the downloaded archive to your SD card:

```
sd:/atmosphere/contents/<TITLE_ID>/exefs/
```

For Super Smash Bros. Ultimate, the full path is:

```
sd:/atmosphere/contents/01006A800016E000/exefs/
```

The `exefs` directory should contain:* `main.npdm` - Process metadata descriptor

* `subsdk*` - System libraries
* Other Skyline core files

4

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Handle the main.npdm File

The `main.npdm` file is **game-specific** and defines process permissions and metadata.

**Important:** Remove the `main.npdm` file from the exefs directory if you are modding any game OTHER than Super Smash Bros. Ultimate.

For games other than SSBU:1. Delete the included `main.npdm` file

1. Create a custom npdm file using:
   * **HACTool** - Extract original npdm from game
   * **npdmtool** - Modify permissions and metadata
2. Or use an npdm provided by the plugin/mod developer

The npdm file controls:* Memory permissions

* System call access
* Thread priority
* Available system resources

5

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Create Plugin Directory

Create the directory structure for plugins:

```
sd:/atmosphere/contents/<TITLE_ID>/romfs/skyline/plugins/
```

For Super Smash Bros. Ultimate:

```
sd:/atmosphere/contents/01006A800016E000/romfs/skyline/plugins/
```

This is where you’ll place plugin `.nro` files. Skyline automatically discovers and loads all NRO files in this directory and its subdirectories.

The path `skyline/plugins` is hardcoded in `PluginManager.hpp:13` as `PLUGIN_PATH`.

6

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Verify Installation

Your SD card structure should look like:

```
SD Card
└── atmosphere/
    └── contents/
        └── 01006A800016E000/
            ├── exefs/
            │   ├── main.npdm
            │   ├── subsdk*
            │   └── [other skyline files]
            └── romfs/
                └── skyline/
                    └── plugins/
                        └── [your .nro plugins here]
```

7

[](https://mintlify.wiki/skyline-dev/skyline/installation#)

Launch the Game

1. Safely eject your SD card and reinsert it into your Switch
2. Boot into Atmosphere CFW
3. Launch Super Smash Bros. Ultimate (or your target game)

Skyline will initialize automatically when the game starts.

## Verification

To verify Skyline is working:1.  **Check for crashes** : If the game launches normally, Skyline loaded successfully

1. **Enable TCP logging** : Connect to your Switch’s IP on port 6969 to view logs
2. **Watch for initialization messages** : Look for `[skyline_main] Beginning initialization.` in logs
3. **Check plugin loading** : Messages like `[PluginManager] Loaded '<plugin>'` confirm plugins loaded

### TCP Logging

Skyline uses `TcpLogger` by default (`source/main.cpp:112`). Connect using:

```
telnet <switch-ip> 6969
```

Or use netcat:

```
nc <switch-ip> 6969
```

You should see output like:

```
[skyline_main] Beginning initialization.
[skyline_main] text: 0x... | rodata: 0x... | data: 0x... | bss: 0x... | heap: 0x...
[skyline_main] Ran hooked nn::ro::initialize (0x0)
[PluginManager] Initializing plugins...
[PluginManager] Opening plugins...
[PluginManager] Loading plugins...
```

## Troubleshooting

### Game Crashes on Launch

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="incompatible-main-npdm-file accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="incompatible-main-npdm-file" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Incompatible main.npdm file</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="corrupted-skyline-files accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="corrupted-skyline-files" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Corrupted Skyline files</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="atmosphere-version-mismatch accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="atmosphere-version-mismatch" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Atmosphere version mismatch</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="game-not-supported accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="game-not-supported" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Game not supported</p></div></summary>

[](https://mintlify.wiki/skyline-dev/skyline/introduction#game-compatibility)

</details>

Plugins Not Loading

Plugins must be valid NRO (Nintendo Relocatable Object) files. Standard NSO or homebrew NRO files will not work.

**Common issues:**1.  **Wrong directory** : Verify plugins are in `romfs/skyline/plugins/`, not `exefs/`

1. **Invalid NRO** : Check that the plugin was compiled correctly for Skyline
2. **Duplicate plugins** : Skyline skips duplicate hashes (detected by SHA256)
3. **Missing symbols** : Plugin may be missing required exports

Check the TCP logs for specific error messages:

```
[PluginManager] Failed to get NRO buffer size for '<plugin>' (0x...), not an nro? Skipping.
[PluginManager] Failed to load '<plugin>' (0x...). Skipping.
[PluginManager] '<plugin>' is detected duplicate, Skipping.
```

### No Log Output

1. **Check network connection** : Ensure your Switch and PC are on the same network
2. **Firewall blocking** : Allow port 6969 through your firewall
3. **Socket initialization failed** : Some games conflict with Skyline’s socket initialization
4. **NOLOG build** : If Skyline was compiled with `-DNOLOG`, logging is disabled

## Compatibility Issues

Skyline’s initialization is not compatible with every game out of the box. Different games may require forked versions of Skyline.

From `README.md:12-14`:> Skyline’s way of initializing is not compatible with every game out of the box due to various reasons that can’t be anticipated. Mod developers might sometimes modify Skyline to make it compatible with a specific game which you should use if you are running into crashes.

If you experience crashes:1. Check if a game-specific fork exists (see [Introduction - Game Compatibility](https://mintlify.wiki/skyline-dev/skyline/introduction#game-compatibility))

1. Try building from a fork that supports your game
2. Create your own compatibility patches (advanced)

## Next Steps

Now that Skyline is installed, you’re ready to:[Quick StartCreate your first Skyline plugin](https://mintlify.wiki/skyline-dev/skyline/quickstart)

[Plugin DevelopmentLearn about plugin architecture and APIs](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started)



# Quick Start

Create your first Skyline plugin in minutes

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

# Quick Start Guide

This guide will walk you through creating your first Skyline plugin - a simple mod that hooks a function and logs messages.## 

Prerequisites

Before creating plugins, ensure you have:* **devkitPro** installed with devkitA64

* **libnx** Switch development libraries
* **Skyline** installed on your Switch ([Installation Guide](https://mintlify.wiki/skyline-dev/skyline/installation))
* Basic knowledge of C/C++
* A text editor or IDE

### Setting Up devkitPro

1

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Install devkitPro

Follow the [official devkitPro installation guide](https://devkitpro.org/wiki/Getting_Started) for your platform.On Linux:

```
wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.amd64.deb
sudo dpkg -i devkitpro-pacman.amd64.deb
```

2

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Install Switch Development Tools

```
sudo dkp-pacman -S switch-dev
```

This installs:* devkitA64 ARM64 compiler

* libnx Nintendo Switch libraries
* Switch build tools

3

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Set Environment Variables

Add to your `.bashrc` or `.zshrc`:

```
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export DEVKITA64=$DEVKITPRO/devkitA64
```

Reload your shell:

```
source ~/.bashrc
```

## Plugin Structure

A Skyline plugin is an **NRO (Nintendo Relocatable Object)** file with:1. A `main()` function as the entry point

1. Hooks to game functions using `A64HookFunction()`
2. Access to Skyline’s logger and utilities

### Basic Plugin Anatomy

```
#include "main.hpp"  // Skyline main header

extern "C" void main() {
    // Your plugin initialization code
}
```

The `main()` function is called by the PluginManager (`PluginManager.cpp:194`) after the plugin is loaded:

```
void (*pluginEntrypoint)() = NULL;
nn::ro::LookupModuleSymbol(reinterpret_cast<uintptr_t*>(&pluginEntrypoint), &plugin.Module, "main");
if (pluginEntrypoint != NULL) {
    pluginEntrypoint();
}
```

## Creating Your First Plugin

Let’s create a simple plugin that logs messages and hooks a function.

1

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Create Project Directory

```
mkdir my-first-plugin
cd my-first-plugin
```

2

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Create Source File

Create `source/main.cpp`:

```
#include "main.hpp"

// Original function pointer
void (*original_function)();

// Our hook function
void hooked_function() {
    skyline::logger::s_Instance->Log("[MyPlugin] Hook called!\n");

    // Call the original function
    if (original_function) {
        original_function();
    }
}

extern "C" void main() {
    skyline::logger::s_Instance->Log("[MyPlugin] Initializing...\n");

    // Log some useful information
    skyline::logger::s_Instance->LogFormat(
        "[MyPlugin] Text: 0x%" PRIx64 " | Heap: 0x%" PRIx64 "\n",
        skyline::utils::g_MainTextAddr,
        skyline::utils::g_MainHeapAddr
    );

    // Install a hook (example - replace with actual game function)
    // A64HookFunction(
    //     reinterpret_cast<void*>(GAME_FUNCTION_ADDRESS),
    //     reinterpret_cast<void*>(hooked_function),
    //     (void**)&original_function
    // );

    skyline::logger::s_Instance->Log("[MyPlugin] Initialization complete!\n");
}
```

The `skyline::logger::s_Instance` global logger is initialized in `main.cpp:112` and available to all plugins.

3

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Create Makefile

Create `Makefile`:

```
.PHONY: all clean

NAME := my-first-plugin

all:
	$(MAKE) -f plugin.mk TARGET=$(NAME)

clean:
	$(MAKE) -f plugin.mk TARGET=$(NAME) clean
	rm -f $(NAME).nro
```

4

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Create Build Rules

Create `plugin.mk` (adapted from Skyline’s build system):

```
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

TARGET   ?= plugin
BUILD    := build
SOURCES  := source
INCLUDES := include

# Architecture and flags
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC

CFLAGS := -g -Wall -O2 -ffunction-sections \
          $(ARCH) $(INCLUDE) -D__SWITCH__

CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
           -Wl,-Map,$(TARGET).map

LIBS := -lnx

# Source files
CPPFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
OFILES   := $(CPPFILES:.cpp=.o)

all: $(TARGET).nro

%.nro: %.elf
	elf2nro $< $@
	@echo "Built: $(TARGET).nro"

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(OFILES) $(TARGET).elf $(TARGET).nro $(TARGET).map
```

5

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Build the Plugin

```
make
```

This compiles your plugin into `my-first-plugin.nro`.

If you get errors about missing headers, ensure Skyline’s include directory is in your include path, or copy the necessary headers from the Skyline repository.

6

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Install the Plugin

Copy the `.nro` file to your Switch:

```
cp my-first-plugin.nro /path/to/sd/atmosphere/contents/01006A800016E000/romfs/skyline/plugins/
```

Or use FTP:

```
curl -T my-first-plugin.nro ftp://switch-ip/atmosphere/contents/01006A800016E000/romfs/skyline/plugins/
```

7

[](https://mintlify.wiki/skyline-dev/skyline/quickstart#)

Test the Plugin

1. Insert your SD card back into your Switch
2. Launch Super Smash Bros. Ultimate
3. Connect to the TCP logger:

   ```
   nc <switch-ip> 6969
   ```
4. Look for your plugin’s messages:

   ```
   [PluginManager] Loading plugins...
   [PluginManager] Loaded 'rom:/skyline/plugins/my-first-plugin.nro'
   [PluginManager] Running `main` for rom:/skyline/plugins/my-first-plugin.nro
   [MyPlugin] Initializing...
   [MyPlugin] Text: 0x... | Heap: 0x...
   [MyPlugin] Initialization complete!
   ```

## Using the Logger

Skyline provides a powerful logging system defined in `skyline/logger/Logger.hpp:14`.### 

Basic Logging

```
// Log a simple string
skyline::logger::s_Instance->Log("Hello from plugin\n");

// Log a std::string
std::string message = "Dynamic message";
skyline::logger::s_Instance->Log(message);
```

### Formatted Logging

```
// Printf-style formatting
skyline::logger::s_Instance->LogFormat(
    "Value: %d, Hex: 0x%X, Pointer: %p\n",
    42, 0xDEADBEEF, some_pointer
);

// 64-bit values
skyline::logger::s_Instance->LogFormat(
    "Address: 0x%" PRIx64 "\n",
    some_64bit_address
);
```

### Raw Output

```
// Send raw bytes (useful for binary data)
char data[] = "Raw data";
skyline::logger::s_Instance->SendRaw(data, sizeof(data));

// Send raw string
skyline::logger::s_Instance->SendRaw("No buffering\n");
```

`SendRaw()` bypasses the logging queue and sends immediately. Use `Log()` for normal messages.

## Hooking Functions

The core of Skyline plugins is **function hooking** using `A64HookFunction()` from `And64InlineHook.hpp:58`.### 

Hook Signature

```
extern "C" void A64HookFunction(
    void* const symbol,      // Address of function to hook
    void* const replace,     // Your replacement function
    void** result           // Pointer to store original function
);
```

### Basic Hook Example

```
// Function pointers
typedef void (*SomeGameFunction)(int param);
SomeGameFunction original_game_function = nullptr;

// Your hook
void my_hook(int param) {
    skyline::logger::s_Instance->LogFormat(
        "[Hook] Called with param: %d\n", param
    );

    // Modify behavior
    if (param == 42) {
        skyline::logger::s_Instance->Log("[Hook] Special value detected!\n");
    }

    // Call original with modified parameter
    original_game_function(param * 2);
}

// Install the hook
extern "C" void main() {
    A64HookFunction(
        reinterpret_cast<void*>(0x71000XXXXX),  // Game function address
        reinterpret_cast<void*>(my_hook),
        (void**)&original_game_function
    );
}
```

### Hook with Return Value

```
typedef int (*CalculateFunction)(float a, float b);
CalculateFunction original_calculate = nullptr;

int hooked_calculate(float a, float b) {
    int result = original_calculate(a, b);

    skyline::logger::s_Instance->LogFormat(
        "[Hook] calculate(%.2f, %.2f) = %d\n",
        a, b, result
    );

    // Modify return value
    return result + 10;
}
```

### Hook with Complex Types

```
struct Vector3 {
    float x, y, z;
};

typedef void (*SetPositionFunc)(void* entity, Vector3* pos);
SetPositionFunc original_set_position = nullptr;

void hooked_set_position(void* entity, Vector3* pos) {
    skyline::logger::s_Instance->LogFormat(
        "[Hook] SetPosition: (%.2f, %.2f, %.2f)\n",
        pos->x, pos->y, pos->z
    );

    // Prevent going out of bounds
    if (pos->y < 0) {
        skyline::logger::s_Instance->Log("[Hook] Prevented OOB!\n");
        pos->y = 0;
    }

    original_set_position(entity, pos);
}
```

Use IDA Pro, Ghidra, or other reverse engineering tools to find function addresses in the game binary.

## Complete Working Example

Here’s a complete plugin that demonstrates logging, hooking, and error handling:source/main.cpp

Makefile

```
#include "main.hpp"

// Hook for a hypothetical game initialization function
typedef void (*GameInitFunc)();
GameInitFunc original_game_init = nullptr;

void hooked_game_init() {
    skyline::logger::s_Instance->Log("[MyMod] Game initializing!\n");
    skyline::logger::s_Instance->Log("[MyMod] Installing custom features...\n");

    // Call original initialization
    if (original_game_init) {
        original_game_init();
    }

    skyline::logger::s_Instance->Log("[MyMod] Game init hooked successfully!\n");
}

// Hook for a frame update function
typedef void (*UpdateFunc)(float deltaTime);
UpdateFunc original_update = nullptr;
int frame_count = 0;

void hooked_update(float deltaTime) {
    frame_count++;

    // Log every 60 frames (about once per second)
    if (frame_count % 60 == 0) {
        skyline::logger::s_Instance->LogFormat(
            "[MyMod] Frame %d, DeltaTime: %.4f\n",
            frame_count, deltaTime
        );
    }

    // Call original update
    original_update(deltaTime);
}

extern "C" void main() {
    skyline::logger::s_Instance->Log("====================================\n");
    skyline::logger::s_Instance->Log("[MyMod] Plugin Loading\n");
    skyline::logger::s_Instance->Log("[MyMod] Version: 1.0.0\n");
    skyline::logger::s_Instance->Log("====================================\n");

    // Log memory regions
    skyline::logger::s_Instance->LogFormat(
        "[MyMod] Memory Layout:\n"
        "  Text:   0x%" PRIx64 "\n"
        "  RoData: 0x%" PRIx64 "\n"
        "  Data:   0x%" PRIx64 "\n"
        "  BSS:    0x%" PRIx64 "\n"
        "  Heap:   0x%" PRIx64 "\n",
        skyline::utils::g_MainTextAddr,
        skyline::utils::g_MainRodataAddr,
        skyline::utils::g_MainDataAddr,
        skyline::utils::g_MainBssAddr,
        skyline::utils::g_MainHeapAddr
    );

    // Install hooks
    skyline::logger::s_Instance->Log("[MyMod] Installing hooks...\n");

    // Example: Hook game init (replace with actual address)
    // A64HookFunction(
    //     reinterpret_cast<void*>(0x71XXXXXXXX),
    //     reinterpret_cast<void*>(hooked_game_init),
    //     (void**)&original_game_init
    // );

    // Example: Hook update loop (replace with actual address)
    // A64HookFunction(
    //     reinterpret_cast<void*>(0x71YYYYYYYY),
    //     reinterpret_cast<void*>(hooked_update),
    //     (void**)&original_update
    // );

    skyline::logger::s_Instance->Log("[MyMod] Hooks installed!\n");
    skyline::logger::s_Instance->Log("[MyMod] Plugin loaded successfully\n");
}
```

## Best Practices

## Always Check Pointers

Before calling original functions, verify they’re not null:

```
if (original_func) {
    original_func();
}
```

## Use Descriptive Prefixes

Prefix all log messages with your plugin name:

```
[MyPlugin] Message here
```

## Handle Exceptions

Skyline’s exception handler will catch crashes, but try-catch around risky operations:

```
try {
    risky_operation();
} catch (...) {
    logger->Log("Error occurred\n");
}
```

## Log Important Events

Log plugin lifecycle events:* Initialization

* Hook installation
* Errors and warnings
* Important state changes

## Debugging Tips

### View All Loaded Plugins

Check the TCP log output after game launch:

```
[PluginManager] Initializing plugins...
[PluginManager] Opening plugins...
[PluginManager] Read rom:/skyline/plugins/plugin1.nro
[PluginManager] Read rom:/skyline/plugins/plugin2.nro
[PluginManager] Loading plugins...
[PluginManager] Loaded 'rom:/skyline/plugins/plugin1.nro'
[PluginManager] Running `main` for rom:/skyline/plugins/plugin1.nro
```

### Common Errors

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="failed-to-get-nro-buffer-size accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="failed-to-get-nro-buffer-size" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Failed to get NRO buffer size</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="failed-to-lookup-symbol accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="failed-to-lookup-symbol" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Failed to lookup symbol</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="game-crashes-after-hook accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="game-crashes-after-hook" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Game crashes after hook</p></div></summary>

</details

## Next Steps

[Plugin StructureLearn about plugin structure and build system](https://mintlify.wiki/skyline-dev/skyline/plugins/plugin-structure)

[API ReferenceExplore Skyline’s full API documentation](https://mintlify.wiki/skyline-dev/skyline/api/hooks/function-hooks)

[Example PluginsBrowse community plugin examples](https://mintlify.wiki/skyline-dev/skyline/plugins/examples)

[TroubleshootingDebug issues and solve common problems](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting)



# System Architecture

Understanding the overall architecture and initialization flow of Skyline

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline is a runtime hooking and code patching framework that runs as a plugin for Nintendo Switch games. It provides a comprehensive system for intercepting function calls, loading dynamic modules, and modifying game behavior at runtime.## 

Core Components

The framework consists of several key subsystems that work together:[Hooking SystemA64InlineHook provides ARM64 instruction-level hooking capabilities](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system)

[Plugin SystemDynamic loading and management of NRO plugin modules](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system)

[Memory ManagementVirtual memory setup and JIT compilation support](https://mintlify.wiki/skyline-dev/skyline/core/memory-management)

## Logger System

TCP and SD card logging for debugging and diagnostics

## Initialization Flow

The initialization sequence is critical to understanding how Skyline integrates into a game process. Here’s the complete flow from `source/main.cpp`:

1

[](https://mintlify.wiki/skyline-dev/skyline/core/architecture#)

Entry Point - skyline_init()

Called when Skyline is first loaded into the game process.**source/main.cpp:142-147**

```
extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // needed for libnx JIT

    skyline_main();
}
```

This function performs two critical setup operations:* Initializes utility subsystems and discovers memory regions

* Sets up virtual memory for JIT compilation support

2

[](https://mintlify.wiki/skyline-dev/skyline/core/architecture#)

Main Initialization - skyline_main()

Initializes core systems and sets up critical hooks.**source/main.cpp:101-140**

```
void skyline_main() {
    // populate our own process handle
    envSetOwnProcessHandle(skyline::proc_handle::Get());

    // init hooking setup
    A64HookInit();

    skyline::logger::setup_socket_hooks();

    // initialize logger
    nn::fs::MountSdCardForDebug("sd");
    skyline::logger::s_Instance = new skyline::logger::TcpLogger();
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\\n");

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, 
                                    sizeof(exception_handler_stack), &exception_info);

    // hook to prevent the game from double mounting romfs
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), 
                   reinterpret_cast<void*>(handleNnFsMountRom),
                   (void**)&nnFsMountRomImpl);

    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), 
                   reinterpret_cast<void*>(nn_ro_init), 
                   (void**)&nnRoInitializeImpl);
}
```

3

[](https://mintlify.wiki/skyline-dev/skyline/core/architecture#)

Hook System Initialization

`A64HookInit()` allocates JIT memory regions for storing trampolines and inline hook handlers.Two separate JIT regions are created:*  **Normal Hook JIT** : Stores trampolines for `A64HookFunction` calls

* **Inline Hook JIT** : Located near the main text region for `A64InlineHook` calls

See [Hooking System](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system) for details.

4

[](https://mintlify.wiki/skyline-dev/skyline/core/architecture#)

RomFS Mount Hook

The framework hooks `nn::fs::MountRom` to detect when the game mounts its RomFS.**source/main.cpp:48-64**

```
Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = 0;
    rc = nnFsMountRomImpl(path, buffer, size);

    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    // Some games call this method multiple times, ensure we only initialize once
    g_MountRomInit.call_once([]() {
        // start task queue
        skyline::utils::SafeTaskQueue* taskQueue =
            new skyline::utils::SafeTaskQueue(100);
        taskQueue->startThread(20, 3, 0x10000);
        taskQueue->push(new std::unique_ptr<skyline::utils::Task>(after_romfs_task));
        nn::os::WaitEvent(&after_romfs_task->completionEvent);
    });

    return rc;
}
```

This ensures plugins are loaded only after the game’s RomFS is available.

5

[](https://mintlify.wiki/skyline-dev/skyline/core/architecture#)

Plugin Loading

Once RomFS is mounted, the `after_romfs_task` executes:**source/main.cpp:26-41**

```
static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    const size_t poolSize = 0x600000;
    void* socketPool = memalign(0x4000, poolSize);
    nn::socket::Initialize(socketPool, poolSize, 0x20000, 14);

    skyline::logger::s_Instance->StartThread();

    // load plugins
    auto manager = new skyline::plugin::Manager();
    manager->LoadPluginsImpl();
}};
```

This initializes networking (for TCP logger) and loads all plugins from `romfs:/skyline/plugins`.

## Memory Layout

Skyline discovers and tracks several key memory regions during initialization:| Region        | Purpose                    | Address Variable       |
| --------------- | ---------------------------- | ------------------------ |
| `.text`   | Game’s executable code    | `g_MainTextAddr`   |
| `.rodata` | Read-only data segment     | `g_MainRodataAddr` |
| `.data`   | Initialized data segment   | `g_MainDataAddr`   |
| `.bss`    | Uninitialized data segment | `g_MainBssAddr`    |
| `heap`    | Dynamic memory allocation  | `g_MainHeapAddr`   |

These addresses are logged during initialization and are accessible via the `skyline::utils` namespace throughout the framework.

## Process Handle Management

Skyline maintains its own process handle for memory operations:

```
envSetOwnProcessHandle(skyline::proc_handle::Get());
```

This handle is used throughout the framework for:* Memory mapping operations in the hooking system

* Virtual memory allocation
* Process memory queries

## Exception Handling

Skyline installs a custom exception handler to provide detailed crash reports:**source/main.cpp:14-24**

```
void exception_handler(nn::os::UserExceptionInfo* info) {
    skyline::logger::s_Instance->LogFormat("Exception occurred!\\n");

    skyline::logger::s_Instance->LogFormat("Error description: %x\\n", info->ErrorDescription);
    for (int i = 0; i < 29; i++)
        skyline::logger::s_Instance->LogFormat("X[%02i]: %" PRIx64 "\\n", i, info->CpuRegisters[i].x);
    skyline::logger::s_Instance->LogFormat("FP: %" PRIx64 "\\n", info->FP.x);
    skyline::logger::s_Instance->LogFormat("LR: %" PRIx64 "\\n", info->LR.x);
    skyline::logger::s_Instance->LogFormat("SP: %" PRIx64 "\\n", info->SP.x);
    skyline::logger::s_Instance->LogFormat("PC: %" PRIx64 "\\n", info->PC.x);
}
```

This handler logs all CPU registers and exception information before the game crashes, making debugging significantly easier.## 

Thread Safety

The hooking system uses a mutex to ensure thread-safe hook installation:**source/skyline/inlinehook/And64InlineHook.cpp:522**

```
static nn::os::MutexType hookMutex;
```

Always ensure hooks are installed during initialization or on a single thread. Concurrent hook installation, while protected by a mutex, can still lead to race conditions in the hooked code.

## Component Relationships

This architecture ensures that:1. Core systems are initialized before any hooks are installed

1. Plugins are loaded only after the game’s file system is ready
2. All memory regions are properly set up before dynamic code generation
3. Exception handling is in place before any potentially unsafe operations

## Next Steps

[Hooking SystemLearn how to intercept and modify function calls](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system)

[Plugin DevelopmentCreate your own Skyline plugins](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system)



# Hooking System

ARM64 function hooking and inline patching with A64InlineHook

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline’s hooking system provides two primary methods for intercepting function calls: **function hooks** (`A64HookFunction`) and **inline hooks** (`A64InlineHook`). Both are built on top of the ARM64 instruction rewriting engine from the [And64InlineHook](https://github.com/Rprop/And64InlineHook) library.## 

Hook Types Comparison

## A64HookFunction

**Function-level hooks** that redirect entire function calls and provide trampolines to call the original.* ✅ Can call original function

* ✅ Full parameter access
* ✅ Type-safe with proper casting
* ❌ Cannot access modified registers

## A64InlineHook

**Inline hooks** that execute custom code at any instruction, preserving full CPU state.* ✅ Access all CPU registers

* ✅ Hook mid-function
* ✅ Examine/modify execution state
* ❌ Cannot easily call original code

## A64HookFunction - Function Hooking

### Function Signature

**include/skyline/inlinehook/And64InlineHook.hpp:58**

```
extern "C" void A64HookFunction(void* const symbol, void* const replace, void** result);
```

### Parameters

* **`symbol`** : Pointer to the function you want to hook
* **`replace`** : Pointer to your replacement function
* **`result`** : Output pointer to the trampoline (for calling the original function)

### How It Works

When you call `A64HookFunction`, the following happens:

1

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Lock Hook Mutex

Ensures thread-safe hook installation.**source/skyline/inlinehook/And64InlineHook.cpp:646**

```
nn::os::LockMutex(&hookMutex);
```

2

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Allocate Trampoline

If a trampoline is requested, allocates space in the JIT buffer.**source/skyline/inlinehook/And64InlineHook.cpp:651-652**

```
uint32_t *rxtrampoline = NULL, *rwtrampoline = NULL;
if (result != NULL) {
    FastAllocateTrampoline(&rxtrampoline, &rwtrampoline);
    *result = rxtrampoline;
}
```

The trampoline contains the original instructions that will be overwritten, followed by a jump back to the function.

3

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Calculate Branch Distance

Determines if a direct branch instruction can reach the replacement function.**source/skyline/inlinehook/And64InlineHook.cpp:579**

```
auto pc_offset = static_cast<int64_t>(__intval(replace) - __intval(symbol)) >> 2;
if (llabs(pc_offset) >= (mask >> 1)) {
    // Long form: requires 5 instructions
} else {
    // Short form: single branch instruction
}
```

4

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Overwrite Original Function

Writes the hook code to the original function location.**Short form (1 instruction):**

```
B replace_function  // Direct branch
```

**Long form (4-5 instructions):**

```
NOP              // Optional alignment
LDR X17, #0x8    // Load address into X17
BR X17           // Branch to X17
.quad replace_addr  // 64-bit address
```

5

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Flush Caches

Ensures the CPU sees the new instructions.**source/skyline/inlinehook/And64InlineHook.cpp:605**

```
__flush_cache(symbol, 5 * sizeof(uint32_t));
```

### Example: Hooking nn::fs::MountRom

From `source/main.cpp:120-121`:

```
Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

// Install the hook
A64HookFunction(
    reinterpret_cast<void*>(nn::fs::MountRom),  // Original function
    reinterpret_cast<void*>(handleNnFsMountRom), // Replacement function
    (void**)&nnFsMountRomImpl                    // Trampoline output
);

// Replacement function with same signature
Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    // Call original function via trampoline
    Result rc = nnFsMountRomImpl(path, buffer, size);

    // Add custom logic
    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    // ... initialization code ...

    return rc;
}
```

Always match the function signature exactly when creating hooks. The trampoline will have the same signature as the original function.

### Example: Hooking a Game Function

```
// Original game function we want to hook
void (*originalPlayerUpdate)(Player* player, float deltaTime);

// Our replacement function
void hookedPlayerUpdate(Player* player, float deltaTime) {
    // Custom logic before
    skyline::logger::s_Instance->LogFormat("Player update called!\\n");

    // Call original function
    originalPlayerUpdate(player, deltaTime);

    // Custom logic after
    if (player->health <= 0) {
        skyline::logger::s_Instance->LogFormat("Player died!\\n");
    }
}

// Install the hook (during initialization)
void installHooks() {
    A64HookFunction(
        (void*)0x71001a2c40,  // Address of player update function
        (void*)hookedPlayerUpdate,
        (void**)&originalPlayerUpdate
    );
}
```

## A64InlineHook - Inline Hooks

### Function Signature

**include/skyline/inlinehook/And64InlineHook.hpp:61**

```
extern "C" void A64InlineHook(void* const symbol, void* const replace);
```

### Parameters

* **`symbol`** : Address of the instruction to hook
* **`replace`** : Callback function that receives the CPU context

### Inline Context Structure

**include/skyline/inlinehook/And64InlineHook.hpp:51-55**

```
struct InlineCtx {
    nn::os::CpuRegister registers[31];  // X0-X30
    nn::os::CpuRegister sp;             // Stack pointer
    nn::os::FpuRegister registers_f[32]; // V0-V31 (SIMD/FP registers)
};
```

This structure provides complete access to the CPU state at the moment of the hook.### 

How It Works

Inline hooks are more complex than function hooks:

1

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Allocate Handler Entry

Each inline hook gets an entry in a pre-allocated pool near the game’s code.**source/skyline/inlinehook/And64InlineHook.cpp:507-512**

```
struct PACKED inline_hook_entry {
    std::array<uint8_t, inline_hook_handler_size> handler;
    const void* cur_handler;
    const void* callback;
    const void* trampoline;
};
```

Pool configuration:* Size: 0x1000 entries (4096 hooks maximum)

* Location: Near `.text` region for short branches

2

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Generate Handler

A small assembly handler saves all registers and calls your callback.The handler:1. Saves all 31 general-purpose registers

1. Saves the stack pointer
2. Saves all 32 SIMD/FP registers
3. Calls your callback with pointer to saved state
4. Restores all registers
5. Executes the trampoline (original instruction)

3

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Install Hook

Uses `A64HookFunction` internally to redirect to the handler.**source/skyline/inlinehook/And64InlineHook.cpp:700**

```
void* trampoline;
A64HookFunction(address, &rx.handler, &trampoline);
```

4

[](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system#)

Populate Entry

Stores callback and trampoline addresses in the handler entry.**source/skyline/inlinehook/And64InlineHook.cpp:707-710**

```
memcpy(rw.handler.data(), (void*)handler_start_addr, inline_hook_handler_size);
rw.cur_handler = &inlineHandlerImpl;
rw.callback = callback;
rw.trampoline = trampoline;
```

### Example: Monitoring Register Values

```
// Callback function - receives CPU context
void monitorRegisters(InlineCtx* ctx) {
    // Access any register
    u64 x0_value = ctx->registers[0].x;
    u64 x1_value = ctx->registers[1].x;

    skyline::logger::s_Instance->LogFormat(
        "Hook called! X0=%llx, X1=%llx\\n", 
        x0_value, x1_value
    );

    // Can also modify registers
    if (x0_value == 0x12345) {
        ctx->registers[0].x = 0x99999;  // Change X0 value
    }
}

// Install the inline hook
void installInlineHook() {
    A64InlineHook(
        (void*)0x71001a2c40,  // Address of instruction to hook
        (void*)monitorRegisters
    );
}
```

### Example: Conditional Execution

```
void conditionalPatch(InlineCtx* ctx) {
    // Check if we're in a specific game state
    u64 gameMode = ctx->registers[5].x;

    if (gameMode == 3) {
        // In multiplayer mode - skip damage calculation
        ctx->registers[0].x = 0;  // Set damage to zero
    }
}

// Hook the damage calculation function
A64InlineHook(
    (void*)getDamageCalculationAddress(),
    (void*)conditionalPatch
);
```

Inline hooks have significant performance overhead because they save and restore all CPU registers. Use them sparingly and only when you need full register access.

## Instruction Relocation

Both hook types use sophisticated instruction relocation to handle PC-relative instructions:### 

Supported Relocations

1. **Branch Instructions** (`B`, `BL`)
   * Recalculates offsets for the new location
   * Converts to absolute jumps if out of range
2. **Conditional Branches** (`B.cond`, `CBZ`, `CBNZ`, `TBZ`, `TBNZ`)
   * Adjusts PC-relative offsets
   * Expands to multi-instruction sequences if needed
3. **Load Literal** (`LDR`, `LDRSW`)
   * Relocates PC-relative loads
   * Embeds literal values in trampoline if necessary
4. **PC-Relative Addressing** (`ADR`, `ADRP`)
   * Recalculates addresses relative to new location
   * Converts to absolute loads for large offsets

### Example Relocation

Original code:

```
0x71001a2c40:  B.EQ  0x71001a2c60  // PC-relative branch
0x71001a2c44:  LDR   X0, [PC, #24] // PC-relative load
```

In trampoline:

```
0x72005000:  B.EQ  0x71001a2c60  // Offset recalculated
0x72005004:  LDR   X0, #12       // Load from embedded literal
0x72005008:  B     0x71001a2c48  // Return to original code
0x72005010:  .quad <original_literal>  // Embedded value
```

## Memory Management

Hooks use the **ControlledPages** system to safely write to executable memory:**source/skyline/inlinehook/And64InlineHook.cpp:581-582**

```
skyline::inlinehook::ControlledPages control(original, 5 * sizeof(uint32_t));
control.claim();
```

See [Memory Management](https://mintlify.wiki/skyline-dev/skyline/core/memory-management#controlled-pages-system) for details on how this works.## 

JIT Memory Allocation

During initialization, two JIT regions are created:### 

Normal Hook JIT

**source/skyline/inlinehook/And64InlineHook.cpp:530-534**

```
Result rc = jitCreate(&__insns_jit, NULL, sizeof(insns_t));
memset(__insns_jit.rw_addr, 0, __insns_jit.size);
rc = jitTransitionToExecutable(&__insns_jit);
```

* Stores trampolines for `A64HookFunction` calls
* Capacity: 2048 trampolines
* Each trampoline: up to 50 instructions

### Inline Hook JIT

**source/skyline/inlinehook/And64InlineHook.cpp:536-553**

```
// Search for space near .text region
auto cur_searching_addr = skyline::utils::g_MainTextAddr - inline_hook_pool_size;

MemoryInfo mem;
while (true) {
    u32 page_info;
    if (R_SUCCEEDED(svcQueryMemory(&mem, &page_info, cur_searching_addr)) &&
        mem.type == MemType_Unmapped &&
        mem.size >= ALIGN_UP(inline_hook_pool_size, PAGE_SIZE)) {
        break;
    }
    cur_searching_addr -= PAGE_SIZE;
}

// Allocate near .text for short branches
rc = jitCreate(&__inline_hook_jit, 
              (void*)ALIGN_DOWN(mem.addr + mem.size - inline_hook_pool_size, PAGE_SIZE),
              inline_hook_pool_size);
```

* Stores handlers for `A64InlineHook` calls
* Capacity: 4096 hooks
* Located near game code for efficient branching

The inline hook JIT is placed near the game’s `.text` section to ensure branch instructions can reach it without needing long-form jumps.

## Best Practices

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="when-to-use-a64hookfunction accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="when-to-use-a64hookfunction" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">When to use A64HookFunction</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="when-to-use-a64inlinehook accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="when-to-use-a64inlinehook" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">When to use A64InlineHook</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="hook-installation-timing accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="hook-installation-timing" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Hook Installation Timing</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="error-handling accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="error-handling" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Error Handling</p></div></summary>

```

```

</details>

## Limitations

**Hook Capacity Limits:*** Maximum function hooks: 2048

* Maximum inline hooks: 4096
* Exceeding these limits will cause runtime errors

**Branch Range:** If a hook target is too far from the JIT region, hooks will use the longer 5-instruction form, which may not fit in small functions.

**Thread Safety:** While hook installation is thread-safe, the hooked functions themselves must handle concurrency appropriately.

## Next Steps

[Memory ManagementUnderstand controlled pages and JIT compilation](https://mintlify.wiki/skyline-dev/skyline/core/memory-management)

[Plugin SystemLearn how to create plugins that use hooks](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system)



# Plugin System

Dynamic loading and management of NRO plugins in Skyline

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline’s plugin system allows you to extend game functionality by loading custom NRO (Nintendo Relocatable Object) modules at runtime. Plugins are dynamically loaded from the game’s RomFS and can use the full Skyline API, including hooks, logging, and memory utilities.## 

Overview

The plugin system is managed by the `PluginManager` class, which handles:* Discovery of plugin files in RomFS

* NRR (Nintendo Relocatable Range) registration for security
* Loading NRO modules with proper memory setup
* Symbol resolution and linking
* Plugin entrypoint execution

## Plugin Location

**include/skyline/plugin/PluginManager.hpp:13**

```
static constexpr auto PLUGIN_PATH = "skyline/plugins";
```

Plugins must be placed in the game’s RomFS at:

```
romfs:/skyline/plugins/
```

For example, if your game mounts RomFS as `rom:/`, plugins should be at:

```
rom:/skyline/plugins/my_plugin.nro
```

The plugin directory is scanned recursively, so you can organize plugins in subdirectories:

```
rom:/skyline/plugins/combat/damage_modifier.nro
rom:/skyline/plugins/graphics/fps_unlocker.nro
```

## Plugin Structure

Each plugin is tracked with the following information:**include/skyline/plugin/PluginManager.hpp:15-23**

```
struct PluginInfo {
    std::string Path;
    std::unique_ptr<u8> Data;
    size_t Size;
    utils::Sha256Hash Hash;
    nn::ro::Module Module;
    std::unique_ptr<u8> BssData;
    size_t BssSize;
};
```

* **Path** : Full path to the plugin file in RomFS
* **Data** : Plugin file contents (NRO executable)
* **Size** : Size of the NRO file
* **Hash** : SHA256 hash for NRR registration
* **Module** : `nn::ro` module handle after loading
* **BssData** : Allocated memory for uninitialized data (.bss section)
* **BssSize** : Size of .bss section

## Loading Process

The plugin loading process happens in several stages after RomFS is mounted:

1

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Initialize nn::ro

The dynamic module loader must be initialized first.**source/skyline/plugin/PluginManager.cpp:18**

```
nn::ro::Initialize();
```

This prepares the Nintendo SDK’s runtime loader for loading NRO files.

2

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Discover Plugins

Recursively walk the plugin directory to find all NRO files.**source/skyline/plugin/PluginManager.cpp:21-25**

```
skyline::utils::walkDirectory(
    utils::g_RomMountStr + PLUGIN_PATH,
    [this](nn::fs::DirectoryEntry const& entry, std::shared_ptr<std::string> path) {
        if (entry.type == nn::fs::DirectoryEntryType_File)
            m_pluginInfos.push_back(PluginInfo{.Path = *path});
    }
);
```

Each file found is added to the plugin list for processing.

3

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Read and Validate Plugins

Each plugin file is read and validated as a proper NRO.**source/skyline/plugin/PluginManager.cpp:42-93**

```
// Open file
nn::fs::FileHandle handle;
rc = nn::fs::OpenFile(&handle, plugin.Path.c_str(), nn::fs::OpenMode_Read);

// Get size
s64 fileSize;
rc = nn::fs::GetFileSize(&fileSize, handle);
nn::fs::CloseFile(handle);

// Allocate and read
plugin.Size = fileSize;
plugin.Data = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.Size));
rc = skyline::utils::readFile(plugin.Path, 0, plugin.Data.get(), plugin.Size);

// Get BSS size requirement
rc = nn::ro::GetBufferSize(&plugin.BssSize, plugin.Data.get());
```

Invalid files are removed from the plugin list.

4

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Calculate Hashes

SHA256 hashes are computed for NRR registration.**source/skyline/plugin/PluginManager.cpp:96-107**

```
nn::ro::NroHeader* nroHeader = (nn::ro::NroHeader*)plugin.Data.get();
nn::crypto::GenerateSha256Hash(
    &plugin.Hash, 
    sizeof(utils::Sha256Hash), 
    nroHeader, 
    nroHeader->size
);

// Check for duplicates
if (sortedHashes.find(plugin.Hash) != sortedHashes.end()) {
    // Skip duplicate
}
sortedHashes.insert(plugin.Hash);
```

Duplicate plugins (same hash) are automatically skipped.

5

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Build and Register NRR

Create an NRR (Nintendo Relocatable Range) buffer to register all plugin hashes.**source/skyline/plugin/PluginManager.cpp:115-154**

```
// Allocate NRR buffer
m_nrrSize = ALIGN_UP(
    sizeof(nn::ro::NrrHeader) + (m_pluginInfos.size() * sizeof(utils::Sha256Hash)), 
    0x1000
);
m_nrrBuffer = std::unique_ptr<u8>((u8*)memalign(0x1000, m_nrrSize));

// Get program ID
u64 program_id = get_program_id();

// Initialize NRR header
auto nrrHeader = reinterpret_cast<nn::ro::NrrHeader*>(m_nrrBuffer.get());
*nrrHeader = nn::ro::NrrHeader{
    .magic = 0x3052524E,  // "NRR0"
    .program_id = {program_id},
    .size = (u32)m_nrrSize,
    .type = 0,  // ForSelf
    .hashes_offset = sizeof(nn::ro::NrrHeader),
    .num_hashes = (u32)m_pluginInfos.size(),
};

// Copy hashes into NRR (must be sorted)
utils::Sha256Hash* hashes =
    reinterpret_cast<utils::Sha256Hash*>((size_t)m_nrrBuffer.get() + nrrHeader->hashes_offset);
auto curHashIdx = 0;
for (auto hash : sortedHashes) {
    hashes[curHashIdx++] = hash;
}

// Register with nn::ro
rc = nn::ro::RegisterModuleInfo(&m_registrationInfo, m_nrrBuffer.get());
```

The NRR system is a security feature that requires all dynamically loaded modules to be pre-registered with their hashes. Hashes must be sorted for registration to succeed.

6

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Load NRO Modules

With the NRR registered, load each plugin module.**source/skyline/plugin/PluginManager.cpp:163-181**

```
// Allocate BSS memory
plugin.BssData = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.BssSize));

// Load module
rc = nn::ro::LoadModule(
    &plugin.Module,           // Output module handle
    plugin.Data.get(),        // NRO data
    plugin.BssData.get(),     // BSS buffer
    plugin.BssSize,           // BSS size
    nn::ro::BindFlag_Now      // Bind immediately
);
```

`BindFlag_Now` ensures all symbols are resolved immediately, so the plugin is fully ready to use.

7

[](https://mintlify.wiki/skyline-dev/skyline/core/plugin-system#)

Execute Plugin Entrypoints

Call the `main` function of each loaded plugin.**source/skyline/plugin/PluginManager.cpp:192-203**

```
// Lookup entrypoint
void (*pluginEntrypoint)() = NULL;
rc = nn::ro::LookupModuleSymbol(
    reinterpret_cast<uintptr_t*>(&pluginEntrypoint), 
    &plugin.Module, 
    "main"
);

// Execute if found
if (pluginEntrypoint != NULL && R_SUCCEEDED(rc)) {
    pluginEntrypoint();
}
```

Each plugin’s `main()` function is its initialization routine.

## Creating a Plugin

### Basic Plugin Template

```
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/Logger.hpp"

extern "C" void main() {
    skyline::logger::s_Instance->Log("[MyPlugin] Loading...\\n");

    // Install hooks
    A64HookFunction(
        (void*)0x71001a2c40,
        (void*)myHookFunction,
        (void**)&originalFunction
    );

    skyline::logger::s_Instance->Log("[MyPlugin] Loaded successfully!\\n");
}
```

The `main()` function must be declared with `extern "C"` linkage so the plugin manager can find it by name.

### Building a Plugin

Your plugin must be compiled as an NRO file. Example Makefile snippet:

```
TARGET := my_plugin
BUILD := build
SOURCES := source

CXXFLAGS := -fPIE -fno-rtti -fno-exceptions -std=gnu++20
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -Wl,-Map,$(notdir $*.map)

# Build as NRO
$(TARGET).nro: $(TARGET).elf
	@elf2nro $< $@

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@
```

Key requirements:* Position-independent code (`-fPIE`)

* Compiled for Nintendo Switch AArch64
* Linked with proper SDK libraries
* Converted to NRO format with `elf2nro`

### Plugin Lifecycle

## Advanced Plugin Features

### Symbol Lookup

Plugins can resolve symbols from other loaded modules:

```
uintptr_t functionAddress;
Result rc = nn::ro::LookupSymbol(&functionAddress, "someGameFunction");

if (R_SUCCEEDED(rc)) {
    // Use the resolved address
    void (*func)() = (void(*)())functionAddress;
    func();
}
```

### Module Symbol Lookup

Look up symbols within a specific plugin:

```
uintptr_t exportedFunction;
rc = nn::ro::LookupModuleSymbol(
    &exportedFunction,
    &plugin.Module,
    "myExportedFunction"
);
```

### Getting Plugin Info at Runtime

The plugin manager can identify which plugin an address belongs to:**include/skyline/plugin/PluginManager.hpp:42**

```
static inline const PluginInfo* GetContainingPlugin(const void* addr);
```

Implementation from `source/skyline/plugin/PluginManager.cpp:209-220`:

```
const PluginInfo* Manager::GetContainingPluginImpl(const void* addr) {
    const PluginInfo* ret = nullptr;
    for (auto& plugin : m_pluginInfos) {
        void* module_start = (void*)plugin.Module.ModuleObject->module_base;
        void* module_end = module_start + plugin.Size;
        if (module_start < addr && addr < module_end) {
            ret = &plugin;
            break;
        }
    }
    return ret;
}
```

This is useful for debugging or implementing plugin-specific behavior.### 

C API for Address Ranges

**include/skyline/plugin/PluginManager.hpp:51-53**

```
extern "C" {
    void get_plugin_addresses(const void* internal_addr, void** start, void** end);
}
```

**source/skyline/plugin/PluginManager.cpp:225-233**

```
void get_plugin_addresses(const void* internal_addr, void** start, void** end) {
    auto info = skyline::plugin::Manager::GetContainingPlugin(internal_addr);
    if (info == nullptr)
        *start = *end = nullptr;
    else {
        *start = (void*)info->Module.ModuleObject->module_base;
        *end = *start + info->Size;
    }
}
```

## NRO File Format

Plugins use the NRO (Nintendo Relocatable Object) format:**include/nn/ro.h:31-50**

```
struct NroHeader {
    u32 entrypoint_insn;
    u32 mod_offset;
    u8 _x8[0x8];
    u32 magic;              // "NRO0"
    u8 _x14[0x4];
    u32 size;               // Total size
    u8 reserved_1C[0x4];
    u32 text_offset;        // .text section
    u32 text_size;
    u32 ro_offset;          // .rodata section
    u32 ro_size;
    u32 rw_offset;          // .data section
    u32 rw_size;
    u32 bss_size;           // .bss section
    u8 _x3C[0x4];
    ModuleId module_id;     // Build ID
    u8 _x60[0x20];
};
```

### Memory Layout

When an NRO is loaded, it’s mapped into memory:

```
[Base Address]
  ├── .text (executable code)
  ├── .rodata (read-only data)
  ├── .data (initialized data)
  └── .bss (uninitialized data) <- separate allocation
```

The `.bss` section is allocated separately because it’s not stored in the file.## 

NRR Security System

The NRR (Nintendo Relocatable Range) system ensures only authorized modules can be loaded:**include/nn/ro.h:58-75**

```
struct NrrHeader {
    u32 magic;              // "NRR0"
    u8 _x4[0xC];
    u64 program_id_mask;
    u64 program_id_pattern;
    u8 _x20[0x10];
    u8 modulus[0x100];
    u8 fixed_key_signature[0x100];
    u8 nrr_signature[0x100];
    ProgramId program_id;   // Current program ID
    u32 size;               // NRR buffer size
    u8 type;                // 0 = ForSelf
    u8 _x33D[3];
    u32 hashes_offset;      // Offset to hash array
    u32 num_hashes;         // Number of hashes
    u8 _x348[8];
};
```

**Critical Requirements for NRR:*** Buffer must be page-aligned (0x1000 bytes)

* Size must be page-aligned
* Hashes must be sorted in ascending order
* Program ID must match current game
* Type must be 0 (ForSelf) for user modules

## Error Handling

The plugin manager gracefully handles errors at each stage:

```
if (R_FAILED(rc)) {
    skyline::logger::s_Instance->LogFormat(
        "[PluginManager] Failed to load '%s' (0x%x). Skipping.",
        plugin.Path.c_str(), rc
    );
    pluginInfoIter = m_pluginInfos.erase(pluginInfoIter);
    continue;
}
```

Plugins that fail to load are removed from the list, and loading continues.### 

Common Errors

| Error                         | Cause                         | Solution                         |
| ----------------------------- | ----------------------------- | -------------------------------- |
| Failed to open file           | File not found or permissions | Check plugin path in RomFS       |
| Failed to get NRO buffer size | Invalid NRO format            | Verify file is proper NRO        |
| Failed to register NRR        | Hash mismatch or unsorted     | Rebuild NRR with correct hashes  |
| Failed to load module         | Missing dependencies          | Ensure all symbols are available |
| Failed to lookup symbol       | No `main` function          | Add `extern "C" void main()`   |

## Best Practices

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="plugin-organization accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="plugin-organization" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Plugin Organization</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="initialization accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="initialization" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Initialization</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="memory-management accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="memory-management" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Memory Management</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="compatibility accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="compatibility" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Compatibility</p></div></summary>

</details>

## Debugging Plugins

### Logging

```
skyline::logger::s_Instance->Log("Simple message\\n");
skyline::logger::s_Instance->LogFormat(
    "Value: %d, Address: %p\\n", 
    value, 
    address
);
```

### Getting Plugin Module Info

```
auto info = skyline::plugin::Manager::GetContainingPlugin((void*)&myFunction);
if (info) {
    skyline::logger::s_Instance->LogFormat(
        "Function is in plugin: %s\\n",
        info->Path.c_str()
    );
}
```

### Verifying Load Status

Check the Skyline log for messages like:

```
[PluginManager] Read rom:/skyline/plugins/my_plugin.nro
[PluginManager] Loaded 'rom:/skyline/plugins/my_plugin.nro'
[PluginManager] Running `main` for rom:/skyline/plugins/my_plugin.nro
[MyPlugin] Loading...
[MyPlugin] Loaded successfully!
[PluginManager] Finished running `main` for 'rom:/skyline/plugins/my_plugin.nro' (0x0)
```

## Next Steps

[Hooking SystemLearn how to hook functions from your plugin](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system)

[Memory ManagementUnderstand memory regions and allocation](https://mintlify.wiki/skyline-dev/skyline/core/memory-management)


# Memory Management

Virtual memory setup, JIT compilation, and controlled page management

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

Skyline’s memory management system provides the foundation for dynamic code generation, safe memory patching, and plugin execution. It consists of three main components: virtual memory allocation, JIT (Just-In-Time) compilation support, and the controlled pages system for safe executable memory modification.## 

Virtual Memory System

The virtual memory system provides address space management using the `virtmem` API, which wraps Nintendo Switch kernel services.### 

API Overview

**include/skyline/nx/kernel/virtmem.h:19-42**

```
void* virtmemReserve(size_t size);
void virtmemFree(void* addr, size_t size);
void* virtmemReserveStack(size_t size);
void virtmemFreeStack(void* addr, size_t size);
void virtmemSetup();
```

### Initialization

Virtual memory must be set up before any JIT operations:**source/main.cpp:144**

```
virtmemSetup();  // needed for libnx JIT
```

This function initializes the virtual memory subsystem by discovering address space regions.### 

Memory Region Types

The Switch’s address space is divided into several regions:**source/skyline/inlinehook/controlledpages.cpp:26-36**

```
struct AddressSpaceInfo {
    uintptr_t heap_base;
    size_t heap_size;
    uintptr_t heap_end;
    uintptr_t alias_base;
    size_t alias_size;
    uintptr_t alias_end;
    uintptr_t aslr_base;
    size_t aslr_size;
    uintptr_t aslr_end;
};
```

## Heap Region

Dynamic memory allocations (malloc, new)

## Alias Region

Stack and aliased memory mappings

## ASLR Region

Main executable and loaded modules

### Address Space Layout

On 64-bit Switch applications:**source/skyline/inlinehook/controlledpages.cpp:42-43**

```
static constexpr uintptr_t AslrBase64Bit = 0x0008000000ul;
static constexpr size_t AslrSize64Bit = 0x7FF8000000ul;
```

```
0x0000000000 - 0x0008000000: Reserved
0x0008000000 - 0x8000000000: ASLR Region (main executable, modules)
   ├── Game .text (executable code)
   ├── Game .rodata (constants)
   ├── Game .data (global variables)
   ├── Skyline framework
   └── Loaded plugins
0x8000000000+: Heap and Alias regions
```

### Finding Mappable Space

Before mapping memory, Skyline searches for available address space:**source/skyline/inlinehook/controlledpages.cpp:64-105**

```
static Result locateMappableSpaceModern(uintptr_t* out_address, size_t size) {
    MemoryInfo mem_info = {};
    u32 page_info = 0;
    uintptr_t cur_base = 0, cur_end = 0;

    AddressSpaceInfo address_space;
    R_TRY(getProcessAddressSpaceInfo(&address_space, CUR_PROCESS_HANDLE));
    cur_base = address_space.aslr_base;
    cur_end = cur_base + size;

    while (true) {
        if (address_space.heap_size &&
            (address_space.heap_base <= cur_end - 1 && cur_base <= address_space.heap_end - 1)) {
            // Skip heap region
            cur_base = address_space.heap_end;
        } else if (address_space.alias_size &&
                   (address_space.alias_base <= cur_end - 1 && cur_base <= address_space.alias_end - 1)) {
            // Skip alias region
            cur_base = address_space.alias_end;
        } else {
            R_ERRORONFAIL(svcQueryMemory(&mem_info, &page_info, cur_base));
            if (mem_info.type == 0 && mem_info.addr - cur_base + mem_info.size >= size) {
                *out_address = cur_base;
                return 0;
            }
            cur_base = mem_info.addr + mem_info.size;
        }
        cur_end = cur_base + size;
    }
}
```

This algorithm:1. Starts searching from the ASLR base

1. Skips heap and alias regions
2. Queries memory info at each candidate address
3. Returns the first unmapped region with sufficient space

## JIT Compilation Support

The JIT system provides executable memory regions for dynamically generated code.### 

JIT Object Structure

**include/skyline/nx/kernel/jit.h:18-26**

```
typedef struct {
    JitType type;
    size_t size;
    void* src_addr;
    void* rx_addr;  // Read-execute alias
    void* rw_addr;  // Read-write alias
    bool is_executable;
    Handle handle;
} Jit;
```

Each JIT buffer has two views of the same memory:*  **rx_addr** : Read-execute alias for running code

* **rw_addr** : Read-write alias for modifying code

This dual-mapping approach allows code to be written through `rw_addr` and executed through `rx_addr`, satisfying W^X (Write XOR Execute) security requirements.

### JIT Types

**include/skyline/nx/kernel/jit.h:11-15**

```
typedef enum {
    JitType_CodeMemory,  // Using svcSetProcessMemoryPermission
    JitType_JitMemory,   // Using code-memory syscalls (4.0.0+)
} JitType;
```

Skyline uses `JitType_CodeMemory` on modern firmware versions.### 

JIT API

Create JIT Buffer

Transition to Writable

Transition to Executable

Destroy JIT Buffer

```
Result jitCreate(Jit* j, void* src_addr, size_t size);
```

### Usage Example from Hook System

**source/skyline/inlinehook/And64InlineHook.cpp:530-534**

```
// Allocate JIT buffer
Result rc = jitCreate(&__insns_jit, NULL, sizeof(insns_t));
R_ERRORONFAIL(rc);

// Clear contents
memset(__insns_jit.rw_addr, 0, __insns_jit.size);

// Make executable
rc = jitTransitionToExecutable(&__insns_jit);
R_ERRORONFAIL(rc);
```

### JIT Memory Transitions

JIT buffers can be toggled between writable and executable:**source/skyline/inlinehook/And64InlineHook.cpp:648**

```
// Make writable for modifications
R_ERRORONFAIL(jitTransitionToWritable(&__insns_jit));

// Write code...
uint32_t* code = (uint32_t*)__insns_jit.rw_addr;
code[0] = 0xd503201f;  // NOP instruction

// Make executable for running
R_ERRORONFAIL(jitTransitionToExecutable(&__insns_jit));
```

Always transition JIT buffers to executable before executing code from them. Attempting to execute from writable memory will crash.

## Controlled Pages System

The controlled pages system provides safe, temporary read-write access to executable memory for patching.### 

Class Definition

**include/skyline/inlinehook/controlledpages.hpp:7-20**

```
namespace skyline::inlinehook {
class ControlledPages {
   private:
    bool isClaimed;
    size_t size;

   public:
    void* rx;  // Original read-execute address
    void* rw;  // Temporary read-write mapping

    ControlledPages(void* addr, size_t size);
    void claim();
    void unclaim();
};
};
```

### How It Works

1

[](https://mintlify.wiki/skyline-dev/skyline/core/memory-management#)

Create ControlledPages Object

```
skyline::inlinehook::ControlledPages control(
    original_function,     // Address to patch
    5 * sizeof(uint32_t)   // Size to patch
);
```

Stores the target address and size for later mapping.

2

[](https://mintlify.wiki/skyline-dev/skyline/core/memory-management#)

Claim - Create RW Mapping

**source/skyline/inlinehook/controlledpages.cpp:117-139**

```
void ControlledPages::claim() {
    if (!isClaimed) {
        // Align to page boundaries
        u64 alignedSrc = ALIGN_DOWN((u64)rx, PAGE_SIZE);
        size_t alignedSize = ALIGN_UP(size, PAGE_SIZE);

        // Find free address space
        u64 dst;
        R_ERRORONFAIL(locateMappableSpace(&dst, alignedSize));

        // Map pages as read-write
        R_ERRORONFAIL(svcMapProcessMemory(
            (void*)dst, 
            skyline::proc_handle::Get(), 
            alignedSrc, 
            alignedSize
        ));

        // Calculate RW pointer
        rw = (void*)(dst + ((s64)rx - alignedSrc));
        isClaimed = true;

        // Sanity check
        if (*(u64*)rx != *(u64*)rw) {
            R_ERRORONFAIL(-1);
        }
    }
}
```

This creates a writable mirror of the executable page at a new address.

3

[](https://mintlify.wiki/skyline-dev/skyline/core/memory-management#)

Modify Memory

```
control.claim();

// Now we can write to executable memory
uint32_t* writable = (uint32_t*)control.rw;
writable[0] = 0x58000051;  // LDR X17, #0x8
writable[1] = 0xd61f0220;  // BR X17
memcpy(writable + 2, &target_addr, sizeof(target_addr));
```

4

[](https://mintlify.wiki/skyline-dev/skyline/core/memory-management#)

Unclaim - Flush and Unmap

**source/skyline/inlinehook/controlledpages.cpp:141-159**

```
void ControlledPages::unclaim() {
    if (isClaimed) {
        u64 alignedSrc = ALIGN_DOWN((u64)rx, PAGE_SIZE);
        void* alignedDst = (void*)ALIGN_DOWN((u64)rw, PAGE_SIZE);
        size_t alignedSize = ALIGN_UP(size, PAGE_SIZE);

        // Flush data cache (write changes to memory)
        armDCacheFlush((void*)alignedDst, size);

        // Invalidate instruction cache (ensure CPU sees changes)
        armICacheInvalidate((void*)alignedSrc, size);

        // Unmap the writable alias
        R_ERRORONFAIL(svcUnmapProcessMemory(
            alignedDst, 
            skyline::proc_handle::Get(), 
            alignedSrc, 
            alignedSize
        ));

        rw = NULL;
        isClaimed = false;
    }
}
```

Critical steps:1.  **Data cache flush** : Writes changes from cache to RAM

1. **Instruction cache invalidate** : Clears old instructions from CPU
2. **Unmap** : Removes the writable alias

### Why Cache Management Matters

Modern CPUs have separate instruction and data caches:

```
CPU Core
├── Instruction Cache (I-Cache)
│   └── Caches instructions for execution
└── Data Cache (D-Cache)
    └── Caches data for reads/writes
```

When you write new instructions:1. Writes go to D-Cache first

1. `armDCacheFlush` forces D-Cache to write to RAM
2. I-Cache may still contain old instructions
3. `armICacheInvalidate` clears I-Cache
4. CPU re-fetches instructions from RAM

**Forgetting cache operations will cause:*** CPU executing old instructions (I-Cache not invalidated)

* Changes not visible in memory (D-Cache not flushed)
* Seemingly random behavior

### Usage in Hook Installation

From `source/skyline/inlinehook/And64InlineHook.cpp:581-611`:

```
void* A64HookFunctionV(void* const symbol, void* const replace, ...) {
    uint32_t* original = static_cast<uint32_t*>(symbol);

    // Determine how many instructions we need to overwrite
    int32_t count = (reinterpret_cast<uint64_t>(original + 2) & 7u) != 0u ? 5 : 4;

    // Create controlled pages for safe writing
    skyline::inlinehook::ControlledPages control(original, count * sizeof(uint32_t));
    control.claim();

    // Get writable pointer
    original = (u32*)control.rw;

    // Build trampoline if requested
    if (rxtrampoline) {
        __fix_instructions(original, (u32*)control.rx, count, rwtrampoline, rxtrampoline);
    }

    // Write hook code
    if (count == 5) {
        original[0] = A64_NOP;
        ++original;
    }
    original[0] = 0x58000051u;  // LDR X17, #0x8
    original[1] = 0xd61f0220u;  // BR X17
    *reinterpret_cast<int64_t*>(original + 2) = __intval(replace);

    // Flush caches (done in unclaim)
    control.unclaim();

    return rxtrampoline;
}
```

## Memory Allocation for Plugins

Plugins require properly aligned memory for their sections:### 

NRO Data Section

**source/skyline/plugin/PluginManager.cpp:68**

```
plugin.Data = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.Size));
```

* Aligned to 0x1000 (4KB page boundary)
* Stores the entire NRO file

### BSS Section

**source/skyline/plugin/PluginManager.cpp:163**

```
plugin.BssData = std::unique_ptr<u8>((u8*)memalign(0x1000, plugin.BssSize));
```

* Separate allocation for uninitialized data
* Also page-aligned for memory protection

Page alignment (0x1000 bytes) is required because:* Memory protection works at page granularity

* `svcMapProcessMemory` requires page-aligned addresses
* Nintendo’s `nn::ro` loader expects aligned sections

## Memory Protection and Permissions

The Switch kernel enforces memory permissions:| Permission | Symbol | Description         |
| ------------ | -------- | --------------------- |
| Read       | R      | Can read memory     |
| Write      | W      | Can write memory    |
| Execute    | X      | Can execute as code |

**W^X Policy (Write XOR Execute):** The kernel generally prevents pages from being both writable and executable simultaneously. This is why we need:* JIT buffers with separate RW/RX mappings

* Controlled pages for temporary RW access
* Cache flushes after writing code

### Typical Memory Permissions

```
Game .text section:     R-X  (read and execute, not writable)
Game .rodata section:   R--  (read-only)
Game .data section:     RW-  (read-write, not executable)
JIT RW mapping:         RW-  (for writing code)
JIT RX mapping:         R-X  (for executing code)
Controlled RW mapping:  RW-  (temporary for patching)
```

## Best Practices

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="virtual-memory-allocation accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="virtual-memory-allocation" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Virtual Memory Allocation</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="jit-buffer-management accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="jit-buffer-management" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">JIT Buffer Management</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="controlled-pages-usage accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="controlled-pages-usage" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Controlled Pages Usage</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="cache-coherency accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="cache-coherency" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Cache Coherency</p></div></summary>

```

```

</details>

Debugging Memory Issues

Common Symptoms and Causes

| Symptom                    | Likely Cause                      | Solution                              |
| -------------------------- | --------------------------------- | ------------------------------------- |
| Crash on code modification | Writing to RX memory              | Use controlled pages                  |
| Old code still executing   | Instruction cache not invalidated | Call `armICacheInvalidate`          |
| Changes not visible        | Data cache not flushed            | Call `armDCacheFlush`               |
| JIT code crashes           | Not transitioned to executable    | Call `jitTransitionToExecutable`    |
| Memory mapping fails       | Not page-aligned                  | Use `memalign(0x1000, size)`        |
| Address space exhausted    | Not freeing mappings              | Call `unclaim()` or `virtmemFree` |

### Memory Debugging Tips

```
// Log memory addresses for debugging
skyline::logger::s_Instance->LogFormat(
    "RX addr: %p, RW addr: %p\\n",
    control.rx,
    control.rw
);

// Verify memory contents match
if (memcmp(control.rx, control.rw, size) != 0) {
    skyline::logger::s_Instance->Log("Memory contents don't match!\\n");
}

// Check alignment
if ((uintptr_t)addr & 0xFFF) {
    skyline::logger::s_Instance->Log("Address not page-aligned!\\n");
}
```

## Next Steps

[Hooking SystemSee how hooks use controlled pages](https://mintlify.wiki/skyline-dev/skyline/core/hooking-system)

[ArchitectureUnderstand the overall system design](https://mintlify.wiki/skyline-dev/skyline/core/architecture)



# Getting Started with Plugin Development

Learn how to set up your development environment and create your first Skyline plugin

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Introduction

Skyline plugins are dynamic modules (.nro files) that extend Super Smash Bros Ultimate functionality through runtime hooking and code patching. Plugins are loaded from `atmosphere/contents/<game titleid>/romfs/skyline/plugins/` and executed at runtime.## 

Prerequisites

1

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Install devkitPro

Download and install [devkitPro](https://devkitpro.org/wiki/Getting_Started) with devkitA64 for ARM64 development.

```
# Verify installation
echo $DEVKITPRO
# Should output: /opt/devkitpro (or your installation path)
```

2

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Install Required Tools

Ensure you have the following tools installed:*  **elf2nro** : Converts ELF executables to NRO format (included with devkitPro)

* **Python 3** : Required for build scripts
* **Make** : For building projects

```
# Check tools
which elf2nro
python3 --version
make --version
```

3

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Set Up libnx

libnx should be included with devkitPro. Verify it’s accessible:

```
ls $DEVKITPRO/libnx/include
```

4

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Clone Skyline

You’ll need the Skyline headers to develop plugins:

```
git clone https://github.com/skyline-dev/skyline.git
cd skyline
```

Skyline’s initialization method varies by game. Some games require modified versions:* [Animal Crossing](https://github.com/3096/skyline)

* [Pokémon Sword/Shield](https://github.com/3096/skyline/tree/sword)
* [Persona 5 Royal](https://github.com/Raytwo/p5rcbt)

## Creating Your First Plugin

1

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Set Up Project Structure

Create a new directory for your plugin:

```
mkdir my-first-plugin
cd my-first-plugin
mkdir -p source include
```

2

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Create Plugin Source

Create `source/main.cpp` with a basic plugin:

```
#include <cstdio>

extern "C" void skyline_tcp_send_raw(void*, size_t);

void log(const char* msg) {
    skyline_tcp_send_raw((void*)msg, strlen(msg));
}

extern "C" void main() {
    log("Hello from my first Skyline plugin!\n");
}
```

The `main()` function is the plugin entry point, called automatically by Skyline’s PluginManager after loading.

3

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Create Makefile

Create a `Makefile` to build your plugin:

```
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

TARGET := my_plugin
SOURCES := source
INCLUDES := include

CXXFLAGS := -g -Wall -O2 -ffunction-sections $(ARCH) -D__SWITCH__ \
            -fno-rtti -fomit-frame-pointer -fno-exceptions \
            -std=c++20

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
           -Wl,-Map,$(TARGET).map -Wl,--version-script=exported.txt \
           -shared -nodefaultlibs

LIBS := -lgcc -lstdc++

CFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
OFILES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

all: $(TARGET).nro

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDES) -c $< -o $@

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

$(TARGET).nro: $(TARGET).elf
	elf2nro $< $@
	@echo "Built $(TARGET).nro"

clean:
	@rm -f $(OFILES) $(TARGET).elf $(TARGET).nro $(TARGET).map
```

4

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Create Export File

Create `exported.txt` to specify exported symbols:

```
{
    global:
        main;
    local: *;
};
```

Only export symbols that need to be visible to Skyline or other plugins. Keep most symbols local for better performance.

5

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Build Your Plugin

Compile the plugin:

```
make
```

This will produce `my_plugin.nro` that can be loaded by Skyline.

6

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Deploy to Switch

Copy the .nro file to your Switch:

```
# Replace <titleid> with your game's title ID
# For SSBU: 01006A800016E000
cp my_plugin.nro /path/to/sd/atmosphere/contents/<titleid>/romfs/skyline/plugins/
```

## Testing Your Plugin

1

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Set Up TCP Logging

Skyline uses TCP logging by default. Set up a listener on your PC:

```
# Using netcat
nc -l -p 6969
```

2

[](https://mintlify.wiki/skyline-dev/skyline/plugins/getting-started#)

Launch the Game

Start the game on your Switch. Your plugin’s log message should appear in the TCP listener:

```
[PluginManager] Loading plugins...
[PluginManager] Loaded 'rom:/skyline/plugins/my_plugin.nro'
[PluginManager] Running `main` for rom:/skyline/plugins/my_plugin.nro
Hello from my first Skyline plugin!
[PluginManager] Finished running `main` for 'rom:/skyline/plugins/my_plugin.nro'
```

If your plugin crashes during loading, check Atmosphere’s crash logs at `atmosphere/crash_reports/` on your SD card.

## Next Steps

Now that you have a working plugin, you can:* Learn about [plugin structure and advanced features](https://mintlify.wiki/skyline-dev/skyline/plugins/plugin-structure)

* Explore [real-world plugin examples](https://mintlify.wiki/skyline-dev/skyline/plugins/examples)
* Study the Skyline API headers for available functions

## Troubleshooting

### Plugin Not Loading

* Verify the plugin path is correct: `romfs/skyline/plugins/`
* Check that the file is a valid .nro file: `file my_plugin.nro`
* Review Skyline logs for error messages about your plugin

### Compilation Errors

* Ensure `DEVKITPRO` environment variable is set
* Verify devkitA64 is installed: `ls $DEVKITPRO/devkitA64`
* Check that you’re using C++17 or later

### Runtime Crashes

* Use exception handlers to catch errors (see examples)
* Verify all hooked functions have correct signatures
* Check that you’re not accessing invalid memory addresses



# Plugin Structure

Understanding the anatomy of a Skyline plugin and how to structure your code

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Plugin File Format

Skyline plugins are **NRO (Nintendo Relocatable Object)** files - position-independent executables that can be loaded at runtime. The plugin system uses Nintendo’s `nn::ro` module loader for dynamic loading.### 

File Requirements

* **Format** : ELF64 ARM64 executable converted to NRO
* **Architecture** : ARMv8-A (Cortex-A57)
* **Compilation** : Position-independent code (`-fPIC`)
* **Extension** : `.nro`
* **Location** : `romfs/skyline/plugins/` in the game’s RomFS

## Directory Structure

A typical plugin project structure:

```
my-plugin/
├── source/
│   ├── main.cpp          # Entry point
│   ├── hooks.cpp         # Function hooks
│   └── utils.cpp         # Helper functions
├── include/
│   ├── main.hpp
│   └── types.hpp
├── Makefile              # Build configuration
├── exported.txt          # Symbol export list
└── README.md
```

## Plugin Entry Point

Every plugin must export a `main()` function that serves as the entry point:

```
extern "C" void main() {
    // Plugin initialization code
    // Set up hooks, initialize data structures, etc.
}
```

The `main()` function is called by Skyline’s PluginManager after the plugin is loaded. See `PluginManager.cpp:194` for the loading process.

### Entry Point Details

* Called **once** during plugin initialization
* Must be exported in `exported.txt`
* Executes **synchronously** during game startup
* No return value expected

Avoid blocking operations in `main()`. Long initialization delays game startup. Consider deferring heavy work to game hooks.

## Symbol Exports

Plugins must specify which symbols are visible externally using a version script (`exported.txt`):

```
{
    global:
        main;              # Entry point (required)
        my_hook_function;  # Custom exported function
    local: *;              # Hide all other symbols
};
```

### Skyline Exported Symbols

Skyline exports these symbols for plugin use:

```
// Logging
extern "C" void skyline_tcp_send_raw(void* data, size_t size);

// Hooking
extern "C" void A64HookFunction(void* target, void* replace, void** original);
extern "C" void A64InlineHook(void* target, void* replace);

// Memory utilities
extern "C" void sky_memcpy(void* dst, const void* src, size_t size);

// Process information
extern "C" u64 get_program_id();
extern "C" void get_plugin_addresses(const void* addr, void** start, void** end);

// Region addresses
extern "C" void* getRegionAddress(skyline::utils::region region);
```

These symbols are defined in `exported.txt` of the main Skyline binary.

## Build System

### Makefile Configuration

Plugins require specific compiler and linker flags:#### 

Compiler Flags

```
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

CXXFLAGS := -g -Wall -ffunction-sections $(ARCH) -D__SWITCH__ \
            -fno-rtti -fomit-frame-pointer \
            -fno-exceptions \
            -fno-asynchronous-unwind-tables \
            -fno-unwind-tables
```

**Key flags explained:*** `-march=armv8-a -mtune=cortex-a57`: Target Switch CPU architecture

* `-fPIC`: Generate position-independent code (required for NRO)
* `-ftls-model=local-exec`: Thread-local storage model
* `-fno-rtti`: Disable runtime type information (reduces size)
* `-fno-exceptions`: Disable C++ exceptions (required)

#### Linker Flags

```
LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs \
           -g $(ARCH) \
           -Wl,-Map,$(TARGET).map \
           -Wl,--version-script=exported.txt \
           -shared \
           -nodefaultlibs

LIBS := -lgcc -lstdc++
```

**Key flags explained:*** `-specs=.../switch.specs`: Use Nintendo Switch linker script

* `-Wl,--version-script=exported.txt`: Control symbol visibility
* `-shared`: Create shared object (plugin)
* `-nodefaultlibs`: Don’t link standard libraries (use game’s)

Using `-fexceptions` or linking against standard libraries will cause crashes. Plugins must use the game’s runtime environment.

### Building from Skyline Source

If building alongside Skyline:

```
# Top-level Makefile
CROSSVER ?= 600  # Game version (6.0.0 = 600)

all:
	$(MAKE) all -f nso.mk CROSSVER=$(CROSSVER)
```

The build system supports multiple game versions:

```
make            # Build for version 600 (default)
make CROSSVER=800  # Build for version 800
```

## Linking Against Skyline APIs

### Using Skyline Headers

Include Skyline headers for type definitions and utilities:

```
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/Logger.hpp"
#include "skyline/utils/cpputils.hpp"
```

### Accessing Game Memory Regions

Skyline provides utilities to access game memory:

```
#include "skyline/utils/cpputils.hpp"

// Get region base addresses
void* text_base = getRegionAddress(skyline::utils::region::Text);
void* rodata_base = getRegionAddress(skyline::utils::region::Rodata);
void* data_base = getRegionAddress(skyline::utils::region::Data);
void* bss_base = getRegionAddress(skyline::utils::region::Bss);
void* heap_base = getRegionAddress(skyline::utils::region::Heap);
```

### Using nn:: APIs

Access Nintendo SDK functions through Skyline’s headers:

```
#include "nn/os.hpp"
#include "nn/fs.h"
#include "nn/socket.h"

// Example: File operations
nn::fs::FileHandle handle;
Result rc = nn::fs::OpenFile(&handle, "rom:/data/file.bin", nn::fs::OpenMode_Read);
if (R_SUCCEEDED(rc)) {
    s64 size;
    nn::fs::GetFileSize(&size, handle);
    nn::fs::CloseFile(handle);
}
```

## Plugin Metadata

### NRO Header

NRO files contain a header with module information:

```
struct NroHeader {
    u32 unused;
    u32 module_header_offset;
    u32 size;  // Total NRO size
    // ... additional fields
};
```

Skyline uses this header to:* Calculate SHA256 hash for module registration

* Determine BSS size requirements
* Validate file integrity

### Module Registration

Plugins are registered with `nn::ro` using NRR (Nintendo Relocatable Registry):1. Plugin files are scanned from `romfs/skyline/plugins/`

1. SHA256 hash is calculated for each plugin
2. Hashes are sorted and placed in an NRR
3. NRR is registered with `nn::ro::RegisterModuleInfo()`
4. Each plugin is loaded with `nn::ro::LoadModule()`

See `PluginManager.cpp:12-207` for the complete loading process.

## Memory Layout

When loaded, plugins have separate memory regions:

```
struct PluginInfo {
    std::string Path;              // Plugin file path
    std::unique_ptr<u8> Data;      // NRO data (.text, .rodata, .data)
    size_t Size;                   // Data size
    utils::Sha256Hash Hash;        // Module hash
    nn::ro::Module Module;         // Module handle
    std::unique_ptr<u8> BssData;   // BSS section
    size_t BssSize;                // BSS size
};
```

* **Data** : Contains code (.text), read-only data (.rodata), and initialized data (.data)
* **BssData** : Zero-initialized data section
* Both regions are page-aligned (0x1000 bytes)

## Plugin Naming Conventions

### File Names

```
my_plugin.nro          # Snake case recommended
CoolFeature.nro        # Pascal case acceptable
player-mods.nro        # Kebab case acceptable
```

### Symbol Names

```
// Use namespaces to avoid conflicts
namespace myplugin {
    void initialize();
    void hook_player_update();
}

// Or prefix exported symbols
extern "C" void myplugin_main();
```

Use descriptive plugin names. The PluginManager logs the filename during loading, making it easier to identify which plugin is active.

## Advanced: Custom Init/Fini

Plugins can define constructor and destructor functions:

```
__attribute__((constructor))
void plugin_constructor() {
    // Called before main()
    // Useful for early initialization
}

__attribute__((destructor))
void plugin_destructor() {
    // Called when plugin is unloaded
    // Note: Skyline doesn't currently unload plugins
}
```

Plugin unloading is not currently implemented in Skyline. Destructor functions will not be called during normal operation.

## Best Practices

1. **Minimize Exports** : Only export `main()` and necessary functions
2. **Use Namespaces** : Avoid symbol conflicts with other plugins
3. **Handle Errors** : Check `Result` codes from nn:: APIs
4. **Log Liberally** : Use `skyline_tcp_send_raw()` for debugging
5. **Test Incrementally** : Start with simple hooks before complex modifications
6. **Document Hooks** : Comment which functions you’re hooking and why

## Next Steps

* Explore [practical plugin examples](https://mintlify.wiki/skyline-dev/skyline/plugins/examples)
* Learn about [function hooking techniques](https://mintlify.wiki/skyline-dev/skyline/api/hooks/function-hooks)
* Study [Skyline’s API reference](https://mintlify.wiki/skyline-dev/skyline/api/logger/overview)



# Plugin Examples

Real-world examples of Skyline plugins with complete, working code

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Overview

This page provides practical, working examples of Skyline plugins. All code is based on actual Skyline APIs and follows best practices for plugin development.

These examples use real API signatures from Skyline source code. No functions or parameters are invented.

## Basic Examples

* Hello World
* Logger Utility
* Memory Region Access

The simplest possible plugin that logs a message:

```
#include <cstring>

extern "C" void skyline_tcp_send_raw(void* data, size_t size);

void log(const char* msg) {
    skyline_tcp_send_raw((void*)msg, strlen(msg));
}

extern "C" void main() {
    log("Hello from Skyline plugin!\n");
}
```

**exported.txt:**

```
{
    global: main;
    local: *;
};
```

This plugin simply sends a message via TCP logging when loaded.

Hooking Examples

* Basic Function Hook
* Inline Hook
* Hook with Context

Replace a function with your own implementation:

```
#include <cstring>

extern "C" void skyline_tcp_send_raw(void* data, size_t size);
extern "C" void A64HookFunction(void* symbol, void* replace, void** result);

void log(const char* msg) {
    skyline_tcp_send_raw((void*)msg, strlen(msg));
}

// Original function pointer (game-specific)
void (*original_update_player)(void* player);

// Hook function - replaces the original
void hooked_update_player(void* player) {
    log("[Hook] Player update called!\n");

    // Call original function
    original_update_player(player);

    log("[Hook] Player update finished!\n");
}

extern "C" void main() {
    log("[Plugin] Installing hooks...\n");

    // Hook the function
    // Replace 0x71234567800 with actual game function address
    void* target = (void*)0x71234567800;
    A64HookFunction(
        target,
        (void*)hooked_update_player,
        (void**)&original_update_player
    );

    log("[Plugin] Hook installed!\n");
}
```

`A64HookFunction` performs a trampoline hook, preserving the original function for calling.

## Advanced Examples

* File System Operations
* Memory Patching
* Plugin Information
* IPC Communication

Read and write files using Nintendo’s filesystem API:

```
#include <cstring>
#include "nn/fs.h"  // From Skyline headers

extern "C" void skyline_tcp_send_raw(void* data, size_t size);
typedef unsigned int Result;

void log_format(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    skyline_tcp_send_raw(buffer, strlen(buffer));
}

void read_config_file() {
    nn::fs::FileHandle handle;
    const char* path = "sd:/config/myplugin.cfg";

    Result rc = nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Read);

    if (R_SUCCEEDED(rc)) {
        // Get file size
        s64 size;
        nn::fs::GetFileSize(&size, handle);

        // Read file
        char* buffer = new char[size + 1];
        nn::fs::ReadFile(handle, 0, buffer, size);
        buffer[size] = '\0';

        log_format("[Plugin] Config file contents:\n%s\n", buffer);

        delete[] buffer;
        nn::fs::CloseFile(handle);
    } else {
        log_format("[Plugin] Failed to open config file: 0x%x\n", rc);
    }
}

void write_log_file() {
    nn::fs::FileHandle handle;
    const char* path = "sd:/logs/myplugin.log";
    const char* data = "Plugin executed successfully\n";

    // Create file
    Result rc = nn::fs::CreateFile(path, strlen(data));
    if (R_SUCCEEDED(rc)) {
        rc = nn::fs::OpenFile(&handle, path, nn::fs::OpenMode_Write);
        if (R_SUCCEEDED(rc)) {
            nn::fs::WriteFile(handle, 0, data, strlen(data), 
                             nn::fs::WriteOption::CreateOption_None);
            nn::fs::CloseFile(handle);
            log_format("[Plugin] Log written to %s\n", path);
        }
    }
}

extern "C" void main() {
    log_format("[Plugin] Testing file operations...\n");
    read_config_file();
    write_log_file();
}
```

SD card must be mounted first. Skyline mounts it automatically in `main.cpp:111`.

## Complete Plugin Example

Here’s a complete, production-ready plugin with multiple features:

```
// main.cpp
#include <cstring>
#include <cstdarg>

extern "C" void skyline_tcp_send_raw(void* data, size_t size);
extern "C" void A64HookFunction(void* symbol, void* replace, void** result);
extern "C" void* getRegionAddress(int region);

namespace myplugin {

// Logger utility
class Logger {
public:
    static void Log(const char* msg) {
        skyline_tcp_send_raw((void*)msg, strlen(msg));
    }

    static void LogFormat(const char* format, ...) {
        char buffer[512];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Log(buffer);
    }
};

// Plugin state
struct PluginState {
    bool initialized;
    int hook_call_count;
    void* text_base;
};

static PluginState g_state = { false, 0, nullptr };

// Hook example
typedef void (*GameUpdateFn)(float delta_time);
GameUpdateFn original_game_update;

void hooked_game_update(float delta_time) {
    g_state.hook_call_count++;

    if (g_state.hook_call_count % 300 == 0) {  // Every ~5 seconds
        Logger::LogFormat("[MyPlugin] Still running (calls: %d)\n", 
                         g_state.hook_call_count);
    }

    // Call original
    original_game_update(delta_time);
}

void initialize() {
    Logger::Log("[MyPlugin] Initializing v1.0.0\n");

    // Get game memory
    g_state.text_base = getRegionAddress(0);  // Text region
    Logger::LogFormat("[MyPlugin] Game text base: 0x%016lx\n", 
                     (u64)g_state.text_base);

    // Install hooks
    // Replace 0x71234567800 with actual function address
    void* update_fn = (void*)((char*)g_state.text_base + 0x1234567800);
    A64HookFunction(
        update_fn,
        (void*)hooked_game_update,
        (void**)&original_game_update
    );

    g_state.initialized = true;
    Logger::Log("[MyPlugin] Initialization complete\n");
}

}  // namespace myplugin

extern "C" void main() {
    myplugin::initialize();
}
```

**Makefile:**

```
TARGET := myplugin
SOURCES := source
INCLUDES := include

ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

CXXFLAGS := -g -Wall -O2 -ffunction-sections $(ARCH) -D__SWITCH__ \
            -fno-rtti -fomit-frame-pointer -fno-exceptions -std=c++20

LDFLAGS := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
           -Wl,-Map,$(TARGET).map -Wl,--version-script=exported.txt \
           -shared -nodefaultlibs

LIBS := -lgcc -lstdc++

CPPFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
OFILES := $(CPPFILES:.cpp=.o)

all: $(TARGET).nro

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDES) -c $< -o $@

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

$(TARGET).nro: $(TARGET).elf
	elf2nro $< $@
	@echo "Built $(TARGET).nro"

clean:
	@rm -f $(OFILES) $(TARGET).elf $(TARGET).nro $(TARGET).map
```

**exported.txt:**

```
{
    global: main;
    local: *;
};
```

## Tips for Plugin Development

## Find Function Addresses

Use IDA Pro, Ghidra, or similar tools to find function addresses in the game binary. Skyline’s symbol files can help.

## Test Incrementally

Start with simple logging, then add hooks one at a time. This makes debugging easier.

## Use Namespaces

Wrap your plugin code in a namespace to avoid symbol conflicts with other plugins.

## Log Everything

TCP logging is your primary debugging tool. Log liberally during development.

## Next Steps

* Review [Skyline API reference](https://mintlify.wiki/skyline-dev/skyline/api/logger/overview) for all available functions
* Learn about [hook implementation details](https://mintlify.wiki/skyline-dev/skyline/api/hooks/function-hooks)
* Join the Skyline community for support and examples

## Common Patterns

### Error Handling

```
Result rc = some_operation();
if (R_FAILED(rc)) {
    Logger::LogFormat("Operation failed: 0x%x\n", rc);
    return;  // Or handle error appropriately
}
```

### Safe Memory Access

```
void* addr = (void*)0x71234567800;

// Always validate before dereferencing
if (addr != nullptr) {
    int value = *(int*)addr;
}
```

### Hook Signature Matching

```
// Ensure hook signature matches exactly
typedef void (*OriginalFn)(void* arg1, int arg2, float arg3);
OriginalFn original;

void hooked(void* arg1, int arg2, float arg3) {
    // Same signature as original
    original(arg1, arg2, arg3);
}
```


# Game Compatibility

Understanding Skyline compatibility across different Nintendo Switch games

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Overview

Skyline was originally designed for **Super Smash Bros. Ultimate** but can be adapted to work with other Nintendo Switch games. However, compatibility is not guaranteed out of the box due to game-specific initialization requirements and memory layouts.

Skyline’s initialization method is not universally compatible with every game. Compatibility issues arise from differences in how games initialize their runtime environment, handle memory, and structure their code.

## Why Compatibility Issues Occur

Compatibility problems stem from several technical factors:*  **Different initialization sequences** : Games initialize their runtime environment in different ways, and Skyline’s hooking mechanism may not align with all initialization patterns

* **Memory layout variations** : Each game has a unique memory structure, and Skyline needs to hook into specific functions that may be located at different addresses or may not exist
* **Game-specific filesystem usage** : Some games handle filesystem operations (like RomFS mounting) differently, which can conflict with Skyline’s hooks
* **Runtime environment differences** : Games may use different versions of Nintendo SDK libraries or implement custom runtime behaviors

## The main.npdm Requirement

Each game requires a **game-specific main.npdm file** that defines:* Process permissions and capabilities

* Filesystem access permissions
* Service access rights
* Kernel capabilities and syscall permissions
* Thread priorities and CPU assignments

The main.npdm file included in Skyline releases is specifically configured for **Super Smash Bros. Ultimate** (title ID `0x01006a800016e000`).

### Installation Instructions

1. Copy the `exefs` directory to: `atmosphere/contents/<game titleid>/`
2. **Remove the `main.npdm` file** unless you are modding Super Smash Bros. Ultimate
3. Obtain or create a game-specific main.npdm file using:
   * **HACTool** : Extract game metadata
   * **npdmtool** : Generate the npdm file with appropriate permissions

Plugin developers typically provide the appropriate main.npdm file for their target game. If you’re developing for a new game, you’ll need to create this file yourself.

## Compatible Games and Forks

Due to compatibility limitations, developers have created game-specific forks of Skyline:[Animal Crossing: New HorizonsModified Skyline for ACNH](https://github.com/3096/skyline)

[Pokémon Sword/ShieldPokémon-specific branch](https://github.com/3096/skyline/tree/sword)

[Xenoblade Chronicles DEXenoblade Definitive Edition support](https://github.com/3096/skyline/tree/xde)

[Dragon Quest XI SDragon Quest XI S adaptation](https://github.com/3096/skyline/tree/jack)

[Persona 5 RoyalP5R-specific implementation](https://github.com/Raytwo/p5rcbt)

[Persona 5 StrikersRust-based Persona 5 Strikers fork](https://github.com/Raytwo/masquerade-rs)

[Fire Emblem: Three HousesFE3H Rust implementation](https://github.com/three-houses-research-team/aldebaran-rs)

Many of these forks require building from source. Pre-built releases may not be available.

## Adapting Skyline for New Games

If you need to adapt Skyline for a game that doesn’t have a fork:

1

[](https://mintlify.wiki/skyline-dev/skyline/advanced/game-compatibility#)

Analyze the game's initialization

Use a debugger or emulator to understand how the game initializes its runtime environment

2

[](https://mintlify.wiki/skyline-dev/skyline/advanced/game-compatibility#)

Create a game-specific main.npdm

Extract the game’s metadata with HACTool and generate an appropriate npdm file with sufficient permissions

3

[](https://mintlify.wiki/skyline-dev/skyline/advanced/game-compatibility#)

Modify Skyline's initialization hooks

Adjust the hooks in `source/main.cpp` to match the game’s initialization sequence. Key areas include:* RomFS mounting hooks (`handleNnFsMountRom`)

* Socket initialization timing
* Plugin loading sequence

4

[](https://mintlify.wiki/skyline-dev/skyline/advanced/game-compatibility#)

Test and iterate

Test your modifications with the TCP logger enabled to identify crashes and hook failures

Some games like Fire Emblem: Three Houses and Pokémon Sword/Shield use socket configuration variants that conflict with Skyline’s default socket initialization. Check `source/main.cpp:29` for notes on these compatibility issues.

## Known Compatibility Notes

### Games with Socket Initialization Issues

From `source/main.cpp:29`:> Some games (Fire Emblem Three Houses, Pokemon Sword/Shield) use the `&Config` variant before Skyline initializes sockets and do not appreciate this.

These games require modified socket initialization timing.### 

Games with Guard Acquire Issues

From `source/main.cpp:38`:> Some older games (Final Fantasy 9) seem to have issues with `_cxa_guard_acquire` which gcc automatically adds when using static instances.

Skyline bypasses the singleton pattern for plugin managers on these games.### 

Games with Multiple RomFS Mounts

From `source/main.cpp:54`:> Some games such as Persona 5 Royal call `nn::fs::MountRom` multiple times.

Skyline uses a `call_once` mechanism to handle these cases properly.## 

Getting Help

If you encounter compatibility issues:1. Check if a game-specific fork exists in the list above

1. Search for game-specific modifications in the community
2. Enable TCP logging to diagnose initialization failures
3. Review the game’s crash reports in Atmosphere’s error logs


# Building from Source

Complete guide to building Skyline from source code

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Prerequisites

Before building Skyline, ensure you have the following tools and libraries installed:

## devkitPro

Complete devkitPro installation with devkitA64 toolchain

## devkitA64

ARM64 cross-compilation toolchain

## libnx

Nintendo Switch development library

## Python 3

Required for build scripts (genPatch.py, sendPatch.py)

### Environment Setup

1

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Install devkitPro

Download and install devkitPro from [https://devkitpro.org](https://devkitpro.org/)

2

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Set DEVKITPRO environment variable

```
export DEVKITPRO=/opt/devkitpro
```

The build system will error if `DEVKITPRO` is not set. Add this to your shell profile (`.bashrc`, `.zshrc`, etc.) for persistence.

3

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Install devkitA64 and libnx

```
sudo dkp-pacman -S devkitA64 libnx switch-tools
```

4

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Verify Python 3

```
python3 --version
```

If `python3` is not available, the Makefile will fall back to `python`.

## Build Process

1

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Clone the repository

```
git clone https://github.com/shadowninja108/Skyline.git
cd Skyline
```

2

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Build Skyline

Run the make command with the desired game version:

```
make CROSSVER=600
```

The `CROSSVER` parameter specifies the target game version without decimal points. For example, `600` represents version `6.0.0` of Super Smash Bros. Ultimate.

3

[](https://mintlify.wiki/skyline-dev/skyline/advanced/building-from-source#)

Locate build output

The compiled NSO file will be located at:

```
build600/Skyline600.nso
```

The build directory and output filename include the CROSSVER value.

## Build Configuration

### CROSSVER Parameter

The `CROSSVER` parameter is critical for building game-specific versions:

```
make CROSSVER=600  # For game version 6.0.0
make CROSSVER=1301 # For game version 13.0.1
```

This parameter affects:*  **Build directory** : `build$(CROSSVER)` (e.g., `build600`)

* **Output filename** : `Skyline$(CROSSVER).nso` (e.g., `Skyline600.nso`)
* **Linker script selection** : `linkerscripts/syms$(CROSSVER).ld`
* **C++ defines** : `-DCROSSVER=$(CROSSVER)` passed to compiler
* **Configuration files** : `patches/configs/$(CROSSVER).config`
* **Symbol maps** : `patches/maps/$(CROSSVER)/*.map`

Default CROSSVER is `600` if not specified. See `Makefile:6`.

### Disabling Logging

To build without logging support:

```
make CROSSVER=600 NOLOG=1
```

This adds the `-DNOLOG` compiler flag, removing TCP logger functionality and reducing binary size.### 

Build Targets

| Target                     | Description                                                  |
| -------------------------- | ------------------------------------------------------------ |
| `make` or `make all`   | Build Skyline NSO (default target)                           |
| `make skyline`           | Explicitly build Skyline NSO                                 |
| `make clean`             | Remove all build artifacts and compiled files                |
| `make send IP=<address>` | Build and send to Switch via network (requires sendPatch.py) |

## Build System Overview

### Makefile Structure

Skyline uses a two-stage makefile system:1.  **Makefile** : Top-level build orchestration

* Manages CROSSVER parameter
* Handles Python script execution
* Coordinates patch generation

1. **nso.mk** : NSO compilation rules

* Defines compiler flags and architecture
* Manages source file compilation
* Links final NSO binary

### Compilation Flags

From `nso.mk:43-54`:

```
ARCH := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec

CFLAGS := -g -Wall -ffunction-sections \
          $(ARCH) $(DEFINES) $(INCLUDE) -D__SWITCH__ -DCROSSVER=$(CROSSVER)

CXXFLAGS := $(CFLAGS) -fno-rtti -fomit-frame-pointer -fno-exceptions \
            -fno-asynchronous-unwind-tables -fno-unwind-tables \
            -enable-libstdcxx-allocator=new -fpermissive
```

Key optimizations:*  **No RTTI** : `-fno-rtti` removes runtime type information

* **No exceptions** : `-fno-exceptions` disables C++ exception handling
* **Frame pointer omission** : `-fomit-frame-pointer` for smaller code size
* **Position-independent code** : `-fPIC` for dynamic linking

### Linker Configuration

From `nso.mk:57`:

```
LDFLAGS = -specs=../switch.specs -g $(ARCH) \
          -Wl,-Map,$(notdir $*.map) \
          -Wl,--version-script=$(TOPDIR)/exported.txt \
          -Wl,-init=__custom_init \
          -Wl,-fini=__custom_fini \
          -Wl,--export-dynamic \
          -nodefaultlibs
```

Important linker settings:*  **Custom init/fini** : `__custom_init` and `__custom_fini` for controlled initialization

* **Export dynamic symbols** : Required for plugin system
* **No default libs** : Manual library linking with `-lgcc -lstdc++ -u malloc`

### Linker Scripts

Skyline uses version-specific linker scripts:

```
linkerscripts/
├── syms600.ld      # Symbols for version 6.0.0
├── syms1301.ld     # Symbols for version 13.0.1
└── application.ld  # Base application linker script
```

The build system copies the appropriate version:

```
# From nso.mk:143
cp linkerscripts/syms$(CROSSVER).ld linkerscripts/symstemp.ld
```

Linker scripts contain game-specific function addresses. Using the wrong CROSSVER will result in crashes or undefined behavior.

## Source Code Structure

From `nso.mk:36`:

```
SOURCES := source $(filter-out %.c %.cpp %.s,$(wildcard source/* source/*/* source/*/*/* source/*/*/*/*))
```

The build system recursively includes all subdirectories under `source/` up to 4 levels deep.### 

Include Directories

From `nso.mk:38`:

```
INCLUDES := include libs/libeiffel/include
```

Skyline includes the **libeiffel** library for additional functionality.## 

Troubleshooting Build Issues

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="error-please-set-devkitpro-in-your-environment accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="error-please-set-devkitpro-in-your-environment" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Error: Please set DEVKITPRO in your environment</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="linker-errors-about-missing-symbols accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="linker-errors-about-missing-symbols" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Linker errors about missing symbols</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="python-script-errors accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="python-script-errors" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Python script errors</p></div></summary>

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="undefined-reference-to-custom-init accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="undefined-reference-to-custom-init" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">undefined reference to '__custom_init'</p></div></summary>

</details>

## Advanced Build Options

### Building for Multiple Versions

You can build for multiple game versions simultaneously:

```
make CROSSVER=600
make CROSSVER=1301
```

Each build uses a separate build directory, so outputs won’t conflict.### 

Network Deployment

For development testing, you can build and deploy to your Switch over the network:

```
make send IP=192.168.1.100 CROSSVER=600
```

This requires:* The `sendPatch.py` script

* Network access to your Switch
* A homebrew environment that accepts network patches

## Build Output

Successful builds produce:

```
build600/
├── Skyline600.nso    # Compiled NSO binary
├── Skyline600.elf    # ELF executable (intermediate)
├── Skyline600.map    # Symbol map for debugging
└── *.o               # Object files
```

Keep the `.map` file for debugging. It contains symbol addresses that help identify crash locations.

## Next Steps

After building Skyline:1. Copy the NSO file to your SD card’s exefs directory

1. Configure the appropriate main.npdm for your game
2. Install plugins to the romfs/skyline/plugins directory
3. Test with TCP logging enabled for debugging


# Troubleshooting

Solutions to common Skyline issues and debugging techniques

> ## Documentation Index
>
> Fetch the complete documentation index at: [https://mintlify.com/skyline-dev/skyline/llms.txt](https://mintlify.com/skyline-dev/skyline/llms.txt)
>
> Use this file to discover all available pages before exploring further.

## Common Issues

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="game-crashes-immediately-on-launch accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="game-crashes-immediately-on-launch" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Game crashes immediately on launch</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#check-main-npdm-configuration)

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#verify-installation-path)

```

```

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#check-game-compatibility)

[](https://mintlify.wiki/skyline-dev/skyline/advanced/game-compatibility)

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="logger-not-connecting-tcp-connection-fails accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="logger-not-connecting-tcp-connection-fails" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Logger not connecting / TCP connection fails</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#verify-logger-setup)

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#common-causes)

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#socket-pool-issues)

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="plugins-not-loading accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="plugins-not-loading" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Plugins not loading</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#correct-plugin-path)

```

```

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#plugin-loading-sequence)

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#debugging-plugin-issues)

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="exception-handler-crash-debugging accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="exception-handler-crash-debugging" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Exception handler / Crash debugging</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#exception-handler-setup)

```

```

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#reading-crash-logs)

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#finding-crash-location)

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#vabort-handler)

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="romfs-not-mounting-plugin-initialization-delayed accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="romfs-not-mounting-plugin-initialization-delayed" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">RomFS not mounting / Plugin initialization delayed</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#romfs-hook-mechanism)

```

```

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#multiple-romfs-mounts)

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#if-plugins-never-load)

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="memory-allocation-failures accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="memory-allocation-failures" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Memory allocation failures</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#socket-pool-allocation)

```

```

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#plugin-memory)

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#debugging-memory-issues)

```

```

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="nn-ro-initialize-hook-not-working accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="nn-ro-initialize-hook-not-working" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">nn::ro::Initialize hook not working</p></div></summary>

### [](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#nnro-hook)

```

```

```

```

</details>

## Debugging Techniques

### Enable TCP Logging

1

[](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#)

Build with logging enabled

Ensure Skyline is built WITHOUT the `NOLOG=1` flag:

```
make CROSSVER=600  # Logging enabled by default
```

2

[](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#)

Set up TCP listener

On your computer, listen for TCP connections:

```
nc -l 6969  # Default port
```

Or use a GUI tool like Hercules or SocketTest.

3

[](https://mintlify.wiki/skyline-dev/skyline/advanced/troubleshooting#)

Launch the game

Skyline will connect to your computer and stream log messages in real-time.

You may need to modify the logger IP address in the source code to match your computer’s IP on the local network.

### Use Atmosphere Crash Reports

Atmosphere generates detailed crash reports at:

```
atmosphere/crash_reports/
```

These reports include:* Complete register dump

* Stack trace
* Memory info
* Module load addresses

Check Ryujinx / Yuzu Output

When testing on emulators:*  **Ryujinx** : Provides detailed logging output

* **Yuzu** : Shows dynamic library loading and hook information

Emulator behavior may differ from real hardware. Always test on actual Switch hardware for production plugins.

Memory Region Analysis

Skyline logs memory regions at startup (`source/main.cpp:135-139`):

```
[skyline_main] text: 0x... | rodata: 0x... | data: 0x... | bss: 0x... | heap: 0x...
```

Use these addresses to:* Calculate offsets for hooks

* Understand memory layout
* Debug pointer issues

## Performance Issues

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="game-runs-slower-with-skyline-loaded accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="game-runs-slower-with-skyline-loaded" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Game runs slower with Skyline loaded</p></div></summary>

</details>

<details class="accordion border-standard rounded-2xl mb-3 overflow-hidden bg-background-light dark:bg-codeblock cursor-default"><summary class="relative not-prose flex flex-row items-center content-center w-full cursor-pointer list-none [&::-webkit-details-marker]:hidden py-4 px-5 space-x-2 hover:bg-gray-100 hover:dark:bg-gray-800 rounded-t-xl" aria-controls="long-loading-times accordion children" aria-expanded="false" data-component-part="accordion-button"><div id="long-loading-times" class="absolute -top-[10.5rem]"></div><div class="mr-0.5" data-component-part="accordion-caret-right"><svg class="h-3 w-3 transition bg-gray-700 dark:bg-gray-400 duration-75"></svg></div><div class="leading-tight text-left w-full" contenteditable="false" data-component-part="accordion-title-container"><p class="m-0 font-medium text-gray-900 dark:text-gray-200" data-component-part="accordion-title">Long loading times</p></div></summary>

</details>

Getting Additional Help

### Information to Provide

When asking for help, include:1. **Game title and version**

1. **Skyline version or build date**
2. **Complete crash log** (TCP output or Atmosphere crash report)
3. **Plugin list** (if applicable)
4. **Build configuration** (CROSSVER, any custom flags)
5. **Installation path verification**

### Useful Resources

* **Atmosphere logs** : `/atmosphere/crash_reports/`
* **Build map file** : `build$(CROSSVER)/Skyline$(CROSSVER).map`
* **Exception handler output** : Via TCP logger
* **Memory region info** : Logged at startup

The memory region addresses logged at startup (text, rodata, data, bss, heap) are crucial for diagnosing hooking and pointer issues. Always include these in bug reports.
