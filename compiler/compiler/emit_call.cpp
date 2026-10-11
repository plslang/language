// compiler/emit_call.cpp - emit .func(args)
#include "compiler.hpp"

using namespace std;

string emitCall(const Stmt& s, const string& fn, const FuncScope& sc,
                const map<string, GlobalInfo>& globals,
                const map<string, FuncInfo>& funcs)
{
    ostringstream out;
    out << "// code - ." << s.callName << "\n";

    auto it = funcs.find(s.callName);
    if (it == funcs.end()) {
        CompileError e(53,
            "call to unknown function '." + s.callName + "'", s.line);
        e.stack.push_back("Compiler::emitCall -> resolve");
        e.stack.push_back("at line " + to_string(s.line));
        throw e;
    }

    const auto& callee = it->second;
    vector<string> args;

    for (const auto& d : callee.declares) {
        Resolved r = resolveName(sc, globals, d.name);
        if (!r.found) {
            CompileError e(1,
                "failed argument: undeclared variable '" + d.name +
                "' at call ." + s.callName + " in function '" + fn + "'",
                s.line);
            e.stack.push_back("Compiler::emitCall -> resolveCall");
            e.stack.push_back("function " + fn + " -> ." + s.callName);
            e.stack.push_back("at line " + to_string(s.line));
            throw e;
        }
        if (r.type != d.type) {
            CompileError e(1,
                "failed argument: type mismatch for '" + d.name +
                "' in ." + s.callName + ": expected " + d.type +
                ", got " + r.type,
                s.line);
            e.stack.push_back("Compiler::emitCall -> resolveCall");
            e.stack.push_back("function " + fn + " -> ." + s.callName);
            e.stack.push_back("at line " + to_string(s.line));
            throw e;
        }
        args.push_back(renderLValue(fn, sc, globals, d.name));
    }

    out << "    fn_" << s.callName << "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) out << ", ";
        out << args[i];
    }
    out << ");\n";

    return out.str();
}