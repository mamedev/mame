#! /usr/bin/env bash
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2017-2025 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
# Copyright (c) 2019      Mohammed Khajapasha <mohammed.khajapasha@intel.com>
# Copyright (c) 2026      Rosen Penev <rosenp@gmail.com>
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

export PS4='# '


_get_source_dir() {
    echo "source__${version}"
}


_get_build_dir() {
    local mingw_part=
    if ${with_mingw}; then
        mingw_part=__windows
    fi

    local char_part=
    if ${unicode_enabled}; then
        char_part=__wchar_t
    elif ${with_unsigned_char}; then
        char_part=__uchar
    else
        char_part=__char
    fi

    local xml_attr_part=
    if ${xml_attr_info_enabled}; then
        xml_attr_part=__attr_info
    fi

    local m32_part=
    if ${with_m32}; then
        m32_part=__m32
    fi

    local ge_part=
    if ${with_ge}; then
        ge_part=__ge
    fi

    local dtd_part=
    if ${with_dtd}; then
        dtd_part=__dtd
    fi

    echo "build__${version}__xml_context_${xml_context}${mingw_part}${char_part}${ge_part}${dtd_part}${xml_attr_part}${m32_part}"
}


_get_coverage_dir() {
    echo "coverage__${version}"
}


_call_cmake() {
    local cmake_args=()

    ${unicode_enabled} \
            && cmake_args+=( -DEXPAT_CHAR_TYPE=wchar_t )

    ${xml_attr_info_enabled} \
            && cmake_args+=( -DEXPAT_ATTR_INFO=ON )

    if [[ ${xml_context} -eq 0 ]]; then
        cmake_args+=( -DEXPAT_CONTEXT_BYTES=OFF )
    else
        cmake_args+=( -DEXPAT_CONTEXT_BYTES=${xml_context} )
    fi

    ${with_mingw} && cmake_args+=( -DCMAKE_TOOLCHAIN_FILE="${abs_source_dir}"/cmake/mingw-toolchain.cmake )
    ${with_m32} && cmake_args+=( -D_EXPAT_M32=ON )
    ${with_ge} || cmake_args+=( -DEXPAT_GE=OFF )
    ${with_dtd} || cmake_args+=( -DEXPAT_DTD=OFF )

    (
        set -x
        cmake "${cmake_args[@]}" "$@" . &>> cmake.log
    )
}


_copy_to() {
    local target_dir="$1"
    [[ -d "${target_dir}" ]] && return 0

    mkdir "${target_dir}"
    git archive --format=tar "${version}" | ( cd "${target_dir}" && tar x )
}


_copy_missing_mingw_libaries() {
    # These extra files are copied because
    # * coverage GCC flags make them needed
    # * With WINEDLLPATH Wine looks for .dll.so in these folders, not .dll
    local target="$1"
    local mingw_gcc_dll_dir="$(dirname "$(ls -1 /usr/lib*/gcc/i686-w64-mingw32/*/{libgcc_s_sjlj-1.dll,libstdc++-6.dll} | head -n1)")"
    for dll in libgcc_s_dw2-1.dll libgcc_s_sjlj-1.dll libstdc++-6.dll; do
        (
            set -x
            ln -s "${mingw_gcc_dll_dir}"/${dll} "${target}"/${dll}
        )
    done

    local mingw_pthread_dll="$(ls -1 /usr/i686-w64-mingw32/lib*/libwinpthread-1.dll 2>/dev/null | head -n1)"
    if [[ -n ${mingw_pthread_dll} ]]; then
        local mingw_pthread_dll_dir="$(dirname "${mingw_pthread_dll}")"
        for dll in libwinpthread-1.dll; do
            source="${mingw_pthread_dll_dir}"/${dll}
            (
                set -x
                ln -s "${source}" "${target}"/${dll}
            )
        done
    fi

    for dll in libexpat{,w}-*.dll; do
        (
            set -x
            ln -s "${abs_build_dir}"/${dll} "${target}"/${dll}
        )
    done
}


