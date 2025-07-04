#! /usr/bin/env bash
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2019-2022 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2024      Dag-Erling Smørgrav <des@des.dev>
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

filename="${1:-tests/xmltest.log}"

sed -i.bak \
        -e '# convert DOS line endings to Unix without resorting to dos2unix' \
        -e $'s/\r//' \
        \
        -e 's/^wine: Call .* msvcrt\.dll\._wperror, aborting$/ibm49i02.dtd: No such file or directory/' \
        \
        -e '/^wine: /d' \
        -e '/^Application tried to create a window, but no driver could be loaded.$/d' \
        -e '/^Make sure that your X server is running and that $DISPLAY is set correctly.$/d' \
        -e '/^err:systray:initialize_systray Could not create tray window$/d' \
        -e '/^[0-9a-f]\+:err:/d' \
        -e '/^wine client error:/d' \
        -e '/^In ibm\/invalid\/P49\/: Unhandled exception: unimplemented .\+/d' \
        \
        "${filename}"
