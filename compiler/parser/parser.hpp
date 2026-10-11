// parser/parser.hpp - AST + parser prototypes
#pragma once

#include "../common.hpp"
#include "../lexer/lexer.hpp"

#include <string>
#include <vector>
#include <memory>

// ---------- return types ----------
enum class FuncRet {
    VOID,
    BOOL,
    INT
};

std::string retName(FuncRet r);

// ---------- expressions ----------
struct Expr {
    enum Kind { NUM, STR, BOOL, VAR, BINOP, CALL } kind;

    long long   numValue = 0;        // NUM
    std::string strValue;            // STR
    bool        boolValue = false;   // BOOL
    std::string varName;             // VAR
    std::string callName;            // CALL
    char        op = 0;              // BINOP
    std::shared_ptr<Expr> lhs;       // BINOP
    std::shared_ptr<Expr> rhs;       // BINOP

    int line = 0;
};
using ExprPtr = std::shared_ptr<Expr>;

ExprPtr makeNum (long long v, int line);
ExprPtr makeStr (const std::string& s, int line);
ExprPtr makeBool(bool v, int line);
ExprPtr makeVar (const std::string& n, int line);
ExprPtr makeBin (char op, ExprPtr l, ExprPtr r, int line);
ExprPtr makeCall(const std::string& name, int line);

// ---------- statements ----------
struct Stmt {
    // [WHILE] добавлены WHILE, PASS, CONTINUE
    enum Kind {
        PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING, CIN, IF,
        WHILE, PASS, CONTINUE,
        DIRECTIVE, RETURN
    } kind;

    bool newline = false;         // PRINT
    ExprPtr printExpr;            // PRINT

    int exitCode = 0;             // EXIT

    std::string callName;         // CALL

    // DECL / ASSIGN / DECLARE / GLOBAL
    std::string varType;          // "int" | "string" | "bool"
    std::string varName;
    ExprPtr varInit;

    std::string usingModule;      // USING

    std::string cinName;          // CIN
    bool        cinLine = false;

    // IF
    ExprPtr cond;
    std::vector<Stmt> thenBody;
    std::vector<Stmt> elseBody;

    // [WHILE] WHILE
    std::vector<Stmt> whileBody;
    int whileBlockLine = -1;      // строка '@whileblock'

    // DIRECTIVE
    std::string directiveName;

    // RETURN
    ExprPtr returnExpr;           // may be nullptr

    int line = 0;
};

// ---------- functions / program ----------
struct Function {
    std::string name;
    FuncRet     retType = FuncRet::VOID;
    std::vector<Stmt> body;
};

struct Program {
    std::vector<Function> functions;
    int mainIndex = -1;
    bool usesMath = false;
};

// ---------- top-level entry ----------
Program parseProgram(const std::vector<Token>& toks);

// ---------- parser context ----------
// [WHILE] контекст прокидывается по всем parse-функциям, чтобы
// корректно ловить 'pass'/'continue' вне цикла и запрещать
// вложенный @whileblock / @ifblock.
struct ParseCtx {
    bool allowIf        = false;  // внутри @ifblock / @whileblock
    bool allowWhile     = false;  // внутри @whileblock (только один уровень)
    bool allowPass      = false;  // внутри while-тела
    bool allowContinue  = false;  // внутри while-тела
};

// ---------- per-statement parsers ----------
Stmt parseStatement(const std::vector<Token>& toks, std::size_t& pos,
                    const ParseCtx& ctx = {});
Stmt parsePrint    (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseExit     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseCall     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseDeclare  (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseGlobal   (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseDecl     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseAssign   (const std::vector<Token>& toks, std::size_t& pos, const std::string& name);
Stmt parseCin      (const std::vector<Token>& toks, std::size_t& pos);

// [WHILE] parseIf теперь принимает ctx — чтобы pass/continue
// вне цикла ловились сразу, а не в семантике.
Stmt parseIf       (const std::vector<Token>& toks, std::size_t& pos,
                    int blockLine, const ParseCtx& ctx);

Stmt parseDirective(const std::vector<Token>& toks, std::size_t& pos);
Stmt parseReturn   (const std::vector<Token>& toks, std::size_t& pos);

// [WHILE]
Stmt parseWhile    (const std::vector<Token>& toks, std::size_t& pos, int blockLine = -1);
Stmt parsePass     (const std::vector<Token>& toks, std::size_t& pos);
Stmt parseContinue (const std::vector<Token>& toks, std::size_t& pos);

// ---------- expression parser ----------
ExprPtr parseExpr(const std::vector<Token>& toks, std::size_t& pos);
ExprPtr parseCmp (const std::vector<Token>& toks, std::size_t& pos);