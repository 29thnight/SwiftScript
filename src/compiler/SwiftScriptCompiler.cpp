#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include "ss_compiler.hpp"
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_chunk.hpp"

using namespace swiftscript;

int main(int argc, char* argv[]) {
    std::string buildType = "Debug";
    std::string inputProject;
    std::string outputFile;

    // 명령행 인자 파싱
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.rfind("-compile:", 0) == 0) {
            buildType = arg.substr(9);
        } else if (arg == "-in" && i + 1 < argc) {
            inputProject = argv[++i];
        }
    }

    if (inputProject.empty()) {
        std::cerr << "Usage: SwiftScriptCompiler -compile:{Debug|Release} -in <project>.ssproject\n";
        return 1;
    }

    // 프로젝트 이름 추출 (확장자 제거)
    std::filesystem::path inPath(inputProject);
    std::string projectName = inPath.stem().string();
    outputFile = projectName + ".ssasm";

    // 소스코드 읽기 (여기서는 .ssproject가 단일 소스 파일이라고 가정)
    std::ifstream in(inputProject);
    if (!in) {
        std::cerr << "Cannot open input file: " << inputProject << "\n";
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    // 컴파일: 소스 → AST → Chunk
    Lexer lexer(source);
    auto tokens = lexer.tokenize_all();
    Parser parser(std::move(tokens));
    auto program = parser.parse();
    Compiler compiler;
    Chunk chunk = compiler.compile(program);

    // 결과물 직렬화
    std::ofstream out(outputFile, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open output file: " << outputFile << "\n";
        return 1;
    }
    chunk.serialize(out);

    std::cout << "Build (" << buildType << ") complete: " << outputFile << "\n";
    return 0;
}
