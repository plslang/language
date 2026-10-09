// parser/parse_cin.cpp - cin(var) / cinline(var)
#include "parser.hpp"

using namespace std;

Stmt parseCin(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    bool isLine = (kw.text == "cinline");
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::LPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected '(' after " + kw.text + ", got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseCin");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected variable name in " + kw.text +
            "(), got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseCin -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string name = toks[pos].text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ')' after " + kw.text +
            " argument, got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseCin -> close");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    Stmt s;
    s.kind = Stmt::CIN;
    s.cinName = name;
    s.cinLine = isLine;
    s.line = kw.line;
    return s;
}