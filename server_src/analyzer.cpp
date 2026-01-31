#include "stdafx.h"
#include "analyzer.hpp"
#include "module_resolver.hpp"
#include "lsp_utils.hpp"

// SwiftScript headers
#include "ss_lexer.hpp"
#include "ss_parser.hpp"
#include "ss_ast.hpp"

using swiftscript::Lexer;
using swiftscript::Parser;
using swiftscript::ParseError;
using swiftscript::TokenType;
using swiftscript::StmtKind;
using swiftscript::ImportStmt;

void Analyzer::PushError(std::vector<DiagnosticItem>& out,
    const std::string& uri,
    int line0, int col0,
    const std::string& msg) {
    DiagnosticItem d;
    d.uri = uri;
    d.line = line0;
    d.col = col0;
    d.endLine = line0;
    d.endCol = col0 + 1;
    d.severity = DiagnosticItem::Severity::Error;
    d.message = msg;
    out.push_back(std::move(d));
}

void Analyzer::Analyze(const std::string& docUri,
    const std::string& text,
    std::vector<DiagnosticItem>& out) {
    out.clear();

    // 1) Lexer
    Lexer lexer(text);
    auto tokens = lexer.tokenize_all();

    for (const auto& tk : tokens) {
        if (tk.type == TokenType::Error) {
            // Lexer error token: message in lexeme
            const int line0 = (tk.line > 0) ? (int)tk.line - 1 : 0;
            const int col0 = (tk.column > 0) ? (int)tk.column - 1 : 0;
            PushError(out, docUri, line0, col0, std::string(tk.lexeme));
            // keep going to gather more errors if any
        }
    }

    // 2) Parser
    std::vector<swiftscript::StmtPtr> program;
    try {
        Parser parser(std::move(tokens));
        program = parser.parse();
    }
    catch (const ParseError& e) {
        const int line0 = (e.line > 0) ? (int)e.line - 1 : 0;
        const int col0 = (e.column > 0) ? (int)e.column - 1 : 0;
        PushError(out, docUri, line0, col0, std::string(e.what()));
        return;
    }
    catch (const std::exception& e) {
        PushError(out, docUri, 0, 0, std::string("Parser exception: ") + e.what());
        return;
    }

    // 3) Import resolve check (MVP: only resolve existence)
    if (resolver_) {
        for (const auto& st : program) {
            if (!st) continue;
            if (st->kind != StmtKind::Import) continue;

            auto* imp = static_cast<ImportStmt*>(st.get());
            std::filesystem::path resolved;
            std::string err;
            if (!resolver_->Resolve(imp->module_path, resolved, err)) {
                // Use stmt->line if available (Stmt has line? not in base; ImportStmt doesn't store line)
                // fallback: report at top of file
                PushError(out, docUri, 0, 0, "Import error: " + err + " (import " + imp->module_path + ")");
            }
        }
    }
}
