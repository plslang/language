// compiler/compile_main.cpp - entry point for codegen
#include "compiler.hpp"

using namespace std;

extern const char* PL_VERSION;
extern const char* PL_LANG_VERSION;

string generateCpp(const Program& prog, const string& srcName) {
    ostringstream out;

    auto globals = collectGlobals(prog);
    auto scopes  = buildScopes(prog, globals);
    auto funcs   = buildFuncInfo(prog);

    semanticCheck(prog, globals, scopes, funcs);

    out << emitBanner(srcName);
    out << emitIncludes(prog.usesMath);
    out << emitGlobals(globals);
    out << emitForwardDeclarations(prog, funcs);

    for (const auto& f : prog.functions) {
        out << emitFunction(f, scopes[f.name], globals, funcs);
    }

    out << emitMainEntry(funcs);

    return out.str();
}