#include <gtest/gtest.h>
#include "test_helpers.hpp"
#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_vm.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace swiftscript::test;

// Custom test listener for logging to file
class FileTestListener : public ::testing::EmptyTestEventListener {
private:
    std::ofstream log_file_;
    std::chrono::steady_clock::time_point test_start_time_;
    std::chrono::steady_clock::time_point suite_start_time_;
    
public:
    FileTestListener(const std::string& filename) {
        log_file_.open(filename);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &time_t);
#else
        localtime_r(&time_t, &tm_buf);
#endif
        
        log_file_ << "========================================\n";
        log_file_ << "SwiftScript Test Results\n";
        log_file_ << "Date: " << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "\n";
        log_file_ << "========================================\n\n";
    }
    
    ~FileTestListener() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }
    
    void OnTestProgramStart(const ::testing::UnitTest& unit_test) override {
        log_file_ << "Starting " << unit_test.total_test_suite_count() 
                  << " test suites with " << unit_test.total_test_count() 
                  << " tests.\n\n";
    }
    
    void OnTestSuiteStart(const ::testing::TestSuite& test_suite) override {
        suite_start_time_ = std::chrono::steady_clock::now();
        log_file_ << "[ Test Suite ] " << test_suite.name() << "\n";
    }
    
    void OnTestStart(const ::testing::TestInfo& test_info) override {
        test_start_time_ = std::chrono::steady_clock::now();
        log_file_ << "  [ RUN      ] " << test_info.test_suite_name() 
                  << "." << test_info.name() << "\n";
    }
    
    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - test_start_time_).count();
        
        if (test_info.result()->Passed()) {
            log_file_ << "  [       OK ] " << test_info.test_suite_name() 
                      << "." << test_info.name() 
                      << " (" << duration << " ms)\n";
        } else {
            log_file_ << "  [  FAILED  ] " << test_info.test_suite_name() 
                      << "." << test_info.name() 
                      << " (" << duration << " ms)\n";
            
            // Log failure details
            for (int i = 0; i < test_info.result()->total_part_count(); ++i) {
                const auto& part = test_info.result()->GetTestPartResult(i);
                if (part.failed()) {
                    log_file_ << "    " << part.file_name() << ":" 
                              << part.line_number() << "\n";
                    log_file_ << "    " << part.summary() << "\n";
                }
            }
        }
    }
    
    void OnTestSuiteEnd(const ::testing::TestSuite& test_suite) override {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - suite_start_time_).count();
        
        log_file_ << "  Tests passed: " << test_suite.successful_test_count() 
                  << "/" << test_suite.total_test_count() 
                  << " (" << duration << " ms)\n\n";
    }
    
    void OnTestProgramEnd(const ::testing::UnitTest& unit_test) override {
        log_file_ << "========================================\n";
        log_file_ << "Test Summary\n";
        log_file_ << "========================================\n";
        log_file_ << "Total test suites: " << unit_test.total_test_suite_count() << "\n";
        log_file_ << "Total tests: " << unit_test.total_test_count() << "\n";
        log_file_ << "Passed: " << unit_test.successful_test_count() << "\n";
        log_file_ << "Failed: " << unit_test.failed_test_count() << "\n";
        log_file_ << "Elapsed time: " << unit_test.elapsed_time() << " ms\n";
        log_file_ << "========================================\n";
        
        if (unit_test.Passed()) {
            log_file_ << "\nALL TESTS PASSED!\n";
        } else {
            log_file_ << "\nSOME TESTS FAILED!\n";
        }
    }
};

// Forward declarations
namespace swiftscript { 
namespace test {

// Class tests
void test_simple_class_method();
void test_initializer_called();
void test_stored_property_defaults();
void test_inherited_method_call();
void test_super_method_call();
void test_inherited_property_defaults();
void test_override_required();
void test_override_without_base_method();
void test_override_init_allowed();
void test_deinit_called();
void test_deinit_with_properties();

// Computed Properties tests
void test_computed_property_getter_only();
void test_computed_property_getter_setter();
void test_computed_property_read_only_shorthand();
void test_computed_property_temperature_conversion();
void test_computed_property_with_logic();
void test_computed_property_access_other_properties();
void test_computed_property_multiple_in_class();

// Struct tests
void test_basic_struct();
void test_memberwise_init();
void test_custom_init();
void test_non_mutating_method();
void test_mutating_method();
void test_value_semantics();
void test_self_access();
void test_multiple_methods();
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

// Enum tests
void test_enum_basic();
void test_enum_raw_values();
void test_enum_switch();
void test_enum_associated_values();
void test_enum_comparison();
void test_enum_methods();
void test_enum_computed_properties();
void test_multiple_enums();

} // namespace test
} // namespace swiftscript

