// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

/**
 * @file dap_server.hpp
 * @brief Debug Adapter Protocol server.
 *
 * Handles DAP JSON messages over stdin/stdout, manages VM execution
 * on a separate thread, and bridges between VS Code and DebugController.
 */

#pragma once

// Standard headers needed before project headers (no PCH)
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <string_view>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <nlohmann/json.hpp>

#include "dap_connection.hpp"
#include "ss_vm.hpp"
#include "ss_debug.hpp"
#include "ss_chunk.hpp"

namespace swive {

class DapServer {
public:
    DapServer();
    ~DapServer();

    /// Listen on TCP port for DAP. Returns actual port.
    uint16_t ListenTcp(uint16_t port);

    /// Main loop ? blocks on message input (stdio or TCP).
    void Run();

private:
    using json = nlohmann::json;

    // ---- Message dispatch ----
    void Dispatch(const std::string& raw);

    // ---- DAP request handlers ----
    void OnInitialize(int seq, const json& args);
    void OnLaunch(int seq, const json& args);
    void OnSetBreakpoints(int seq, const json& args);
    void OnConfigurationDone(int seq, const json& args);
    void OnThreads(int seq, const json& args);
    void OnStackTrace(int seq, const json& args);
    void OnScopes(int seq, const json& args);
    void OnVariables(int seq, const json& args);
    void OnContinue(int seq, const json& args);
    void OnNext(int seq, const json& args);
    void OnStepIn(int seq, const json& args);
    void OnStepOut(int seq, const json& args);
    void OnPause(int seq, const json& args);
    void OnEvaluate(int seq, const json& args);
    void OnDisconnect(int seq, const json& args);

    // ---- Response / event helpers ----
    void SendResponse(int request_seq, const std::string& command,
                      bool success, const json& body = json::object());
    void SendErrorResponse(int request_seq, const std::string& command,
                           const std::string& message);
    void SendEvent(const std::string& event, const json& body = json::object());
    void SendStoppedEvent(const std::string& reason, int thread_id = 1);
    void SendOutputEvent(const std::string& text,
                         const std::string& category = "console");
    void SendTerminatedEvent();

    // ---- VM thread ----
    void RunVM();

    // ---- State ----
    DapConnection connection_;
    DebugController controller_;
    std::unique_ptr<VM> vm_;
    std::unique_ptr<Assembly> assembly_;

    std::thread vm_thread_;
    std::atomic<bool> vm_running_{false};
    std::atomic<bool> disconnecting_{false};

    int seq_counter_{1};
    std::mutex seq_mutex_;

    // Launch parameters
    std::string program_path_;
    std::string build_type_{"Debug"};
    bool stop_on_entry_{false};
    bool entry_reported_{false};

    // Helpers
    static std::string GetValueTypeName(const Value& val);
};

} // namespace swive
