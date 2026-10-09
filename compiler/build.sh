#!/usr/bin/env bash
# build.sh - build the pl compiler
set -e

echo "[build] compiling pl ..."

g++ -O2 -std=c++17 \
    main.cpp \
    config.cpp \
    lexer/*.cpp \
    parser/*.cpp \
    compiler/*.cpp \
    -o pl

echo "[build] done: ./pl"