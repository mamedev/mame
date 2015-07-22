// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vmadl_vmudl(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo, rsp_vec_t *acc_md, rsp_vec_t *acc_hi)
{
	rsp_vec_t hi = _mm_mulhi_epu16(vs, vt);

	if (iw & 0x8) // VMADL
	{
		// Tricky part: start accumulating everything.
		// Get/keep the carry as we'll add it in later.
		rsp_vec_t overflow_mask = _mm_adds_epu16(*acc_lo, hi);
		*acc_lo = _mm_add_epi16(*acc_lo, hi);

		overflow_mask = _mm_cmpeq_epi16(*acc_lo, overflow_mask);
		overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);
		hi = _mm_sub_epi16(zero, overflow_mask);

		// Check for overflow of the upper sum.
		//
		// TODO: Since hi can only be {0,1}, we should
		// be able to generalize this for performance.
		overflow_mask = _mm_adds_epu16(*acc_md, hi);
		*acc_md = _mm_add_epi16(*acc_md, hi);

		overflow_mask = _mm_cmpeq_epi16(*acc_md, overflow_mask);
		overflow_mask = _mm_cmpeq_epi16(overflow_mask, zero);

		// Finish up the accumulation of the... accumulator.
		// Since the product was unsigned, only worry about
		// positive overflow (i.e.: borrowing not possible).
		*acc_hi = _mm_sub_epi16(*acc_hi, overflow_mask);

		return uclamp_acc(*acc_lo, *acc_md, *acc_hi, zero);
	}
	else // VMUDL
	{
		*acc_lo = hi;
		*acc_md = zero;
		*acc_hi = zero;

		return hi;
	}
}
