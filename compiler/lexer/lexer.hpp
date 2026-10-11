// lexer/lexer.hpp - shared types and prototypes for the lexer
#pragma once

#include "../common.hpp"

#include <string>
#include <vector>

// ---------- token ----------
struct Token {
    enum Type {
        IDENT, NUMBER, STRING,
        LPAREN, RPAREN, COLON, DOT, EQUAL,
        PLUS, MINUS, STAR, SLASH,
        EQEQ, NEQ, LT, GT, LTE, GTE,
        BANG,
        AT,
        END
    };

    Type type;
    std::string text;
    int line = 0;
    int col  = 0;
};

// ---------- entry point ----------
std::vector<Token> lexSource(const std::string& src);

// ---------- per-lexer helpers ----------
bool tryLexIdent (const std::string& src, std::size_t pos, Token& out,
                  std::size_t& newPos, int& line, int& col);
bool tryLexNumber(const std::string& src, std::size_t pos, Token& out,
                  std::size_t& newPos, int& line, int& col);
bool tryLexString(const std::string& src, std::size_t pos, Token& out,
                  std::size_t& newPos, int& line, int& col);
bool tryLexPunct (const std::string& src, std::size_t pos, Token& out,
                  std::size_t& newPos, int& line, int& col);

// comment skipper
std::size_t skipComment(const std::string& src, std::size_t pos, int& col);