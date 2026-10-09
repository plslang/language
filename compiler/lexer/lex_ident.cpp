// lexer/lex_ident.cpp - lex identifiers and keywords
#include <string>
#include <cctype>

using namespace std;

struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL, END } type;
    string text;
    int line;
    int col;
};

bool tryLexIdent(const string& src, size_t pos, Token& out, size_t& newPos,
                 int& line, int& col)
{
    if (pos >= src.size()) return false;
    char c = src[pos];
    if (!isalpha((unsigned char)c) && c != '_') return false;

    int startLine = line;
    int startCol  = col;
    string id;
    size_t p = pos;
    int lc = col;

    while (p < src.size() &&
           (isalnum((unsigned char)src[p]) || src[p] == '_'))
    {
        id += src[p++];
        lc++;
    }

    out.type = Token::IDENT;
    out.text = id;
    out.line = startLine;
    out.col  = startCol;

    newPos = p;
    col = lc;
    // line unchanged — identifiers can't contain '\n'
    return true;
}