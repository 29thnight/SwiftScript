#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace swiftscript;

std::string execute_code(const std::string& source) {
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
        std::streambuf* old_cout = std::cout.rdbuf(output.rdbuf());
        struct CoutRestorer {
            std::streambuf* old_buf;
            ~CoutRestorer() { std::cout.rdbuf(old_buf); }
        } restorer{old_cout};
        
        vm.execute(chunk);
        return output.str();
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

void test_switch_basic() {
    std::cout << "Test: Basic switch statement ... ";
    
    std::string source = R"(
        var value = 2
        switch value {
        case 1:
            print("One")
        case 2:
            print("Two")
        case 3:
            print("Three")
        default:
            print("Other")
        }
    )";
    
    std::string result = execute_code(source);
    assert(result.find("Two") != std::string::npos);
    assert(result.find("One") == std::string::npos);
    assert(result.find("Three") == std::string::npos);
    
    std::cout << "PASSED\n";
}

void test_switch_default() {
    std::cout << "Test: Switch with default case ... ";
    
    std::string source = R"(
        var value = 99
        switch value {
        case 1:
            print("One")
        case 2:
            print("Two")
        default:
            print("Default")
        }
    )";
    
    std::string result = execute_code(source);
    assert(result.find("Default") != std::string::npos);
    
    std::cout << "PASSED\n";
}

void test_switch_range() {
    std::cout << "Test: Switch with range pattern ... ";
    
    std::string source = R"(
        var score = 85
        switch score {
        case 90...100:
            print("A")
        case 80...89:
            print("B")
        case 70...79:
            print("C")
        default:
            print("F")
        }
    )";
    
    std::string result = execute_code(source);
    assert(result.find("B") != std::string::npos);
    
    std::cout << "PASSED\n";
}

void test_switch_multiple_patterns() {
    std::cout << "Test: Switch with multiple patterns ... ";
    
    std::string source = R"(
        var day = 6
        switch day {
        case 1, 2, 3, 4, 5:
            print("Weekday")
        case 6, 7:
            print("Weekend")
        default:
            print("Invalid")
        }
    )";
    
    std::string result = execute_code(source);
    assert(result.find("Weekend") != std::string::npos);
    
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "======================================\n";
    std::cout << "  SWITCH STATEMENT TEST SUITE\n";
    std::cout << "======================================\n\n";

    try {
        test_switch_basic();
        test_switch_default();
        test_switch_range();
        test_switch_multiple_patterns();
        
        std::cout << "\n======================================\n";
        std::cout << "  ALL SWITCH TESTS PASSED!\n";
        std::cout << "======================================\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n======================================\n";
        std::cerr << "  TEST FAILED!\n";
        std::cerr << "  Error: " << e.what() << "\n";
        std::cerr << "======================================\n";
        return 1;
    }
}
