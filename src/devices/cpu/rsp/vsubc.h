// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vsubc(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *eq, rsp_vec_t *sn)
{
	rsp_vec_t sat_udiff = _mm_subs_epu16(vs, vt);
	rsp_vec_t equal = _mm_cmpeq_epi16(vs, vt);
	rsp_vec_t sat_udiff_zero = _mm_cmpeq_epi16(sat_udiff, zero);

	*eq = _mm_cmpeq_epi16(equal, zero);
	*sn = _mm_andnot_si128(equal, sat_udiff_zero);

	return _mm_sub_epi16(vs, vt);
}
