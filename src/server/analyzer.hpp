#pragma once
#include <string>
#include <vector>
#include <filesystem>

class ProjectModuleResolver;

struct DiagnosticItem {
    std::string uri;     // document uri
    int line = 0;        // 0-based for LSP
    int col = 0;         // 0-based
    int endLine = 0;
    int endCol = 1;
    enum class Severity { Error = 1, Warning = 2, Info = 3, Hint = 4 } severity = Severity::Error;
    std::string message;
};

class Analyzer {
public:
    explicit Analyzer(ProjectModuleResolver* resolver = nullptr)
        : resolver_(resolver) {
    }

    void SetResolver(ProjectModuleResolver* resolver) { resolver_ = resolver; }

    // Analyze a single document text
    void Analyze(const std::string& docUri,
        const std::string& text,
        std::vector<DiagnosticItem>& out);

private:
    ProjectModuleResolver* resolver_{ nullptr };

    static void PushError(std::vector<DiagnosticItem>& out,
        const std::string& uri,
        int line0, int col0,
        const std::string& msg);
};
