cmake_minimum_required(VERSION 3.14)
project(SwiftScriptTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Include FetchContent for downloading Google Test
include(FetchContent)

# Fetch Google Test
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Source files
set(SWIFTSCRIPT_SOURCES
    ../src/ss_chunk.cpp
    ../src/ss_compiler.cpp
    ../src/ss_lexer.cpp
    ../src/ss_parser.cpp
    ../src/ss_token.cpp
    ../src/ss_value.cpp
    ../src/ss_vm.cpp
)

# Include directories
include_directories(
    ../include
    .
)

# Manual test executable (single main)
add_executable(swiftscript_tests
    test_main.cpp
    test_class.cpp
    test_struct.cpp
    test_closure.cpp
    test_switch.cpp
    test_new_features.cpp
    ${SWIFTSCRIPT_SOURCES}
)

# Google Test executable
add_executable(swiftscript_gtest
    test_gtest.cpp
    test_class.cpp
    test_struct.cpp
    test_closure.cpp
    test_switch.cpp
    test_enum.cpp
    ${SWIFTSCRIPT_SOURCES}
)

# Link Google Test to the Google Test executable
target_link_libraries(swiftscript_gtest
    GTest::gtest_main
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(swiftscript_gtest)

# Optional: Add manual test as CTest
add_test(NAME ManualTests COMMAND swiftscript_tests)
