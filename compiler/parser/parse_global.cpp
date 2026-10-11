// parser/parse_global.cpp - global <type> <name> [= <expr>]
#include "parser.hpp"

using namespace std;

Stmt parseGlobal(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT ||
        (toks[pos].text != "int" && toks[pos].text != "string" &&
         toks[pos].text != "bool"))
    {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: global requires type (int|string|bool), got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseGlobal -> type");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string type = toks[pos].text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: global requires variable name, got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseGlobal -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string name = toks[pos].text;
    pos++;

    Stmt s;
    s.kind = Stmt::GLOBAL;
    s.varType = type;
    s.varName = name;
    s.varInit = nullptr;
    s.line = kw.line;

    // '=' optional
    if (pos >= toks.size() || toks[pos].type != Token::EQUAL) {
        return s;
    }
    pos++;

    if (pos >= toks.size()) {
        CompileError e(1,
            "failed argument: unexpected end of file in global initializer",
            kw.line);
        e.stack.push_back("Parser::parseGlobal -> value");
        throw e;
    }

    s.varInit = parseExpr(toks, pos);
    return s;
}