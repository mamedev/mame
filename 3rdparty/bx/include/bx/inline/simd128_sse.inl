/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                   \
			template<>                                                                                 \
			BX_SIMD_FORCE_INLINE simd128_sse_t simd_swiz_##_x##_y##_z##_w(simd128_sse_t _a)            \
			{                                                                                          \
				return _mm_shuffle_ps( _a, _a, _MM_SHUFFLE(ELEM##_w, ELEM##_z, ELEM##_y, ELEM##_x ) ); \
			}

#include "simd128_swizzle.inl"

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                  \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_sse_t _test) \
			{                                                                    \
				return 0x0 != (_mm_movemask_ps(_test)&(_mask) );                 \
			}                                                                    \
			                                                                     \
			template<>                                                           \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_sse_t _test) \
			{                                                                    \
				return (_mask) == (_mm_movemask_ps(_test)&(_mask) );             \
			}

BX_SIMD128_IMPLEMENT_TEST(x    , 0x1)
BX_SIMD128_IMPLEMENT_TEST(y    , 0x2)
BX_SIMD128_IMPLEMENT_TEST(xy   , 0x3)
BX_SIMD128_IMPLEMENT_TEST(z    , 0x4)
BX_SIMD128_IMPLEMENT_TEST(xz   , 0x5)
BX_SIMD128_IMPLEMENT_TEST(yz   , 0x6)
BX_SIMD128_IMPLEMENT_TEST(xyz  , 0x7)
BX_SIMD128_IMPLEMENT_TEST(w    , 0x8)
BX_SIMD128_IMPLEMENT_TEST(xw   , 0x9)
BX_SIMD128_IMPLEMENT_TEST(yw   , 0xa)
BX_SIMD128_IMPLEMENT_TEST(xyw  , 0xb)
BX_SIMD128_IMPLEMENT_TEST(zw   , 0xc)
BX_SIMD128_IMPLEMENT_TEST(xzw  , 0xd)
BX_SIMD128_IMPLEMENT_TEST(yzw  , 0xe)
BX_SIMD128_IMPLEMENT_TEST(xyzw , 0xf)

