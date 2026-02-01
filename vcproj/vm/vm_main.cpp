#include "pch_.h"
#include "ss_project.hpp"
#include "ss_project_resolver.hpp"
#include "ss_vm.hpp"
#include "ss_chunk.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace swiftscript
{
    // .ssasm 파일에서 Chunk를 읽어 VM에서 실행
    inline Value AssmblyRun(VM& vm, const std::string& ssasm_path)
    {
        std::ifstream in(ssasm_path, std::ios::binary);
        if (!in) {
            throw std::runtime_error("Cannot open .ssasm file: " + ssasm_path);
        }
        Chunk chunk = Chunk::deserialize(in);
        return vm.execute(chunk);
    }
}

int main(int argc, char* argv[]) {
    using namespace swiftscript;

    if (argc < 2) {
        std::cerr << "Usage: SwiftScriptVM <program>.ssasm\n";
        return 1;
    }

    std::string ssasm_path = argv[1];

    try {
        VM vm;
        Value result = AssmblyRun(vm, ssasm_path);
        // 필요시 결과 출력
        std::cout << "Program finished. Return value: " << result.to_string() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "VM error: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}