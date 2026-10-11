// parser/parse_decl.cpp - int x = <expr> / int x / bool b = ...
#include "parser.hpp"

using namespace std;

Stmt parseDecl(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    string type = kw.text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected variable name after '" + type +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseDecl -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string name = toks[pos].text;
    pos++;

    Stmt s;
    s.kind = Stmt::DECL;
    s.varType = type;
    s.varName = name;
    s.varInit = nullptr;
    s.line = kw.line;

    if (pos >= toks.size() || toks[pos].type != Token::EQUAL) {
        return s;
    }
    pos++;

    if (pos >= toks.size()) {
        CompileError e(1,
            "failed argument: unexpected end of file in declaration of '" +
            name + "'", kw.line);
        e.stack.push_back("Parser::parseDecl -> value");
        throw e;
    }

    s.varInit = parseExpr(toks, pos);
    return s;
}