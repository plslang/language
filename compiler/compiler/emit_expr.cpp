// compiler/emit_expr.cpp - render Expr tree to C++
#include "compiler.hpp"

using namespace std;

string renderExpr(const ExprPtr& e, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals)
{
    if (!e) {
        CompileError err(1, "internal: null expression");
        err.stack.push_back("Compiler::renderExpr");
        throw err;
    }

    switch (e->kind) {
        case Expr::NUM:
            return to_string(e->numValue);
        case Expr::STR:
            return cppStringLiteral(e->strValue);
        case Expr::VAR: {
            Resolved r = resolveName(sc, globals, e->varName);
            switch (r.where) {
                case Resolved::LOCAL:
                case Resolved::DECLARE: return cppIdentFor(fn, e->varName);
                case Resolved::GLOBAL:  return cppGlobalIdent(e->varName);
                default:                return e->varName;
            }
        }
        case Expr::BINOP: {
            string l = renderExpr(e->lhs, fn, sc, globals);
            string r = renderExpr(e->rhs, fn, sc, globals);
            ostringstream out;
            out << "(" << l << " " << e->op << " " << r << ")";
            return out.str();
        }
    }
    return "";
}