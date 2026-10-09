// compiler/resolve.cpp - name resolution
#include "compiler.hpp"

using namespace std;

Resolved resolveName(const FuncScope& sc,
                     const map<string, GlobalInfo>& globals,
                     const string& name)
{
    Resolved r;
    auto it = sc.vars.find(name);
    if (it != sc.vars.end()) {
        r.found = true; r.type = it->second; r.where = Resolved::LOCAL; return r;
    }
    auto it2 = sc.declareTypes.find(name);
    if (it2 != sc.declareTypes.end()) {
        r.found = true; r.type = it2->second; r.where = Resolved::DECLARE; return r;
    }
    auto it3 = globals.find(name);
    if (it3 != globals.end()) {
        r.found = true; r.type = it3->second.type; r.where = Resolved::GLOBAL; return r;
    }
    return r;
}

string renderLValue(const string& fn,
                    const FuncScope& sc,
                    const map<string, GlobalInfo>& globals,
                    const string& name)
{
    Resolved r = resolveName(sc, globals, name);
    switch (r.where) {
        case Resolved::LOCAL:
        case Resolved::DECLARE: return cppIdentFor(fn, name);
        case Resolved::GLOBAL:  return cppGlobalIdent(name);
        default:                return cppIdentFor(fn, name);
    }
}