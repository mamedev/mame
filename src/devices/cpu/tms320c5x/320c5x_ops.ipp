// license:BSD-3-Clause
// copyright-holders:Ville Linde
// stack is LIFO and is 8 levels deep, there is no stackpointer on the real chip
void tms320c51_device::PUSH_STACK(uint16_t pc)
{
	m_pcstack_ptr = (m_pcstack_ptr - 1) & 7;
	m_pcstack[m_pcstack_ptr] = pc;
}

uint16_t tms320c51_device::POP_STACK()
{
	uint16_t pc = m_pcstack[m_pcstack_ptr];
	m_pcstack_ptr = (m_pcstack_ptr + 1) & 7;
	m_pcstack[(m_pcstack_ptr + 7) & 7] = m_pcstack[(m_pcstack_ptr + 6) & 7];
	return pc;
}

int32_t tms320c51_device::SUB(uint32_t a, uint32_t b, bool shift16)
{
	uint32_t res = a - b;

	if (shift16)
	{
		// C is cleared if borrow was generated, otherwise unaffected
		if (a < res) m_st1.c = 0;
	}
	else
	{
		// C is cleared if borrow was generated
		m_st1.c = (a < res) ? 0 : 1;
	}

	// check overflow
	if ((a ^ b) & (a ^ res) & 0x80000000)
	{
		if (m_st0.ovm)  // overflow saturation mode
		{
			res = (int32_t(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		m_st0.ov = 1;
	}

	return (int32_t)(res);
}

int32_t tms320c51_device::ADD(uint32_t a, uint32_t b, bool shift16)
{
	uint32_t res = a + b;

	if (shift16)
	{
		// C is set if carry was generated, otherwise unaffected
		if (a > res) m_st1.c = 1;
	}
	else
	{
		// C is set if carry was generated
		m_st1.c = (a > res) ? 1 : 0;
	}

	// check overflow
	if ((a ^ res) & (b ^ res) & 0x80000000)
	{
		if (m_st0.ovm)  // overflow saturation mode
		{
			res = ((int32_t)(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		m_st0.ov = 1;
	}

	return (int32_t)(res);
}


void tms320c51_device::UPDATE_AR(int ar, int step)
{
	int cenb1 = (m_cbcr >> 3) & 0x1;
	int car1 = m_cbcr & 0x7;
	int cenb2 = (m_cbcr >> 7) & 0x1;
	int car2 = (m_cbcr >> 4) & 0x7;

	if (cenb1 && ar == car1)
	{
		// update circular buffer 1, note that it only checks ==
		if (m_ar[ar] == m_cber1)
		{
			m_ar[ar] = m_cbsr1;
		}
		else
		{
			m_ar[ar] += step;
		}
	}
	else if (cenb2 && ar == car2)
	{
		// update circular buffer 2, note that it only checks ==
		if (m_ar[ar] == m_cber2)
		{
			m_ar[ar] = m_cbsr2;
		}
		else
		{
			m_ar[ar] += step;
		}
	}
	else
	{
		m_ar[ar] += step;
	}
}

void tms320c51_device::UPDATE_ARP(int nar)
{
	m_st1.arb = m_st0.arp;
	m_st0.arp = nar;
}

uint16_t tms320c51_device::GET_ADDRESS()
{
	if (m_op & 0x80)        // Indirect Addressing
	{
		uint16_t ea;
		int arp = m_st0.arp;
		int nar = m_op & 0x7;

		ea = m_ar[arp];

		switch ((m_op >> 3) & 0xf)
		{
			case 0x0:   // *            (no operation)
			{
				break;
			}
			case 0x1:   // *, ARn       (NAR -> ARP)
			{
				UPDATE_ARP(nar);
				break;
			}
			case 0x2:   // *-           ((CurrentAR)-1 -> CurrentAR)
			{
				UPDATE_AR(arp, -1);
				break;
			}
			case 0x3:   // *-, ARn      ((CurrentAR)-1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, -1);
				UPDATE_ARP(nar);
				break;
			}
			case 0x4:   // *+           ((CurrentAR)+1 -> CurrentAR)
			{
				UPDATE_AR(arp, 1);
				break;
			}
			case 0x5:   // *+, ARn      ((CurrentAR)+1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, 1);
				UPDATE_ARP(nar);
				break;
			}
			case 0xa:   // *0-          ((CurrentAR) - INDX)
			{
				UPDATE_AR(arp, -m_indx);
				break;
			}
			case 0xb:   // *0-, ARn     ((CurrentAR) - INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, -m_indx);
				UPDATE_ARP(nar);
				break;
			}
			case 0xc:   // *0+          ((CurrentAR) + INDX -> CurrentAR)
			{
				UPDATE_AR(arp, m_indx);
				break;
			}
			case 0xd:   // *0+, ARn     ((CurrentAR) + INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, m_indx);
				UPDATE_ARP(nar);
				break;
			}

			default:    fatalerror("TMS320C5x: GET_ADDRESS: unimplemented indirect addressing mode %d at %04X (%04X)\n", (m_op >> 3) & 0xf, m_pc, m_op);
		}

		return ea;
	}
	else                    // Direct Addressing
	{
		return m_st0.dp | (m_op & 0x7f);
	}
}

bool tms320c51_device::GET_ZLVC_CONDITION(int zlvc, int zlvc_mask)
{
	if (zlvc_mask & 0x2)            // OV-bit
	{
		if ((zlvc & 0x2) && m_st0.ov == 0)          // OV
			return false;
		if (((zlvc & 0x2) == 0) && m_st0.ov != 0)   // NOV
			return false;
	}
	if (zlvc_mask & 0x1)            // C-bit
	{
		if ((zlvc & 0x1) && m_st1.c == 0)           // C
			return false;
		if (((zlvc & 0x1) == 0) && m_st1.c != 0)        // NC
			return false;
	}

	switch ((zlvc_mask & 0xc) | ((zlvc >> 2) & 0x3))
	{
		case 0x00: break;                                           // MZ=0, ML=0, Z=0, L=0
		case 0x01: break;                                           // MZ=0, ML=0, Z=0, L=1
		case 0x02: break;                                           // MZ=0, ML=0, Z=1, L=0
		case 0x03: break;                                           // MZ=0, ML=0, Z=1, L=1
		case 0x04: if ((int32_t)(m_acc) <= 0) return false; break;        // MZ=0, ML=1, Z=0, L=0  (GT)
		case 0x05: if ((int32_t)(m_acc) >= 0) return false; break;        // MZ=0, ML=1, Z=0, L=1  (LT)
		case 0x06: if ((int32_t)(m_acc) <= 0) return false; break;        // MZ=0, ML=1, Z=1, L=0  (GT)
		case 0x07: if ((int32_t)(m_acc) >= 0) return false; break;        // MZ=0, ML=1, Z=1, L=1  (LT)
		case 0x08: if ((int32_t)(m_acc) == 0) return false; break;        // MZ=1, ML=0, Z=0, L=0  (NEQ)
		case 0x09: if ((int32_t)(m_acc) == 0) return false; break;        // MZ=1, ML=0, Z=0, L=1  (NEQ)
		case 0x0a: if ((int32_t)(m_acc) != 0) return false; break;        // MZ=1, ML=0, Z=1, L=0  (EQ)
		case 0x0b: if ((int32_t)(m_acc) != 0) return false; break;        // MZ=1, ML=0, Z=1, L=1  (EQ)
		case 0x0c: if ((int32_t)(m_acc) <= 0) return false; break;        // MZ=1, ML=1, Z=0, L=0  (GT)
		case 0x0d: if ((int32_t)(m_acc) >= 0) return false; break;        // MZ=1, ML=1, Z=0, L=1  (LT)
		case 0x0e: if ((int32_t)(m_acc) < 0) return false;     break;     // MZ=1, ML=1, Z=1, L=0  (GEQ)
		case 0x0f: if ((int32_t)(m_acc) > 0) return false;     break;     // MZ=1, ML=1, Z=1, L=1  (LEQ)
	}
	return true;
}

bool tms320c51_device::GET_TP_CONDITION(int tp)
{
	switch (tp)
	{
		case 0:     // BIO pin low
			// TODO
			return false;

		case 1:     // TC == 1
			return m_st1.tc != 0;

		case 2:     // TC == 0
			return m_st1.tc == 0;

		case 3:
			return true;
	}
	return true;
}

int32_t tms320c51_device::PREG_PSCALER(int32_t preg)
{
	switch (m_st1.pm & 3)
	{
		case 0:     // No shift
		{
			return preg;
		}
		case 1:     // Left-shifted 1 bit, LSB zero-filled
		{
			return preg << 1;
		}
		case 2:     // Left-shifted 4 bits, 4 LSBs zero-filled
		{
			return preg << 4;
		}
		case 3:     // Right-shifted 6 bits, sign-extended, 6 LSBs lost
		{
			return (int32_t)(preg >> 6);
		}
	}
	return 0;
}



void tms320c51_device::op_invalid()
{
	fatalerror("TMS320C5x: invalid op at %08X\n", m_pc-1);
}

/*****************************************************************************/

void tms320c51_device::op_abs()
{
	fatalerror("TMS320C5x: unimplemented op abs at %08X\n", m_pc-1);
}

void tms320c51_device::op_adcb()
{
	fatalerror("TMS320C5x: unimplemented op adcb at %08X\n", m_pc-1);
}

void tms320c51_device::op_add_mem()
{
	int32_t d;
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);
	int shift = (m_op >> 8) & 0xf;

	if (m_st1.sxm)
	{
		d = (int32_t)(int16_t)(data) << shift;
	}
	else
	{
		d = (uint32_t)(uint16_t)(data) << shift;
	}

	m_acc = ADD(m_acc, d, false);

	CYCLES(1);
}

void tms320c51_device::op_add_simm()
{
	uint16_t imm = m_op & 0xff;

	m_acc = ADD(m_acc, imm, false);

	CYCLES(1);
}

void tms320c51_device::op_add_limm()
{
	int32_t d;
	uint16_t imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		d = (int32_t)(int16_t)(imm) << shift;
	}
	else
	{
		d = (uint32_t)(uint16_t)(imm) << shift;
	}

	m_acc = ADD(m_acc, d, false);

	CYCLES(2);
}

