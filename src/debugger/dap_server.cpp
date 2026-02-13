// SPDX-License-Identifier: MIT
// Copyright (c) 2025 29thnight

#include "dap_server.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_compiler.hpp"
#include "ss_project.hpp"
#include "ss_project_resolver.hpp"

namespace swive {

DapServer::DapServer() = default;

DapServer::~DapServer() {
    disconnecting_ = true;
    if (vm_thread_.joinable()) {
        controller_.resume();
        vm_thread_.join();
    }
}

uint16_t DapServer::ListenTcp(uint16_t port) {
    return connection_.ListenTcp(port);
}

void DapServer::Run() {
    // In TCP mode, wait for VS Code to connect
    connection_.AcceptClient();  // No-op if no listen socket (stdio mode)

    connection_.Run([this](const std::string& raw) { Dispatch(raw); });
}

void DapServer::Dispatch(const std::string& raw) {
    json msg;
    try { msg = json::parse(raw); } catch (...) { return; }

    std::string type = msg.value("type", "");
    if (type != "request") return;

    int seq = msg.value("seq", 0);
    std::string command = msg.value("command", "");
    json args = msg.value("arguments", json::object());

    if      (command == "initialize")        OnInitialize(seq, args);
    else if (command == "launch")            OnLaunch(seq, args);
    else if (command == "setBreakpoints")    OnSetBreakpoints(seq, args);
    else if (command == "configurationDone") OnConfigurationDone(seq, args);
    else if (command == "threads")           OnThreads(seq, args);
    else if (command == "stackTrace")        OnStackTrace(seq, args);
    else if (command == "scopes")            OnScopes(seq, args);
    else if (command == "variables")         OnVariables(seq, args);
    else if (command == "continue")          OnContinue(seq, args);
    else if (command == "next")              OnNext(seq, args);
    else if (command == "stepIn")            OnStepIn(seq, args);
    else if (command == "stepOut")           OnStepOut(seq, args);
    else if (command == "pause")             OnPause(seq, args);
    else if (command == "evaluate")          OnEvaluate(seq, args);
    else if (command == "disconnect")        OnDisconnect(seq, args);
    else SendResponse(seq, command, true);
}

// ============================================================================
// DAP Request Handlers
// ============================================================================

void DapServer::OnInitialize(int seq, const json&) {
    json caps;
    caps["supportsConfigurationDoneRequest"] = true;
    caps["supportsFunctionBreakpoints"] = false;
    caps["supportsConditionalBreakpoints"] = false;
    caps["supportsEvaluateForHovers"] = false;
    caps["supportsStepBack"] = false;
    caps["supportsSetVariable"] = false;
    caps["supportsRestartFrame"] = false;
    caps["supportsGotoTargetsRequest"] = false;
    caps["supportsStepInTargetsRequest"] = false;
    caps["supportsCompletionsRequest"] = false;
    caps["supportsModulesRequest"] = false;
    caps["supportsExceptionOptions"] = false;
    caps["supportsTerminateRequest"] = false;
    SendResponse(seq, "initialize", true, caps);
    SendEvent("initialized");
}

void DapServer::OnLaunch(int seq, const json& args) {
    program_path_  = args.value("program", "");
    build_type_    = args.value("buildType", "Debug");
    stop_on_entry_ = args.value("stopOnEntry", false);

    if (program_path_.empty()) {
        SendErrorResponse(seq, "launch", "Missing 'program' in launch config.");
        return;
    }
    namespace fs = std::filesystem;
    if (!fs::exists(program_path_)) {
        SendErrorResponse(seq, "launch", "Not found: " + program_path_);
        return;
    }

    try {
        SSProject project;
        std::string load_err;
        if (!LoadSSProject(program_path_, project, load_err)) {
            SendErrorResponse(seq, "launch", "Project load failed: " + load_err);
            return;
        }
        std::string source;
        {
            std::ifstream f(project.entry_file, std::ios::binary);
            if (!f.is_open()) {
                SendErrorResponse(seq, "launch",
                    "Cannot open: " + project.entry_file.string());
                return;
            }
            source.assign((std::istreambuf_iterator<char>(f)),
                           std::istreambuf_iterator<char>());
        }
        Lexer lexer(source);
        auto tokens = lexer.tokenize_all();
        Parser parser(std::move(tokens));
        auto program_ast = parser.parse();

        ProjectModuleResolver resolver(project.import_roots);
        Compiler compiler;
        compiler.set_base_directory(project.project_dir.string());
        compiler.set_module_resolver(&resolver);

        // Normalize the source file path so it matches VS Code's breakpoint paths
        std::string source_path = project.entry_file.string();
        try {
            source_path = std::filesystem::weakly_canonical(project.entry_file).string();
        } catch (...) {}
        compiler.set_source_file(source_path);
        compiler.set_emit_debug_info(true);
        assembly_ = std::make_unique<Assembly>(compiler.compile(program_ast));

        // Diagnostics: verify debug info in assembled bodies
        std::cerr << "[DAP] Compiler source_file=" << source_path << std::endl;
        std::cerr << "[DAP] Assembly bodies=" << assembly_->method_bodies.size() << std::endl;
        if (!assembly_->method_definitions.empty()) {
            std::cerr << "[DAP] Entry body_ptr=" << assembly_->method_definitions.front().body_ptr << std::endl;
        }
        for (size_t i = 0; i < assembly_->method_bodies.size(); ++i) {
            auto& mb = assembly_->method_bodies[i];
            if (mb.debug_info) {
                std::cerr << "[DAP] body[" << i << "] debug_info: fn="
                          << mb.debug_info->function_name
                          << " src=" << mb.debug_info->source_file
                          << " locals=" << mb.debug_info->locals.size()
                          << std::endl;
            } else {
                std::cerr << "[DAP] body[" << i << "] debug_info: NULL"
                          << " line_info=" << mb.line_info.size() << std::endl;
            }
        }

        // Set default source file on controller for bodies missing debug_info
        controller_.set_default_source_file(source_path);

        SendOutputEvent("Compiled: " + source_path);
    } catch (const std::exception& e) {
        SendErrorResponse(seq, "launch",
            std::string("Compilation failed: ") + e.what());
        return;
    }

    controller_.set_blocking_mode(true);
    controller_.set_callback(
        [this, stop_on_entry = stop_on_entry_](DebugEvent event, const DebugFrame&) {
            std::string reason;
            switch (event) {
            case DebugEvent::BreakpointHit: reason = "breakpoint"; break;
            case DebugEvent::StepCompleted:
                // First pause from stopOnEntry should report "entry"
                if (stop_on_entry && !entry_reported_) {
                    reason = "entry";
                    entry_reported_ = true;
                } else {
                    reason = "step";
                }
                break;
            default: reason = "pause"; break;
            }
            SendStoppedEvent(reason);
        });

    VMConfig config;
    config.enable_debug = true;
    vm_ = std::make_unique<VM>(config);
    vm_->attach_debugger(&controller_);

    // In TCP mode, VM uses stdout directly (terminal I/O for print/readLine)
    // In stdio mode, redirect print to DAP output events (stdout is DAP protocol)
    if (!connection_.IsTcp()) {
        vm_->set_output_handler([this](const std::string& text) {
            SendOutputEvent(text, "stdout");
        });
    }

    if (stop_on_entry_) controller_.pause();
    SendResponse(seq, "launch", true);
}

void DapServer::OnSetBreakpoints(int seq, const json& args) {
    json src = args.value("source", json::object());
    std::string path = src.value("path", "");
    json bp_arr = json::array();
    if (args.contains("breakpoints")) {
        std::vector<uint32_t> lines;
        for (const auto& bp : args["breakpoints"])
            lines.push_back(bp.value("line", 0));
        std::string norm = path;
        try { norm = std::filesystem::weakly_canonical(path).string(); }
        catch (...) {}
        std::cerr << "[DAP] setBreakpoints: path=" << norm << " lines=";
        for (auto ln : lines) std::cerr << ln << " ";
        std::cerr << std::endl;
        controller_.set_breakpoints_for_source(norm, lines);
        for (uint32_t ln : lines) {
            json b;
            b["verified"] = true;
            b["line"] = ln;
            b["source"] = { {"path", path} };
            bp_arr.push_back(b);
        }
    }
    json body;
    body["breakpoints"] = bp_arr;
    SendResponse(seq, "setBreakpoints", true, body);
}

void DapServer::OnConfigurationDone(int seq, const json&) {
    SendResponse(seq, "configurationDone", true);
    if (assembly_ && vm_ && !vm_running_) {
        vm_running_ = true;
        vm_thread_ = std::thread([this]() { RunVM(); });
    }
}

void DapServer::OnThreads(int seq, const json&) {
    json body;
    body["threads"] = json::array({ { {"id", 1}, {"name", "main"} } });
    SendResponse(seq, "threads", true, body);
}

void DapServer::OnStackTrace(int seq, const json&) {
    if (!vm_ || !controller_.is_paused()) {
        SendErrorResponse(seq, "stackTrace", "VM is not paused.");
        return;
    }
    auto frames = controller_.get_stack_trace(*vm_);
    json sf = json::array();
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& f = frames[i];
        json fr;
        fr["id"]     = static_cast<int>(i);
        fr["name"]   = f.function_name;
        fr["line"]   = f.line;
        fr["column"] = 1;
        if (!f.source_file.empty()) {
            fr["source"] = {
                {"name", std::filesystem::path(f.source_file).filename().string()},
                {"path", f.source_file}
            };
        }
        sf.push_back(fr);
    }
    json body;
    body["stackFrames"] = sf;
    body["totalFrames"] = static_cast<int>(sf.size());
    SendResponse(seq, "stackTrace", true, body);
}

