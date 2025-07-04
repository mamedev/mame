#
#                          __  __            _
#                       ___\ \/ /_ __   __ _| |_
#                      / _ \\  /| '_ \ / _` | __|
#                     |  __//  \| |_) | (_| | |_
#                      \___/_/\_\ .__/ \__,_|\__|
#                               |_| XML parser
#
# Copyright (c) 2017-2025 Sebastian Pipping <sebastian@pipping.org>
# Copyright (c) 2018      KangLin <kl222@126.com>
# Copyright (c) 2022      Johnny Jazeix <jazeix@gmail.com>
# Copyright (c) 2023      Sony Corporation / Snild Dolkow <snild@sony.com>
# Copyright (c) 2024      Alexander Bluhm <alexander.bluhm@gmx.net>
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

AUTOMAKE_OPTIONS = \
    dist-bzip2 \
    dist-lzip \
    dist-xz \
    foreign \
    subdir-objects

ACLOCAL_AMFLAGS = -I m4
LIBTOOLFLAGS = --verbose

SUBDIRS = lib # lib goes first to build first
if WITH_EXAMPLES
SUBDIRS += examples
endif
if WITH_TESTS
SUBDIRS += tests  
endif
if WITH_XMLWF
SUBDIRS += xmlwf doc
endif

pkgconfig_DATA = expat.pc
pkgconfigdir = $(libdir)/pkgconfig


dist_cmake_DATA = \
    cmake/autotools/expat.cmake

nodist_cmake_DATA = \
    cmake/autotools/expat-config-version.cmake \
    cmake/autotools/expat-noconfig.cmake \
    cmake/expat-config.cmake

cmakedir = $(libdir)/cmake/expat-@PACKAGE_VERSION@


_EXTRA_DIST_CMAKE = \
    cmake/autotools/expat-noconfig__linux.cmake.in \
    cmake/autotools/expat-noconfig__macos.cmake.in \
    cmake/autotools/expat-noconfig__windows.cmake.in \
    cmake/autotools/expat-package-init.cmake \
    cmake/mingw-toolchain.cmake \
    \
    CMakeLists.txt \
    CMake.README \
    ConfigureChecks.cmake \
    expat.pc.cmake \
    expat_config.h.cmake

_EXTRA_DIST_WINDOWS = \
    win32/build_expat_iss.bat \
    win32/expat.iss \
    win32/MANIFEST.txt \
    win32/README.txt \
    win32/version.rc.cmake

EXTRA_DIST = \
    $(_EXTRA_DIST_CMAKE) \
    $(_EXTRA_DIST_WINDOWS) \
    \
    conftools/expat.m4 \
    conftools/get-version.sh \
    \
    fuzz/xml_lpm_fuzzer.cpp \
    fuzz/xml_lpm_fuzzer.proto \
    fuzz/xml_parsebuffer_fuzzer.c \
    fuzz/xml_parse_fuzzer.c \
    \
    xmlwf/xmlwf_helpgen.py \
    xmlwf/xmlwf_helpgen.sh \
    \
    buildconf.sh \
    Changes \
    README.md \
    \
    fix-xmltest-log.sh \
    test-driver-wrapper.sh


.PHONY: buildlib
buildlib:
	@echo 'ERROR: Running "make buildlib LIBRARY=libexpatw.la"' >&2
	@echo 'ERROR: is no longer supported.  INSTEAD please:' >&2
	@echo 'ERROR:' >&2
	@echo 'ERROR:  * Mass-patch Makefile.am, e.g.' >&2
	@echo 'ERROR:    # find . -name Makefile.am -exec sed \' >&2
	@echo 'ERROR:          -e "s,libexpat\.la,libexpatw.la," \' >&2
	@echo 'ERROR:          -e "s,libexpat_la,libexpatw_la," \' >&2
	@echo 'ERROR:          -i.bak {} +' >&2
	@echo 'ERROR:' >&2
	@echo 'ERROR:  * Run automake to re-generate Makefile.in files' >&2
	@echo 'ERROR:' >&2
	@echo 'ERROR:  * Use "./configure --without-xmlwf" and/or' >&2
	@echo 'ERROR:    "make -C lib all install" to bypass compilation' >&2
	@echo 'ERROR:    of xmlwf (e.g. with -DXML_UNICODE)' >&2
	@echo 'ERROR:' >&2
	@false


.PHONY: run-benchmark
run-benchmark:
	$(MAKE) -C tests/benchmark
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/recset.xml 65535 3
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/aaaaaa_attr.xml 4096 3
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/aaaaaa_cdata.xml 4096 3
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/aaaaaa_comment.xml 4096 3
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/aaaaaa_tag.xml 4096 3
	./run.sh tests/benchmark/benchmark@EXEEXT@ -n $(top_srcdir)/../testdata/largefiles/aaaaaa_text.xml 4096 3

.PHONY: download-xmlts-zip
download-xmlts-zip:
	if test "$(XMLTS_ZIP)" = ""; then \
		wget --output-document=tests/xmlts.zip \
			https://www.w3.org/XML/Test/xmlts20080827.zip; \
	else \
		cp $(XMLTS_ZIP) tests/xmlts.zip; \
	fi

tests/xmlts.zip:
	$(MAKE) download-xmlts-zip

.PHONY: extract-xmlts-zip
extract-xmlts-zip: tests/xmlts.zip
	[ -f $(builddir)/tests/xmlts.zip ] || $(MAKE) download-xmlts-zip  # vpath workaround
	cd tests && unzip -q xmlts.zip

tests/xmlconf: tests/xmlts.zip
	$(MAKE) extract-xmlts-zip

.PHONY: run-xmltest
run-xmltest: tests/xmlconf
if WITH_XMLWF
	[ -d $(builddir)/tests/xmlconf ] || $(MAKE) extract-xmlts-zip  # vpath workaround
	$(MAKE) -C lib
	$(MAKE) -C xmlwf
	$(srcdir)/tests/xmltest.sh "$(abs_builddir)/run.sh $(abs_builddir)/xmlwf/xmlwf@EXEEXT@" 2>&1 | tee $(builddir)/tests/xmltest.log
	$(srcdir)/fix-xmltest-log.sh $(builddir)/tests/xmltest.log
	diff -u $(srcdir)/tests/xmltest.log.expected $(builddir)/tests/xmltest.log
else
	@echo 'ERROR: xmlwf is needed for "make run-xmltest".' >&2
	@echo 'ERROR: Please re-configure without --without-xmlwf.' >&2
	@false
endif

.PHONY: qa
qa:
	QA_COMPILER=clang QA_SANITIZER=address   ./qa.sh
	QA_COMPILER=clang QA_SANITIZER=memory    ./qa.sh
	QA_COMPILER=clang QA_SANITIZER=undefined ./qa.sh
	QA_COMPILER=gcc   QA_PROCESSOR=gcov      ./qa.sh
