// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vadd(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t carry, rsp_vec_t *acc_lo)
{
	// VCC uses unsaturated arithmetic.
	rsp_vec_t vd = _mm_add_epi16(vs, vt);
	*acc_lo = _mm_sub_epi16(vd, carry);

	// VD is the signed sum of the two sources and the carry. Since we
	// have to saturate the sum of all three, we have to be clever.
	rsp_vec_t minimum = _mm_min_epi16(vs, vt);
	rsp_vec_t maximum = _mm_max_epi16(vs, vt);
	minimum = _mm_subs_epi16(minimum, carry);
	return _mm_adds_epi16(minimum, maximum);
}
