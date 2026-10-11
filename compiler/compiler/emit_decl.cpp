// compiler/emit_decl.cpp - emit local variable declaration
#include "compiler.hpp"

using namespace std;

string emitDecl(const Stmt& s, const string& fn, const FuncScope& sc,
                const map<string, GlobalInfo>& globals,
                const map<string, FuncInfo>& funcs)
{
    ostringstream out;

    string init;
    if (s.varInit) {
        init = renderExpr(s.varInit, fn, sc, globals, funcs);
    } else {
        init = defaultFor(s.varType);
    }

    out << "// code - " << s.varType << " " << s.varName << " = ...\n";
    out << "    " << typeFor(s.varType) << " "
        << cppIdentFor(fn, s.varName) << " = " << init << ";\n";

    return out.str();
}