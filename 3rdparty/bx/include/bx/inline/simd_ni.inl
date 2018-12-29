/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

namespace bx
{
	template<typename Ty>
	BX_SIMD_INLINE Ty simd_shuf_xAzC_ni(Ty _a, Ty _b)
	{
		const Ty xAyB   = simd_shuf_xAyB(_a, _b);
		const Ty zCwD   = simd_shuf_zCwD(_a, _b);
		const Ty result = simd_shuf_xyAB(xAyB, zCwD);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_shuf_yBwD_ni(Ty _a, Ty _b)
	{
		const Ty xAyB   = simd_shuf_xAyB(_a, _b);
		const Ty zCwD   = simd_shuf_zCwD(_a, _b);
		const Ty result = simd_shuf_zwCD(xAyB, zCwD);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_madd_ni(Ty _a, Ty _b, Ty _c)
	{
		const Ty mul    = simd_mul(_a, _b);
		const Ty result = simd_add(mul, _c);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_nmsub_ni(Ty _a, Ty _b, Ty _c)
	{
		const Ty mul    = simd_mul(_a, _b);
		const Ty result = simd_sub(_c, mul);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_div_nr_ni(Ty _a, Ty _b)
	{
		const Ty oneish  = simd_isplat<Ty>(0x3f800001);
		const Ty est     = simd_rcp_est(_b);
		const Ty iter0   = simd_mul(_a, est);
		const Ty tmp1    = simd_nmsub(_b, est, oneish);
		const Ty result  = simd_madd(tmp1, iter0, iter0);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rcp_ni(Ty _a)
	{
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty result = simd_div(one, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_orx_ni(Ty _a)
	{
		const Ty zwxy   = simd_swiz_zwxy(_a);
		const Ty tmp0   = simd_or(_a, zwxy);
		const Ty tmp1   = simd_swiz_yyyy(_a);
		const Ty tmp2   = simd_or(tmp0, tmp1);
		const Ty mf000  = simd_ild<Ty>(UINT32_MAX, 0, 0, 0);
		const Ty result = simd_and(tmp2, mf000);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_orc_ni(Ty _a, Ty _b)
	{
		const Ty aorb   = simd_or(_a, _b);
		const Ty mffff  = simd_isplat<Ty>(UINT32_MAX);
		const Ty result = simd_xor(aorb, mffff);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_neg_ni(Ty _a)
	{
		const Ty zero   = simd_zero<Ty>();
		const Ty result = simd_sub(zero, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_selb_ni(Ty _mask, Ty _a, Ty _b)
	{
		const Ty sel_a  = simd_and(_a, _mask);
		const Ty sel_b  = simd_andc(_b, _mask);
		const Ty result = simd_or(sel_a, sel_b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sels_ni(Ty _test, Ty _a, Ty _b)
	{
		const Ty mask   = simd_sra(_test, 31);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_not_ni(Ty _a)
	{
		const Ty mffff  = simd_isplat<Ty>(UINT32_MAX);
		const Ty result = simd_xor(_a, mffff);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_min_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_cmplt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_max_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_cmpgt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_abs_ni(Ty _a)
	{
		const Ty a_neg  = simd_neg(_a);
		const Ty result = simd_max(a_neg, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_imin_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_icmplt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_imax_ni(Ty _a, Ty _b)
	{
		const Ty mask   = simd_icmpgt(_a, _b);
		const Ty result = simd_selb(mask, _a, _b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_clamp_ni(Ty _a, Ty _min, Ty _max)
	{
		const Ty tmp    = simd_min(_a, _max);
		const Ty result = simd_max(tmp, _min);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_lerp_ni(Ty _a, Ty _b, Ty _s)
	{
		const Ty ba     = simd_sub(_b, _a);
		const Ty result = simd_madd(_s, ba, _a);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sqrt_nr_ni(Ty _a)
	{
		const Ty half   = simd_splat<Ty>(0.5f);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp0   = simd_rsqrt_est(_a);
		const Ty tmp1   = simd_mul(tmp0, _a);
		const Ty tmp2   = simd_mul(tmp1, half);
		const Ty tmp3   = simd_nmsub(tmp0, tmp1, one);
		const Ty result = simd_madd(tmp3, tmp2, tmp1);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_sqrt_nr1_ni(Ty _a)
	{
		const Ty half = simd_splat<Ty>(0.5f);

		Ty result = _a;
		for (uint32_t ii = 0; ii < 11; ++ii)
		{
			const Ty tmp1 = simd_div(_a, result);
			const Ty tmp2 = simd_add(tmp1, result);
			result        = simd_mul(tmp2, half);
		}

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_ni(Ty _a)
	{
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty sqrt   = simd_sqrt(_a);
		const Ty result = simd_div(one, sqrt);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_nr_ni(Ty _a)
	{
		const Ty rsqrt           = simd_rsqrt_est(_a);
		const Ty iter0           = simd_mul(_a, rsqrt);
		const Ty iter1           = simd_mul(iter0, rsqrt);
		const Ty half            = simd_splat<Ty>(0.5f);
		const Ty half_rsqrt      = simd_mul(half, rsqrt);
		const Ty three           = simd_splat<Ty>(3.0f);
		const Ty three_sub_iter1 = simd_sub(three, iter1);
		const Ty result          = simd_mul(half_rsqrt, three_sub_iter1);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_rsqrt_carmack_ni(Ty _a)
	{
		const Ty half    = simd_splat<Ty>(0.5f);
		const Ty ah      = simd_mul(half, _a);
		const Ty ashift  = simd_sra(_a, 1);
		const Ty magic   = simd_isplat<Ty>(0x5f3759df);
		const Ty msuba   = simd_isub(magic, ashift);
		const Ty msubasq = simd_mul(msuba, msuba);
		const Ty tmp0    = simd_splat<Ty>(1.5f);
		const Ty tmp1    = simd_mul(ah, msubasq);
		const Ty tmp2    = simd_sub(tmp0, tmp1);
		const Ty result  = simd_mul(msuba, tmp2);

		return result;
	}

	namespace simd_logexp_detail
	{
		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly1(Ty _a, float _b, float _c)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty cccc   = simd_splat<Ty>(_c);
			const Ty result = simd_madd(cccc, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly2(Ty _a, float _b, float _c, float _d)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly1(_a, _c, _d);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly3(Ty _a, float _b, float _c, float _d, float _e)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly2(_a, _c, _d, _e);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly4(Ty _a, float _b, float _c, float _d, float _e, float _f)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly3(_a, _c, _d, _e, _f);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_poly5(Ty _a, float _b, float _c, float _d, float _e, float _f, float _g)
		{
			const Ty bbbb   = simd_splat<Ty>(_b);
			const Ty poly   = simd_poly4(_a, _c, _d, _e, _f, _g);
			const Ty result = simd_madd(poly, _a, bbbb);

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_logpoly(Ty _a)
		{
#if 1
			const Ty result = simd_poly5(_a
				, 3.11578814719469302614f, -3.32419399085241980044f
				, 2.59883907202499966007f, -1.23152682416275988241f
				, 0.318212422185251071475f, -0.0344359067839062357313f
				);
#elif 0
			const Ty result = simd_poly4(_a
				, 2.8882704548164776201f, -2.52074962577807006663f
				, 1.48116647521213171641f, -0.465725644288844778798f
				, 0.0596515482674574969533f
				);
#elif 0
			const Ty result = simd_poly3(_a
				, 2.61761038894603480148f, -1.75647175389045657003f
				, 0.688243882994381274313f, -0.107254423828329604454f
				);
#else
			const Ty result = simd_poly2(_a
				, 2.28330284476918490682f, -1.04913055217340124191f
				, 0.204446009836232697516f
				);
#endif

			return result;
		}

		template<typename Ty>
		BX_SIMD_INLINE Ty simd_exppoly(Ty _a)
		{
#if 1
			const Ty result = simd_poly5(_a
				, 9.9999994e-1f, 6.9315308e-1f
				, 2.4015361e-1f, 5.5826318e-2f
				, 8.9893397e-3f, 1.8775767e-3f
				);
#elif 0
			const Ty result = simd_poly4(_a
				, 1.0000026f, 6.9300383e-1f
				, 2.4144275e-1f, 5.2011464e-2f
				, 1.3534167e-2f
				);
#elif 0
			const Ty result = simd_poly3(_a
				, 9.9992520e-1f, 6.9583356e-1f
				, 2.2606716e-1f, 7.8024521e-2f
				);
#else
			const Ty result = simd_poly2(_a
				, 1.0017247f, 6.5763628e-1f
				, 3.3718944e-1f
				);
#endif // 0

			return result;
		}
	} // namespace simd_internal

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_log2_ni(Ty _a)
	{
		const Ty expmask  = simd_isplat<Ty>(0x7f800000);
		const Ty mantmask = simd_isplat<Ty>(0x007fffff);
		const Ty one      = simd_splat<Ty>(1.0f);

		const Ty c127     = simd_isplat<Ty>(127);
		const Ty aexp     = simd_and(_a, expmask);
		const Ty aexpsr   = simd_srl(aexp, 23);
		const Ty tmp0     = simd_isub(aexpsr, c127);
		const Ty exp      = simd_itof(tmp0);

		const Ty amask    = simd_and(_a, mantmask);
		const Ty mant     = simd_or(amask, one);

		const Ty poly     = simd_logexp_detail::simd_logpoly(mant);

		const Ty mandiff  = simd_sub(mant, one);
		const Ty result   = simd_madd(poly, mandiff, exp);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_exp2_ni(Ty _a)
	{
		const Ty min      = simd_splat<Ty>( 129.0f);
		const Ty max      = simd_splat<Ty>(-126.99999f);
		const Ty tmp0     = simd_min(_a, min);
		const Ty aaaa     = simd_max(tmp0, max);

		const Ty half     = simd_splat<Ty>(0.5f);
		const Ty tmp2     = simd_sub(aaaa, half);
		const Ty ipart    = simd_ftoi(tmp2);
		const Ty iround   = simd_itof(ipart);
		const Ty fpart    = simd_sub(aaaa, iround);

		const Ty c127     = simd_isplat<Ty>(127);
		const Ty tmp5     = simd_iadd(ipart, c127);
		const Ty expipart = simd_sll(tmp5, 23);

		const Ty expfpart = simd_logexp_detail::simd_exppoly(fpart);

		const Ty result   = simd_mul(expipart, expfpart);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_pow_ni(Ty _a, Ty _b)
	{
		const Ty alog2  = simd_log2(_a);
		const Ty alog2b = simd_mul(alog2, _b);
		const Ty result = simd_exp2(alog2b);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_dot3_ni(Ty _a, Ty _b)
	{
		const Ty xyzw   = simd_mul(_a, _b);
		const Ty xxxx   = simd_swiz_xxxx(xyzw);
		const Ty yyyy   = simd_swiz_yyyy(xyzw);
		const Ty zzzz   = simd_swiz_zzzz(xyzw);
		const Ty tmp1   = simd_add(xxxx, yyyy);
		const Ty result = simd_add(zzzz, tmp1);
		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_cross3_ni(Ty _a, Ty _b)
	{
		// a.yzx * b.zxy - a.zxy * b.yzx == (a * b.yzx - a.yzx * b).yzx
#if 0
		const Ty a_yzxw = simd_swiz_yzxw(_a);
		const Ty a_zxyw = simd_swiz_zxyw(_a);
		const Ty b_zxyw = simd_swiz_zxyw(_b);
		const Ty b_yzxw = simd_swiz_yzxw(_b);
		const Ty tmp    = simd_mul(a_yzxw, b_zxyw);
		const Ty result = simd_nmsub(a_zxyw, b_yzxw, tmp);
#else
		const Ty a_yzxw = simd_swiz_yzxw(_a);
		const Ty b_yzxw = simd_swiz_yzxw(_b);
		const Ty tmp0   = simd_mul(_a, b_yzxw);
		const Ty tmp1   = simd_nmsub(a_yzxw, _b, tmp0);
		const Ty result = simd_swiz_yzxw(tmp1);
#endif

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_normalize3_ni(Ty _a)
	{
		const Ty dot3    = simd_dot3(_a, _a);
		const Ty invSqrt = simd_rsqrt(dot3);
		const Ty result  = simd_mul(_a, invSqrt);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_dot_ni(Ty _a, Ty _b)
	{
		const Ty xyzw   = simd_mul(_a, _b);
		const Ty yzwx   = simd_swiz_yzwx(xyzw);
		const Ty tmp0   = simd_add(xyzw, yzwx);
		const Ty zwxy   = simd_swiz_zwxy(tmp0);
		const Ty result = simd_add(tmp0, zwxy);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_ceil_ni(Ty _a)
	{
		const Ty tmp0   = simd_ftoi(_a);
		const Ty tmp1   = simd_itof(tmp0);
		const Ty mask   = simd_cmplt(tmp1, _a);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp2   = simd_and(one, mask);
		const Ty result = simd_add(tmp1, tmp2);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE Ty simd_floor_ni(Ty _a)
	{
		const Ty tmp0   = simd_ftoi(_a);
		const Ty tmp1   = simd_itof(tmp0);
		const Ty mask   = simd_cmpgt(tmp1, _a);
		const Ty one    = simd_splat<Ty>(1.0f);
		const Ty tmp2   = simd_and(one, mask);
		const Ty result = simd_sub(tmp1, tmp2);

		return result;
	}

	template<typename Ty>
	BX_SIMD_FORCE_INLINE Ty simd_round_ni(Ty _a)
	{
		const Ty tmp    = simd_ftoi(_a);
		const Ty result = simd_itof(tmp);

		return result;
	}

	template<typename Ty>
	BX_SIMD_INLINE bool simd_test_any_ni(Ty _a)
	{
		const Ty mask = simd_sra(_a, 31);
		const Ty zwxy = simd_swiz_zwxy(mask);
		const Ty tmp0 = simd_or(mask, zwxy);
		const Ty tmp1 = simd_swiz_yyyy(tmp0);
		const Ty tmp2 = simd_or(tmp0, tmp1);
		int res;
		simd_stx(&res, tmp2);
		return 0 != res;
	}

	template<typename Ty>
	BX_SIMD_INLINE bool simd_test_all_ni(Ty _a)
	{
		const Ty bits  = simd_sra(_a, 31);
		const Ty m1248 = simd_ild<Ty>(1, 2, 4, 8);
		const Ty mask  = simd_and(bits, m1248);
		const Ty zwxy  = simd_swiz_zwxy(mask);
		const Ty tmp0  = simd_or(mask, zwxy);
		const Ty tmp1  = simd_swiz_yyyy(tmp0);
		const Ty tmp2  = simd_or(tmp0, tmp1);
		int res;
		simd_stx(&res, tmp2);
		return 0xf == res;
	}

} // namespace bx
