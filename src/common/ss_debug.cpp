// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

#include "pch.h"
#include "ss_debug.hpp"
#include "ss_vm.hpp"

namespace swive {

// ============================================================================
// Breakpoint Management
// ============================================================================

uint32_t DebugController::add_breakpoint(uint32_t line, const std::string& source_file) {
    Breakpoint bp;
    bp.id = next_breakpoint_id_++;
    bp.line = line;
    bp.source_file = source_file;
    bp.enabled = true;
    bp.hit_count = 0;

    size_t idx = breakpoints_.size();
    breakpoints_.push_back(std::move(bp));
    line_to_bp_.emplace(line, idx);

    return breakpoints_.back().id;
}

bool DebugController::remove_breakpoint(uint32_t breakpoint_id) {
    for (size_t i = 0; i < breakpoints_.size(); ++i) {
        if (breakpoints_[i].id == breakpoint_id) {
            uint32_t line = breakpoints_[i].line;

            // Remove from line_to_bp_
            auto range = line_to_bp_.equal_range(line);
            for (auto it = range.first; it != range.second; ++it) {
                if (it->second == i) {
                    line_to_bp_.erase(it);
                    break;
                }
            }

            // Remove from vector (swap with last)
            if (i < breakpoints_.size() - 1) {
                // Update the line_to_bp_ for the swapped element
                uint32_t last_line = breakpoints_.back().line;
                auto range2 = line_to_bp_.equal_range(last_line);
                for (auto it = range2.first; it != range2.second; ++it) {
                    if (it->second == breakpoints_.size() - 1) {
                        it->second = i;
                        break;
                    }
                }
                breakpoints_[i] = std::move(breakpoints_.back());
            }
            breakpoints_.pop_back();
            return true;
        }
    }
    return false;
}

void DebugController::enable_breakpoint(uint32_t breakpoint_id, bool enabled) {
    for (auto& bp : breakpoints_) {
        if (bp.id == breakpoint_id) {
            bp.enabled = enabled;
            return;
        }
    }
}

void DebugController::clear_all_breakpoints() {
    breakpoints_.clear();
    line_to_bp_.clear();
}

// ============================================================================
// Execution Control
// ============================================================================

void DebugController::step_over() {
    step_mode_ = StepMode::StepOver;
    // Mark that we are stepping — skip_bp_on_resume_ prevents the breakpoint
    // on the current line from re-triggering before we actually move.
    skip_bp_on_resume_ = true;
    paused_ = false;
    if (blocking_mode_) notify_resume();
}

void DebugController::step_into() {
    step_mode_ = StepMode::StepInto;
    skip_bp_on_resume_ = true;
    paused_ = false;
    if (blocking_mode_) notify_resume();
}

void DebugController::step_out() {
    step_mode_ = StepMode::StepOut;
    skip_bp_on_resume_ = true;
    paused_ = false;
    if (blocking_mode_) notify_resume();
}

void DebugController::resume() {
    step_mode_ = StepMode::None;
    // On continue, also skip the breakpoint on the current line once,
    // otherwise the VM immediately re-hits the same breakpoint.
    skip_bp_on_resume_ = true;
    paused_ = false;
    pause_requested_ = false;
    if (blocking_mode_) notify_resume();
}

void DebugController::pause() {
    pause_requested_ = true;
}

// ============================================================================
// Instruction Hook (called by VM at every instruction boundary)
// ============================================================================

bool DebugController::on_instruction(VM& vm, size_t ip, body_idx body,
                                     const MethodBody* method_body) {
    if (!method_body || ip >= method_body->line_info.size()) {
        return false;
    }

    uint32_t line = get_line(ip, method_body);
    if (line == 0) {
        // Skip instructions with no line info, but do NOT update prev_line_
        // so the next valid line will still be detected as a transition.
        return false;
    }

    // Detect whether we moved to a different line
    bool line_changed = (line != prev_line_);

    // Fast path: same line and no pause/step pending
    if (!line_changed && !pause_requested_ && step_mode_ == StepMode::None) {
        return false;
    }

    uint32_t old_line = prev_line_;
    prev_line_ = line;

    // Once we move to a different line, clear the one-shot skip flag.
    if (line_changed) {
        skip_bp_on_resume_ = false;
    }

    bool should_pause = false;
    DebugEvent event = DebugEvent::BreakpointHit;

    // Check pause request
    if (pause_requested_) {
        pause_requested_ = false;
        should_pause = true;
        event = DebugEvent::StepCompleted;
    }

    // Check breakpoints (with source file matching).
    // skip_bp_on_resume_ prevents re-hitting the same breakpoint
    // immediately after step/continue on the current line.
    std::string current_source = get_source_file(method_body);
    if (!should_pause && !skip_bp_on_resume_ && check_breakpoint(line, current_source)) {
        std::cerr << "[DAP] Breakpoint hit: line=" << line << " source=" << current_source << std::endl;
        should_pause = true;
        event = DebugEvent::BreakpointHit;
        // Increment hit count
        auto range = line_to_bp_.equal_range(line);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second < breakpoints_.size() && breakpoints_[it->second].enabled) {
                breakpoints_[it->second].hit_count++;
            }
        }
    }

    // Check step conditions — only when the line actually changed.
    if (!should_pause && step_mode_ != StepMode::None && line_changed) {
        size_t current_depth = vm.call_frames().size();

        switch (step_mode_) {
        case StepMode::StepInto:
            should_pause = true;
            break;

        case StepMode::StepOver:
            if (current_depth <= step_frame_depth_) {
                should_pause = true;
            }
            break;

        case StepMode::StepOut:
            if (current_depth < step_frame_depth_) {
                should_pause = true;
            }
            break;

        default:
            break;
        }

        if (should_pause) {
            event = DebugEvent::StepCompleted;
            step_mode_ = StepMode::None;
        }
    }

    if (should_pause) {
        paused_ = true;

        // Record step state for future step commands
        step_frame_depth_ = vm.call_frames().size();
        step_start_line_ = line;

        if (callback_) {
            DebugFrame frame = build_current_frame(vm, ip, method_body);
            callback_(event, frame);
        }

        // In blocking mode: block VM thread until DAP thread calls resume/step
        if (blocking_mode_) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                pause_cv_.notify_all();  // Notify DAP thread that VM is paused
            }
            std::unique_lock<std::mutex> lock(mutex_);
            resume_cv_.wait(lock, [this] { return !paused_.load(); });
        }

        return true;
    }

    return false;
}

