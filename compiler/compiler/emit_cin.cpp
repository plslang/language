// compiler/emit_cin.cpp - emit cin(var) / cinline(var)
#include "compiler.hpp"

using namespace std;

string emitCin(const Stmt& s, const string& fn, const FuncScope& sc,
               const map<string, GlobalInfo>& globals)
{
    ostringstream out;
    string lhs = renderLValue(fn, sc, globals, s.cinName);

    if (s.cinLine) {
        out << "// code - cinline(" << s.cinName << ")\n";
        out << "    getline(cin >> ws, " << lhs << ");\n";
    } else {
        out << "// code - cin(" << s.cinName << ")\n";
        out << "    cin >> " << lhs << ";\n";
    }
    return out.str();
}