// license:BSD-3-Clause
// copyright-holders:Ville Linde
// stack is LIFO and is 8 levels deep, there is no stackpointer on the real chip
void tms32051_device::PUSH_STACK(UINT16 pc)
{
	m_pcstack_ptr = (m_pcstack_ptr - 1) & 7;
	m_pcstack[m_pcstack_ptr] = pc;
}

UINT16 tms32051_device::POP_STACK()
{
	UINT16 pc = m_pcstack[m_pcstack_ptr];
	m_pcstack_ptr = (m_pcstack_ptr + 1) & 7;
	m_pcstack[(m_pcstack_ptr + 7) & 7] = m_pcstack[(m_pcstack_ptr + 6) & 7];
	return pc;
}

INT32 tms32051_device::SUB(UINT32 a, UINT32 b, bool shift16)
{
	UINT32 res = a - b;

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
			res = ((INT32)(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		m_st0.ov = 1;
	}

	return (INT32)(res);
}

INT32 tms32051_device::ADD(UINT32 a, UINT32 b, bool shift16)
{
	UINT32 res = a + b;

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
			res = ((INT32)(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		m_st0.ov = 1;
	}

	return (INT32)(res);
}


void tms32051_device::UPDATE_AR(int ar, int step)
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

void tms32051_device::UPDATE_ARP(int nar)
{
	m_st1.arb = m_st0.arp;
	m_st0.arp = nar;
}

UINT16 tms32051_device::GET_ADDRESS()
{
	if (m_op & 0x80)        // Indirect Addressing
	{
		UINT16 ea;
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

			default:    fatalerror("32051: GET_ADDRESS: unimplemented indirect addressing mode %d at %04X (%04X)\n", (m_op >> 3) & 0xf, m_pc, m_op);
		}

		return ea;
	}
	else                    // Direct Addressing
	{
		return m_st0.dp | (m_op & 0x7f);
	}
}

bool tms32051_device::GET_ZLVC_CONDITION(int zlvc, int zlvc_mask)
{
	if (zlvc_mask & 0x2)			// OV-bit
	{
		if ((zlvc & 0x2) && m_st0.ov == 0)			// OV
			return false;
		if (((zlvc & 0x2) == 0) && m_st0.ov != 0)	// NOV
			return false;
	}
	if (zlvc_mask & 0x1)			// C-bit
	{
		if ((zlvc & 0x1) && m_st1.c == 0)			// C
			return false;
		if (((zlvc & 0x1) == 0) && m_st1.c != 0)		// NC
			return false;
	}

	switch ((zlvc_mask & 0xc) | ((zlvc >> 2) & 0x3))
	{
		case 0x00: break;											// MZ=0, ML=0, Z=0, L=0
		case 0x01: break;											// MZ=0, ML=0, Z=0, L=1
		case 0x02: break;											// MZ=0, ML=0, Z=1, L=0
		case 0x03: break;											// MZ=0, ML=0, Z=1, L=1
		case 0x04: if ((INT32)(m_acc) <= 0) return false; break;		// MZ=0, ML=1, Z=0, L=0  (GT)
		case 0x05: if ((INT32)(m_acc) >= 0) return false; break;		// MZ=0, ML=1, Z=0, L=1  (LT)
		case 0x06: if ((INT32)(m_acc) <= 0) return false; break;		// MZ=0, ML=1, Z=1, L=0  (GT)
		case 0x07: if ((INT32)(m_acc) >= 0) return false; break;		// MZ=0, ML=1, Z=1, L=1  (LT)
		case 0x08: if ((INT32)(m_acc) == 0) return false; break;		// MZ=1, ML=0, Z=0, L=0  (NEQ)
		case 0x09: if ((INT32)(m_acc) == 0) return false; break;		// MZ=1, ML=0, Z=0, L=1  (NEQ)
		case 0x0a: if ((INT32)(m_acc) != 0) return false; break;		// MZ=1, ML=0, Z=1, L=0  (EQ)
		case 0x0b: if ((INT32)(m_acc) != 0) return false; break;		// MZ=1, ML=0, Z=1, L=1  (EQ)
		case 0x0c: if ((INT32)(m_acc) <= 0) return false; break;		// MZ=1, ML=1, Z=0, L=0  (GT)
		case 0x0d: if ((INT32)(m_acc) >= 0) return false; break;		// MZ=1, ML=1, Z=0, L=1  (LT)
		case 0x0e: if ((INT32)(m_acc) < 0) return false;	 break;		// MZ=1, ML=1, Z=1, L=0  (GEQ)
		case 0x0f: if ((INT32)(m_acc) > 0) return false;	 break;		// MZ=1, ML=1, Z=1, L=1  (LEQ)
	}
	return true;
}

bool tms32051_device::GET_TP_CONDITION(int tp)
{
	switch (tp)
	{
		case 0:		// BIO pin low
			// TODO
			return false;
		
		case 1:		// TC == 1
			return m_st1.tc != 0;

		case 2:		// TC == 0
			return m_st1.tc == 0;

		case 3:
			return true;
	}
	return true;
}

INT32 tms32051_device::PREG_PSCALER(INT32 preg)
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
			return (INT32)(preg >> 6);
		}
	}
	return 0;
}



