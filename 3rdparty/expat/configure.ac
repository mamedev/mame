dnl   configuration script for expat
dnl   Process this file with autoconf to produce a configure script.
dnl                            __  __            _
dnl                         ___\ \/ /_ __   __ _| |_
dnl                        / _ \\  /| '_ \ / _` | __|
dnl                       |  __//  \| |_) | (_| | |_
dnl                        \___/_/\_\ .__/ \__,_|\__|
dnl                                 |_| XML parser
dnl
dnl   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
dnl   Copyright (c) 2000-2005 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
dnl   Copyright (c) 2001-2003 Greg Stein <gstein@users.sourceforge.net>
dnl   Copyright (c) 2006-2012 Karl Waclawek <karl@waclawek.net>
dnl   Copyright (c) 2016-2025 Sebastian Pipping <sebastian@pipping.org>
dnl   Copyright (c) 2017      S. P. Zeidler <spz@netbsd.org>
dnl   Copyright (c) 2017      Stephen Groat <stephen@groat.us>
dnl   Copyright (c) 2017-2020 Joe Orton <jorton@redhat.com>
dnl   Copyright (c) 2018      Yury Gribov <tetra2005@gmail.com>
dnl   Copyright (c) 2018      Benjamin Peterson <benjamin@python.org>
dnl   Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
dnl   Copyright (c) 2018      KangLin <kl222@126.com>
dnl   Copyright (c) 2019      Mohammed Khajapasha <mohammed.khajapasha@intel.com>
dnl   Copyright (c) 2019      Kishore Kunche <kishore.kunche@intel.com>
dnl   Copyright (c) 2020      Jeffrey Walton <noloader@gmail.com>
dnl   Copyright (c) 2024      Ferenc Géczi <ferenc.gm@gmail.com>
dnl   Copyright (c) 2024      Dag-Erling Smørgrav <des@des.dev>
dnl   Licensed under the MIT license:
dnl
dnl   Permission is  hereby granted,  free of charge,  to any  person obtaining
dnl   a  copy  of  this  software   and  associated  documentation  files  (the
dnl   "Software"),  to  deal in  the  Software  without restriction,  including
dnl   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
dnl   distribute, sublicense, and/or sell copies of the Software, and to permit
dnl   persons  to whom  the Software  is  furnished to  do so,  subject to  the
dnl   following conditions:
dnl
dnl   The above copyright  notice and this permission notice  shall be included
dnl   in all copies or substantial portions of the Software.
dnl
dnl   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
dnl   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
dnl   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
dnl   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
dnl   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
dnl   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
dnl   USE OR OTHER DEALINGS IN THE SOFTWARE.

dnl Ensure that Expat is configured with autoconf 2.69 or newer.
AC_PREREQ([2.69])

dnl Get the version number of Expat, using m4's esyscmd() command to run
dnl the command at m4-generation time. This allows us to create an m4
dnl symbol holding the correct version number. AC_INIT requires the
dnl version number at m4-time, rather than when ./configure is run, so
dnl all this must happen as part of m4, not as part of the shell code
dnl contained in ./configure.
dnl
dnl NOTE: esyscmd() is a GNU M4 extension. Thus, we wrap it in an appropriate
dnl test. I believe this test will work, but I don't have a place with non-
dnl GNU M4 to test it right now.
m4_define([expat_version],
  m4_ifdef([__gnu__],
           [esyscmd(conftools/get-version.sh lib/expat.h)],
           [2.2.x]))
AC_INIT([expat], expat_version, [https://github.com/libexpat/libexpat/issues])
m4_undefine([expat_version])

AC_CONFIG_SRCDIR([Makefile.in])
AC_CONFIG_AUX_DIR([conftools])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])  # to allow argument --disable-maintainer-mode


dnl
dnl Increment LIBREVISION if source code has changed at all
dnl
dnl If the API has changed, increment LIBCURRENT and set LIBREVISION to 0
dnl
dnl If the API changes compatibly (i.e. simply adding a new function
dnl without changing or removing earlier interfaces), then increment LIBAGE.
dnl
dnl If the API changes incompatibly set LIBAGE back to 0
dnl

