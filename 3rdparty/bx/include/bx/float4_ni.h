/*
 * Copyright 2010-2016 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_FLOAT4_NI_H_HEADER_GUARD
#define BX_FLOAT4_NI_H_HEADER_GUARD

namespace bx
{
	BX_FLOAT4_INLINE float4_t float4_rcp_ni(float4_t _a);

	BX_FLOAT4_INLINE float4_t float4_shuf_xAzC_ni(float4_t _a, float4_t _b)
	{
		const float4_t xAyB   = float4_shuf_xAyB(_a, _b);
		const float4_t zCwD   = float4_shuf_zCwD(_a, _b);
		const float4_t result = float4_shuf_xyAB(xAyB, zCwD);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_shuf_yBwD_ni(float4_t _a, float4_t _b)
	{
		const float4_t xAyB   = float4_shuf_xAyB(_a, _b);
		const float4_t zCwD   = float4_shuf_zCwD(_a, _b);
		const float4_t result = float4_shuf_zwCD(xAyB, zCwD);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_madd_ni(float4_t _a, float4_t _b, float4_t _c)
	{
		const float4_t mul    = float4_mul(_a, _b);
		const float4_t result = float4_add(mul, _c);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_nmsub_ni(float4_t _a, float4_t _b, float4_t _c)
	{
		const float4_t mul    = float4_mul(_a, _b);
		const float4_t result = float4_sub(_c, mul);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_div_nr_ni(float4_t _a, float4_t _b)
	{
		const float4_t oneish  = float4_isplat(0x3f800001);
		const float4_t est     = float4_rcp_est(_b);
		const float4_t iter0   = float4_mul(_a, est);
		const float4_t tmp1    = float4_nmsub(_b, est, oneish);
		const float4_t result  = float4_madd(tmp1, iter0, iter0);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_rcp_ni(float4_t _a)
	{
		const float4_t one    = float4_splat(1.0f);
		const float4_t result = float4_div(one, _a);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_orx_ni(float4_t _a)
	{
		const float4_t zwxy   = float4_swiz_zwxy(_a);
		const float4_t tmp0   = float4_or(_a, zwxy);
		const float4_t tmp1   = float4_swiz_yyyy(_a);
		const float4_t tmp2   = float4_or(tmp0, tmp1);
		const float4_t mf000  = float4_ild(UINT32_MAX, 0, 0, 0);
		const float4_t result = float4_and(tmp2, mf000);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_orc_ni(float4_t _a, float4_t _b)
	{
		const float4_t aorb   = float4_or(_a, _b);
		const float4_t mffff  = float4_isplat(UINT32_MAX);
		const float4_t result = float4_xor(aorb, mffff);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_neg_ni(float4_t _a)
	{
		const float4_t zero   = float4_zero();
		const float4_t result = float4_sub(zero, _a);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_selb_ni(float4_t _mask, float4_t _a, float4_t _b)
	{
		const float4_t sel_a  = float4_and(_a, _mask);
		const float4_t sel_b  = float4_andc(_b, _mask);
		const float4_t result = float4_or(sel_a, sel_b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_sels_ni(float4_t _test, float4_t _a, float4_t _b)
	{
		const float4_t mask   = float4_sra(_test, 31);
		const float4_t result = float4_selb(mask, _a, _b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_not_ni(float4_t _a)
	{
		const float4_t mffff  = float4_isplat(UINT32_MAX);
		const float4_t result = float4_xor(_a, mffff);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_min_ni(float4_t _a, float4_t _b)
	{
		const float4_t mask   = float4_cmplt(_a, _b);
		const float4_t result = float4_selb(mask, _a, _b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_max_ni(float4_t _a, float4_t _b)
	{
		const float4_t mask   = float4_cmpgt(_a, _b);
		const float4_t result = float4_selb(mask, _a, _b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_abs_ni(float4_t _a)
	{
		const float4_t a_neg  = float4_neg(_a);
		const float4_t result = float4_max(a_neg, _a);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_imin_ni(float4_t _a, float4_t _b)
	{
		const float4_t mask   = float4_icmplt(_a, _b);
		const float4_t result = float4_selb(mask, _a, _b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_imax_ni(float4_t _a, float4_t _b)
	{
		const float4_t mask   = float4_icmpgt(_a, _b);
		const float4_t result = float4_selb(mask, _a, _b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_clamp_ni(float4_t _a, float4_t _min, float4_t _max)
	{
		const float4_t tmp    = float4_min(_a, _max);
		const float4_t result = float4_max(tmp, _min);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_lerp_ni(float4_t _a, float4_t _b, float4_t _s)
	{
		const float4_t ba     = float4_sub(_b, _a);
		const float4_t result = float4_madd(_s, ba, _a);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_sqrt_nr_ni(float4_t _a)
	{
		const float4_t half   = float4_splat(0.5f);
		const float4_t one    = float4_splat(1.0f);
		const float4_t tmp0   = float4_rsqrt_est(_a);
		const float4_t tmp1   = float4_mul(tmp0, _a);
		const float4_t tmp2   = float4_mul(tmp1, half);
		const float4_t tmp3   = float4_nmsub(tmp0, tmp1, one);
		const float4_t result = float4_madd(tmp3, tmp2, tmp1);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_sqrt_nr1_ni(float4_t _a)
	{
		const float4_t half = float4_splat(0.5f);

		float4_t result = _a;
		for (uint32_t ii = 0; ii < 11; ++ii)
		{
			const float4_t tmp1 = float4_div(_a, result);
			const float4_t tmp2 = float4_add(tmp1, result);
			result              = float4_mul(tmp2, half);
		}

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_rsqrt_ni(float4_t _a)
	{
		const float4_t one    = float4_splat(1.0f);
		const float4_t sqrt   = float4_sqrt(_a);
		const float4_t result = float4_div(one, sqrt);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_rsqrt_nr_ni(float4_t _a)
	{
		const float4_t rsqrt           = float4_rsqrt_est(_a);
		const float4_t iter0           = float4_mul(_a, rsqrt);
		const float4_t iter1           = float4_mul(iter0, rsqrt);
		const float4_t half            = float4_splat(0.5f);
		const float4_t half_rsqrt      = float4_mul(half, rsqrt);
		const float4_t three           = float4_splat(3.0f);
		const float4_t three_sub_iter1 = float4_sub(three, iter1);
		const float4_t result          = float4_mul(half_rsqrt, three_sub_iter1);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_rsqrt_carmack_ni(float4_t _a)
	{
		const float4_t half    = float4_splat(0.5f);
		const float4_t ah      = float4_mul(half, _a);
		const float4_t ashift  = float4_sra(_a, 1);
		const float4_t magic   = float4_isplat(0x5f3759df);
		const float4_t msuba   = float4_isub(magic, ashift);
		const float4_t msubasq = float4_mul(msuba, msuba);
		const float4_t tmp0    = float4_splat(1.5f);
		const float4_t tmp1    = float4_mul(ah, msubasq);
		const float4_t tmp2    = float4_sub(tmp0, tmp1);
		const float4_t result  = float4_mul(msuba, tmp2);

		return result;
	}

	namespace float4_logexp_detail
	{
		BX_FLOAT4_INLINE float4_t float4_poly1(float4_t _a, float _b, float _c)
		{
			const float4_t bbbb   = float4_splat(_b);
			const float4_t cccc   = float4_splat(_c);
			const float4_t result = float4_madd(cccc, _a, bbbb);

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_poly2(float4_t _a, float _b, float _c, float _d)
		{
			const float4_t bbbb   = float4_splat(_b);
			const float4_t poly   = float4_poly1(_a, _c, _d);
			const float4_t result = float4_madd(poly, _a, bbbb);

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_poly3(float4_t _a, float _b, float _c, float _d, float _e)
		{
			const float4_t bbbb   = float4_splat(_b);
			const float4_t poly   = float4_poly2(_a, _c, _d, _e);
			const float4_t result = float4_madd(poly, _a, bbbb);

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_poly4(float4_t _a, float _b, float _c, float _d, float _e, float _f)
		{
			const float4_t bbbb   = float4_splat(_b);
			const float4_t poly   = float4_poly3(_a, _c, _d, _e, _f);
			const float4_t result = float4_madd(poly, _a, bbbb);

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_poly5(float4_t _a, float _b, float _c, float _d, float _e, float _f, float _g)
		{
			const float4_t bbbb   = float4_splat(_b);
			const float4_t poly   = float4_poly4(_a, _c, _d, _e, _f, _g);
			const float4_t result = float4_madd(poly, _a, bbbb);

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_logpoly(float4_t _a)
		{
#if 1
			const float4_t result = float4_poly5(_a
				, 3.11578814719469302614f, -3.32419399085241980044f
				, 2.59883907202499966007f, -1.23152682416275988241f
				, 0.318212422185251071475f, -0.0344359067839062357313f
				);
#elif 0
			const float4_t result = float4_poly4(_a
				, 2.8882704548164776201f, -2.52074962577807006663f
				, 1.48116647521213171641f, -0.465725644288844778798f
				, 0.0596515482674574969533f
				);
#elif 0
			const float4_t result = float4_poly3(_a
				, 2.61761038894603480148f, -1.75647175389045657003f
				, 0.688243882994381274313f, -0.107254423828329604454f
				);
#else
			const float4_t result = float4_poly2(_a
				, 2.28330284476918490682f, -1.04913055217340124191f
				, 0.204446009836232697516f
				);
#endif

			return result;
		}

		BX_FLOAT4_INLINE float4_t float4_exppoly(float4_t _a)
		{
#if 1
			const float4_t result = float4_poly5(_a
				, 9.9999994e-1f, 6.9315308e-1f
				, 2.4015361e-1f, 5.5826318e-2f
				, 8.9893397e-3f, 1.8775767e-3f
				);
#elif 0
			const float4_t result = float4_poly4(_a
				, 1.0000026f, 6.9300383e-1f
				, 2.4144275e-1f, 5.2011464e-2f
				, 1.3534167e-2f
				);
#elif 0
			const float4_t result = float4_poly3(_a
				, 9.9992520e-1f, 6.9583356e-1f
				, 2.2606716e-1f, 7.8024521e-2f
				);
#else
			const float4_t result = float4_poly2(_a
				, 1.0017247f, 6.5763628e-1f
				, 3.3718944e-1f
				);
#endif // 0

			return result;
		}
	} // namespace float4_internal

	BX_FLOAT4_INLINE float4_t float4_log2_ni(float4_t _a)
	{
		const float4_t expmask  = float4_isplat(0x7f800000);
		const float4_t mantmask = float4_isplat(0x007fffff);
		const float4_t one      = float4_splat(1.0f);

		const float4_t c127     = float4_isplat(127);
		const float4_t aexp     = float4_and(_a, expmask);
		const float4_t aexpsr   = float4_srl(aexp, 23);
		const float4_t tmp0     = float4_isub(aexpsr, c127);
		const float4_t exp      = float4_itof(tmp0);

		const float4_t amask    = float4_and(_a, mantmask);
		const float4_t mant     = float4_or(amask, one);

		const float4_t poly     = float4_logexp_detail::float4_logpoly(mant);

		const float4_t mandiff  = float4_sub(mant, one);
		const float4_t result   = float4_madd(poly, mandiff, exp);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_exp2_ni(float4_t _a)
	{
		const float4_t min      = float4_splat( 129.0f);
		const float4_t max      = float4_splat(-126.99999f);
		const float4_t tmp0     = float4_min(_a, min);
		const float4_t aaaa     = float4_max(tmp0, max);

		const float4_t half     = float4_splat(0.5f);
		const float4_t tmp2     = float4_sub(aaaa, half);
		const float4_t ipart    = float4_ftoi(tmp2);
		const float4_t iround   = float4_itof(ipart);
		const float4_t fpart    = float4_sub(aaaa, iround);

		const float4_t c127     = float4_isplat(127);
		const float4_t tmp5     = float4_iadd(ipart, c127);
		const float4_t expipart = float4_sll(tmp5, 23);

		const float4_t expfpart = float4_logexp_detail::float4_exppoly(fpart);

		const float4_t result   = float4_mul(expipart, expfpart);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_pow_ni(float4_t _a, float4_t _b)
	{
		const float4_t alog2  = float4_log2(_a);
		const float4_t alog2b = float4_mul(alog2, _b);
		const float4_t result = float4_exp2(alog2b);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_dot3_ni(float4_t _a, float4_t _b)
	{
		const float4_t xyzw   = float4_mul(_a, _b);
		const float4_t xxxx   = float4_swiz_xxxx(xyzw);
		const float4_t yyyy   = float4_swiz_yyyy(xyzw);
		const float4_t zzzz   = float4_swiz_zzzz(xyzw);
		const float4_t tmp1   = float4_add(xxxx, yyyy);
		const float4_t result = float4_add(zzzz, tmp1);
		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_cross3_ni(float4_t _a, float4_t _b)
	{
		// a.yzx * b.zxy - a.zxy * b.yzx == (a * b.yzx - a.yzx * b).yzx
#if 0
		const float4_t a_yzxw = float4_swiz_yzxw(_a);
		const float4_t a_zxyw = float4_swiz_zxyw(_a);
		const float4_t b_zxyw = float4_swiz_zxyw(_b);
		const float4_t b_yzxw = float4_swiz_yzxw(_b);
		const float4_t tmp    = float4_mul(a_yzxw, b_zxyw);
		const float4_t result = float4_nmsub(a_zxyw, b_yzxw, tmp);
#else
		const float4_t a_yzxw = float4_swiz_yzxw(_a);
		const float4_t b_yzxw = float4_swiz_yzxw(_b);
		const float4_t tmp0   = float4_mul(_a, b_yzxw);
		const float4_t tmp1   = float4_nmsub(a_yzxw, _b, tmp0);
		const float4_t result = float4_swiz_yzxw(tmp1);
#endif

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_normalize3_ni(float4_t _a)
	{
		const float4_t dot3    = float4_dot3(_a, _a);
		const float4_t invSqrt = float4_rsqrt(dot3);
		const float4_t result  = float4_mul(_a, invSqrt);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_dot_ni(float4_t _a, float4_t _b)
	{
		const float4_t xyzw   = float4_mul(_a, _b);
		const float4_t yzwx   = float4_swiz_yzwx(xyzw);
		const float4_t tmp0   = float4_add(xyzw, yzwx);
		const float4_t zwxy   = float4_swiz_zwxy(tmp0);
		const float4_t result = float4_add(tmp0, zwxy);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_ceil_ni(float4_t _a)
	{
		const float4_t tmp0   = float4_ftoi(_a);
		const float4_t tmp1   = float4_itof(tmp0);
		const float4_t mask   = float4_cmplt(tmp1, _a);
		const float4_t one    = float4_splat(1.0f);
		const float4_t tmp2   = float4_and(one, mask);
		const float4_t result = float4_add(tmp1, tmp2);

		return result;
	}

	BX_FLOAT4_INLINE float4_t float4_floor_ni(float4_t _a)
	{
		const float4_t tmp0   = float4_ftoi(_a);
		const float4_t tmp1   = float4_itof(tmp0);
		const float4_t mask   = float4_cmpgt(tmp1, _a);
		const float4_t one    = float4_splat(1.0f);
		const float4_t tmp2   = float4_and(one, mask);
		const float4_t result = float4_sub(tmp1, tmp2);

		return result;
	}

	BX_FLOAT4_INLINE bool float4_test_any_ni(float4_t _a)
	{
		const float4_t mask   = float4_sra(_a, 31);
		const float4_t zwxy   = float4_swiz_zwxy(mask);
		const float4_t tmp0   = float4_or(mask, zwxy);
		const float4_t tmp1   = float4_swiz_yyyy(tmp0);
		const float4_t tmp2   = float4_or(tmp0, tmp1);
		int res;
		float4_stx(&res, tmp2);
		return 0 != res;
	}

	BX_FLOAT4_INLINE bool float4_test_all_ni(float4_t _a)
	{
		const float4_t bits   = float4_sra(_a, 31);
		const float4_t m1248  = float4_ild(1, 2, 4, 8);
		const float4_t mask   = float4_and(bits, m1248);
		const float4_t zwxy   = float4_swiz_zwxy(mask);
		const float4_t tmp0   = float4_or(mask, zwxy);
		const float4_t tmp1   = float4_swiz_yyyy(tmp0);
		const float4_t tmp2   = float4_or(tmp0, tmp1);
		int res;
		float4_stx(&res, tmp2);
		return 0xf == res;
	}

} // namespace bx

#endif // BX_FLOAT4_NI_H_HEADER_GUARD
