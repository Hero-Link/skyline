
#include "skyline/logger/DualLogger.hpp"
#include "skyline/utils/cpputils.hpp"
#include "nn/fs.h"

#define PORT 6969

extern "C" void skyline_dual_send_raw(char* data, size_t size) __attribute__((visibility("default")));

void skyline_dual_send_raw(char* data, u64 size) { skyline::logger::s_Instance->Log(data, size); }

namespace skyline::logger {
nn::fs::FileHandle dualfileHandle;
s64 Dualoffset;
int g_dualtcpSocket;

DualLogger::DualLogger(std::string path) {
    nn::fs::DirectoryEntryType type;
    Result rc = nn::fs::GetEntryType(&type, path.c_str());

    if (rc == 0x202) {  // Path does not exist
        rc = nn::fs::CreateFile(path.c_str(), 0);
    } else if (R_FAILED(rc))
        return;

    if (type == nn::fs::DirectoryEntryType_Directory) return;

    R_ERRORONFAIL(nn::fs::OpenFile(&dualfileHandle, path.c_str(), nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append));
}

void DualLogger::Initialize() {
    // Sockets are already initialized by skyline_socket_init() in TcpLogger.cpp.
    // Just set up the TCP accept for this logger's socket.

    struct sockaddr_in serverAddr;
    g_dualtcpSocket = nn::socket::Socket(AF_INET, SOCK_STREAM, 0);
    if (g_dualtcpSocket < 0) return;

    int flags = 1;
    nn::socket::SetSockOpt(g_dualtcpSocket, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof(flags));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = nn::socket::InetHtons(PORT);

    int rval = nn::socket::Bind(g_dualtcpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (rval < 0) {
        nn::socket::Close(g_dualtcpSocket);
        return;
    }

    rval = nn::socket::Listen(g_dualtcpSocket, 1);
    if (rval < 0) {
        nn::socket::Close(g_dualtcpSocket);
        return;
    }

    u32 addrLen = sizeof(serverAddr);
    s32 listenSocket = g_dualtcpSocket;
    g_dualtcpSocket = nn::socket::Accept(listenSocket, (struct sockaddr*)&serverAddr, &addrLen);
    nn::socket::Close(listenSocket);
}

void DualLogger::SendRaw(void* data, size_t size) {
    nn::fs::SetFileSize(dualfileHandle, Dualoffset + size);

    nn::socket::Send(g_dualtcpSocket, data, size, 0);

    nn::fs::WriteFile(dualfileHandle, Dualoffset, data, size,
                      nn::fs::WriteOption::CreateOption(nn::fs::WriteOptionFlag_Flush));

    Dualoffset += size;
};
};  // namespace skyline::logger
