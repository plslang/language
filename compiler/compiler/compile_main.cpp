// compiler/compile_main.cpp - entry point for codegen
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <memory>
#include <stdexcept>

using namespace std;

extern const char* PL_VERSION;
extern const char* PL_LANG_VERSION;

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

struct DeclareInfo {
    string type;
    string name;
};

// ---------- cross-module prototypes ----------
map<string, GlobalInfo> collectGlobals(const Program& prog);
map<string, FuncScope>  buildScopes(const Program& prog,
                                    const map<string, GlobalInfo>& globals);
map<string, vector<DeclareInfo>> buildDeclares(const Program& prog);

void semanticCheck(const Program& prog,
                   const map<string, GlobalInfo>& globals,
                   const map<string, FuncScope>& scopes,
                   const map<string, vector<DeclareInfo>>& declares);

string emitBanner(const string& srcName);
string emitIncludes(bool usesMath);
string emitGlobals(const map<string, GlobalInfo>& globals);
string emitForwardDeclarations(const Program& prog,
                               const map<string, vector<DeclareInfo>>& declares);
string emitFunction(const Function& f,
                    const FuncScope& sc,
                    const map<string, GlobalInfo>& globals,
                    const map<string, vector<DeclareInfo>>& declares);
string emitMainEntry(const map<string, vector<DeclareInfo>>& declares);

string isoDateNow();

string generateCpp(const Program& prog, const string& srcName) {
    ostringstream out;

    auto globals  = collectGlobals(prog);
    auto scopes   = buildScopes(prog, globals);
    auto declares = buildDeclares(prog);

    semanticCheck(prog, globals, scopes, declares);

    out << emitBanner(srcName);
    out << emitIncludes(prog.usesMath);
    out << emitGlobals(globals);
    out << emitForwardDeclarations(prog, declares);

    for (const auto& f : prog.functions) {
        out << emitFunction(f, scopes[f.name], globals, declares);
    }

    out << emitMainEntry(declares);

    return out.str();
}