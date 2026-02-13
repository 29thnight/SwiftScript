// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

/**
 * @file dap_connection.cpp
 * @brief Content-Length DAP connection over stdio or TCP socket.
 */

#include "dap_connection.hpp"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>

// ============================================================================
// Construction / Destruction
// ============================================================================

DapConnection::DapConnection() {
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

DapConnection::~DapConnection() {
    if (client_socket_ != kInvalidSocket) {
#ifdef _WIN32
        closesocket(client_socket_);
#else
        ::close(client_socket_);
#endif
    }
    if (listen_socket_ != kInvalidSocket) {
#ifdef _WIN32
        closesocket(listen_socket_);
#else
        ::close(listen_socket_);
#endif
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

// ============================================================================
// TCP Server
// ============================================================================

uint16_t DapConnection::ListenTcp(uint16_t port) {
    listen_socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket_ == kInvalidSocket) return 0;

    // Allow address reuse
    int opt = 1;
    setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    if (::bind(listen_socket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        return 0;
    }
    if (::listen(listen_socket_, 1) != 0) {
        return 0;
    }

    // Get actual port (important when port==0)
    socklen_t len = sizeof(addr);
    getsockname(listen_socket_, reinterpret_cast<sockaddr*>(&addr), &len);
    return ntohs(addr.sin_port);
}

bool DapConnection::AcceptClient() {
    if (listen_socket_ == kInvalidSocket) return false;
    client_socket_ = ::accept(listen_socket_, nullptr, nullptr);
    return client_socket_ != kInvalidSocket;
}

// ============================================================================
// Message Loop
// ============================================================================

void DapConnection::Run(std::function<void(const std::string&)> onMessage) {
    std::string json;
    while (ReadMessage(json)) {
        onMessage(json);
    }
}

void DapConnection::Send(const std::string& json) {
    std::lock_guard<std::mutex> lock(write_mutex_);
    std::string header = "Content-Length: " + std::to_string(json.size()) + "\r\n\r\n";
    WriteRaw(header + json);
}

// ============================================================================
// I/O ? reads/writes via TCP socket if connected, else stdio
// ============================================================================

bool DapConnection::ReadMessage(std::string& outJson) {
    std::string line;
    size_t contentLen = 0;

    while (ReadLine(line)) {
        if (line == "\r\n" || line == "\n" || line.empty()) break;

        const std::string key = "Content-Length:";
        if (line.rfind(key, 0) == 0) {
            std::string rest = line.substr(key.size());
            size_t i = 0;
            while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t')) ++i;
            contentLen = static_cast<size_t>(std::stoul(rest.substr(i)));
        }
    }

    if (contentLen == 0) return false;
    return ReadBytes(contentLen, outJson);
}

bool DapConnection::ReadLine(std::string& outLine) {
    outLine.clear();
    char ch;
    while (true) {
        if (IsTcp()) {
            int r = ::recv(client_socket_, &ch, 1, 0);
            if (r <= 0) return false;
        } else {
            if (!std::cin.get(ch)) return false;
        }
        outLine.push_back(ch);
        if (ch == '\n') return true;
    }
}

bool DapConnection::ReadBytes(size_t n, std::string& out) {
    out.resize(n);
    if (IsTcp()) {
        size_t received = 0;
        while (received < n) {
            int r = ::recv(client_socket_, out.data() + received,
                           static_cast<int>(n - received), 0);
            if (r <= 0) return false;
            received += static_cast<size_t>(r);
        }
        return true;
    } else {
        return static_cast<bool>(
            std::cin.read(out.data(), static_cast<std::streamsize>(n)));
    }
}

void DapConnection::WriteRaw(const std::string& data) {
    if (IsTcp()) {
        size_t sent = 0;
        while (sent < data.size()) {
            int r = ::send(client_socket_, data.data() + sent,
                           static_cast<int>(data.size() - sent), 0);
            if (r <= 0) return;
            sent += static_cast<size_t>(r);
        }
    } else {
        std::cout << data;
        std::cout.flush();
    }
}
