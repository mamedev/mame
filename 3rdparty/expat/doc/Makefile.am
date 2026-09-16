#
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2017-2024 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2017      Stephen Groat <stephen@groat.us>
# Copyright (c) 2017      Joe Orton <jorton@redhat.com>
# Copyright (c) 2024      Tomas Korbar <tkorbar@redhat.com>
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

.PHONY: dist-hook  # not inside conditional to avoid automake warning

if WITH_MANPAGE
dist_man_MANS = xmlwf.1

xmlwf.1: xmlwf.xml
	-rm -f $@
	test "x$(DOCBOOK_TO_MAN)" != x && $(DOCBOOK_TO_MAN) $<
	test -f $@ || mv XMLWF.1 $@
endif

if !WITH_DISTRIBUTABLE_MANPAGE
dist-hook:
	@echo 'ERROR: Configure with --with-docbook for "make dist".' 1>&2
	@false
endif

if !WITH_PREBUILT_MANPAGE
CLEANFILES = xmlwf.1
endif

EXTRA_DIST = \
    ok.min.css \
    reference.html \
    style.css \
    xmlwf.xml
