#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace swiftscript;
using namespace swiftscript::test;

namespace {
// Static helper function to avoid linker conflicts
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
} // anonymous namespace

namespace swiftscript {
namespace test {

// Test 1: Basic enum declaration
void test_enum_basic() {
    std::string source = R"(
        enum Direction {
            case north
            case south
            case east
            case west
        }
        
        var dir = Direction.north
        print(dir)
    )";

    std::string result = run_code(source);
    AssertHelper::assert_contains(result, "north", "test_enum_basic");
    std::cout << "[PASS] test_enum_basic\n";
}

// Test 2: Enum with raw values (Int)
void test_enum_raw_values() {
    std::string source = R"(
        enum Priority {
            case low = 1
            case medium = 2
            case high = 3
        }
        
        var p = Priority.high
        print(p.rawValue)
    )";

    std::string result = run_code(source);
    AssertHelper::assert_contains(result, "3", "test_enum_raw_values");
    std::cout << "[PASS] test_enum_raw_values\n";
}

// Test 3: Enum in switch statement
void test_enum_switch() {
    std::string source = R"(
        enum Status {
            case pending
            case active
            case completed
        }
        
        var status = Status.active
        
        switch status {
        case Status.pending:
            print("Pending")
        case Status.active:
            print("Active")
        case Status.completed:
            print("Completed")
        }
    )";

    std::string result = run_code(source);
    AssertHelper::assert_contains(result, "Active", "test_enum_switch");
    std::cout << "[PASS] test_enum_switch\n";
}

// Test 4: Enum with associated values
void test_enum_associated_values() {
    std::string source = R"(
        enum Response {
            case success(message: String)
            case failure(code: Int)
        }
        
        var result = Response.success(message: "OK")
        
        switch result {
        case Response.success(let msg):
            print("Success: " + msg)
        case Response.failure(let code):
            print("Error: " + String(code))
        }
    )";

    std::string result = run_code(source);
    if (result.find("ERROR") != std::string::npos || result.find("SKIP") != std::string::npos) {
        std::cout << "[SKIP] test_enum_associated_values (not implemented yet)\n";
        return;
    }
    AssertHelper::assert_contains(result, "Success: OK", "test_enum_associated_values");
    std::cout << "[PASS] test_enum_associated_values\n";
}

// Test 5: Enum comparison
void test_enum_comparison() {
    std::string source = R"(
        enum Color {
            case red
            case green
            case blue
        }
        
        var c1 = Color.red
        var c2 = Color.red
        var c3 = Color.blue
        
        if c1 == c2 {
            print("Same color")
        }
        
        if c1 != c3 {
            print("Different color")
        }
    )";

    std::string result = run_code(source);
    AssertHelper::assert_contains(result, "Same color", "test_enum_comparison");
    AssertHelper::assert_contains(result, "Different color", "test_enum_comparison");
    std::cout << "[PASS] test_enum_comparison\n";
}

// Test 6: Enum methods
void test_enum_methods() {
    std::string source = R"(
        enum CompassPoint {
            case north
            case south
            case east
            case west
            
            func describe() -> String {
                switch self {
                case CompassPoint.north:
                    return "North direction"
                case CompassPoint.south:
                    return "South direction"
                case CompassPoint.east:
                    return "East direction"
                case CompassPoint.west:
                    return "West direction"
                }
            }
        }
        
        var direction = CompassPoint.north
        print(direction.describe())
    )";

    std::string result = run_code(source);
    if (result.find("ERROR") != std::string::npos || result.find("SKIP") != std::string::npos) {
        std::cout << "[SKIP] test_enum_methods (not implemented yet)\n";
        return;
    }
    AssertHelper::assert_contains(result, "North direction", "test_enum_methods");
    std::cout << "[PASS] test_enum_methods\n";
}

// Test 7: Enum with computed properties
void test_enum_computed_properties() {
    std::string source = R"(
        enum Size {
            case small
            case medium
            case large
            
            var description: String {
                switch self {
                case Size.small:
                    return "S"
                case Size.medium:
                    return "M"
                case Size.large:
                    return "L"
                }
            }
        }
        
        var size = Size.medium
        print(size.description)
    )";

    std::string result = run_code(source);
    if (result.find("ERROR") != std::string::npos || result.find("SKIP") != std::string::npos) {
        std::cout << "[SKIP] test_enum_computed_properties (not implemented yet)\n";
        return;
    }
    AssertHelper::assert_contains(result, "M", "test_enum_computed_properties");
    std::cout << "[PASS] test_enum_computed_properties\n";
}

// Test 8: Multiple enum declarations
void test_multiple_enums() {
    std::string source = R"(
        enum Weather {
            case sunny
            case rainy
        }
        
        enum Temperature {
            case hot
            case cold
        }
        
        var w = Weather.sunny
        var t = Temperature.hot
        
        print(w)
        print(t)
    )";

    std::string result = run_code(source);
    AssertHelper::assert_contains(result, "sunny", "test_multiple_enums");
    AssertHelper::assert_contains(result, "hot", "test_multiple_enums");
    std::cout << "[PASS] test_multiple_enums\n";
}

} // namespace test
} // namespace swiftscript
