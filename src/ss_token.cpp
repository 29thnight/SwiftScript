#include "ss_token.hpp"
#include <unordered_map>

namespace swiftscript {

bool Token::is_keyword() const {
    return type >= TokenType::Func && type <= TokenType::Super;
}

bool Token::is_operator() const {
    return type >= TokenType::Plus && type <= TokenType::Arrow;
}

bool Token::is_literal() const {
    return (type >= TokenType::Integer && type <= TokenType::Null) ||
           type == TokenType::String;
}

std::string Token::to_string() const {
    std::string result = TokenUtils::token_type_name(type);
    if (!lexeme.empty()) {
        result += " '" + std::string(lexeme) + "'";
    }
    result += " at line " + std::to_string(line) + ":" + std::to_string(column);
    return result;
}

const char* TokenUtils::token_type_name(TokenType type) {
    switch (type) {
        case TokenType::Eof: return "EOF";
        case TokenType::Error: return "ERROR";
        case TokenType::Comment: return "COMMENT";
        case TokenType::Integer: return "INTEGER";
        case TokenType::Float: return "FLOAT";
        case TokenType::String: return "STRING";
        case TokenType::True: return "TRUE";
        case TokenType::False: return "FALSE";
        case TokenType::Null: return "NULL";
        case TokenType::Identifier: return "IDENTIFIER";
        
        // Keywords
        case TokenType::Func: return "FUNC";
        case TokenType::Class: return "CLASS";
        case TokenType::Struct: return "STRUCT";
        case TokenType::Enum: return "ENUM";
        case TokenType::Protocol: return "PROTOCOL";
        case TokenType::Extension: return "EXTENSION";
        case TokenType::Var: return "VAR";
        case TokenType::Let: return "LET";
        case TokenType::Weak: return "WEAK";
        case TokenType::Unowned: return "UNOWNED";
        case TokenType::If: return "IF";
        case TokenType::Else: return "ELSE";
        case TokenType::Switch: return "SWITCH";
        case TokenType::Case: return "CASE";
        case TokenType::Default: return "DEFAULT";
        case TokenType::For: return "FOR";
        case TokenType::While: return "WHILE";
        case TokenType::Repeat: return "REPEAT";
        case TokenType::Break: return "BREAK";
        case TokenType::Continue: return "CONTINUE";
        case TokenType::Return: return "RETURN";
        case TokenType::In: return "IN";
        case TokenType::Import: return "IMPORT";
        case TokenType::Public: return "PUBLIC";
        case TokenType::Private: return "PRIVATE";
        case TokenType::Internal: return "INTERNAL";
        case TokenType::Static: return "STATIC";
        case TokenType::Override: return "OVERRIDE";
        case TokenType::Init: return "INIT";
        case TokenType::Deinit: return "DEINIT";
        case TokenType::Self: return "SELF";
        case TokenType::Super: return "SUPER";
        
        // Operators
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Percent: return "PERCENT";
        case TokenType::Equal: return "EQUAL";
        case TokenType::PlusEqual: return "PLUS_EQUAL";
        case TokenType::MinusEqual: return "MINUS_EQUAL";
        case TokenType::StarEqual: return "STAR_EQUAL";
        case TokenType::SlashEqual: return "SLASH_EQUAL";
        case TokenType::EqualEqual: return "EQUAL_EQUAL";
        case TokenType::NotEqual: return "NOT_EQUAL";
        case TokenType::Less: return "LESS";
        case TokenType::Greater: return "GREATER";
        case TokenType::LessEqual: return "LESS_EQUAL";
        case TokenType::GreaterEqual: return "GREATER_EQUAL";
        case TokenType::And: return "AND";
        case TokenType::Or: return "OR";
        case TokenType::Not: return "NOT";
        case TokenType::BitwiseAnd: return "BITWISE_AND";
        case TokenType::BitwiseOr: return "BITWISE_OR";
        case TokenType::BitwiseXor: return "BITWISE_XOR";
        case TokenType::BitwiseNot: return "BITWISE_NOT";
        case TokenType::LeftShift: return "LEFT_SHIFT";
        case TokenType::RightShift: return "RIGHT_SHIFT";
        case TokenType::Question: return "QUESTION";
        case TokenType::Colon: return "COLON";
        case TokenType::Arrow: return "ARROW";
        
        // Delimiters
        case TokenType::LeftParen: return "LEFT_PAREN";
        case TokenType::RightParen: return "RIGHT_PAREN";
        case TokenType::LeftBrace: return "LEFT_BRACE";
        case TokenType::RightBrace: return "RIGHT_BRACE";
        case TokenType::LeftBracket: return "LEFT_BRACKET";
        case TokenType::RightBracket: return "RIGHT_BRACKET";
        case TokenType::Comma: return "COMMA";
        case TokenType::Dot: return "DOT";
        case TokenType::Semicolon: return "SEMICOLON";
        
        // Range operators
        case TokenType::Range: return "RANGE";
        case TokenType::RangeInclusive: return "RANGE_INCLUSIVE";
        
        default: return "UNKNOWN";
    }
}

TokenType TokenUtils::keyword_type(std::string_view str) {
    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"func", TokenType::Func},
        {"class", TokenType::Class},
        {"struct", TokenType::Struct},
        {"enum", TokenType::Enum},
        {"protocol", TokenType::Protocol},
        {"extension", TokenType::Extension},
        {"var", TokenType::Var},
        {"let", TokenType::Let},
        {"weak", TokenType::Weak},
        {"unowned", TokenType::Unowned},
        {"if", TokenType::If},
        {"else", TokenType::Else},
        {"switch", TokenType::Switch},
        {"case", TokenType::Case},
        {"default", TokenType::Default},
        {"for", TokenType::For},
        {"while", TokenType::While},
        {"repeat", TokenType::Repeat},
        {"break", TokenType::Break},
        {"continue", TokenType::Continue},
        {"return", TokenType::Return},
        {"in", TokenType::In},
        {"import", TokenType::Import},
        {"public", TokenType::Public},
        {"private", TokenType::Private},
        {"internal", TokenType::Internal},
        {"static", TokenType::Static},
        {"override", TokenType::Override},
        {"init", TokenType::Init},
        {"deinit", TokenType::Deinit},
        {"self", TokenType::Self},
        {"super", TokenType::Super},
        {"true", TokenType::True},
        {"false", TokenType::False},
        {"null", TokenType::Null},
    };
    
    auto it = keywords.find(str);
    return it != keywords.end() ? it->second : TokenType::Identifier;
}

