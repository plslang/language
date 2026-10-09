// compiler/emit_print.cpp - emit print / println
#include "compiler.hpp"

using namespace std;

string emitPrint(const Stmt& s, const string& fn, const FuncScope& sc,
                 const map<string, GlobalInfo>& globals)
{
    ostringstream out;

    out << "// code - " << (s.newline ? "println" : "print") << "(...)\n";
    out << "    cout << " << renderExpr(s.printExpr, fn, sc, globals);
    if (s.newline) out << " << endl";
    out << ";\n";

    return out.str();
}