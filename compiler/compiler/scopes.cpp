// compiler/scopes.cpp - per-function scope: locals + declares
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

struct FuncScope {
    map<string,string> vars;
    map<string,string> declareTypes;
    set<string>        declares;
};

map<string, FuncScope> buildScopes(const Program& prog,
                                   const map<string, GlobalInfo>& globals)
{
    map<string, FuncScope> scopes;

    for (const auto& f : prog.functions) {
        FuncScope sc;

        for (const auto& s : f.body) {
            if (s.kind == Stmt::DECL) {
                if (sc.declares.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: local variable '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with declare of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                sc.vars[s.varName] = s.varType;

            } else if (s.kind == Stmt::DECLARE) {
                if (sc.vars.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: declare '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with local variable of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                if (globals.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: declare '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with global of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                sc.declares.insert(s.varName);
                sc.declareTypes[s.varName] = s.varType;
            }
        }

        scopes[f.name] = sc;
    }
    return scopes;
}