#pragma once

#include <arpa/inet.h>

#include "alloc.h"
#include "mem.h"
#include "nn/socket.h"
#include "nn/time.h"
#include "operator.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/Logger.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "skyline/nx/kernel/svc.h"

#ifdef __cplusplus
}
#endif

namespace skyline::logger {

// Logger that sends to both svcOutputDebugString and a TCP socket
class DualLogger : public Logger {
   public:
    virtual void Initialize();
    virtual bool ShouldFlush() override;
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "DualLogger"; }
};

    void setup_socket_hooks();
    void skyline_socket_init();
    void start_listen_thread();
};  // namespace skyline::logger
