// compiler/emit_loop_ctl.cpp - pass / continue -> break / continue
#include "compiler.hpp"

using namespace std;

string emitPass(const Stmt& s) {
    ostringstream out;
    out << "// code - pass\n";
    out << "break;\n";
    (void)s;
    return out.str();
}

string emitContinue(const Stmt& s) {
    ostringstream out;
    out << "// code - continue\n";
    out << "continue;\n";
    (void)s;
    return out.str();
}