// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vcr(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *ge, rsp_vec_t *le) {
	// sign = (vs ^ vt) < 0
	rsp_vec_t sign = _mm_xor_si128(vs, vt);
	sign = _mm_srai_epi16(sign, 15);

	// Compute le
	rsp_vec_t diff_lez = _mm_and_si128(vs, sign);
	diff_lez = _mm_add_epi16(diff_lez, vt);
	*le = _mm_srai_epi16(diff_lez, 15);

	// Compute ge
	rsp_vec_t diff_gez = _mm_or_si128(vs, sign);
	diff_gez = _mm_min_epi16(diff_gez, vt);
	*ge = _mm_cmpeq_epi16(diff_gez, vt);

	// sign_notvt = sn ? ~vt : vt
	rsp_vec_t sign_notvt = _mm_xor_si128(vt, sign);

	// Compute result:
#if (defined(__SSE4_1__) || defined(_MSC_VER))
	rsp_vec_t diff_sel_mask = _mm_blendv_epi8(*ge, *le, sign);
	return _mm_blendv_epi8(vs, sign_notvt, diff_sel_mask);
#else
	rsp_vec_t diff_sel_mask = _mm_sub_epi16(*le, *ge);
	diff_sel_mask = _mm_and_si128(diff_sel_mask, sign);
	diff_sel_mask = _mm_add_epi16(diff_sel_mask, *ge);

	zero = _mm_sub_epi16(sign_notvt, vs);
	zero = _mm_and_si128(zero, diff_sel_mask);
	return _mm_add_epi16(zero, vs);
#endif
}
