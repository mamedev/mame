#
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2017-2026 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2017      Tomasz Kłoczko <kloczek@fedoraproject.org>
# Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
# Copyright (c) 2026      Gordon Messmer <gordon.messmer@gmail.com>
# Copyright (c) 2026      Nick Begg <nick@stunttruck.net>
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

include_HEADERS = \
    ../expat_config.h \
    expat.h \
    expat_external.h

lib_LTLIBRARIES = libexpat.la
if WITH_TESTS
noinst_LTLIBRARIES = libtestpat.la
endif

libexpat_la_LDFLAGS = \
    @AM_LDFLAGS@ \
    @LIBM@ \
    -no-undefined \
    -version-info @LIBCURRENT@:@LIBREVISION@:@LIBAGE@
if HAVE_VSCRIPT
libexpat_la_LDFLAGS += $(VSCRIPT_LDFLAGS),@builddir@/libexpat.map
endif

libexpat_la_SOURCES = \
    xmlparse.c \
    xmltok.c \
    xmlrole.c

if WITH_ARC4RANDOM
    libexpat_la_SOURCES += \
        random_arc4random.c \
        random_arc4random.h
endif

if WITH_ARC4RANDOM_BUF
    libexpat_la_SOURCES += \
        random_arc4random_buf.c \
        random_arc4random_buf.h
endif

if ! MINGW
    libexpat_la_SOURCES += \
        random_dev_urandom.c \
        random_dev_urandom.h
endif

if WITH_GETENTROPY
    libexpat_la_SOURCES += \
        random_getentropy.c \
        random_getentropy.h
endif

if WITH_EITHER_GETRANDOM
    libexpat_la_SOURCES += \
        random_getrandom.c \
        random_getrandom.h
endif

if MINGW
    libexpat_la_SOURCES += \
        random_rand_s.c \
        random_rand_s.h
endif

if WITH_TESTS
libtestpat_la_CPPFLAGS = -DXML_TESTING

libtestpat_la_SOURCES = $(libexpat_la_SOURCES)
endif

doc_DATA = \
    ../AUTHORS \
    ../Changes

install-data-hook:
	cd "$(DESTDIR)$(docdir)" && $(am__mv) Changes changelog

uninstall-local:
	$(RM) "$(DESTDIR)$(docdir)/changelog"

EXTRA_DIST = \
    ascii.h \
    asciitab.h \
    expat_external.h \
    expat.h \
    fallthrough.h \
    iasciitab.h \
    internal.h \
    latin1tab.h \
    libexpat.def.cmake \
    memory_sanitizer.h \
    nametab.h \
    siphash.h \
    utf8tab.h \
    winconfig.h \
    xcsinc.c \
    xmlrole.h \
    xmltok.h \
    xmltok_impl.c \
    xmltok_impl.h \
    xmltok_ns.c
