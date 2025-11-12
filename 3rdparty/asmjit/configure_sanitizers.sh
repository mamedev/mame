#!/bin/sh

BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_TEST=1"

echo "== [Configuring Build - Release_ASAN] =="
eval cmake . -B build/Release_ASAN ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=address "$@"
echo ""

echo "== [Configuring Build - Release_MSAN] =="
eval cmake . -B build/Release_MSAN ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=memory "$@"
echo ""

echo "== [Configuring Build - Release_UBSAN] =="
eval cmake . -B build/Release_UBSAN ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=undefined "$@"
echo ""
