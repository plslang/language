// compiler/semantic.cpp - semantic checks before codegen
#include "compiler.hpp"

using namespace std;

string inferExprType(const ExprPtr& e,
                     const FuncScope& sc,
                     const map<string, GlobalInfo>& globals,
                     const map<string, FuncInfo>& funcs,
                     const string& fn)
{
    if (!e) {
        CompileError err(1, "failed argument: empty expression");
        err.stack.push_back("Compiler::semanticCheck -> expr");
        throw err;
    }

    switch (e->kind) {
        case Expr::NUM:  return "int";
        case Expr::STR:  return "string";
        case Expr::BOOL: return "bool";

        case Expr::VAR: {
            Resolved r = resolveName(sc, globals, e->varName);
            if (!r.found) {
                CompileError err(1,
                    "failed argument: undeclared variable '" + e->varName +
                    "' in function '" + fn + "'",
                    e->line);
                err.stack.push_back("Compiler::semanticCheck -> expr -> var");
                err.stack.push_back("function " + fn);
                err.stack.push_back("at line " + to_string(e->line));
                throw err;
            }
            return r.type;
        }

        case Expr::CALL: {
            auto it = funcs.find(e->callName);
            if (it == funcs.end()) {
                CompileError err(53,
                    "call to unknown function '." + e->callName + "'",
                    e->line);
                err.stack.push_back("Compiler::semanticCheck -> expr -> call");
                err.stack.push_back("at line " + to_string(e->line));
                throw err;
            }
            const auto& fi = it->second;

            for (const auto& d : fi.declares) {
                Resolved r = resolveName(sc, globals, d.name);
                if (!r.found) {
                    CompileError err(53,
                        "undeclared variable '" + d.name +
                        "' at call ." + e->callName +
                        " in function '" + fn + "'",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> call args");
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                if (r.type != d.type) {
                    CompileError err(53,
                        "type mismatch for '" + d.name +
                        "' in ." + e->callName + ": expected " + d.type +
                        ", got " + r.type,
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> call args");
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
            }

            if (fi.retType == FuncRet::VOID) return "void";
            if (fi.retType == FuncRet::BOOL) return "bool";
            return "int";
        }

        case Expr::BINOP: {
            string lt = inferExprType(e->lhs, sc, globals, funcs, fn);
            string rt = inferExprType(e->rhs, sc, globals, funcs, fn);

            // ---------- '+' ----------
            if (e->op == '+') {
                // string + anything / anything + string → string (concat)
                if (lt == "string" || rt == "string") {
                    // bool с обеих сторон от int/string — ок через хелпер
                    // (bool + bool сюда не попадёт, потому что нет string)
                    return "string";
                }
                // int + int → int
                if (lt == "int" && rt == "int") {
                    return "int";
                }
                // bool + bool, bool + int, int + bool → ошибка
                if (lt == "bool" || rt == "bool") {
                    CompileError err(71,
                        "arithmetic on bool is not allowed in function '" +
                        fn + "' (got " + lt + " and " + rt + ")",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> binop + bool");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
            }

            // ---------- '-', '*', '/' ----------
            if (e->op == '-' || e->op == '*' || e->op == '/') {
                if (lt == "string" || rt == "string") {
                    string opName =
                        (e->op == '-') ? "-" :
                        (e->op == '*') ? "*" : "/";
                    CompileError err(70,
                        "operator '" + opName +
                        "' requires int operands in function '" + fn +
                        "' (got " + lt + " and " + rt + ")",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> binop str");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                if (lt == "bool" || rt == "bool") {
                    CompileError err(72,
                        "arithmetic on bool is not allowed in function '" +
                        fn + "' (got " + lt + " and " + rt + ")",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> binop bool");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                if (lt != "int" || rt != "int") {
                    CompileError err(1,
                        "failed argument: arithmetic requires int operands in function '" +
                        fn + "' (got " + lt + " and " + rt + ")",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> binop");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                return "int";
            }

            // ---------- '==' / '!=' ----------
            if (e->op == '=' || e->op == '!') {
                if (lt != rt) {
                    CompileError err(33,
                        "cannot compare " + lt + " and " + rt +
                        " in 'if' condition",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> cmp");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                return "int";
            }

            // ---------- '<' '>' '<=' '>=' ----------
            if (e->op == '<' || e->op == '>' || e->op == 'L' || e->op == 'G') {
                if (lt != "int" || rt != "int") {
                    string opName =
                        (e->op == '<') ? "<" :
                        (e->op == '>') ? ">" :
                        (e->op == 'L') ? "<=" : ">=";
                    CompileError err(34,
                        "ordering comparison '" + opName +
                        "' requires int operands (got " + lt + ")",
                        e->line);
                    err.stack.push_back("Compiler::semanticCheck -> expr -> cmp");
                    err.stack.push_back("function " + fn);
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                return "int";
            }
        }
    }
    return "int";
}

static void checkBlock(const vector<Stmt>& body,
                       const FuncScope& sc,
                       const map<string, GlobalInfo>& globals,
                       const map<string, FuncInfo>& funcs,
                       const string& fn,
                       FuncRet currentRet)
{
    for (const auto& s : body) {
        if (s.kind == Stmt::PRINT) {
            (void)inferExprType(s.printExpr, sc, globals, funcs, fn);

        } else if (s.kind == Stmt::DECL) {
            if (s.varInit) {
                string t = inferExprType(s.varInit, sc, globals, funcs, fn);
                if (s.varType == "int" && t != "int") {
                    CompileError e(1,
                        "failed argument: cannot initialize int '" + s.varName +
                        "' with " + t + " in function '" + fn + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> decl");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
                if (s.varType == "string" && t != "string") {
                    CompileError e(1,
                        "failed argument: cannot initialize string '" + s.varName +
                        "' with " + t + " in function '" + fn + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> decl");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
                if (s.varType == "bool" && t != "bool" && t != "int") {
                    CompileError e(55,
                        "cannot initialize bool '" + s.varName +
                        "' with " + t + " in function '" + fn + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> decl -> bool");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
            }

        } else if (s.kind == Stmt::ASSIGN) {
            Resolved r = resolveName(sc, globals, s.varName);
            if (!r.found) {
                CompileError e(1,
                    "failed argument: assignment to undeclared variable '" +
                    s.varName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> assignLHS");
                e.stack.push_back("function " + fn);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }
            string t = inferExprType(s.varInit, sc, globals, funcs, fn);
            if (r.type == "int" && t != "int") {
                CompileError e(1,
                    "failed argument: cannot assign " + t + " to int '" +
                    s.varName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> assignRHS");
                e.stack.push_back("function " + fn);
                throw e;
            }
            if (r.type == "string" && t != "string") {
                CompileError e(1,
                    "failed argument: cannot assign " + t + " to string '" +
                    s.varName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> assignRHS");
                e.stack.push_back("function " + fn);
                throw e;
            }
            if (r.type == "bool" && t != "bool" && t != "int") {
                CompileError e(56,
                    "cannot assign " + t + " to bool '" +
                    s.varName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> assignRHS -> bool");
                e.stack.push_back("function " + fn);
                throw e;
            }

        } else if (s.kind == Stmt::CIN) {
            Resolved r = resolveName(sc, globals, s.cinName);
            if (!r.found) {
                CompileError e(1,
                    "failed argument: cin() to undeclared variable '" +
                    s.cinName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> cin");
                e.stack.push_back("function " + fn);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }
            if (r.type != "int" && r.type != "string") {
                CompileError e(1,
                    "failed argument: cin() requires int or string '" +
                    s.cinName + "' in function '" + fn + "'",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> cin");
                e.stack.push_back("function " + fn);
                throw e;
            }

        } else if (s.kind == Stmt::IF) {
            string ct = inferExprType(s.cond, sc, globals, funcs, fn);
            if (ct != "int" && ct != "bool") {
                CompileError e(35,
                    "'if' condition must be int or bool (got " + ct + ")",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> if condition");
                e.stack.push_back("function " + fn);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }
            checkBlock(s.thenBody, sc, globals, funcs, fn, currentRet);
            if (!s.elseBody.empty()) {
                checkBlock(s.elseBody, sc, globals, funcs, fn, currentRet);
            }

        } else if (s.kind == Stmt::WHILE) {
            string ct = inferExprType(s.cond, sc, globals, funcs, fn);
            if (ct != "int" && ct != "bool") {
                CompileError e(69,
                    "'while' condition must be int or bool (got " + ct + ")",
                    s.line);
                e.stack.push_back("Compiler::semanticCheck -> while condition");
                e.stack.push_back("function " + fn);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }
            checkBlock(s.whileBody, sc, globals, funcs, fn, currentRet);

        } else if (s.kind == Stmt::PASS) {
            // no-op
        } else if (s.kind == Stmt::CONTINUE) {
            // no-op

        } else if (s.kind == Stmt::RETURN) {
            if (currentRet == FuncRet::VOID) {
                if (s.returnExpr) {
                    CompileError e(57,
                        "void function '" + fn + "' cannot return a value",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> return");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
            } else {
                if (!s.returnExpr) {
                    CompileError e(54,
                        "function '" + fn + "' with return type " +
                        retName(currentRet) + " must return a value",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> return");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
                string t = inferExprType(s.returnExpr, sc, globals, funcs, fn);
                if (currentRet == FuncRet::BOOL && t != "bool" && t != "int") {
                    CompileError e(58,
                        "bool function '" + fn + "' must return bool or int (got " +
                        t + ")", s.line);
                    e.stack.push_back("Compiler::semanticCheck -> return bool");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
                if (currentRet == FuncRet::INT && t != "int") {
                    CompileError e(58,
                        "int function '" + fn + "' must return int (got " +
                        t + ")", s.line);
                    e.stack.push_back("Compiler::semanticCheck -> return int");
                    e.stack.push_back("function " + fn);
                    throw e;
                }
            }
        }
    }
}

void semanticCheck(const Program& prog,
                   const map<string, GlobalInfo>& globals,
                   const map<string, FuncScope>& scopes,
                   const map<string, FuncInfo>& funcs)
{
    for (const auto& f : prog.functions) {
        const FuncScope& sc = scopes.at(f.name);
        checkBlock(f.body, sc, globals, funcs, f.name, f.retType);
    }
}