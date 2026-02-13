// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

/**
 * @file dap_connection.hpp
 * @brief Content-Length DAP connection over stdio or TCP socket.
 */

#pragma once

#include <string>
#include <functional>
#include <mutex>
#include <cstdint>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socket_t = SOCKET;
constexpr socket_t kInvalidSocket = INVALID_SOCKET;
#else
using socket_t = int;
constexpr socket_t kInvalidSocket = -1;
#endif

class DapConnection {
public:
    DapConnection();
    ~DapConnection();

    /// Listen on a TCP port. Returns the actual port (useful when port==0).
    uint16_t ListenTcp(uint16_t port = 0);

    /// Accept one client connection (blocks until a client connects).
    bool AcceptClient();

    /// True if connected via TCP socket.
    bool IsTcp() const { return client_socket_ != kInvalidSocket; }

    /// Blocking loop: reads messages until EOF / disconnect.
    void Run(std::function<void(const std::string&)> onMessage);

    /// Thread-safe: send a JSON string with Content-Length header.
    void Send(const std::string& json);

private:
    std::mutex write_mutex_;
    socket_t listen_socket_{kInvalidSocket};
    socket_t client_socket_{kInvalidSocket};

    bool ReadMessage(std::string& outJson);
    bool ReadLine(std::string& outLine);
    bool ReadBytes(size_t n, std::string& out);
    void WriteRaw(const std::string& data);
};
