// parser/parse_call.cpp - .funcname
#include "parser.hpp"

using namespace std;

Stmt parseCall(const vector<Token>& toks, size_t& pos) {
    const Token& dot = toks[pos];
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected function name after '.', got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseCall");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    Stmt s;
    s.kind = Stmt::CALL;
    s.callName = toks[pos].text;
    s.line = dot.line;
    pos++;
    return s;
}