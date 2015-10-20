// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vmadh_vmudh(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo, rsp_vec_t *acc_md, rsp_vec_t *acc_hi)
{
	rsp_vec_t lo = _mm_mullo_epi16(vs, vt);
	rsp_vec_t hi = _mm_mulhi_epi16(vs, vt);

	if (iw & 0x8) // VMADH
	{
		// Tricky part: start accumulating everything.
		// Get/keep the carry as we'll add it in later.
		rsp_vec_t overflow_mask = _mm_adds_epu16(*acc_md, lo);
		*acc_md = _mm_add_epi16(*acc_md, lo);

		overflow_mask = _mm_cmpeq_epi16(*acc_md, overflow_mask);
		overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

		hi = _mm_sub_epi16(hi, overflow_mask);
		*acc_hi = _mm_add_epi16(*acc_hi, hi);
	}
	else // VMUDH
	{
		*acc_lo = zero;
		*acc_md = lo;
		*acc_hi = hi;
	}

	return sclamp_acc_to_mid(*acc_md, *acc_hi);
}
