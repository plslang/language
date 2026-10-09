// compiler/emit_assign.cpp - emit assignment
#include "compiler.hpp"

using namespace std;

string emitAssign(const Stmt& s, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals)
{
    ostringstream out;
    string lhs = renderLValue(fn, sc, globals, s.varName);

    out << "// code - " << s.varName << " = ...\n";
    out << "    " << lhs << " = "
        << renderExpr(s.varInit, fn, sc, globals) << ";\n";

    return out.str();
}