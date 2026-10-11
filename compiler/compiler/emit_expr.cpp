// compiler/emit_expr.cpp - render Expr tree to C++
#include "compiler.hpp"

using namespace std;

static string opToCpp(char op) {
    switch (op) {
        case '+': return "+";
        case '-': return "-";
        case '*': return "*";
        case '/': return "/";
        case '=': return "==";
        case '!': return "!=";
        case '<': return "<";
        case '>': return ">";
        case 'L': return "<=";
        case 'G': return ">=";
    }
    return "?";
}

// [CONCAT] оборачивает операнд в to_string / __pla_bool_to_str,
// если он не string, а мы эмитим конкатенацию.
static string asStringOperand(const ExprPtr& e, const string& rendered,
                              const string& type)
{
    if (type == "string") return rendered;
    if (type == "int")    return "to_string(" + rendered + ")";
    if (type == "bool")   return "__pla_bool_to_str(" + rendered + ")";
    (void)e;
    return rendered;
}

string renderExpr(const ExprPtr& e, const string& fn, const FuncScope& sc,
                  const map<string, GlobalInfo>& globals,
                  const map<string, FuncInfo>& funcs)
{
    if (!e) {
        CompileError err(1, "internal: null expression");
        err.stack.push_back("Compiler::renderExpr");
        throw err;
    }

    switch (e->kind) {
        case Expr::NUM:
            return to_string(e->numValue);

        case Expr::STR:
            return cppStringLiteral(e->strValue);

        case Expr::BOOL:
            return e->boolValue ? "true" : "false";

        case Expr::VAR: {
            Resolved r = resolveName(sc, globals, e->varName);
            switch (r.where) {
                case Resolved::LOCAL:
                case Resolved::DECLARE: return cppIdentFor(fn, e->varName);
                case Resolved::GLOBAL:  return cppGlobalIdent(e->varName);
                default:                return e->varName;
            }
        }

        case Expr::CALL: {
            auto it = funcs.find(e->callName);
            if (it == funcs.end()) {
                CompileError err(53,
                    "call to unknown function '." + e->callName + "'",
                    e->line);
                err.stack.push_back("Compiler::renderExpr -> call");
                err.stack.push_back("at line " + to_string(e->line));
                throw err;
            }
            const auto& ci = it->second;

            vector<string> args;
            for (const auto& d : ci.declares) {
                Resolved r = resolveName(sc, globals, d.name);
                if (!r.found) {
                    CompileError err(53,
                        "undeclared variable '" + d.name +
                        "' at call ." + e->callName +
                        " in function '" + fn + "'",
                        e->line);
                    err.stack.push_back("Compiler::renderExpr -> call args");
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                if (r.type != d.type) {
                    CompileError err(53,
                        "type mismatch for '" + d.name +
                        "' in ." + e->callName + ": expected " + d.type +
                        ", got " + r.type,
                        e->line);
                    err.stack.push_back("Compiler::renderExpr -> call args");
                    err.stack.push_back("at line " + to_string(e->line));
                    throw err;
                }
                args.push_back(renderLValue(fn, sc, globals, d.name));
            }

            ostringstream out;
            out << "fn_" << e->callName << "(";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i) out << ", ";
                out << args[i];
            }
            out << ")";
            return out.str();
        }

        case Expr::BINOP: {
            string l = renderExpr(e->lhs, fn, sc, globals, funcs);
            string r = renderExpr(e->rhs, fn, sc, globals, funcs);

            // [CONCAT] '+' над string — конкатенация.
            // '-' '*' '/' — гарантированно int-int (семантика проверила).
            if (e->op == '+') {
                string lt = inferExprType(e->lhs, sc, globals, funcs, fn);
                string rt = inferExprType(e->rhs, sc, globals, funcs, fn);

                if (lt == "string" || rt == "string") {
                    string ls = asStringOperand(e->lhs, l, lt);
                    string rs = asStringOperand(e->rhs, r, rt);
                    ostringstream out;
                    out << "(" << ls << " + " << rs << ")";
                    return out.str();
                }
            }

            ostringstream out;
            out << "(" << l << " " << opToCpp(e->op) << " " << r << ")";
            return out.str();
        }
    }
    return "";
}