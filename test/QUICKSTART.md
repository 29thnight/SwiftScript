## 빠른 시작 가이드

### 1. 빌드 (Windows)

```cmd
cd test
build.bat
```

### 2. 테스트 실행

**방법 A: 수동 테스트 러너**
```cmd
cd build\Release
swiftscript_tests.exe
```

**방법 B: Google Test**
```cmd
cd build\Release
swiftscript_gtest.exe
```

**방법 C: CTest**
```cmd
cd build
ctest -C Release --output-on-failure
```

### 3. 빌드 (Linux/macOS)

```bash
cd test
chmod +x build.sh
./build.sh

# 실행
cd build
./swiftscript_tests    # 수동
./swiftscript_gtest    # Google Test
ctest --output-on-failure  # CTest
```

## 주요 변경사항

### ? 해결된 문제
- **Main 함수 중복**: 이제 각 테스트 파일에 main이 없습니다
- **빌드 에러**: 모든 테스트를 한 번에 빌드 가능
- **테스트 실행**: 통합된 단일 실행 파일

### ?? 핵심 파일
- `test_main.cpp` → 여기에만 main 함수 존재 (수동)
- `test_gtest.cpp` → Google Test용 main 함수
- `test_*.cpp` → 테스트 함수만 포함 (main 없음)

### ?? Visual Studio 사용자

**프로젝트 구성:**
1. 새 콘솔 프로젝트 생성
2. 다음 중 하나만 추가:
   - `test_main.cpp` (수동 실행용)
   - `test_gtest.cpp` (Google Test용)
3. 나머지 파일 추가:
   - `test_class.cpp`
   - `test_struct.cpp`
   - `test_closure.cpp`
   - `test_switch.cpp`
   - `test_new_features.cpp`
   - `src/*.cpp` (모든 소스)

?? **주의**: `test_main.cpp`와 `test_gtest.cpp`를 동시에 추가하면 안 됩니다!

## 테스트 추가 예제

### 1. 테스트 함수 작성

```cpp
// test_myfeature.cpp
#include "test_helpers.hpp"
#include "ss_vm.hpp"

std::string run_code(const std::string& source) {
    // 표준 코드 실행 헬퍼
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize_all();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Compiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VMConfig config;
        config.enable_debug = false;
        VM vm(config);
        
        std::ostringstream output;
        std::streambuf* old = std::cout.rdbuf(output.rdbuf());
        struct Restore { std::streambuf* old; ~Restore(){ std::cout.rdbuf(old); } } restore{old};
        
        vm.execute(chunk);
        return output.str();
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

void test_my_feature() {
    std::string source = R"(
        var x = 42
        print(x)
    )";
    
    auto output = run_code(source);
    AssertHelper::assert_no_error(output);
    AssertHelper::assert_contains(output, "42", "Should print 42");
}
```

### 2. test_main.cpp에 등록

```cpp
// test_main.cpp에 추가
void test_my_feature(); // 선언

int main() {
    // ...
    runner.run_test("my feature test", test_my_feature);
    // ...
}
```

### 3. test_gtest.cpp에 등록

```cpp
// test_gtest.cpp에 추가
void test_my_feature(); // 선언

TEST(MyFeatureTests, BasicTest) {
    EXPECT_NO_THROW(test_my_feature());
}
```

### 4. CMakeLists.txt 업데이트

```cmake
# 새 파일 추가
add_executable(swiftscript_tests
    test_main.cpp
    test_myfeature.cpp  # 추가
    # ... 기타
)
```

## FAQ

### Q: 왜 main이 두 개인가요?
A: 
- `test_main.cpp`: 간단한 수동 실행용
- `test_gtest.cpp`: Google Test 프레임워크용
- 필요에 따라 하나만 사용하면 됩니다

### Q: 기존 테스트 코드가 깨지나요?
A: 
- 아니요! 테스트 함수 자체는 변경 없음
- Main 함수만 별도 파일로 분리됨

### Q: Visual Studio에서 빌드 에러가 나요
A:
1. `test_main.cpp`와 `test_gtest.cpp`를 동시에 포함했는지 확인
2. 하나만 포함해야 합니다
3. 또는 CMake 사용 권장

### Q: Google Test 없이도 되나요?
A:
- 네! `test_main.cpp`만 사용하면 됩니다
- Google Test는 선택사항입니다

### Q: 특정 테스트만 실행하려면?
A:
- 수동: 함수 호출 주석 처리
- Google Test: `--gtest_filter` 사용
  ```bash
  ./swiftscript_gtest --gtest_filter=ClassTests.*
  ```

## 유용한 명령어

### Google Test 필터링
```bash
# 특정 스위트
./swiftscript_gtest --gtest_filter=ClassTests.*

# 특정 테스트
./swiftscript_gtest --gtest_filter=ClassTests.SimpleMethod

# 여러 패턴
./swiftscript_gtest --gtest_filter=ClassTests.*:StructTests.*

# 제외
./swiftscript_gtest --gtest_filter=-*Slow*
```

### 반복 실행
```bash
./swiftscript_gtest --gtest_repeat=10
./swiftscript_gtest --gtest_repeat=100 --gtest_break_on_failure
```

### 상세 출력
```bash
./swiftscript_gtest --gtest_print_time=1
ctest --verbose
```

## 문제 해결

### 링커 에러: "multiple definition of main"
→ `test_main.cpp`와 `test_gtest.cpp` 둘 다 포함했는지 확인. 하나만 포함할 것.

### CMake가 Google Test를 찾지 못함
→ 인터넷 연결 확인. CMake가 자동으로 다운로드합니다.

### 테스트가 실패함
→ `--output-on-failure` 옵션으로 상세 정보 확인
```bash
ctest --output-on-failure
./swiftscript_gtest --gtest_print_time=1
```

## 다음 단계

1. ? 기본 빌드 및 실행 완료
2. ?? [README_TESTS.md](README_TESTS.md) 읽기
3. ?? 새 테스트 추가해보기
4. ?? CI/CD 통합 고려

## 요약

```
이전: test_class.cpp, test_struct.cpp 등 각각 main() → 빌드 에러
이후: test_main.cpp 하나에만 main() → 모든 테스트 통합 실행 ?
추가: test_gtest.cpp로 Google Test 지원 ?
```

간단합니다!