void DapServer::OnScopes(int seq, const json& args) {
    int fid = args.value("frameId", 0);
    json scopes = json::array();
    json ls;
    ls["name"] = "Locals";
    ls["presentationHint"] = "locals";
    ls["variablesReference"] = 1000 + fid;
    ls["expensive"] = false;
    scopes.push_back(ls);
    json body;
    body["scopes"] = scopes;
    SendResponse(seq, "scopes", true, body);
}

void DapServer::OnVariables(int seq, const json& args) {
    int ref = args.value("variablesReference", 0);
    if (!vm_ || !controller_.is_paused()) {
        SendErrorResponse(seq, "variables", "VM is not paused.");
        return;
    }
    json variables = json::array();
    if (ref >= 1000 && ref < 2000) {
        int dap_fid = ref - 1000;
        auto frames = controller_.get_stack_trace(*vm_);

        // Use the frame_index from the trace consistently for all DAP frame IDs.
        // frames[0] = innermost (current), frames[1] = caller, etc.
        if (static_cast<size_t>(dap_fid) < frames.size()) {
            size_t fi = frames[dap_fid].frame_index;
            auto locals = controller_.get_locals(*vm_, fi);
            for (const auto& v : locals) {
                json var;
                var["name"]  = v.name;
                var["value"] = v.value.to_string();
                var["type"]  = GetValueTypeName(v.value);
                var["variablesReference"] = 0;
                variables.push_back(var);
            }
        }
    }
    json body;
    body["variables"] = variables;
    SendResponse(seq, "variables", true, body);
}

