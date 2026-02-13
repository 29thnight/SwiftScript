# SwiftScript DAP (Debug Adapter Protocol) 구현 계획

## 1. 현재 상태 분석

### 이미 있는 인프라
| 구성 요소 | 상태 | 위치 |
|-----------|------|------|
| `DebugController` forward decl | ? 선언만 | `ss_vm.hpp:22` |
| VM debug hook (`on_instruction`) | ? 호출 코드 있음 | `ss_vm.cpp:run()` |
| `VM::attach_debugger()` | ? 구현됨 | `ss_vm.hpp:117-118` |
| VM 상태 접근자 | ? 구현됨 | `ss_vm.hpp:122-127` (`call_frames()`, `stack()`, `current_ip()` 등) |
| `MethodBody::line_info` | ? IP→소스 라인 매핑 | `ss_chunk.hpp` |
| `DebugInfo` (locals) | ? 구조체 정의됨 | `ss_chunk.hpp` |
| `DebugLocalInfo` | ? 슬롯/스코프/이름 | `ss_chunk.hpp` |
| LSP 서버 (참고용) | ? Content-Length stdio | `lsp_connection.hpp/cpp` |
| VS Code 확장 | ? LSP + 터미널 실행 | `extension.ts`, `package.json` |
| `DebugController` 클래스 정의 | ? 없음 | - |
| DAP 서버 | ? 없음 | - |
| VS Code debugger 설정 | ? 없음 | `package.json` |

### 아키텍처 개요

```
┌──────────────────┐      stdin/stdout       ┌─────────────────────────┐
│   VS Code IDE    │ ?──── DAP (JSON) ─────? │  SwiveDebugAdapter.exe  │
│  (debug client)  │                         │                         │
│                  │                         │  ┌───────────────────┐  │
│  - breakpoints   │                         │  │   DapServer       │  │
│  - step/continue │                         │  │   (JSON parser)   │  │
│  - variables     │                         │  ├───────────────────┤  │
│  - call stack    │                         │  │ DebugController   │  │
│  - watch         │                         │  │ (VM 제어/검사)     │  │
│                  │                         │  ├───────────────────┤  │
│                  │                         │  │   VM + Compiler   │  │
│                  │                         │  │   (실행 엔진)      │  │
│                  │                         │  └───────────────────┘  │
└──────────────────┘                         └─────────────────────────┘
```

## 2. 구현 파일 목록

### C++ (src/debugger/)
| 파일 | 역할 |
|------|------|
| `ss_debug_controller.hpp` | DebugController 클래스 정의 (breakpoint, step, pause) |
| `ss_debug_controller.cpp` | DebugController 구현 |
| `dap_server.hpp` | DAP JSON 메시지 처리, 이벤트 발행 |
| `dap_server.cpp` | DAP 서버 구현 |
| `dap_main.cpp` | SwiveDebugAdapter.exe 진입점 |
| `dap_connection.hpp` | Content-Length stdio 통신 (LSP 코드 재사용) |
| `dap_connection.cpp` | 통신 구현 |

### VS Code 확장 수정
| 파일 | 변경 |
|------|------|
| `package.json` | `contributes.debuggers` 섹션 추가 |
| `src/extension.ts` | `DebugAdapterDescriptorFactory` 등록 |

### 빌드
| 파일 | 역할 |
|------|------|
| `vcproj/debugger/SwiveDebugAdapter.vcxproj` | 디버그 어댑터 프로젝트 |

## 3. 핵심 클래스 설계

### 3.1 DebugController

