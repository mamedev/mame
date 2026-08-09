#! /usr/bin/env bash
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2016-2025 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2019      Philippe Antoine <contact@catenacyber.fr>
# Copyright (c) 2019-2025 Hanno Böck <hanno@gentoo.org>
# Copyright (c) 2024      Alexander Bluhm <alexander.bluhm@gmx.net>
# Copyright (c) 2026      Matthew Fernandez <matthew.fernandez@gmail.com>
# Licensed under the MIT license:
#
# Permission is  hereby granted,  free of charge,  to any  person obtaining
# a  copy  of  this  software   and  associated  documentation  files  (the
# "Software"),  to  deal in  the  Software  without restriction,  including
# without  limitation the  rights  to use,  copy,  modify, merge,  publish,
# distribute, sublicense, and/or sell copies of the Software, and to permit
# persons  to whom  the Software  is  furnished to  do so,  subject to  the
# following conditions:
#
# The above copyright  notice and this permission notice  shall be included
# in all copies or substantial portions of the Software.
#
# THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
# EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
# NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
# DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
# USE OR OTHER DEALINGS IN THE SOFTWARE.

set -e
set -o nounset


ANNOUNCE() {
    local open='\e[1m'
    local close='\e[0m'

    echo -e -n "${open}" >&2
    echo -n "# $*" >&2
    echo -e "${close}" >&2
}


WARNING() {
    local open='\e[1;33m'
    local close='\e[0m'

    echo -e -n "${open}" >&2
    echo -n "WARNING: $*" >&2
    echo -e "${close}" >&2
}


RUN() {
    ANNOUNCE "$@"
    env "$@"
}


populate_environment() {
    : ${MAKE:=make}

    case "${QA_COMPILER}" in
        clang)
            : ${CC:=clang}
            : ${CXX:=clang++}
            : ${LD:=clang++}
            ;;
        gcc)
            : ${CC:=gcc}
            : ${CXX:=g++}
            : ${LD:=ld}
            ;;
    esac

    : ${BASE_COMPILE_FLAGS:="-pipe -Wall -Wextra -pedantic -Wno-overlength-strings"}
    : ${BASE_LINK_FLAGS:=}

    if [[ ${QA_COMPILER} = clang ]]; then
        case "${QA_SANITIZER}" in
            address)
                # https://clang.llvm.org/docs/AddressSanitizer.html
                BASE_COMPILE_FLAGS+=" -g -fsanitize=address -fno-omit-frame-pointer -fno-common"
                BASE_LINK_FLAGS+=" -g -fsanitize=address"
                # macOS's XCode does not support LeakSanitizer and reports error:
                # AddressSanitizer: detect_leaks is not supported on this platform.
                if [[ "$(uname -s)" != Darwin* ]]; then
                    export ASAN_OPTIONS=detect_leaks=1
                fi
                ;;
            cfi)
                BASE_COMPILE_FLAGS+=' -fsanitize=cfi -flto -fvisibility=hidden -fno-sanitize-trap=all -fsanitize-cfi-cross-dso'
                BASE_LINK_FLAGS+=' -fuse-ld=gold'
                ;;
            memory)
                # https://clang.llvm.org/docs/MemorySanitizer.html
                BASE_COMPILE_FLAGS+=" -fsanitize=memory -fno-omit-frame-pointer -g -O2 -fsanitize-memory-track-origins"
                ;;
            undefined)
                # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
                BASE_COMPILE_FLAGS+=" -fsanitize=undefined"
                BASE_LINK_FLAGS+=" -fsanitize=undefined"
                export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1:abort_on_error=1"
                ;;
        esac
    fi


    if [[ ${QA_COMPILER} = gcc ]]; then
        case "${QA_PROCESSOR}" in
            egypt) BASE_COMPILE_FLAGS+=" -fdump-rtl-expand" ;;
            gcov) BASE_COMPILE_FLAGS+=" --coverage -O0" ;;
        esac
    fi


    CFLAGS="-std=c99 ${BASE_COMPILE_FLAGS} ${CFLAGS:-}"
    CXXFLAGS="-std=c++11 ${BASE_COMPILE_FLAGS} ${CXXFLAGS:-}"
    LDFLAGS="${BASE_LINK_FLAGS} ${LDFLAGS:-}"
}


run_cmake() {
    local cmake_args=(
        -DCMAKE_C_COMPILER="${CC}"
        -DCMAKE_C_FLAGS="${CFLAGS}"

        -DCMAKE_CXX_COMPILER="${CXX}"
        -DCMAKE_CXX_FLAGS="${CXXFLAGS}"

        -DCMAKE_LINKER="${LD}"
        -DCMAKE_EXE_LINKER_FLAGS="${LDFLAGS}"
        -DCMAKE_MODULE_LINKER_FLAGS="${LDFLAGS}"
        -DCMAKE_SHARED_LINKER_FLAGS="${LDFLAGS}"

        -DEXPAT_WARNINGS_AS_ERRORS=ON
    )
    RUN cmake "${cmake_args[@]}" "$@" .
}


