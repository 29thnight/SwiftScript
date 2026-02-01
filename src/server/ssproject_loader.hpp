#pragma once
#include <filesystem>
#include <vector>
#include <string>

struct SSProjectInfo {
    std::filesystem::path project_file;
    std::filesystem::path project_dir;
    std::filesystem::path entry_file;                 // absolute
    std::vector<std::filesystem::path> import_roots;  // absolute
};

bool FindFirstSSProject(const std::filesystem::path& rootDir, std::filesystem::path& outProject);
bool LoadSSProjectInfo(const std::filesystem::path& ssproject, SSProjectInfo& out, std::string& err);
