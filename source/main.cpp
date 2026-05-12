#include "main.hpp"

#include "skyline/logger/KernelLogger.hpp"
// #include "skyline/logger/DualLogger.hpp"
#include "skyline/utils/ipc.hpp"
#include "skyline/utils/cpputils.hpp"
#include "skyline/utils/utils.h"
#include "skyline/utils/call_once.hpp"
#include "skyline/utils/cur_proc_handle.hpp"

// For handling exceptions
char ALIGNA(0x1000) exception_handler_stack[0x4000];
nn::os::UserExceptionInfo exception_info;

void exception_handler(nn::os::UserExceptionInfo* info) {
    skyline::logger::s_Instance->LogFormat("Exception occurred!\n");

    skyline::logger::s_Instance->LogFormat("Error description: %x\n", info->ErrorDescription);
    for (int i = 0; i < 29; i++)
        skyline::logger::s_Instance->LogFormat("X[%02i]: %" PRIx64 "\n", i, info->CpuRegisters[i].x);
    skyline::logger::s_Instance->LogFormat("FP: %" PRIx64 "\n", info->FP.x);
    skyline::logger::s_Instance->LogFormat("LR: %" PRIx64 "\n", info->LR.x);
    skyline::logger::s_Instance->LogFormat("SP: %" PRIx64 "\n", info->SP.x);
    skyline::logger::s_Instance->LogFormat("PC: %" PRIx64 "\n", info->PC.x);
}

static skyline::utils::Task* after_romfs_task = new skyline::utils::Task{[]() {
    // load plugins
    // Note: Bypassing the singleton-like system because some older games (Final Fantasy 9) seem to have issues with _cxa_guard_acquire which gcc automatically adds when using the static instance
    auto manager = new skyline::plugin::Manager();
    manager->LoadPluginsImpl();
}};

void stub() {}

static skyline::utils::Once g_MountRomInit;
Result (*nnFsMountRomImpl)(char const*, void*, unsigned long);

Result handleNnFsMountRom(char const* path, void* buffer, unsigned long size) {
    Result rc = 0;
    rc = nnFsMountRomImpl(path, buffer, size);

    skyline::utils::g_RomMountStr = std::string(path) + ":/";

    g_MountRomInit.call_once([]() {
        // Load plugins synchronously (no SafeTaskQueue/thread — HLE unreliable)
        auto manager = new skyline::plugin::Manager();
        manager->LoadPluginsImpl();
    });

    return rc;
}

void (*VAbortImpl)(char const*, char const*, char const*, int, Result const*, nn::os::UserExceptionInfo const*, char const*, va_list args);
void handleNnDiagDetailVAbortImpl(char const* str1, char const* str2, char const* str3, int int1, Result const* code, nn::os::UserExceptionInfo const* ExceptionInfo, char const* fmt, va_list args) {
    int len = vsnprintf(nullptr, 0, fmt, args);
    char* fmt_info = new char[len + 1];
    vsprintf(fmt_info, fmt, args);

    const char* fmt_str = "%s\n%s\n%s\n%d\nError: 0x%x\n%s";
    len = snprintf(nullptr, 0, fmt_str, str1, str2, str3, int1, *code, fmt_info);
    char* report = new char[len + 1];
    sprintf(report, fmt_str, str1, str2, str3, int1, *code, fmt_info);

    skyline::logger::s_Instance->LogFormat("%s", report);
    nn::err::ApplicationErrorArg* error =
        new nn::err::ApplicationErrorArg(69, "The software is aborting.", report,
                                         nn::settings::LanguageCode::Make(nn::settings::Language::Language_English));
    nn::err::ShowApplicationError(*error);
    delete[] report;
    delete[] fmt_info;
    VAbortImpl(str1, str2, str3, int1, code, ExceptionInfo, fmt, args);
}

static skyline::utils::Once g_RoInit;
Result (*nnRoInitializeImpl)();