run_compile() {
    local make_args=(
        VERBOSE=1
        -j2
    )

    RUN "${MAKE}" "${make_args[@]}" clean all
}


run_tests() {
    case "${QA_PROCESSOR}" in
        egypt) return 0 ;;
    esac

    if [[ ${CC} =~ mingw ]]; then
        for i in tests xmlwf ; do
            mingw32_dir="$(dirname "$(ls -1 /usr/lib*/gcc/i686-w64-mingw32/*/{libgcc_s_sjlj-1.dll,libstdc++-6.dll} | head -n1)")"
            RUN ln -s \
                    /usr/i686-w64-mingw32/lib/libwinpthread-1.dll \
                    "${mingw32_dir}"/libgcc_s_dw2-1.dll \
                    "${mingw32_dir}"/libgcc_s_sjlj-1.dll \
                    "${mingw32_dir}"/libstdc++-6.dll \
                    "$PWD"/libexpat{,w}-*.dll \
                    ${i}/
        done
    fi

    local make_args=(
        CTEST_OUTPUT_ON_FAILURE=1
        CTEST_PARALLEL_LEVEL=2
        VERBOSE=1
    )

    RUN "${MAKE}" "${make_args[@]}" test

    if [[ ! $* =~ -DEXPAT_DTD=OFF ]]; then
        # NOTE: This is meant to mitigate Wine crashing randomly in CI
        if [[ ${CC} =~ mingw ]]; then
            local retry='retry --times 5 --'
        else
            local retry=
        fi

        RUN ${retry} "${MAKE}" "${make_args[@]}" run-xmltest
    fi
}


run_processor() {
    if [[ ${QA_COMPILER} != gcc ]]; then
        return 0
    fi

    case "${QA_PROCESSOR}" in
    egypt)
        local DOT_FORMAT="${DOT_FORMAT:-svg}"
        local o="callgraph.${DOT_FORMAT}"
        ANNOUNCE "egypt ...... | dot ...... > ${o}"
        find . -name '*.expand' \
                | sort \
                | xargs -r egypt \
                | unflatten -c 20 \
                | dot -T${DOT_FORMAT} -Grankdir=LR \
                > "${o}"
        ;;
    gcov)
        for gcov_dir in lib xmlwf ; do
        (
            cd "${gcov_dir}" || exit 1
            for gcda_file in $(find . -name '*.gcda' | sort) ; do
                RUN gcov -s .libs/ ${gcda_file}
            done
        )
        done

        RUN find . -name '*.gcov' | sort
        ;;
    esac
}


run() {
    populate_environment
    dump_config

    run_cmake "$@"
    run_compile
    run_tests "$@"
    run_processor
}


dump_config() {
    cat <<EOF
Configuration:
  QA_COMPILER=${QA_COMPILER}  # auto-detected from \$CC and \$CXX
  QA_PROCESSOR=${QA_PROCESSOR}  # GCC only
  QA_SANITIZER=${QA_SANITIZER}

  CFLAGS=${CFLAGS}
  CXXFLAGS=${CXXFLAGS}
  LDFLAGS=${LDFLAGS}

  CC=${CC}
  CXX=${CXX}
  LD=${LD}
  MAKE=${MAKE}

Compiler (\$CC):
EOF
    "${CC}" --version | sed 's,^,  ,'
    echo
}


classify_compiler() {
    local i
    for i in "${CC:-}" "${CXX:-}"; do
        [[ "$i" =~ clang ]] && { echo clang ; return ; }
    done
    echo gcc
}


process_config() {
    case "${QA_COMPILER:=$(classify_compiler)}" in
        clang|gcc) ;;
        *) usage; exit 1 ;;
    esac


    if [[ ${QA_COMPILER} != gcc && -n ${QA_PROCESSOR:-} ]]; then
        WARNING "QA_COMPILER=${QA_COMPILER} is not 'gcc' -- ignoring QA_PROCESSOR=${QA_PROCESSOR}"
    fi

    case "${QA_PROCESSOR:=gcov}" in
        egypt|gcov) ;;
        *) usage; exit 1 ;;
    esac


    if [[ ${QA_COMPILER} != clang && ( ${QA_SANITIZER:-} == cfi || ${QA_SANITIZER:-} == memory ) ]]; then
        WARNING "QA_COMPILER=${QA_COMPILER} is not 'clang' -- ignoring QA_SANITIZER=${QA_SANITIZER}" >&2
        QA_SANITIZER=
    fi

    case "${QA_SANITIZER:=address}" in
        address|cfi|memory|undefined) ;;
        *) usage; exit 1 ;;
    esac
}


usage() {
    cat <<"EOF"
Usage:
  $ ./qa.sh [ARG ..]

Environment variables
  QA_COMPILER=(clang|gcc)                      # default: auto-detected
  QA_PROCESSOR=(egypt|gcov)                    # default: gcov
  QA_SANITIZER=(address|cfi|memory|undefined)  # default: address

EOF
}


main() {
    if [[ ${1:-} = --help ]]; then
        usage; exit 0
    fi

    process_config

    run "$@"
}


main "$@"
