// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vabs(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo)
{
	rsp_vec_t vs_zero = _mm_cmpeq_epi16(vs, zero);
	rsp_vec_t sign_lt = _mm_srai_epi16(vs, 15);
	rsp_vec_t vd = _mm_andnot_si128(vs_zero, vt);

	// Careful: if VT = 0x8000 and VS is negative,
	// acc_lo will be 0x8000 but vd will be 0x7FFF.
	vd = _mm_xor_si128(vd, sign_lt);
	*acc_lo = _mm_sub_epi16(vd, sign_lt);
	return _mm_subs_epi16(vd, sign_lt);
}
