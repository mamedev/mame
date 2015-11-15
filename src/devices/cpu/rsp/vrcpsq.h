// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vrcp_vrsq(UINT32 iw, INT32 dp, UINT32 src, UINT32 e, UINT32 dest, UINT32 de)
{
	// Get the element from VT.
	INT16 vt = m_v[src].s[e & 0x7];

	UINT32 dp_input = ((UINT32) m_div_in << 16) | (UINT16) vt;
	UINT32 sp_input = vt;

	INT32 input = (dp) ? dp_input : sp_input;
	INT32 input_mask = input >> 31;
	INT32 data = input ^ input_mask;

	if (input > -32768)
	{
		data -= input_mask;
	}

	// Handle edge cases.
	INT32 result;
	if (data == 0)
	{
		result = 0x7fffffff;
	}
	else if (input == -32768)
	{
		result = 0xffff0000;
	}
	else // Main case: compute the reciprocal.
	{
		UINT32 shift = count_leading_zeros(data);
		UINT32 idx = (((UINT64) data << shift) & 0x7FC00000) >> 22;

		if (iw & 0x4) // VRSQ
		{
			idx = ((idx | 0x200) & 0x3fe) | (shift % 2);
			result = rsp_divtable[idx];

			result = ((0x10000 | result) << 14) >> ((31 - shift) >> 1);
		}
		else // VRCP
		{
			result = rsp_divtable[idx];

			result = ((0x10000 | result) << 14) >> (31 - shift);
		}

		result = result ^ input_mask;
	}

	// Write out the results.
	m_div_out = result >> 16;
	m_v[dest].s[de & 0x7] = result;

	return vec_load_unshuffled_operand(m_v[dest].s);
}
