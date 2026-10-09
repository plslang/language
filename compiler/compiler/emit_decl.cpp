// compiler/emit_decl.cpp - emit local variable declaration
#include "compiler.hpp"

using namespace std;

string emitDecl(const Stmt& s, const string& fn, const FuncScope& sc,
                const map<string, GlobalInfo>& globals)
{
    ostringstream out;

    string init;
    if (s.varInit) {
        init = renderExpr(s.varInit, fn, sc, globals);
    } else {
        init = (s.varType == "string") ? "\"\"" : "0";
    }

    out << "// code - " << s.varType << " " << s.varName << " = ...\n";
    out << "    " << typeFor(s.varType) << " "
        << cppIdentFor(fn, s.varName) << " = " << init << ";\n";

    return out.str();
}