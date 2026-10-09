// config.cpp - version, colors, and small utilities shared across modules
#include <string>
#include <sstream>
#include <ctime>
#include <cctype>

using namespace std;

// ---------- version ----------
const char* PL_VERSION      = "0.7.0";
const char* PL_LANG_VERSION = "pla-lang v0.6";

// ---------- ANSI colors ----------
const char* C_RESET   = "\033[0m";
const char* C_RED     = "\033[1;31m";
const char* C_GREEN   = "\033[1;32m";
const char* C_YELLOW  = "\033[1;33m";
const char* C_MAGENTA = "\033[1;35m";
const char* C_CYAN    = "\033[1;36m";
const char* C_BOLD    = "\033[1m";
const char* C_DIM     = "\033[2m";

// ---------- date ----------
string isoDateNow() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", lt);
    return string(buf);
}

// ---------- cpp helpers ----------
string escapeCpp(const string& s) {
    string out;
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"':  out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            case '\r': out += "\\r"; break;
            default: out += c;
        }
    }
    return out;
}

string cppStringLiteral(const string& s) {
    return "\"" + escapeCpp(s) + "\"";
}

string cppIdentFor(const string& fn, const string& var) {
    return fn + "__" + var;
}

string cppGlobalIdent(const string& name) {
    return "g__" + name;
}

string typeFor(const string& t) {
    if (t == "int")    return "long long";
    if (t == "string") return "std::string";
    return "auto";
}

bool isNumericLiteral(const string& s) {
    if (s.empty()) return false;
    for (char c : s) if (!isdigit((unsigned char)c)) return false;
    return true;
}

string defaultFor(const string& type) {
    if (type == "int")    return "0";
    if (type == "string") return "\"\"";
    return "{}";
}