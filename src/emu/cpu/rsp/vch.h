// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vch(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *ge, rsp_vec_t *le, rsp_vec_t *eq, rsp_vec_t *sign, rsp_vec_t *vce) {
	// sign = (vs ^ vt) < 0
	*sign = _mm_xor_si128(vs, vt);
	*sign = _mm_cmplt_epi16(*sign, zero);

	// sign_negvt = sign ? -vt : vt
	rsp_vec_t sign_negvt = _mm_xor_si128(vt, *sign);
	sign_negvt = _mm_sub_epi16(sign_negvt, *sign);

	// Compute diff, diff_zero:
	rsp_vec_t diff = _mm_sub_epi16(vs, sign_negvt);
	rsp_vec_t diff_zero = _mm_cmpeq_epi16(diff, zero);

	// Compute le/ge:
	rsp_vec_t vt_neg = _mm_cmplt_epi16(vt, zero);
	rsp_vec_t diff_lez = _mm_cmpgt_epi16(diff, zero);
	rsp_vec_t diff_gez = _mm_or_si128(diff_lez, diff_zero);
	diff_lez = _mm_cmpeq_epi16(zero, diff_lez);

#if (defined(__SSE4_1__) || defined(_MSC_VER))
	*ge = _mm_blendv_epi8(diff_gez, vt_neg, *sign);
	*le = _mm_blendv_epi8(vt_neg, diff_lez, *sign);
#else
	*ge = _mm_and_si128(*sign, vt_neg);
	diff_gez = _mm_andnot_si128(*sign, diff_gez);
	*ge = _mm_or_si128(*ge, diff_gez);

	*le = _mm_and_si128(*sign, diff_lez);
	diff_lez = _mm_andnot_si128(*sign, vt_neg);
	*le = _mm_or_si128(*le, diff_lez);
#endif

	// Compute vce:
	*vce = _mm_cmpeq_epi16(diff, *sign);
	*vce = _mm_and_si128(*vce, *sign);

	// Compute !eq:
	*eq = _mm_or_si128(diff_zero, *vce);
	*eq = _mm_cmpeq_epi16(*eq, zero);

	// Compute result:
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	rsp_vec_t diff_sel_mask = _mm_blendv_epi8(*ge, *le, *sign);
	return _mm_blendv_epi8(vs, sign_negvt, diff_sel_mask);
#else
	diff_lez = _mm_and_si128(*sign, *le);
	diff_gez = _mm_andnot_si128(*sign, *ge);
	rsp_vec_t diff_sel_mask = _mm_or_si128(diff_lez, diff_gez);

	diff_lez = _mm_and_si128(diff_sel_mask, sign_negvt);
	diff_gez = _mm_andnot_si128(diff_sel_mask, vs);
	return _mm_or_si128(diff_lez, diff_gez);
#endif
}
