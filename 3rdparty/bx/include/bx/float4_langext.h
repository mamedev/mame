/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_LANGEXT_H_HEADER_GUARD
#define BX_FLOAT4_LANGEXT_H_HEADER_GUARD

#include <math.h>

namespace bx
{
	typedef union float4_t
	{
		float    __attribute__((vector_size(16))) vf;
		int32_t  __attribute__((vector_size(16))) vi;
		uint32_t __attribute__((vector_size(16))) vu;
		float    fxyzw[4];
		int32_t  ixyzw[4];
		uint32_t uxyzw[4];

	} float4_t;

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			BX_FLOAT4_FORCE_INLINE float4_t float4_swiz_##_x##_y##_z##_w(float4_t _a) \
			{ \
				float4_t result; \
				result.vf = __builtin_shufflevector(_a.vf, _a.vf, ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w); \
				return result; \
			}

#include "float4_swizzle.inl"

#undef IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define IMPLEMENT_TEST(_xyzw, _mask) \
			BX_FLOAT4_FORCE_INLINE bool float4_test_any_##_xyzw(float4_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return 0 != (tmp&(_mask) ); \
			} \
			\
			BX_FLOAT4_FORCE_INLINE bool float4_test_all_##_xyzw(float4_t _test) \
			{ \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3) \
				             | ( (_test.uxyzw[2]>>31)<<2) \
				             | ( (_test.uxyzw[1]>>31)<<1) \
				             | (  _test.uxyzw[0]>>31)     \
				             ; \
				return (_mask) == (tmp&(_mask) ); \
			}

IMPLEMENT_TEST(x    , 0x1);
IMPLEMENT_TEST(y    , 0x2);
IMPLEMENT_TEST(xy   , 0x3);
IMPLEMENT_TEST(z    , 0x4);
IMPLEMENT_TEST(xz   , 0x5);
IMPLEMENT_TEST(yz   , 0x6);
IMPLEMENT_TEST(xyz  , 0x7);
IMPLEMENT_TEST(w    , 0x8);
IMPLEMENT_TEST(xw   , 0x9);
IMPLEMENT_TEST(yw   , 0xa);
IMPLEMENT_TEST(xyw  , 0xb);
IMPLEMENT_TEST(zw   , 0xc);
IMPLEMENT_TEST(xzw  , 0xd);
IMPLEMENT_TEST(yzw  , 0xe);
IMPLEMENT_TEST(xyzw , 0xf);

