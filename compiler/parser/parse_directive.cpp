// parser/parse_directive.cpp - @name
#include "parser.hpp"

using namespace std;

Stmt parseDirective(const vector<Token>& toks, size_t& pos) {
    const Token& at = toks[pos];   // AT
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected directive name after '@', got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseDirective -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    Stmt s;
    s.kind = Stmt::DIRECTIVE;
    s.directiveName = toks[pos].text;
    s.line = at.line;
    pos++;
    return s;
}