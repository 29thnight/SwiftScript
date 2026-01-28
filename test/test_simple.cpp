#include "ss_vm.hpp"
#include "ss_value.hpp"
#include "ss_core.hpp"
#include <iostream>
#include <cassert>

using namespace swiftscript;

//int main() {
//    std::cout << "SwiftScript Simple Test\n";
//    std::cout << "========================\n\n";
//    
//    // Test 1: Value types
//    std::cout << "Test 1: Value Types\n";
//    Value v_int = Value::from_int(42);
//    Value v_float = Value::from_float(3.14);
//    Value v_bool = Value::from_bool(true);
//    Value v_null = Value::null();
//    
//    std::cout << "Int: " << v_int.to_string() << "\n";
//    std::cout << "Float: " << v_float.to_string() << "\n";
//    std::cout << "Bool: " << v_bool.to_string() << "\n";
//    std::cout << "Null: " << v_null.to_string() << "\n";
//    std::cout << "Value size: " << Value::size() << " bytes\n\n";
//    
//    // Test 2: Simple object allocation
//    std::cout << "Test 2: String Object\n";
//    VMConfig config;
//    config.enable_debug = true;
//    VM vm(config);
//    
//    auto* str = vm.allocate_object<StringObject>("Hello");
//    std::cout << "Created string: " << str->to_string() << "\n";
//    std::cout << "Initial RC: " << str->rc.strong_count << "\n\n";
//    
//    // Test 3: RC operations
//    std::cout << "Test 3: Reference Counting\n";
//    RC::retain(str);
//    std::cout << "After retain: " << str->rc.strong_count << "\n";
//    
//    RC::release(&vm, str);
//    std::cout << "After release: " << str->rc.strong_count << "\n";
//    std::cout << "Deferred releases: " << vm.get_deferred_releases().size() << "\n\n";
//    
//    // Test 4: Deferred cleanup
//    std::cout << "Test 4: Deferred Cleanup\n";
//    vm.run_cleanup();
//    std::cout << "After cleanup, deferred releases: " << vm.get_deferred_releases().size() << "\n\n";
//    
//    // Test 5: List
//    std::cout << "Test 5: List Object\n";
//    auto* list = vm.allocate_object<ListObject>();
//    list->elements.push_back(Value::from_int(1));
//    list->elements.push_back(Value::from_int(2));
//    list->elements.push_back(Value::from_int(3));
//    std::cout << "List: " << list->to_string() << "\n";
//    std::cout << "List RC: " << list->rc.strong_count << "\n\n";
//    
//    std::cout << "✓ All tests passed!\n\n";
//    
//    return 0;
//}