// ============================================================================
// Class Tests
// ============================================================================

TEST(ClassTests, SimpleClassMethod) {
    EXPECT_NO_THROW(swiftscript::test::test_simple_class_method());
}

TEST(ClassTests, InitializerCalled) {
    EXPECT_NO_THROW(swiftscript::test::test_initializer_called());
}

TEST(ClassTests, StoredPropertyDefaults) {
    EXPECT_NO_THROW(swiftscript::test::test_stored_property_defaults());
}

TEST(ClassTests, InheritedMethodCall) {
    EXPECT_NO_THROW(swiftscript::test::test_inherited_method_call());
}

TEST(ClassTests, SuperMethodCall) {
    EXPECT_NO_THROW(swiftscript::test::test_super_method_call());
}

TEST(ClassTests, InheritedPropertyDefaults) {
    EXPECT_NO_THROW(swiftscript::test::test_inherited_property_defaults());
}

TEST(ClassTests, OverrideRequired) {
    EXPECT_NO_THROW(swiftscript::test::test_override_required());
}

TEST(ClassTests, OverrideWithoutBaseMethod) {
    EXPECT_NO_THROW(swiftscript::test::test_override_without_base_method());
}

TEST(ClassTests, OverrideInitAllowed) {
    EXPECT_NO_THROW(swiftscript::test::test_override_init_allowed());
}

TEST(ClassTests, DeinitCalled) {
    EXPECT_NO_THROW(swiftscript::test::test_deinit_called());
}

TEST(ClassTests, DeinitWithProperties) {
    EXPECT_NO_THROW(swiftscript::test::test_deinit_with_properties());
}

// ============================================================================
// Computed Properties Tests
// ============================================================================

TEST(ComputedPropertiesTests, GetterOnly) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_getter_only());
}

TEST(ComputedPropertiesTests, GetterSetter) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_getter_setter());
}

TEST(ComputedPropertiesTests, ReadOnlyShorthand) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_read_only_shorthand());
}

TEST(ComputedPropertiesTests, TemperatureConversion) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_temperature_conversion());
}

TEST(ComputedPropertiesTests, WithLogic) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_with_logic());
}

TEST(ComputedPropertiesTests, AccessOtherProperties) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_access_other_properties());
}

TEST(ComputedPropertiesTests, MultipleInClass) {
    EXPECT_NO_THROW(swiftscript::test::test_computed_property_multiple_in_class());
}

// ============================================================================
// Struct Tests
// ============================================================================

TEST(StructTests, BasicStruct) {
    EXPECT_NO_THROW(swiftscript::test::test_basic_struct());
}

TEST(StructTests, MemberwiseInit) {
    EXPECT_NO_THROW(swiftscript::test::test_memberwise_init());
}

TEST(StructTests, CustomInit) {
    EXPECT_NO_THROW(swiftscript::test::test_custom_init());
}

TEST(StructTests, NonMutatingMethod) {
    EXPECT_NO_THROW(swiftscript::test::test_non_mutating_method());
}

TEST(StructTests, MutatingMethod) {
    EXPECT_NO_THROW(swiftscript::test::test_mutating_method());
}

TEST(StructTests, ValueSemantics) {
    EXPECT_NO_THROW(swiftscript::test::test_value_semantics());
}

TEST(StructTests, SelfAccess) {
    EXPECT_NO_THROW(swiftscript::test::test_self_access());
}

TEST(StructTests, MultipleMethods) {
    EXPECT_NO_THROW(swiftscript::test::test_multiple_methods());
}

TEST(StructTests, PropertyModification) {
    EXPECT_NO_THROW(swiftscript::test::test_property_modification());
}

TEST(StructTests, NestedStruct) {
    // This test may skip if not supported
    EXPECT_NO_THROW(swiftscript::test::test_nested_struct());
}

// ============================================================================
// Closure Tests
// ============================================================================

TEST(ClosureTests, BasicClosure) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_basic());
}

TEST(ClosureTests, ClosureNoParams) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_no_params());
}

TEST(ClosureTests, ClosureSingleParam) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_single_param());
}

TEST(ClosureTests, ClosureAsArgument) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_as_argument());
}

