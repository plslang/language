// parser/parse_loop_ctl.cpp - pass / continue
#include "parser.hpp"

using namespace std;

Stmt parsePass(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];   // "pass"
    pos++;

    Stmt s;
    s.kind = Stmt::PASS;
    s.line = kw.line;
    return s;
}

Stmt parseContinue(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];   // "continue"
    pos++;

    Stmt s;
    s.kind = Stmt::CONTINUE;
    s.line = kw.line;
    return s;
}