void tms32051_device::op_invalid()
{
	fatalerror("32051: invalid op at %08X\n", m_pc-1);
}

/*****************************************************************************/

void tms32051_device::op_abs()
{
	fatalerror("32051: unimplemented op abs at %08X\n", m_pc-1);
}

void tms32051_device::op_adcb()
{
	fatalerror("32051: unimplemented op adcb at %08X\n", m_pc-1);
}

void tms32051_device::op_add_mem()
{
	INT32 d;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int shift = (m_op >> 8) & 0xf;

	if (m_st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	m_acc = ADD(m_acc, d, false);

	CYCLES(1);
}

void tms32051_device::op_add_simm()
{
	UINT16 imm = m_op & 0xff;

	m_acc = ADD(m_acc, imm, false);

	CYCLES(1);
}

void tms32051_device::op_add_limm()
{
	INT32 d;
	UINT16 imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	m_acc = ADD(m_acc, d, false);

	CYCLES(2);
}

void tms32051_device::op_add_s16_mem()
{
	UINT16 ea = GET_ADDRESS();
	UINT32 data = DM_READ16(ea) << 16;

	m_acc = ADD(m_acc, data, true);

	CYCLES(1);
}

void tms32051_device::op_addb()
{
	m_acc = ADD(m_acc, m_accb, false);

	CYCLES(1);
}

void tms32051_device::op_addc()
{
	fatalerror("32051: unimplemented op addc at %08X\n", m_pc-1);
}

void tms32051_device::op_adds()
{
	fatalerror("32051: unimplemented op adds at %08X\n", m_pc-1);
}

void tms32051_device::op_addt()
{
	fatalerror("32051: unimplemented op addt at %08X\n", m_pc-1);
}

void tms32051_device::op_and_mem()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_acc &= (UINT32)(data);

	CYCLES(1);
}

void tms32051_device::op_and_limm()
{
	UINT32 imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc &= imm << shift;

	CYCLES(2);
}

void tms32051_device::op_and_s16_limm()
{
	fatalerror("32051: unimplemented op and s16 limm at %08X\n", m_pc-1);
}

void tms32051_device::op_andb()
{
	fatalerror("32051: unimplemented op andb at %08X\n", m_pc-1);
}

void tms32051_device::op_bsar()
{
	int shift = (m_op & 0xf) + 1;

	if (m_st1.sxm)
	{
		m_acc = (INT32)(m_acc) >> shift;
	}
	else
	{
		m_acc = (UINT32)(m_acc) >> shift;
	}

	CYCLES(1);
}

void tms32051_device::op_cmpl()
{
	m_acc = ~(UINT32)(m_acc);

	CYCLES(1);
}

void tms32051_device::op_crgt()
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

