/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4X4_H_HEADER_GUARD
#define BX_FLOAT4X4_H_HEADER_GUARD

#include "float4_t.h"

namespace bx
{
	BX_ALIGN_DECL_16(struct) float4x4_t
	{
		float4_t col[4];
	};

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul_xyz1(float4_t _a, const float4x4_t* _b)
	{
		const float4_t xxxx   = float4_swiz_xxxx(_a);
		const float4_t yyyy   = float4_swiz_yyyy(_a);
		const float4_t zzzz   = float4_swiz_zzzz(_a);
		const float4_t col0   = float4_mul(_b->col[0], xxxx);
		const float4_t col1   = float4_mul(_b->col[1], yyyy);
		const float4_t col2   = float4_madd(_b->col[2], zzzz, col0);
		const float4_t col3   = float4_add(_b->col[3], col1);
		const float4_t result = float4_add(col2, col3);

		return result;
	}

	BX_FLOAT4_FORCE_INLINE float4_t float4_mul(float4_t _a, const float4x4_t* _b)
	{
		const float4_t xxxx   = float4_swiz_xxxx(_a);
		const float4_t yyyy   = float4_swiz_yyyy(_a);
		const float4_t zzzz   = float4_swiz_zzzz(_a);
		const float4_t wwww   = float4_swiz_wwww(_a);
		const float4_t col0   = float4_mul(_b->col[0], xxxx);
		const float4_t col1   = float4_mul(_b->col[1], yyyy);
		const float4_t col2   = float4_madd(_b->col[2], zzzz, col0);
		const float4_t col3   = float4_madd(_b->col[3], wwww, col1);
		const float4_t result = float4_add(col2, col3);

		return result;
	}

	BX_FLOAT4_INLINE void float4x4_mul(float4x4_t* __restrict _result, const float4x4_t* __restrict _a, const float4x4_t* __restrict _b)
	{
		_result->col[0] = float4_mul(_a->col[0], _b);
		_result->col[1] = float4_mul(_a->col[1], _b);
		_result->col[2] = float4_mul(_a->col[2], _b);
		_result->col[3] = float4_mul(_a->col[3], _b);
	}

	BX_FLOAT4_FORCE_INLINE void float4x4_transpose(float4x4_t* __restrict _result, const float4x4_t* __restrict _mtx)
	{
		const float4_t aibj = float4_shuf_xAyB(_mtx->col[0], _mtx->col[2]); // aibj
		const float4_t emfn = float4_shuf_xAyB(_mtx->col[1], _mtx->col[3]); // emfn
		const float4_t ckdl = float4_shuf_zCwD(_mtx->col[0], _mtx->col[2]); // ckdl
		const float4_t gohp = float4_shuf_zCwD(_mtx->col[1], _mtx->col[3]); // gohp
		_result->col[0] = float4_shuf_xAyB(aibj, emfn); // aeim
		_result->col[1] = float4_shuf_zCwD(aibj, emfn); // bfjn
		_result->col[2] = float4_shuf_xAyB(ckdl, gohp); // cgko
		_result->col[3] = float4_shuf_zCwD(ckdl, gohp); // dhlp
	}

