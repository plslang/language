// compiler/emit_if.cpp - emit if / else if / else
#include "compiler.hpp"

using namespace std;

static string pad(int n) {
    return string(n * 4, ' ');
}

string emitIf(const Stmt& s, const string& fn, const FuncScope& sc,
              const map<string, GlobalInfo>& globals,
              const map<string, FuncInfo>& funcs,
              int indent)
{
    ostringstream out;

    out << pad(indent)
        << "if (" << renderExpr(s.cond, fn, sc, globals, funcs) << ") {\n";

    // для thenBody используем currentRet = VOID — так emitReturn
    // внутри if не знает про тип функции. Но по факту return в if
    // возвращает из ВСЕЙ функции. Значит, надо прокинуть currentRet.
    //
    // Проблема: emitIf не знает retType текущей функции. Прокидываем
    // его через funcs по имени fn.
    FuncRet curRet = FuncRet::VOID;
    auto it = funcs.find(fn);
    if (it != funcs.end()) curRet = it->second.retType;

    out << emitStmtBlock(s.thenBody, fn, sc, globals, funcs, indent + 1, curRet);
    out << pad(indent) << "}";

    if (!s.elseBody.empty()) {
        if (s.elseBody.size() == 1 && s.elseBody[0].kind == Stmt::IF) {
            out << " else ";
            string nested = emitIf(s.elseBody[0], fn, sc, globals, funcs, indent);
            string p = pad(indent);
            if (nested.rfind(p, 0) == 0) nested = nested.substr(p.size());
            out << nested;
        } else {
            out << " else {\n";
            out << emitStmtBlock(s.elseBody, fn, sc, globals, funcs, indent + 1, curRet);
            out << pad(indent) << "}";
        }
    }

    out << "\n";
    return out.str();
}