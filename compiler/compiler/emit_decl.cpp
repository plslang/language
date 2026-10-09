// compiler/emit_decl.cpp - emit local variable declaration
#include <string>
#include <sstream>
#include <map>
#include <set>
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

string cppIdentFor(const string& fn, const string& var);
string typeFor(const string& t);
string renderExpr(const ExprPtr& e, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals);

string emitDecl(const Stmt& s, const string& fn, const FuncScope& sc,
                const map<string, GlobalInfo>& globals)
{
    ostringstream out;

    out << "// code - " << s.varType << " " << s.varName << " = ...\n";
    out << "    " << typeFor(s.varType) << " "
        << cppIdentFor(fn, s.varName) << " = "
        << renderExpr(s.varInit, fn, sc, globals) << ";\n";

    return out.str();
}