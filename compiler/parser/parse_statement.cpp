// parser/parse_statement.cpp - statement dispatcher
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

// ---------- per-statement parsers ----------
Stmt parsePrint(const vector<Token>& toks, size_t& pos);
Stmt parseExit(const vector<Token>& toks, size_t& pos);
Stmt parseCall(const vector<Token>& toks, size_t& pos);
Stmt parseDeclare(const vector<Token>& toks, size_t& pos);
Stmt parseGlobal(const vector<Token>& toks, size_t& pos);
Stmt parseDecl(const vector<Token>& toks, size_t& pos);
Stmt parseAssign(const vector<Token>& toks, size_t& pos, const string& name);

static Stmt parseUsing(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected module name after 'using', got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseUsing -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string mod = toks[pos].text;
    pos++;

    if (mod != "math") {
        CompileError e(1,
            "failed argument: unknown module '" + mod + "' (expected 'math')",
            kw.line);
        e.stack.push_back("Parser::parseUsing -> module");
        e.stack.push_back("at line " + to_string(kw.line));
        throw e;
    }

    Stmt s;
    s.kind = Stmt::USING;
    s.usingModule = mod;
    s.line = kw.line;
    return s;
}

Stmt parseStatement(const vector<Token>& toks, size_t& pos) {
    const Token& t = toks[pos];

    if (t.type == Token::DOT) {
        return parseCall(toks, pos);
    }

    if (t.type == Token::IDENT) {
        if (t.text == "using") {
            return parseUsing(toks, pos);
        }
        if (t.text == "print" || t.text == "println") {
            return parsePrint(toks, pos);
        }
        if (t.text == "exit") {
            return parseExit(toks, pos);
        }
        if (t.text == "declare") {
            return parseDeclare(toks, pos);
        }
        if (t.text == "global") {
            return parseGlobal(toks, pos);
        }
        if (t.text == "int" || t.text == "string") {
            return parseDecl(toks, pos);
        }
        if (pos + 1 < toks.size() && toks[pos + 1].type == Token::EQUAL) {
            string name = t.text;
            return parseAssign(toks, pos, name);
        }

        CompileError e(1,
            "failed argument: unknown statement starting with '" + t.text + "'",
            t.line);
        e.stack.push_back("Parser::parseStatement -> dispatch");
        e.stack.push_back("at line " + to_string(t.line));
        throw e;
    }

    CompileError e(1,
        "failed argument: unexpected token '" + t.text + "'",
        t.line);
    e.stack.push_back("Parser::parseStatement");
    e.stack.push_back("at line " + to_string(t.line));
    throw e;
}