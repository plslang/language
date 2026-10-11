// compiler/emit_return.cpp - emit return [expr]
#include "compiler.hpp"

using namespace std;

string emitReturn(const Stmt& s, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals,
                  const map<string, FuncInfo>& funcs,
                  FuncRet currentRet)
{
    ostringstream out;

    if (currentRet == FuncRet::VOID) {
        // return; (без значения)
        out << "// code - return\n";
        out << "    return;\n";
        return out.str();
    }

    // bool / int — всегда с выражением
    if (!s.returnExpr) {
        // parse_function.cpp уже отсёк это, но подстрахуемся
        CompileError e(54,
            "function '" + fn + "' with return type " + retName(currentRet) +
            " must return a value", s.line);
        e.stack.push_back("Compiler::emitReturn");
        e.stack.push_back("at line " + to_string(s.line));
        throw e;
    }

    if (currentRet == FuncRet::BOOL) {
        // нормализуем: любое ненулевое → true, 0 → false
        // в C++ bool сам нормализует, но обернём для явности
        out << "// code - return (bool)\n";
        out << "    return ("
            << renderExpr(s.returnExpr, fn, sc, globals, funcs)
            << ") != 0;\n";
        return out.str();
    }

    // int
    out << "// code - return\n";
    out << "    return "
        << renderExpr(s.returnExpr, fn, sc, globals, funcs) << ";\n";
    return out.str();
}