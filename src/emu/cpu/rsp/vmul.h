// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

//
// TODO: CHECK ME.
//

inline rsp_vec_t vec_vmulf_vmulu(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t zero, rsp_vec_t *acc_lo, rsp_vec_t *acc_md, rsp_vec_t *acc_hi)
{
	rsp_vec_t lo = _mm_mullo_epi16(vs, vt);
	rsp_vec_t round = _mm_cmpeq_epi16(zero, zero);
	rsp_vec_t sign1 = _mm_srli_epi16(lo, 15);
	lo = _mm_add_epi16(lo, lo);
	round = _mm_slli_epi16(round, 15);
	rsp_vec_t hi = _mm_mulhi_epi16(vs, vt);
	rsp_vec_t sign2 = _mm_srli_epi16(lo, 15);
	*acc_lo = _mm_add_epi16(round, lo);
	sign1 = _mm_add_epi16(sign1, sign2);

	hi = _mm_slli_epi16(hi, 1);
	rsp_vec_t eq = _mm_cmpeq_epi16(vs, vt);
	rsp_vec_t neq = eq;
	*acc_md = _mm_add_epi16(hi, sign1);

	rsp_vec_t neg = _mm_srai_epi16(*acc_md, 15);

	if (iw & 0x1) // VMULU
	{
		*acc_hi = _mm_andnot_si128(eq, neg);
		hi =_mm_or_si128(*acc_md, neg);
		return _mm_andnot_si128(*acc_hi, hi);
	}
	else // VMULF
	{
		eq = _mm_and_si128(eq, neg);
		*acc_hi = _mm_andnot_si128(neq, neg);
		return _mm_add_epi16(*acc_md, eq);
	}
}
