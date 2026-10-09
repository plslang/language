// compiler/semantic.cpp - semantic checks before codegen
#include "compiler.hpp"

using namespace std;

static string inferExprType(const ExprPtr& e,
                            const FuncScope& sc,
                            const map<string, GlobalInfo>& globals,
                            const string& fn)
{
    if (!e) {
        CompileError err(1, "failed argument: empty expression");
        err.stack.push_back("Compiler::semanticCheck -> expr");
        throw err;
    }

    switch (e->kind) {
        case Expr::NUM: return "int";
        case Expr::STR: return "string";

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

        case Expr::BINOP: {
            string lt = inferExprType(e->lhs, sc, globals, fn);
            string rt = inferExprType(e->rhs, sc, globals, fn);
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
    }
    return "int";
}

void semanticCheck(const Program& prog,
                   const map<string, GlobalInfo>& globals,
                   const map<string, FuncScope>& scopes,
                   const map<string, vector<DeclareInfo>>& declares)
{
    for (const auto& f : prog.functions) {
        for (const auto& s : f.body) {
            if (s.kind == Stmt::CALL) {
                if (!declares.count(s.callName)) {
                    CompileError e(1,
                        "failed argument: call to unknown function '." +
                        s.callName + "'", s.line);
                    e.stack.push_back("Compiler::semanticCheck -> resolveCalls");
                    e.stack.push_back("at line " + to_string(s.line));
                    throw e;
                }
            }
        }
    }

    for (const auto& f : prog.functions) {
        const FuncScope& sc = scopes.at(f.name);
        for (const auto& s : f.body) {
            if (s.kind == Stmt::PRINT) {
                (void)inferExprType(s.printExpr, sc, globals, f.name);

            } else if (s.kind == Stmt::DECL) {
                if (s.varInit) {
                    string t = inferExprType(s.varInit, sc, globals, f.name);
                    if (s.varType == "int" && t != "int") {
                        CompileError e(1,
                            "failed argument: cannot initialize int '" + s.varName +
                            "' with " + t + " in function '" + f.name + "'",
                            s.line);
                        e.stack.push_back("Compiler::semanticCheck -> decl");
                        e.stack.push_back("function " + f.name);
                        throw e;
                    }
                    if (s.varType == "string" && t != "string") {
                        CompileError e(1,
                            "failed argument: cannot initialize string '" + s.varName +
                            "' with " + t + " in function '" + f.name + "'",
                            s.line);
                        e.stack.push_back("Compiler::semanticCheck -> decl");
                        e.stack.push_back("function " + f.name);
                        throw e;
                    }
                }

            } else if (s.kind == Stmt::ASSIGN) {
                Resolved r = resolveName(sc, globals, s.varName);
                if (!r.found) {
                    CompileError e(1,
                        "failed argument: assignment to undeclared variable '" +
                        s.varName + "' in function '" + f.name + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> assignLHS");
                    e.stack.push_back("function " + f.name);
                    e.stack.push_back("at line " + to_string(s.line));
                    throw e;
                }
                string t = inferExprType(s.varInit, sc, globals, f.name);
                if (r.type == "int" && t != "int") {
                    CompileError e(1,
                        "failed argument: cannot assign " + t + " to int '" +
                        s.varName + "' in function '" + f.name + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> assignRHS");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                if (r.type == "string" && t != "string") {
                    CompileError e(1,
                        "failed argument: cannot assign " + t + " to string '" +
                        s.varName + "' in function '" + f.name + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> assignRHS");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }

            } else if (s.kind == Stmt::CIN) {
                Resolved r = resolveName(sc, globals, s.cinName);
                if (!r.found) {
                    CompileError e(1,
                        "failed argument: cin() to undeclared variable '" +
                        s.cinName + "' in function '" + f.name + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> cin");
                    e.stack.push_back("function " + f.name);
                    e.stack.push_back("at line " + to_string(s.line));
                    throw e;
                }
                if (r.type != "int" && r.type != "string") {
                    CompileError e(1,
                        "failed argument: cin() requires int or string '" +
                        s.cinName + "' in function '" + f.name + "'",
                        s.line);
                    e.stack.push_back("Compiler::semanticCheck -> cin");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
            }
        }
    }
}