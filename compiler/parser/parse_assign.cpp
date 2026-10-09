// parser/parse_assign.cpp - x = <expr>
#include "parser.hpp"

using namespace std;

Stmt parseAssign(const vector<Token>& toks, size_t& pos, const string& name) {
    const Token& nameTok = toks[pos];
    pos++; // name
    pos++; // '='

    if (pos >= toks.size()) {
        CompileError e(1,
            "failed argument: unexpected end of file in assignment to '" +
            name + "'", nameTok.line);
        e.stack.push_back("Parser::parseAssign -> value");
        throw e;
    }

    ExprPtr val = parseExpr(toks, pos);

    Stmt s;
    s.kind = Stmt::ASSIGN;
    s.varName = name;
    s.varInit = val;
    s.line = nameTok.line;
    return s;
}