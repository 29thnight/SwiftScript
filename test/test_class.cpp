#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

using namespace swiftscript;

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

void test_simple_class_method() {
    std::cout << "Test: simple class method ... ";
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
    assert(out.find("hi") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_initializer_called() {
    std::cout << "Test: initializer is invoked ... ";
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
    assert(out.find("init called") != std::string::npos);
    assert(out.find("123") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_stored_property_defaults() {
    std::cout << "Test: stored property defaults ... ";
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
    assert(out.find("42") != std::string::npos);
    assert(out.find("box") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_inherited_method_call() {
    std::cout << "Test: inherited method call ... ";
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
    assert(out.find("woof") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_super_method_call() {
    std::cout << "Test: super method call ... ";
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
    assert(out.find("animal") != std::string::npos);
    assert(out.find("dog") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_inherited_property_defaults() {
    std::cout << "Test: inherited property defaults ... ";
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
    assert(out.find("1") != std::string::npos);
    assert(out.find("2") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_override_required() {
    std::cout << "Test: override keyword required ... ";
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
    assert(out.find("ERROR") != std::string::npos || 
           out.find("override") != std::string::npos ||
           out.find("overrides") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_override_without_base_method() {
    std::cout << "Test: override without base method (should error) ... ";
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
    assert(out.find("ERROR") != std::string::npos || 
           out.find("override") != std::string::npos ||
           out.find("does not override") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_override_init_allowed() {
    std::cout << "Test: override init without keyword allowed ... ";
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
    assert(out.find("derived") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_deinit_called() {
    std::cout << "Test: deinit is called on deallocation ... ";
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
    assert(out.find("created") != std::string::npos);
    assert(out.find("cleanup") != std::string::npos);
    assert(out.find("done") != std::string::npos);
    std::cout << "PASSED\n";
}

void test_deinit_with_properties() {
    std::cout << "Test: deinit can access properties ... ";
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
    assert(out.find("42") != std::string::npos);
    std::cout << "PASSED\n";
}

int main() {
    std::cout << "======================================\n";
    std::cout << "  CLASS TEST SUITE\n";
    std::cout << "======================================\n\n";

    try {
        test_simple_class_method();
        test_initializer_called();
        test_stored_property_defaults();
        test_inherited_method_call();
        test_super_method_call();
        test_inherited_property_defaults();
        test_override_required();
        test_override_without_base_method();
        test_override_init_allowed();
        test_deinit_called();
        test_deinit_with_properties();

        std::cout << "\n======================================\n";
        std::cout << "  ALL CLASS TESTS PASSED!\n";
        std::cout << "======================================\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n======================================\n";
        std::cerr << "  CLASS TEST FAILED!\n";
        std::cerr << "  Error: " << e.what() << "\n";
        std::cerr << "======================================\n";
        return 1;
    }
}
