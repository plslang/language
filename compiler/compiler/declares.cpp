// compiler/declares.cpp - per-function ordered declare list
#include "compiler.hpp"

using namespace std;

map<string, vector<DeclareInfo>> buildDeclares(const Program& prog) {
    map<string, vector<DeclareInfo>> m;

    for (const auto& f : prog.functions) {
        vector<DeclareInfo> v;
        for (const auto& s : f.body) {
            if (s.kind == Stmt::DECLARE) {
                v.push_back({s.varType, s.varName});
            }
        }
        m[f.name] = v;
    }
    return m;
}