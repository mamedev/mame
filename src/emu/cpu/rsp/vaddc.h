// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vaddc(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *sn)
{
	rsp_vec_t sat_sum = _mm_adds_epu16(vs, vt);
	rsp_vec_t unsat_sum = _mm_add_epi16(vs, vt);

	*sn = _mm_cmpeq_epi16(sat_sum, unsat_sum);
	*sn = _mm_cmpeq_epi16(*sn, zero);

	return unsat_sum;
}
