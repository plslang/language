// parser/parse_expr.cpp - recursive-descent expression parser
#include "parser.hpp"

using namespace std;

static ExprPtr parseAtom(const vector<Token>& toks, size_t& pos) {
    if (pos >= toks.size()) {
        CompileError e(1, "failed argument: unexpected end of file in expression");
        e.stack.push_back("Parser::parseExpr -> atom");
        throw e;
    }

    const Token& t = toks[pos];

    if (t.type == Token::NUMBER) { pos++; return makeNum(stoll(t.text), t.line); }
    if (t.type == Token::STRING) { pos++; return makeStr(t.text, t.line); }

    // .name — вызов функции в выражении
    if (t.type == Token::DOT) {
        int line = t.line;
        pos++;
        if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
            const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
            CompileError e(1,
                "failed argument: expected function name after '.', got '" +
                tk.text + "'", tk.line);
            e.stack.push_back("Parser::parseExpr -> call");
            e.stack.push_back("at line " + to_string(tk.line));
            throw e;
        }
        string name = toks[pos].text;
        pos++;
        return makeCall(name, line);
    }

    if (t.type == Token::IDENT) {
        if (t.text == "true")  { pos++; return makeBool(true,  t.line); }
        if (t.text == "false") { pos++; return makeBool(false, t.line); }
        pos++;
        return makeVar(t.text, t.line);
    }

    if (t.type == Token::LPAREN) {
        pos++;
        ExprPtr inner = parseCmp(toks, pos);
        if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
            const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
            CompileError e(1,
                "failed argument: expected ')' in expression, got '" +
                tk.text + "'", tk.line);
            e.stack.push_back("Parser::parseExpr -> atom -> close");
            e.stack.push_back("at line " + to_string(tk.line));
            throw e;
        }
        pos++;
        return inner;
    }

    CompileError e(1,
        "failed argument: unexpected token in expression: '" + t.text + "'",
        t.line);
    e.stack.push_back("Parser::parseExpr -> atom");
    e.stack.push_back("at line " + to_string(t.line));
    throw e;
}

static ExprPtr parseTerm(const vector<Token>& toks, size_t& pos) {
    ExprPtr left = parseAtom(toks, pos);
    while (pos < toks.size() &&
           (toks[pos].type == Token::STAR || toks[pos].type == Token::SLASH))
    {
        char op = (toks[pos].type == Token::STAR) ? '*' : '/';
        int line = toks[pos].line;
        pos++;
        ExprPtr right = parseAtom(toks, pos);
        left = makeBin(op, left, right, line);
    }
    return left;
}

ExprPtr parseExpr(const vector<Token>& toks, size_t& pos) {
    ExprPtr left = parseTerm(toks, pos);
    while (pos < toks.size() &&
           (toks[pos].type == Token::PLUS || toks[pos].type == Token::MINUS))
    {
        char op = (toks[pos].type == Token::PLUS) ? '+' : '-';
        int line = toks[pos].line;
        pos++;
        ExprPtr right = parseTerm(toks, pos);
        left = makeBin(op, left, right, line);
    }
    return left;
}

ExprPtr parseCmp(const vector<Token>& toks, size_t& pos) {
    ExprPtr left = parseExpr(toks, pos);

    if (pos < toks.size()) {
        char op = 0;
        string opName;
        switch (toks[pos].type) {
            case Token::EQEQ: op = '='; opName = "=="; break;
            case Token::NEQ:  op = '!'; opName = "!="; break;
            case Token::LT:   op = '<'; opName = "<";  break;
            case Token::GT:   op = '>'; opName = ">";  break;
            case Token::LTE:  op = 'L'; opName = "<="; break;
            case Token::GTE:  op = 'G'; opName = ">="; break;
            default: break;
        }

        if (op) {
            int line = toks[pos].line;
            pos++;

            if (pos >= toks.size() ||
                toks[pos].type == Token::COLON ||
                toks[pos].type == Token::END)
            {
                const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                CompileError e(28,
                    "incomplete condition after operator '" + opName +
                    "' in 'if'", tk.line);
                e.stack.push_back("Parser::parseCmp -> right operand");
                e.stack.push_back("at line " + to_string(tk.line));
                throw e;
            }

            ExprPtr right = parseExpr(toks, pos);
            return makeBin(op, left, right, line);
        }
    }
    return left;
}