#undef IMPLEMENT_TEST

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xyAB(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 1, 4, 5);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_ABxy(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 4, 5, 0, 1);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CDzw(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 5, 7, 2, 3);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zwCD(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 3, 5, 7);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xAyB(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 1, 5);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_yBxA(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 0, 4);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zCwD(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 6, 3, 7);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CzDw(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 6, 2, 7, 3);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xAzC(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 2, 6);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_yBwD(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 3, 7);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float float4_x(float4_t _a)
	{
		return _a.fxyzw[0];
	}

	BX_FLOAT4_FORCE_INLINE float float4_y(float4_t _a)
	{
		return _a.fxyzw[1];
	}

	BX_FLOAT4_FORCE_INLINE float float4_z(float4_t _a)
	{
		return _a.fxyzw[2];
	}

	BX_FLOAT4_FORCE_INLINE float float4_w(float4_t _a)
	{
		return _a.fxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		float4_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	BX_FLOAT4_FORCE_INLINE void float4_st(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE void float4_stx(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	BX_FLOAT4_FORCE_INLINE void float4_stream(void* _ptr, float4_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(float _x, float _y, float _z, float _w)
	{
		float4_t result;
		result.vf = { _x, _y, _z, _w };
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		float4_t result;
		result.vu = { _x, _y, _z, _w };
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		float4_t result;
		result.vu = { val, val, val, val };
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(float _a)
	{
		return float4_ld(_a, _a, _a, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isplat(uint32_t _a)
	{
		return float4_ild(_a, _a, _a, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_zero()
	{
		return float4_ild(0, 0, 0, 0);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_itof(float4_t _a)
	{
		float4_t result;
		result.vf = __builtin_convertvector(_a.vi, float __attribute__((vector_size(16))) );
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ftoi(float4_t _a)
	{
		float4_t result;
		result.vi = __builtin_convertvector(_a.vf, int32_t __attribute__((vector_size(16))) );
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_round(float4_t _a)
	{
		const float4_t tmp    = float4_ftoi(_a);
		const float4_t result = float4_itof(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_add(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = _a.vf + _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sub(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = _a.vf - _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = _a.vf * _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_div(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vf = _a.vf / _b.vf;
		return result;
	}

#if 0
	BX_FLOAT4_FORCE_INLINE float4_t float4_rcp_est(float4_t _a)
	{
		float4_t result;
		const float4_t one = float4_splat(1.0f);
		result.vf = one / _a.vf;
		return result;
	}
#endif // 0

	BX_FLOAT4_FORCE_INLINE float4_t float4_sqrt(float4_t _a)
	{
		float4_t result;
		result.vf[0] = sqrtf(_a.vf[0]);
		result.vf[1] = sqrtf(_a.vf[1]);
		result.vf[2] = sqrtf(_a.vf[2]);
		result.vf[3] = sqrtf(_a.vf[3]);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rsqrt_est(float4_t _a)
	{
		float4_t result;
		result.vf[0] = 1.0f / sqrtf(_a.vf[0]);
		result.vf[1] = 1.0f / sqrtf(_a.vf[1]);
		result.vf[2] = 1.0f / sqrtf(_a.vf[2]);
		result.vf[3] = 1.0f / sqrtf(_a.vf[3]);
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpeq(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vf == _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmplt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vf < _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmple(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vf <= _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpgt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vf > _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpge(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vf >= _b.vf;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_and(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vu = _a.vu & _b.vu;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_andc(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vu = _a.vu & ~_b.vu;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_or(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vu = _a.vu | _b.vu;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_xor(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vu = _a.vu ^ _b.vu;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sll(float4_t _a, int _count)
	{
		float4_t result;
		const float4_t count = float4_isplat(_count);
		result.vu = _a.vu << count.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_srl(float4_t _a, int _count)
	{
		float4_t result;
		const float4_t count = float4_isplat(_count);
		result.vu = _a.vu >> count.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sra(float4_t _a, int _count)
	{
		float4_t result;
		const float4_t count = float4_isplat(_count);
		result.vi = _a.vi >> count.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpeq(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vi == _b.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmplt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vi < _b.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpgt(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vi > _b.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_iadd(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vi + _b.vi;
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isub(float4_t _a, float4_t _b)
	{
		float4_t result;
		result.vi = _a.vi - _b.vi;
		return result;
	}

} // namespace bx

#define float4_rcp           float4_rcp_ni
#define float4_orx           float4_orx_ni
#define float4_orc           float4_orc_ni
#define float4_neg           float4_neg_ni
#define float4_madd          float4_madd_ni
#define float4_nmsub         float4_nmsub_ni
#define float4_div_nr        float4_div_nr_ni
#define float4_selb          float4_selb_ni
#define float4_sels          float4_sels_ni
#define float4_not           float4_not_ni
#define float4_abs           float4_abs_ni
#define float4_clamp         float4_clamp_ni
#define float4_lerp          float4_lerp_ni
#define float4_rcp_est       float4_rcp_ni
#define float4_rsqrt         float4_rsqrt_ni
#define float4_rsqrt_nr      float4_rsqrt_nr_ni
#define float4_rsqrt_carmack float4_rsqrt_carmack_ni
#define float4_sqrt_nr       float4_sqrt_nr_ni
#define float4_log2          float4_log2_ni
#define float4_exp2          float4_exp2_ni
#define float4_pow           float4_pow_ni
#define float4_cross3        float4_cross3_ni
#define float4_normalize3    float4_normalize3_ni
#define float4_dot3          float4_dot3_ni
#define float4_dot           float4_dot_ni
#define float4_ceil          float4_ceil_ni
#define float4_floor         float4_floor_ni
#define float4_min           float4_min_ni
#define float4_max           float4_max_ni
#define float4_imin          float4_imin_ni
#define float4_imax          float4_imax_ni
#include "float4_ni.h"

#endif // BX_FLOAT4_LANGEXT_H_HEADER_GUARD
