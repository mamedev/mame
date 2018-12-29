/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4X4_H_HEADER_GUARD
#	error "Must be included from bx/float4x4.h!"
#endif // BX_FLOAT4X4_H_HEADER_GUARD

namespace bx
{
	BX_SIMD_FORCE_INLINE simd128_t simd_mul_xyz1(simd128_t _a, const float4x4_t* _b)
	{
		const simd128_t xxxx   = simd_swiz_xxxx(_a);
		const simd128_t yyyy   = simd_swiz_yyyy(_a);
		const simd128_t zzzz   = simd_swiz_zzzz(_a);
		const simd128_t col0   = simd_mul(_b->col[0], xxxx);
		const simd128_t col1   = simd_mul(_b->col[1], yyyy);
		const simd128_t col2   = simd_madd(_b->col[2], zzzz, col0);
		const simd128_t col3   = simd_add(_b->col[3], col1);
		const simd128_t result = simd_add(col2, col3);

		return result;
	}

	BX_SIMD_FORCE_INLINE simd128_t simd_mul(simd128_t _a, const float4x4_t* _b)
	{
		const simd128_t xxxx   = simd_swiz_xxxx(_a);
		const simd128_t yyyy   = simd_swiz_yyyy(_a);
		const simd128_t zzzz   = simd_swiz_zzzz(_a);
		const simd128_t wwww   = simd_swiz_wwww(_a);
		const simd128_t col0   = simd_mul(_b->col[0], xxxx);
		const simd128_t col1   = simd_mul(_b->col[1], yyyy);
		const simd128_t col2   = simd_madd(_b->col[2], zzzz, col0);
		const simd128_t col3   = simd_madd(_b->col[3], wwww, col1);
		const simd128_t result = simd_add(col2, col3);

		return result;
	}

	BX_SIMD_INLINE void float4x4_mul(float4x4_t* _result, const float4x4_t* _a, const float4x4_t* _b)
	{
		_result->col[0] = simd_mul(_a->col[0], _b);
		_result->col[1] = simd_mul(_a->col[1], _b);
		_result->col[2] = simd_mul(_a->col[2], _b);
		_result->col[3] = simd_mul(_a->col[3], _b);
	}

	BX_SIMD_FORCE_INLINE void float4x4_transpose(float4x4_t* _result, const float4x4_t* _mtx)
	{
		const simd128_t aibj = simd_shuf_xAyB(_mtx->col[0], _mtx->col[2]); // aibj
		const simd128_t emfn = simd_shuf_xAyB(_mtx->col[1], _mtx->col[3]); // emfn
		const simd128_t ckdl = simd_shuf_zCwD(_mtx->col[0], _mtx->col[2]); // ckdl
		const simd128_t gohp = simd_shuf_zCwD(_mtx->col[1], _mtx->col[3]); // gohp
		_result->col[0] = simd_shuf_xAyB(aibj, emfn); // aeim
		_result->col[1] = simd_shuf_zCwD(aibj, emfn); // bfjn
		_result->col[2] = simd_shuf_xAyB(ckdl, gohp); // cgko
		_result->col[3] = simd_shuf_zCwD(ckdl, gohp); // dhlp
	}

