#!/bin/sh
# USAGE: get-version.sh path/to/expat.h
#
# This script will print Expat's version number on stdout. For example:
#
#   $ ./conftools/get-version.sh ./lib/expat.h
#   1.95.3
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2002 Greg Stein <gstein@users.sourceforge.net>
# Copyright (c) 2017 Kerin Millar <kfm@plushkava.net>
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

if test $# = 0; then
  echo "ERROR: pathname for expat.h was not provided."
  echo ""
  echo "USAGE: $0 path/to/expat.h"
  exit 1
fi
if test $# != 1; then
  echo "ERROR: too many arguments were provided."
  echo ""
  echo "USAGE: $0 path/to/expat.h"
  exit 1
fi

hdr="$1"
if test ! -r "$hdr"; then
  echo "ERROR: '$hdr' does not exist, or is not readable."
  exit 1
fi

MAJOR_VERSION=$(sed -n -e '/MAJOR_VERSION/s/[^0-9]*//gp' "$hdr")
MINOR_VERSION=$(sed -n -e '/MINOR_VERSION/s/[^0-9]*//gp' "$hdr")
MICRO_VERSION=$(sed -n -e '/MICRO_VERSION/s/[^0-9]*//gp' "$hdr")

printf '%s.%s.%s' "$MAJOR_VERSION" "$MINOR_VERSION" "$MICRO_VERSION"
