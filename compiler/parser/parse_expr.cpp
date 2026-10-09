// parser/parse_expr.cpp - recursive-descent expression parser
// grammar:
//   expr   := term (('+' | '-') term)*
//   term   := atom (('*' | '/') atom)*
//   atom   := NUMBER | STRING | IDENT | '(' expr ')'
#include <string>
#include <vector>
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

struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL,
                PLUS, MINUS, STAR, SLASH, END } type;
    string text;
    int line;
    int col;
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

ExprPtr makeNum(long long v, int line);
ExprPtr makeStr(const string& s, int line);
ExprPtr makeVar(const string& n, int line);
ExprPtr makeBin(char op, ExprPtr l, ExprPtr r, int line);

ExprPtr parseExpr(const vector<Token>& toks, size_t& pos);

// ---------- atom ----------
static ExprPtr parseAtom(const vector<Token>& toks, size_t& pos) {
    if (pos >= toks.size()) {
        CompileError e(1, "failed argument: unexpected end of file in expression");
        e.stack.push_back("Parser::parseExpr -> atom");
        throw e;
    }

    const Token& t = toks[pos];

    if (t.type == Token::NUMBER) {
        pos++;
        return makeNum(stoll(t.text), t.line);
    }
    if (t.type == Token::STRING) {
        pos++;
        return makeStr(t.text, t.line);
    }
    if (t.type == Token::IDENT) {
        pos++;
        return makeVar(t.text, t.line);
    }
    if (t.type == Token::LPAREN) {
        pos++;
        ExprPtr inner = parseExpr(toks, pos);
        if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
            const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
            CompileError e(1,
                "failed argument: expected ')' in expression, got '" +
                tk.text + "'", tk.line);
            e.stack.push_back("Parser::parseExpr -> atom -> close");
            e.stack.push_back("at line " + to_string(tk.line));
            throw e;
        }
        pos++;
        return inner;
    }

    CompileError e(1,
        "failed argument: unexpected token in expression: '" + t.text + "'",
        t.line);
    e.stack.push_back("Parser::parseExpr -> atom");
    e.stack.push_back("at line " + to_string(t.line));
    throw e;
}

// ---------- term: * / ----------
static ExprPtr parseTerm(const vector<Token>& toks, size_t& pos) {
    ExprPtr left = parseAtom(toks, pos);
    while (pos < toks.size() &&
           (toks[pos].type == Token::STAR || toks[pos].type == Token::SLASH))
    {
        char op = (toks[pos].type == Token::STAR) ? '*' : '/';
        int line = toks[pos].line;
        pos++;
        ExprPtr right = parseAtom(toks, pos);
        left = makeBin(op, left, right, line);
    }
    return left;
}

// ---------- expr: + - ----------
ExprPtr parseExpr(const vector<Token>& toks, size_t& pos) {
    ExprPtr left = parseTerm(toks, pos);
    while (pos < toks.size() &&
           (toks[pos].type == Token::PLUS || toks[pos].type == Token::MINUS))
    {
        char op = (toks[pos].type == Token::PLUS) ? '+' : '-';
        int line = toks[pos].line;
        pos++;
        ExprPtr right = parseTerm(toks, pos);
        left = makeBin(op, left, right, line);
    }
    return left;
}