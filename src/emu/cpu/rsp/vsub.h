// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vsub(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t carry, rsp_vec_t *acc_lo)
{
	// acc_lo uses saturated arithmetic.
	rsp_vec_t unsat_diff = _mm_sub_epi16(vt, carry);
	rsp_vec_t sat_diff = _mm_subs_epi16(vt, carry);

	*acc_lo = _mm_sub_epi16(vs, unsat_diff);
	rsp_vec_t vd = _mm_subs_epi16(vs, sat_diff);

	// VD is the signed diff of the two sources and the carry. Since we
	// have to saturate the diff of all three, we have to be clever.
	rsp_vec_t overflow = _mm_cmpgt_epi16(sat_diff, unsat_diff);
	return _mm_adds_epi16(vd, overflow);
}
