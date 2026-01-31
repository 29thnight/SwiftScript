#include "pch_.h"
#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include "test_helpers.hpp"

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

void test_basic_tuple_literal() {
    std::string source = R"(
        let point = (10, 20)
        print(point)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "(10, 20)", "should print tuple");
}

void test_named_tuple_literal() {
    std::string source = R"(
        let point = (x: 5, y: 15)
        print(point)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "(x: 5, y: 15)", "should print named tuple");
}

void test_tuple_index_access() {
    std::string source = R"(
        let point = (10, 20)
        print(point.0)
        print(point.1)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "10", "should access first element");
    AssertHelper::assert_contains(out, "20", "should access second element");
}

void test_tuple_label_access() {
    std::string source = R"(
        let point = (x: 5, y: 15)
        print(point.x)
        print(point.y)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "5", "should access x");
    AssertHelper::assert_contains(out, "15", "should access y");
}

void test_mixed_tuple() {
    std::string source = R"(
        let mixed = (name: "Alice", 30)
        print(mixed.name)
        print(mixed.1)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "Alice", "should access labeled element");
    AssertHelper::assert_contains(out, "30", "should access unlabeled element by index");
}

void test_tuple_with_different_types() {
    std::string source = R"(
        let person = (name: "Bob", age: 25, active: true)
        print(person.name)
        print(person.age)
        print(person.active)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "Bob", "should access name");
    AssertHelper::assert_contains(out, "25", "should access age");
    AssertHelper::assert_contains(out, "true", "should access active");
}

void test_tuple_in_variable() {
    std::string source = R"(
        var coord = (1, 2, 3)
        print(coord.0)
        print(coord.1)
        print(coord.2)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "1", "should access first element");
    AssertHelper::assert_contains(out, "2", "should access second element");
    AssertHelper::assert_contains(out, "3", "should access third element");
}

void test_nested_tuple() {
    std::string source = R"(
        let nested = ((1, 2), (3, 4))
        print(nested.0)
        print(nested.1)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "(1, 2)", "should access first tuple");
    AssertHelper::assert_contains(out, "(3, 4)", "should access second tuple");
}

void test_tuple_destructuring_basic() {
    std::string source = R"(
        let point = (10, 20)
        let (x, y) = point
        print(x)
        print(y)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "10", "should extract x");
    AssertHelper::assert_contains(out, "20", "should extract y");
}

void test_tuple_destructuring_three_elements() {
    std::string source = R"(
        let coords = (1, 2, 3)
        let (a, b, c) = coords
        print(a)
        print(b)
        print(c)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "1", "should extract a");
    AssertHelper::assert_contains(out, "2", "should extract b");
    AssertHelper::assert_contains(out, "3", "should extract c");
}

void test_tuple_destructuring_with_var() {
    std::string source = R"(
        let point = (100, 200)
        var (x, y) = point
        print(x)
        print(y)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "100", "should extract x");
    AssertHelper::assert_contains(out, "200", "should extract y");
}

void test_tuple_destructuring_with_wildcard() {
    std::string source = R"(
        let data = (1, 2, 3)
        let (first, _, last) = data
        print(first)
        print(last)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "1", "should extract first");
    AssertHelper::assert_contains(out, "3", "should extract last");
}

void test_func_return_tuple() {
    std::string source = R"(
        func getPoint() -> (Int, Int) {
            return (10, 20)
        }
        let pt = getPoint()
        print(pt.0)
        print(pt.1)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "10", "should access first element");
    AssertHelper::assert_contains(out, "20", "should access second element");
}

void test_func_return_named_tuple() {
    std::string source = R"(
        func getRect() -> (w: Int, h: Int) {
            return (w: 100, h: 50)
        }
        let rect = getRect()
        print(rect.w)
        print(rect.h)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "100", "should access w");
    AssertHelper::assert_contains(out, "50", "should access h");
}

void test_func_return_tuple_destructuring() {
    std::string source = R"(
        func getCoords() -> (Int, Int, Int) {
            return (1, 2, 3)
        }
        let (x, y, z) = getCoords()
        print(x)
        print(y)
        print(z)
    )";
    auto out = run_code(source);
    AssertHelper::assert_no_error(out);
    AssertHelper::assert_contains(out, "1", "should extract x");
    AssertHelper::assert_contains(out, "2", "should extract y");
    AssertHelper::assert_contains(out, "3", "should extract z");
}

} // namespace test
} // namespace swiftscript
