// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vdivh(UINT32 src, UINT32 e, UINT32 dest, UINT32 de)
{
	// Get the element from VT.
	m_div_in = m_v[src].s[e & 0x7];

	// Write out the upper part of the result.
	m_v[dest].s[de & 0x7] = m_div_out;
	return vec_load_unshuffled_operand(m_v[dest].s);
}
