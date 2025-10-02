#!/bin/sh

CURRENT_DIR="`pwd`"
BUILD_DIR="${CURRENT_DIR}/../build"
BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_TEST=1"

echo "== [Configuring Build - Release_ASAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_ASAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=address
echo ""

echo "== [Configuring Build - Release_UBSAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_UBSAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=undefined
echo ""

echo "== [Configuring Build - Release_MSAN] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release_MSAN" ${BUILD_OPTIONS} -DCMAKE_BUILD_TYPE=Release -DASMJIT_SANITIZE=memory
echo ""
