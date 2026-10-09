// compiler/emit_declare.cpp - emit declare marker (bound as parameter)
#include "compiler.hpp"

using namespace std;

string emitDeclare(const Stmt& s, const string& fn) {
    ostringstream out;
    out << "// code - declare " << s.varType << " " << s.varName << "\n";
    out << "// (bound as reference parameter: "
        << cppIdentFor(fn, s.varName) << ")\n";
    return out.str();
}