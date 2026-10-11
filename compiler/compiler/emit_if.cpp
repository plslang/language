// compiler/emit_if.cpp - emit if / else if / else
#include "compiler.hpp"

using namespace std;

static string pad(int n) {
    return string(n * 4, ' ');
}

string emitIf(const Stmt& s, const string& fn, const FuncScope& sc,
              const map<string, GlobalInfo>& globals,
              const map<string, vector<DeclareInfo>>& declares,
              int indent)
{
    ostringstream out;

    out << pad(indent)
        << "if (" << renderExpr(s.cond, fn, sc, globals) << ") {\n";
    out << emitStmtBlock(s.thenBody, fn, sc, globals, declares, indent + 1);
    out << pad(indent) << "}";

    if (!s.elseBody.empty()) {
        // else if: единственный Stmt::IF внутри elseBody
        if (s.elseBody.size() == 1 && s.elseBody[0].kind == Stmt::IF) {
            out << " else ";
            // рекурсивно, но без ведущего pad — потому что уже написали "else "
            string nested = emitIf(s.elseBody[0], fn, sc, globals,
                                   declares, indent);
            // убираем ведущий pad из nested
            string p = pad(indent);
            if (nested.rfind(p, 0) == 0) nested = nested.substr(p.size());
            out << nested;
        } else {
            out << " else {\n";
            out << emitStmtBlock(s.elseBody, fn, sc, globals,
                                 declares, indent + 1);
            out << pad(indent) << "}";
        }
    }

    out << "\n";
    return out.str();
}