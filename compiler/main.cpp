// main.cpp - entry point for pl compiler
#include "common.hpp"
#include "config.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "compiler/compiler.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <stdexcept>

using namespace std;

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

// ---------- pretty error printer ----------
static void printError(const CompileError& e, const string& srcPath,
                       const string& source)
{
    // ── заголовок
    cerr << C_RED << C_BOLD << "error[" << e.code << "]" << C_RESET
         << C_RED << ": " << e.message << C_RESET << "\n";

    // ── строка + подсветка
    if (e.line > 0) {
        // собираем все строки исходника
        vector<string> lines;
        {
            istringstream ss(source);
            string ln;
            while (getline(ss, ln)) lines.push_back(ln);
        }

        int width = to_string(e.line).size();

        cerr << C_CYAN << "  --> " << C_RESET
             << srcPath << C_YELLOW << ":" << e.line << C_RESET << "\n";

        cerr << C_DIM << string(width + 2, ' ') << "|" << C_RESET << "\n";

        if (e.line >= 1 && e.line <= (int)lines.size()) {
            // строка
            cerr << C_YELLOW << " " << e.line << " " << C_RESET
                 << C_DIM << "|" << C_RESET << " "
                 << lines[e.line - 1] << "\n";

            // подчёркивание под всей строкой
            cerr << C_DIM << string(width + 2, ' ') << "|" << C_RESET << "\n";
        } else {
            cerr << C_YELLOW << " " << e.line << " " << C_RESET
                 << C_DIM << "|" << C_RESET << " "
                 << C_DIM << "(line not available)" << C_RESET << "\n";
        }
    }

    // ── стектрейс
    if (!e.stack.empty()) {
        cerr << C_MAGENTA << "  stack:" << C_RESET << "\n";
        for (const auto& s : e.stack) {
            cerr << C_DIM << "    at " << C_RESET
                 << C_MAGENTA << s << C_RESET << "\n";
        }
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