#include "ss_vm.hpp"
#include "ss_value.hpp"
#include "ss_core.hpp"
#include <iostream>

using namespace swiftscript;

//int main() {
//    std::cout << "SwiftScript Object Test\n";
//    std::cout << "========================\n\n";
//    
//    VMConfig config;
//    config.enable_debug = true;
//    VM vm(config);
//    
//    std::cout << "Test 1: String allocation\n";
//    auto* str = vm.allocate_object<StringObject>("Hello, SwiftScript!");
//    std::cout << "String: " << str->to_string() << "\n";
//    std::cout << "Initial RC: " << str->rc.strong_count << "\n\n";
//    
//    std::cout << "Test 2: Retain\n";
//    RC::retain(str);
//    std::cout << "After retain: " << str->rc.strong_count << "\n\n";
//    
//    std::cout << "Test 3: Release\n";
//    RC::release(&vm, str);
//    std::cout << "After release: " << str->rc.strong_count << "\n\n";
//    
//    std::cout << "✓ All tests passed!\n";
//    return 0;
//}