TEST(ClosureTests, ClosureMultipleStatements) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_multiple_statements());
}

TEST(ClosureTests, FunctionReturningClosure) {
    EXPECT_NO_THROW(swiftscript::test::test_function_returning_closure());
}

TEST(ClosureTests, ClosureVariableAssignment) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_variable_assignment());
}

TEST(ClosureTests, ClosureCapturesOuterVariable) {
    EXPECT_NO_THROW(swiftscript::test::test_closure_captures_outer_variable());
}

TEST(ClosureTests, NestedClosureAfterScopeExit) {
    EXPECT_NO_THROW(swiftscript::test::test_nested_closure_captures_after_scope_exit());
}

// ============================================================================
// Switch Tests
// ============================================================================

TEST(SwitchTests, BasicSwitch) {
    EXPECT_NO_THROW(swiftscript::test::test_switch_basic());
}

TEST(SwitchTests, SwitchDefault) {
    EXPECT_NO_THROW(swiftscript::test::test_switch_default());
}

TEST(SwitchTests, SwitchRange) {
    EXPECT_NO_THROW(swiftscript::test::test_switch_range());
}

TEST(SwitchTests, SwitchMultiplePatterns) {
    EXPECT_NO_THROW(swiftscript::test::test_switch_multiple_patterns());
}

// ============================================================================
// Enum Tests
// ============================================================================

TEST(EnumTests, BasicEnum) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_basic());
}

TEST(EnumTests, EnumRawValues) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_raw_values());
}

TEST(EnumTests, EnumSwitch) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_switch());
}

TEST(EnumTests, EnumAssociatedValues) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_associated_values());
}

TEST(EnumTests, EnumComparison) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_comparison());
}

TEST(EnumTests, EnumMethods) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_methods());
}

TEST(EnumTests, EnumComputedProperties) {
    EXPECT_NO_THROW(swiftscript::test::test_enum_computed_properties());
}

TEST(EnumTests, MultipleEnums) {
    EXPECT_NO_THROW(swiftscript::test::test_multiple_enums());
}

// ============================================================================
// Inline Enum Tests (Quick Verification)
// ============================================================================

TEST(EnumInlineTests, BasicEnumDeclaration) {
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
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("north"), std::string::npos) << "Expected 'north' in output, got: " << result;
}

TEST(EnumInlineTests, EnumRawValues) {
    std::string source = R"(
        enum Priority {
            case low = 1
            case medium = 2
            case high = 3
        }
        
        var p = Priority.high
        print(p.rawValue)
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("3"), std::string::npos) << "Expected '3' in output, got: " << result;
}

TEST(EnumInlineTests, EnumComparison) {
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
            print("SAME")
        }
        
        if c1 != c3 {
            print("DIFFERENT")
        }
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("SAME"), std::string::npos) << "Expected 'SAME' in output";
    EXPECT_NE(result.find("DIFFERENT"), std::string::npos) << "Expected 'DIFFERENT' in output";
}

TEST(EnumInlineTests, EnumWithMethod) {
    std::string source = R"(
        enum CompassPoint {
            case north
            case south
            case east
            case west
            
            func describe() -> String {
                return "Direction"
            }
        }
        
        var direction = CompassPoint.north
        print(direction.describe())
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("Direction"), std::string::npos) 
        << "Expected 'Direction' in output, got: " << result;
}

TEST(EnumInlineTests, EnumMethodWithSelfSwitch) {
    std::string source = R"(
        enum Direction {
            case north
            case south
            case east
            case west
            
            func describe() -> String {
                switch self {
                case Direction.north:
                    return "NORTH"
                case Direction.south:
                    return "SOUTH"
                case Direction.east:
                    return "EAST"
                case Direction.west:
                    return "WEST"
                }
            }
        }
        
        var dir = Direction.north
        print(dir.describe())
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("NORTH"), std::string::npos) 
        << "Expected 'NORTH' in output, got: " << result;
}

TEST(EnumInlineTests, EnumInSwitchStatement) {
    std::string source = R"(
        enum Status {
            case pending
            case active
            case completed
        }
        
        var status = Status.active
        
        switch status {
        case Status.pending:
            print("PENDING")
        case Status.active:
            print("ACTIVE")
        case Status.completed:
            print("COMPLETED")
        }
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(program);
    
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    EXPECT_NO_THROW(vm.execute(chunk));
    
    std::cout.rdbuf(old);
    std::string result = output.str();
    
    EXPECT_NE(result.find("ACTIVE"), std::string::npos) 
        << "Expected 'ACTIVE' in output, got: " << result;
}