void tms320c51_device::op_add_s16_mem()
{
	uint16_t ea = GET_ADDRESS();
	uint32_t data = DM_READ16(ea) << 16;

	m_acc = ADD(m_acc, data, true);

	CYCLES(1);
}

void tms320c51_device::op_addb()
{
	m_acc = ADD(m_acc, m_accb, false);

	CYCLES(1);
}

void tms320c51_device::op_addc()
{
	fatalerror("TMS320C5x: unimplemented op addc at %08X\n", m_pc-1);
}

void tms320c51_device::op_adds()
{
	fatalerror("TMS320C5x: unimplemented op adds at %08X\n", m_pc-1);
}

void tms320c51_device::op_addt()
{
	fatalerror("TMS320C5x: unimplemented op addt at %08X\n", m_pc-1);
}

void tms320c51_device::op_and_mem()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_acc &= (uint32_t)(data);

	CYCLES(1);
}

void tms320c51_device::op_and_limm()
{
	uint32_t imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc &= imm << shift;

	CYCLES(2);
}

void tms320c51_device::op_and_s16_limm()
{
	fatalerror("TMS320C5x: unimplemented op and s16 limm at %08X\n", m_pc-1);
}

void tms320c51_device::op_andb()
{
	fatalerror("TMS320C5x: unimplemented op andb at %08X\n", m_pc-1);
}

