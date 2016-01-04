/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_NEON_H_HEADER_GUARD
#define BX_FLOAT4_NEON_H_HEADER_GUARD

namespace bx
{
	typedef __builtin_neon_sf  float4_t __attribute__( (__vector_size__(16) ) );

	typedef __builtin_neon_sf  _f32x2_t __attribute__( (__vector_size__( 8) ) );
	typedef __builtin_neon_si  _i32x4_t __attribute__( (__vector_size__(16) ) );
	typedef __builtin_neon_usi _u32x4_t __attribute__( (__vector_size__(16) ) );

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define IMPLEMENT_SWIZZLE(_x, _y, _z, _w) \
			BX_FLOAT4_FORCE_INLINE float4_t float4_swiz_##_x##_y##_z##_w(float4_t _a) \
			{ \
				return __builtin_shuffle(_a, (_u32x4_t){ ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w }); \
			}

#include "float4_swizzle.inl"

#undef IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define IMPLEMENT_TEST(_xyzw, _swizzle) \
			BX_FLOAT4_FORCE_INLINE bool float4_test_any_##_xyzw(float4_t _test); \
			BX_FLOAT4_FORCE_INLINE bool float4_test_all_##_xyzw(float4_t _test);

IMPLEMENT_TEST(x    , xxxx);
IMPLEMENT_TEST(y    , yyyy);
IMPLEMENT_TEST(xy   , xyyy);
IMPLEMENT_TEST(z    , zzzz);
IMPLEMENT_TEST(xz   , xzzz);
IMPLEMENT_TEST(yz   , yzzz);
IMPLEMENT_TEST(xyz  , xyzz);
IMPLEMENT_TEST(w    , wwww);
IMPLEMENT_TEST(xw   , xwww);
IMPLEMENT_TEST(yw   , ywww);
IMPLEMENT_TEST(xyw  , xyww);
IMPLEMENT_TEST(zw   , zwww);
IMPLEMENT_TEST(xzw  , xzww);
IMPLEMENT_TEST(yzw  , yzww);
IMPLEMENT_TEST(xyzw , xyzw);

#undef IMPLEMENT_TEST

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xyAB(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 0, 1, 4, 5 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_ABxy(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 4, 5, 0, 1 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CDzw(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 6, 7, 2, 3 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zwCD(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 2, 3, 6, 7 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_xAyB(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 0, 4, 1, 5 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_yBxA(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 1, 5, 0, 4 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_zCwD(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 2, 6, 3, 7 });
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_shuf_CzDw(float4_t _a, float4_t _b)
	{
		return __builtin_shuffle(_a, _b, (_u32x4_t){ 6, 2, 7, 3 });
	}

	BX_FLOAT4_FORCE_INLINE float float4_x(float4_t _a)
	{
		return __builtin_neon_vget_lanev4sf(_a, 0, 3);
	}

	BX_FLOAT4_FORCE_INLINE float float4_y(float4_t _a)
	{
		return __builtin_neon_vget_lanev4sf(_a, 1, 3);
	}

	BX_FLOAT4_FORCE_INLINE float float4_z(float4_t _a)
	{
		return __builtin_neon_vget_lanev4sf(_a, 2, 3);
	}

	BX_FLOAT4_FORCE_INLINE float float4_w(float4_t _a)
	{
		return __builtin_neon_vget_lanev4sf(_a, 3, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(const void* _ptr)
	{
		return __builtin_neon_vld1v4sf( (const __builtin_neon_sf*)_ptr);
	}

	BX_FLOAT4_FORCE_INLINE void float4_st(void* _ptr, float4_t _a)
	{
		__builtin_neon_vst1v4sf( (__builtin_neon_sf*)_ptr, _a);
	}

	BX_FLOAT4_FORCE_INLINE void float4_stx(void* _ptr, float4_t _a)
	{
		__builtin_neon_vst1_lanev4sf( (__builtin_neon_sf*)_ptr, _a, 0); 
	}

	BX_FLOAT4_FORCE_INLINE void float4_stream(void* _ptr, float4_t _a)
	{
		__builtin_neon_vst1v4sf( (__builtin_neon_sf*)_ptr, _a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ld(float _x, float _y, float _z, float _w)
	{
		const float4_t val[4] = {_x, _y, _z, _w};
		return float4_ld(val);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const uint32_t val[4] = {_x, _y, _z, _w};
		const _i32x4_t tmp    = __builtin_neon_vld1v4si( (const __builtin_neon_si*)val);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(const void* _ptr)
	{
		const float4_t tmp0   = __builtin_neon_vld1v4sf( (const __builtin_neon_sf *)_ptr);
		const _f32x2_t tmp1   = __builtin_neon_vget_lowv4sf(tmp0);
		const float4_t result = __builtin_neon_vdup_lanev4sf(tmp1, 0);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_splat(float _a)
	{
		return __builtin_neon_vdup_nv4sf(_a);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isplat(uint32_t _a)
	{
		const _i32x4_t tmp    = __builtin_neon_vdup_nv4si( (__builtin_neon_si)_a);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_zero()
	{
		return float4_isplat(0);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_itof(float4_t _a)
	{
		const _i32x4_t itof   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const float4_t result = __builtin_neon_vcvtv4si(itof, 1);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_ftoi(float4_t _a)
	{
		const _i32x4_t ftoi   = __builtin_neon_vcvtv4sf(_a, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(ftoi);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_add(float4_t _a, float4_t _b)
	{
		return __builtin_neon_vaddv4sf(_a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sub(float4_t _a, float4_t _b)
	{
		return __builtin_neon_vsubv4sf(_a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul(float4_t _a, float4_t _b)
	{
		return __builtin_neon_vmulv4sf(_a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rcp_est(float4_t _a)
	{
		return __builtin_neon_vrecpev4sf(_a, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_rsqrt_est(float4_t _a)
	{
		return __builtin_neon_vrsqrtev4sf(_a, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpeq(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp    = __builtin_neon_vceqv4sf(_a, _b, 3);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmplt(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp    = __builtin_neon_vcgtv4sf(_b, _a, 3);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmple(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp    = __builtin_neon_vcgev4sf(_b, _a, 3);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpgt(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp    = __builtin_neon_vcgtv4sf(_a, _b, 3);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_cmpge(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp    = __builtin_neon_vcgev4sf(_a, _b, 3);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_min(float4_t _a, float4_t _b)
	{
		return __builtin_neon_vminv4sf(_a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_max(float4_t _a, float4_t _b)
	{
		return __builtin_neon_vmaxv4sf(_a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_and(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vandv4si(tmp0, tmp1, 0);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_andc(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vbicv4si(tmp0, tmp1, 0);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_or(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vorrv4si(tmp0, tmp1, 0);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_xor(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_veorv4si(tmp0, tmp1, 0);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sll(float4_t _a, int _count)
	{
		if (__builtin_constant_p(_count) )
		{
			const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
			const _i32x4_t tmp1   = __builtin_neon_vshl_nv4si(tmp0, _count, 0);
			const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

			return result;
		}

		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t shift  = __builtin_neon_vdup_nv4si( (__builtin_neon_si)_count);
		const _i32x4_t tmp1   = __builtin_neon_vshlv4si(tmp0, shift, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_srl(float4_t _a, int _count)
	{
		if (__builtin_constant_p(_count) )
		{
			const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
			const _i32x4_t tmp1   = __builtin_neon_vshr_nv4si(tmp0, _count, 0);
			const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

			return result;
		}

		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t shift  = __builtin_neon_vdup_nv4si( (__builtin_neon_si)-_count);
		const _i32x4_t tmp1   = __builtin_neon_vshlv4si(tmp0, shift, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_sra(float4_t _a, int _count)
	{
		if (__builtin_constant_p(_count) )
		{
			const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
			const _i32x4_t tmp1   = __builtin_neon_vshr_nv4si(tmp0, _count, 1);
			const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

			return result;
		}

		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t shift  = __builtin_neon_vdup_nv4si( (__builtin_neon_si)-_count);
		const _i32x4_t tmp1   = __builtin_neon_vshlv4si(tmp0, shift, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp1);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_madd(float4_t _a, float4_t _b, float4_t _c)
	{
		return __builtin_neon_vmlav4sf(_c, _a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_nmsub(float4_t _a, float4_t _b, float4_t _c)
	{
		return __builtin_neon_vmlsv4sf(_c, _a, _b, 3);
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpeq(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vceqv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmplt(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vcgtv4si(tmp1, tmp0, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_icmpgt(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vcgtv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_imin(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vminv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_imax(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vmaxv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_iadd(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vaddv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_isub(float4_t _a, float4_t _b)
	{
		const _i32x4_t tmp0   = __builtin_neon_vreinterpretv4siv4sf(_a);
		const _i32x4_t tmp1   = __builtin_neon_vreinterpretv4siv4sf(_b);
		const _i32x4_t tmp2   = __builtin_neon_vsubv4si(tmp0, tmp1, 1);
		const float4_t result = __builtin_neon_vreinterpretv4sfv4si(tmp2);

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
#define float4_div           float4_div_nr_ni
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
#define float4_sqrt          float4_sqrt_nr_ni
#define float4_log2          float4_log2_ni
#define float4_exp2          float4_exp2_ni
#define float4_pow           float4_pow_ni
#define float4_cross3        float4_cross3_ni
#define float4_normalize3    float4_normalize3_ni
#define float4_dot3          float4_dot3_ni
#define float4_dot           float4_dot_ni
#define float4_ceil          float4_ceil_ni
#define float4_floor         float4_floor_ni

#include "float4_ni.h"

namespace bx
{
#define IMPLEMENT_TEST(_xyzw, _swizzle) \
			BX_FLOAT4_FORCE_INLINE bool float4_test_any_##_xyzw(float4_t _test) \
			{ \
				const float4_t tmp0 = float4_swiz_##_swizzle(_test); \
				return float4_test_any_ni(tmp0); \
			} \
			\
			BX_FLOAT4_FORCE_INLINE bool float4_test_all_##_xyzw(float4_t _test) \
			{ \
				const float4_t tmp0 = float4_swiz_##_swizzle(_test); \
				return float4_test_all_ni(tmp0); \
			}

IMPLEMENT_TEST(x    , xxxx);
IMPLEMENT_TEST(y    , yyyy);
IMPLEMENT_TEST(xy   , xyyy);
IMPLEMENT_TEST(z    , zzzz);
IMPLEMENT_TEST(xz   , xzzz);
IMPLEMENT_TEST(yz   , yzzz);
IMPLEMENT_TEST(xyz  , xyzz);
IMPLEMENT_TEST(w    , wwww);
IMPLEMENT_TEST(xw   , xwww);
IMPLEMENT_TEST(yw   , ywww);
IMPLEMENT_TEST(xyw  , xyww);
IMPLEMENT_TEST(zw   , zwww);
IMPLEMENT_TEST(xzw  , xzww);
IMPLEMENT_TEST(yzw  , yzww);

	BX_FLOAT4_FORCE_INLINE bool float4_test_any_xyzw(float4_t _test)
	{
		return float4_test_any_ni(_test);
	}

	BX_FLOAT4_FORCE_INLINE bool float4_test_all_xyzw(float4_t _test)
	{
		return float4_test_all_ni(_test);
	}

#undef IMPLEMENT_TEST
} // namespace bx

#endif // BX_FLOAT4_NEON_H_HEADER_GUARD
