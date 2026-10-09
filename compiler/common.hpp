// common.hpp - types shared across the whole compiler
#pragma once

#include <string>
#include <vector>
#include <exception>

// ---------- one and only CompileError ----------
struct CompileError : public std::exception {
    int code;
    std::string message;
    std::vector<std::string> stack;
    int line;
    CompileError(int c, std::string msg, int ln = -1)
        : code(c), message(std::move(msg)), line(ln) {}
    const char* what() const noexcept override { return message.c_str(); }
};