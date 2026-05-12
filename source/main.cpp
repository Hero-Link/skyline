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

// ---- RTC diagnostic: dump all dispatch entries ----
static uint64_t g_rtc_ctx;
static bool     g_rtc_pend;

// Diagnostic: dump ALL dispatch entries [0x0..0xF] for region 8 calls
static void wrap0_callback(InlineCtx* ctx) {
    uint32_t w1 = (uint32_t)ctx->registers[1].x;
    if (((w1 >> 12) & 0xF) != 8) return;
    g_rtc_ctx  = ctx->registers[0].x;
    g_rtc_pend = true;

    if (!skyline::logger::s_Instance) return;
    uint64_t c = g_rtc_ctx;
    // Dump all 16 dispatch slots
    uint32_t d[16];
    for (int i = 0; i < 16; i++) d[i] = *(uint32_t*)(c + 0x48 + i*4);
    skyline::logger::s_Instance->LogFormat(
        "[W0] W1=%08x d:%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x reg8=0x%x",
        w1, d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7],
        d[8],d[9],d[10],d[11],d[12],d[13],d[14],d[15],
        *(uint32_t*)(c+0x68));
}
static void lr_callback(InlineCtx* ctx) { g_rtc_pend = false; }

void skyline_main() {
    // populate our own process handle
    envSetOwnProcessHandle(skyline::proc_handle::Get());

    // init inline hooking
    A64HookInit();

    // KernelLogger: outputs via svcOutputDebugString, no SD mount needed
    skyline::logger::s_Instance = new skyline::logger::KernelLogger();
    skyline::logger::s_Instance->StartThread();
    skyline::logger::s_Instance->Log("[skyline_main] Beginning initialization.\n");

    // override exception handler to dump info
    nn::os::SetUserExceptionHandler(exception_handler, exception_handler_stack, sizeof(exception_handler_stack),
                                    &exception_info);

    // Hook MountRom to load plugins on first ROM mount
    A64HookFunction(reinterpret_cast<void*>(nn::fs::MountRom), reinterpret_cast<void*>(handleNnFsMountRom),
                    (void**)&nnFsMountRomImpl);

    // Hook ro::Initialize to prevent game from double-initializing
    A64HookFunction(reinterpret_cast<void*>(nn::ro::Initialize), reinterpret_cast<void*>(nn_ro_init), (void**)&nnRoInitializeImpl);

    // ---- RTC: W0 hook at dispatch wrapper entry ----
    {
        uint64_t addr = skyline::utils::g_MainTextAddr + 0xF88D0;
        A64InlineHook(reinterpret_cast<void*>(addr),
                      reinterpret_cast<void*>(wrap0_callback));
        skyline::logger::s_Instance->LogFormat("[RTC] W0 hook @ 0x%llx", addr);
    }

    // ---- RTC: LR hook at dispatch wrapper return point ----
    {
        uint64_t addr = skyline::utils::g_MainTextAddr + 0xF6E10;
        A64InlineHook(reinterpret_cast<void*>(addr),
                      reinterpret_cast<void*>(lr_callback));
        skyline::logger::s_Instance->LogFormat("[RTC] LR hook @ 0x%llx", addr);
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
