#!/bin/bash
# Configures builds for our CI environment.

# Print commands and exit on error.
set -ex

if [ "$KOKORO_BUILD_ID" ]; then
    echo "Running job $KOKORO_JOB_NAME"
    TARGET=`echo "$KOKORO_JOB_NAME" | awk -F "/" '{print $NF}'`
fi

if [ ! "$TARGET" ]; then
    if [ "$1" ]; then
        TARGET=$1
    else
        TARGET=release
    fi
fi

echo "Building $TARGET target"

pushd `dirname $0`/../../.. > /dev/null

BUILD_RELEASE=
BUILD_DEBUG=
BUILD_CMAKE=
RUN_TESTS=

if [ "$TARGET" == "presubmit" ]; then
    BUILD_DEBUG=1
    BUILD_RELEASE=1
    BUILD_CMAKE=1
    RUN_TESTS=1
fi

if [ "$TARGET" == "debug" ]; then
    BUILD_DEBUG=1
fi

if [ "$TARGET" == "release" ]; then
    BUILD_RELEASE=1
fi

if [ "$TARGET" == "continuous" ]; then
    BUILD_DEBUG=1
    BUILD_RELEASE=1
    BUILD_CMAKE=1
    RUN_TESTS=1
fi

if [ "$BUILD_DEBUG" == "1" ]; then
    echo "Starting debug build"
    bazel build -c dbg //...

    if [ "$RUN_TESTS" == "1" ]; then
        bazel test -c dbg //...
    fi
fi

if [ "$BUILD_RELEASE" == "1" ]; then
    echo "Starting release build"
    bazel build -c opt //...

    if [ "$RUN_TESTS" == "1" ]; then
        bazel test -c opt //...
    fi
fi

if [ "$BUILD_CMAKE" == "1" ]; then
    echo "Starting cmake build"
    mkdir build
    pushd build
    cmake ..
    make -j$((`nproc`+1))
    popd
fi