void tms32051_device::op_crlt()
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

void tms32051_device::op_exar()
{
	INT32 tmp = m_acc;
	m_acc = m_accb;
	m_accb = tmp;

	CYCLES(1);
}

void tms32051_device::op_lacb()
{
	m_acc = m_accb;

	CYCLES(1);
}

void tms32051_device::op_lacc_mem()
{
	int shift = (m_op >> 8) & 0xf;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	if (m_st1.sxm)
	{
		m_acc = (INT32)(INT16)(data) << shift;
	}
	else
	{
		m_acc = (UINT32)(UINT16)(data) << shift;
	}

	CYCLES(1);
}

void tms32051_device::op_lacc_limm()
{
	UINT16 imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		m_acc = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		m_acc = (UINT32)(UINT16)(imm) << shift;
	}

	CYCLES(1);
}

void tms32051_device::op_lacc_s16_mem()
{
	UINT16 ea = GET_ADDRESS();
	m_acc = DM_READ16(ea) << 16;

	CYCLES(1);
}

void tms32051_device::op_lacl_simm()
{
	m_acc = m_op & 0xff;

	CYCLES(1);
}

void tms32051_device::op_lacl_mem()
{
	UINT16 ea = GET_ADDRESS();
	m_acc = DM_READ16(ea) & 0xffff;

	CYCLES(1);
}

void tms32051_device::op_lact()
{
	fatalerror("32051: unimplemented op lact at %08X\n", m_pc-1);
}

void tms32051_device::op_lamm()
{
	UINT16 ea = GET_ADDRESS() & 0x7f;
	m_acc = DM_READ16(ea) & 0xffff;

	CYCLES(1);
}

void tms32051_device::op_neg()
{
	if ((UINT32)(m_acc) == 0x80000000)
	{
		m_st0.ov = 1;
		m_st1.c = 0;
		m_acc = (m_st0.ovm) ? 0x7fffffff : 0x80000000;
	}
	else
	{
		m_acc = 0 - (UINT32)(m_acc);
		m_st1.c = (m_acc == 0) ? 1 : 0;
	}

	CYCLES(1);
}

void tms32051_device::op_norm()
{
	fatalerror("32051: unimplemented op norm at %08X\n", m_pc-1);
}

void tms32051_device::op_or_mem()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_acc |= (UINT32)(data);

	CYCLES(1);
}

void tms32051_device::op_or_limm()
{
	UINT32 imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc |= imm << shift;

	CYCLES(1);
}

void tms32051_device::op_or_s16_limm()
{
	fatalerror("32051: unimplemented op or s16 limm at %08X\n", m_pc-1);
}

void tms32051_device::op_orb()
{
	m_acc |= m_accb;

	CYCLES(1);
}

void tms32051_device::op_rol()
{
	fatalerror("32051: unimplemented op rol at %08X\n", m_pc-1);
}

