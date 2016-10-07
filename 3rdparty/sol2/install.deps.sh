#!/usr/bin/env bash

# This script installs and configures the dependencies for the project

case `uname` in 
    Darwin)     export OS_NAME="osx"    ;;
    Linux)      export OS_NAME="linux"  ;;
esac

echo "Building on: ${OS_NAME}"

if env | grep -qE '^(?:TRAVIS|CI)='; then
#    We're on Travis, intialize variables:
    echo "Detected CI Build -> CI=${CI}"
else
#   We're building locally
    export CI=false
    echo "Detected Local Build -> CI=${CI}"
fi

export_compiler_vars() {
    case ${COMPILER} in
        appleclang*) 
            export CC=clang
            export CXX=clang++
        ;;

        clang*)
            export CC=$(echo ${COMPILER} | sed 's/\+//g')
            export CXX=${COMPILER}
        ;;

        g++-*)
            export CC=$(echo ${COMPILER} | sed 's/\+/c/g')
            export CXX=${COMPILER} 
        ;;

        *) echo "Invalid compiler version" ; exit 2 ;;
    esac

    echo "CC=${CC}"
    $CC --version

    echo "CXX=${CXX}"
    $CXX --version
}

install_os_deps() {
    # Install all of the OS specific OS dependencies
    echo "Install: os-based dependencies"

    local wd=`pwd`

    case ${OS_NAME} in
        osx)
            export HOMEBREW_NO_EMOJI=1

            echo "brew update ..."; brew update > /dev/null

            case ${COMPILER} in
                appleclang*) ;;

                g++-5)
                    brew install gcc5 
                    brew link gcc5 --overwrite --force
                ;;

                g++-4.9) ;;

                *) echo "Invalid compiler version" ; exit 2 ;;
            esac

            brew install ninja

            local lua_pkg
            case ${LUA_VERSION} in
                lua53)      lua_pkg=lua53   ;;
                lua52)      lua_pkg=lua     ;;
                lua51)      lua_pkg=lua51   ;; 
                luajit52)   lua_pkg=luajit  ;; 
                luajit)     lua_pkg=luajit  ;;
                *)  echo "Invalid Lua Version for OSX" 
                    exit 2 
                    ;;
            esac

            brew install ${lua_pkg}
        ;;

        linux) 
            # no extras currently
        ;;
    esac

    cd ${wd}
}
