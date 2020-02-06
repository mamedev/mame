/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{

#if BX_COMPILER_CLANG
#	define SHUFFLE_A(_a,  _i0, _i1, _i2, _i3)     __builtin_shufflevector(_a, _a, _i0, _i1, _i2, _i3 )
#	define SHUFFLE_AB(_a, _b, _i0, _i1, _i2, _i3) __builtin_shufflevector(_a, _b, _i0, _i1, _i2, _i3 )
#else
#	define SHUFFLE_A(_a,  _i0, _i1, _i2, _i3)     __builtin_shuffle(_a, (uint32x4_t){ _i0, _i1, _i2, _i3 })
#	define SHUFFLE_AB(_a, _b, _i0, _i1, _i2, _i3) __builtin_shuffle(_a, _b, (uint32x4_t){ _i0, _i1, _i2, _i3 })
#endif

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                  \
			template<>                                                                                \
			BX_SIMD_FORCE_INLINE simd128_neon_t simd_swiz_##_x##_y##_z##_w(simd128_neon_t _a)         \
			{                                                                                         \
				return SHUFFLE_A(_a, ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w ); \
			}

#include "simd128_swizzle.inl"

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _swizzle)                                \
			template<>                                                            \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_neon_t _test) \
			{                                                                     \
				const simd128_neon_t tmp0 = simd_swiz_##_swizzle(_test);          \
				return simd_test_any_ni(tmp0);                                    \
			}                                                                     \
			                                                                      \
			template<>                                                            \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_neon_t _test) \
			{                                                                     \
				const simd128_neon_t tmp0 = simd_swiz_##_swizzle(_test);          \
				return simd_test_all_ni(tmp0);                                    \
			}

