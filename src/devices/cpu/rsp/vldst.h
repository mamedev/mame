// license:BSD-3-Clause
// copyright-holders:Tyler J. Stachecki,Ryan Holtz

// LBV, LDV, LLV, LSV, SBV, SDV, SLV, SSV
inline void vec_lbdlsv_sbdlsv(UINT32 iw, UINT32 rs)
{
	const UINT32 shift_and_idx = (iw >> 11) & 0x3;
	rsp_vec_t dqm = _mm_loadl_epi64((rsp_vec_t *) (m_vec_helpers.bdls_lut[shift_and_idx]));

	const UINT32 addr = (rs + (sign_extend_6(iw) << shift_and_idx)) & 0xfff;
	const UINT32 element = (iw >> 7) & 0xf;
	UINT16* regp = m_v[(iw >> 16) & 0x1f].s;

	if (iw >> 29 & 0x1)
	{
		vec_store_group1(addr, element, regp, vec_load_unshuffled_operand(regp), dqm);
	}
	else
	{
		vec_load_group1(addr, element, regp, vec_load_unshuffled_operand(regp), dqm);
	}
}

// LPV, LUV, SPV, SUV
inline void vec_lfhpuv_sfhpuv(UINT32 iw, UINT32 rs)
{
	static const enum rsp_mem_request_type fhpu_type_lut[4] = {
		RSP_MEM_REQUEST_PACK,
		RSP_MEM_REQUEST_UPACK,
		RSP_MEM_REQUEST_HALF,
		RSP_MEM_REQUEST_FOURTH
	};

	const UINT32 addr = (rs + (sign_extend_6(iw) << 3)) & 0xfff;
	const UINT32 element = (iw >> 7) & 0xf;
	UINT16* regp = m_v[(iw >> 16) & 0x1f].s;

	rsp_mem_request_type request_type = fhpu_type_lut[((iw >> 11) & 0x1f) - 6];
	if ((iw >> 29) & 0x1)
	{
		vec_store_group2(addr, element, regp, vec_load_unshuffled_operand(regp), _mm_setzero_si128(), request_type);
	}
	else
	{
		vec_load_group2(addr, element, regp, vec_load_unshuffled_operand(regp), _mm_setzero_si128(), request_type);
	}
}

// LQV, LRV, SQV, SRV
inline void vec_lqrv_sqrv(UINT32 iw, UINT32 rs)
{
	rs &= 0xfff;

	const UINT32 addr = rs + (sign_extend_6(iw) << 4);
	const UINT32 element = (iw >> 7) & 0xf;
	UINT16* regp = m_v[(iw >> 16) & 0x1f].s;

	memcpy(m_vdqm.s, m_vec_helpers.qr_lut[addr & 0xf], sizeof(m_vdqm.s));

	rsp_mem_request_type request_type = (iw >> 11 & 0x1) ? RSP_MEM_REQUEST_REST : RSP_MEM_REQUEST_QUAD;
	if ((iw >> 29) & 0x1)
	{
		vec_store_group4(addr, element, regp, vec_load_unshuffled_operand(regp), vec_load_unshuffled_operand(m_vdqm.s), request_type);
	}
	else
	{
		vec_load_group4(addr, element, regp, vec_load_unshuffled_operand(regp), vec_load_unshuffled_operand(m_vdqm.s), request_type);
	}
}
