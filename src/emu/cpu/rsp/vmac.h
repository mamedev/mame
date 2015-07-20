// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vmacf_vmacu(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo, rsp_vec_t *acc_mid, rsp_vec_t *acc_hi)
{
	// Get the product and shift it over
	// being sure to save the carries.
	rsp_vec_t lo = _mm_mullo_epi16(vs, vt);
	rsp_vec_t hi = _mm_mulhi_epi16(vs, vt);

	rsp_vec_t mid = _mm_slli_epi16(hi, 1);
	rsp_vec_t carry = _mm_srli_epi16(lo, 15);
	hi = _mm_srai_epi16(hi, 15);
	mid = _mm_or_si128(mid, carry);
	lo = _mm_slli_epi16(lo, 1);

	// Tricky part: start accumulating everything.
	// Get/keep the carry as we'll add it in later.
	rsp_vec_t overflow_mask = _mm_adds_epu16(*acc_lo, lo);
	*acc_lo = _mm_add_epi16(*acc_lo, lo);

	overflow_mask = _mm_cmpeq_epi16(*acc_lo, overflow_mask);
	overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

	// Add in the carry. If the middle portion is
	// already 0xFFFF and we have a carry, we have
	// to carry the all the way up to hi.
	mid = _mm_sub_epi16(mid, overflow_mask);
	carry = _mm_cmpeq_epi16(mid, zero);
	carry = _mm_and_si128(carry, overflow_mask);
	hi = _mm_sub_epi16(hi, carry);

	// Accumulate the middle portion.
	overflow_mask = _mm_adds_epu16(*acc_mid, mid);
	*acc_mid = _mm_add_epi16(*acc_mid, mid);

	overflow_mask = _mm_cmpeq_epi16(*acc_mid, overflow_mask);
	overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

	// Finish up the accumulation of the... accumulator.
	*acc_hi = _mm_add_epi16(*acc_hi, hi);
	*acc_hi = _mm_sub_epi16(*acc_hi, overflow_mask);

	if (iw & 0x1) // VMACU
	{
		rsp_vec_t overflow_hi_mask = _mm_srai_epi16(*acc_hi, 15);
		rsp_vec_t overflow_mid_mask = _mm_srai_epi16(*acc_mid, 15);
		mid = _mm_or_si128(overflow_mid_mask, *acc_mid);
		overflow_mask = _mm_cmpgt_epi16(*acc_hi, zero);
		mid = _mm_andnot_si128(overflow_hi_mask, mid);
		return _mm_or_si128(overflow_mask, mid);
	}
	else // VMACF
	{
		return sclamp_acc_to_mid(*acc_mid, *acc_hi);
	}
}
