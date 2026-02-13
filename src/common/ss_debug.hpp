// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

/**
 * @file ss_debug.hpp
 * @brief Debug controller for breakpoints, stepping, and stack inspection.
 */

#pragma once

#include "ss_value.hpp"
#include "ss_chunk.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace swive {

class VM;
class CallFrame;

// Debug event types
enum class DebugEvent {
    BreakpointHit,
    StepCompleted
};

// Step mode for stepping execution
enum class StepMode {
    None,       // Normal execution
    StepOver,   // Execute to next line in current frame
    StepInto,   // Execute to next line (entering functions)
    StepOut     // Execute until current frame returns
};

// Breakpoint information
struct Breakpoint {
    uint32_t id{0};
    std::string source_file;
    uint32_t line{0};
    bool enabled{true};
    uint32_t hit_count{0};
};

// Variable snapshot for inspection
struct DebugVariable {
    std::string name;
    Value value;
    uint16_t slot{0};
};

// Stack frame snapshot for inspection
struct DebugFrame {
    std::string function_name;
    std::string source_file;
    uint32_t line{0};
    size_t frame_index{0};
    std::vector<DebugVariable> locals;
};

// Debug event callback
using DebugCallback = std::function<void(DebugEvent event, const DebugFrame& frame)>;

class DebugController {
public:
    DebugController() = default;
    ~DebugController() = default;

    // --- Breakpoint management ---
    uint32_t add_breakpoint(uint32_t line, const std::string& source_file = "");
    bool remove_breakpoint(uint32_t breakpoint_id);
    void enable_breakpoint(uint32_t breakpoint_id, bool enabled);
    void clear_all_breakpoints();
    const std::vector<Breakpoint>& breakpoints() const { return breakpoints_; }

    // --- Execution control ---
    void step_over();
    void step_into();
    void step_out();
    void resume();
    void pause();

    // --- DAP blocking mode ---
    // When enabled, on_instruction() blocks the VM thread until resume/step is called.
    void set_blocking_mode(bool enabled) { blocking_mode_ = enabled; }
    bool is_blocking_mode() const { return blocking_mode_; }

    // DAP-style: replace all breakpoints for a source file
    void set_breakpoints_for_source(const std::string& source_file,
                                    const std::vector<uint32_t>& lines);

    // Set default source file (fallback for bodies without debug_info)
    void set_default_source_file(const std::string& path) {
        default_source_file_ = normalize_path(path);
    }

    // Wait until VM thread is paused (for DAP thread to call after sending stopped event)
    void wait_until_paused();

    // Notify the blocked VM thread to resume (used in blocking mode)
    void notify_resume();

    // --- Inspection (call while paused) ---
    std::vector<DebugFrame> get_stack_trace(VM& vm) const;
    std::vector<DebugVariable> get_locals(VM& vm, size_t frame_index = 0) const;

    // --- Callback ---
    void set_callback(DebugCallback callback) { callback_ = std::move(callback); }

    // --- Called by VM at each instruction boundary ---
    // Returns true if execution should pause
    bool on_instruction(VM& vm, size_t ip, body_idx body,
                        const MethodBody* method_body);

    // --- State queries ---
    bool is_paused() const { return paused_; }
    StepMode step_mode() const { return step_mode_; }

private:
    // Breakpoint storage
    std::vector<Breakpoint> breakpoints_;
    uint32_t next_breakpoint_id_{1};
    std::unordered_multimap<uint32_t, size_t> line_to_bp_; // line -> breakpoint index

    // Step state
    StepMode step_mode_{StepMode::None};
    size_t step_frame_depth_{0};
    uint32_t step_start_line_{0};
    // When true, skip breakpoint checks until the line changes.
    // Prevents re-hitting the same breakpoint after step/continue.
    bool skip_bp_on_resume_{false};

    // Pause state
    std::atomic<bool> paused_{false};
    std::atomic<bool> pause_requested_{false};

    // DAP blocking mode synchronization
    bool blocking_mode_{false};
    std::mutex mutex_;
    std::condition_variable resume_cv_;
    std::condition_variable pause_cv_;

    // Previous line (to detect line transitions)
    uint32_t prev_line_{0};

    // Callback
    DebugCallback callback_;

    // Default source file (fallback for bodies without debug_info)
    std::string default_source_file_;

    // Helpers
    uint32_t get_line(size_t ip, const MethodBody* body) const;
    std::string get_source_file(const MethodBody* body) const;
    DebugFrame build_current_frame(VM& vm, size_t ip, const MethodBody* body) const;
    bool check_breakpoint(uint32_t line, const std::string& source) const;
    static std::string normalize_path(const std::string& p);
};

} // namespace swive
