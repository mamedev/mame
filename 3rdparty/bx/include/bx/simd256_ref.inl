/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD256_REF_H_HEADER_GUARD
#define BX_SIMD256_REF_H_HEADER_GUARD

#include "simd_ni.inl"

namespace bx
{
	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(const void* _ptr)
	{
		const simd128_t* ptr = reinterpret_cast<const simd128_t*>(_ptr);
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd128_t>(&ptr[0]);
		result.simd128_1 = simd_ld<simd128_t>(&ptr[1]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd256_ref_t& _a)
	{
		simd128_t* result = reinterpret_cast<simd128_t*>(_ptr);
		simd_st<simd128_t>(&result[0], _a.simd128_0);
		simd_st<simd128_t>(&result[1], _a.simd128_1);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(float _x, float _y, float _z, float _w, float _a, float _b, float _c, float _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd128_t>(_x, _y, _z, _w);
		result.simd128_1 = simd_ld<simd128_t>(_a, _b, _c, _d);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ild<simd128_t>(_x, _y, _z, _w);
		result.simd128_1 = simd_ild<simd128_t>(_a, _b, _c, _d);
		return result;
	}

} // namespace bx

#endif // BX_SIMD256_REF_H_HEADER_GUARD
