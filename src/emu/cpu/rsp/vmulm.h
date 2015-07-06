// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vmadm_vmudm(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo, rsp_vec_t *acc_md, rsp_vec_t *acc_hi)
{
	rsp_vec_t lo = _mm_mullo_epi16(vs, vt);
	rsp_vec_t hi = _mm_mulhi_epu16(vs, vt);

	// What we really want to do is unsigned vs * signed vt.
	// However, we have no such instructions to do so.
	//
	// There's a trick to "fix" an unsigned product, though:
	// If vt was negative, take the upper 16-bits of the product
	// and subtract vs.
	rsp_vec_t sign = _mm_srai_epi16(vs, 15);
	vt = _mm_and_si128(vt, sign);
	hi = _mm_sub_epi16(hi, vt);

	if (iw & 0x8) // VMADM
	{
		// Tricky part: start accumulating everything.
		// Get/keep the carry as we'll add it in later.
		rsp_vec_t overflow_mask = _mm_adds_epu16(*acc_lo, lo);
		*acc_lo = _mm_add_epi16(*acc_lo, lo);

		overflow_mask = _mm_cmpeq_epi16(*acc_lo, overflow_mask);
		overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

		// This is REALLY clever. Since the product results from
		// two 16-bit components, one positive and one negative,
		// we don't have to worry about carrying the 1 (we can
		// only borrow) past 32-bits. So we can just add it here.
		hi = _mm_sub_epi16(hi, overflow_mask);

		// Check for overflow of the upper sum.
		overflow_mask = _mm_adds_epu16(*acc_md, hi);
		*acc_md = _mm_add_epi16(*acc_md, hi);

		overflow_mask = _mm_cmpeq_epi16(*acc_md, overflow_mask);
		overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

		// Finish up the accumulation of the... accumulator.
		*acc_hi = _mm_add_epi16(*acc_hi, _mm_srai_epi16(hi, 15));
		*acc_hi = _mm_sub_epi16(*acc_hi, overflow_mask);

		return sclamp_acc_to_mid(*acc_md, *acc_hi);
	}
	else // VMUDM
	{
		*acc_lo = lo;
		*acc_md = hi;
		*acc_hi = _mm_srai_epi16(hi, 15);

		return hi;
	}
}
