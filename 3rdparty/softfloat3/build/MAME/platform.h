
/*============================================================================

This C header file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3e, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/
#
/*----------------------------------------------------------------------------
Softfloat 3 MAME modifications
*----------------------------------------------------------------------------*/
#ifdef LSB_FIRST
#define LITTLEENDIAN 1
#else
#define LITTLEENDIAN 0
#endif

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/
#define INLINE static inline

/*----------------------------------------------------------------------------
*----------------------------------------------------------------------------*/

#if defined(_MSC_VER) && !defined(__clang__)

#define _INC_MALLOC 0
#include <intrin.h>

// MSVC has __lzcnt16 as well, but opts-GCC.h expects __lzcnt for uint16_t and uint32_t
#if defined(_M_IX86) || defined(_M_AMD64)
#define __builtin_clz __lzcnt
#endif // defined(_M_IX86) || defined(_M_AMD64)
#if defined(_M_AMD64)
#define SOFTFLOAT_BUILTIN_CLZ 1
#define __builtin_clzll __lzcnt64
#endif // defined(_M_AMD64)

#else // defined(_MSC_VER)

// true for GCC and Clang on Intel and ARM, and MSVC on Intel.
#define SOFTFLOAT_BUILTIN_CLZ 1
#if defined(PTR64)
#define SOFTFLOAT_INTRINSIC_INT128 1
#endif // defined(PTR64)

#endif // defined(_MSC_VER)

#include "../../source/include/opts-GCC.h"
