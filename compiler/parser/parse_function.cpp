// parser/parse_function.cpp - parse functions + top-level parseProgram
#include "parser.hpp"

using namespace std;

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

    while (pos < toks.size() && toks[pos].type != Token::END) {
        if (toks[pos].type == Token::IDENT &&
            pos + 1 < toks.size() &&
            toks[pos + 1].type == Token::COLON)
        {
            break;
        }
        fn.body.push_back(parseStatement(toks, pos));
    }

    return fn;
}

Program parseProgram(const vector<Token>& toks) {
    Program prog;
    size_t pos = 0;

    // using directives only allowed before first function
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
        bool hasExit = false;
        for (const auto& s : m.body) {
            if (s.kind == Stmt::EXIT) { hasExit = true; break; }
        }
        if (!hasExit) {
            CompileError e(2, "no return: missing exit() in main");
            e.stack.push_back("Parser::parseProgram -> checkExit");
            e.stack.push_back("at end of main body");
            throw e;
        }
    }

    return prog;
}