// parser/parse_function.cpp - parse functions + top-level parseProgram
#include "parser.hpp"

using namespace std;

// ---------- recursive hasExit ----------
static bool hasExitRec(const vector<Stmt>& body) {
    for (const auto& s : body) {
        if (s.kind == Stmt::EXIT) return true;
        if (s.kind == Stmt::IF) {
            if (hasExitRec(s.thenBody)) return true;
            if (hasExitRec(s.elseBody)) return true;
        }
    }
    return false;
}

// ---------- parse body of a function ----------
static vector<Stmt> parseFunctionBody(const vector<Token>& toks, size_t& pos,
                                      const string& fnName)
{
    vector<Stmt> body;

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        // новая функция: IDENT + COLON
        if (t.type == Token::IDENT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::COLON)
        {
            break;
        }

        // @ifblock / @endblock / @unknown
        if (t.type == Token::AT) {
            if (pos + 1 >= toks.size() || toks[pos + 1].type != Token::IDENT) {
                const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                CompileError e(24,
                    "unknown directive '@' (expected '@ifblock' or '@endblock')",
                    tk.line);
                e.stack.push_back("Parser::parseFunctionBody -> directive");
                e.stack.push_back("at line " + to_string(tk.line));
                throw e;
            }

            const string& name = toks[pos + 1].text;
            int atLine = t.line;

            if (name == "ifblock") {
                pos += 2;   // съели @ifblock

                // 22: '@ifblock' must be followed by an 'if'
                if (pos >= toks.size() ||
                    toks[pos].type != Token::IDENT ||
                    toks[pos].text != "if")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(22,
                        "'@ifblock' must be followed by an 'if' statement, got '" +
                        tk.text + "'", tk.line);
                    e.stack.push_back("Parser::parseFunctionBody -> @ifblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }

                Stmt ifStmt = parseIf(toks, pos, atLine);
                body.push_back(move(ifStmt));

                // ждём @endblock
                if (pos >= toks.size() ||
                    toks[pos].type != Token::AT ||
                    pos + 1 >= toks.size() ||
                    toks[pos + 1].type != Token::IDENT ||
                    toks[pos + 1].text != "endblock")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(20,
                        "'@ifblock' opened at line " + to_string(atLine) +
                        " is never closed with '@endblock'", tk.line);
                    e.stack.push_back("Parser::parseFunctionBody -> @endblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }
                pos += 2;   // съели @endblock
                continue;
            }

            if (name == "endblock") {
                CompileError e(21,
                    "'@endblock' without matching '@ifblock'", atLine);
                e.stack.push_back("Parser::parseFunctionBody -> @endblock");
                e.stack.push_back("at line " + to_string(atLine));
                throw e;
            }

            CompileError e(24,
                "unknown directive '@" + name +
                "' (expected '@ifblock' or '@endblock')", atLine);
            e.stack.push_back("Parser::parseFunctionBody -> directive");
            e.stack.push_back("at line " + to_string(atLine));
            throw e;
        }

        body.push_back(parseStatement(toks, pos));
    }

    return body;
}

// ---------- parse one function ----------
static Function parseFunction(const vector<Token>& toks, size_t& pos) {
    Function fn;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected function name, got '" + tk.text + "'",
            tk.line);
        e.stack.push_back("Parser::parseFunction -> name");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    fn.name = toks[pos].text;
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::COLON) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "failed argument: expected ':' after function name, got '" +
            tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseFunction -> colon");
        e.stack.push_back("function " + fn.name);
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    fn.body = parseFunctionBody(toks, pos, fn.name);
    return fn;
}

// ---------- top-level ----------
Program parseProgram(const vector<Token>& toks) {
    Program prog;
    size_t pos = 0;

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        if (t.type == Token::IDENT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::COLON)
        {
            break;
        }

        if (t.type == Token::IDENT && t.text == "using") {
            Stmt u = parseStatement(toks, pos);
            (void)u;
            prog.usesMath = true;
            continue;
        }

        CompileError e(1,
            "failed argument: only 'using math' allowed before functions, got '" +
            t.text + "'", t.line);
        e.stack.push_back("Parser::parseProgram -> using-block");
        e.stack.push_back("at line " + to_string(t.line));
        throw e;
    }

    while (pos < toks.size() && toks[pos].type != Token::END) {
        Function fn = parseFunction(toks, pos);

        for (auto& f : prog.functions) {
            if (f.name == fn.name) {
                CompileError e(1,
                    "failed argument: duplicate function '" + fn.name + "'",
                    fn.body.empty() ? toks[pos].line : fn.body.front().line);
                e.stack.push_back("Parser::parseProgram -> duplicate check");
                e.stack.push_back("function " + fn.name);
                throw e;
            }
        }
        prog.functions.push_back(move(fn));
    }

    for (size_t i = 0; i < prog.functions.size(); ++i) {
        if (prog.functions[i].name == "main") {
            prog.mainIndex = (int)i;
            break;
        }
    }
    if (prog.mainIndex < 0) {
        CompileError e(0, "no main function found");
        e.stack.push_back("Parser::parseProgram -> findMain");
        e.stack.push_back("at end of file");
        throw e;
    }

    {
        const Function& m = prog.functions[prog.mainIndex];
        if (!hasExitRec(m.body)) {
            CompileError e(2, "no return: missing exit() in main");
            e.stack.push_back("Parser::parseProgram -> checkExit");
            e.stack.push_back("at end of main body");
            throw e;
        }
    }

    return prog;
}