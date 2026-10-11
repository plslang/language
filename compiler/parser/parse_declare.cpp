// parser/parse_declare.cpp - declare <type> <name>
#include "parser.hpp"

using namespace std;

Stmt parseDeclare(const vector<Token>& toks, size_t& pos) {
    const Token& kw = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT ||
        (toks[pos].text != "int" && toks[pos].text != "string" &&
         toks[pos].text != "bool"))
    {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: declare requires type (int|string|bool), got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseDeclare -> type");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string type = toks[pos].text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: declare requires variable name, got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseDeclare -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    string name = toks[pos].text;
    pos++;

    Stmt s;
    s.kind = Stmt::DECLARE;
    s.varType = type;
    s.varName = name;
    s.line = kw.line;
    return s;
}