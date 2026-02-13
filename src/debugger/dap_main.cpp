// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

/**
 * @file dap_main.cpp
 * @brief SwiveDebugAdapter.exe entry point.
 *
 * Supports two modes:
 *   stdio mode (default): DAP over stdin/stdout (no VM I/O possible)
 *   TCP mode (--dap-port N): DAP over TCP, stdin/stdout free for VM I/O
 *
 * Usage: SwiveDebugAdapter.exe [--dap-port <port>]
 *   --dap-port 0  ¡æ pick a free port, print it to stdout for the extension
 */

#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#include "dap_server.hpp"

int main(int argc, char* argv[]) {
    uint16_t dap_port = 0;
    bool use_tcp = false;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--dap-port") == 0 && i + 1 < argc) {
            dap_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            use_tcp = true;
        }
    }

    try {
        swive::DapServer server;

        if (use_tcp) {
            // TCP mode: DAP over socket, stdin/stdout free for VM I/O
            uint16_t actual_port = server.ListenTcp(dap_port);
            if (actual_port == 0) {
                std::cerr << "Failed to listen on TCP port" << std::endl;
                return 1;
            }
            // Print port for extension to connect
            std::cout << actual_port << std::endl;
            std::cout.flush();

            server.Run();
        } else {
            // stdio mode: DAP over stdin/stdout
#ifdef _WIN32
            _setmode(_fileno(stdin), _O_BINARY);
            _setmode(_fileno(stdout), _O_BINARY);
#endif
            std::ios_base::sync_with_stdio(false);
            std::cin.tie(nullptr);

            server.Run();
        }
    } catch (const std::exception& e) {
        std::cerr << "DAP server fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