Result nn_ro_init() {
    Result ret = 0;

    g_RoInit.call_once([&ret]() {
        ret = nnRoInitializeImpl();
        skyline::logger::s_Instance->LogFormat("[skyline_main] Ran hooked nn::ro::initialize (0x%x)", ret);
    });

    return ret;
}

// ---- RTC Diagnostic via A64InlineHook ----
// caseD_2 sets a flag; FUN_001fea4c checks it to only log GPIO writes
// ---- RTC Implementation ----

// Hook all 4 dispatch wrappers: capture when region == 8 (GPIO range)
// All decode W1 the same way: region=(W1>>12)&0xF, type_idx=(W1>>8)&0xF
static void disp_callback(InlineCtx* ctx) {
    uint32_t w1 = (uint32_t)ctx->registers[1].x;
    uint32_t region = (w1 >> 12) & 0xF;
    if (region != 8) return;
    if (!skyline::logger::s_Instance) return;

    uint32_t type_idx = (w1 >> 8) & 0xF;
    uint32_t sub     = (w1 >> 16) & 0xF;
    uint64_t lr      = ctx->registers[30].x;

    // Also read dispatch table entry for this type
    uint64_t ctx_ptr  = ctx->registers[0].x;
    uint32_t disp_val = *(uint32_t*)(ctx_ptr + 0x48 + type_idx * 4);
    uint32_t reg_val  = *(uint32_t*)(ctx_ptr + 0x48 + region * 4);

    skyline::logger::s_Instance->LogFormat(
        "[R8] W1=%08x region=%x type=0x%x sub=%x disp=0x%x reg=0x%x LR=%llx",
        w1, region, type_idx, sub, disp_val, reg_val, lr);
}

void skyline_main() {
    // populate our own process handle
    envSetOwnProcessHandle(skyline::proc_handle::Get());

    // init inline hooking
    A64HookInit();

    // socket init/hooks disabled: interferes with NSO network stack
    // skyline::logger::skyline_socket_init();
    // skyline::logger::setup_socket_hooks();

    // KernelLogger: outputs via svcOutputDebugString, no SD mount needed
    // (MountSdCardForDebug causes ResultFsDefaultAllocatorAlreadyUsed)
    skyline::logger::s_Instance = new skyline::logger::KernelLogger();
    skyline::logger::s_Instance->StartThread();
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // TCP listen thread disabled: no DualLogger
    // skyline::logger::start_listen_thread();

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);

    // Hook MountRom to load plugins on first ROM mount
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);

    // Hook ro::Initialize to prevent game from double-initializing
    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(nn_ro_init), (void**)&nnRoInitializeImpl);

    // ---- RTC: hook all 4 dispatch wrappers ----
    {
        uint64_t addrs[] = {
            skyline::utils::g_MainTextAddr + 0xF88D0,  // param_5=0 (write)
            skyline::utils::g_MainTextAddr + 0xF8A90,  // param_5=1
            skyline::utils::g_MainTextAddr + 0xF8AB0,  // param_5=2 (caseD_2)
            skyline::utils::g_MainTextAddr + 0xF8AD0,  // param_5=3 (direct write)
        };
        for (int i = 0; i < 4; i++) {
            A64InlineHook(reinterpret_cast<void*>(addrs[i]),
                          reinterpret_cast<void*>(disp_callback));
        }
        skyline::logger::s_Instance->LogFormat("[RTC] Hooked 4 disp wrappers");
    }

    skyline::logger::s_Instance->LogFormat("[skyline_main] text: 0x%" PRIx64 " | rodata: 0x%" PRIx64
                                           " | data: 0x%" PRIx64 " | bss: 0x%" PRIx64 " | heap: 0x%" PRIx64,
                                           skyline::utils::g_MainTextAddr, skyline::utils::g_MainRodataAddr,
                                           skyline::utils::g_MainDataAddr, skyline::utils::g_MainBssAddr,
                                           skyline::utils::g_MainHeapAddr);
}

extern "C" void skyline_init() {
    skyline::utils::init();
    virtmemSetup();  // needed for libnx JIT

    skyline_main();
}
