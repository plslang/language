// lexer/lex_punct.cpp - lex punctuation: ( ) : . = + - * /
#include <string>

using namespace std;

struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL,
                PLUS, MINUS, STAR, SLASH, END } type;
    string text;
    int line;
    int col;
};

bool tryLexPunct(const string& src, size_t pos, Token& out, size_t& newPos,
                 int& line, int& col)
{
    if (pos >= src.size()) return false;

    char c = src[pos];
    Token::Type t;
    bool ok = true;

    switch (c) {
        case '(': t = Token::LPAREN; break;
        case ')': t = Token::RPAREN; break;
        case ':': t = Token::COLON;  break;
        case '.': t = Token::DOT;    break;
        case '=': t = Token::EQUAL;  break;
        case '+': t = Token::PLUS;   break;
        case '-': t = Token::MINUS;  break;
        case '*': t = Token::STAR;   break;
        case '/': t = Token::SLASH;  break;
        default:  ok = false;        break;
    }

    if (!ok) return false;

    out.type = t;
    out.text = string(1, c);
    out.line = line;
    out.col  = col;

    newPos = pos + 1;
    col++;
    return true;
}