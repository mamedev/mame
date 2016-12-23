/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD128_REF_H_HEADER_GUARD
#define BX_SIMD128_REF_H_HEADER_GUARD

#include <math.h> // sqrtf

#define simd_shuf_xAzC simd_shuf_xAzC_ni
#define simd_shuf_yBwD simd_shuf_yBwD_ni
#define simd_rcp simd_rcp_ni
#define simd_orx simd_orx_ni
#define simd_orc simd_orc_ni
#define simd_neg simd_neg_ni
#define simd_madd simd_madd_ni
#define simd_nmsub simd_nmsub_ni
#define simd_div_nr simd_div_nr_ni
#define simd_selb simd_selb_ni
#define simd_sels simd_sels_ni
#define simd_not simd_not_ni
#define simd_abs simd_abs_ni
#define simd_clamp simd_clamp_ni
#define simd_lerp simd_lerp_ni
#define simd_rsqrt simd_rsqrt_ni
#define simd_rsqrt_nr simd_rsqrt_nr_ni
#define simd_rsqrt_carmack simd_rsqrt_carmack_ni
#define simd_sqrt_nr simd_sqrt_nr_ni
#define simd_log2 simd_log2_ni
#define simd_exp2 simd_exp2_ni
#define simd_pow simd_pow_ni
#define simd_cross3 simd_cross3_ni
#define simd_normalize3 simd_normalize3_ni
#define simd_dot3 simd_dot3_ni
#define simd_dot simd_dot_ni
#define simd_ceil simd_ceil_ni
#define simd_floor simd_floor_ni

#include "simd_ni.inl"

namespace bx
{
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			template<> \
			BX_SIMD_FORCE_INLINE simd128_ref_t simd_swiz_##_x##_y##_z##_w(simd128_ref_t _a) \
			{ \
				simd128_ref_t result; \
				result.ixyzw[0] = _a.ixyzw[ELEM##_x]; \
				result.ixyzw[1] = _a.ixyzw[ELEM##_y]; \
				result.ixyzw[2] = _a.ixyzw[ELEM##_z]; \
				result.ixyzw[3] = _a.ixyzw[ELEM##_w]; \
				return result; \
			}

#include "simd128_swizzle.inl"

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask) \
			template<> \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_ref_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return 0 != (tmp&(_mask) ); \
			} \
			\
			template<> \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_ref_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return (_mask) == (tmp&(_mask) ); \
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
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_shuf_yBxA(simd128_ref_t _a, simd128_ref_t _b)
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
		result.fxyzw[0] = sqrtf(_a.fxyzw[0]);
		result.fxyzw[1] = sqrtf(_a.fxyzw[1]);
		result.fxyzw[2] = sqrtf(_a.fxyzw[2]);
		result.fxyzw[3] = sqrtf(_a.fxyzw[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_ref_t simd_rsqrt_est(simd128_ref_t _a)
	{
		simd128_ref_t result;
		result.fxyzw[0] = 1.0f / sqrtf(_a.fxyzw[0]);
		result.fxyzw[1] = 1.0f / sqrtf(_a.fxyzw[1]);
		result.fxyzw[2] = 1.0f / sqrtf(_a.fxyzw[2]);
		result.fxyzw[3] = 1.0f / sqrtf(_a.fxyzw[3]);
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

} // namespace bx

#endif // BX_SIMD128_REF_H_HEADER_GUARD
