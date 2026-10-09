// compiler/globals.cpp - collect global variables
#include "compiler.hpp"

using namespace std;

map<string, GlobalInfo> collectGlobals(const Program& prog) {
    map<string, GlobalInfo> globals;

    for (const auto& f : prog.functions) {
        for (const auto& s : f.body) {
            if (s.kind != Stmt::GLOBAL) continue;

            auto it = globals.find(s.varName);
            if (it != globals.end()) {
                CompileError e(1,
                    "failed argument: global '" + s.varName +
                    "' already declared (first in function '" +
                    it->second.declaredIn + "')",
                    s.line);
                e.stack.push_back("Compiler::collectGlobals -> duplicate");
                e.stack.push_back("function " + f.name);
                e.stack.push_back("at line " + to_string(s.line));
                throw e;
            }

            GlobalInfo gi;
            gi.type = s.varType;
            gi.init = s.varInit;
            gi.declaredIn = f.name;
            gi.line = s.line;

            globals[s.varName] = gi;
        }
    }
    return globals;
}