// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t rsp_vmudh(rsp_vec_t vs, rsp_vec_t vt, rsp_vec_t *acc_md, rsp_vec_t *acc_hi)
{
	*acc_md = _mm_mullo_epi16(vs, vt);
	*acc_hi = _mm_mulhi_epi16(vs, vt);

	return sclamp_acc_to_mid(*acc_md, *acc_hi);
}

