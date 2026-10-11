// lexer/lex_source.cpp - main tokenizer loop
#include "lexer.hpp"
#include <cctype>

using namespace std;

vector<Token> lexSource(const string& src) {
    vector<Token> tokens;
    size_t pos = 0;
    int line = 1, col = 1;

    while (pos < src.size()) {
        char c = src[pos];

        if (c == '\n') { line++; col = 1; pos++; continue; }
        if (isspace((unsigned char)c)) { col++; pos++; continue; }

        // ';' is ignored — treated like whitespace
        if (c == ';') { col++; pos++; continue; }

        // comments: -- until end of line
        if (c == '-' && pos + 1 < src.size() && src[pos+1] == '-') {
            while (pos < src.size() && src[pos] != '\n') { pos++; col++; }
            continue;
        }

        Token tk;
        size_t newPos = pos;
        bool matched = false;

        if (!matched && tryLexIdent (src, pos, tk, newPos, line, col)) matched = true;
        if (!matched && tryLexNumber(src, pos, tk, newPos, line, col)) matched = true;
        if (!matched && tryLexString(src, pos, tk, newPos, line, col)) matched = true;
        if (!matched && tryLexPunct (src, pos, tk, newPos, line, col)) matched = true;

        if (matched) {
            tokens.push_back(tk);
            pos = newPos;
            continue;
        }

        throw CompileError(10,
            string("unexpected character '") + c + "'", line);
    }

    Token end;
    end.type = Token::END;
    end.text = "";
    end.line = line;
    end.col = col;
    tokens.push_back(end);
    return tokens;
}