void tms320c51_device::op_bsar()
{
	int shift = (m_op & 0xf) + 1;

	if (m_st1.sxm)
	{
		m_acc = (int32_t)(m_acc) >> shift;
	}
	else
	{
		m_acc = (uint32_t)(m_acc) >> shift;
	}

	CYCLES(1);
}

void tms320c51_device::op_cmpl()
{
	m_acc = ~(uint32_t)(m_acc);

	CYCLES(1);
}

void tms320c51_device::op_crgt()
{
	if (m_acc >= m_accb)
	{
		m_accb = m_acc;
		m_st1.c = 1;
	}
	else
	{
		m_acc = m_accb;
		m_st1.c = 0;
	}

	CYCLES(1);
}

void tms320c51_device::op_crlt()
{
	if (m_acc >= m_accb)
	{
		m_acc = m_accb;
		m_st1.c = 0;
	}
	else
	{
		m_accb = m_acc;
		m_st1.c = 1;
	}

	CYCLES(1);
}

void tms320c51_device::op_exar()
{
	int32_t tmp = m_acc;
	m_acc = m_accb;
	m_accb = tmp;

	CYCLES(1);
}

void tms320c51_device::op_lacb()
{
	m_acc = m_accb;

	CYCLES(1);
}

void tms320c51_device::op_lacc_mem()
{
	int shift = (m_op >> 8) & 0xf;
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	if (m_st1.sxm)
	{
		m_acc = (int32_t)(int16_t)(data) << shift;
	}
	else
	{
		m_acc = (uint32_t)(uint16_t)(data) << shift;
	}

	CYCLES(1);
}

void tms320c51_device::op_lacc_limm()
{
	uint16_t imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		m_acc = (int32_t)(int16_t)(imm) << shift;
	}
	else
	{
		m_acc = (uint32_t)(uint16_t)(imm) << shift;
	}

	CYCLES(1);
}

void tms320c51_device::op_lacc_s16_mem()
{
	uint16_t ea = GET_ADDRESS();
	m_acc = DM_READ16(ea) << 16;

	CYCLES(1);
}

void tms320c51_device::op_lacl_simm()
{
	m_acc = m_op & 0xff;

	CYCLES(1);
}

void tms320c51_device::op_lacl_mem()
{
	uint16_t ea = GET_ADDRESS();
	m_acc = DM_READ16(ea) & 0xffff;

	CYCLES(1);
}

void tms320c51_device::op_lact()
{
	fatalerror("TMS320C5x: unimplemented op lact at %08X\n", m_pc-1);
}

void tms320c51_device::op_lamm()
{
	uint16_t ea = GET_ADDRESS() & 0x7f;
	m_acc = DM_READ16(ea) & 0xffff;

	CYCLES(1);
}

void tms320c51_device::op_neg()
{
	if ((uint32_t)(m_acc) == 0x80000000)
	{
		m_st0.ov = 1;
		m_st1.c = 0;
		m_acc = (m_st0.ovm) ? 0x7fffffff : 0x80000000;
	}
	else
	{
		m_acc = 0 - (uint32_t)(m_acc);
		m_st1.c = (m_acc == 0) ? 1 : 0;
	}

	CYCLES(1);
}

void tms320c51_device::op_norm()
{
	fatalerror("TMS320C5x: unimplemented op norm at %08X\n", m_pc-1);
}

