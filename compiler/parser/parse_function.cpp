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
        // [WHILE]
        if (s.kind == Stmt::WHILE) {
            if (hasExitRec(s.whileBody)) return true;
        }
    }
    return false;
}

// ---------- recursive hasReturn ----------
static bool hasReturnRec(const vector<Stmt>& body) {
    for (const auto& s : body) {
        if (s.kind == Stmt::RETURN) return true;
        if (s.kind == Stmt::IF) {
            if (hasReturnRec(s.thenBody)) return true;
            if (hasReturnRec(s.elseBody)) return true;
        }
        // [WHILE]
        if (s.kind == Stmt::WHILE) {
            if (hasReturnRec(s.whileBody)) return true;
        }
    }
    return false;
}

// ---------- helper: is this a new function definition? ----------
static bool startsFunction(const vector<Token>& toks, size_t pos) {
    if (pos >= toks.size()) return false;
    if (toks[pos].type != Token::IDENT) return false;

    if (pos + 1 >= toks.size()) return false;

    // IDENT ':'
    if (toks[pos + 1].type == Token::COLON) return true;

    // IDENT '(' '!'
    if (toks[pos + 1].type == Token::LPAREN &&
        pos + 2 < toks.size() &&
        toks[pos + 2].type == Token::BANG)
    {
        return true;
    }

    return false;
}

