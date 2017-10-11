// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

inline rsp_vec_t vec_vrcp_vrsq(uint32_t iw, int32_t dp, uint32_t src, uint32_t e, uint32_t dest, uint32_t de)
{
	// Get the element from VT.
	int16_t vt = m_v[src].s[e & 0x7];

	uint32_t dp_input = ((uint32_t) m_div_in << 16) | (uint16_t) vt;
	uint32_t sp_input = vt;

	int32_t input = (dp) ? dp_input : sp_input;
	int32_t input_mask = input >> 31;
	int32_t data = input ^ input_mask;

	if (input > -32768)
	{
		data -= input_mask;
	}

	// Handle edge cases.
	int32_t result;
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
		uint32_t shift = count_leading_zeros(data);
		uint32_t idx = (((uint64_t) data << shift) & 0x7FC00000) >> 22;

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