LIBCURRENT=11  # sync
LIBREVISION=2  # with
LIBAGE=10      # CMakeLists.txt!

AC_CONFIG_HEADERS([expat_config.h])
AH_TOP([#ifndef EXPAT_CONFIG_H
#define EXPAT_CONFIG_H 1])
AH_BOTTOM([#endif // ndef EXPAT_CONFIG_H])

AM_PROG_AR
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

LT_PREREQ([2.4])
LT_INIT([win32-dll])

AC_SUBST(LIBCURRENT)
AC_SUBST(LIBREVISION)
AC_SUBST(LIBAGE)

AC_LANG([C])
AC_PROG_CC_C99

AS_IF([test "${ac_cv_prog_cc_c99}" = no],
  [AC_MSG_ERROR([Expat requires a C99 compiler.])])

AS_IF([test "$GCC" = yes],
  [AX_APPEND_COMPILE_FLAGS([-Wall -Wextra], [AM_CFLAGS])
   dnl Be careful about adding the -fexceptions option; some versions of
   dnl GCC don't support it and it causes extra warnings that are only
   dnl distracting; avoid.
   AX_APPEND_COMPILE_FLAGS([-fexceptions], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing -Wmissing-prototypes -Wstrict-prototypes], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-pedantic -Wduplicated-cond -Wduplicated-branches -Wlogical-op], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-Wrestrict -Wnull-dereference -Wjump-misses-init -Wdouble-promotion], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-Wshadow -Wformat=2 -Wno-pedantic-ms-format -Wmisleading-indentation], [AM_CFLAGS])])

AC_LANG_PUSH([C++])
AC_PROG_CXX

AS_IF([test "$GCC" = yes],
  [AX_APPEND_COMPILE_FLAGS([-Wall -Wextra], [AM_CXXFLAGS])
   dnl Be careful about adding the -fexceptions option; some versions of
   dnl GCC don't support it and it causes extra warnings that are only
   dnl distracting; avoid.
   AX_APPEND_COMPILE_FLAGS([-fexceptions], [AM_CXXFLAGS])
   AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing], [AM_CXXFLAGS])])
AC_LANG_POP([C++])

AS_IF([test "$GCC" = yes],
  [AX_APPEND_LINK_FLAGS([-fno-strict-aliasing],[AM_LDFLAGS])])

dnl patching ${archive_cmds} to affect generation of file "libtool" to fix linking with clang (issue #312)
AS_CASE(["$LD"],[*clang*],
  [AS_CASE(["${host_os}"],
     [*linux*],[archive_cmds='$CC -shared $pic_flag $libobjs $deplibs $compiler_flags $wl-soname $wl$soname -o $lib'])])

EXPATCFG_COMPILER_SUPPORTS_VISIBILITY([
  AX_APPEND_FLAG([-fvisibility=hidden],              [AM_CFLAGS])
  AS_IF([test "${enable_shared}" = yes],
        [AX_APPEND_FLAG([-DXML_ENABLE_VISIBILITY=1], [AM_CPPFLAGS])])])

dnl Checks for typedefs, structures, and compiler characteristics.

dnl We define BYTEORDER to 1234 when the platform is little endian; it
dnl defines it to 4321 when the  platform is big endian.  We also define
dnl WORDS_BIGENDIAN to 1 when the platform is big endian.
dnl
dnl A  long time  ago (early 2000  years) AC_C_BIGENDIAN  was considered
dnl wrong when cross compiling, now (2018, GNU Autoconf 2.69) we assume
dnl it is fine.
AC_C_BIGENDIAN([AC_DEFINE([WORDS_BIGENDIAN], 1)
                AS_VAR_SET([BYTEORDER], 4321)],
               [AS_VAR_SET([BYTEORDER], 1234)])
AC_DEFINE_UNQUOTED([BYTEORDER], $BYTEORDER, [1234 = LILENDIAN, 4321 = BIGENDIAN])

AC_C_CONST

AC_ARG_WITH([xmlwf],
  [AS_HELP_STRING([--without-xmlwf], [do not build xmlwf])],
  [],
  [with_xmlwf=yes])
AM_CONDITIONAL([WITH_XMLWF], [test x${with_xmlwf} = xyes])

AC_ARG_WITH([examples],
  [AS_HELP_STRING([--without-examples], [do not build examples @<:@default=included@:>@])],
  [],
  [with_examples=yes])
AM_CONDITIONAL([WITH_EXAMPLES], [test x${with_examples} = xyes])

AC_ARG_WITH([tests],
  [AS_HELP_STRING([--without-tests], [do not build tests @<:@default=included@:>@])],
  [],
  [with_tests=yes])
AM_CONDITIONAL([WITH_TESTS], [test x${with_tests} = xyes])

AS_IF([test x${with_tests} = xyes],
      [AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])])

