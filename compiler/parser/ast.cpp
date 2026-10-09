// parser/ast.cpp - AST helpers
#include "parser.hpp"

using namespace std;

ExprPtr makeNum(long long v, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::NUM; e->numValue = v; e->line = line;
    return e;
}
ExprPtr makeStr(const string& s, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::STR; e->strValue = s; e->line = line;
    return e;
}
ExprPtr makeVar(const string& n, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::VAR; e->varName = n; e->line = line;
    return e;
}
ExprPtr makeBin(char op, ExprPtr l, ExprPtr r, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::BINOP; e->op = op;
    e->lhs = move(l); e->rhs = move(r); e->line = line;
    return e;
}