void tms320c51_device::op_or_mem()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_acc |= (uint32_t)(data);

	CYCLES(1);
}

void tms320c51_device::op_or_limm()
{
	uint32_t imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc |= imm << shift;

	CYCLES(1);
}

void tms320c51_device::op_or_s16_limm()
{
	fatalerror("TMS320C5x: unimplemented op or s16 limm at %08X\n", m_pc-1);
}

void tms320c51_device::op_orb()
{
	m_acc |= m_accb;

	CYCLES(1);
}

void tms320c51_device::op_rol()
{
	fatalerror("TMS320C5x: unimplemented op rol at %08X\n", m_pc-1);
}

void tms320c51_device::op_rolb()
{
	uint32_t acc = m_acc;
	uint32_t accb = m_accb;
	uint32_t c = m_st1.c & 1;

	m_acc = (acc << 1) | ((accb >> 31) & 1);
	m_accb = (accb << 1) | c;
	m_st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

void tms320c51_device::op_ror()
{
	fatalerror("TMS320C5x: unimplemented op ror at %08X\n", m_pc-1);
}

void tms320c51_device::op_rorb()
{
	fatalerror("TMS320C5x: unimplemented op rorb at %08X\n", m_pc-1);
}

void tms320c51_device::op_sacb()
{
	m_accb = m_acc;

	CYCLES(1);
}

void tms320c51_device::op_sach()
{
	uint16_t ea = GET_ADDRESS();
	int shift = (m_op >> 8) & 0x7;

	DM_WRITE16(ea, (uint16_t)((m_acc << shift) >> 16));
	CYCLES(1);
}

void tms320c51_device::op_sacl()
{
	uint16_t ea = GET_ADDRESS();
	int shift = (m_op >> 8) & 0x7;

	DM_WRITE16(ea, (uint16_t)(m_acc << shift));
	CYCLES(1);
}

void tms320c51_device::op_samm()
{
	uint16_t ea = GET_ADDRESS();
	ea &= 0x7f;

	DM_WRITE16(ea, (uint16_t)(m_acc));
	CYCLES(1);
}

void tms320c51_device::op_sath()
{
	fatalerror("TMS320C5x: unimplemented op sath at %08X\n", m_pc-1);
}

void tms320c51_device::op_satl()
{
	int count = m_treg1 & 0xf;
	if (m_st1.sxm)
	{
		m_acc = (int32_t)(m_acc) >> count;
	}
	else
	{
		m_acc = (uint32_t)(m_acc) >> count;
	}

	CYCLES(1);
}

void tms320c51_device::op_sbb()
{
	uint32_t res = m_acc - m_accb;

	// C is cleared if borrow was generated
	m_st1.c = ((uint32_t)(m_acc) < res) ? 0 : 1;

	m_acc = res;

	CYCLES(1);
}

void tms320c51_device::op_sbbb()
{
	fatalerror("TMS320C5x: unimplemented op sbbb at %08X\n", m_pc-1);
}

void tms320c51_device::op_sfl()
{
	m_st1.c = (m_acc >> 31) & 1;
	m_acc = m_acc << 1;

	CYCLES(1);
}

void tms320c51_device::op_sflb()
{
	uint32_t acc = m_acc;
	uint32_t accb = m_accb;

	m_acc = (acc << 1) | ((accb >> 31) & 1);
	m_accb = (accb << 1);
	m_st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

void tms320c51_device::op_sfr()
{
	m_st1.c = m_acc & 1;

	if (m_st1.sxm)
	{
		m_acc = (int32_t)(m_acc) >> 1;
	}
	else
	{
		m_acc = (uint32_t)(m_acc) >> 1;
	}

	CYCLES(1);
}

void tms320c51_device::op_sfrb()
{
	fatalerror("TMS320C5x: unimplemented op sfrb at %08X\n", m_pc-1);
}

void tms320c51_device::op_sub_mem()
{
	int32_t d;
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);
	int shift = (m_op >> 8) & 0xf;

	if (m_st1.sxm)
	{
		d = (int32_t)(int16_t)(data) << shift;
	}
	else
	{
		d = (uint32_t)(uint16_t)(data) << shift;
	}

	m_acc = SUB(m_acc, d, false);

	CYCLES(1);
}

void tms320c51_device::op_sub_s16_mem()
{
	fatalerror("TMS320C5x: unimplemented op sub s16 mem at %08X\n", m_pc-1);
}

void tms320c51_device::op_sub_simm()
{
	uint16_t imm = m_op & 0xff;

	m_acc = SUB(m_acc, imm, false);

	CYCLES(1);
}

void tms320c51_device::op_sub_limm()
{
	int32_t d;
	uint16_t imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		d = (int32_t)(int16_t)(imm) << shift;
	}
	else
	{
		d = (uint32_t)(uint16_t)(imm) << shift;
	}

	m_acc = SUB(m_acc, d, false);

	CYCLES(2);
}

void tms320c51_device::op_subb()
{
	fatalerror("TMS320C5x: unimplemented op subb at %08X\n", m_pc-1);
}

