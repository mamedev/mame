# expatcfg-compiler-supports-visibility.m4 --
#
# SYNOPSIS
#
#    EXPATCFG_COMPILER_SUPPORTS_VISIBILITY([ACTION-IF-YES],
#                                          [ACTION-IF-NO])
#
# DESCRIPTION
#
#   Check if  the selected compiler supports  the "visibility" attribute
#   and  set   the  variable  "expatcfg_cv_compiler_supports_visibility"
#   accordingly to "yes" or "no".
#
#   In addition, execute ACTION-IF-YES or ACTION-IF-NO.
#
# LICENSE
#
#   Copyright (c) 2018 The Expat Authors.
#
#   Copying and distribution of this file, with or without modification,
#   are permitted in  any medium without royalty  provided the copyright
#   notice and this  notice are preserved.  This file  is offered as-is,
#   without any warranty.

AC_DEFUN([EXPATCFG_COMPILER_SUPPORTS_VISIBILITY],
  [AC_CACHE_CHECK([whether compiler supports visibility],
     [expatcfg_cv_compiler_supports_visibility],
     [AS_VAR_SET([expatcfg_cv_compiler_supports_visibility],[no])
      AS_VAR_COPY([OLDFLAGS],[CFLAGS])
      AS_VAR_APPEND([CFLAGS],[" -fvisibility=hidden -Wall -Werror -Wno-unknown-warning-option"])
      AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
          void __attribute__((visibility("default"))) foo(void);
          void foo(void) {}
        ]])],
        [AS_VAR_SET([expatcfg_cv_compiler_supports_visibility],[yes])])
      AS_VAR_COPY([CFLAGS],[OLDFLAGS])])
   AS_IF([test "$expatcfg_cv_compiler_supports_visibility" = yes],[$1],[$2])])

# end of file
