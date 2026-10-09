// lexer/lex_string.cpp - lex string literals with escapes
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL, END } type;
    string text;
    int line;
    int col;
};

struct CompileError : public std::exception {
    int code;
    string message;
    vector<string> stack;
    int line;
    CompileError(int c, string msg, int ln = -1)
        : code(c), message(move(msg)), line(ln) {}
    const char* what() const noexcept override { return message.c_str(); }
};

bool tryLexString(const string& src, size_t pos, Token& out, size_t& newPos,
                  int& line, int& col)
{
    if (pos >= src.size() || src[pos] != '"') return false;

    int startLine = line;
    int startCol  = col;
    size_t p = pos + 1;
    int lc = col + 1;
    int ln = line;
    string s;

    while (p < src.size() && src[p] != '"') {
        if (src[p] == '\\' && p + 1 < src.size()) {
            p++;
            lc++;
            char e = src[p++];
            lc++;
            switch (e) {
                case 'n': s += '\n'; break;
                case 't': s += '\t'; break;
                case 'r': s += '\r'; break;
                case '\\': s += '\\'; break;
                case '"': s += '"'; break;
                default: s += e; break;
            }
        } else {
            if (src[p] == '\n') { ln++; lc = 1; }
            else lc++;
            s += src[p++];
        }
    }

    if (p >= src.size()) {
        throw CompileError(10, "unterminated string literal", startLine);
    }

    p++; // skip closing quote
    lc++;

    out.type = Token::STRING;
    out.text = s;
    out.line = startLine;
    out.col  = startCol;

    newPos = p;
    line = ln;
    col = lc;
    return true;
}