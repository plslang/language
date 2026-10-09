// parser/parse_exit.cpp - exit(N)
#include "parser.hpp"

using namespace std;

Stmt parseExit(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::LPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected '(' after exit, got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseExit");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::NUMBER) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected number in exit(), got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseExit -> number");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    const Token& num = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ')' after exit argument, got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseExit -> close");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    Stmt s;
    s.kind = Stmt::EXIT;
    s.exitCode = stoi(num.text);
    s.line = kw.line;
    return s;
}