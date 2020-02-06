/*
 * Copyright 2010-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SIMD_T_H_HEADER_GUARD
#	error "Must be included from bx/simd_t.h!"
#endif // BX_SIMD_T_H_HEADER_GUARD

namespace bx
{
	BX_CONST_FUNC float sqrtRef(float);

#define ELEMx 0
#define ELEMy 1
#define ELEMz 2
#define ELEMw 3
#define BX_SIMD128_IMPLEMENT_SWIZZLE(_x, _y, _z, _w)                                                       \
			template<>                                                                                     \
			BX_SIMD_FORCE_INLINE simd128_langext_t simd_swiz_##_x##_y##_z##_w(simd128_langext_t _a)        \
			{                                                                                              \
				simd128_langext_t result;                                                                  \
				result.vf = __builtin_shufflevector(_a.vf, _a.vf, ELEM##_x, ELEM##_y, ELEM##_z, ELEM##_w); \
				return result;                                                                             \
			}

#include "simd128_swizzle.inl"

#undef BX_SIMD128_IMPLEMENT_SWIZZLE
#undef ELEMw
#undef ELEMz
#undef ELEMy
#undef ELEMx

#define BX_SIMD128_IMPLEMENT_TEST(_xyzw, _mask)                                      \
			template<>                                                               \
			BX_SIMD_FORCE_INLINE bool simd_test_any_##_xyzw(simd128_langext_t _test) \
			{                                                                        \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                            \
				             | ( (_test.uxyzw[2]>>31)<<2)                            \
				             | ( (_test.uxyzw[1]>>31)<<1)                            \
				             | (  _test.uxyzw[0]>>31)                                \
				             ;                                                       \
				return 0 != (tmp&(_mask) );                                          \
			}                                                                        \
			                                                                         \
			template<>                                                               \
			BX_SIMD_FORCE_INLINE bool simd_test_all_##_xyzw(simd128_langext_t _test) \
			{                                                                        \
				uint32_t tmp = ( (_test.uxyzw[3]>>31)<<3)                            \
				             | ( (_test.uxyzw[2]>>31)<<2)                            \
				             | ( (_test.uxyzw[1]>>31)<<1)                            \
				             | (  _test.uxyzw[0]>>31)                                \
				             ;                                                       \
				return (_mask) == (tmp&(_mask) );                                    \
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
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xyAB(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 1, 4, 5);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_ABxy(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 4, 5, 0, 1);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_CDzw(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 6, 7, 2, 3);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_zwCD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 3, 6, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xAyB(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 1, 5);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_AxBy(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 0, 4);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_zCwD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 2, 6, 3, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_CzDw(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 6, 2, 7, 3);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_xAzC(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 0, 4, 2, 6);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_shuf_yBwD(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = __builtin_shufflevector(_a.vf, _b.vf, 1, 5, 3, 7);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_x(simd128_langext_t _a)
	{
		return _a.fxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_y(simd128_langext_t _a)
	{
		return _a.fxyzw[1];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_z(simd128_langext_t _a)
	{
		return _a.fxyzw[2];
	}

	template<>
	BX_SIMD_FORCE_INLINE float simd_w(simd128_langext_t _a)
	{
		return _a.fxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ld(const void* _ptr)
	{
		const uint32_t* input = reinterpret_cast<const uint32_t*>(_ptr);
		simd128_langext_t result;
		result.uxyzw[0] = input[0];
		result.uxyzw[1] = input[1];
		result.uxyzw[2] = input[2];
		result.uxyzw[3] = input[3];
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_st(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stx(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
	}

	template<>
	BX_SIMD_FORCE_INLINE void simd_stream(void* _ptr, simd128_langext_t _a)
	{
		uint32_t* result = reinterpret_cast<uint32_t*>(_ptr);
		result[0] = _a.uxyzw[0];
		result[1] = _a.uxyzw[1];
		result[2] = _a.uxyzw[2];
		result[3] = _a.uxyzw[3];
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ld(float _x, float _y, float _z, float _w)
	{
		simd128_langext_t result;
		result.vf = (float __attribute__((vector_size(16)))){ _x, _y, _z, _w };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ild(uint32_t _x, uint32_t _y, uint32_t _z, uint32_t _w)
	{
		simd128_langext_t result;
		result.vu = (uint32_t __attribute__((vector_size(16)))){ _x, _y, _z, _w };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_splat(const void* _ptr)
	{
		const uint32_t val = *reinterpret_cast<const uint32_t*>(_ptr);
		simd128_langext_t result;
		result.vu = (uint32_t __attribute__((vector_size(16)))){ val, val, val, val };
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_splat(float _a)
	{
		return simd_ld<simd128_langext_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_isplat(uint32_t _a)
	{
		return simd_ild<simd128_langext_t>(_a, _a, _a, _a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_zero()
	{
		return simd_ild<simd128_langext_t>(0, 0, 0, 0);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_itof(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf = __builtin_convertvector(_a.vi, float __attribute__((vector_size(16))) );
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ftoi(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vi = __builtin_convertvector(_a.vf, int32_t __attribute__((vector_size(16))) );
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_round(simd128_langext_t _a)
	{
		const simd128_langext_t tmp    = simd_ftoi(_a);
		const simd128_langext_t result = simd_itof(tmp);

		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_add(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf + _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sub(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf - _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_mul(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf * _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_div(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vf = _a.vf / _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sqrt(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf[0] = sqrtRef(_a.vf[0]);
		result.vf[1] = sqrtRef(_a.vf[1]);
		result.vf[2] = sqrtRef(_a.vf[2]);
		result.vf[3] = sqrtRef(_a.vf[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_est(simd128_langext_t _a)
	{
		simd128_langext_t result;
		result.vf[0] = 1.0f / sqrtRef(_a.vf[0]);
		result.vf[1] = 1.0f / sqrtRef(_a.vf[1]);
		result.vf[2] = 1.0f / sqrtRef(_a.vf[2]);
		result.vf[3] = 1.0f / sqrtRef(_a.vf[3]);
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpeq(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf == _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpneq(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf != _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmplt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf < _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmple(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf <= _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpgt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf > _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cmpge(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vf >= _b.vf;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_and(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu & _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_andc(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu & ~_b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_or(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu | _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_xor(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vu = _a.vu ^ _b.vu;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sll(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vu = _a.vu << count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_srl(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vu = _a.vu >> count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sra(simd128_langext_t _a, int _count)
	{
		simd128_langext_t result;
		const simd128_langext_t count = simd_isplat<simd128_langext_t>(_count);
		result.vi = _a.vi >> count.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmpeq(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi == _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmplt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi < _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_icmpgt(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi > _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_iadd(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi + _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_isub(simd128_langext_t _a, simd128_langext_t _b)
	{
		simd128_langext_t result;
		result.vi = _a.vi - _b.vi;
		return result;
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rcp(simd128_langext_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_orx(simd128_langext_t _a)
	{
		return simd_orx_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_orc(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_orc_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_neg(simd128_langext_t _a)
	{
		return simd_neg_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_madd(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _c)
	{
		return simd_madd_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_nmsub(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _c)
	{
		return simd_nmsub_ni(_a, _b, _c);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_div_nr(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_div_nr_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_selb(simd128_langext_t _mask, simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_selb_ni(_mask, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sels(simd128_langext_t _test, simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_sels_ni(_test, _a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_not(simd128_langext_t _a)
	{
		return simd_not_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_abs(simd128_langext_t _a)
	{
		return simd_abs_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_clamp(simd128_langext_t _a, simd128_langext_t _min, simd128_langext_t _max)
	{
		return simd_clamp_ni(_a, _min, _max);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_lerp(simd128_langext_t _a, simd128_langext_t _b, simd128_langext_t _s)
	{
		return simd_lerp_ni(_a, _b, _s);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rcp_est(simd128_langext_t _a)
	{
		return simd_rcp_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt(simd128_langext_t _a)
	{
		return simd_rsqrt_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_nr(simd128_langext_t _a)
	{
		return simd_rsqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_rsqrt_carmack(simd128_langext_t _a)
	{
		return simd_rsqrt_carmack_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_sqrt_nr(simd128_langext_t _a)
	{
		return simd_sqrt_nr_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_log2(simd128_langext_t _a)
	{
		return simd_log2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_exp2(simd128_langext_t _a)
	{
		return simd_exp2_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_pow(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_pow_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_cross3(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_cross3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_normalize3(simd128_langext_t _a)
	{
		return simd_normalize3_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_dot3(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_dot3_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_dot(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_dot_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_ceil(simd128_langext_t _a)
	{
		return simd_ceil_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_floor(simd128_langext_t _a)
	{
		return simd_floor_ni(_a);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_min(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_min_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_max(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_max_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_imin(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_imin_ni(_a, _b);
	}

	template<>
	BX_SIMD_FORCE_INLINE simd128_langext_t simd_imax(simd128_langext_t _a, simd128_langext_t _b)
	{
		return simd_imax_ni(_a, _b);
	}

	typedef simd128_langext_t simd128_t;

} // namespace bx
