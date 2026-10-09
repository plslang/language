// compiler/emit_global.cpp - emit global marker inside function body
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

string cppGlobalIdent(const string& name);

string emitGlobalMarker(const Stmt& s) {
    ostringstream out;
    out << "// code - global " << s.varType << " " << s.varName
        << " = ...\n";
    out << "// (defined above as " << cppGlobalIdent(s.varName)
        << ")\n";
    return out.str();
}