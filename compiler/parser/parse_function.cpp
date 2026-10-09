// parser/parse_function.cpp - parse functions + top-level parseProgram
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

struct Function {
    string name;
    vector<Stmt> body;
};

struct Program {
    vector<Function> functions;
    int mainIndex = -1;
    bool usesMath = false;
};

// ---------- cross-module prototypes ----------
Stmt parseStatement(const vector<Token>& toks, size_t& pos);

// ---------- parseFunction ----------
Function parseFunction(const vector<Token>& toks, size_t& pos) {
    Function fn;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected function name, got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseFunction -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    fn.name = toks[pos].text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::COLON) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ':' after function name, got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseFunction -> colon");
        e.stack.push_back("function " + fn.name);
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    while (pos < toks.size() && toks[pos].type != Token::END) {
        if (toks[pos].type == Token::IDENT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::COLON)
        {
            break;
        }
        fn.body.push_back(parseStatement(toks, pos));
    }

    return fn;
}

// ---------- parseProgram ----------
Program parseProgram(const vector<Token>& toks) {
    Program prog;
    size_t pos = 0;

    // using directives are only allowed before the first function
    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        // if next is `name :` it's a function -> stop using-block
        if (t.type == Token::IDENT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::COLON)
        {
            break;
        }

        if (t.type == Token::IDENT && t.text == "using") {
            Stmt u = parseStatement(toks, pos);
            prog.usesMath = true; // only 'math' allowed in parseUsing
            continue;
        }

        // anything else at top-level before a function is an error
        CompileError e(1,
            "failed argument: only 'using math' allowed before functions, got '" +
            t.text + "'", t.line);
        e.stack.push_back("Parser::parseProgram -> using-block");
        e.stack.push_back("at line " + to_string(t.line));
        throw e;
    }

    while (pos < toks.size() && toks[pos].type != Token::END) {
        Function fn = parseFunction(toks, pos);

        for (auto& f : prog.functions) {
            if (f.name == fn.name) {
                CompileError e(1,
                    "failed argument: duplicate function '" + fn.name + "'",
                    fn.body.empty() ? toks[pos].line : fn.body.front().line);
                e.stack.push_back("Parser::parseProgram -> duplicate check");
                e.stack.push_back("function " + fn.name);
                throw e;
            }
        }
        prog.functions.push_back(move(fn));
    }

    for (size_t i = 0; i < prog.functions.size(); ++i) {
        if (prog.functions[i].name == "main") {
            prog.mainIndex = (int)i;
            break;
        }
    }
    if (prog.mainIndex < 0) {
        CompileError e(0, "no main function found");
        e.stack.push_back("Parser::parseProgram -> findMain");
        e.stack.push_back("at end of file");
        throw e;
    }

    {
        const Function& m = prog.functions[prog.mainIndex];
        bool hasExit = false;
        for (const auto& s : m.body) {
            if (s.kind == Stmt::EXIT) { hasExit = true; break; }
        }
        if (!hasExit) {
            CompileError e(2, "no return: missing exit() in main");
            e.stack.push_back("Parser::parseProgram -> checkExit");
            e.stack.push_back("at end of main body");
            throw e;
        }
    }

    return prog;
}