void tms320c51_device::op_subc()
{
	fatalerror("TMS320C5x: unimplemented op subc at %08X\n", m_pc-1);
}

void tms320c51_device::op_subs()
{
	fatalerror("TMS320C5x: unimplemented op subs at %08X\n", m_pc-1);
}

void tms320c51_device::op_subt()
{
	fatalerror("TMS320C5x: unimplemented op subt at %08X\n", m_pc-1);
}

void tms320c51_device::op_xor_mem()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_acc ^= (uint32_t)(data);

	CYCLES(1);
}

void tms320c51_device::op_xor_limm()
{
	uint32_t imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc ^= imm << shift;

	CYCLES(1);
}

void tms320c51_device::op_xor_s16_limm()
{
	fatalerror("TMS320C5x: unimplemented op xor s16 limm at %08X\n", m_pc-1);
}

void tms320c51_device::op_xorb()
{
	fatalerror("TMS320C5x: unimplemented op xorb at %08X\n", m_pc-1);
}

void tms320c51_device::op_zalr()
{
	fatalerror("TMS320C5x: unimplemented op zalr at %08X\n", m_pc-1);
}

void tms320c51_device::op_zap()
{
	m_acc = 0;
	m_preg = 0;

	CYCLES(1);
}

/*****************************************************************************/

void tms320c51_device::op_adrk()
{
	uint16_t imm = m_op & 0xff;
	UPDATE_AR(m_st0.arp, imm);

	CYCLES(1);
}

void tms320c51_device::op_cmpr()
{
	m_st1.tc = 0;

	switch (m_op & 0x3)
	{
		case 0:         // (CurrentAR) == ARCR
		{
			if (m_ar[m_st0.arp] == m_arcr)
			{
				m_st1.tc = 1;
			}
			break;
		}
		case 1:         // (CurrentAR) < ARCR
		{
			if (m_ar[m_st0.arp] < m_arcr)
			{
				m_st1.tc = 1;
			}
			break;
		}
		case 2:         // (CurrentAR) > ARCR
		{
			if (m_ar[m_st0.arp] > m_arcr)
			{
				m_st1.tc = 1;
			}
			break;
		}
		case 3:         // (CurrentAR) != ARCR
		{
			if (m_ar[m_st0.arp] != m_arcr)
			{
				m_st1.tc = 1;
			}
			break;
		}
	}

	CYCLES(1);
}

void tms320c51_device::op_lar_mem()
{
	int arx = (m_op >> 8) & 0x7;
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_ar[arx] = data;

	CYCLES(2);
}

void tms320c51_device::op_lar_simm()
{
	int arx = (m_op >> 8) & 0x7;
	m_ar[arx] = m_op & 0xff;

	CYCLES(2);
}

void tms320c51_device::op_lar_limm()
{
	int arx = m_op & 0x7;
	uint16_t imm = ROPCODE();
	m_ar[arx] = imm;

	CYCLES(2);
}

void tms320c51_device::op_ldp_mem()
{
	fatalerror("TMS320C5x: unimplemented op ldp mem at %08X\n", m_pc-1);
}

void tms320c51_device::op_ldp_imm()
{
	m_st0.dp = (m_op & 0x1ff) << 7;
	CYCLES(2);
}

void tms320c51_device::op_mar()
{
	// direct addressing is NOP
	if (m_op & 0x80)
	{
		GET_ADDRESS();
	}
	CYCLES(1);
}

void tms320c51_device::op_sar()
{
	int arx = (m_op >> 8) & 0x7;
	uint16_t ar = m_ar[arx];
	uint16_t ea = GET_ADDRESS();
	DM_WRITE16(ea, ar);

	CYCLES(1);
}

void tms320c51_device::op_sbrk()
{
	uint16_t imm = m_op & 0xff;
	UPDATE_AR(m_st0.arp, -imm);

	CYCLES(1);
}

/*****************************************************************************/

void tms320c51_device::op_b()
{
	uint16_t pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP

	CHANGE_PC(pma);
	CYCLES(4);
}

void tms320c51_device::op_bacc()
{
	CHANGE_PC((uint16_t)(m_acc));

	CYCLES(4);
}

void tms320c51_device::op_baccd()
{
	uint16_t pc = (uint16_t)(m_acc);

	delay_slot(m_pc);
	CHANGE_PC(pc);

	CYCLES(2);
}

void tms320c51_device::op_banz()
{
	uint16_t pma = ROPCODE();

	if (m_ar[m_st0.arp] != 0)
	{
		CHANGE_PC(pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}

	GET_ADDRESS();      // modify AR/ARP
}

void tms320c51_device::op_banzd()
{
	fatalerror("TMS320C5x: unimplemented op banzd at %08X\n", m_pc-1);
}

void tms320c51_device::op_bcnd()
{
	uint16_t pma = ROPCODE();

	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		CHANGE_PC(pma);
		CYCLES(4);

		// clear overflow
		if (m_op & 0x2)
			m_st0.ov = 0;
	}
	else
	{
		CYCLES(2);
	}
}

