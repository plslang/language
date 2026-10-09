// compiler/resolve.cpp - name resolution and render helpers
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

struct Resolved {
    bool found = false;
    string type;
    enum Where { NONE, LOCAL, DECLARE, GLOBAL } where = NONE;
};

// ---------- from config.cpp ----------
string cppIdentFor(const string& fn, const string& var);
string cppGlobalIdent(const string& name);
bool   isNumericLiteral(const string& s);

// ---------- resolveName ----------
Resolved resolveName(const FuncScope& sc,
                     const map<string, GlobalInfo>& globals,
                     const string& name)
{
    Resolved r;
    auto it = sc.vars.find(name);
    if (it != sc.vars.end()) {
        r.found = true; r.type = it->second; r.where = Resolved::LOCAL; return r;
    }
    auto it2 = sc.declareTypes.find(name);
    if (it2 != sc.declareTypes.end()) {
        r.found = true; r.type = it2->second; r.where = Resolved::DECLARE; return r;
    }
    auto it3 = globals.find(name);
    if (it3 != globals.end()) {
        r.found = true; r.type = it3->second.type; r.where = Resolved::GLOBAL; return r;
    }
    return r;
}

// ---------- renderLValue ----------
string renderLValue(const string& fn,
                    const FuncScope& sc,
                    const map<string, GlobalInfo>& globals,
                    const string& name)
{
    Resolved r = resolveName(sc, globals, name);
    switch (r.where) {
        case Resolved::LOCAL:
        case Resolved::DECLARE: return cppIdentFor(fn, name);
        case Resolved::GLOBAL:  return cppGlobalIdent(name);
        default:                return cppIdentFor(fn, name);
    }
}