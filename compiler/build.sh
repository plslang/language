#!/usr/bin/env bash
# build.sh - build the pl compiler
#
# usage:
#   ./build.sh              # release (-O2)
#   ./build.sh debug        # debug (-O0 -g, no sanitizers)
#   ./build.sh asan         # address sanitizer
#   ./build.sh ubsan        # undefined-behavior sanitizer
#   ./build.sh tsan         # thread sanitizer (на будущее)
#   ./build.sh clean        # удалить артефакты
#   ./build.sh help         # эта справка

set -euo pipefail

# ---------- configuration ----------
SRC_MAIN="main.cpp config.cpp"
SRC_DIRS="lexer parser compiler"
INCLUDES="-I."
STD="-std=c++17"

OUT_RELEASE="pl"
OUT_DEBUG="pl_debug"
OUT_ASAN="pl_asan"
OUT_UBSAN="pl_ubsan"
OUT_TSAN="pl_tsan"

WARN="-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-unused-parameter"

# ---------- colors ----------
if [ -t 1 ]; then
    C_RESET="\033[0m"
    C_BOLD="\033[1m"
    C_GREEN="\033[1;32m"
    C_YELLOW="\033[1;33m"
    C_RED="\033[1;31m"
    C_CYAN="\033[1;36m"
    C_DIM="\033[2m"
else
    C_RESET=""; C_BOLD=""; C_GREEN=""; C_YELLOW=""
    C_RED=""; C_CYAN=""; C_DIM=""
fi

info()  { echo -e "${C_GREEN}[build]${C_RESET} $*"; }
warn()  { echo -e "${C_YELLOW}[build]${C_RESET} $*"; }
err()   { echo -e "${C_RED}[build]${C_RESET} $*" >&2; }
step()  { echo -e "${C_CYAN}  ->${C_RESET} $*"; }

# ---------- helpers ----------
collect_sources() {
    local files="$SRC_MAIN"
    for d in $SRC_DIRS; do
        if [ -d "$d" ]; then
            files="$files $d/*.cpp"
        fi
    done
    echo "$files"
}

compile() {
    local out="$1"
    local flags="$2"
    local sources
    sources="$(collect_sources)"

    step "output:  $out"
    step "flags:   $flags"
    step "sources: $sources"

    # shellcheck disable=SC2086
    g++ $STD $INCLUDES $flags $sources -o "$out"

    info "done: ./$out"
}

# ---------- targets ----------
build_release() {
    info "compiling release ..."
    compile "$OUT_RELEASE" "-O2 -DNDEBUG $WARN"
}

build_debug() {
    info "compiling debug ..."
    compile "$OUT_DEBUG" "-O0 -g3 -DDEBUG $WARN"
}

build_asan() {
    info "compiling with AddressSanitizer ..."
    compile "$OUT_ASAN" "-O1 -g -fsanitize=address -fno-omit-frame-pointer $WARN"
}

build_ubsan() {
    info "compiling with UndefinedBehaviorSanitizer ..."
    compile "$OUT_UBSAN" "-O1 -g -fsanitize=undefined -fno-omit-frame-pointer $WARN"
}

build_tsan() {
    info "compiling with ThreadSanitizer ..."
    compile "$OUT_TSAN" "-O1 -g -fsanitize=thread -fno-omit-frame-pointer $WARN"
}

clean() {
    info "cleaning ..."
    rm -f "$OUT_RELEASE" "$OUT_DEBUG" "$OUT_ASAN" "$OUT_UBSAN" "$OUT_TSAN"
    rm -f ./*.o ./lexer/*.o ./parser/*.o ./compiler/*.o
    rm -f ./*.gcda ./*.gcno ./*.gcov
    info "clean done"
}

usage() {
    cat <<EOF
${C_BOLD}pl build script${C_RESET}

usage:
  ./build.sh              release build (-O2)
  ./build.sh debug        debug build (-O0 -g3, DEBUG defined)
  ./build.sh asan         AddressSanitizer build
  ./build.sh ubsan        UndefinedBehaviorSanitizer build
  ./build.sh tsan         ThreadSanitizer build
  ./build.sh all          build release + debug + asan + ubsan
  ./build.sh clean        remove all build artifacts
  ./build.sh help         show this help

outputs:
  release -> ./$OUT_RELEASE
  debug   -> ./$OUT_DEBUG
  asan    -> ./$OUT_ASAN
  ubsan   -> ./$OUT_UBSAN
  tsan    -> ./$OUT_TSAN
EOF
}

build_all() {
    build_release
    build_debug
    build_asan
    build_ubsan
}

# ---------- entry ----------
case "${1:-release}" in
    ""|release|rel|r)   build_release ;;
    debug|dbg|d)        build_debug ;;
    asan|a)             build_asan ;;
    ubsan|u)            build_ubsan ;;
    tsan|t)             build_tsan ;;
    all)                build_all ;;
    clean|c)            clean ;;
    help|-h|--help)     usage ;;
    *)
        err "unknown target: $1"
        echo
        usage
        exit 1
        ;;
esac