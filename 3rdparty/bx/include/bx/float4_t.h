/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_T_H_HEADER_GUARD
#define BX_FLOAT4_T_H_HEADER_GUARD

#include "bx.h"

#define BX_FLOAT4_FORCE_INLINE BX_FORCE_INLINE
#define BX_FLOAT4_INLINE static inline

#if defined(__SSE2__) || (BX_COMPILER_MSVC && (BX_ARCH_64BIT || _M_IX86_FP >= 2) )
#	include "float4_sse.h"
#elif defined(__ARM_NEON__) && !BX_COMPILER_CLANG
#	include "float4_neon.h"
#elif BX_COMPILER_CLANG \
		&& !BX_PLATFORM_EMSCRIPTEN \
		&& !BX_PLATFORM_IOS \
		&& BX_CLANG_HAS_EXTENSION(attribute_ext_vector_type)
#	include "float4_langext.h"
#else
#	ifndef BX_FLOAT4_WARN_REFERENCE_IMPL
#		define BX_FLOAT4_WARN_REFERENCE_IMPL 0
#	endif // BX_FLOAT4_WARN_REFERENCE_IMPL

#	if BX_FLOAT4_WARN_REFERENCE_IMPL
#		pragma message("************************************\nUsing SIMD reference implementation!\n************************************")
#	endif // BX_FLOAT4_WARN_REFERENCE_IMPL

#	include "float4_ref.h"
#endif //

#endif // BX_FLOAT4_T_H_HEADER_GUARD