// ---------- parse !type ----------
static FuncRet parseRetType(const vector<Token>& toks, size_t& pos,
                            const string& fnName)
{
    if (pos >= toks.size() || toks[pos].type != Token::LPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(46,
            "expected '(' after function name in '" + fnName +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseRetType -> lparen");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::BANG) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(47,
            "expected '!' in return type of '" + fnName +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseRetType -> bang");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::IDENT) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(48,
            "expected type name (void|bool|int) in '" + fnName +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseRetType -> type");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    string typeName = toks[pos].text;
    FuncRet ret;
    if      (typeName == "void") ret = FuncRet::VOID;
    else if (typeName == "bool") ret = FuncRet::BOOL;
    else if (typeName == "int")  ret = FuncRet::INT;
    else {
        CompileError e(49,
            "unknown return type '" + typeName +
            "' in '" + fnName + "' (expected void|bool|int)",
            toks[pos].line);
        e.stack.push_back("Parser::parseRetType -> unknown type");
        e.stack.push_back("at line " + to_string(toks[pos].line));
        throw e;
    }
    pos++;

    if (pos >= toks.size() || toks[pos].type != Token::RPAREN) {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(50,
            "expected ')' after return type of '" + fnName +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseRetType -> rparen");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }
    pos++;

    return ret;
}

// ---------- parse body of a function ----------
static vector<Stmt> parseFunctionBody(const vector<Token>& toks, size_t& pos,
                                      const string& fnName)
{
    vector<Stmt> body;

    // [WHILE] на верхнем уровне функции всё обёрнуто.
    ParseCtx top;   // все поля false

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        // новая функция?
        if (startsFunction(toks, pos)) break;

        if (t.type == Token::AT) {
            if (pos + 1 >= toks.size() || toks[pos + 1].type != Token::IDENT) {
                const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                CompileError e(24,
                    "unknown directive '@' (expected '@ifblock', '@whileblock' or '@endblock')",
                    tk.line);
                e.stack.push_back("Parser::parseFunctionBody -> directive");
                e.stack.push_back("at line " + to_string(tk.line));
                throw e;
            }

            const string& name = toks[pos + 1].text;
            int atLine = t.line;

            // ---- @ifblock ----
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
                    e.stack.push_back("Parser::parseFunctionBody -> @ifblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }

                // ctx для if: allowIf сбрасываем — внутри if другой
                // @ifblock не разрешён (E23 в parseIfBody). pass/continue
                // наследуются от top (false) — вне while они невалидны,
                // и parseStatement их отловит сразу.
                ParseCtx ifCtx = top;
                ifCtx.allowIf = false;

                Stmt ifStmt = parseIf(toks, pos, atLine, ifCtx);
                body.push_back(move(ifStmt));

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
                pos += 2;
                continue;
            }

            // ---- [WHILE] @whileblock ----
            if (name == "whileblock") {
                pos += 2;

                if (pos >= toks.size() ||
                    toks[pos].type != Token::IDENT ||
                    toks[pos].text != "while")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(64,
                        "'@whileblock' must be followed by a 'while' statement, got '" +
                        tk.text + "'", tk.line);
                    e.stack.push_back("Parser::parseFunctionBody -> @whileblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }

                Stmt whileStmt = parseWhile(toks, pos, atLine);
                body.push_back(move(whileStmt));

                if (pos >= toks.size() ||
                    toks[pos].type != Token::AT ||
                    pos + 1 >= toks.size() ||
                    toks[pos + 1].type != Token::IDENT ||
                    toks[pos + 1].text != "endblock")
                {
                    const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
                    CompileError e(20,
                        "'@whileblock' opened at line " + to_string(atLine) +
                        " is never closed with '@endblock'", tk.line);
                    e.stack.push_back("Parser::parseFunctionBody -> @endblock");
                    e.stack.push_back("at line " + to_string(tk.line));
                    throw e;
                }
                pos += 2;
                continue;
            }

            // ---- @endblock без открытия ----
            if (name == "endblock") {
                CompileError e(21,
                    "'@endblock' without matching '@ifblock' / '@whileblock'", atLine);
                e.stack.push_back("Parser::parseFunctionBody -> @endblock");
                e.stack.push_back("at line " + to_string(atLine));
                throw e;
            }

            CompileError e(24,
                "unknown directive '@" + name +
                "' (expected '@ifblock', '@whileblock' or '@endblock')", atLine);
            e.stack.push_back("Parser::parseFunctionBody -> directive");
            e.stack.push_back("at line " + to_string(atLine));
            throw e;
        }

        body.push_back(parseStatement(toks, pos, top));
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

    if (pos < toks.size() && toks[pos].type == Token::COLON) {
        fn.retType = FuncRet::VOID;
        pos++;
    } else if (pos < toks.size() && toks[pos].type == Token::LPAREN) {
        fn.retType = parseRetType(toks, pos, fn.name);
        if (pos >= toks.size() || toks[pos].type != Token::COLON) {
            const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
            CompileError e(1,
                "expected ':' after return type of '" + fn.name +
                "', got '" + tk.text + "'", tk.line);
            e.stack.push_back("Parser::parseFunction -> colon");
            e.stack.push_back("function " + fn.name);
            e.stack.push_back("at line " + to_string(tk.line));
            throw e;
        }
        pos++;
    } else {
        const Token& tk = toks[pos < toks.size() ? pos : toks.size() - 1];
        CompileError e(1,
            "expected ':' or '(' after function name '" + fn.name +
            "', got '" + tk.text + "'", tk.line);
        e.stack.push_back("Parser::parseFunction -> colon or paren");
        e.stack.push_back("at line " + to_string(tk.line));
        throw e;
    }

    fn.body = parseFunctionBody(toks, pos, fn.name);

    if (fn.name == "main" && fn.retType != FuncRet::VOID) {
        CompileError e(51,
            "Hey. I know you try to change the type of main, but thats impossible. Good try.",
            fn.body.empty() ? toks[pos].line : fn.body.front().line);
        e.stack.push_back("Parser::parseFunction -> main type check");
        throw e;
    }

    if (fn.retType == FuncRet::INT && !hasReturnRec(fn.body)) {
        CompileError e(52,
            "int function '" + fn.name +
            "' must always have at least one return",
            fn.body.empty() ? 0 : fn.body.front().line);
        e.stack.push_back("Parser::parseFunction -> int return check");
        e.stack.push_back("function " + fn.name);
        throw e;
    }

    return fn;
}

// ---------- top-level ----------
Program parseProgram(const vector<Token>& toks) {
    Program prog;
    size_t pos = 0;

    while (pos < toks.size() && toks[pos].type != Token::END) {
        const Token& t = toks[pos];

        if (startsFunction(toks, pos)) break;

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