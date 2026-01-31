#include <gtest/gtest.h>
#include "ss_project.hpp"
#include "ss_runner.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

class ProjectRunnerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create directory structure
        // Ensure clean state
        if (fs::exists("TestProject")) {
            fs::remove_all("TestProject");
        }
        fs::create_directories("TestProject/Libs");
        fs::create_directories("TestProject/Scripts");
    }

    void TearDown() override {
        // Cleanup
        if (fs::exists("TestProject")) {
            fs::remove_all("TestProject");
        }
    }
    
    void CreateFile(const std::string& path, const std::string& content) {
        std::ofstream f(path);
        f << content;
        f.close();
    }
};

TEST_F(ProjectRunnerTest, BasicProjectExecution) {
    // 1. Module
    CreateFile("TestProject/Libs/MathLib.ss", 
        "public func add(a: Int, b: Int) -> Int {\n"
        "    return a + b\n"
        "}\n");

    // 2. Entry
    CreateFile("TestProject/Scripts/main.ss",
        "import MathLib\n"
        "print(MathLib.add(a: 10, b: 20))\n"
        "func main() {\n"
        "    print(\"Main executed\")\n"
        "}\n");

    // 3. Project File
    CreateFile("TestProject/project.ssproject",
        "<Project>\n"
        "    <Entry>Scripts/main.ss</Entry>\n"
        "    <ImportRoots>\n"
        "        <Root>Libs</Root>\n"
        "        <Root>Scripts</Root>\n"
        "    </ImportRoots>\n"
        "</Project>\n");

    // 4. Load
    swiftscript::SSProject project;
    std::string error;
    bool loaded = swiftscript::LoadSSProject("TestProject/project.ssproject", project, error);
    ASSERT_TRUE(loaded) << "Failed to load project: " << error;

    // Check loaded paths
    ASSERT_EQ(project.import_roots.size(), 2);
    
    // 5. Run and Capture stdout
    testing::internal::CaptureStdout();
    
    try {
        swiftscript::VM vm;
        swiftscript::RunProject(vm, project);
    } catch (const std::exception& e) {
        std::string output = testing::internal::GetCapturedStdout();
        FAIL() << "Execution failed: " << e.what() << "\nOutput so far: " << output;
    }
    
    std::string output = testing::internal::GetCapturedStdout();
    
    // Check output contains module usage result
    EXPECT_TRUE(output.find("30") != std::string::npos) << "Output should contain '30', got:\n" << output;
    
    // Check output contains main() execution result
    EXPECT_TRUE(output.find("Main executed") != std::string::npos) << "Output should contain 'Main executed', got:\n" << output;
}
