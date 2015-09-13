// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vand_vnand(UINT32 iw, rsp_vec_t vs, rsp_vec_t vt) {
	rsp_vec_t vmask = _mm_load_si128((rsp_vec_t *) m_vec_helpers.logic_mask[iw & 0x1]);

	rsp_vec_t vd = _mm_and_si128(vs, vt);
	return _mm_xor_si128(vd, vmask);
}
