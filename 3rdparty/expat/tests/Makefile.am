#
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2017-2024 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2017-2022 Rhodri James <rhodri@wildebeest.org.uk>
# Copyright (c) 2020      Jeffrey Walton <noloader@gmail.com>
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

SUBDIRS = . benchmark

AM_CPPFLAGS = @AM_CPPFLAGS@ -I$(srcdir)/../lib -DXML_TESTING

check_PROGRAMS = runtests runtests_cxx
TESTS = runtests runtests_cxx

# To support MinGW and Non-MinGW at the same time:
LOG_DRIVER = $(srcdir)/../test-driver-wrapper.sh

runtests_SOURCES = \
    acc_tests.c \
    alloc_tests.c \
    basic_tests.c \
    chardata.c \
    common.c \
    dummy.c \
    handlers.c \
    memcheck.c \
    minicheck.c \
    misc_tests.c \
    ns_tests.c \
    nsalloc_tests.c \
    runtests.c \
    structdata.c

runtests_cxx_SOURCES = \
    acc_tests_cxx.cpp \
    alloc_tests_cxx.cpp \
    basic_tests_cxx.cpp \
    chardata_cxx.cpp \
    common_cxx.cpp \
    dummy_cxx.cpp \
    handlers_cxx.cpp \
    memcheck_cxx.cpp \
    minicheck_cxx.cpp \
    misc_tests_cxx.cpp \
    nsalloc_tests_cxx.cpp \
    ns_tests_cxx.cpp \
    runtests_cxx.cpp \
    structdata_cxx.cpp

runtests_LDADD = ../lib/libtestpat.la
runtests_cxx_LDADD = ../lib/libtestpat.la

runtests_LDFLAGS = @AM_LDFLAGS@ @LIBM@
runtests_cxx_LDFLAGS = @AM_LDFLAGS@ @LIBM@

EXTRA_DIST = \
    acc_tests.h \
    alloc_tests.h \
    basic_tests.h \
    chardata.h \
    common.h \
    dummy.h \
    handlers.h \
    misc_tests.h \
    ns_tests.h \
    nsalloc_tests.h \
    structdata.h \
    minicheck.h \
    memcheck.h \
    README.md \
    udiffer.py \
    xmltest.log.expected \
    xmltest.sh