	BX_SIMD_INLINE void float4x4_inverse(float4x4_t* _result, const float4x4_t* _a)
	{
		const simd128_t tmp0 = simd_shuf_xAzC(_a->col[0], _a->col[1]);
		const simd128_t tmp1 = simd_shuf_xAzC(_a->col[2], _a->col[3]);
		const simd128_t tmp2 = simd_shuf_yBwD(_a->col[0], _a->col[1]);
		const simd128_t tmp3 = simd_shuf_yBwD(_a->col[2], _a->col[3]);
		const simd128_t t0   = simd_shuf_xyAB(tmp0, tmp1);
		const simd128_t t1   = simd_shuf_xyAB(tmp3, tmp2);
		const simd128_t t2   = simd_shuf_zwCD(tmp0, tmp1);
		const simd128_t t3   = simd_shuf_zwCD(tmp3, tmp2);

		const simd128_t t23 = simd_mul(t2, t3);
		const simd128_t t23_yxwz = simd_swiz_yxwz(t23);
		const simd128_t t23_wzyx = simd_swiz_wzyx(t23);

		simd128_t cof0, cof1, cof2, cof3;

		const simd128_t zero = simd_zero();
		cof0 = simd_nmsub(t1, t23_yxwz, zero);
		cof0 = simd_madd(t1, t23_wzyx, cof0);

		cof1 = simd_nmsub(t0, t23_yxwz, zero);
		cof1 = simd_madd(t0, t23_wzyx, cof1);
		cof1 = simd_swiz_zwxy(cof1);

		const simd128_t t12 = simd_mul(t1, t2);
		const simd128_t t12_yxwz = simd_swiz_yxwz(t12);
		const simd128_t t12_wzyx = simd_swiz_wzyx(t12);

		cof0 = simd_madd(t3, t12_yxwz, cof0);
		cof0 = simd_nmsub(t3, t12_wzyx, cof0);

		cof3 = simd_mul(t0, t12_yxwz);
		cof3 = simd_nmsub(t0, t12_wzyx, cof3);
		cof3 = simd_swiz_zwxy(cof3);

		const simd128_t t1_zwxy = simd_swiz_zwxy(t1);
		const simd128_t t2_zwxy = simd_swiz_zwxy(t2);

		const simd128_t t13 = simd_mul(t1_zwxy, t3);
		const simd128_t t13_yxwz = simd_swiz_yxwz(t13);
		const simd128_t t13_wzyx = simd_swiz_wzyx(t13);

		cof0 = simd_madd(t2_zwxy, t13_yxwz, cof0);
		cof0 = simd_nmsub(t2_zwxy, t13_wzyx, cof0);

		cof2 = simd_mul(t0, t13_yxwz);
		cof2 = simd_nmsub(t0, t13_wzyx, cof2);
		cof2 = simd_swiz_zwxy(cof2);

		const simd128_t t01 = simd_mul(t0, t1);
		const simd128_t t01_yxwz = simd_swiz_yxwz(t01);
		const simd128_t t01_wzyx = simd_swiz_wzyx(t01);

		cof2 = simd_nmsub(t3, t01_yxwz, cof2);
		cof2 = simd_madd(t3, t01_wzyx, cof2);

		cof3 = simd_madd(t2_zwxy, t01_yxwz, cof3);
		cof3 = simd_nmsub(t2_zwxy, t01_wzyx, cof3);

		const simd128_t t03 = simd_mul(t0, t3);
		const simd128_t t03_yxwz = simd_swiz_yxwz(t03);
		const simd128_t t03_wzyx = simd_swiz_wzyx(t03);

		cof1 = simd_nmsub(t2_zwxy, t03_yxwz, cof1);
		cof1 = simd_madd(t2_zwxy, t03_wzyx, cof1);

		cof2 = simd_madd(t1, t03_yxwz, cof2);
		cof2 = simd_nmsub(t1, t03_wzyx, cof2);

		const simd128_t t02 = simd_mul(t0, t2_zwxy);
		const simd128_t t02_yxwz = simd_swiz_yxwz(t02);
		const simd128_t t02_wzyx = simd_swiz_wzyx(t02);

		cof1 = simd_madd(t3, t02_yxwz, cof1);
		cof1 = simd_nmsub(t3, t02_wzyx, cof1);

		cof3 = simd_nmsub(t1, t02_yxwz, cof3);
		cof3 = simd_madd(t1, t02_wzyx, cof3);

		const simd128_t det    = simd_dot(t0, cof0);
		const simd128_t invdet = simd_rcp(det);

		_result->col[0] = simd_mul(cof0, invdet);
		_result->col[1] = simd_mul(cof1, invdet);
		_result->col[2] = simd_mul(cof2, invdet);
		_result->col[3] = simd_mul(cof3, invdet);
	}

} // namespace bx
