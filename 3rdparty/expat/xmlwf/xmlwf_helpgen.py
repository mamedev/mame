#! /usr/bin/env python3
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2019 Expat development team
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

import argparse

epilog = """
xmlwf of libexpat is software libre, licensed under the MIT license.
Please report bugs at https://github.com/libexpat/libexpat/issues.  Thank you!
"""

parser = argparse.ArgumentParser(prog='xmlwf', add_help=False,
                                 description='xmlwf - Determines if an XML document is well-formed',
                                 formatter_class=argparse.RawTextHelpFormatter,
                                 epilog=epilog)

input_related = parser.add_argument_group('input control arguments')
input_related.add_argument('-s', action='store_true', help='print an error if the document is not [s]tandalone')
input_related.add_argument('-n', action='store_true', help='enable [n]amespace processing')
input_related.add_argument('-p', action='store_true', help='enable processing external DTDs and [p]arameter entities')
input_related.add_argument('-x', action='store_true', help='enable processing of e[x]ternal entities')
input_related.add_argument('-e', action='store', metavar='ENCODING', help='override any in-document [e]ncoding declaration')
input_related.add_argument('-w', action='store_true', help='enable support for [W]indows code pages')
input_related.add_argument('-r', action='store_true', help='disable memory-mapping and use normal file [r]ead IO calls instead')

output_related = parser.add_argument_group('output control arguments')
output_related.add_argument('-d', action='store', metavar='DIRECTORY', help='output [d]estination directory')
output_mode = output_related.add_mutually_exclusive_group()
output_mode.add_argument('-c', action='store_true', help='write a [c]opy of input XML, not canonical XML')
output_mode.add_argument('-m', action='store_true', help='write [m]eta XML, not canonical XML')
output_mode.add_argument('-t', action='store_true', help='write no XML output for [t]iming of plain parsing')
output_related.add_argument('-N', action='store_true', help='enable adding doctype and [n]otation declarations')

parser.add_argument('files', metavar='FILE', nargs='*', help='file to process (default: STDIN)')

info = parser.add_argument_group('info arguments')
info = info.add_mutually_exclusive_group()
info.add_argument('-h', action='store_true', help='show this [h]elp message and exit')
info.add_argument('-v', action='store_true', help='show program\'s [v]ersion number and exit')


if __name__ == '__main__':
    parser.print_help()
