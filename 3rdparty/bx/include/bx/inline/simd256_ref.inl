/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(const void* _ptr)
	{
		const simd256_ref_t::type* ptr = reinterpret_cast<const simd256_ref_t::type*>(_ptr);
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd256_ref_t::type>(&ptr[0]);
		result.simd128_1 = simd_ld<simd256_ref_t::type>(&ptr[1]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd256_ref_t& _a)
	{
		simd256_ref_t* result = reinterpret_cast<simd256_ref_t*>(_ptr);
		simd_st<simd256_ref_t::type>(&result[0], _a.simd128_0);
		simd_st<simd256_ref_t::type>(&result[1], _a.simd128_1);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ld(float _x, float _y, float _z, float _w, float _a, float _b, float _c, float _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ld<simd256_ref_t::type>(_x, _y, _z, _w);
		result.simd128_1 = simd_ld<simd256_ref_t::type>(_a, _b, _c, _d);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w, uint32_t _a, uint32_t _b, uint32_t _c, uint32_t _d)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ild<simd256_ref_t::type>(_x, _y, _z, _w);
		result.simd128_1 = simd_ild<simd256_ref_t::type>(_a, _b, _c, _d);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_splat(float _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_splat<simd256_ref_t::type>(_a);
		result.simd128_1 = simd_splat<simd256_ref_t::type>(_a);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_isplat(uint32_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_isplat<simd256_ref_t::type>(_a);
		result.simd128_1 = simd_isplat<simd256_ref_t::type>(_a);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_itof(simd256_ref_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_itof(_a.simd128_0);
		result.simd128_1 = simd_itof(_a.simd128_1);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd256_ref_t simd_ftoi(simd256_ref_t _a)
	{
		simd256_ref_t result;
		result.simd128_0 = simd_ftoi(_a.simd128_0);
		result.simd128_1 = simd_ftoi(_a.simd128_1);
		return result;
	}

} // namespace bx