// ============================================================================
// Inspection
// ============================================================================

std::vector<DebugFrame> DebugController::get_stack_trace(VM& vm) const {
    std::vector<DebugFrame> trace;

    // Current frame (top-level or latest call)
    const auto& call_frames = vm.call_frames();
    const MethodBody* current_body = vm.current_method_body();

    // Build current (innermost) frame
    if (current_body) {
        DebugFrame top;
        if (!call_frames.empty()) {
            top.function_name = call_frames.back().function_name;
            top.frame_index = call_frames.size() - 1;
        } else {
            // Top-level: use SIZE_MAX as sentinel so get_locals() takes the
            // call_frames.empty() branch instead of indexing into an empty vector.
            top.function_name = "<top-level>";
            top.frame_index = SIZE_MAX;
        }
        size_t ip = vm.current_ip();
        if (ip > 0 && ip <= current_body->line_info.size()) {
            top.line = current_body->line_info[ip - 1];
        }
        if (current_body->debug_info && !current_body->debug_info->source_file.empty()) {
            top.source_file = current_body->debug_info->source_file;
        } else {
            top.source_file = default_source_file_;
        }
        top.locals = get_locals(vm, top.frame_index);
        trace.push_back(std::move(top));
    }

    // Walk call frames from top to bottom (skip the last one, already handled)
    for (int i = static_cast<int>(call_frames.size()) - 2; i >= 0; --i) {
        const CallFrame& cf = call_frames[i];
        DebugFrame frame;
        frame.function_name = cf.function_name;
        frame.frame_index = static_cast<size_t>(i);

        // Get line from return address in the caller's chunk
        if (cf.chunk && cf.body_index < cf.chunk->method_bodies.size()) {
            const MethodBody& caller_body = cf.chunk->method_bodies[cf.body_index];
            if (cf.return_address > 0 && cf.return_address <= caller_body.line_info.size()) {
                frame.line = caller_body.line_info[cf.return_address - 1];
            }
            if (caller_body.debug_info && !caller_body.debug_info->source_file.empty()) {
                frame.source_file = caller_body.debug_info->source_file;
            } else {
                frame.source_file = default_source_file_;
            }
        }

        frame.locals = get_locals(vm, static_cast<size_t>(i));
        trace.push_back(std::move(frame));
    }

    return trace;
}

