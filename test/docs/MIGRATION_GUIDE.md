# SwiftScript 테스트 시스템 개선 요약

## 변경 사항

### 1. Main 함수 중복 문제 해결

**이전:**
- 각 테스트 파일(`test_class.cpp`, `test_struct.cpp` 등)마다 `main()` 함수가 존재
- 여러 파일을 동시에 빌드하면 링커 에러 발생

**이후:**
- 각 테스트 파일에서 `main()` 함수 제거
- 테스트 함수만 남김 (예: `test_simple_class_method()`)
- 단일 진입점 파일 생성:
  - `test_main.cpp`: 모든 테스트를 수동으로 실행하는 통합 main
  - `test_gtest.cpp`: Google Test 프레임워크 통합 main

### 2. 파일 구조

```
test/
├── test_main.cpp           # 새로운 통합 main (수동 실행)
├── test_gtest.cpp          # 새로운 Google Test main
├── test_class.cpp          # main 제거됨 ?
├── test_struct.cpp         # main 제거됨 ?
├── test_closure.cpp        # main 제거됨 ?
├── test_switch.cpp         # main 제거됨 ?
├── test_new_features.cpp   # main → main_new_features()로 변경
├── test_helpers.hpp        # 기존 헬퍼 유지
├── test_all_tests.hpp      # 새로운 테스트 선언 헤더
├── CMakeLists.txt          # 새로운 빌드 설정
├── build.bat               # Windows 빌드 스크립트
├── build.sh                # Linux/Mac 빌드 스크립트
└── README_TESTS.md         # 상세 사용법
```

### 3. Google Test 통합

**Google Test 기능:**
- 테스트 자동 발견 및 실행
- 상세한 테스트 결과 보고
- 필터링 및 병렬 실행 지원
- CI/CD 통합 용이

**테스트 케이스 예:**
```cpp
TEST(ClassTests, SimpleClassMethod) {
    EXPECT_NO_THROW(test_simple_class_method());
}
```

### 4. 빌드 방법

#### CMake 사용 (권장)
```bash
cd test
mkdir build && cd build
cmake ..
cmake --build .

# 실행
./swiftscript_tests      # 수동 실행
./swiftscript_gtest      # Google Test
ctest --output-on-failure # CTest
```

#### 빌드 스크립트 사용
```bash
# Windows
build.bat

# Linux/Mac
chmod +x build.sh
./build.sh
```

## 테스트 추가 방법

### 1단계: 테스트 함수 작성
```cpp
// test_my_feature.cpp
void test_my_new_feature() {
    auto out = run_code("...");
    AssertHelper::assert_no_error(out);
}
```

### 2단계: 헤더에 선언
```cpp
// test_all_tests.hpp
void test_my_new_feature();
```

### 3단계: Main에 등록
```cpp
// test_main.cpp
runner.run_test("my feature", test_my_new_feature);
```

### 4단계: Google Test에 등록
```cpp
// test_gtest.cpp
TEST(MyTests, NewFeature) {
    EXPECT_NO_THROW(test_my_new_feature());
}
```

## 장점

### 1. 컴파일 에러 해결
- ? Main 함수 중복 없음
- ? 모든 테스트를 한 번에 빌드 가능
- ? 선택적으로 개별 테스트 실행 가능

### 2. 테스트 관리 개선
- ? 통합된 테스트 실행
- ? 체계적인 테스트 조직
- ? 쉬운 테스트 추가/제거

### 3. Google Test 통합
- ? 산업 표준 테스트 프레임워크
- ? 풍부한 assertion 기능
- ? CI/CD 통합 지원
- ? 테스트 필터링 및 반복 실행

### 4. 확장성
- ? 새로운 테스트 쉽게 추가
- ? 테스트 스위트 분리 가능
- ? Mock/Stub 지원 (Google Mock)

## 사용 예제

### 수동 테스트 실행
```bash
$ ./swiftscript_tests

======================================
  SwiftScript Test Suite
======================================

--- Class Tests ---
[PASS] simple class method
[PASS] initializer is invoked
...

--- Struct Tests ---
[PASS] basic struct declaration
...

Summary: 45/45 tests passed
??? ALL TESTS PASSED! ???
```

### Google Test 실행
```bash
$ ./swiftscript_gtest

[==========] Running 45 tests from 4 test suites.
[----------] 11 tests from ClassTests
[ RUN      ] ClassTests.SimpleClassMethod
[       OK ] ClassTests.SimpleClassMethod (12 ms)
...
[==========] 45 tests from 4 test suites ran. (234 ms total)
[  PASSED  ] 45 tests.
```

### 특정 테스트만 실행
```bash
$ ./swiftscript_gtest --gtest_filter=ClassTests.*
$ ./swiftscript_gtest --gtest_filter=*Closure*
```

### 반복 실행 (안정성 테스트)
```bash
$ ./swiftscript_gtest --gtest_repeat=100
```

## 마이그레이션 가이드

기존 코드를 수정할 필요 없이:
1. `test_main.cpp` 또는 `test_gtest.cpp`를 빌드에 포함
2. 다른 test_*.cpp 파일들은 그대로 유지
3. CMakeLists.txt로 빌드하면 자동 처리

## 추가 개선 가능 사항

### 향후 고려사항
- [ ] Benchmark 테스트 (Google Benchmark)
- [ ] Code coverage 측정 (gcov/lcov)
- [ ] Sanitizer 통합 (ASan, UBSan)
- [ ] Continuous testing (watch mode)
- [ ] 테스트 데이터 파일 분리

## 참고 문서

- [README_TESTS.md](README_TESTS.md) - 상세 사용법
- [test_helpers.hpp](test_helpers.hpp) - 헬퍼 함수
- [Google Test Primer](https://google.github.io/googletest/primer.html)