_run() {
    local source_dir="$1"
    local build_dir="$2"
    local abs_source_dir="${PWD}/${source_dir}"
    local abs_build_dir="${PWD}/${build_dir}"
    local capture_dir=.

    local BASE_FLAGS='-pipe -Wall -Wextra -pedantic -Wno-overlength-strings'
    BASE_FLAGS+=' --coverage --no-inline'

    ${with_unsigned_char} && BASE_FLAGS="${BASE_FLAGS} -funsigned-char"

    local CFLAGS="-std=c99 ${BASE_FLAGS}"
    local CXXFLAGS="-std=c++11 ${BASE_FLAGS}"

    (
        set -e
        cd "${build_dir}"

        _call_cmake \
                -DCMAKE_C_FLAGS="${CFLAGS}" \
                -DCMAKE_CXX_FLAGS="${CXXFLAGS}"

        (
            set -x
            make &> build.log

            lcov -c -d "${capture_dir}" -i -o "${coverage_info}-zero" &> run.log
        )

        if ${with_mingw}; then
            for d in tests xmlwf ; do
                mkdir -p "${d}"
                _copy_missing_mingw_libaries "${d}"
            done
        fi

        # NOTE: This is meant to mitigate Wine crashing randomly in CI
        if ${with_mingw} && [[ ${CI} = true ]]; then
            local retry='retry --times 5 --'
        else
            local retry=
        fi

        set -x
        make CTEST_OUTPUT_ON_FAILURE=1 test
        if ${with_dtd}; then
            ${retry} make run-xmltest
        fi

        lcov -c -d "${capture_dir}" -o "${coverage_info}-test" &>> run.log
        lcov \
                -a "${coverage_info}-zero" \
                -a "${coverage_info}-test" \
                -o "${coverage_info}-all" \
                &>> run.log

        # Make sure that files overlap in report despite different build folders
        sed "/SF:/ s,${build_dir}/,${source_dir}/," "${coverage_info}-all" > "${coverage_info}"
    ) |& sed 's,^,  ,'
    res=${PIPESTATUS[0]}

    if [[ ${res} -eq 0 ]]; then
        echo PASSED
    else
        echo FAILED >&2
        return 1
    fi
}


_merge_coverage_info() {
    local coverage_dir="$1"
    shift
    local build_dirs=( "$@" )

    mkdir -p "${coverage_dir}"
    (
        local lcov_merge_args=( -q -q )
        for build_dir in "${build_dirs[@]}"; do
            lcov_merge_args+=( -a "${build_dir}/${coverage_info}" )
        done
        lcov_merge_args+=( -o "${coverage_dir}/${coverage_info}" )

        set -x
        lcov "${lcov_merge_args[@]}"
    ) |& tee "${coverage_dir}/merge.log"
}


_clean_coverage_info() {
    local coverage_dir="$1"
    local pattern
    for pattern in \
            '/usr/**mingw**/include/*' \
            '*/CMakeFiles/*' \
            '*/examples/*' \
            '*/tests/*' \
        ; do
        (
            set -x
            lcov -q -q -o "${coverage_dir}/${coverage_info}" -r "${coverage_dir}/${coverage_info}" "${pattern}"
        ) |& tee "${coverage_dir}/clean.log"
    done
}


_render_html_report() {
    local coverage_dir="$1"
    (
        set -x
        genhtml -o "${coverage_dir}" "${coverage_dir}/${coverage_info}" &> "${coverage_dir}/render.log"
    )
}


_show_summary() {
    local coverage_dir="$1"
    (
        set -x
        lcov -q -q -l "${coverage_dir}/${coverage_info}"
    ) | grep -v '^\['
}


_main() {
    version="$(git describe --tags 2>/dev/null || echo HEAD)"
    coverage_info=coverage.info

    local build_dirs=()
    local source_dir="$(_get_source_dir)"
    local coverage_dir="$(_get_coverage_dir)"

    _copy_to "${source_dir}"

    _build_case() {
        local build_dir="$(_get_build_dir)"

        echo "[${build_dir}]"
        _copy_to "${build_dir}"

        # Make sure we don't need to download xmlts.zip over and over again
        if [[ ${#build_dirs[*]} -gt 0 ]]; then
            ln -s "$PWD/${build_dirs[0]}/tests/xmlts.zip" "${build_dir}"/tests/
        fi

        _run "${source_dir}" "${build_dir}"

        build_dirs+=( "${build_dir}" )
    }

    # All combinations:
    with_unsigned_char=false
    with_m32=false
    with_dtd=true
    with_ge=true
    for with_mingw in true false ; do
        for unicode_enabled in true false ; do
            if ${unicode_enabled} && ! ${with_mingw} ; then
                continue
            fi

            for xml_attr_info_enabled in true false ; do
                for xml_context in 0 1024 ; do
                    _build_case
                done
            done
        done
    done

    # Single cases:
    with_unsigned_char=true _build_case
    with_m32=true _build_case
    with_dtd=false with_ge=true _build_case
    with_dtd=false with_ge=false _build_case

    echo
    echo 'Merging coverage files...'
    _merge_coverage_info "${coverage_dir}" "${build_dirs[@]}"

    echo
    echo 'Cleaning coverage files...'
    _clean_coverage_info "${coverage_dir}"

    echo
    echo 'Rendering HTML report...'
    _render_html_report "${coverage_dir}"
    echo "--> ${coverage_dir}/index.html"

    echo
    echo 'Rendering ASCII report...'
    _show_summary "${coverage_dir}"
}


_main