BX_SIMD128_IMPLEMENT_TEST(x,   xxxx);
BX_SIMD128_IMPLEMENT_TEST(y,   yyyy);
BX_SIMD128_IMPLEMENT_TEST(xy,  xyyy);
BX_SIMD128_IMPLEMENT_TEST(z,   zzzz);
BX_SIMD128_IMPLEMENT_TEST(xz,  xzzz);
BX_SIMD128_IMPLEMENT_TEST(yz,  yzzz);
BX_SIMD128_IMPLEMENT_TEST(xyz, xyzz);
BX_SIMD128_IMPLEMENT_TEST(w,   wwww);
BX_SIMD128_IMPLEMENT_TEST(xw,  xwww);
BX_SIMD128_IMPLEMENT_TEST(yw,  ywww);
BX_SIMD128_IMPLEMENT_TEST(xyw, xyww);
BX_SIMD128_IMPLEMENT_TEST(zw,  zwww);
BX_SIMD128_IMPLEMENT_TEST(xzw, xzww);
BX_SIMD128_IMPLEMENT_TEST(yzw, yzww);
#undef BX_SIMD128_IMPLEMENT_TEST

	template<>
	BX_SIMD_FORCE_INLINE bool simd_test_any_xyzw(simd128_neon_t _test)
	{
		return simd_test_any_ni(_test);
	}

	template<>
	BX_SIMD_FORCE_INLINE bool simd_test_all_xyzw(simd128_neon_t _test)
	{
		return simd_test_all_ni(_test);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_xyAB(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 0, 1, 4, 5 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_ABxy(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 4, 5, 0, 1 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_CDzw(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 6, 7, 2, 3 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_zwCD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 2, 3, 6, 7 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_xAyB(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 0, 4, 1, 5 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_AxBy(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 4, 0, 5, 1 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_zCwD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 2, 6, 3, 7 );
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_shuf_CzDw(simd128_neon_t _a, simd128_neon_t _b)
	{
		return SHUFFLE_AB(_a, _b, 6, 2, 7, 3 );
	}
#undef SHUFFLE_A
#undef SHUFFLE_AB

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 1);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 2);
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_neon_t _a)
	{
		return vgetq_lane_f32(_a, 3);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ld(const void* _ptr)
	{
		return vld1q_f32( (const float32_t*)_ptr);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_neon_t _a)
	{
		vst1q_f32( (float32_t*)_ptr, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_neon_t _a)
	{
		vst1q_lane_f32( (float32_t*)_ptr, _a, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_neon_t _a)
	{
		vst1q_f32( (float32_t*)_ptr, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ld(float _x, float _y, float _z, float _w)
	{
		const float32_t val[4] = {_x, _y, _z, _w};
		return simd_ld<simd128_neon_t>(val);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		const uint32_t   val[4]    = {_x, _y, _z, _w};
		const uint32x4_t tmp       = vld1q_u32(val);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_splat(const void* _ptr)
	{
		const simd128_neon_t tmp0   = vld1q_f32( (const float32_t*)_ptr);
		const float32x2_t   tmp1   = vget_low_f32(tmp0);
		const simd128_neon_t result = vdupq_lane_f32(tmp1, 0);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_splat(float _a)
	{
		return vdupq_n_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_isplat(uint32_t _a)
	{
		const int32x4_t tmp    = vdupq_n_s32(_a);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_zero()
	{
		return simd_isplat<simd128_neon_t>(0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_itof(simd128_neon_t _a)
	{
		const int32x4_t itof   = vreinterpretq_s32_f32(_a);
		const simd128_neon_t  result = vcvtq_f32_s32(itof);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ftoi(simd128_neon_t _a)
	{
		const int32x4_t ftoi  = vcvtq_s32_f32(_a);
		const simd128_neon_t result = vreinterpretq_f32_s32(ftoi);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_add(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vaddq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sub(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vsubq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_mul(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vmulq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rcp_est(simd128_neon_t _a)
	{
		return vrecpeq_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_est(simd128_neon_t _a)
	{
		return vrsqrteq_f32(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpeq(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp    = vceqq_f32(_a, _b);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpneq(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_cmpneq_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmplt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp        = vcltq_f32(_a, _b);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmple(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp        = vcleq_f32(_a, _b);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpgt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp        = vcgtq_f32(_a, _b);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cmpge(simd128_neon_t _a, simd128_neon_t _b)
	{
		const uint32x4_t tmp        = vcgeq_f32(_a, _b);
		const simd128_neon_t result = vreinterpretq_f32_u32(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_min(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vminq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_max(simd128_neon_t _a, simd128_neon_t _b)
	{
		return vmaxq_f32(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_and(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vandq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_andc(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vbicq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_or(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vorrq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_xor(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = veorq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sll(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
			const uint32x4_t tmp1   = vshlq_n_u32(tmp0, _count);
			const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

			return result;
		}
#endif
		const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
		const int32x4_t  shift  = vdupq_n_s32(_count);
		const uint32x4_t tmp1   = vshlq_u32(tmp0, shift);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_srl(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
			const uint32x4_t tmp1   = vshrq_n_u32(tmp0, _count);
			const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

			return result;
		}
#endif
		const uint32x4_t tmp0   = vreinterpretq_u32_f32(_a);
		const int32x4_t  shift  = vdupq_n_s32(-_count);
		const uint32x4_t tmp1   = vshlq_u32(tmp0, shift);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sra(simd128_neon_t _a, int _count)
	{
#if !BX_COMPILER_CLANG
		if (__builtin_constant_p(_count) )
		{
			const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
			const int32x4_t tmp1   = vshrq_n_s32(tmp0, _count);
			const simd128_neon_t  result = vreinterpretq_f32_s32(tmp1);

			return result;
		}
#endif
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t shift  = vdupq_n_s32(-_count);
		const int32x4_t tmp1   = vshlq_s32(tmp0, shift);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp1);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmpeq(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vceqq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmplt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vcltq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_icmpgt(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t  tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t  tmp1   = vreinterpretq_s32_f32(_b);
		const uint32x4_t tmp2   = vcgtq_s32(tmp0, tmp1);
		const simd128_neon_t   result = vreinterpretq_f32_u32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_imin(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vminq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_imax(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vmaxq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_iadd(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vaddq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_isub(simd128_neon_t _a, simd128_neon_t _b)
	{
		const int32x4_t tmp0   = vreinterpretq_s32_f32(_a);
		const int32x4_t tmp1   = vreinterpretq_s32_f32(_b);
		const int32x4_t tmp2   = vsubq_s32(tmp0, tmp1);
		const simd128_neon_t  result = vreinterpretq_f32_s32(tmp2);

		return result;
	}

	template<>
	BX_SIMD_INLINE simd128_neon_t simd_shuf_xAzC(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_shuf_xAzC_ni(_a, _b);
	}

	template<>
	BX_SIMD_INLINE simd128_neon_t simd_shuf_yBwD(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_shuf_yBwD_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rcp(simd128_neon_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_orx(simd128_neon_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_orc(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_neg(simd128_neon_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_madd(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_nmsub(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_div_nr(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_div(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_selb(simd128_neon_t _mask, simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sels(simd128_neon_t _test, simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_not(simd128_neon_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_abs(simd128_neon_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_clamp(simd128_neon_t _a, simd128_neon_t _min, simd128_neon_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_lerp(simd128_neon_t _a, simd128_neon_t _b, simd128_neon_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt(simd128_neon_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_nr(simd128_neon_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_rsqrt_carmack(simd128_neon_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sqrt_nr(simd128_neon_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_sqrt(simd128_neon_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_log2(simd128_neon_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_exp2(simd128_neon_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_pow(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_cross3(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_normalize3(simd128_neon_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_dot3(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_dot(simd128_neon_t _a, simd128_neon_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_ceil(simd128_neon_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_neon_t simd_floor(simd128_neon_t _a)
	{
		return simd_floor_ni(_a);
	}

	typedef simd128_neon_t simd128_t;

} // namespace bx
