# SwiftScript Test Suite

이 디렉토리는 SwiftScript의 테스트 스위트를 포함합니다. 두 가지 방식으로 테스트를 실행할 수 있습니다:

1. **단일 main 함수를 사용한 수동 테스트 실행**
2. **Google Test 프레임워크를 사용한 자동화 테스트**

## 파일 구조

```
test/
├── test_main.cpp          # 단일 main 함수 (모든 테스트 통합)
├── test_gtest.cpp         # Google Test 통합 파일
├── test_class.cpp         # 클래스 테스트 함수들
├── test_struct.cpp        # 구조체 테스트 함수들
├── test_closure.cpp       # 클로저 테스트 함수들
├── test_switch.cpp        # switch 문 테스트 함수들
├── test_new_features.cpp  # 신규 기능 종합 테스트
├── test_helpers.hpp       # 테스트 헬퍼 유틸리티
├── test_all_tests.hpp     # 테스트 함수 선언 모음
├── CMakeLists.txt         # CMake 빌드 설정
└── README_TESTS.md        # 이 문서
```

## 빌드 및 실행 방법

### 방법 1: CMake 사용 (권장)

#### 빌드

```bash
cd test
mkdir build
cd build
cmake ..
cmake --build .
```

#### 실행

**단일 테스트 실행:**
```bash
./swiftscript_tests
```

**Google Test 실행:**
```bash
./swiftscript_gtest
```

**CTest로 모든 테스트 실행:**
```bash
ctest --output-on-failure
```

### 방법 2: Visual Studio에서 직접 빌드

#### 프로젝트 설정

1. Visual Studio에서 솔루션 열기
2. 테스트 프로젝트 설정:
   - `test_main.cpp`를 포함하는 실행 프로젝트 생성
   - 또는 `test_gtest.cpp`를 포함하는 Google Test 프로젝트 생성
3. 다음 파일들을 프로젝트에 추가:
   - `test_class.cpp`
   - `test_struct.cpp`
   - `test_closure.cpp`
   - `test_switch.cpp`
   - `test_new_features.cpp`
   - 모든 `src/*.cpp` 파일

#### 주의사항

?? **각 테스트 cpp 파일에는 이제 main 함수가 없습니다!**
- `test_main.cpp` 또는 `test_gtest.cpp` 중 하나만 빌드에 포함해야 합니다.
- 둘 다 포함하면 main 함수가 중복되어 링커 에러가 발생합니다.

## Google Test 설치

### Windows (vcpkg)

```bash
vcpkg install gtest:x64-windows
```

### Linux/macOS

CMakeLists.txt가 자동으로 Google Test를 다운로드하고 빌드합니다.

또는 패키지 매니저 사용:

**Ubuntu/Debian:**
```bash
sudo apt-get install libgtest-dev
```

**macOS (Homebrew):**
```bash
brew install googletest
```

## 테스트 추가하기

### 새로운 테스트 함수 추가

1. **테스트 함수 작성** (예: `test_my_feature.cpp`):

```cpp
#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"

using namespace swiftscript;
using namespace swiftscript::test;

std::string run_code(const std::string& source) {
    // ... (다른 테스트 파일의 run_code 참고)
}

void test_my_new_feature() {
    std::string source = R"(
        // SwiftScript 코드
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "expected", "설명");
}
```

2. **test_all_tests.hpp에 선언 추가**:

```cpp
void test_my_new_feature();
```

3. **test_main.cpp에 테스트 등록**:

```cpp
runner.run_test("my new feature", test_my_new_feature);
```

4. **test_gtest.cpp에 Google Test 케이스 추가**:

```cpp
TEST(MyFeatureTests, NewFeature) {
    EXPECT_NO_THROW(test_my_new_feature());
}
```

## 테스트 헬퍼 사용법

### AssertHelper

```cpp
// 에러 확인
AssertHelper::assert_error(output, "설명");
AssertHelper::assert_no_error(output);

// 문자열 포함 확인
AssertHelper::assert_contains(output, "expected_text", "설명");

// 정확한 매칭
AssertHelper::assert_equals(output, "expected", "설명");
```

### TestRunner

```cpp
TestRunner runner;
runner.run_test("테스트 이름", test_function);
runner.print_summary();
bool success = runner.all_passed();
```

### MemoryTracker

```cpp
auto& tracker = MemoryTracker::instance();
tracker.start_tracking();
// ... 테스트 코드 ...
tracker.print_memory_stats();
tracker.print_leak_report();
```

## 테스트 스위트별 설명

### ClassTests
- 클래스 선언, 메서드, 프로퍼티
- 상속, override, super
- init/deinit 생명주기

### StructTests
- 구조체 선언, memberwise init
- mutating/non-mutating 메서드
- 값 타입 의미론 (value semantics)

### ClosureTests
- 클로저 문법, 캡처
- 함수 인자/반환 값으로서의 클로저
- 중첩 클로저

### SwitchTests
- switch 문 기본 동작
- 범위 패턴, 다중 패턴
- default 케이스

### NewFeatureTests
- 종합적인 신규 기능 테스트
- 배열, 딕셔너리
- 연산자, 제어 흐름

## 트러블슈팅

### "multiple definition of main" 에러

하나의 빌드에서 `test_main.cpp`와 `test_gtest.cpp`를 동시에 포함하지 마세요.

### Google Test를 찾을 수 없음

CMake가 자동으로 다운로드하도록 설정되어 있습니다. 인터넷 연결을 확인하세요.

### 링커 에러

모든 SwiftScript 소스 파일(`src/*.cpp`)이 빌드에 포함되었는지 확인하세요.

## CI/CD 통합

### GitHub Actions 예제

```yaml
name: Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build
        run: |
          cd test
          mkdir build && cd build
          cmake ..
          cmake --build .
      - name: Run tests
        run: |
          cd test/build
          ctest --output-on-failure
```

## 추가 리소스

- [Google Test 문서](https://google.github.io/googletest/)
- [CMake 문서](https://cmake.org/documentation/)
- [test_helpers.hpp](test_helpers.hpp) - 헬퍼 함수 상세 설명
