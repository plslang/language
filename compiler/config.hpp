// config.hpp - declarations for config.cpp
#pragma once

#include "parser/parser.hpp"

#include <string>

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

std::string isoDateNow();

std::string escapeCpp(const std::string& s);
std::string cppStringLiteral(const std::string& s);
std::string cppIdentFor(const std::string& fn, const std::string& var);
std::string cppGlobalIdent(const std::string& name);

std::string typeFor(const std::string& t);
std::string cppRetType(FuncRet r);

bool        isNumericLiteral(const std::string& s);
std::string defaultFor(const std::string& type);