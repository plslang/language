// compiler/emit_exit.cpp - emit exit(N)
#include <string>
#include <sstream>

using namespace std;

struct Stmt {
    enum Kind { PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING } kind;
    bool newline = false;
    void* printExpr = nullptr;
    int exitCode = 0;
    string callName;
    string varType;
    string varName;
    void* varInit = nullptr;
    string usingModule;
    int line = 0;
};

string emitExit(const Stmt& s) {
    ostringstream out;
    out << "// code - exit(" << s.exitCode << ")\n";
    out << "    exit(" << s.exitCode << ");\n";
    return out.str();
}