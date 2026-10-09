// compiler/declares.cpp - per-function ordered declare list
#include <string>
#include <vector>
#include <map>
#include <memory>

using namespace std;

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

struct DeclareInfo {
    string type;
    string name;
};

map<string, vector<DeclareInfo>> buildDeclares(const Program& prog) {
    map<string, vector<DeclareInfo>> m;

    for (const auto& f : prog.functions) {
        vector<DeclareInfo> v;
        for (const auto& s : f.body) {
            if (s.kind == Stmt::DECLARE) {
                v.push_back({s.varType, s.varName});
            }
        }
        m[f.name] = v;
    }
    return m;
}