// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

static inline rsp_vec_t sclamp_acc_to_mid(rsp_vec_t acc_mid, rsp_vec_t acc_hi)
{
	return _mm_packs_epi32(
		_mm_unpacklo_epi16(acc_mid, acc_hi),
		_mm_unpackhi_epi16(acc_mid, acc_hi)
	);
}

static inline rsp_vec_t uclamp_acc(rsp_vec_t val, rsp_vec_t acc_mid, rsp_vec_t acc_hi, rsp_vec_t zero)
{
	rsp_vec_t hi_negative = _mm_srai_epi16(acc_hi, 15); // 0x0000
	rsp_vec_t mid_negative = _mm_srai_epi16(acc_mid, 15); // 0xffff

	// We don't have to clamp if the HI part of the
	// accumulator is sign-extended down to the MD part.
	rsp_vec_t hi_sign_check = _mm_cmpeq_epi16(hi_negative, acc_hi); // 0x0000
	rsp_vec_t mid_sign_check = _mm_cmpeq_epi16(hi_negative, mid_negative); // 0x0000
	rsp_vec_t clamp_mask = _mm_and_si128(mid_sign_check, hi_sign_check); // 0x0000

	// Generate the value in the event we need to clamp.
	//   * hi_negative, mid_sign => xxxx
	//   * hi_negative, !mid_sign => 0000
	//   * !hi_negative, mid_sign => FFFF
	//   * !hi_negative, !mid_sign => xxxx
	rsp_vec_t clamped_val = _mm_cmpeq_epi16(hi_negative, zero); // 0xffff

#if (defined(__SSE4_1__) || defined(_MSC_VER))
	return _mm_blendv_epi8(clamped_val, val, clamp_mask);
#else
	clamped_val = _mm_and_si128(clamp_mask, val);
	val = _mm_andnot_si128(clamp_mask, clamped_val);
	return _mm_or_si128(val, clamped_val);
#endif
}