#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_xyAB(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movelh_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_ABxy(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movelh_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_CDzw(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movehl_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_zwCD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_movehl_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_xAyB(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpacklo_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_AxBy(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpacklo_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_zCwD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpackhi_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_shuf_CzDw(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_unpackhi_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_sse_t _a)
	{
		return _mm_cvtss_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_sse_t _a)
	{
		const simd128_sse_t yyyy = simd_swiz_yyyy(_a);
		const float result  = _mm_cvtss_f32(yyyy);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_sse_t _a)
	{
		const simd128_sse_t zzzz = simd_swiz_zzzz(_a);
		const float result  = _mm_cvtss_f32(zzzz);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_sse_t _a)
	{
		const simd128_sse_t wwww = simd_swiz_wwww(_a);
		const float result  = _mm_cvtss_f32(wwww);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ld(const void* _ptr)
	{
		return _mm_load_ps(reinterpret_cast<const float*>(_ptr) );
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_sse_t _a)
	{
		_mm_store_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_sse_t _a)
	{
		_mm_store_ss(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_sse_t _a)
	{
		_mm_stream_ps(reinterpret_cast<float*>(_ptr), _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ld(float _x, float _y, float _z, float _w)
	{
		return _mm_set_ps(_w, _z, _y, _x);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const __m128i set     = _mm_set_epi32(_w, _z, _y, _x);
		const simd128_sse_t result = _mm_castsi128_ps(set);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_splat(const void* _ptr)
	{
		const simd128_sse_t x___   = _mm_load_ss(reinterpret_cast<const float*>(_ptr) );
		const simd128_sse_t result = simd_swiz_xxxx(x___);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_splat(float _a)
	{
		return _mm_set1_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_isplat(uint32_t _a)
	{
		const __m128i splat   = _mm_set1_epi32(_a);
		const simd128_sse_t result = _mm_castsi128_ps(splat);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_zero()
	{
		return _mm_setzero_ps();
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_itof(simd128_sse_t _a)
	{
		const __m128i  itof   = _mm_castps_si128(_a);
		const simd128_sse_t result = _mm_cvtepi32_ps(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_ftoi(simd128_sse_t _a)
	{
		const __m128i ftoi    = _mm_cvtps_epi32(_a);
		const simd128_sse_t result = _mm_castsi128_ps(ftoi);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_round(simd128_sse_t _a)
	{
#if defined(__SSE4_1__)
		return _mm_round_ps(_a, _MM_FROUND_NINT);
#else
		const __m128i round   = _mm_cvtps_epi32(_a);
		const simd128_sse_t result = _mm_cvtepi32_ps(round);

		return result;
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_add(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_add_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sub(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_sub_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_mul(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_mul_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_div(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_div_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_rcp_est(simd128_sse_t _a)
	{
		return _mm_rcp_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sqrt(simd128_sse_t _a)
	{
		return _mm_sqrt_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_rsqrt_est(simd128_sse_t _a)
	{
		return _mm_rsqrt_ps(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_dot3(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		return _mm_dp_ps(_a, _b, 0x77);
#else
		return simd_dot3_ni(_a, _b);
#endif // defined(__SSE4__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_dot(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		return _mm_dp_ps(_a, _b, 0xFF);
#else
		return simd_dot_ni(_a, _b);
#endif // defined(__SSE4__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpeq(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpeq_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpneq(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpneq_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmplt(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmplt_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmple(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmple_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpgt(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpgt_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_cmpge(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_cmpge_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_min(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_min_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_max(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_max_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_and(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_and_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_andc(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_andnot_ps(_b, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_or(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_or_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_xor(simd128_sse_t _a, simd128_sse_t _b)
	{
		return _mm_xor_ps(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sll(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_slli_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_srl(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srli_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_sra(simd128_sse_t _a, int _count)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i shift   = _mm_srai_epi32(a, _count);
		const simd128_sse_t result = _mm_castsi128_ps(shift);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmpeq(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpeq_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmplt(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmplt_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_icmpgt(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_cmpgt_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_imin(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_min_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
#else
		return simd_imin_ni(_a, _b);
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_imax(simd128_sse_t _a, simd128_sse_t _b)
	{
#if defined(__SSE4_1__)
		const __m128i tmp0    = _mm_castps_si128(_a);
		const __m128i tmp1    = _mm_castps_si128(_b);
		const __m128i tmp2    = _mm_max_epi32(tmp0, tmp1);
		const simd128_sse_t result = _mm_castsi128_ps(tmp2);

		return result;
#else
		return simd_imax_ni(_a, _b);
#endif // defined(__SSE4_1__)
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_iadd(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i add     = _mm_add_epi32(a, b);
		const simd128_sse_t result = _mm_castsi128_ps(add);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_sse_t simd_isub(simd128_sse_t _a, simd128_sse_t _b)
	{
		const __m128i a       = _mm_castps_si128(_a);
		const __m128i b       = _mm_castps_si128(_b);
		const __m128i sub     = _mm_sub_epi32(a, b);
		const simd128_sse_t result = _mm_castsi128_ps(sub);

		return result;
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_shuf_xAzC(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_shuf_yBwD(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rcp(simd128_sse_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_orx(simd128_sse_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_orc(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_neg(simd128_sse_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_madd(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_nmsub(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_div_nr(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_selb(simd128_sse_t _mask, simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_sels(simd128_sse_t _test, simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_not(simd128_sse_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_abs(simd128_sse_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_clamp(simd128_sse_t _a, simd128_sse_t _min, simd128_sse_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_lerp(simd128_sse_t _a, simd128_sse_t _b, simd128_sse_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt(simd128_sse_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt_nr(simd128_sse_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_rsqrt_carmack(simd128_sse_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_sqrt_nr(simd128_sse_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_log2(simd128_sse_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_exp2(simd128_sse_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_pow(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_cross3(simd128_sse_t _a, simd128_sse_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_normalize3(simd128_sse_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_ceil(simd128_sse_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_INLINE simd128_sse_t simd_floor(simd128_sse_t _a)
	{
		return simd_floor_ni(_a);
	}

	typedef simd128_sse_t simd128_t;

} // namespace bx
