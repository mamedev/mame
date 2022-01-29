#!/bin/sh

BUILD_DIR="build_xcode"
CURRENT_DIR=`pwd`

mkdir -p ../${BUILD_DIR}
cd ../${BUILD_DIR}
cmake .. -G"Xcode" -DASMJIT_TEST=1
cd ${CURRENT_DIR}
