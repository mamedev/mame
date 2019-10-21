/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	BX_CONST_FUNC float sqrt(float);
	BX_CONST_FUNC float rsqrt(float);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                        \
			template<>                                                                      \
			BX_SIMD_FORCE_INLINE simd128_ref_t simd_swiz_##_x##_y##_z##_w(simd128_ref_t _a) \
			{                                                                               \
				simd128_ref_t result;                                                       \
				result.ixyzw[0] = _a.ixyzw[ELEM##_x];                                       \
				result.ixyzw[1] = _a.ixyzw[ELEM##_y];                                       \
				result.ixyzw[2] = _a.ixyzw[ELEM##_z];                                       \
				result.ixyzw[3] = _a.ixyzw[ELEM##_w];                                       \
				return result;                                                              \
			}

#include "simd128_swizzle.inl"

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                  \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_ref_t _test) \
			{                                                                    \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                        \
				             | ( (_test.uxyzw[2]>>31)<<2)                        \
				             | ( (_test.uxyzw[1]>>31)<<1)                        \
				             | (  _test.uxyzw[0]>>31)                            \
				             ;                                                   \
				return 0 != (tmp&(_mask) );                                      \
			}                                                                    \
			                                                                     \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_ref_t _test) \
			{                                                                    \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                        \
				             | ( (_test.uxyzw[2]>>31)<<2)                        \
				             | ( (_test.uxyzw[1]>>31)<<1)                        \
				             | (  _test.uxyzw[0]>>31)                            \
				             ;                                                   \
				return (_mask) == (tmp&(_mask) );                                \
			}

BX_SIMD128_IMPLEMENT_TEST(x    , 0x1);
BX_SIMD128_IMPLEMENT_TEST(y    , 0x2);
BX_SIMD128_IMPLEMENT_TEST(xy   , 0x3);
BX_SIMD128_IMPLEMENT_TEST(z    , 0x4);
BX_SIMD128_IMPLEMENT_TEST(xz   , 0x5);
BX_SIMD128_IMPLEMENT_TEST(yz   , 0x6);
BX_SIMD128_IMPLEMENT_TEST(xyz  , 0x7);
BX_SIMD128_IMPLEMENT_TEST(w    , 0x8);
BX_SIMD128_IMPLEMENT_TEST(xw   , 0x9);
BX_SIMD128_IMPLEMENT_TEST(yw   , 0xa);
BX_SIMD128_IMPLEMENT_TEST(xyw  , 0xb);
BX_SIMD128_IMPLEMENT_TEST(zw   , 0xc);
BX_SIMD128_IMPLEMENT_TEST(xzw  , 0xd);
BX_SIMD128_IMPLEMENT_TEST(yzw  , 0xe);
BX_SIMD128_IMPLEMENT_TEST(xyzw , 0xf);

#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xyAB(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1];
		result.uxyzw[2] = _b.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_ABxy(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _a.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_CDzw(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[3];
		result.uxyzw[2] = _a.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_zwCD(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[3];
		result.uxyzw[2] = _b.uxyzw[2];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xAyB(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0];
		result.uxyzw[1] = _b.uxyzw[0];
		result.uxyzw[2] = _a.uxyzw[1];
		result.uxyzw[3] = _b.uxyzw[1];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_AxBy(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[1];
		result.uxyzw[1] = _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[0];
		result.uxyzw[3] = _b.uxyzw[0];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_zCwD(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[2];
		result.uxyzw[1] = _b.uxyzw[2];
		result.uxyzw[2] = _a.uxyzw[3];
		result.uxyzw[3] = _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_CzDw(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _b.uxyzw[2];
		result.uxyzw[1] = _a.uxyzw[2];
		result.uxyzw[2] = _b.uxyzw[3];
		result.uxyzw[3] = _a.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_ref_t _a)
	{
		return _a.fxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_ref_t _a)
	{
		return _a.fxyzw[1];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_ref_t _a)
	{
		return _a.fxyzw[2];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_ref_t _a)
	{
		return _a.fxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		simd128_ref_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_ref_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ld(float _x, float _y, float _z, float _w)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _x;
		result.fxyzw[1] = _y;
		result.fxyzw[2] = _z;
		result.fxyzw[3] = _w;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _x;
		result.uxyzw[1] = _y;
		result.uxyzw[2] = _z;
		result.uxyzw[3] = _w;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		simd128_ref_t result;
		result.uxyzw[0] = val;
		result.uxyzw[1] = val;
		result.uxyzw[2] = val;
		result.uxyzw[3] = val;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_splat(float _a)
	{
		return simd_ld<simd128_ref_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_isplat(uint32_t _a)
	{
		return simd_ild<simd128_ref_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_zero()
	{
		return simd_ild<simd128_ref_t>(0, 0, 0, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_itof(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = (float)_a.ixyzw[0];
		result.fxyzw[1] = (float)_a.ixyzw[1];
		result.fxyzw[2] = (float)_a.ixyzw[2];
		result.fxyzw[3] = (float)_a.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ftoi(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.ixyzw[0] = (int)_a.fxyzw[0];
		result.ixyzw[1] = (int)_a.fxyzw[1];
		result.ixyzw[2] = (int)_a.fxyzw[2];
		result.ixyzw[3] = (int)_a.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_round(simd128_ref_t _a)
	{
		return simd_round_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_add(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] + _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] + _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] + _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] + _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sub(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] - _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] - _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] - _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] - _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_mul(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] * _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] * _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] * _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] * _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_div(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] / _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] / _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] / _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] / _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rcp_est(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = 1.0f / _a.fxyzw[0];
		result.fxyzw[1] = 1.0f / _a.fxyzw[1];
		result.fxyzw[2] = 1.0f / _a.fxyzw[2];
		result.fxyzw[3] = 1.0f / _a.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sqrt(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = sqrt(_a.fxyzw[0]);
		result.fxyzw[1] = sqrt(_a.fxyzw[1]);
		result.fxyzw[2] = sqrt(_a.fxyzw[2]);
		result.fxyzw[3] = sqrt(_a.fxyzw[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_est(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = rsqrt(_a.fxyzw[0]);
		result.fxyzw[1] = rsqrt(_a.fxyzw[1]);
		result.fxyzw[2] = rsqrt(_a.fxyzw[2]);
		result.fxyzw[3] = rsqrt(_a.fxyzw[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpeq(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] == _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] == _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] == _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] == _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmplt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmple(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] <= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] <= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] <= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] <= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpgt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cmpge(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.fxyzw[0] >= _b.fxyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.fxyzw[1] >= _b.fxyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.fxyzw[2] >= _b.fxyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.fxyzw[3] >= _b.fxyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_min(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] < _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] < _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] < _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] < _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_max(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.fxyzw[0] = _a.fxyzw[0] > _b.fxyzw[0] ? _a.fxyzw[0] : _b.fxyzw[0];
		result.fxyzw[1] = _a.fxyzw[1] > _b.fxyzw[1] ? _a.fxyzw[1] : _b.fxyzw[1];
		result.fxyzw[2] = _a.fxyzw[2] > _b.fxyzw[2] ? _a.fxyzw[2] : _b.fxyzw[2];
		result.fxyzw[3] = _a.fxyzw[3] > _b.fxyzw[3] ? _a.fxyzw[3] : _b.fxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_and(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] & _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_andc(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] & ~_b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] & ~_b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] & ~_b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] & ~_b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_or(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] | _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] | _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] | _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] | _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_xor(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] ^ _b.uxyzw[0];
		result.uxyzw[1] = _a.uxyzw[1] ^ _b.uxyzw[1];
		result.uxyzw[2] = _a.uxyzw[2] ^ _b.uxyzw[2];
		result.uxyzw[3] = _a.uxyzw[3] ^ _b.uxyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sll(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] << _count;
		result.uxyzw[1] = _a.uxyzw[1] << _count;
		result.uxyzw[2] = _a.uxyzw[2] << _count;
		result.uxyzw[3] = _a.uxyzw[3] << _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_srl(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.uxyzw[0] = _a.uxyzw[0] >> _count;
		result.uxyzw[1] = _a.uxyzw[1] >> _count;
		result.uxyzw[2] = _a.uxyzw[2] >> _count;
		result.uxyzw[3] = _a.uxyzw[3] >> _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sra(simd128_ref_t _a, int _count)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] >> _count;
		result.ixyzw[1] = _a.ixyzw[1] >> _count;
		result.ixyzw[2] = _a.ixyzw[2] >> _count;
		result.ixyzw[3] = _a.ixyzw[3] >> _count;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmpeq(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] == _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] == _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] == _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] == _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmplt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_icmpgt(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? 0xffffffff : 0x0;
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? 0xffffffff : 0x0;
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? 0xffffffff : 0x0;
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? 0xffffffff : 0x0;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_imin(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] < _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] < _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] < _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] < _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_imax(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] > _b.ixyzw[0] ? _a.ixyzw[0] : _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] > _b.ixyzw[1] ? _a.ixyzw[1] : _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] > _b.ixyzw[2] ? _a.ixyzw[2] : _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] > _b.ixyzw[3] ? _a.ixyzw[3] : _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_iadd(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] + _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] + _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] + _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] + _b.ixyzw[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_isub(simd128_ref_t _a, simd128_ref_t _b)
	{
		simd128_ref_t result;
		result.ixyzw[0] = _a.ixyzw[0] - _b.ixyzw[0];
		result.ixyzw[1] = _a.ixyzw[1] - _b.ixyzw[1];
		result.ixyzw[2] = _a.ixyzw[2] - _b.ixyzw[2];
		result.ixyzw[3] = _a.ixyzw[3] - _b.ixyzw[3];
		return result;
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_zero()
	{
		return simd_zero<simd128_t>();
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ld(const void* _ptr)
	{
		return simd_ld<simd128_t>(_ptr);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ld(float _x, float _y, float _z, float _w)
	{
		return simd_ld<simd128_t>(_x, _y, _z, _w);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		return simd_ild<simd128_t>(_x, _y, _z, _w);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_splat(const void* _ptr)
	{
		return simd_splat<simd128_t>(_ptr);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_splat(float _a)
	{
		return simd_splat<simd128_t>(_a);
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_isplat(uint32_t _a)
	{
		return simd_isplat<simd128_t>(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_xAzC(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_yBwD(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rcp(simd128_ref_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_orx(simd128_ref_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_orc(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_neg(simd128_ref_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_madd(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_nmsub(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_div_nr(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_selb(simd128_ref_t _mask, simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sels(simd128_ref_t _test, simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_not(simd128_ref_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_abs(simd128_ref_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_clamp(simd128_ref_t _a, simd128_ref_t _min, simd128_ref_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_lerp(simd128_ref_t _a, simd128_ref_t _b, simd128_ref_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt(simd128_ref_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_nr(simd128_ref_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_carmack(simd128_ref_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_sqrt_nr(simd128_ref_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_log2(simd128_ref_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_exp2(simd128_ref_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_pow(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_cross3(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_normalize3(simd128_ref_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_dot3(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_dot(simd128_ref_t _a, simd128_ref_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_ceil(simd128_ref_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_floor(simd128_ref_t _a)
	{
		return simd_floor_ni(_a);
	}

} // namespace bx
