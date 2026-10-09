// lexer/lex_comment.cpp - lex -- comments
#include <string>

using namespace std;

// Skips a `-- comment` up to (but not including) end of line.
// Returns the new position. Updates col; line is not touched (comment ends
// before '\n', the newline itself is consumed by the main loop).
size_t skipComment(const string& src, size_t pos, int& col) {
    if (pos + 1 >= src.size()) return pos;
    if (src[pos] != '-' || src[pos+1] != '-') return pos;

    size_t p = pos;
    while (p < src.size() && src[p] != '\n') {
        p++;
        col++;
    }
    return p;
}