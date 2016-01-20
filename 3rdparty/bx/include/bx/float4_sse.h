/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_SSE_H_HEADER_GUARD
#define BX_FLOAT4_SSE_H_HEADER_GUARD

#include <emmintrin.h> // __m128i
#if defined(__SSE4_1__)
#	include <smmintrin.h>
#endif // defined(__SSE4_1__)
#include <xmmintrin.h> // __m128

namespace bx
{
	typedef __m128 float4_t;

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			BX_FLOAT4_FORCE_INLINE float4_t float4_swiz_##_x##_y##_z##_w(float4_t _a) \
			{ \
				return _mm_shuffle_ps( _a, _a, _MM_SHUFFLE(ELEM##_w, ELEM##_z, ELEM##_y, ELEM##_x ) ); \
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
				return 0x0 != (_mm_movemask_ps(_test)&(_mask) ); \
			} \
			\
			BX_FLOAT4_FORCE_INLINE bool float4_test_all_##_xyzw(float4_t _test) \
			{ \
				return (_mask) == (_mm_movemask_ps(_test)&(_mask) ); \
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
		return _mm_movelh_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_ABxy(float4_t _a, float4_t _b)
	{
		return _mm_movelh_ps(_b, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CDzw(float4_t _a, float4_t _b)
	{
		return _mm_movehl_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zwCD(float4_t _a, float4_t _b)
	{
		return _mm_movehl_ps(_b, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xAyB(float4_t _a, float4_t _b)
	{
		return _mm_unpacklo_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_yBxA(float4_t _a, float4_t _b)
	{
		return _mm_unpacklo_ps(_b, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zCwD(float4_t _a, float4_t _b)
	{
		return _mm_unpackhi_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CzDw(float4_t _a, float4_t _b)
	{
		return _mm_unpackhi_ps(_b, _a);
	}

	BX_FLOAT4_FORCE_INLINE float float4_x(float4_t _a)
	{
		return _mm_cvtss_f32(_a);
	}

	BX_FLOAT4_FORCE_INLINE float float4_y(float4_t _a)
	{
		const float4_t yyyy = float4_swiz_yyyy(_a);
		const float result  = _mm_cvtss_f32(yyyy);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float float4_z(float4_t _a)
	{
		const float4_t zzzz = float4_swiz_zzzz(_a);
		const float result  = _mm_cvtss_f32(zzzz);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float float4_w(float4_t _a)
	{
		const float4_t wwww = float4_swiz_wwww(_a);
		const float result  = _mm_cvtss_f32(wwww);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(const void* _ptr)
	{
		return _mm_load_ps(reinterpret_cast<const float*>(_ptr) );
	}

	BX_FLOAT4_FORCE_INLINE void float4_st(void* _ptr, float4_t _a)
	{
		_mm_store_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	BX_FLOAT4_FORCE_INLINE void float4_stx(void* _ptr, float4_t _a)
	{
		_mm_store_ss(reinterpret_cast<float*>(_ptr), _a);
	}

	BX_FLOAT4_FORCE_INLINE void float4_stream(void* _ptr, float4_t _a)
	{
		_mm_stream_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(float _x, float _y, float _z, float _w)
	{
		return _mm_set_ps(_w, _z, _y, _x);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const __m128i set     = _mm_set_epi32(_w, _z, _y, _x);
		const float4_t result = _mm_castsi128_ps(set);
		
		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(const void* _ptr)
	{
		const float4_t x___   = _mm_load_ss(reinterpret_cast<const float*>(_ptr) );
		const float4_t result = float4_swiz_xxxx(x___);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(float _a)
	{
		return _mm_set1_ps(_a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isplat(uint32_t _a)
	{
		const __m128i splat   = _mm_set1_epi32(_a);
		const float4_t result = _mm_castsi128_ps(splat);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_zero()
	{
		return _mm_setzero_ps();
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_itof(float4_t _a)
	{
		const __m128i  itof   = _mm_castps_si128(_a);
		const float4_t result = _mm_cvtepi32_ps(itof);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ftoi(float4_t _a)
	{
		const __m128i ftoi    = _mm_cvtps_epi32(_a);
		const float4_t result = _mm_castsi128_ps(ftoi);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_round(float4_t _a)
	{
#if defined(__SSE4_1__)
		return _mm_round_ps(_a, _MM_FROUND_NINT);
#else
		const __m128i round   = _mm_cvtps_epi32(_a);
		const float4_t result = _mm_cvtepi32_ps(round);

		return result;
#endif // defined(__SSE4_1__)
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_add(float4_t _a, float4_t _b)
	{
		return _mm_add_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sub(float4_t _a, float4_t _b)
	{
		return _mm_sub_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul(float4_t _a, float4_t _b)
	{
		return _mm_mul_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_div(float4_t _a, float4_t _b)
	{
		return _mm_div_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rcp_est(float4_t _a)
	{
		return _mm_rcp_ps(_a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sqrt(float4_t _a)
	{
		return _mm_sqrt_ps(_a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rsqrt_est(float4_t _a)
	{
		return _mm_rsqrt_ps(_a);
	}

#if defined(__SSE4_1__)
	BX_FLOAT4_FORCE_INLINE float4_t float4_dot3(float4_t _a, float4_t _b)
	{
		return _mm_dp_ps(_a, _b, 0x77);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_dot(float4_t _a, float4_t _b)
	{
		return _mm_dp_ps(_a, _b, 0xFF);
	}
#endif // defined(__SSE4__)

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpeq(float4_t _a, float4_t _b)
	{
		return _mm_cmpeq_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmplt(float4_t _a, float4_t _b)
	{
		return _mm_cmplt_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmple(float4_t _a, float4_t _b)
	{
		return _mm_cmple_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpgt(float4_t _a, float4_t _b)
	{
		return _mm_cmpgt_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpge(float4_t _a, float4_t _b)
	{
		return _mm_cmpge_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_min(float4_t _a, float4_t _b)
	{
		return _mm_min_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_max(float4_t _a, float4_t _b)
	{
		return _mm_max_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_and(float4_t _a, float4_t _b)
	{
		return _mm_and_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_andc(float4_t _a, float4_t _b)
	{
		return _mm_andnot_ps(_b, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_or(float4_t _a, float4_t _b)
	{
		return _mm_or_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_xor(float4_t _a, float4_t _b)
	{
		return _mm_xor_ps(_a, _b);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sll(float4_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_slli_epi32(a, _count);
		const float4_t result = _mm_castsi128_ps(shift);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_srl(float4_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srli_epi32(a, _count);
		const float4_t result = _mm_castsi128_ps(shift);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sra(float4_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srai_epi32(a, _count);
		const float4_t result = _mm_castsi128_ps(shift);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpeq(float4_t _a, float4_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpeq_epi32(tmp0, tmp1);
		const float4_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmplt(float4_t _a, float4_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmplt_epi32(tmp0, tmp1);
		const float4_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpgt(float4_t _a, float4_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpgt_epi32(tmp0, tmp1);
		const float4_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

#if defined(__SSE4_1__)
	BX_FLOAT4_FORCE_INLINE float4_t float4_imin(float4_t _a, float4_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_min_epi32(tmp0, tmp1);
		const float4_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_imax(float4_t _a, float4_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_max_epi32(tmp0, tmp1);
		const float4_t result = _mm_castsi128_ps(tmp2);

		return result;
	}
#endif // defined(__SSE4_1__)

	BX_FLOAT4_FORCE_INLINE float4_t float4_iadd(float4_t _a, float4_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i add     = _mm_add_epi32(a, b);
		const float4_t result = _mm_castsi128_ps(add);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isub(float4_t _a, float4_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i sub     = _mm_sub_epi32(a, b);
		const float4_t result = _mm_castsi128_ps(sub);

		return result;
	}

} // namespace bx

#define float4_shuf_xAzC     float4_shuf_xAzC_ni
#define float4_shuf_yBwD     float4_shuf_yBwD_ni
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
#define float4_rsqrt         float4_rsqrt_ni
#define float4_rsqrt_nr      float4_rsqrt_nr_ni
#define float4_rsqrt_carmack float4_rsqrt_carmack_ni
#define float4_sqrt_nr       float4_sqrt_nr_ni
#define float4_log2          float4_log2_ni
#define float4_exp2          float4_exp2_ni
#define float4_pow           float4_pow_ni
#define float4_cross3        float4_cross3_ni
#define float4_normalize3    float4_normalize3_ni
#define float4_ceil          float4_ceil_ni
#define float4_floor         float4_floor_ni

#if !defined(__SSE4_1__)
#	define float4_dot3       float4_dot3_ni
#	define float4_dot        float4_dot_ni
#	define float4_imin       float4_imin_ni
#	define float4_imax       float4_imax_ni
#endif // defined(__SSE4_1__)

#include "float4_ni.h"

#endif // BX_FLOAT4_SSE_H_HEADER_GUARD
