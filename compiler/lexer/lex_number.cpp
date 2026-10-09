// lexer/lex_number.cpp - lex integers
#include "lexer.hpp"
#include <cctype>

using namespace std;

bool tryLexNumber(const string& src, size_t pos, Token& out, size_t& newPos,
                  int& line, int& col)
{
    if (pos >= src.size()) return false;
    if (!isdigit((unsigned char)src[pos])) return false;

    int startLine = line;
    int startCol  = col;
    string num;
    size_t p = pos;
    int lc = col;

    while (p < src.size() && isdigit((unsigned char)src[p])) {
        num += src[p++];
        lc++;
    }

    out.type = Token::NUMBER;
    out.text = num;
    out.line = startLine;
    out.col  = startCol;

    newPos = p;
    col = lc;
    return true;
}