	BX_FLOAT4_INLINE void float4x4_inverse(float4x4_t* __restrict _result, const float4x4_t* __restrict _a)
	{
		const float4_t tmp0 = float4_shuf_xAzC(_a->col[0], _a->col[1]);
		const float4_t tmp1 = float4_shuf_xAzC(_a->col[2], _a->col[3]);
		const float4_t tmp2 = float4_shuf_yBwD(_a->col[0], _a->col[1]);
		const float4_t tmp3 = float4_shuf_yBwD(_a->col[2], _a->col[3]);
		const float4_t t0   = float4_shuf_xyAB(tmp0, tmp1);
		const float4_t t1   = float4_shuf_xyAB(tmp3, tmp2);
		const float4_t t2   = float4_shuf_zwCD(tmp0, tmp1);
		const float4_t t3   = float4_shuf_zwCD(tmp3, tmp2);

		const float4_t t23 = float4_mul(t2, t3);
		const float4_t t23_yxwz = float4_swiz_yxwz(t23);
		const float4_t t23_wzyx = float4_swiz_wzyx(t23);

		float4_t cof0, cof1, cof2, cof3;

		const float4_t zero = float4_zero();
		cof0 = float4_nmsub(t1, t23_yxwz, zero);
		cof0 = float4_madd(t1, t23_wzyx, cof0);

		cof1 = float4_nmsub(t0, t23_yxwz, zero);
		cof1 = float4_madd(t0, t23_wzyx, cof1);
		cof1 = float4_swiz_zwxy(cof1);
		
		const float4_t t12 = float4_mul(t1, t2);
		const float4_t t12_yxwz = float4_swiz_yxwz(t12);
		const float4_t t12_wzyx = float4_swiz_wzyx(t12);
		
		cof0 = float4_madd(t3, t12_yxwz, cof0);
		cof0 = float4_nmsub(t3, t12_wzyx, cof0);

		cof3 = float4_mul(t0, t12_yxwz);
		cof3 = float4_nmsub(t0, t12_wzyx, cof3);
		cof3 = float4_swiz_zwxy(cof3);

		const float4_t t1_zwxy = float4_swiz_zwxy(t1);
		const float4_t t2_zwxy = float4_swiz_zwxy(t2);

		const float4_t t13 = float4_mul(t1_zwxy, t3);
		const float4_t t13_yxwz = float4_swiz_yxwz(t13);
		const float4_t t13_wzyx = float4_swiz_wzyx(t13);

		cof0 = float4_madd(t2_zwxy, t13_yxwz, cof0);
		cof0 = float4_nmsub(t2_zwxy, t13_wzyx, cof0);

		cof2 = float4_mul(t0, t13_yxwz);
		cof2 = float4_nmsub(t0, t13_wzyx, cof2);
		cof2 = float4_swiz_zwxy(cof2);

		const float4_t t01 = float4_mul(t0, t1);
		const float4_t t01_yxwz = float4_swiz_yxwz(t01);
		const float4_t t01_wzyx = float4_swiz_wzyx(t01);

		cof2 = float4_nmsub(t3, t01_yxwz, cof2);
		cof2 = float4_madd(t3, t01_wzyx, cof2);

		cof3 = float4_madd(t2_zwxy, t01_yxwz, cof3);
		cof3 = float4_nmsub(t2_zwxy, t01_wzyx, cof3);

		const float4_t t03 = float4_mul(t0, t3);
		const float4_t t03_yxwz = float4_swiz_yxwz(t03);
		const float4_t t03_wzyx = float4_swiz_wzyx(t03);

		cof1 = float4_nmsub(t2_zwxy, t03_yxwz, cof1);
		cof1 = float4_madd(t2_zwxy, t03_wzyx, cof1);

		cof2 = float4_madd(t1, t03_yxwz, cof2);
		cof2 = float4_nmsub(t1, t03_wzyx, cof2);

		const float4_t t02 = float4_mul(t0, t2_zwxy);
		const float4_t t02_yxwz = float4_swiz_yxwz(t02);
		const float4_t t02_wzyx = float4_swiz_wzyx(t02);

		cof1 = float4_madd(t3, t02_yxwz, cof1);
		cof1 = float4_nmsub(t3, t02_wzyx, cof1);

		cof3 = float4_nmsub(t1, t02_yxwz, cof3);
		cof3 = float4_madd(t1, t02_wzyx, cof3);

		const float4_t det    = float4_dot(t0, cof0);
		const float4_t invdet = float4_rcp(det);

		_result->col[0] = float4_mul(cof0, invdet);
		_result->col[1] = float4_mul(cof1, invdet);
		_result->col[2] = float4_mul(cof2, invdet);
		_result->col[3] = float4_mul(cof3, invdet);
	}

} // namespace bx

#endif // BX_FLOAT4X4_H_HEADER_GUARD