void DapServer::OnContinue(int seq, const json&) {
    json body;
    body["allThreadsContinued"] = true;
    SendResponse(seq, "continue", true, body);
    controller_.resume();
}

void DapServer::OnNext(int seq, const json&) {
    SendResponse(seq, "next", true);
    controller_.step_over();
}

void DapServer::OnStepIn(int seq, const json&) {
    SendResponse(seq, "stepIn", true);
    controller_.step_into();
}

void DapServer::OnStepOut(int seq, const json&) {
    SendResponse(seq, "stepOut", true);
    controller_.step_out();
}

void DapServer::OnPause(int seq, const json&) {
    SendResponse(seq, "pause", true);
    controller_.pause();
}

void DapServer::OnEvaluate(int seq, const json& args) {
    std::string expr = args.value("expression", "");
    json body;
    body["result"] = "(evaluate not yet supported: " + expr + ")";
    body["variablesReference"] = 0;
    SendResponse(seq, "evaluate", true, body);
}

void DapServer::OnDisconnect(int seq, const json&) {
    SendResponse(seq, "disconnect", true);
    disconnecting_ = true;
    if (controller_.is_paused()) controller_.resume();
    if (vm_thread_.joinable()) vm_thread_.join();
    std::exit(0);
}

// ============================================================================
// VM thread
// ============================================================================

