// compiler/emit_call.cpp - emit .func(args)
#include <string>
#include <sstream>
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

struct DeclareInfo {
    string type;
    string name;
};

struct Resolved {
    bool found = false;
    string type;
    enum Where { NONE, LOCAL, DECLARE, GLOBAL } where = NONE;
};

Resolved resolveName(const FuncScope& sc,
                     const map<string, GlobalInfo>& globals,
                     const string& name);

string renderLValue(const string& fn, const FuncScope& sc,
                    const map<string, GlobalInfo>& globals, const string& name);

string emitCall(const Stmt& s, const string& fn, const FuncScope& sc,
                const map<string, GlobalInfo>& globals,
                const map<string, vector<DeclareInfo>>& declares)
{
    ostringstream out;
    out << "// code - ." << s.callName << "\n";

    const auto& calleeDecls = declares.at(s.callName);

    vector<string> args;
    for (const auto& d : calleeDecls) {
        Resolved r = resolveName(sc, globals, d.name);
        if (!r.found) {
            CompileError e(1,
                "failed argument: undeclared variable '" + d.name +
                "' at call ." + s.callName + " in function '" + fn + "'",
                s.line);
            e.stack.push_back("Compiler::emitCall -> resolveCall");
            e.stack.push_back("function " + fn + " -> ." + s.callName);
            e.stack.push_back("at line " + to_string(s.line));
            throw e;
        }
        if (r.type != d.type) {
            CompileError e(1,
                "failed argument: type mismatch for '" + d.name +
                "' in ." + s.callName + ": expected " + d.type +
                ", got " + r.type,
                s.line);
            e.stack.push_back("Compiler::emitCall -> resolveCall");
            e.stack.push_back("function " + fn + " -> ." + s.callName);
            e.stack.push_back("at line " + to_string(s.line));
            throw e;
        }
        args.push_back(renderLValue(fn, sc, globals, d.name));
    }

    out << "    fn_" << s.callName << "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) out << ", ";
        out << args[i];
    }
    out << ");\n";

    return out.str();
}