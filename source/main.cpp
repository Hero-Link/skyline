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

// ---- RTC v8: W0+LR hooks for W1=0925849C (time read path) ----
// W1=0925849C naturally produces reg=0x1~0xD (13 values cycling).
// We replace those with real BCD-encoded calendar time.
static uint64_t g_rtc_ctx;
static bool     g_rtc_pend;
static uint32_t g_rtc_time_val;   // BCD time byte to write to ctx[0x68]
static int      g_rtc_time_idx;   // 0-6 cycling through 7 time bytes

static uint8_t rtc_bcd(unsigned val) {
    return ((val / 10) << 4) | (val % 10);
}

static void rtc_prepare_time() {
    // Hardcoded reference: use system time offset for testing
    // BCD bytes: year, month, day, weekday, hour, min, sec
    uint8_t bytes[7] = {
        rtc_bcd(26),  // year 2026→26
        rtc_bcd(5),   // month May
        rtc_bcd(13),  // day 13
        rtc_bcd(3),   // weekday Wed=3
        rtc_bcd(14),  // hour (approx now)
        rtc_bcd(30),  // minute
        rtc_bcd(0)    // second
    };
    g_rtc_time_idx = (g_rtc_time_idx + 1) % 7;
    g_rtc_time_val = bytes[g_rtc_time_idx];
}

// GPIO host address for region 8, cached on first W0 call
static uint64_t g_gpio_host;  // base_ptr + offset for GPIO data register
static bool     g_gpio_found;

static void wrap0_callback(InlineCtx* ctx) {
    uint32_t w1 = (uint32_t)ctx->registers[1].x;
    if (((w1 >> 12) & 0xF) != 8) return;

    uint64_t c = ctx->registers[0].x;
    g_rtc_ctx = c;

    // Find GPIO host address on first call
    if (!g_gpio_found) {
        uint64_t rt = *(uint64_t*)(c + 0x980);
        if (rt) {
            uint64_t re = *(uint64_t*)(rt + 0x90);
            if (re) {
                uint64_t base = *(uint64_t*)(re + 0x10);
                uint32_t size = *(uint32_t*)(re + 0x20);
                g_gpio_host = base + ((size-1) & 0xC4);
                g_gpio_found = true;
                skyline::logger::s_Instance->LogFormat(
                    "[GPIO] base=%llx size=0x%x host=%llx",
                    base, size, g_gpio_host);
            }
        }
    }

    // Write 0xFF to GPIO host on every region-8 call
    if (g_gpio_found) {
        *(uint32_t*)(g_gpio_host) = 0x000000FF;
    }

    g_rtc_pend = true;
}

// LR: fires after dispatch wrapper returns → write BCD time to ctx[0x68]
static void lr_callback(InlineCtx* ctx) {
    if (!g_rtc_pend) return;
    g_rtc_pend = false;

    uint32_t old = *(uint32_t*)(g_rtc_ctx + 0x68);
    *(uint32_t*)(g_rtc_ctx + 0x68) = g_rtc_time_val;

    if (skyline::logger::s_Instance)
        skyline::logger::s_Instance->LogFormat(
            "[LR] reg: 0x%x→0x%x idx=%d", old, g_rtc_time_val, g_rtc_time_idx);
}

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

    // ---- RTC: W0 + LR double-hook for time read (W1=0925849C) ----
    {
        uint64_t w0_addr = skyline::utils::g_MainTextAddr + 0xF88D0;
        uint64_t lr_addr = skyline::utils::g_MainTextAddr + 0xF6E10;
        A64InlineHook(reinterpret_cast<void*>(w0_addr),
                      reinterpret_cast<void*>(wrap0_callback));
        A64InlineHook(reinterpret_cast<void*>(lr_addr),
                      reinterpret_cast<void*>(lr_callback));
        skyline::logger::s_Instance->LogFormat("[RTC] W0+LR hooks for time read");
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
