// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vcl(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *ge, rsp_vec_t *le, rsp_vec_t eq, rsp_vec_t sign, rsp_vec_t vce)
{
	// sign_negvt = sign ? -vt : vt
	rsp_vec_t sign_negvt = _mm_xor_si128(vt, sign);
	sign_negvt = _mm_sub_epi16(sign_negvt, sign);

	// Compute diff, diff_zero, ncarry, and nvce:
	// Note: diff = sign ? (vs + vt) : (vs - vt).
	rsp_vec_t diff = _mm_sub_epi16(vs, sign_negvt);
	rsp_vec_t ncarry = _mm_adds_epu16(vs, vt);
	ncarry = _mm_cmpeq_epi16(diff, ncarry);
	rsp_vec_t nvce = _mm_cmpeq_epi16(vce, zero);
	rsp_vec_t diff_zero = _mm_cmpeq_epi16(diff, zero);

	// Compute results for if (sign && ne):
	rsp_vec_t le_case1 = _mm_and_si128(diff_zero, ncarry);
	le_case1 = _mm_and_si128(nvce, le_case1);
	rsp_vec_t le_case2 = _mm_or_si128(diff_zero, ncarry);
	le_case2 = _mm_and_si128(vce, le_case2);
	rsp_vec_t le_eq = _mm_or_si128(le_case1, le_case2);

	// Compute results for if (!sign && ne):
	rsp_vec_t ge_eq = _mm_subs_epu16(vt, vs);
	ge_eq = _mm_cmpeq_epi16(ge_eq, zero);

	// Blend everything together. Caveat: we don't update
	// the results of ge/le if ne is false, so be careful.
	rsp_vec_t do_le = _mm_andnot_si128(eq, sign);
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	*le = _mm_blendv_epi8(*le, le_eq, do_le);
#else
	le_eq = _mm_and_si128(do_le, le_eq);
	*le = _mm_andnot_si128(do_le, *le);
	*le = _mm_or_si128(le_eq, *le);
#endif

	rsp_vec_t do_ge = _mm_or_si128(sign, eq);
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	*ge = _mm_blendv_epi8(ge_eq, *ge, do_ge);
#else
	*ge = _mm_and_si128(do_ge, *ge);
	ge_eq = _mm_andnot_si128(do_ge, ge_eq);
	*ge = _mm_or_si128(ge_eq, *ge);
#endif

	// Mux the result based on the value of sign.
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	rsp_vec_t mux_mask = _mm_blendv_epi8(*ge, *le, sign);
#else
	do_le = _mm_and_si128(sign, *le);
	do_ge = _mm_andnot_si128(sign, *ge);
	rsp_vec_t mux_mask  = _mm_or_si128(do_le, do_ge);
#endif

#if (defined(__SSE4_1__) || defined(_MSC_VER))
	return _mm_blendv_epi8(vs, sign_negvt, mux_mask);
#else
	sign_negvt = _mm_and_si128(mux_mask, sign_negvt);
	vs = _mm_andnot_si128(mux_mask, vs);
	return _mm_or_si128(sign_negvt, vs);
#endif
}
