// main.cpp - entry point for pl compiler
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <stdexcept>

using namespace std;

// ---------- version / colors (defined in config.cpp) ----------
extern const char* PL_VERSION;
extern const char* PL_LANG_VERSION;

extern const char* C_RESET;
extern const char* C_RED;
extern const char* C_GREEN;
extern const char* C_YELLOW;
extern const char* C_MAGENTA;
extern const char* C_CYAN;
extern const char* C_BOLD;
extern const char* C_DIM;

// ---------- shared error type ----------
struct CompileError : public std::exception {
    int code;
    string message;
    vector<string> stack;
    int line;
    CompileError(int c, string msg, int ln = -1)
        : code(c), message(move(msg)), line(ln) {}
    const char* what() const noexcept override { return message.c_str(); }
};

// ---------- token (mirrors lexer/tokens.cpp) ----------
struct Token {
    enum Type { IDENT, NUMBER, STRING, LPAREN, RPAREN, COLON, DOT, EQUAL,
                PLUS, MINUS, STAR, SLASH, END } type;
    string text;
    int line;
    int col;
};

// ---------- expressions (mirrors parser/ast.cpp) ----------
struct Expr {
    enum Kind { NUM, STR, VAR, BINOP } kind;

    long long numValue = 0;      // NUM
    string    strValue;          // STR
    string    varName;           // VAR
    char      op = 0;            // BINOP: '+', '-', '*', '/'
    shared_ptr<Expr> lhs;        // BINOP
    shared_ptr<Expr> rhs;        // BINOP

    int line = 0;
};
using ExprPtr = shared_ptr<Expr>;

// ---------- AST (mirrors parser/ast.cpp) ----------
struct Stmt {
    enum Kind { PRINT, EXIT, CALL, DECL, ASSIGN, DECLARE, GLOBAL, USING } kind;

    bool newline = false;
    ExprPtr printExpr;           // PRINT

    int exitCode = 0;            // EXIT

    string callName;             // CALL

    string varType;              // DECL / DECLARE / GLOBAL
    string varName;              // DECL / ASSIGN / DECLARE / GLOBAL
    ExprPtr varInit;             // DECL / ASSIGN / GLOBAL

    string usingModule;          // USING

    int line = 0;
};

struct Function {
    string name;
    vector<Stmt> body;
};

struct Program {
    vector<Function> functions;
    int mainIndex = -1;
    bool usesMath = false;
};

// ---------- cross-module prototypes ----------
vector<Token> lexSource(const string& src);
Program parseProgram(const vector<Token>& toks);
string generateCpp(const Program& prog, const string& srcName);

// ---------- utils ----------
static string readFile(const string& path) {
    ifstream f(path, ios::binary);
    if (!f) throw CompileError(11, "cannot open file: " + path);
    ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static string baseNameNoExt(const string& path) {
    size_t slash = path.find_last_of("/\\");
    string name = (slash == string::npos) ? path : path.substr(slash + 1);
    size_t dot = name.find_last_of('.');
    if (dot != string::npos) name = name.substr(0, dot);
    return name;
}

static string getExtension(const string& path) {
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of('.');
    if (dot == string::npos) return "";
    if (slash != string::npos && dot < slash) return "";
    return path.substr(dot + 1);
}

static void printError(const CompileError& e, const string& srcPath,
                       const string& source)
{
    cerr << C_RED << C_BOLD << "error[" << e.code << "]" << C_RESET
         << C_RED << ": " << e.message << C_RESET << "\n";

    if (e.line > 0) {
        cerr << C_CYAN << "  --> " << C_RESET
             << srcPath << C_YELLOW << ":" << e.line << C_RESET << "\n";

        istringstream ss(source);
        string ln;
        int cur = 1;
        while (getline(ss, ln)) {
            if (cur == e.line) {
                cerr << C_DIM << "   |" << C_RESET << "\n";
                cerr << C_YELLOW << " " << cur << " | " << C_RESET
                     << ln << "\n";
                cerr << C_DIM << "   |" << C_RESET << "\n";
                break;
            }
            cur++;
        }
    }

    cerr << C_MAGENTA << "stacktrace:" << C_RESET << "\n";
    for (const auto& s : e.stack) {
        cerr << C_DIM << "  at " << C_RESET << C_MAGENTA << s << C_RESET << "\n";
    }
}

static void printInfo(const string& msg) {
    cout << C_GREEN << "[pl]" << C_RESET << " " << msg << "\n";
}

int main(int argc, char** argv) {
    if (argc >= 2 && string(argv[1]) == "--version") {
        cout << "pl " << PL_VERSION << " (" << PL_LANG_VERSION << ")\n";
        return 0;
    }
    if (argc < 2 || string(argv[1]) == "--help" || string(argv[1]) == "-h") {
        cout << C_BOLD << "pl" << C_RESET << " " << PL_VERSION
             << " - compiler for .pla -> C++\n\n";
        cout << "usage: " << argv[0] << " <file.pla>\n";
        cout << "       " << argv[0] << " --version\n";
        cout << "       " << argv[0] << " --help\n";
        return (argc < 2) ? 1 : 0;
    }

    string inputPath = argv[1];
    string ext = getExtension(inputPath);
    if (ext != "pla") {
        cerr << C_RED << "[pl] error:" << C_RESET
             << " expected '.pla' file, got '." << ext << "'\n";
        return 1;
    }

    string source;
    try {
        source = readFile(inputPath);
    } catch (const CompileError& e) {
        printError(e, inputPath, "");
        return e.code;
    }

    try {
        printInfo("reading " + inputPath);

        auto tokens = lexSource(source);
        printInfo("lexed " + to_string(tokens.size()) + " tokens");

        Program prog = parseProgram(tokens);
        printInfo("parsed " + to_string(prog.functions.size()) + " function(s)");
        if (prog.usesMath) printInfo("using math");

        string cpp = generateCpp(prog, inputPath);

        string base = baseNameNoExt(inputPath);
        string cppPath = base + ".cpp";
        string binPath = base;

        printInfo("generating " + cppPath);
        {
            ofstream out(cppPath);
            if (!out) throw CompileError(11, "cannot write " + cppPath);
            out << cpp;
        }

        string cmd = "g++ -O2 -std=c++17 \"" + cppPath + "\" -o \"" + binPath + "\"";
        printInfo("compiling: " + cmd);

        int rc = system(cmd.c_str());
        if (rc != 0) {
            cerr << C_RED << "[pl] error:" << C_RESET
                 << " C++ compiler failed (code " << rc << ")\n";
            return 12;
        }

        cout << C_GREEN << "[pl] success:" << C_RESET
             << " built " << C_BOLD << binPath << C_RESET << "\n";
        return 0;

    } catch (const CompileError& e) {
        printError(e, inputPath, source);
        return e.code;
    } catch (const exception& e) {
        cerr << C_RED << "[pl] internal error:" << C_RESET
             << " " << e.what() << "\n";
        return 1;
    }
}