```cpp
// ss_debug_controller.hpp
#pragma once
#include "ss_vm.hpp"
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace swive {

enum class StepMode {
    Run,        // 자유 실행
    StepOver,   // 현재 라인 다음 라인까지
    StepInto,   // 다음 명령 (함수 진입)
    StepOut,    // 현재 프레임 리턴까지
    Pause       // 일시정지 요청됨
};

struct Breakpoint {
    int id;
    std::string source_path;  // 정규화된 절대 경로
    int line;                 // 1-based
    bool verified{false};
};

class DebugController {
public:
    // VM이 매 명령마다 호출. true면 일시정지됨.
    bool on_instruction(VM& vm, size_t ip,
                        body_idx body, const MethodBody* method);

    // 외부(DAP 스레드)에서 호출
    void set_breakpoints(const std::string& source,
                         const std::vector<int>& lines);
    void continue_execution();
    void step_over();
    void step_into();
    void step_out();
    void pause();

    // 일시정지 콜백 (DAP가 등록)
    using PauseCallback = std::function<void(
        const std::string& reason,  // "breakpoint" | "step" | "pause"
        int thread_id
    )>;
    void set_pause_callback(PauseCallback cb);

    // 상태 조회 (일시정지 상태에서만 안전)
    struct FrameInfo {
        int id;
        std::string name;
        std::string source;
        int line;
    };
    std::vector<FrameInfo> get_stack_frames(const VM& vm) const;

    struct VariableInfo {
        std::string name;
        std::string value;
        std::string type;
        int variables_reference{0};  // 0 = leaf
    };
    std::vector<VariableInfo> get_variables(const VM& vm,
                                            int frame_id) const;
    std::vector<VariableInfo> get_object_children(const VM& vm,
                                                  int ref_id) const;

private:
    std::mutex mutex_;
    std::condition_variable resume_cv_;
    bool paused_{false};

    StepMode step_mode_{StepMode::Run};
    int step_frame_depth_{0};     // StepOver/StepOut 기준 프레임 깊이
    int step_line_{-1};           // StepOver 시 현재 라인

    std::unordered_map<std::string,
        std::unordered_set<int>> breakpoints_;  // source → {lines}
    int next_bp_id_{1};

    PauseCallback pause_callback_;

    // IP → 소스 라인 변환
    int resolve_line(const MethodBody* method, size_t ip) const;

    // 현재 소스 파일 추출
    std::string resolve_source(const MethodBody* method) const;

    // 일시정지 진입 (mutex 보유 상태에서 호출)
    void enter_pause(const std::string& reason);
};

} // namespace swive
```

### 3.2 DapServer

```cpp
// dap_server.hpp - 핵심 메시지 타입
class DapServer {
public:
    void run();  // stdin 메시지 루프

private:
    // DAP Request 핸들러들
    void handle_initialize(int seq, const json& args);
    void handle_launch(int seq, const json& args);
    void handle_set_breakpoints(int seq, const json& args);
    void handle_threads(int seq, const json& args);
    void handle_stack_trace(int seq, const json& args);
    void handle_scopes(int seq, const json& args);
    void handle_variables(int seq, const json& args);
    void handle_continue(int seq, const json& args);
    void handle_next(int seq, const json& args);       // step over
    void handle_step_in(int seq, const json& args);
    void handle_step_out(int seq, const json& args);
    void handle_pause(int seq, const json& args);
    void handle_evaluate(int seq, const json& args);
    void handle_disconnect(int seq, const json& args);

    // DAP Event 발행
    void send_event(const std::string& event, const json& body);
    void send_stopped_event(const std::string& reason, int thread_id);
    void send_output_event(const std::string& text, const std::string& category);
    void send_terminated_event();

    // VM 실행 스레드 (별도 스레드에서 실행)
    std::thread vm_thread_;
    VM vm_;
    DebugController controller_;
    DapConnection connection_;
};
```

## 4. DAP 프로토콜 메시지 흐름

### 4.1 초기화 시퀀스
```
VS Code                          SwiveDebugAdapter
  │                                      │
  │─── initialize ──────────────────────?│  capabilities 응답
  │?── response ─────────────────────────│
  │                                      │
  │─── launch { program, ... } ────────?│  컴파일 + VM 생성
  │?── response ─────────────────────────│
  │?── initialized event ───────────────│
  │                                      │
  │─── setBreakpoints ─────────────────?│  브레이크포인트 설정
  │?── response (verified) ─────────────│
  │                                      │
  │─── configurationDone ──────────────?│  VM 실행 시작 (별도 스레드)
  │?── response ─────────────────────────│
```

