#! /usr/bin/env bash
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2024-2026 Sebastian Pipping <sebastian@pipping.org>
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

set -e -u -o pipefail

cd "$(dirname "$(type -P "$0")")"

checks_to_enable=(
    bugprone-narrowing-conversions
    bugprone-suspicious-string-compare
    misc-no-recursion
    readability-avoid-const-params-in-decls
    readability-named-parameter
)
checks_to_enable_flat="${checks_to_enable[*]}"  # i.e. flat string separated by spaces

checks_to_disable=(
    # Would need a closer look before any adjustments
    clang-analyzer-optin.performance.Padding

    # Used only in xmlwf, manually checked to be good enough for now
    clang-analyzer-security.insecureAPI.strcpy
)
checks_to_disable_flat="${checks_to_disable[*]}"  # i.e. flat string separated by spaces

checks="${checks_to_enable_flat// /,},-${checks_to_disable_flat// /,-}"

args=(
    --checks="${checks}"
    --header-filter='.*'  # .. to display errors from all non-system headers
    --warnings-as-errors=\*
)

flags=(
    -std=c99

    -Ilib/

    -DENCODING_FOR_FUZZING=UTF-8
    -DXML_ATTR_INFO
    -DXML_CLANG_TIDY
    -DXML_DTD
    -DXML_GE
    -DXML_NS
    -DXML_TESTING
)

if [[ $# -gt 0 ]]; then
    files=( "$@" )
else
    # For the list of excluded files please note:
    # https://github.com/libexpat/libexpat/issues/119
    files=( $(
        git ls-files -- \*.c | grep -v \
        -e '^lib/xcsinc\.c$' \
        -e '^xmlwf/ct\.c$' \
        -e '^xmlwf/xmlmime\.c$' \
        -e '^xmlwf/win32filemap\.c$' \
    ) )
fi

set -x

type -P clang-tidy

clang-tidy --version

clang-tidy --checks="${checks}" --verify-config

clang-tidy --checks="${checks}" --list-checks

# These are the checks clang-tidy has *more* that so far we are missing out on,
# for good and for bad
clang-tidy --checks=\* --list-checks \
    | grep -v -f <(clang-tidy --checks="${checks}" --list-checks | xargs -n1) \
    | sed 's,\(\s*\)\([^-]\+\)-[^ ]\+,\1\2-*,' \
    | sort -u \
    | sed 's/^$/Disabled checks (simplified):/'

pwd

exec clang-tidy "${args[@]}" "${files[@]}" -- "${flags[@]}"
