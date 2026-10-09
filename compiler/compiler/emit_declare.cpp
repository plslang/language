// compiler/emit_declare.cpp - emit declare marker (bound as parameter)
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

string cppIdentFor(const string& fn, const string& var);

string emitDeclare(const Stmt& s, const string& fn) {
    ostringstream out;
    out << "// code - declare " << s.varType << " " << s.varName << "\n";
    out << "// (bound as reference parameter: "
        << cppIdentFor(fn, s.varName) << ")\n";
    return out.str();
}