// parser/parse_if.cpp - if cond: ... [else if cond: ...]* [else: ...]
#include "parser.hpp"

using namespace std;

// Тело if: statement'ы до `else` или `@endblock`.
// ctx наследуется от вызывающего, но с поправками:
//   allowIf = true   (внутри if можно @ifblock? нет — E23)
//   allowWhile = false (E60)
//   allowPass/allowContinue — наследуются (если мы внутри while,
//   они true и pass/continue валидны; если снаружи — false,
//   и parseStatement сразу бросит E61/E62).
static vector<Stmt> parseIfBody(const vector<Token>& toks, size_t& pos,
                                int blockLine, const ParseCtx& outer)
{
    vector<Stmt> body;

    ParseCtx inner = outer;
    inner.allowIf    = false;  // вложенный @ifblock запрещён (E23)
    inner.allowWhile = false;  // @whileblock внутри if запрещён (E60)

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        // else — конец thenBody
        if (t.type == Token::IDENT && t.text == "else") break;

        // @-директивы
        if (t.type == Token::AT) {
            if (pos + 1 < toks.size() &&
                toks[pos + 1].type == Token::IDENT)
            {
                const string& name = toks[pos + 1].text;

                if (name == "endblock") break;

                if (name == "ifblock") {
                    CompileError e(23,
                        "nested '@ifblock' is not supported (inner block at line " +
                        to_string(t.line) + " inside block opened at line " +
                        to_string(blockLine) + ")",
                        t.line);
                    e.stack.push_back("Parser::parseIfBody -> nested @ifblock");
                    e.stack.push_back("at line " + to_string(t.line));
                    throw e;
                }

                if (name == "whileblock") {
                    CompileError e(60,
                        "nested '@whileblock' is not supported (inner block at line " +
                        to_string(t.line) + " inside block opened at line " +
                        to_string(blockLine) + ")",
                        t.line);
                    e.stack.push_back("Parser::parseIfBody -> nested @whileblock");
                    e.stack.push_back("at line " + to_string(t.line));
                    throw e;
                }
            }
            CompileError e(24,
                "unknown directive '@" +
                (pos + 1 < toks.size() ? toks[pos + 1].text : "") +
                "' (expected '@ifblock' or '@endblock')",
                t.line);
            e.stack.push_back("Parser::parseIfBody -> directive");
            e.stack.push_back("at line " + to_string(t.line));
            throw e;
        }

        body.push_back(parseStatement(toks, pos, inner));
    }

    return body;
}

Stmt parseIf(const vector<Token>& toks, size_t& pos,
             int blockLine, const ParseCtx& ctx)
{
    const Token& kw = toks[pos];   // "if"
    pos++;

    Stmt s;
    s.kind = Stmt::IF;
    s.line = kw.line;

    // 27: 'if' requires a condition
    if (pos >= toks.size() ||
        toks[pos].type == Token::COLON ||
        toks[pos].type == Token::END)
    {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(27, "'if' requires a condition", tk.line);
        e.stack.push_back("Parser::parseIf -> condition");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    s.cond = parseCmp(toks, pos);

    // 63: '=' вместо '=='
    if (pos < toks.size() && toks[pos].type == Token::EQUAL) {
        CompileError e(63,
            "did you mean '=='? '=' is assignment, not comparison",
            toks[pos].line);
        e.stack.push_back("Parser::parseIf -> condition '='");
        e.stack.push_back("at line " + to_string(toks[pos].line));
        throw e;
    }

    // 25: expected ':' after if condition
    if (pos >= toks.size() || toks[pos].type != Token::COLON) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(25,
            "expected ':' after 'if' condition, got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseIf -> colon");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    s.thenBody = parseIfBody(toks, pos, blockLine, ctx);

    // 45: 'if' body is empty
    if (s.thenBody.empty()) {
        CompileError e(45,
            "'if' body is empty (no statements between ':' and 'else'/'@endblock')",
            s.line);
        e.stack.push_back("Parser::parseIf -> empty thenBody");
        e.stack.push_back("at line " + to_string(s.line));
        throw e;
    }

    if (pos < toks.size() &&
        toks[pos].type == Token::IDENT &&
        toks[pos].text == "else")
    {
        int elseLine = toks[pos].line;
        pos++;

        // else if
        if (pos < toks.size() &&
            toks[pos].type == Token::IDENT &&
            toks[pos].text == "if")
        {
            Stmt nested = parseIf(toks, pos, blockLine, ctx);
            s.elseBody.push_back(move(nested));
            return s;
        }

        // 32: multiple 'else' in one if-chain
        if (pos < toks.size() &&
            toks[pos].type == Token::IDENT &&
            toks[pos].text == "else")
        {
            CompileError e(32,
                "multiple 'else' in one if-chain", toks[pos].line);
            e.stack.push_back("Parser::parseIf -> multiple else");
            e.stack.push_back("at line " + to_string(toks[pos].line));
            throw e;
        }

        // 26: expected ':' after else
        if (pos >= toks.size() || toks[pos].type != Token::COLON) {
            const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
            CompileError e(26,
                "expected ':' after 'else', got '" + tk.text + "'",
                tk.line);
            e.stack.push_back("Parser::parseIf -> else colon");
            e.stack.push_back("at line " + to_string(tk.line));
            throw e;
        }
        pos++;

        // 41: else cannot be followed by @endblock without a body
        if (pos < toks.size() &&
            toks[pos].type == Token::AT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::IDENT &&
            toks[pos + 1].text == "endblock")
        {
            CompileError e(41,
                "'else' cannot be followed by '@endblock' without a body",
                elseLine);
            e.stack.push_back("Parser::parseIf -> else empty body");
            e.stack.push_back("at line " + to_string(elseLine));
            throw e;
        }

        s.elseBody = parseIfBody(toks, pos, blockLine, ctx);
    }

    return s;
}