// compiler/emit_expr.cpp - render Expr tree to C++
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <memory>
#include <stdexcept>
#include <vector>

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

string cppStringLiteral(const string& s);
string cppIdentFor(const string& fn, const string& var);
string cppGlobalIdent(const string& name);
Resolved resolveName(const FuncScope& sc,
                     const map<string, GlobalInfo>& globals,
                     const string& name);

string renderExpr(const ExprPtr& e, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals)
{
    if (!e) {
        CompileError err(1, "internal: null expression");
        err.stack.push_back("Compiler::renderExpr");
        throw err;
    }

    switch (e->kind) {
        case Expr::NUM:
            return to_string(e->numValue);

        case Expr::STR:
            return cppStringLiteral(e->strValue);

        case Expr::VAR: {
            Resolved r = resolveName(sc, globals, e->varName);
            switch (r.where) {
                case Resolved::LOCAL:
                case Resolved::DECLARE:
                    return cppIdentFor(fn, e->varName);
                case Resolved::GLOBAL:
                    return cppGlobalIdent(e->varName);
                default:
                    return e->varName;
            }
        }

        case Expr::BINOP: {
            string l = renderExpr(e->lhs, fn, sc, globals);
            string r = renderExpr(e->rhs, fn, sc, globals);
            ostringstream out;
            out << "(" << l << " " << e->op << " " << r << ")";
            return out.str();
        }
    }
    return "";
}