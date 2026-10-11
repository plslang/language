// parser/parse_while.cpp - while cond: ... (inside @whileblock)
#include "parser.hpp"

using namespace std;

// Тело while: statement'ы до @endblock.
static vector<Stmt> parseWhileBody(const vector<Token>& toks, size_t& pos,
                                   int blockLine)
{
    vector<Stmt> body;

    ParseCtx inner;
    inner.allowIf       = true;
    inner.allowWhile    = false;
    inner.allowPass     = true;
    inner.allowContinue = true;

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        if (t.type == Token::AT) {
            if (pos + 1 >= toks.size() || toks[pos + 1].type != Token::IDENT) {
                const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                CompileError e(24,
                    "unknown directive '@' (expected '@ifblock' or '@endblock')",
                    tk.line);
                e.stack.push_back("Parser::parseWhileBody -> directive");
                e.stack.push_back("at line " + to_string(tk.line));
                throw e;
            }

            const string& name = toks[pos + 1].text;

            if (name == "endblock") break;

            if (name == "whileblock") {
                CompileError e(60,
                    "nested '@whileblock' is not supported (inner block at line " +
                    to_string(t.line) + " inside block opened at line " +
                    to_string(blockLine) + ")",
                    t.line);
                e.stack.push_back("Parser::parseWhileBody -> nested @whileblock");
                e.stack.push_back("at line " + to_string(t.line));
                throw e;
            }

            if (name == "ifblock") {
                pos += 2;

                if (pos >= toks.size() ||
                    toks[pos].type != Token::IDENT ||
                    toks[pos].text != "if")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(22,
                        "'@ifblock' must be followed by an 'if' statement, got '" +
                        tk.text + "'", tk.line);
                    e.stack.push_back("Parser::parseWhileBody -> @ifblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }

                Stmt ifStmt = parseIf(toks, pos, t.line, inner);
                body.push_back(move(ifStmt));

                if (pos >= toks.size() ||
                    toks[pos].type != Token::AT ||
                    pos + 1 >= toks.size() ||
                    toks[pos + 1].type != Token::IDENT ||
                    toks[pos + 1].text != "endblock")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(20,
                        "'@ifblock' opened at line " + to_string(t.line) +
                        " is never closed with '@endblock'", tk.line);
                    e.stack.push_back("Parser::parseWhileBody -> @endblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }
                pos += 2;
                continue;
            }

            CompileError e(24,
                "unknown directive '@" + name +
                "' (expected '@ifblock' or '@endblock')", t.line);
            e.stack.push_back("Parser::parseWhileBody -> directive");
            e.stack.push_back("at line " + to_string(t.line));
            throw e;
        }

        body.push_back(parseStatement(toks, pos, inner));
    }

    return body;
}

Stmt parseWhile(const vector<Token>& toks, size_t& pos, int blockLine) {
    const Token& kw = toks[pos];   // "while"
    pos++;

    Stmt s;
    s.kind = Stmt::WHILE;
    s.line = kw.line;
    s.whileBlockLine = blockLine;

    // 68: 'while' requires a condition
    if (pos >= toks.size() ||
        toks[pos].type == Token::COLON ||
        toks[pos].type == Token::END)
    {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(68, "'while' requires a condition", tk.line);
        e.stack.push_back("Parser::parseWhile -> condition");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    s.cond = parseCmp(toks, pos);

    // 63: '=' вместо '=='
    if (pos < toks.size() && toks[pos].type == Token::EQUAL) {
        CompileError e(63,
            "did you mean '=='? '=' is assignment, not comparison",
            toks[pos].line);
        e.stack.push_back("Parser::parseWhile -> condition '='");
        e.stack.push_back("at line " + to_string(toks[pos].line));
        throw e;
    }

    // 67: expected ':' after while condition
    if (pos >= toks.size() || toks[pos].type != Token::COLON) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(67,
            "expected ':' after 'while' condition, got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseWhile -> colon");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    s.whileBody = parseWhileBody(toks, pos, blockLine);

    // 66: 'while' body is empty
    if (s.whileBody.empty()) {
        CompileError e(66,
            "'while' body is empty (no statements between ':' and '@endblock')",
            s.line);
        e.stack.push_back("Parser::parseWhile -> empty body");
        e.stack.push_back("at line " + to_string(s.line));
        throw e;
    }

    return s;
}