bool TokenUtils::is_keyword(std::string_view str) {
    return keyword_type(str) != TokenType::Identifier;
}

bool TokenUtils::is_assignment_operator(TokenType type) {
    return type == TokenType::Equal ||
           type == TokenType::PlusEqual ||
           type == TokenType::MinusEqual ||
           type == TokenType::StarEqual ||
           type == TokenType::SlashEqual;
}

bool TokenUtils::is_comparison_operator(TokenType type) {
    return type == TokenType::EqualEqual ||
           type == TokenType::NotEqual ||
           type == TokenType::Less ||
           type == TokenType::Greater ||
           type == TokenType::LessEqual ||
           type == TokenType::GreaterEqual;
}

bool TokenUtils::is_binary_operator(TokenType type) {
    return (type >= TokenType::Plus && type <= TokenType::RightShift) ||
           type == TokenType::Question;
}

bool TokenUtils::is_unary_operator(TokenType type) {
    return type == TokenType::Minus ||
           type == TokenType::Not ||
           type == TokenType::BitwiseNot;
}

int TokenUtils::operator_precedence(TokenType type) {
    switch (type) {
        // Highest precedence
        case TokenType::Star:
        case TokenType::Slash:
        case TokenType::Percent:
            return 13;
            
        case TokenType::Plus:
        case TokenType::Minus:
            return 12;
            
        case TokenType::LeftShift:
        case TokenType::RightShift:
            return 11;
            
        case TokenType::Less:
        case TokenType::Greater:
        case TokenType::LessEqual:
        case TokenType::GreaterEqual:
            return 9;
            
        case TokenType::EqualEqual:
        case TokenType::NotEqual:
            return 8;
            
        case TokenType::BitwiseAnd:
            return 7;
            
        case TokenType::BitwiseXor:
            return 6;
            
        case TokenType::BitwiseOr:
            return 5;
            
        case TokenType::And:
            return 4;
            
        case TokenType::Or:
            return 3;
            
        case TokenType::Question:  // Ternary
            return 2;
            
        case TokenType::Equal:
        case TokenType::PlusEqual:
        case TokenType::MinusEqual:
        case TokenType::StarEqual:
        case TokenType::SlashEqual:
            return 1;
            
        default:
            return 0;
    }
}

} // namespace swiftscript