TEST(EnumInlineTests, EnumSimpleComputedProperty) {
    std::string source = R"(
        enum Answer {
            case yes
            case no
            
            var text: String {
                return "Answer"
            }
        }
        
        var answer = Answer.yes
        var result = answer.text
        print(result)
    )";
    
    swiftscript::Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    swiftscript::Parser parser(std::move(tokens));
    auto program = parser.parse();
    
    // DIAGNOSTIC: Check if enum has methods
    bool found_enum = false;
    int method_count = 0;
    int computed_property_count = 0;
    
    for (const auto& stmt : program) {
        if (stmt->kind == swiftscript::StmtKind::EnumDecl) {
            found_enum = true;
            auto* enum_stmt = static_cast<swiftscript::EnumDeclStmt*>(stmt.get());
            method_count = static_cast<int>(enum_stmt->methods.size());
            
            for (const auto& method : enum_stmt->methods) {
                if (method->is_computed_property) {
                    computed_property_count++;
                }
            }
        }
    }
    
    // Print diagnostic info
    std::cout << "DIAGNOSTIC: Found enum: " << found_enum << std::endl;
    std::cout << "DIAGNOSTIC: Method count: " << method_count << std::endl;
    std::cout << "DIAGNOSTIC: Computed property count: " << computed_property_count << std::endl;
    
    ASSERT_TRUE(found_enum) << "Enum not found in AST";
    ASSERT_EQ(computed_property_count, 1) << "Expected 1 computed property, got " << computed_property_count;
    
    // Now try to compile
    swiftscript::Compiler compiler;
    swiftscript::Chunk chunk = compiler.compile(std::move(program));
    
    // DIAGNOSTIC: Check bytecode for OP_DEFINE_COMPUTED_PROPERTY
    bool found_computed_property_opcode = false;
    size_t computed_property_position = 0;
    
    // Also check for OP_ENUM and OP_ENUM_CASE positions
    for (size_t i = 0; i < chunk.code.size(); ++i) {
        auto opcode = static_cast<swiftscript::OpCode>(chunk.code[i]);
        
        if (opcode == swiftscript::OpCode::OP_ENUM) {
            std::cout << "DIAGNOSTIC: OP_ENUM at position " << i << std::endl;
        }
        else if (opcode == swiftscript::OpCode::OP_ENUM_CASE) {
            std::cout << "DIAGNOSTIC: OP_ENUM_CASE at position " << i << std::endl;
        }
        else if (opcode == swiftscript::OpCode::OP_DEFINE_COMPUTED_PROPERTY) {
            found_computed_property_opcode = true;
            computed_property_position = i;
            std::cout << "DIAGNOSTIC: OP_DEFINE_COMPUTED_PROPERTY at position " << i << std::endl;
        }
        else if (opcode == swiftscript::OpCode::OP_METHOD) {
            std::cout << "DIAGNOSTIC: OP_METHOD at position " << i << std::endl;
        }
    }
    
    if (!found_computed_property_opcode) {
        std::cout << "DIAGNOSTIC: OP_DEFINE_COMPUTED_PROPERTY NOT FOUND in bytecode!" << std::endl;
    }
    
    // Run
    swiftscript::VMConfig config;
    config.enable_debug = false;
    swiftscript::VM vm(config);
    
    std::ostringstream output;
    std::streambuf* old = std::cout.rdbuf(output.rdbuf());
    
    try {
        vm.execute(chunk);
        std::cout.rdbuf(old);
        std::string result = output.str();
        
        std::cout << "OUTPUT: " << result << std::endl;
        
        EXPECT_NE(result.find("Answer"), std::string::npos) 
            << "Expected 'Answer' in output, got: " << result;
    } catch (const std::exception& e) {
        std::cout.rdbuf(old);
        std::cout << "EXCEPTION: " << e.what() << std::endl;
        FAIL() << "Exception: " << e.what();
    }
}

// ============================================================================
// Main function for Google Test
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Add custom listener for file logging
    ::testing::TestEventListeners& listeners = 
        ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new FileTestListener("testlog.txt"));
    
    std::cout << "Running tests... Results will be saved to testlog.txt\n\n";
    
    int result = RUN_ALL_TESTS();
    
    std::cout << "\nTest results have been saved to testlog.txt\n";
    
    return result;
}
