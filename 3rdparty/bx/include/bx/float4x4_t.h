/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4X4_H_HEADER_GUARD
#define BX_FLOAT4X4_H_HEADER_GUARD

#include "simd_t.h"

namespace bx
{
	///
	BX_ALIGN_DECL_16(struct) float4x4_t
	{
		simd128_t col[4];
	};

	///
	simd128_t simd_mul_xyz1(simd128_t _a, const float4x4_t* _b);

	///
	simd128_t simd_mul(simd128_t _a, const float4x4_t* _b);

	///
	void float4x4_mul(float4x4_t* _result, const float4x4_t* _a, const float4x4_t* _b);

	///
	void float4x4_transpose(float4x4_t* _result, const float4x4_t* _mtx);

	///
	void float4x4_inverse(float4x4_t* _result, const float4x4_t* _a);

} // namespace bx

#include "inline/float4x4_t.inl"

#endif // BX_FLOAT4X4_H_HEADER_GUARD