### 4.2 중단점 히트
```
  │                          VM 스레드: on_instruction()에서 중단
  │?── stopped event ───────────────────│  reason: "breakpoint"
  │                                      │
  │─── threads ────────────────────────?│
  │?── response ─────────────────────────│
  │                                      │
  │─── stackTrace ─────────────────────?│
  │?── response (frames[]) ─────────────│
  │                                      │
  │─── scopes { frameId } ────────────?│
  │?── response (Local, Global) ────────│
  │                                      │
  │─── variables { ref } ─────────────?│
  │?── response (name, value, type) ────│
```

### 4.3 Step / Continue
```
  │─── next (stepOver) ────────────────?│  step_mode_ = StepOver
  │?── response ─────────────────────────│  resume_cv_.notify()
  │                          VM 스레드 재개, 다음 라인에서 멈춤
  │?── stopped event ───────────────────│  reason: "step"
```

## 5. 핵심 구현 로직

### 5.1 on_instruction (매 바이트코드 명령)

```cpp
bool DebugController::on_instruction(VM& vm, size_t ip,
                                     body_idx body, const MethodBody* method) {
    std::unique_lock<std::mutex> lock(mutex_);

    int current_line = resolve_line(method, ip);
    std::string source = resolve_source(method);
    int frame_depth = static_cast<int>(vm.call_frames().size());

    bool should_pause = false;

    // 1. Pause 요청 확인
    if (step_mode_ == StepMode::Pause) {
        should_pause = true;
    }
    // 2. 브레이크포인트 확인
    else if (current_line > 0 && !source.empty()) {
        auto it = breakpoints_.find(source);
        if (it != breakpoints_.end() && it->second.count(current_line)) {
            should_pause = true;
        }
    }
    // 3. Step 모드 확인
    else if (step_mode_ == StepMode::StepInto) {
        if (current_line != step_line_) should_pause = true;
    }
    else if (step_mode_ == StepMode::StepOver) {
        if (frame_depth <= step_frame_depth_ && current_line != step_line_)
            should_pause = true;
    }
    else if (step_mode_ == StepMode::StepOut) {
        if (frame_depth < step_frame_depth_) should_pause = true;
    }

    if (should_pause) {
        enter_pause(step_mode_ == StepMode::Run ? "breakpoint" : "step");
        // 블록: DAP 스레드가 continue/step 호출할 때까지 대기
        resume_cv_.wait(lock, [this] { return !paused_; });
    }

    return should_pause;
}
```

### 5.2 IP → 소스 라인 매핑

```cpp
int DebugController::resolve_line(const MethodBody* method, size_t ip) const {
    if (!method || ip >= method->line_info.size()) return -1;
    return static_cast<int>(method->line_info[ip]);
}

std::string DebugController::resolve_source(const MethodBody* method) const {
    if (!method || !method->debug_info) return "";
    return method->debug_info->source_file;
}
```

### 5.3 변수 검사

```cpp
std::vector<VariableInfo> DebugController::get_variables(
    const VM& vm, int frame_id) const
{
    std::vector<VariableInfo> result;
    const auto& frames = vm.call_frames();
    const auto& stack = vm.stack();

    // frame_id로 프레임 찾기
    size_t frame_index = /* ... */;
    const CallFrame& frame = frames[frame_index];
    const MethodBody* body = /* resolve from frame.body_index */;

    if (body && body->debug_info) {
        for (const auto& local : body->debug_info->locals) {
            size_t slot = frame.stack_base + local.slot_index;
            if (slot < stack.size()) {
                const Value& val = stack[slot];
                result.push_back({
                    local.name,
                    val.to_string(),
                    local.type_name.empty() ? val.type_name() : local.type_name,
                    val.is_object() ? next_ref_id() : 0
                });
            }
        }
    }
    return result;
}
```

## 6. VS Code 확장 변경

### 6.1 package.json 추가

```json
{
  "contributes": {
    "debuggers": [
      {
        "type": "swive",
        "label": "Swive Debug",
        "languages": ["swive"],
        "configurationAttributes": {
          "launch": {
            "required": ["program"],
            "properties": {
              "program": {
                "type": "string",
                "description": "Path to .ssproject file"
              },
              "buildType": {
                "type": "string",
                "enum": ["Debug", "Release"],
                "default": "Debug"
              },
              "stopOnEntry": {
                "type": "boolean",
                "default": false
              }
            }
          }
        },
        "configurationSnippets": [
          {
            "label": "Swive: Launch",
            "body": {
              "type": "swive",
              "request": "launch",
              "name": "Debug Swive",
              "program": "^\"${workspaceFolder}/${1:Project}.ssproject\"",
              "buildType": "Debug"
            }
          }
        ]
      }
    ]
  }
}
```