std::vector<DebugVariable> DebugController::get_locals(VM& vm, size_t frame_index) const {
    std::vector<DebugVariable> vars;
    const auto& call_frames = vm.call_frames();
    const auto& stack = vm.stack();

    size_t stack_base = 0;
    const MethodBody* body = nullptr;
    size_t ip = 0;

    if (call_frames.empty() || frame_index == SIZE_MAX) {
        // Top-level: stack_base = 0, use current body
        stack_base = 0;
        body = vm.current_method_body();
        ip = vm.current_ip();
    } else if (frame_index < call_frames.size()) {
        const CallFrame& cf = call_frames[frame_index];
        stack_base = cf.stack_base;

        // Determine body for this frame
        if (frame_index == call_frames.size() - 1) {
            // Current (innermost) frame
            body = vm.current_method_body();
            ip = vm.current_ip();
        } else {
            // Older frame
            if (cf.chunk && cf.body_index < cf.chunk->method_bodies.size()) {
                body = &cf.chunk->method_bodies[cf.body_index];
            }
            ip = cf.return_address;
        }
    }

    if (!body || !body->debug_info) {
        // No debug info: return unnamed locals by slot index
        size_t stack_end;
        if (frame_index + 1 < call_frames.size()) {
            stack_end = call_frames[frame_index + 1].stack_base;
        } else {
            stack_end = stack.size();
        }

        for (size_t slot = stack_base; slot < stack_end; ++slot) {
            DebugVariable var;
            var.name = "local_" + std::to_string(slot - stack_base);
            var.slot = static_cast<uint16_t>(slot - stack_base);
            if (slot < stack.size()) {
                var.value = stack[slot];
            }
            vars.push_back(std::move(var));
        }
        return vars;
    }

    // Use debug info to get named locals
    const DebugInfo& debug = *body->debug_info;
    for (const auto& local : debug.locals) {
        // Check if variable is in scope at current IP
        if (ip >= local.scope_start_offset &&
            (local.scope_end_offset == 0 || ip < local.scope_end_offset)) {
            size_t abs_slot = stack_base + local.slot_index;
            if (abs_slot < stack.size()) {
                DebugVariable var;
                var.name = local.name;
                var.slot = local.slot_index;
                var.value = stack[abs_slot];
                vars.push_back(std::move(var));
            }
        }
    }

    return vars;
}

// ============================================================================
// Helpers
// ============================================================================

uint32_t DebugController::get_line(size_t ip, const MethodBody* body) const {
    if (!body || ip >= body->line_info.size()) return 0;
    return body->line_info[ip];
}

DebugFrame DebugController::build_current_frame(VM& vm, size_t ip,
                                                 const MethodBody* body) const {
    DebugFrame frame;
    const auto& call_frames = vm.call_frames();

    if (!call_frames.empty()) {
        frame.function_name = call_frames.back().function_name;
        frame.frame_index = call_frames.size() - 1;
    } else {
        frame.function_name = "<top-level>";
        frame.frame_index = SIZE_MAX;
    }

    frame.line = get_line(ip, body);

    if (body && body->debug_info && !body->debug_info->source_file.empty()) {
        frame.source_file = body->debug_info->source_file;
    } else {
        frame.source_file = default_source_file_;
    }

    frame.locals = get_locals(vm, frame.frame_index);
    return frame;
}

std::string DebugController::get_source_file(const MethodBody* body) const {
    if (body && body->debug_info && !body->debug_info->source_file.empty()) {
        return normalize_path(body->debug_info->source_file);
    }
    // Fallback to default source file (set by DAP server)
    return default_source_file_;
}

std::string DebugController::normalize_path(const std::string& p) {
    try {
        return std::filesystem::weakly_canonical(p).string();
    } catch (...) {
        // Fallback: just convert separators
        std::string result = p;
        std::replace(result.begin(), result.end(), '/', '\\');
        return result;
    }
}

static bool paths_equal(const std::string& a, const std::string& b) {
#ifdef _WIN32
    // Case-insensitive comparison on Windows
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = a[i], cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca += 32;
        if (cb >= 'A' && cb <= 'Z') cb += 32;
        if (ca == '/') ca = '\\';
        if (cb == '/') cb = '\\';
        if (ca != cb) return false;
    }
    return true;
#else
    return a == b;
#endif
}

bool DebugController::check_breakpoint(uint32_t line, const std::string& source) const {
    auto range = line_to_bp_.equal_range(line);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second < breakpoints_.size() && breakpoints_[it->second].enabled) {
            const auto& bp = breakpoints_[it->second];
            // Match if breakpoint has no source file, or sources match
            // Match if:
            //  - breakpoint has no source constraint, OR
            //  - both have source files and they match
            // Do NOT match when the current source is empty but the BP has a source.
            if (bp.source_file.empty() ||
                (!source.empty() && paths_equal(bp.source_file, source))) {
                return true;
            }
        }
    }
    return false;
}

// ============================================================================
// DAP Blocking Mode Support
// ============================================================================

void DebugController::set_breakpoints_for_source(
    const std::string& source_file,
    const std::vector<uint32_t>& lines)
{
    std::string norm_source = normalize_path(source_file);

    // Remove all existing breakpoints for this source
    for (size_t i = breakpoints_.size(); i > 0; --i) {
        if (breakpoints_[i - 1].source_file == norm_source) {
            remove_breakpoint(breakpoints_[i - 1].id);
        }
    }

    // Add new breakpoints with normalized path
    for (uint32_t line : lines) {
        add_breakpoint(line, norm_source);
    }
}

void DebugController::wait_until_paused() {
    std::unique_lock<std::mutex> lock(mutex_);
    pause_cv_.wait(lock, [this] { return paused_.load(); });
}

void DebugController::notify_resume() {
    std::lock_guard<std::mutex> lock(mutex_);
    resume_cv_.notify_one();
}

} // namespace swive
