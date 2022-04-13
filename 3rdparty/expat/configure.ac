dnl   configuration script for expat
dnl   Process this file with autoconf to produce a configure script.
dnl
dnl   Copyright 2000 Clark Cooper
dnl
dnl   This file is part of EXPAT.
dnl
dnl   EXPAT is free software; you can redistribute it and/or modify it
dnl   under the terms of the License (based on the MIT/X license) contained
dnl   in the file COPYING that comes with this distribution.
dnl

dnl Ensure that Expat is configured with autoconf 2.69 or newer.
AC_PREREQ(2.69)

dnl Get the version number of Expat, using m4's esyscmd() command to run
dnl the command at m4-generation time. This allows us to create an m4
dnl symbol holding the correct version number. AC_INIT() requires the
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
AC_INIT(expat, expat_version, expat-bugs@libexpat.org)
m4_undefine([expat_version])

AC_CONFIG_SRCDIR([Makefile.in])
AC_CONFIG_AUX_DIR([conftools])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE


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

LIBCURRENT=7    # sync
LIBREVISION=12  # with
LIBAGE=6        # CMakeLists.txt!

AX_APPEND_FLAG([-DHAVE_EXPAT_CONFIG_H], [AM_CPPFLAGS])
AC_CONFIG_HEADER([expat_config.h])

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

AS_IF([test "$GCC" = yes],
  [AX_APPEND_COMPILE_FLAGS([-Wall -Wextra], [AM_CFLAGS])
   dnl Be careful about adding the -fexceptions option; some versions of
   dnl GCC don't support it and it causes extra warnings that are only
   dnl distracting; avoid.
   AX_APPEND_COMPILE_FLAGS([-fexceptions], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-fno-strict-aliasing -Wmissing-prototypes -Wstrict-prototypes], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-pedantic -Wduplicated-cond -Wduplicated-branches -Wlogical-op], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-Wrestrict -Wnull-dereference -Wjump-misses-init -Wdouble-promotion], [AM_CFLAGS])
   AX_APPEND_COMPILE_FLAGS([-Wshadow -Wformat=2 -Wmisleading-indentation], [AM_CFLAGS])])

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
  AX_APPEND_FLAG([-fvisibility=hidden],       [AM_CFLAGS])
  AX_APPEND_FLAG([-DXML_ENABLE_VISIBILITY=1], [AM_CPPFLAGS])])

dnl Checks for header files.
AC_HEADER_STDC

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
AC_TYPE_SIZE_T

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


AS_VAR_SET([EXPATCFG_ON_MINGW],[no])
AS_CASE("${host_os}",
  [mingw*],
  [AS_VAR_SET([EXPATCFG_ON_MINGW],[yes])
   AC_MSG_NOTICE([detected OS: MinGW])])
AM_CONDITIONAL([MINGW], [test x${EXPATCFG_ON_MINGW} = xyes])

AM_CONDITIONAL([UNICODE], [echo -- "${CPPFLAGS}${CFLAGS}" | ${FGREP} XML_UNICODE >/dev/null])


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
AC_MSG_CHECKING([for arc4random_buf (BSD or libbsd)])
AC_LINK_IFELSE([AC_LANG_SOURCE([
    #include <stdlib.h>  /* for arc4random_buf on BSD, for NULL */
    #if defined(HAVE_LIBBSD)
    # include <bsd/stdlib.h>
    #endif
    int main() {
      arc4random_buf(NULL, 0U);
      return 0;
    }
  ])],
  [AC_DEFINE([HAVE_ARC4RANDOM_BUF], [1], [Define to 1 if you have the `arc4random_buf' function.])
   AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])

   AC_MSG_CHECKING([for arc4random (BSD, macOS or libbsd)])
   AC_LINK_IFELSE([AC_LANG_SOURCE([
       #if defined(HAVE_LIBBSD)
       # include <bsd/stdlib.h>
       #else
       # include <stdlib.h>
       #endif
       int main() {
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
       int main() {
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
       #include <stdlib.h>  /* for NULL */
       #include <unistd.h>  /* for syscall */
       #include <sys/syscall.h>  /* for SYS_getrandom */
       int main() {
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
     [AS_VAR_SET(enable_xml_context,1024)])
   AC_DEFINE_UNQUOTED([XML_CONTEXT_BYTES], [${enable_xml_context}],
     [Define to specify how much context to retain around the current parse point.])])

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

AM_CONDITIONAL(WITH_DOCBOOK, [test "x${DOCBOOK_TO_MAN}" != x])

dnl write the Automake flags we set
AC_SUBST([AM_CPPFLAGS])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

dnl updating _EXPAT_OUTPUT_NAME variable to effect the package name in expat.pc file (issue #361)
AC_SUBST(_EXPAT_OUTPUT_NAME, ["$PACKAGE_NAME"])

AC_CONFIG_FILES([Makefile]
  [expat.pc]
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
