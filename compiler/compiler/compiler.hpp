// compiler/compiler.hpp - types + prototypes for the code generator
#pragma once

#include "../common.hpp"
#include "../parser/parser.hpp"

#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <sstream>

// ---------- globals ----------
struct GlobalInfo {
    std::string type;
    ExprPtr     init;
    std::string declaredIn;
    int         line = 0;
};

// ---------- per-function scope ----------
struct FuncScope {
    std::map<std::string, std::string> vars;
    std::map<std::string, std::string> declareTypes;
    std::set<std::string>              declares;
};

// ---------- declare info ----------
struct DeclareInfo {
    std::string type;
    std::string name;
};

// ---------- name resolution ----------
struct Resolved {
    bool found = false;
    std::string type;
    enum Where { NONE, LOCAL, DECLARE, GLOBAL } where = NONE;
};

// ---------- config.cpp helpers ----------
std::string cppStringLiteral(const std::string& s);
std::string cppIdentFor(const std::string& fn, const std::string& var);
std::string cppGlobalIdent(const std::string& name);
std::string typeFor(const std::string& t);
bool        isNumericLiteral(const std::string& s);
std::string isoDateNow();

// ---------- passes ----------
std::map<std::string, GlobalInfo> collectGlobals(const Program& prog);
std::map<std::string, FuncScope>  buildScopes(const Program& prog,
                                              const std::map<std::string, GlobalInfo>& globals);
std::map<std::string, std::vector<DeclareInfo>> buildDeclares(const Program& prog);

// ---------- name resolution ----------
Resolved resolveName(const FuncScope& sc,
                     const std::map<std::string, GlobalInfo>& globals,
                     const std::string& name);

std::string renderExpr(const ExprPtr& e, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals);
std::string renderLValue(const std::string& fn, const FuncScope& sc,
                         const std::map<std::string, GlobalInfo>& globals,
                         const std::string& name);

// ---------- semantic ----------
void semanticCheck(const Program& prog,
                   const std::map<std::string, GlobalInfo>& globals,
                   const std::map<std::string, FuncScope>& scopes,
                   const std::map<std::string, std::vector<DeclareInfo>>& declares);

// ---------- emitters ----------
std::string emitPrint (const Stmt& s, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals);
std::string emitExit  (const Stmt& s);
std::string emitCall  (const Stmt& s, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals,
                       const std::map<std::string, std::vector<DeclareInfo>>& declares);
std::string emitDecl  (const Stmt& s, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals);
std::string emitAssign(const Stmt& s, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals);
std::string emitDeclare(const Stmt& s, const std::string& fn);
std::string emitGlobalMarker(const Stmt& s);
std::string emitCin   (const Stmt& s, const std::string& fn, const FuncScope& sc,
                       const std::map<std::string, GlobalInfo>& globals);

// if / else
std::string emitIf(const Stmt& s, const std::string& fn, const FuncScope& sc,
                   const std::map<std::string, GlobalInfo>& globals,
                   const std::map<std::string, std::vector<DeclareInfo>>& declares,
                   int indent);

// ---------- block emitter (used by emitIf) ----------
std::string emitStmtBlock(const std::vector<Stmt>& body,
                          const std::string& fn,
                          const FuncScope& sc,
                          const std::map<std::string, GlobalInfo>& globals,
                          const std::map<std::string, std::vector<DeclareInfo>>& declares,
                          int indent);

// ---------- codegen ----------
std::string emitBanner(const std::string& srcName);
std::string emitIncludes(bool usesMath);
std::string emitGlobals(const std::map<std::string, GlobalInfo>& globals);
std::string emitForwardDeclarations(const Program& prog,
                                    const std::map<std::string, std::vector<DeclareInfo>>& declares);
std::string emitFunction(const Function& f, const FuncScope& sc,
                         const std::map<std::string, GlobalInfo>& globals,
                         const std::map<std::string, std::vector<DeclareInfo>>& declares);
std::string emitMainEntry(const std::map<std::string, std::vector<DeclareInfo>>& declares);

std::string generateCpp(const Program& prog, const std::string& srcName);