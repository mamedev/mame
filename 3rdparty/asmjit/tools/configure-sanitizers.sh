#!/bin/sh

CURRENT_DIR=`pwd`
BUILD_DIR="build"
BUILD_OPTIONS="-DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASMJIT_TEST=1"

echo "** Configuring '${BUILD_DIR}_rel_asan' [Sanitize=Address] **"
mkdir -p ../${BUILD_DIR}_rel_asan
cd ../${BUILD_DIR}_rel_asan
eval cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release ${BUILD_OPTIONS} -DASMJIT_SANITIZE=address
cd ${CURRENT_DIR}

echo "** Configuring '${BUILD_DIR}_rel_ubsan' [Sanitize=Undefined] **"
mkdir -p ../${BUILD_DIR}_rel_ubsan
cd ../${BUILD_DIR}_rel_ubsan
eval cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release ${BUILD_OPTIONS} -DASMJIT_SANITIZE=undefined
cd ${CURRENT_DIR}