void DapServer::RunVM() {
    try {
        SendOutputEvent("Execution started.");
        Value result = vm_->execute(*assembly_);
        if (!disconnecting_) {
            SendOutputEvent("Execution finished. Result: " + result.to_string());
            SendTerminatedEvent();
        }
    } catch (const std::exception& e) {
        if (!disconnecting_) {
            SendOutputEvent(std::string("Runtime error: ") + e.what(), "stderr");
            SendTerminatedEvent();
        }
    }
    vm_running_ = false;
}

// ============================================================================
// Protocol helpers
// ============================================================================

void DapServer::SendResponse(int request_seq, const std::string& command,
                              bool success, const json& body) {
    json r;
    { std::lock_guard<std::mutex> lk(seq_mutex_); r["seq"] = seq_counter_++; }
    r["type"] = "response";
    r["request_seq"] = request_seq;
    r["success"] = success;
    r["command"] = command;
    if (!body.empty()) r["body"] = body;
    connection_.Send(r.dump());
}

void DapServer::SendErrorResponse(int request_seq, const std::string& command,
                                   const std::string& message) {
    json r;
    { std::lock_guard<std::mutex> lk(seq_mutex_); r["seq"] = seq_counter_++; }
    r["type"] = "response";
    r["request_seq"] = request_seq;
    r["success"] = false;
    r["command"] = command;
    r["message"] = message;
    connection_.Send(r.dump());
}

void DapServer::SendEvent(const std::string& event, const json& body) {
    json e;
    { std::lock_guard<std::mutex> lk(seq_mutex_); e["seq"] = seq_counter_++; }
    e["type"] = "event";
    e["event"] = event;
    if (!body.empty()) e["body"] = body;
    connection_.Send(e.dump());
}

void DapServer::SendStoppedEvent(const std::string& reason, int thread_id) {
    json b;
    b["reason"] = reason;
    b["threadId"] = thread_id;
    b["allThreadsStopped"] = true;
    SendEvent("stopped", b);
}

void DapServer::SendOutputEvent(const std::string& text,
                                 const std::string& category) {
    json b;
    b["category"] = category;
    // stdout category comes from VM print() which already includes newline
    if (category == "stdout") {
        b["output"] = text;
    } else {
        b["output"] = text + "\n";
    }
    SendEvent("output", b);
}

void DapServer::SendTerminatedEvent() {
    SendEvent("terminated");
}

std::string DapServer::GetValueTypeName(const Value& val) {
    if (val.is_null())  return "nil";
    if (val.is_bool())  return "Bool";
    if (val.is_int())   return "Int";
    if (val.is_float()) return "Float";
    if (val.is_object() && val.as_object())
        return object_type_name(val.as_object()->type);
    return "Unknown";
}

} // namespace swive