void tms320c51_device::op_bcndd()
{
	uint16_t pma = ROPCODE();

	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		// clear overflow
		if (m_op & 0x2)
			m_st0.ov = 0;

		delay_slot(m_pc);
		CHANGE_PC(pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

void tms320c51_device::op_bd()
{
	uint16_t pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP

	delay_slot(m_pc);
	CHANGE_PC(pma);
	CYCLES(2);
}

void tms320c51_device::op_cala()
{
	PUSH_STACK(m_pc);

	CHANGE_PC(m_acc);

	CYCLES(4);
}

void tms320c51_device::op_calad()
{
	uint16_t pma = m_acc;
	PUSH_STACK(m_pc+2);

	delay_slot(m_pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

void tms320c51_device::op_call()
{
	uint16_t pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP
	PUSH_STACK(m_pc);

	CHANGE_PC(pma);

	CYCLES(4);
}

void tms320c51_device::op_calld()
{
	uint16_t pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP
	PUSH_STACK(m_pc+2);

	delay_slot(m_pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

void tms320c51_device::op_cc()
{
	uint16_t pma = ROPCODE();

	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		PUSH_STACK(m_pc);

		CHANGE_PC(pma);
		CYCLES(4);

		// clear overflow
		if (m_op & 0x2)
			m_st0.ov = 0;
	}
	else
	{
		CYCLES(2);
	}
}

void tms320c51_device::op_ccd()
{
	uint16_t pma = ROPCODE();

	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		PUSH_STACK(m_pc+2);

		// clear overflow
		if (m_op & 0x2)
			m_st0.ov = 0;

		delay_slot(m_pc);
		CHANGE_PC(pma);
	}

	CYCLES(2);
}

void tms320c51_device::op_intr()
{
	fatalerror("TMS320C5x: unimplemented op intr at %08X\n", m_pc-1);
}

void tms320c51_device::op_nmi()
{
	fatalerror("TMS320C5x: unimplemented op nmi at %08X\n", m_pc-1);
}

void tms320c51_device::op_retc()
{
	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		uint16_t pc = POP_STACK();
		CHANGE_PC(pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

void tms320c51_device::op_retcd()
{
	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		uint16_t pc = POP_STACK();
		delay_slot(m_pc);
		CHANGE_PC(pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

void tms320c51_device::op_rete()
{
	uint16_t pc = POP_STACK();
	CHANGE_PC(pc);

	restore_interrupt_context();

	m_st0.intm = 0;

	CYCLES(4);
}

void tms320c51_device::op_reti()
{
	fatalerror("TMS320C5x: unimplemented op reti at %08X\n", m_pc-1);
}

void tms320c51_device::op_trap()
{
	fatalerror("TMS320C5x: unimplemented op trap at %08X\n", m_pc-1);
}

void tms320c51_device::op_xc()
{
	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		CYCLES(1);
	}
	else
	{
		int n = ((m_op >> 12) & 0x1) + 1;
		CHANGE_PC(m_pc + n);
		CYCLES(1 + n);
	}
}

/*****************************************************************************/

void tms320c51_device::op_bldd_slimm()
{
	uint16_t pfc = ROPCODE();

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_bldd_dlimm()
{
	uint16_t pfc = ROPCODE();

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_bldd_sbmar()
{
	uint16_t pfc = m_bmar;

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_bldd_dbmar()
{
	uint16_t pfc = m_bmar;

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_bldp()
{
	uint16_t pfc = m_bmar;

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(1);

		m_rptc--;
	};
}

void tms320c51_device::op_blpd_bmar()
{
	fatalerror("TMS320C5x: unimplemented op bpld bmar at %08X\n", m_pc-1);
}

void tms320c51_device::op_blpd_imm()
{
	uint16_t pfc = ROPCODE();

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

/*****************************************************************************/

void tms320c51_device::op_dmov()
{
	fatalerror("TMS320C5x: unimplemented op dmov at %08X\n", m_pc-1);
}

void tms320c51_device::op_in()
{
	fatalerror("TMS320C5x: unimplemented op in at %08X\n", m_pc-1);
}

void tms320c51_device::op_lmmr()
{
	uint16_t pfc = ROPCODE();

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(pfc);
		DM_WRITE16(ea & 0x7f, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_out()
{
	uint16_t port = ROPCODE();
	uint16_t ea = GET_ADDRESS();

	uint16_t data = DM_READ16(ea);
	m_io.write_word(port << 1, data);

	// TODO: handle repeat
	CYCLES(3);
}

void tms320c51_device::op_smmr()
{
	uint16_t pfc = ROPCODE();

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(ea & 0x7f);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_tblr()
{
	uint16_t pfc = (uint16_t)(m_acc);

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms320c51_device::op_tblw()
{
	uint16_t pfc = (uint16_t)(m_acc);

	while (m_rptc > -1)
	{
		uint16_t ea = GET_ADDRESS();
		uint16_t data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

/*****************************************************************************/

void tms320c51_device::op_apl_dbmr()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	data &= m_dbmr;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms320c51_device::op_apl_imm()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t imm = ROPCODE();
	uint16_t data = DM_READ16(ea);

	data &= imm;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms320c51_device::op_cpl_dbmr()
{
	fatalerror("TMS320C5x: unimplemented op cpl dbmr at %08X\n", m_pc-1);
}

void tms320c51_device::op_cpl_imm()
{
	uint16_t imm = ROPCODE();
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_st1.tc = (data == imm) ? 1 : 0;

	CYCLES(1);
}

void tms320c51_device::op_opl_dbmr()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);
	data |= m_dbmr;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms320c51_device::op_opl_imm()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t imm = ROPCODE();
	uint16_t data = DM_READ16(ea);
	data |= imm;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms320c51_device::op_splk()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t imm = ROPCODE();

	DM_WRITE16(ea, imm);

	CYCLES(2);
}

void tms320c51_device::op_xpl_dbmr()
{
	fatalerror("TMS320C5x: unimplemented op xpl dbmr at %08X\n", m_pc-1);
}

void tms320c51_device::op_xpl_imm()
{
	fatalerror("TMS320C5x: unimplemented op xpl imm at %08X\n", m_pc-1);
}

void tms320c51_device::op_apac()
{
	int32_t spreg = PREG_PSCALER(m_preg);
	m_acc = ADD(m_acc, spreg, false);

	CYCLES(1);
}

void tms320c51_device::op_lph()
{
	fatalerror("TMS320C5x: unimplemented op lph at %08X\n", m_pc-1);
}

void tms320c51_device::op_lt()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_treg0 = data;
	if (m_pmst.trm == 0)
	{
		m_treg1 = data;
		m_treg2 = data;
	}

	CYCLES(1);
}

void tms320c51_device::op_lta()
{
	int32_t spreg;
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_treg0 = data;
	spreg = PREG_PSCALER(m_preg);
	m_acc = ADD(m_acc, spreg, false);
	if (m_pmst.trm == 0)
	{
		m_treg1 = data;
		m_treg2 = data;
	}

	CYCLES(1);
}

void tms320c51_device::op_ltd()
{
	fatalerror("TMS320C5x: unimplemented op ltd at %08X\n", m_pc-1);
}

void tms320c51_device::op_ltp()
{
	fatalerror("TMS320C5x: unimplemented op ltp at %08X\n", m_pc-1);
}

void tms320c51_device::op_lts()
{
	fatalerror("TMS320C5x: unimplemented op lts at %08X\n", m_pc-1);
}

void tms320c51_device::op_mac()
{
	fatalerror("TMS320C5x: unimplemented op mac at %08X\n", m_pc-1);
}

void tms320c51_device::op_macd()
{
	fatalerror("TMS320C5x: unimplemented op macd at %08X\n", m_pc-1);
}

void tms320c51_device::op_madd()
{
	fatalerror("TMS320C5x: unimplemented op madd at %08X\n", m_pc-1);
}

void tms320c51_device::op_mads()
{
	fatalerror("TMS320C5x: unimplemented op mads at %08X\n", m_pc-1);
}

void tms320c51_device::op_mpy_mem()
{
	uint16_t ea = GET_ADDRESS();
	int16_t data = DM_READ16(ea);

	m_preg = (int32_t)(data) * (int32_t)(int16_t)(m_treg0);

	CYCLES(1);
}

void tms320c51_device::op_mpy_simm()
{
	fatalerror("TMS320C5x: unimplemented op mpy simm at %08X\n", m_pc-1);
}

void tms320c51_device::op_mpy_limm()
{
	fatalerror("TMS320C5x: unimplemented op mpy limm at %08X\n", m_pc-1);
}

void tms320c51_device::op_mpya()
{
	fatalerror("TMS320C5x: unimplemented op mpya at %08X\n", m_pc-1);
}

void tms320c51_device::op_mpys()
{
	fatalerror("TMS320C5x: unimplemented op mpys at %08X\n", m_pc-1);
}

void tms320c51_device::op_mpyu()
{
	fatalerror("TMS320C5x: unimplemented op mpyu at %08X\n", m_pc-1);
}

void tms320c51_device::op_pac()
{
	fatalerror("TMS320C5x: unimplemented op pac at %08X\n", m_pc-1);
}

void tms320c51_device::op_spac()
{
	fatalerror("TMS320C5x: unimplemented op spac at %08X\n", m_pc-1);
}

void tms320c51_device::op_sph()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t spreg = (uint16_t)(PREG_PSCALER(m_preg) >> 16);
	DM_WRITE16(ea, spreg);

	CYCLES(1);
}

void tms320c51_device::op_spl()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t spreg = (uint16_t)(PREG_PSCALER(m_preg));
	DM_WRITE16(ea, spreg);

	CYCLES(1);
}

void tms320c51_device::op_spm()
{
	m_st1.pm = m_op & 0x3;

	CYCLES(1);
}

void tms320c51_device::op_sqra()
{
	fatalerror("TMS320C5x: unimplemented op sqra at %08X\n", m_pc-1);
}

void tms320c51_device::op_sqrs()
{
	fatalerror("TMS320C5x: unimplemented op sqrs at %08X\n", m_pc-1);
}

void tms320c51_device::op_zpr()
{
	fatalerror("TMS320C5x: unimplemented op zpr at %08X\n", m_pc-1);
}

void tms320c51_device::op_bit()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_st1.tc = (data >> (~m_op >> 8 & 0xf)) & 1;

	CYCLES(1);
}

void tms320c51_device::op_bitt()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);

	m_st1.tc = (data >> (~m_treg2 & 0xf)) & 1;

	CYCLES(1);
}

void tms320c51_device::op_clrc_ov()
{
	m_st0.ovm = 0;

	CYCLES(1);
}

void tms320c51_device::op_clrc_ext()
{
	m_st1.sxm = 0;

	CYCLES(1);
}

void tms320c51_device::op_clrc_hold()
{
	fatalerror("TMS320C5x: unimplemented op clrc hold at %08X\n", m_pc-1);
}

void tms320c51_device::op_clrc_tc()
{
	fatalerror("TMS320C5x: unimplemented op clrc tc at %08X\n", m_pc-1);
}

void tms320c51_device::op_clrc_carry()
{
	fatalerror("TMS320C5x: unimplemented op clrc carry at %08X\n", m_pc-1);
}

void tms320c51_device::op_clrc_cnf()
{
	m_st1.cnf = 0;

	CYCLES(1);
}

void tms320c51_device::op_clrc_intm()
{
	m_st0.intm = 0;

	check_interrupts();

	CYCLES(1);
}

void tms320c51_device::op_clrc_xf()
{
	m_st1.xf = 0;

	CYCLES(1);
}

void tms320c51_device::op_idle()
{
	m_idle = true;
}

void tms320c51_device::op_idle2()
{
	fatalerror("TMS320C5x: unimplemented op idle2 at %08X\n", m_pc-1);
}

void tms320c51_device::op_lst_st0()
{
	fatalerror("TMS320C5x: unimplemented op lst st0 at %08X\n", m_pc-1);
}

void tms320c51_device::op_lst_st1()
{
	fatalerror("TMS320C5x: unimplemented op lst st1 at %08X\n", m_pc-1);
}

void tms320c51_device::op_pop()
{
	m_acc = POP_STACK();

	CYCLES(1);
}

void tms320c51_device::op_popd()
{
	fatalerror("TMS320C5x: unimplemented op popd at %08X\n", m_pc-1);
}

void tms320c51_device::op_pshd()
{
	fatalerror("TMS320C5x: unimplemented op pshd at %08X\n", m_pc-1);
}

void tms320c51_device::op_push()
{
	fatalerror("TMS320C5x: unimplemented op push at %08X\n", m_pc-1);
}

void tms320c51_device::op_rpt_mem()
{
	uint16_t ea = GET_ADDRESS();
	uint16_t data = DM_READ16(ea);
	m_rptc = data;
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(1);
}

void tms320c51_device::op_rpt_limm()
{
	m_rptc = (uint16_t)ROPCODE();
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(2);
}

void tms320c51_device::op_rpt_simm()
{
	m_rptc = (m_op & 0xff);
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(1);
}

void tms320c51_device::op_rptb()
{
	uint16_t pma = ROPCODE();
	m_pmst.braf = 1;
	m_pasr = m_pc;
	m_paer = pma + 1;

	CYCLES(2);
}

void tms320c51_device::op_rptz()
{
	fatalerror("TMS320C5x: unimplemented op rptz at %08X\n", m_pc-1);
}

void tms320c51_device::op_setc_ov()
{
	m_st0.ovm = 1;

	CYCLES(1);
}

void tms320c51_device::op_setc_ext()
{
	m_st1.sxm = 1;

	CYCLES(1);
}

void tms320c51_device::op_setc_hold()
{
	fatalerror("TMS320C5x: unimplemented op setc hold at %08X\n", m_pc-1);
}

void tms320c51_device::op_setc_tc()
{
	m_st1.tc = 1;

	CYCLES(1);
}

void tms320c51_device::op_setc_carry()
{
	fatalerror("TMS320C5x: unimplemented op setc carry at %08X\n", m_pc-1);
}

void tms320c51_device::op_setc_xf()
{
	m_st1.xf = 1;

	CYCLES(1);
}

void tms320c51_device::op_setc_cnf()
{
	m_st1.cnf = 1;

	CYCLES(1);
}

void tms320c51_device::op_setc_intm()
{
	m_st0.intm = 1;

	check_interrupts();

	CYCLES(1);
}

void tms320c51_device::op_sst_st0()
{
	fatalerror("TMS320C5x: unimplemented op sst st0 at %08X\n", m_pc-1);
}

void tms320c51_device::op_sst_st1()
{
	fatalerror("TMS320C5x: unimplemented op sst st1 at %08X\n", m_pc-1);
}