AS_VAR_SET([EXPATCFG_ON_MINGW],[no])
AS_CASE("${host_os}",
  [mingw*],
  [AS_VAR_SET([EXPATCFG_ON_MINGW],[yes])
   AC_MSG_NOTICE([detected OS: MinGW])])
AM_CONDITIONAL([MINGW], [test x${EXPATCFG_ON_MINGW} = xyes])

dnl Note: Prefix "_INTERNAL_" here means exclusive use inside of file configure.ac
AM_CONDITIONAL([UNICODE], [echo -- "${CPPFLAGS}${CFLAGS}" | ${FGREP} XML_UNICODE >/dev/null])
AM_CONDITIONAL([_INTERNAL_UNICODE_WCHAR_T], [echo -- "${CPPFLAGS}${CFLAGS}" | ${FGREP} XML_UNICODE_WCHAR_T >/dev/null])
AM_CONDITIONAL([_INTERNAL_MIN_SIZE], [echo -- "${CPPFLAGS}${CFLAGS}" | ${FGREP} XML_MIN_SIZE >/dev/null])
AM_CONDITIONAL([_INTERNAL_LARGE_SIZE], [echo -- "${CPPFLAGS}${CFLAGS}" | ${FGREP} XML_LARGE_SIZE >/dev/null])

LT_LIB_M

AC_ARG_WITH([libbsd],
  [AS_HELP_STRING([--with-libbsd], [utilize libbsd (for arc4random_buf)])],
  [],
  [with_libbsd=no])
AS_IF([test "x${with_libbsd}" != xno],
  [AC_CHECK_LIB([bsd],
     [arc4random_buf],
     [],
     [AS_IF([test "x${with_libbsd}" = xyes],
        [AC_MSG_ERROR([Enforced use of libbsd cannot be satisfied.])])])])
AC_MSG_CHECKING([for arc4random_buf (BSD, libbsd or glibc 2.36+)])
AC_LINK_IFELSE([AC_LANG_SOURCE([
    #if defined(HAVE_LIBBSD)
    # include <bsd/stdlib.h>
    #else
    # include <stdlib.h>  /* for arc4random_buf on BSD */
    #endif
    int main(void) {
      char dummy[[123]];  // double brackets for m4
      arc4random_buf(dummy, 0U);
      return 0;
    }
  ])],
  [AC_DEFINE([HAVE_ARC4RANDOM_BUF], [1], [Define to 1 if you have the `arc4random_buf' function.])
   AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])

   AC_MSG_CHECKING([for arc4random (BSD, macOS, libbsd or glibc 2.36+)])
   AC_LINK_IFELSE([AC_LANG_SOURCE([
       #if defined(HAVE_LIBBSD)
       # include <bsd/stdlib.h>
       #else
       # include <stdlib.h>
       #endif
       int main(void) {
          arc4random();
          return 0;
       }
     ])],
     [AC_DEFINE([HAVE_ARC4RANDOM], [1], [Define to 1 if you have the `arc4random' function.])
      AC_MSG_RESULT([yes])],
     [AC_MSG_RESULT([no])])])

AC_ARG_WITH([getrandom],
  [AS_HELP_STRING([--with-getrandom],
                  [enforce the use of getrandom function in the system @<:@default=check@:>@])
AS_HELP_STRING([--without-getrandom],
               [skip auto detect of getrandom @<:@default=check@:>@])],
  [],
  [with_getrandom=check])

