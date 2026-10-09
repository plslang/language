// parser/parse_print.cpp - print(...) / println(...)
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

using namespace std;

struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL,
                PLUS, MINUS, STAR, SLASH, END } type;
    string text;
    int line;
    int col;
};

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

ExprPtr parseExpr(const vector<Token>& toks, size_t& pos);

Stmt parsePrint(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    bool nl = (kw.text == "println");
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::LPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected '(' after " + kw.text + ", got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parsePrint");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size()) {
        CompileError e(1,
            "failed argument: unexpected end of file in " + kw.text + "()",
            kw.line);
        e.stack.push_back("Parser::parsePrint");
        throw e;
    }

    ExprPtr arg = parseExpr(toks, pos);

    if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ')' after " + kw.text +
            " argument, got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parsePrint -> close");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    Stmt s;
    s.kind = Stmt::PRINT;
    s.newline = nl;
    s.printExpr = arg;
    s.line = kw.line;
    return s;
}