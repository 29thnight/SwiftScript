#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"
#include <gtest/gtest.h>
#include <sstream>

namespace swiftscript {

namespace {
// Helper function to run SwiftScript code
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

// Test basic protocol declaration
TEST(ProtocolTest, BasicProtocolDeclaration) {
    const char* code = R"(
        protocol Drawable {
            func draw()
            var size: Int { get set }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test protocol with method requirements
TEST(ProtocolTest, ProtocolMethodRequirements) {
    const char* code = R"(
        protocol Vehicle {
            func start()
            func stop()
            func getSpeed() -> Int
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test protocol with property requirements
TEST(ProtocolTest, ProtocolPropertyRequirements) {
    const char* code = R"(
        protocol Named {
            var name: String { get }
            var fullName: String { get set }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test protocol inheritance
TEST(ProtocolTest, ProtocolInheritance) {
    const char* code = R"(
        protocol Animal {
            func makeSound()
        }
        
        protocol Pet: Animal {
            var name: String { get }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test protocol with mutating method
TEST(ProtocolTest, ProtocolMutatingMethod) {
    const char* code = R"(
        protocol Counter {
            mutating func increment()
            var count: Int { get }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test struct conforming to protocol
TEST(ProtocolTest, StructProtocolConformance) {
    const char* code = R"(
        protocol Drawable {
            func draw()
        }
        
        struct Circle: Drawable {
            var radius: Int
            
            func draw() {
                print("Drawing circle")
            }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test class conforming to protocol
TEST(ProtocolTest, ClassProtocolConformance) {
    const char* code = R"(
        protocol Describable {
            func describe() -> String
        }
        
        class Person: Describable {
            var name: String
            
            func describe() -> String {
                return name
            }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test class with superclass and protocol
TEST(ProtocolTest, ClassSuperclassAndProtocol) {
    const char* code = R"(
        protocol Flyable {
            func fly()
        }
        
        class Animal {
            var name: String
        }
        
        class Bird: Animal, Flyable {
            func fly() {
                print("Flying")
            }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test multiple protocol conformance
TEST(ProtocolTest, MultipleProtocolConformance) {
    const char* code = R"(
        protocol Drawable {
            func draw()
        }
        
        protocol Movable {
            func move()
        }
        
        struct Sprite: Drawable, Movable {
            func draw() {
                print("Drawing")
            }
            
            func move() {
                print("Moving")
            }
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

// Test protocol with multiple method parameters
TEST(ProtocolTest, ProtocolMethodParameters) {
    const char* code = R"(
        protocol Calculator {
            func add(a: Int, b: Int) -> Int
            func multiply(x: Int, y: Int) -> Int
        }
    )";
    
    ASSERT_NO_THROW(run_code(code));
}

} // namespace swiftscript
