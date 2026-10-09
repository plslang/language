// parser/ast.cpp - AST structures shared by all parser files
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

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

// ---------- expressions ----------
struct Expr {
    enum Kind { NUM, STR, VAR, BINOP } kind;

    // NUM
    long long numValue = 0;

    // STR
    string strValue;

    // VAR
    string varName;

    // BINOP
    char op = 0;                 // '+', '-', '*', '/'
    shared_ptr<Expr> lhs;
    shared_ptr<Expr> rhs;

    int line = 0;
};

using ExprPtr = shared_ptr<Expr>;

ExprPtr makeNum(long long v, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::NUM; e->numValue = v; e->line = line;
    return e;
}
ExprPtr makeStr(const string& s, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::STR; e->strValue = s; e->line = line;
    return e;
}
ExprPtr makeVar(const string& n, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::VAR; e->varName = n; e->line = line;
    return e;
}
ExprPtr makeBin(char op, ExprPtr l, ExprPtr r, int line) {
    auto e = make_shared<Expr>();
    e->kind = Expr::BINOP; e->op = op;
    e->lhs = move(l); e->rhs = move(r); e->line = line;
    return e;
}

// ---------- statements ----------
struct Stmt {
    enum Kind { PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING } kind;

    bool newline = false;
    ExprPtr printExpr;            // PRINT

    int exitCode = 0;             // EXIT

    string callName;              // CALL

    string varType;               // DECL / DECLARE / GLOBAL
    string varName;               // DECL / ASSIGN / DECLARE / GLOBAL
    ExprPtr varInit;              // DECL / ASSIGN / GLOBAL initializer

    string usingModule;           // USING

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