### 6.2 extension.ts 추가

```typescript
class SwiveDebugAdapterFactory
    implements vscode.DebugAdapterDescriptorFactory
{
    constructor(private context: vscode.ExtensionContext) {}

    createDebugAdapterDescriptor(
        session: vscode.DebugSession
    ): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
        const bundled = this.context.asAbsolutePath(
            path.join("server", "SwiveDebugAdapter.exe")
        );
        return new vscode.DebugAdapterExecutable(bundled);
    }
}

// activate() 내부에 추가:
context.subscriptions.push(
    vscode.debug.registerDebugAdapterDescriptorFactory(
        "swive",
        new SwiveDebugAdapterFactory(context)
    )
);
```

## 7. 빌드 구성

### SwiveDebugAdapter.vcxproj

```
프로젝트 참조:
  - SwiveCore.vcxproj      (값/객체)
  - SwiveCompilerLib.vcxproj (컴파일러)
  - SwiveVMLib.vcxproj       (VM)
  - SwiveLib.vcxproj          (공통)

추가 포함 디렉터리:
  - $(SolutionDir)src\common\
  - $(SolutionDir)src\debugger\
  - $(SolutionDir)vcproj\core\
  - nlohmann/json (헤더 온리)

출력: SwiveDebugAdapter.exe
```

## 8. 의존성

- **nlohmann/json**: DAP JSON 파싱 (이미 SwiveServer에서 사용 중)
- **std::thread**: VM 실행 스레드
- **std::mutex/condition_variable**: DAP↔VM 동기화

## 9. 구현 순서 (단계별)

### Phase 1: DebugController (VM 레이어)
1. `ss_debug_controller.hpp/cpp` 작성
2. 브레이크포인트 설정/조회
3. Step 모드 (StepOver, StepInto, StepOut)
4. 변수 검사 (`get_variables`, `get_stack_frames`)
5. 컴파일러에서 `DebugInfo` 생성 (`source_file`, `locals` 채우기)

### Phase 2: DAP 서버
1. `dap_connection.hpp/cpp` (Content-Length stdio)
2. `dap_server.hpp/cpp` (JSON 메시지 디스패치)
3. `initialize` / `launch` / `disconnect`
4. `setBreakpoints` / `configurationDone`
5. `threads` / `stackTrace` / `scopes` / `variables`
6. `continue` / `next` / `stepIn` / `stepOut` / `pause`
7. `evaluate` (watch 표현식)
8. `dap_main.cpp` 진입점

### Phase 3: VS Code 확장
1. `package.json`에 `debuggers` 추가
2. `extension.ts`에 `DebugAdapterDescriptorFactory` 등록
3. `launch.json` 스니펫 제공

### Phase 4: 컴파일러 디버그 정보 강화
1. `Compiler`에서 `DebugInfo` 생성 로직 추가
2. 로컬 변수 이름/슬롯/스코프 범위 기록
3. 소스 파일 경로 기록
4. `Assembly::serialize/deserialize`에 `DebugInfo` 포함

## 10. launch.json 사용 예시

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "type": "swive",
            "request": "launch",
            "name": "Debug MyProject",
            "program": "${workspaceFolder}/MyProject.ssproject",
            "buildType": "Debug",
            "stopOnEntry": false
        }
    ]
}
```

## 11. 제약 사항 및 향후 과제

| 항목 | 현재 | 향후 |
|------|------|------|
| 스레드 | 싱글 스레드 (thread_id=1 고정) | 멀티 스레드/코루틴 |
| Conditional BP | 미지원 | `condition` 표현식 평가 |
| Logpoint | 미지원 | `logMessage` 지원 |
| Hot reload | 미지원 | 소스 변경 시 재컴파일 |
| Remote debug | 로컬만 | TCP 소켓 지원 |
| Exception BP | 미지원 | try/catch 연동 |
