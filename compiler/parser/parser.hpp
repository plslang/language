// parser/parser.hpp - AST + parser prototypes
#pragma once

#include "../common.hpp"
#include "../lexer/lexer.hpp"

#include <string>
#include <vector>
#include <memory>

// ---------- expressions ----------
struct Expr {
    enum Kind { NUM, STR, VAR, BINOP } kind;

    long long numValue = 0;       // NUM
    std::string strValue;         // STR
    std::string varName;          // VAR
    char op = 0;                  // BINOP: '+','-','*','/','=','!','<','>','L','G'
    std::shared_ptr<Expr> lhs;    // BINOP
    std::shared_ptr<Expr> rhs;    // BINOP

    int line = 0;
};
using ExprPtr = std::shared_ptr<Expr>;

ExprPtr makeNum(long long v, int line);
ExprPtr makeStr(const std::string& s, int line);
ExprPtr makeVar(const std::string& n, int line);
ExprPtr makeBin(char op, ExprPtr l, ExprPtr r, int line);

// ---------- statements ----------
struct Stmt {
    enum Kind {
        PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING, CIN, IF, DIRECTIVE
    } kind;

    bool newline = false;         // PRINT
    ExprPtr printExpr;            // PRINT

    int exitCode = 0;             // EXIT

    std::string callName;         // CALL

    std::string varType;          // DECL / DECLARE / GLOBAL
    std::string varName;          // DECL / ASSIGN / DECLARE / GLOBAL
    ExprPtr varInit;              // DECL / ASSIGN / GLOBAL

    std::string usingModule;      // USING

    std::string cinName;          // CIN
    bool        cinLine = false;  // CIN

    // IF
    ExprPtr cond;                 // IF: condition
    std::vector<Stmt> thenBody;   // IF: then branch
    std::vector<Stmt> elseBody;   // IF: else branch (may be empty)

    // DIRECTIVE
    std::string directiveName;    // DIRECTIVE: "ifblock" | "endblock" | ...

    int line = 0;
};

// ---------- functions / program ----------
struct Function {
    std::string name;
    std::vector<Stmt> body;
};

struct Program {
    std::vector<Function> functions;
    int mainIndex = -1;
    bool usesMath = false;
};

// ---------- top-level entry ----------
Program parseProgram(const std::vector<Token>& toks);

// ---------- per-statement parsers ----------
Stmt parseStatement(const std::vector<Token>& toks, std::size_t& pos);
Stmt parsePrint    (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseExit     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseCall     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseDeclare  (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseGlobal   (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseDecl     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseAssign   (const std::vector<Token>& toks, std::size_t& pos, const std::string& name);
Stmt parseCin      (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseIf       (const std::vector<Token>& toks, std::size_t& pos, int blockLine = -1);
Stmt parseDirective(const std::vector<Token>& toks, std::size_t& pos);

// ---------- expression parser ----------
ExprPtr parseExpr(const std::vector<Token>& toks, std::size_t& pos);
ExprPtr parseCmp (const std::vector<Token>& toks, std::size_t& pos);