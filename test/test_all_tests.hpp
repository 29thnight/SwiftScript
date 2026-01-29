#pragma once

#include <functional>
#include <string>
#include <vector>

namespace swiftscript {
namespace test {

// Test registry for manual test runner
struct TestCase {
    std::string name;
    std::function<void()> test_func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry registry;
        return registry;
    }

    void register_test(const std::string& suite, const std::string& name, std::function<void()> func) {
        test_cases_.push_back({suite + "::" + name, func});
    }

    const std::vector<TestCase>& get_tests() const {
        return test_cases_;
    }

    void clear() {
        test_cases_.clear();
    }

private:
    std::vector<TestCase> test_cases_;
};

// Macro for registering tests (works for both manual and Google Test)
#define REGISTER_TEST(suite, name, func) \
    namespace { \
        struct TestRegistrar_##suite##_##name { \
            TestRegistrar_##suite##_##name() { \
                swiftscript::test::TestRegistry::instance().register_test(#suite, #name, func); \
            } \
        }; \
        static TestRegistrar_##suite##_##name registrar_##suite##_##name; \
    }

} // namespace test
} // namespace swiftscript

// Forward declarations for test suites
namespace swiftscript {
namespace test {

// Class tests
void test_simple_class_method();
void test_class_with_properties();
void test_class_inheritance();
void test_class_method_override();
void test_super_keyword();
void test_init_method();
void test_init_with_params();
void test_init_inherited();
void test_property_let();
void test_property_var();
void test_method_access_property();
void test_override_keyword_required();
void test_override_without_base_error();
void test_override_init_allowed();
void test_deinit_called();
void test_deinit_with_properties();

// Struct tests
void test_basic_struct();
void test_memberwise_init();
void test_custom_init();
void test_non_mutating_method();
void test_mutating_method();
void test_mutating_self_assignment();
void test_let_vs_var_instances();
void test_struct_with_methods();
void test_property_modification();
void test_nested_struct();

// Closure tests
void test_closure_basic();
void test_closure_no_params();
void test_closure_single_param();
void test_closure_as_argument();
void test_closure_multiple_statements();
void test_function_returning_closure();
void test_closure_variable_assignment();
void test_closure_captures_outer_variable();
void test_nested_closure_captures_after_scope_exit();

// Switch tests
void test_switch_basic();
void test_switch_default();
void test_switch_range();
void test_switch_multiple_patterns();


} // namespace test
} // namespace swiftscript
