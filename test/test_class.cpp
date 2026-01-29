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

void test_simple_class_method() {
    std::string source = R"(
        class Greeter {
            func greet() {
                print("hi")
            }
        }
        var g = Greeter()
        g.greet()
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "hi", "greet() should print 'hi'");
}

void test_initializer_called() {
    std::string source = R"(
        class Counter {
            func init() {
                print("init called")
            }
            func value() -> Int {
                return 123
            }
        }
        var c = Counter()
        print(c.value())
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_order(out, "init called", "123", "init should be called before value()");
}

void test_stored_property_defaults() {
    std::string source = R"(
        class Box {
            var value: Int = 42
            let label = "box"
            func describe() {
                print(label)
            }
        }
        var box = Box()
        print(box.value)
        box.describe()
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "42", "Property value should be 42");
    AssertHelper::assert_contains(out, "box", "Property label should be 'box'");
}

void test_inherited_method_call() {
    std::string source = R"(
        class Animal {
            func speak() {
                print("woof")
            }
        }
        class Dog: Animal {
        }
        var d = Dog()
        d.speak()
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "woof", "Inherited method should work");
}

void test_super_method_call() {
    std::string source = R"(
        class Animal {
            func speak() {
                print("animal")
            }
        }
        class Dog: Animal {
            override func speak() {
                super.speak()
                print("dog")
            }
        }
        var d = Dog()
        d.speak()
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_order(out, "animal", "dog", "super.speak() should be called first");
}

void test_inherited_property_defaults() {
    std::string source = R"(
        class Base {
            var a: Int = 1
        }
        class Derived: Base {
            var b: Int = 2
        }
        var obj = Derived()
        print(obj.a)
        print(obj.b)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    OutputMatcher::assert_contains_all(out, {"1", "2"});
    AssertHelper::assert_order(out, "1", "2", "Properties should print in order a, b");
}

void test_override_required() {
    std::string source = R"(
        class Animal {
            func speak() {
                print("animal")
            }
        }
        class Dog: Animal {
           func speak() {
                print("dog")
            }
        }
        var d = Dog()
        d.speak()
    )";
    auto out = run_code(source);
    // Should error: override not used
    AssertHelper::assert_error(out, "Missing override keyword should produce error");
}

void test_override_without_base_method() {
    std::string source = R"(
        class Animal {
            func speak() {
                print("animal")
            }
        }
        class Dog: Animal {
            override func bark() {
                print("woof")
            }
        }
        var d = Dog()
        d.bark()
    )";
    auto out = run_code(source);
    // Should error: override used but no base method
    AssertHelper::assert_error(out, "Override without base method should produce error");
}

void test_override_init_allowed() {
    std::string source = R"(
        class Base {
            func init() {
                print("base")
            }
        }
        class Derived: Base {
            func init() {
                print("derived")
            }
        }
        var d = Derived()
    )";
    auto out = run_code(source);
    // init can be overridden without override keyword
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "derived", "Derived init should be called");
}

void test_deinit_called() {
    std::string source = R"(
        class Resource {
            deinit {
                print("cleanup")
            }
        }
        func test() {
            var r = Resource()
            print("created")
        }
        test()
        print("done")
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    
    // Note: deinit may be called after function returns (deferred deallocation)
    // Just verify all three messages are present
    AssertHelper::assert_contains(out, "created", "Should create resource");
    AssertHelper::assert_contains(out, "cleanup", "Should call deinit");
    AssertHelper::assert_contains(out, "done", "Should complete");
    
    // If VM uses immediate deallocation (ARC-style), order would be: created, cleanup, done
    // If VM uses deferred deallocation (GC-style), order would be: created, done, cleanup
    // Both are valid depending on memory management strategy
}

void test_deinit_with_properties() {
    std::string source = R"(
        class Counter {
            var value: Int = 42
            deinit {
                print(value)
            }
        }
        func test() {
            var c = Counter()
        }
        test()
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "42", "deinit should access property value");
}

// ============================================================================
// Computed Properties Tests
// ============================================================================

void test_computed_property_getter_only() {
    std::string source = R"(
        class Circle {
            var radius: Int = 0
            
            var diameter: Int {
                get {
                    return radius * 2
                }
            }
        }
        
        var c = Circle()
        c.radius = 5
        print(c.diameter)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "10", "Computed diameter should be 10");
}

void test_computed_property_getter_setter() {
    std::string source = R"(
        class Rectangle {
            var width: Int = 0
            var height: Int = 0
            
            var area: Int {
                get {
                    return width * height
                }
                set {
                    width = newValue / height
                }
            }
        }
        
        var r = Rectangle()
        r.width = 4
        r.height = 5
        print(r.area)
        r.area = 40
        print(r.width)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "20", "Area should be 20");
    AssertHelper::assert_contains(out, "8", "Width should be 8 after setting area");
}

void test_computed_property_read_only_shorthand() {
    std::string source = R"(
        class Point {
            var x: Int = 0
            var y: Int = 0
            
            var magnitude: Int {
                return x * x + y * y
            }
        }
        
        var p = Point()
        p.x = 3
        p.y = 4
        print(p.magnitude)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "25", "Magnitude should be 25");
}

void test_computed_property_temperature_conversion() {
    std::string source = R"(
        class Temperature {
            var celsius: Int = 0
            
            var fahrenheit: Int {
                get {
                    return celsius * 2 + 32
                }
                set {
                    celsius = (newValue - 32) / 2
                }
            }
        }
        
        var t = Temperature()
        t.celsius = 100
        print(t.fahrenheit)
        t.fahrenheit = 32
        print(t.celsius)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "232", "100C should be ~232 (simplified)");
    AssertHelper::assert_contains(out, "0", "32F should be 0C");
}

void test_computed_property_with_logic() {
    std::string source = R"(
        class Person {
            var birthYear: Int = 2000
            
            var age: Int {
                get {
                    return 2024 - birthYear
                }
                set {
                    birthYear = 2024 - newValue
                }
            }
        }
        
        var p = Person()
        p.birthYear = 1990
        print(p.age)
        p.age = 30
        print(p.birthYear)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "34", "Age should be 34");
    AssertHelper::assert_contains(out, "1994", "Birth year should be 1994");
}

void test_computed_property_access_other_properties() {
    std::string source = R"(
        class BankAccount {
            var balance: Int = 0
            var interestRate: Int = 5
            
            var interest: Int {
                return balance * interestRate / 100
            }
            
            var totalWithInterest: Int {
                return balance + interest
            }
        }
        
        var account = BankAccount()
        account.balance = 1000
        print(account.interest)
        print(account.totalWithInterest)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "50", "Interest should be 50");
    AssertHelper::assert_contains(out, "1050", "Total with interest should be 1050");
}

void test_computed_property_multiple_in_class() {
    std::string source = R"(
        class Square {
            var side: Int = 0
            
            var area: Int {
                return side * side
            }
            
            var perimeter: Int {
                return side * 4
            }
        }
        
        var s = Square()
        s.side = 5
        print(s.area)
        print(s.perimeter)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "25", "Area should be 25");
    AssertHelper::assert_contains(out, "20", "Perimeter should be 20");
}

} // namespace test
} // namespace swiftscript