AS_IF([test "x$with_getrandom" != xno],
  [AC_MSG_CHECKING([for getrandom (Linux 3.17+, glibc 2.25+)])
   AC_LINK_IFELSE([AC_LANG_SOURCE([
       #include <stdlib.h>  /* for NULL */
       #include <sys/random.h>
       int main(void) {
         return getrandom(NULL, 0U, 0U);
       }
     ])],
     [AC_DEFINE([HAVE_GETRANDOM], [1], [Define to 1 if you have the `getrandom' function.])
      AC_MSG_RESULT([yes])],
     [AC_MSG_RESULT([no])
      AS_IF([test "x$with_getrandom" = xyes],
        [AC_MSG_ERROR([enforced the use of getrandom --with-getrandom, but not detected])])])])

AC_ARG_WITH([sys_getrandom],
  [AS_HELP_STRING([--with-sys-getrandom],
                  [enforce the use of syscall SYS_getrandom function in the system @<:@default=check@:>@])
AS_HELP_STRING([--without-sys-getrandom],
               [skip auto detect of syscall SYS_getrandom @<:@default=check@:>@])],
  [],
  [with_sys_getrandom=check])

AS_IF([test "x$with_sys_getrandom" != xno],
  [AC_MSG_CHECKING([for syscall SYS_getrandom (Linux 3.17+)])
   AC_LINK_IFELSE([AC_LANG_SOURCE([
       #define _GNU_SOURCE
       #include <stdlib.h>  /* for NULL */
       #include <unistd.h>  /* for syscall */
       #include <sys/syscall.h>  /* for SYS_getrandom */
       int main(void) {
         syscall(SYS_getrandom, NULL, 0, 0);
         return 0;
     }
   ])],
     [AC_DEFINE([HAVE_SYSCALL_GETRANDOM], [1], [Define to 1 if you have `syscall' and `SYS_getrandom'.])
      AC_MSG_RESULT([yes])],
     [AC_MSG_RESULT([no])
      AS_IF([test "x$with_sys_getrandom" = xyes],
        [AC_MSG_ERROR([enforced the use of syscall SYS_getrandom --with-sys-getrandom, but not detected])])])])

dnl Only needed for xmlwf:
AC_CHECK_HEADERS(fcntl.h unistd.h)
AC_TYPE_OFF_T
AC_FUNC_MMAP

AS_IF([test "$ac_cv_func_mmap_fixed_mapped" = "yes"],
  [AS_VAR_SET(FILEMAP,unixfilemap)],
  [AS_VAR_SET(FILEMAP,readfilemap)])
AC_SUBST(FILEMAP)


dnl Some basic configuration:
AC_DEFINE([XML_NS], 1,
          [Define to make XML Namespaces functionality available.])
AC_DEFINE([XML_GE], 1,
          [Define as 1/0 to enable/disable support for general entities.])
AC_DEFINE([XML_DTD], 1,
          [Define to make parameter entity parsing functionality available.])
