#!/bin/sh

BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_TEST=1"

echo "== [Configuring Build - Debug] =="
eval cmake . -B build/Debug -DCMAKE_BUILD_TYPE=Debug ${BUILD_OPTIONS} "$@"
echo ""

echo "== [Configuring Build - Release] =="
eval cmake . -B build/Release -DCMAKE_BUILD_TYPE=Release ${BUILD_OPTIONS} "$@"
echo ""
