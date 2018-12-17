/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

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

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_splat(float _a)
	{
		return _mm256_set1_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_isplat(uint32_t _a)
	{
		const __m256i splat        = _mm256_set1_epi32(_a);
		const simd256_avx_t result = _mm256_castsi256_ps(splat);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_itof(simd256_avx_t _a)
	{
		const __m256i  itof        = _mm256_castps_si256(_a);
		const simd256_avx_t result = _mm256_cvtepi32_ps(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_avx_t simd_ftoi(simd256_avx_t _a)
	{
		const __m256i ftoi         = _mm256_cvtps_epi32(_a);
		const simd256_avx_t result = _mm256_castsi256_ps(ftoi);

		return result;
	}

	typedef simd256_avx_t simd256_t;

} // namespace bx
