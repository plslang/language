// compiler/func_info.cpp - collect FuncInfo (return type + declares)
#include "compiler.hpp"

using namespace std;

map<string, FuncInfo> buildFuncInfo(const Program& prog) {
    map<string, FuncInfo> m;

    for (const auto& f : prog.functions) {
        FuncInfo fi;
        fi.retType = f.retType;

        for (const auto& s : f.body) {
            if (s.kind == Stmt::DECLARE) {
                fi.declares.push_back({s.varType, s.varName});
            }
        }
        m[f.name] = fi;
    }
    return m;
}