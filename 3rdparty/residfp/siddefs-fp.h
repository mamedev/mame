/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2026 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 1999 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SIDDEFS_FP_H
#define SIDDEFS_FP_H

// Check c++ standard
#if __cplusplus >= 202302L
#  ifndef HAVE_CXX23
#    define HAVE_CXX23
#  endif
#endif

#if __cplusplus >= 202002L
#  ifndef HAVE_CXX20
#    define HAVE_CXX20
#  endif
#endif

#if __cplusplus >= 201703L
#  define LIKELY    [[ likely ]]
#  define UNLIKELY  [[ unlikely ]]
#  define MAYBE_UNUSED [[ maybe_unused ]]
#  ifndef HAVE_CXX17
#    define HAVE_CXX17
#  endif
#else
#  define LIKELY
#  define UNLIKELY
#  define MAYBE_UNUSED
#endif

#if __cplusplus >= 201402L
#  ifndef HAVE_CXX14
#    define HAVE_CXX14
#  endif
#endif

#if __cplusplus >= 201103L
#  define MAKE_UNIQUE(type, ...) std::make_unique<type>(__VA_ARGS__)
#  ifndef HAVE_CXX11
#    define HAVE_CXX11
#  endif
#else
#  define MAKE_UNIQUE(type, ...) std::unique_ptr<type>(new type(__VA_ARGS__))
#endif

#if __cplusplus < 201103L
#  error "This is not a C++11 compiler"
#endif

// Compilation configuration.
#define RESIDFP_BRANCH_HINTS 1

// Compiler specifics.
#if defined(__GNUC__) || defined(__clang__)
#  define HAVE_BUILTIN_EXPECT 1
#else
#  define HAVE_BUILTIN_EXPECT 0
#endif

// Branch prediction macros, lifted off the Linux kernel.
#if RESIDFP_BRANCH_HINTS && HAVE_BUILTIN_EXPECT
#  define likely(x)      __builtin_expect(!!(x), 1)
#  define unlikely(x)    __builtin_expect(!!(x), 0)
#else
#  define likely(x)      (x)
#  define unlikely(x)    (x)
#endif

extern "C"
{
#ifndef __VERSION_CC__
extern const char* residfp_version_string;
#else
const char* residfp_version_string = "1.2.3";
#endif
}

// Inlining on/off.
#define RESIDFP_INLINING 1
#define RESIDFP_INLINE inline

#endif // SIDDEFS_FP_H
