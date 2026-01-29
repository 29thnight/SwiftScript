#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

using namespace swiftscript;

namespace swiftscript {
namespace test {

// Helper to create temporary test files
void create_test_file(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to create test file: " + filename);
    }
    file << content;
    file.close();
}

void cleanup_test_file(const std::string& filename) {
    std::remove(filename.c_str());
}

std::string run_code_with_imports(const std::string& source, const std::string& base_dir = "") {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize_all();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Compiler compiler;
        if (!base_dir.empty()) {
            compiler.set_base_directory(base_dir);
        }
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

void test_simple_import() {
    std::cout << "Test: Simple import ... ";
    
    // Create helper module
    create_test_file("test_math.ss", R"(
        func add(a: Int, b: Int) -> Int {
            return a + b
        }
    )");
    
    std::string source = R"(
        import "test_math.ss"
        print(add(2, 3))
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_math.ss");
    
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "5", "add(2, 3) should print 5");
    std::cout << "PASSED\n";
}

void test_import_class() {
    std::cout << "Test: Import class ... ";
    
    // Create helper module with class
    create_test_file("test_greeter.ss", R"(
        class Greeter {
            func greet() {
                print("Hello from module!")
            }
        }
    )");
    
    std::string source = R"(
        import "test_greeter.ss"
        var g = Greeter()
        g.greet()
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_greeter.ss");
    
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "Hello from module!", "Should call imported class method");
    std::cout << "PASSED\n";
}

void test_import_multiple_functions() {
    std::cout << "Test: Import multiple functions ... ";
    
    create_test_file("test_utils.ss", R"(
        func multiply(a: Int, b: Int) -> Int {
            return a * b
        }
        
        func divide(a: Int, b: Int) -> Int {
            return a / b
        }
    )");
    
    std::string source = R"(
        import "test_utils.ss"
        print(multiply(4, 5))
        print(divide(20, 4))
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_utils.ss");
    
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains_all(out, {"20", "5"});
    std::cout << "PASSED\n";
}

void test_nested_import() {
    std::cout << "Test: Nested import (A imports B, B imports C) ... ";
    
    // Create module C
    create_test_file("test_c.ss", R"(
        func c_func() {
            print("C")
        }
    )");
    
    // Create module B that imports C
    create_test_file("test_b.ss", R"(
        import "test_c.ss"
        func b_func() {
            print("B")
            c_func()
        }
    )");
    
    // Main imports B
    std::string source = R"(
        import "test_b.ss"
        print("A")
        b_func()
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_c.ss");
    cleanup_test_file("test_b.ss");
    
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_order(out, "A", "B", "Should print A then B");
    AssertHelper::assert_order(out, "B", "C", "Should print B then C");
    std::cout << "PASSED\n";
}

void test_duplicate_import() {
    std::cout << "Test: Duplicate import (should import only once) ... ";
    
    create_test_file("test_counter.ss", R"(
        var counter: Int = 0
        func increment() {
            counter = counter + 1
            print(counter)
        }
    )");
    
    std::string source = R"(
        import "test_counter.ss"
        import "test_counter.ss"
        increment()
        increment()
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_counter.ss");
    
    AssertHelper::assert_no_error(out);
    // Should print 1, 2 (not 0, 1, 0, 1 if imported twice)
    AssertHelper::assert_order(out, "1", "2", "Counter should increment properly");
    std::cout << "PASSED\n";
}

void test_circular_import_detection() {
    std::cout << "Test: Circular import detection ... ";
    
    // Create module A that imports B
    create_test_file("test_a.ss", R"(
        import "test_b.ss"
        func a_func() {
            print("A")
        }
    )");
    
    // Create module B that imports A (circular)
    create_test_file("test_b.ss", R"(
        import "test_a.ss"
        func b_func() {
            print("B")
        }
    )");
    
    std::string source = R"(
        import "test_a.ss"
        a_func()
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_a.ss");
    cleanup_test_file("test_b.ss");
    
    AssertHelper::assert_error(out, "Should detect circular import");
    std::cout << "PASSED\n";
}

void test_import_nonexistent_file() {
    std::cout << "Test: Import nonexistent file ... ";
    
    std::string source = R"(
        import "nonexistent_module.ss"
        print("Should not reach here")
    )";
    
    auto out = run_code_with_imports(source, ".");
    AssertHelper::assert_error(out, "Should error on nonexistent file");
    std::cout << "PASSED\n";
}

void test_import_with_variables() {
    std::cout << "Test: Import with global variables ... ";
    
    create_test_file("test_config.ss", R"(
        var app_name = "MyApp"
        var version: Int = 1
        
        func show_info() {
            print(app_name)
            print(version)
        }
    )");
    
    std::string source = R"(
        import "test_config.ss"
        show_info()
    )";
    
    auto out = run_code_with_imports(source, ".");
    cleanup_test_file("test_config.ss");
    
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains_all(out, {"MyApp", "1"});
    std::cout << "PASSED\n";
}

} // namespace test
} // namespace swiftscript