AC_DEFINE([XML_DEV_URANDOM], 1,
          [Define to include code reading entropy from `/dev/urandom'.])

AC_ARG_ENABLE([xml-attr-info],
  [AS_HELP_STRING([--enable-xml-attr-info],
     [Enable retrieving the byte offsets for attribute names and values @<:@default=no@:>@])],
  [],
  [enable_xml_attr_info=no])
AS_IF([test "x${enable_xml_attr_info}" = "xyes"],
      [AC_DEFINE([XML_ATTR_INFO], 1,
         [Define to allow retrieving the byte offsets for attribute names and values.])])

AC_ARG_ENABLE([xml-context],
  AS_HELP_STRING([--enable-xml-context @<:@COUNT@:>@],
    [Retain context around the current parse point;
     default is enabled and a size of 1024 bytes])
AS_HELP_STRING([--disable-xml-context],
    [Do not retain context around the current parse point]),
    [enable_xml_context=${enableval}])
AS_IF([test "x${enable_xml_context}" != "xno"],
  [AS_IF([test "x${enable_xml_context}" = "xyes" \
            -o "x${enable_xml_context}" = "x"],
     [AS_VAR_SET(enable_xml_context,1024)])],
  [AS_VAR_SET(enable_xml_context,0)])
AC_DEFINE_UNQUOTED([XML_CONTEXT_BYTES], [${enable_xml_context}],
  [Define to specify how much context to retain around the current parse point, 0 to disable.])

AC_ARG_WITH([docbook],
  [AS_HELP_STRING([--with-docbook],
                  [enforce XML to man page compilation @<:@default=check@:>@])
AS_HELP_STRING([--without-docbook],
               [skip XML to man page compilation @<:@default=check@:>@])],
  [],
  [with_docbook=check])

AC_ARG_VAR([DOCBOOK_TO_MAN], [docbook2x-man command])
AS_IF([test "x$with_docbook" != xno],
  [AC_CHECK_PROGS([DOCBOOK_TO_MAN], [docbook2x-man db2x_docbook2man docbook2man docbook-to-man])])
AS_IF([test "x${DOCBOOK_TO_MAN}" = x -a "x$with_docbook" = xyes],
  [AC_MSG_ERROR([Required program 'docbook2x-man' not found.])])
AS_IF([test "x${DOCBOOK_TO_MAN}" != x -a "x$with_docbook" != xno],
  [AS_IF([${DOCBOOK_TO_MAN} --help | grep -i -q -F sgmlbase],
    [AC_MSG_ERROR([Your local ${DOCBOOK_TO_MAN} was found to work with SGML rather
  than XML. Please install docbook2X and use variable DOCBOOK_TO_MAN to point
  configure to command docbook2x-man of docbook2X.
  Or use DOCBOOK_TO_MAN="xmlto man --skip-validation" if you have xmlto around.
  You can also configure using --without-docbook if you can do without a man
  page for xmlwf.])])])

dnl This will make sure that a release tarball shipping a pre-rendered xmlwf man page will
dnl get it installed, when no working flavor of docbook2man is available (or wanted).
dnl This relies on file xmlwf.1 being at least as recent as its source file xmlwf.xml.
AS_IF([test -f "${srcdir}"/doc/xmlwf.1],
  [AM_CONDITIONAL(WITH_MANPAGE, [true])
   AS_IF([test "x$with_docbook" = xno -o "x${DOCBOOK_TO_MAN}" = x],
     [AM_CONDITIONAL(WITH_PREBUILT_MANPAGE, [true])
      AM_CONDITIONAL(WITH_DISTRIBUTABLE_MANPAGE, [false])],
     [AM_CONDITIONAL(WITH_PREBUILT_MANPAGE, [false])
      AM_CONDITIONAL(WITH_DISTRIBUTABLE_MANPAGE, [true])])
   ],
  [AS_IF([test "x$with_docbook" != xno -a "x${DOCBOOK_TO_MAN}" != x],
     [AM_CONDITIONAL(WITH_MANPAGE, [true])
      AM_CONDITIONAL(WITH_DISTRIBUTABLE_MANPAGE, [true])],
     [AM_CONDITIONAL(WITH_MANPAGE, [false])
      AM_CONDITIONAL(WITH_DISTRIBUTABLE_MANPAGE, [false])])
   AM_CONDITIONAL(WITH_PREBUILT_MANPAGE, [false])])

dnl Configure CMake file templates
dnl NOTE: The *_TRUE variables read here are Automake conditionals
dnl       that are either set to "" when enabled or to "#" when disabled
dnl       (because they are used to dynamically comment out certain things)
AS_IF([test "x${enable_xml_attr_info}" = xyes],
  [EXPAT_ATTR_INFO=ON],
  [EXPAT_ATTR_INFO=OFF])
EXPAT_DTD=ON
AS_IF([test "x${_INTERNAL_LARGE_SIZE_TRUE}" = x],
  [EXPAT_LARGE_SIZE=ON],
  [EXPAT_LARGE_SIZE=OFF])
AS_IF([test "x${_INTERNAL_MIN_SIZE_TRUE}" = x],
  [EXPAT_MIN_SIZE=ON],
  [EXPAT_MIN_SIZE=OFF])
EXPAT_NS=ON
AS_IF([test "x${enable_xml_context}" != xno],
  [EXPAT_CONTEXT_BYTES=${enable_xml_context}],
  [EXPAT_CONTEXT_BYTES=OFF])
AS_IF([test "x${UNICODE_TRUE}" = x],
  [AS_IF(
     [test "x${_INTERNAL_UNICODE_WCHAR_T_TRUE}" = x],
        [EXPAT_CHAR_TYPE=wchar_t],
        [EXPAT_CHAR_TYPE=ushort])],
  [EXPAT_CHAR_TYPE=char])
PACKAGE_INIT="${srcdir}"/cmake/autotools/expat-package-init.cmake
LIBDIR_BASENAME="$(basename "${libdir}")"
SO_MAJOR="$(expr "${LIBCURRENT}" - "${LIBAGE}")"
SO_MINOR="${LIBAGE}"
SO_PATCH="${LIBREVISION}"
AC_SUBST([EXPAT_ATTR_INFO])
AC_SUBST([EXPAT_DTD])
AC_SUBST([EXPAT_LARGE_SIZE])
AC_SUBST([EXPAT_MIN_SIZE])
AC_SUBST([EXPAT_NS])
AC_SUBST([EXPAT_CONTEXT_BYTES])
AC_SUBST([EXPAT_CHAR_TYPE])
AC_SUBST_FILE([PACKAGE_INIT])
AC_SUBST([LIBDIR_BASENAME])
AC_SUBST([SO_MAJOR])
AC_SUBST([SO_MINOR])
AC_SUBST([SO_PATCH])

dnl The canonical way of doing this is AC_CHECK_SIZEOF(void *), but
dnl that adds SIZEOF_VOID_P to expat_config.h.in, making it difficult
dnl to have 32-bit and 64-bit versions of libexpat installed on the
dnl same system with a single, shared copy of the header.
AC_COMPUTE_INT(SIZEOF_VOID_P, [sizeof(void *)])
AC_SUBST([SIZEOF_VOID_P])

dnl write the Automake flags we set
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

dnl Emulate the use of CMAKE_SHARED_LIBRARY_PREFIX under CMake
AC_MSG_CHECKING([for shared library name prefix])
AS_CASE("${host_os}",
  [cygwin*], [CMAKE_SHARED_LIBRARY_PREFIX=cyg],
  [CMAKE_SHARED_LIBRARY_PREFIX=lib])
AC_MSG_RESULT([${CMAKE_SHARED_LIBRARY_PREFIX}])
AC_SUBST([CMAKE_SHARED_LIBRARY_PREFIX])

AS_CASE("${host_os}",
  [darwin*], [CMAKE_NOCONFIG_SOURCE=cmake/autotools/expat-noconfig__macos.cmake.in],
  [mingw*|cygwin*], [CMAKE_NOCONFIG_SOURCE=cmake/autotools/expat-noconfig__windows.cmake.in],
  [CMAKE_NOCONFIG_SOURCE=cmake/autotools/expat-noconfig__linux.cmake.in])
AC_CONFIG_FILES([Makefile]
  [expat.pc]
  [cmake/expat-config.cmake]
  [cmake/autotools/expat-config-version.cmake]
  [cmake/autotools/expat-noconfig.cmake:${CMAKE_NOCONFIG_SOURCE}]
  [doc/Makefile]
  [examples/Makefile]
  [lib/Makefile]
  [tests/Makefile]
  [tests/benchmark/Makefile]
  [xmlwf/Makefile])
AC_CONFIG_FILES([run.sh], [chmod +x run.sh])
AC_OUTPUT


AC_MSG_NOTICE([

Automake flags (can be overridden by user flags):
  [AM_CPPFLAGS]: ${AM_CPPFLAGS}
    [AM_CFLAGS]: ${AM_CFLAGS}
  [AM_CXXFLAGS]: ${AM_CXXFLAGS}
   [AM_LDFLAGS]: ${AM_LDFLAGS}

User flags (override Automake flags on conflict):
     CPPFLAGS: ${CPPFLAGS}
       CFLAGS: ${CFLAGS}
     CXXFLAGS: ${CXXFLAGS}
      LDFLAGS: ${LDFLAGS}])
