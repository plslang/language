// parser/parse_return.cpp - return [expr]
#include "parser.hpp"

using namespace std;

Stmt parseReturn(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];   // "return"
    pos++;

    Stmt s;
    s.kind = Stmt::RETURN;
    s.line = kw.line;
    s.returnExpr = nullptr;

    if (pos >= toks.size()) return s;

    Token::Type nt = toks[pos].type;

    bool startsExpr =
        (nt == Token::NUMBER || nt == Token::STRING ||
         nt == Token::IDENT  || nt == Token::DOT    ||
         nt == Token::LPAREN || nt == Token::BANG);

    if (startsExpr && nt == Token::IDENT) {
        const string& txt = toks[pos].text;
        if (txt == "print"    || txt == "println"  ||
            txt == "exit"     || txt == "cin"      ||
            txt == "cinline"  || txt == "if"       ||
            txt == "else"     || txt == "return"   ||
            txt == "int"      || txt == "string"   ||
            txt == "bool"     || txt == "global"   ||
            txt == "declare"  || txt == "using"    ||
            txt == "pass"     || txt == "continue" ||
            txt == "while")
        {
            startsExpr = false;
        }
    }

    // [FIX] IDENT ':' или IDENT '(' '!' — это начало новой функции,
    // а не выражение. Значит return без значения.
    if (startsExpr && nt == Token::IDENT && pos + 1 < toks.size()) {
        Token::Type nt2 = toks[pos + 1].type;
        if (nt2 == Token::COLON) {
            startsExpr = false;
        } else if (nt2 == Token::LPAREN &&
                   pos + 2 < toks.size() &&
                   toks[pos + 2].type == Token::BANG)
        {
            startsExpr = false;
        }
    }

    if (startsExpr) {
        s.returnExpr = parseExpr(toks, pos);
    }

    return s;
}