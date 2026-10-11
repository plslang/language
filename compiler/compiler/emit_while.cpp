// compiler/emit_while.cpp - emit while cond: ... { ... }
#include "compiler.hpp"

using namespace std;

static string pad(int n) {
    return string(n * 4, ' ');
}

string emitWhile(const Stmt& s, const string& fn, const FuncScope& sc,
                 const map<string, GlobalInfo>& globals,
                 const map<string, FuncInfo>& funcs,
                 int indent)
{
    ostringstream out;

    out << pad(indent)
        << "while (" << renderExpr(s.cond, fn, sc, globals, funcs) << ") {\n";

    // curRet — как в emitIf: чтобы return внутри while знал тип функции.
    FuncRet curRet = FuncRet::VOID;
    auto it = funcs.find(fn);
    if (it != funcs.end()) curRet = it->second.retType;

    out << emitStmtBlock(s.whileBody, fn, sc, globals, funcs,
                         indent + 1, curRet);

    out << pad(indent) << "}\n";
    return out.str();
}