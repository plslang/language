// compiler/emit_global.cpp - emit global marker inside function body
#include "compiler.hpp"

using namespace std;

string emitGlobalMarker(const Stmt& s) {
    ostringstream out;
    out << "// code - global " << s.varType << " " << s.varName << " = ...\n";
    out << "// (defined above as " << cppGlobalIdent(s.varName) << ")\n";
    return out.str();
}