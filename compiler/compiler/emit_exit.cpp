// compiler/emit_exit.cpp - emit exit(N)
#include "compiler.hpp"

using namespace std;

string emitExit(const Stmt& s) {
    ostringstream out;
    out << "// code - exit(" << s.exitCode << ")\n";
    out << "    exit(" << s.exitCode << ");\n";
    return out.str();
}