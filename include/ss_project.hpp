#pragma once
#include <filesystem>
#include <vector>
#include <string>

namespace swiftscript {

struct SSProject {
    std::filesystem::path project_file;
    std::filesystem::path project_dir;

    std::filesystem::path entry_file;                 // e.g. Scripts/main.ss
    std::vector<std::filesystem::path> import_roots;  // e.g. Scripts, Libs
};

bool LoadSSProject(const std::filesystem::path& ssproject, SSProject& out, std::string& err);

} // namespace swiftscript
