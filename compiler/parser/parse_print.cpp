// parser/parse_print.cpp - print(...) / println(...)
#include "parser.hpp"

using namespace std;

Stmt parsePrint(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    bool nl = (kw.text == "println");
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::LPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected '(' after " + kw.text + ", got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parsePrint");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size()) {
        CompileError e(1,
            "failed argument: unexpected end of file in " + kw.text + "()",
            kw.line);
        e.stack.push_back("Parser::parsePrint");
        throw e;
    }

    ExprPtr arg = parseExpr(toks, pos);

    if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ')' after " + kw.text +
            " argument, got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parsePrint -> close");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    Stmt s;
    s.kind = Stmt::PRINT;
    s.newline = nl;
    s.printExpr = arg;
    s.line = kw.line;
    return s;
}