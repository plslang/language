// compiler/scopes.cpp - per-function scope: locals + declares
#include "compiler.hpp"

using namespace std;

map<string, FuncScope> buildScopes(const Program& prog,
                                   const map<string, GlobalInfo>& globals)
{
    map<string, FuncScope> scopes;

    for (const auto& f : prog.functions) {
        FuncScope sc;

        for (const auto& s : f.body) {
            if (s.kind == Stmt::DECL) {
                if (sc.declares.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: local variable '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with declare of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                sc.vars[s.varName] = s.varType;

            } else if (s.kind == Stmt::DECLARE) {
                if (sc.vars.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: declare '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with local variable of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                if (globals.count(s.varName)) {
                    CompileError e(1,
                        "failed argument: declare '" + s.varName +
                        "' in function '" + f.name +
                        "' conflicts with global of same name",
                        s.line);
                    e.stack.push_back("Compiler::buildScopes -> conflict");
                    e.stack.push_back("function " + f.name);
                    throw e;
                }
                sc.declares.insert(s.varName);
                sc.declareTypes[s.varName] = s.varType;
            }
        }

        scopes[f.name] = sc;
    }
    return scopes;
}