void tms32051_device::op_rolb()
{
	UINT32 acc = m_acc;
	UINT32 accb = m_accb;
	UINT32 c = m_st1.c & 1;

	m_acc = (acc << 1) | ((accb >> 31) & 1);
	m_accb = (accb << 1) | c;
	m_st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

void tms32051_device::op_ror()
{
	fatalerror("32051: unimplemented op ror at %08X\n", m_pc-1);
}

void tms32051_device::op_rorb()
{
	fatalerror("32051: unimplemented op rorb at %08X\n", m_pc-1);
}

void tms32051_device::op_sacb()
{
	m_accb = m_acc;

	CYCLES(1);
}

void tms32051_device::op_sach()
{
	UINT16 ea = GET_ADDRESS();
	int shift = (m_op >> 8) & 0x7;

	DM_WRITE16(ea, (UINT16)((m_acc << shift) >> 16));
	CYCLES(1);
}

void tms32051_device::op_sacl()
{
	UINT16 ea = GET_ADDRESS();
	int shift = (m_op >> 8) & 0x7;

	DM_WRITE16(ea, (UINT16)(m_acc << shift));
	CYCLES(1);
}

void tms32051_device::op_samm()
{
	UINT16 ea = GET_ADDRESS();
	ea &= 0x7f;

	DM_WRITE16(ea, (UINT16)(m_acc));
	CYCLES(1);
}

void tms32051_device::op_sath()
{
	fatalerror("32051: unimplemented op sath at %08X\n", m_pc-1);
}

void tms32051_device::op_satl()
{
	int count = m_treg1 & 0xf;
	if (m_st1.sxm)
	{
		m_acc = (INT32)(m_acc) >> count;
	}
	else
	{
		m_acc = (UINT32)(m_acc) >> count;
	}

	CYCLES(1);
}

void tms32051_device::op_sbb()
{
	UINT32 res = m_acc - m_accb;

	// C is cleared if borrow was generated
	m_st1.c = ((UINT32)(m_acc) < res) ? 0 : 1;

	m_acc = res;

	CYCLES(1);
}

void tms32051_device::op_sbbb()
{
	fatalerror("32051: unimplemented op sbbb at %08X\n", m_pc-1);
}

void tms32051_device::op_sfl()
{
	m_st1.c = (m_acc >> 31) & 1;
	m_acc = m_acc << 1;

	CYCLES(1);
}

void tms32051_device::op_sflb()
{
	UINT32 acc = m_acc;
	UINT32 accb = m_accb;

	m_acc = (acc << 1) | ((accb >> 31) & 1);
	m_accb = (accb << 1);
	m_st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

void tms32051_device::op_sfr()
{
	m_st1.c = m_acc & 1;

	if (m_st1.sxm)
	{
		m_acc = (INT32)(m_acc) >> 1;
	}
	else
	{
		m_acc = (UINT32)(m_acc) >> 1;
	}

	CYCLES(1);
}

void tms32051_device::op_sfrb()
{
	fatalerror("32051: unimplemented op sfrb at %08X\n", m_pc-1);
}

void tms32051_device::op_sub_mem()
{
	INT32 d;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int shift = (m_op >> 8) & 0xf;

	if (m_st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	m_acc = SUB(m_acc, d, false);

	CYCLES(1);
}

void tms32051_device::op_sub_s16_mem()
{
	fatalerror("32051: unimplemented op sub s16 mem at %08X\n", m_pc-1);
}

void tms32051_device::op_sub_simm()
{
	UINT16 imm = m_op & 0xff;

	m_acc = SUB(m_acc, imm, false);

	CYCLES(1);
}

void tms32051_device::op_sub_limm()
{
	INT32 d;
	UINT16 imm = ROPCODE();
	int shift = m_op & 0xf;

	if (m_st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	m_acc = SUB(m_acc, d, false);

	CYCLES(2);
}

void tms32051_device::op_subb()
{
	fatalerror("32051: unimplemented op subb at %08X\n", m_pc-1);
}

void tms32051_device::op_subc()
{
	fatalerror("32051: unimplemented op subc at %08X\n", m_pc-1);
}

void tms32051_device::op_subs()
{
	fatalerror("32051: unimplemented op subs at %08X\n", m_pc-1);
}

void tms32051_device::op_subt()
{
	fatalerror("32051: unimplemented op subt at %08X\n", m_pc-1);
}

void tms32051_device::op_xor_mem()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_acc ^= (UINT32)(data);

	CYCLES(1);
}

void tms32051_device::op_xor_limm()
{
	UINT32 imm = ROPCODE();
	int shift = m_op & 0xf;

	m_acc ^= imm << shift;

	CYCLES(1);
}

void tms32051_device::op_xor_s16_limm()
{
	fatalerror("32051: unimplemented op xor s16 limm at %08X\n", m_pc-1);
}

void tms32051_device::op_xorb()
{
	fatalerror("32051: unimplemented op xorb at %08X\n", m_pc-1);
}

void tms32051_device::op_zalr()
{
	fatalerror("32051: unimplemented op zalr at %08X\n", m_pc-1);
}

void tms32051_device::op_zap()
{
	m_acc = 0;
	m_preg = 0;

	CYCLES(1);
}

/*****************************************************************************/

void tms32051_device::op_adrk()
{
	UINT16 imm = m_op & 0xff;
	UPDATE_AR(m_st0.arp, imm);

	CYCLES(1);
}

void tms32051_device::op_cmpr()
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

void tms32051_device::op_lar_mem()
{
	int arx = (m_op >> 8) & 0x7;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_ar[arx] = data;

	CYCLES(2);
}

void tms32051_device::op_lar_simm()
{
	int arx = (m_op >> 8) & 0x7;
	m_ar[arx] = m_op & 0xff;

	CYCLES(2);
}

void tms32051_device::op_lar_limm()
{
	int arx = m_op & 0x7;
	UINT16 imm = ROPCODE();
	m_ar[arx] = imm;

	CYCLES(2);
}

void tms32051_device::op_ldp_mem()
{
	fatalerror("32051: unimplemented op ldp mem at %08X\n", m_pc-1);
}

void tms32051_device::op_ldp_imm()
{
	m_st0.dp = (m_op & 0x1ff) << 7;
	CYCLES(2);
}

void tms32051_device::op_mar()
{
	// direct addressing is NOP
	if (m_op & 0x80)
	{
		GET_ADDRESS();
	}
	CYCLES(1);
}

void tms32051_device::op_sar()
{
	int arx = (m_op >> 8) & 0x7;
	UINT16 ar = m_ar[arx];
	UINT16 ea = GET_ADDRESS();
	DM_WRITE16(ea, ar);

	CYCLES(1);
}

void tms32051_device::op_sbrk()
{
	UINT16 imm = m_op & 0xff;
	UPDATE_AR(m_st0.arp, -imm);

	CYCLES(1);
}

/*****************************************************************************/

void tms32051_device::op_b()
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP

	CHANGE_PC(pma);
	CYCLES(4);
}

void tms32051_device::op_bacc()
{
	CHANGE_PC((UINT16)(m_acc));

	CYCLES(4);
}

void tms32051_device::op_baccd()
{
	UINT16 pc = (UINT16)(m_acc);

	delay_slot(m_pc);
	CHANGE_PC(pc);

	CYCLES(2);
}

void tms32051_device::op_banz()
{
	UINT16 pma = ROPCODE();

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

void tms32051_device::op_banzd()
{
	fatalerror("32051: unimplemented op banzd at %08X\n", m_pc-1);
}

void tms32051_device::op_bcnd()
{
	UINT16 pma = ROPCODE();

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

void tms32051_device::op_bcndd()
{
	UINT16 pma = ROPCODE();

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

void tms32051_device::op_bd()
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP

	delay_slot(m_pc);
	CHANGE_PC(pma);
	CYCLES(2);
}

void tms32051_device::op_cala()
{
	PUSH_STACK(m_pc);

	CHANGE_PC(m_acc);

	CYCLES(4);
}

void tms32051_device::op_calad()
{
	UINT16 pma = m_acc;
	PUSH_STACK(m_pc+2);

	delay_slot(m_pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

void tms32051_device::op_call()
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP
	PUSH_STACK(m_pc);

	CHANGE_PC(pma);

	CYCLES(4);
}

void tms32051_device::op_calld()
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();      // update AR/ARP
	PUSH_STACK(m_pc+2);

	delay_slot(m_pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

void tms32051_device::op_cc()
{
	UINT16 pma = ROPCODE();

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

void tms32051_device::op_ccd()
{
	UINT16 pma = ROPCODE();

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

void tms32051_device::op_intr()
{
	fatalerror("32051: unimplemented op intr at %08X\n", m_pc-1);
}

void tms32051_device::op_nmi()
{
	fatalerror("32051: unimplemented op nmi at %08X\n", m_pc-1);
}

void tms32051_device::op_retc()
{
	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		UINT16 pc = POP_STACK();
		CHANGE_PC(pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

void tms32051_device::op_retcd()
{
	if (GET_ZLVC_CONDITION((m_op >> 4) & 0xf, m_op & 0xf) && GET_TP_CONDITION((m_op >> 8) & 0x3))
	{
		UINT16 pc = POP_STACK();
		delay_slot(m_pc);
		CHANGE_PC(pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

void tms32051_device::op_rete()
{
	UINT16 pc = POP_STACK();
	CHANGE_PC(pc);

	restore_interrupt_context();

	m_st0.intm = 0;

	CYCLES(4);
}

void tms32051_device::op_reti()
{
	fatalerror("32051: unimplemented op reti at %08X\n", m_pc-1);
}

void tms32051_device::op_trap()
{
	fatalerror("32051: unimplemented op trap at %08X\n", m_pc-1);
}

void tms32051_device::op_xc()
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

void tms32051_device::op_bldd_slimm()
{
	UINT16 pfc = ROPCODE();

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_bldd_dlimm()
{
	UINT16 pfc = ROPCODE();

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_bldd_sbmar()
{
	UINT16 pfc = m_bmar;

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_bldd_dbmar()
{
	UINT16 pfc = m_bmar;

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_bldp()
{
	UINT16 pfc = m_bmar;

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(1);

		m_rptc--;
	};
}

void tms32051_device::op_blpd_bmar()
{
	fatalerror("32051: unimplemented op bpld bmar at %08X\n", m_pc-1);
}

void tms32051_device::op_blpd_imm()
{
	UINT16 pfc = ROPCODE();

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

/*****************************************************************************/

void tms32051_device::op_dmov()
{
	fatalerror("32051: unimplemented op dmov at %08X\n", m_pc-1);
}

void tms32051_device::op_in()
{
	fatalerror("32051: unimplemented op in at %08X\n", m_pc-1);
}

void tms32051_device::op_lmmr()
{
	UINT16 pfc = ROPCODE();

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(pfc);
		DM_WRITE16(ea & 0x7f, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_out()
{
	UINT16 port = ROPCODE();
	UINT16 ea = GET_ADDRESS();
	
	UINT16 data = DM_READ16(ea);
	m_io->write_word(port << 1, data);

	// TODO: handle repeat
	CYCLES(3);
}

void tms32051_device::op_smmr()
{
	UINT16 pfc = ROPCODE();

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea & 0x7f);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_tblr()
{
	UINT16 pfc = (UINT16)(m_acc);

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

void tms32051_device::op_tblw()
{
	UINT16 pfc = (UINT16)(m_acc);

	while (m_rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		m_rptc--;
	};
}

/*****************************************************************************/

void tms32051_device::op_apl_dbmr()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	
	data &= m_dbmr;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms32051_device::op_apl_imm()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();
	UINT16 data = DM_READ16(ea);
	
	data &= imm;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms32051_device::op_cpl_dbmr()
{
	fatalerror("32051: unimplemented op cpl dbmr at %08X\n", m_pc-1);
}

void tms32051_device::op_cpl_imm()
{
	UINT16 imm = ROPCODE();
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_st1.tc = (data == imm) ? 1 : 0;

	CYCLES(1);
}

void tms32051_device::op_opl_dbmr()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	data |= m_dbmr;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms32051_device::op_opl_imm()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();
	UINT16 data = DM_READ16(ea);
	data |= imm;

	m_st1.tc = (data == 0) ? 1 : 0;

	DM_WRITE16(ea, data);
	CYCLES(1);
}

void tms32051_device::op_splk()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();

	DM_WRITE16(ea, imm);

	CYCLES(2);
}

void tms32051_device::op_xpl_dbmr()
{
	fatalerror("32051: unimplemented op xpl dbmr at %08X\n", m_pc-1);
}

void tms32051_device::op_xpl_imm()
{
	fatalerror("32051: unimplemented op xpl imm at %08X\n", m_pc-1);
}

void tms32051_device::op_apac()
{
	INT32 spreg = PREG_PSCALER(m_preg);
	m_acc = ADD(m_acc, spreg, false);

	CYCLES(1);
}

void tms32051_device::op_lph()
{
	fatalerror("32051: unimplemented op lph at %08X\n", m_pc-1);
}

void tms32051_device::op_lt()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_treg0 = data;
	if (m_pmst.trm == 0)
	{
		m_treg1 = data;
		m_treg2 = data;
	}

	CYCLES(1);
}

void tms32051_device::op_lta()
{
	INT32 spreg;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

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

void tms32051_device::op_ltd()
{
	fatalerror("32051: unimplemented op ltd at %08X\n", m_pc-1);
}

void tms32051_device::op_ltp()
{
	fatalerror("32051: unimplemented op ltp at %08X\n", m_pc-1);
}

void tms32051_device::op_lts()
{
	fatalerror("32051: unimplemented op lts at %08X\n", m_pc-1);
}

void tms32051_device::op_mac()
{
	fatalerror("32051: unimplemented op mac at %08X\n", m_pc-1);
}

void tms32051_device::op_macd()
{
	fatalerror("32051: unimplemented op macd at %08X\n", m_pc-1);
}

void tms32051_device::op_madd()
{
	fatalerror("32051: unimplemented op madd at %08X\n", m_pc-1);
}

void tms32051_device::op_mads()
{
	fatalerror("32051: unimplemented op mads at %08X\n", m_pc-1);
}

void tms32051_device::op_mpy_mem()
{
	UINT16 ea = GET_ADDRESS();
	INT16 data = DM_READ16(ea);

	m_preg = (INT32)(data) * (INT32)(INT16)(m_treg0);

	CYCLES(1);
}

void tms32051_device::op_mpy_simm()
{
	fatalerror("32051: unimplemented op mpy simm at %08X\n", m_pc-1);
}

void tms32051_device::op_mpy_limm()
{
	fatalerror("32051: unimplemented op mpy limm at %08X\n", m_pc-1);
}

void tms32051_device::op_mpya()
{
	fatalerror("32051: unimplemented op mpya at %08X\n", m_pc-1);
}

void tms32051_device::op_mpys()
{
	fatalerror("32051: unimplemented op mpys at %08X\n", m_pc-1);
}

void tms32051_device::op_mpyu()
{
	fatalerror("32051: unimplemented op mpyu at %08X\n", m_pc-1);
}

void tms32051_device::op_pac()
{
	fatalerror("32051: unimplemented op pac at %08X\n", m_pc-1);
}

void tms32051_device::op_spac()
{
	fatalerror("32051: unimplemented op spac at %08X\n", m_pc-1);
}

void tms32051_device::op_sph()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 spreg = (UINT16)(PREG_PSCALER(m_preg) >> 16);
	DM_WRITE16(ea, spreg);

	CYCLES(1);
}

void tms32051_device::op_spl()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 spreg = (UINT16)(PREG_PSCALER(m_preg));
	DM_WRITE16(ea, spreg);

	CYCLES(1);
}

void tms32051_device::op_spm()
{
	m_st1.pm = m_op & 0x3;

	CYCLES(1);
}

void tms32051_device::op_sqra()
{
	fatalerror("32051: unimplemented op sqra at %08X\n", m_pc-1);
}

void tms32051_device::op_sqrs()
{
	fatalerror("32051: unimplemented op sqrs at %08X\n", m_pc-1);
}

void tms32051_device::op_zpr()
{
	fatalerror("32051: unimplemented op zpr at %08X\n", m_pc-1);
}

void tms32051_device::op_bit()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_st1.tc = (data >> (~m_op >> 8 & 0xf)) & 1;

	CYCLES(1);
}

void tms32051_device::op_bitt()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	m_st1.tc = (data >> (~m_treg2 & 0xf)) & 1;

	CYCLES(1);
}

void tms32051_device::op_clrc_ov()
{
	m_st0.ovm = 0;

	CYCLES(1);
}

void tms32051_device::op_clrc_ext()
{
	m_st1.sxm = 0;

	CYCLES(1);
}

void tms32051_device::op_clrc_hold()
{
	fatalerror("32051: unimplemented op clrc hold at %08X\n", m_pc-1);
}

void tms32051_device::op_clrc_tc()
{
	fatalerror("32051: unimplemented op clrc tc at %08X\n", m_pc-1);
}

void tms32051_device::op_clrc_carry()
{
	fatalerror("32051: unimplemented op clrc carry at %08X\n", m_pc-1);
}

void tms32051_device::op_clrc_cnf()
{
	m_st1.cnf = 0;

	CYCLES(1);
}

void tms32051_device::op_clrc_intm()
{
	m_st0.intm = 0;

	check_interrupts();

	CYCLES(1);
}

void tms32051_device::op_clrc_xf()
{
	m_st1.xf = 0;

	CYCLES(1);
}

void tms32051_device::op_idle()
{
	m_idle = true;
}

void tms32051_device::op_idle2()
{
	fatalerror("32051: unimplemented op idle2 at %08X\n", m_pc-1);
}

void tms32051_device::op_lst_st0()
{
	fatalerror("32051: unimplemented op lst st0 at %08X\n", m_pc-1);
}

void tms32051_device::op_lst_st1()
{
	fatalerror("32051: unimplemented op lst st1 at %08X\n", m_pc-1);
}

void tms32051_device::op_pop()
{
	m_acc = POP_STACK();

	CYCLES(1);
}

void tms32051_device::op_popd()
{
	fatalerror("32051: unimplemented op popd at %08X\n", m_pc-1);
}

void tms32051_device::op_pshd()
{
	fatalerror("32051: unimplemented op pshd at %08X\n", m_pc-1);
}

void tms32051_device::op_push()
{
	fatalerror("32051: unimplemented op push at %08X\n", m_pc-1);
}

void tms32051_device::op_rpt_mem()
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	m_rptc = data;
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(1);
}

void tms32051_device::op_rpt_limm()
{
	m_rptc = (UINT16)ROPCODE();
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(2);
}

void tms32051_device::op_rpt_simm()
{
	m_rptc = (m_op & 0xff);
	m_rpt_start = m_pc;
	m_rpt_end = m_pc;

	CYCLES(1);
}

void tms32051_device::op_rptb()
{
	UINT16 pma = ROPCODE();
	m_pmst.braf = 1;
	m_pasr = m_pc;
	m_paer = pma + 1;

	CYCLES(2);
}

void tms32051_device::op_rptz()
{
	fatalerror("32051: unimplemented op rptz at %08X\n", m_pc-1);
}

void tms32051_device::op_setc_ov()
{
	m_st0.ovm = 1;

	CYCLES(1);
}

void tms32051_device::op_setc_ext()
{
	m_st1.sxm = 1;

	CYCLES(1);
}

void tms32051_device::op_setc_hold()
{
	fatalerror("32051: unimplemented op setc hold at %08X\n", m_pc-1);
}

void tms32051_device::op_setc_tc()
{
	m_st1.tc = 1;

	CYCLES(1);
}

void tms32051_device::op_setc_carry()
{
	fatalerror("32051: unimplemented op setc carry at %08X\n", m_pc-1);
}

void tms32051_device::op_setc_xf()
{
	m_st1.xf = 1;

	CYCLES(1);
}

void tms32051_device::op_setc_cnf()
{
	m_st1.cnf = 1;

	CYCLES(1);
}

void tms32051_device::op_setc_intm()
{
	m_st0.intm = 1;

	check_interrupts();

	CYCLES(1);
}

void tms32051_device::op_sst_st0()
{
	fatalerror("32051: unimplemented op sst st0 at %08X\n", m_pc-1);
}

void tms32051_device::op_sst_st1()
{
	fatalerror("32051: unimplemented op sst st1 at %08X\n", m_pc-1);
}
