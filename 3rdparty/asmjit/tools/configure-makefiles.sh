#!/bin/sh

CURRENT_DIR="`pwd`"
BUILD_DIR="${CURRENT_DIR}/../build"
BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_TEST=1"

echo "== [Configuring Build - Debug] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Debug" -DCMAKE_BUILD_TYPE=Debug ${BUILD_OPTIONS}
echo ""

echo "== [Configuring Build - Release] =="
eval cmake "${CURRENT_DIR}/.." -B "${BUILD_DIR}/Release" -DCMAKE_BUILD_TYPE=Release ${BUILD_OPTIONS}
echo ""
