
#pragma once

#include <arpa/inet.h>

#include "alloc.h"
#include "mem.h"
#include "nn/socket.h"
#include "nn/time.h"
#include "operator.h"
#include "skyline/inlinehook/And64InlineHook.hpp"
#include "skyline/logger/Logger.hpp"

namespace skyline::logger {
class TcpLogger : public Logger {
   public:
    virtual void Initialize();
    virtual bool ShouldFlush() override;
    virtual void SendRaw(void*, size_t);
    virtual std::string FriendlyName() { return "TcpLogger"; }
};

    void setup_socket_hooks();
    void skyline_socket_init();
    void skyline_socket_finalize();
    void start_listen_thread();
};  // namespace skyline::logger

