#!/bin/bash

sudo apt-get --yes install subversion screen gcc g++ cmake ninja-build golang autoconf libtool apache2 python-dev pkg-config zlib1g-dev libgcrypt11-dev

mkdir -p clang
cd clang
git clone https://chromium.googlesource.com/chromium/src/tools/clang
cd ..
clang/clang/scripts/update.py
sudo cp -rf third_party/llvm-build/Release+Asserts/lib/* /usr/local/lib/
sudo cp -rf third_party/llvm-build/Release+Asserts/bin/* /usr/local/bin
