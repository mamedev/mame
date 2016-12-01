/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD256_AVX_H_HEADER_GUARD
#define BX_SIMD256_AVX_H_HEADER_GUARD

#include "simd_ni.inl"

namespace bx
{

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ld(const void* _ptr)
	{
		return _mm256_load_ps(reinterpret_cast<const float*>(_ptr) );
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd256_avx_t _a)
	{
		_mm256_store_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ld(float _x, float _y, float _z, float _w, float _A, float _B, float _C, float _D)
	{
		return _mm256_set_ps(_D, _C, _B, _A, _w, _z, _y, _x);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _A, uint32_t _B, uint32_t _C, uint32_t _D)
	{
		const __m256i set          = _mm256_set_epi32(_D, _C, _B, _A, _w, _z, _y, _x);
		const simd256_avx_t result = _mm256_castsi256_ps(set);

		return result;
	}

	typedef simd256_avx_t simd256_t;

} // namespace bx

#endif // BX_SIMD256_AVX_H_HEADER_GUARD
