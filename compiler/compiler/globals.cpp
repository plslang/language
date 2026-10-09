// compiler/globals.cpp - collect global variables
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

using namespace std;

struct CompileError : public std::exception {
    int code;
    string message;
    vector<string> stack;
    int line;
    CompileError(int c, string msg, int ln = -1)
        : code(c), message(move(msg)), line(ln) {}
    const char* what() const noexcept override { return message.c_str(); }
};

struct Expr {
    enum Kind { NUM, STR, VAR, BINOP } kind;
    long long numValue = 0;
    string strValue;
    string varName;
    char op = 0;
    shared_ptr<Expr> lhs;
    shared_ptr<Expr> rhs;
    int line = 0;
};
using ExprPtr = shared_ptr<Expr>;

struct Stmt {
    enum Kind { PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING } kind;
    bool newline = false;
    ExprPtr printExpr;
    int exitCode = 0;
    string callName;
    string varType;
    string varName;
    ExprPtr varInit;
    string usingModule;
    int line = 0;
};

struct Function {
    string name;
    vector<Stmt> body;
};

struct Program {
    vector<Function> functions;
    int mainIndex = -1;
    bool usesMath = false;
};

struct GlobalInfo {
    string type;
    ExprPtr init;
    string declaredIn;
    int    line = 0;
};

map<string, GlobalInfo> collectGlobals(const Program& prog) {
    map<string, GlobalInfo> globals;

    for (const auto& f : prog.functions) {
        for (const auto& s : f.body) {
            if (s.kind != Stmt::GLOBAL) continue;

            auto it = globals.find(s.varName);
            if (it != globals.end()) {
                CompileError e(1,
                    "failed argument: global '" + s.varName +
                    "' already declared (first in function '" +
                    it->second.declaredIn + "')",
                    s.line);
                e.stack.push_back("Compiler::collectGlobals -> duplicate");
                e.stack.push_back("function " + f.name);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }

            GlobalInfo gi;
            gi.type = s.varType;
            gi.init = s.varInit;
            gi.declaredIn = f.name;
            gi.line = s.line;

            globals[s.varName] = gi;
        }
    }
    return globals;
}