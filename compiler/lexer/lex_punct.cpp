// lexer/lex_punct.cpp - lex punctuation and operators
#include "lexer.hpp"

using namespace std;

bool tryLexPunct(const string& src, size_t pos, Token& out, size_t& newPos,
                 int& line, int& col)
{
    if (pos >= src.size()) return false;

    char c = src[pos];
    char n = (pos + 1 < src.size()) ? src[pos + 1] : '\0';

    Token::Type t;
    size_t consumed = 1;

    switch (c) {
        case '=':
            if (n == '=') { t = Token::EQEQ; consumed = 2; }
            else          { t = Token::EQUAL; }
            break;
        case '!':
            if (n == '=') { t = Token::NEQ; consumed = 2; }
            else          { t = Token::BANG; }
            break;
        case '<':
            if (n == '=') { t = Token::LTE; consumed = 2; }
            else          { t = Token::LT; }
            break;
        case '>':
            if (n == '=') { t = Token::GTE; consumed = 2; }
            else          { t = Token::GT; }
            break;
        case '(': t = Token::LPAREN; break;
        case ')': t = Token::RPAREN; break;
        case ':': t = Token::COLON;  break;
        case '.': t = Token::DOT;    break;
        case '+': t = Token::PLUS;   break;
        case '-': t = Token::MINUS;  break;
        case '*': t = Token::STAR;   break;
        case '/': t = Token::SLASH;  break;
        case '@': t = Token::AT;     break;
        default:  return false;
    }

    out.type = t;
    out.text = src.substr(pos, consumed);
    out.line = line;
    out.col  = col;

    newPos = pos + consumed;
    col += (int)consumed;
    return true;
}