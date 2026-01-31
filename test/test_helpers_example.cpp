// Example usage of DebugHelper utilities for SwiftScript testing
// This demonstrates all the features of the test framework
#include "pch_.h"
#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"

using namespace swiftscript;
using namespace swiftscript::test;

std::string run_code(const std::string& source) {
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

// Example 1: Using AssertHelper for better error messages
void test_assert_helper_example() {
    std::string source = R"(
        print("Hello")
        print("World")
    )";
    auto out = run_code(source);
    
    // Better than: assert(out.find("Hello") != std::string::npos);
    AssertHelper::assert_contains(out, "Hello", "Should print Hello");
    AssertHelper::assert_contains(out, "World", "Should print World");
    
    // Verify order
    AssertHelper::assert_order(out, "Hello", "World", "Hello should come before World");
    
    // Verify no errors
    AssertHelper::assert_no_error(out);
}

// Example 2: Using OutputMatcher for exact output verification
void test_output_matcher_example() {
    std::string source = R"(
        print("Line 1")
        print("Line 2")
        print("Line 3")
    )";
    auto out = run_code(source);
    
    // Exact line-by-line matching
    OutputMatcher::assert_exact_output(out, {
        "Line 1",
        "Line 2",
        "Line 3"
    });
    
    // Verify all required strings are present
    OutputMatcher::assert_contains_all(out, {"Line 1", "Line 2", "Line 3"});
    
    // Verify order
    OutputMatcher::assert_output_order(out, {"Line 1", "Line 2", "Line 3"});
}

// Example 3: Using PerformanceProfiler
void test_performance_profiling() {
    std::string source = R"(
        var sum = 0
        for i in 0..<1000 {
            sum = sum + i
        }
        print(sum)
    )";
    
    {
        PerformanceProfiler profiler("Loop execution");
        profiler.start();
        auto out = run_code(source);
        profiler.stop_and_print();  // Automatically prints timing
        
        AssertHelper::assert_contains(out, "499500");
    }
}

// Example 4: Using TestRunner for automatic test management
void example_test_1() {
    auto out = run_code("print(1 + 1)");
    AssertHelper::assert_contains(out, "2");
}

void example_test_2() {
    auto out = run_code("print(\"test\")");
    AssertHelper::assert_contains(out, "test");
}

void example_failing_test() {
    auto out = run_code("print(\"wrong\")");
    AssertHelper::assert_contains(out, "correct");  // This will fail
}

// Example 5: Memory tracking (conceptual - needs VM integration)
void test_memory_tracking_example() {
    // Enable memory tracking for this scope
    MemoryTrackingScope scope("Class instantiation test");
    
    std::string source = R"(
        class MyClass {
            var value: Int = 42
        }
        var obj = MyClass()
        print(obj.value)
    )";
    
    auto out = run_code(source);
    AssertHelper::assert_contains(out, "42");
    
    // Memory tracking will automatically check for leaks at end of scope
}

// Example 6: Complex test scenario with multiple checks
void test_complex_scenario() {
    std::string source = R"(
        class Calculator {
            var result: Int = 0
            
            func add(a: Int, b: Int) -> Int {
                result = a + b
                return result
            }
            
            func multiply(a: Int, b: Int) -> Int {
                result = a * b
                return result
            }
        }
        
        var calc = Calculator()
        print("Initial: ")
        print(calc.result)
        
        print("Add 5 + 3: ")
        print(calc.add(5, 3))
        
        print("Multiply 4 * 6: ")
        print(calc.multiply(4, 6))
    )";
    
    auto out = run_code(source);
    
    // Multiple assertions with context
    AssertHelper::assert_no_error(out, "Calculator code should run without errors");
    AssertHelper::assert_contains(out, "Initial:", "Should print label");
    AssertHelper::assert_contains(out, "0", "Initial value should be 0");
    AssertHelper::assert_contains(out, "8", "5 + 3 should equal 8");
    AssertHelper::assert_contains(out, "24", "4 * 6 should equal 24");
    
    // Verify execution order
    OutputMatcher::assert_output_order(out, {
        "Initial:",
        "0",
        "Add 5 + 3:",
        "8",
        "Multiply 4 * 6:",
        "24"
    });
}

// Example 7: Error testing with better assertions
void test_error_handling_example() {
    std::string source = R"(
        class Animal {
            func speak() {
                print("generic")
            }
        }
        class Dog: Animal {
            func speak() {  // Missing override keyword
                print("bark")
            }
        }
    )";
    
    auto out = run_code(source);
    
    // Check that error is reported
    AssertHelper::assert_error(out, "Should error on missing override");
    
    // Could also check for specific error message
    AssertHelper::assert_contains(out, "override", "Error should mention 'override'");
}

// Example 8: Comparing values
void test_value_comparison() {
    std::string source = R"(
        var x = 10
        var y = 20
        print(x)
        print(y)
    )";
    
    auto out = run_code(source);
    auto lines = OutputMatcher::split_lines(out);
    
    AssertHelper::assert_equals(size_t(2), lines.size(), "Should have 2 lines of output");
    AssertHelper::assert_equals(std::string("10"), lines[0], "First line should be 10");
    AssertHelper::assert_equals(std::string("20"), lines[1], "Second line should be 20");
}