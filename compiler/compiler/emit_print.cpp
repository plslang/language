// compiler/emit_print.cpp - emit print / println
#include "compiler.hpp"

using namespace std;

string emitPrint(const Stmt& s, const string& fn, const FuncScope& sc,
                 const map<string, GlobalInfo>& globals,
                 const map<string, FuncInfo>& funcs)
{
    ostringstream out;

    string t = inferExprType(s.printExpr, sc, globals, funcs, fn);

    out << "// code - " << (s.newline ? "println" : "print") << "(...)\n";
    out << "    cout << ";
    if (t == "bool") out << "boolalpha << ";
    out << renderExpr(s.printExpr, fn, sc, globals, funcs);
    if (s.newline) out << " << endl";
    out << ";\n";

    return out.str();
}