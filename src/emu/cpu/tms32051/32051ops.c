INLINE void PUSH_STACK(UINT16 pc)
{
	if (tms.pcstack_ptr >= 8)
	{
		fatalerror("32051: stack overflow at %04X!\n", tms.pc);
	}

	tms.pcstack[tms.pcstack_ptr] = pc;
	tms.pcstack_ptr++;
}

INLINE UINT16 POP_STACK(void)
{
	UINT16 pc;
	tms.pcstack_ptr--;
	if (tms.pcstack_ptr < 0)
	{
		fatalerror("32051: stack underflow at %04X!\n", tms.pc);
	}

	pc = tms.pcstack[tms.pcstack_ptr];
	return pc;
}

INLINE INT32 SUB(INT32 a, INT32 b, int shift16)
{
	INT64 res = a - b;
	if (tms.st0.ovm)	// overflow saturation mode
	{
		if ((res & 0x80000000) && ((UINT32)(res >> 32) & 0xffffffff) != 0xffffffff)
		{
			res = 0x80000000;
		}
		else if (((res & 0x80000000) == 0) && ((UINT32)(res >> 32) & 0xffffffff) != 0)
		{
			res = 0x7fffffff;
		}
	}
	else				// normal mode, result is not modified
	{
		// set OV flag if overflow occured, this is a sticky flag
		if (((a) ^ (b)) & ((a) ^ ((INT32)res)) & 0x80000000)
		{
			tms.st0.ov = 1;
		}
	}

	// set carry
	if (!shift16)
	{
		// C is cleared if borrow was generated
		if (((UINT64)(a) + (UINT64)(~b)) & U64(0x100000000))
		{
			tms.st1.c = 0;
		}
		else
		{
			tms.st1.c = 1;
		}
	}
	else
	{
		// if 16-bit shift, C is cleared if borrow was generated, otherwise C is unaffected
		if (((UINT64)(a) + (UINT64)(~b)) & U64(0x100000000))
		{
			tms.st1.c = 0;
		}
	}

	return (INT32)(res);
}

INLINE INT32 ADD(INT32 a, INT32 b, int shift16)
{
	INT64 res = a + b;
	if (tms.st0.ovm)	// overflow saturation mode
	{
		if ((res & 0x80000000) && ((UINT32)(res >> 32) & 0xffffffff) != 0xffffffff)
		{
			res = 0x80000000;
		}
		else if (((res & 0x80000000) == 0) && ((UINT32)(res >> 32) & 0xffffffff) != 0)
		{
			res = 0x7fffffff;
		}
	}
	else				// normal mode, result is not modified
	{
		// set OV flag if overflow occured, this is a sticky flag
		if (((res) ^ (b)) & ((res) ^ (a)) & 0x80000000)
		{
			tms.st0.ov = 1;
		}
	}

	// set carry
	if (!shift16)
	{
		// C is set if carry was generated
		if (res & U64(0x100000000))
		{
			tms.st1.c = 1;
		}
		else
		{
			tms.st1.c = 0;
		}
	}
	else
	{
		// if 16-bit shift, C is set carry was generated, otherwise C is unaffected
		if (res & U64(0x100000000))
		{
			tms.st1.c = 1;
		}
	}

	return (INT32)(res);
}


INLINE void UPDATE_AR(int ar, int step)
{
	int cenb1 = (tms.cbcr >> 3) & 0x1;
	int car1 = tms.cbcr & 0x7;
	int cenb2 = (tms.cbcr >> 7) & 0x1;
	int car2 = (tms.cbcr >> 4) & 0x7;

	if (cenb1 && ar == car1)
	{
		// update circular buffer 1
		if (tms.ar[ar] == tms.cber1)
		{
			tms.ar[ar] = tms.cbsr1;
		}
		else
		{
			tms.ar[ar] += step;
		}
	}
	else if (cenb2 && ar == car2)
	{
		// update circular buffer 2
		if (tms.ar[ar] == tms.cber2)
		{
			tms.ar[ar] = tms.cbsr2;
		}
		else
		{
			tms.ar[ar] += step;
		}
	}
	else
	{
		tms.ar[ar] += step;
	}
}

INLINE void UPDATE_ARP(int nar)
{
	tms.st1.arb = tms.st0.arp;
	tms.st0.arp = nar;
}

static UINT16 GET_ADDRESS(void)
{
	if (tms.op & 0x80)		// Indirect Addressing
	{
		UINT16 ea;
		int arp = tms.st0.arp;
		int nar = tms.op & 0x7;

		ea = tms.ar[arp];

		switch ((tms.op >> 3) & 0xf)
		{
			case 0x0:	// *            (no operation)
			{
				break;
			}
			case 0x1:	// *, ARn       (NAR -> ARP)
			{
				UPDATE_ARP(nar);
				break;
			}
			case 0x2:	// *-           ((CurrentAR)-1 -> CurrentAR)
			{
				UPDATE_AR(arp, -1);
				break;
			}
			case 0x3:	// *-, ARn      ((CurrentAR)-1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, -1);
				UPDATE_ARP(nar);
				break;
			}
			case 0x4:	// *+           ((CurrentAR)+1 -> CurrentAR)
			{
				UPDATE_AR(arp, 1);
				break;
			}
			case 0x5:	// *+, ARn      ((CurrentAR)+1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, 1);
				UPDATE_ARP(nar);
				break;
			}
			case 0xa:	// *0-          ((CurrentAR) - INDX)
			{
				UPDATE_AR(arp, -tms.indx);
				break;
			}
			case 0xb:	// *0-, ARn     ((CurrentAR) - INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, -tms.indx);
				UPDATE_ARP(nar);
				break;
			}
			case 0xc:	// *0+          ((CurrentAR) + INDX -> CurrentAR)
			{
				UPDATE_AR(arp, tms.indx);
				break;
			}
			case 0xd:	// *0+, ARn     ((CurrentAR) + INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(arp, tms.indx);
				UPDATE_ARP(nar);
				break;
			}

			default:	fatalerror("32051: GET_ADDRESS: unimplemented indirect addressing mode %d at %04X (%04X)\n", (tms.op >> 3) & 0xf, tms.pc, tms.op);
		}

		return ea;
	}
	else					// Direct Addressing
	{
		return tms.st0.dp | (tms.op & 0x7f);
	}
}

static int GET_ZLVC_CONDITION(int zlvc, int zlvc_mask)
{
	int condition = 0;

	if (zlvc_mask & 0x8)		// Z-bit
	{
		if ((zlvc & 0x8) && (INT32)(tms.acc) == 0)				// EQ
		{
			condition = 1;
		}
		else if ((zlvc & 0x8) == 0 && (INT32)(tms.acc) != 0)	// NEQ
		{
			condition = 1;
		}
	}
	if (zlvc_mask & 0x4)		// L-bit
	{
		if ((zlvc & 0x4) && (INT32)(tms.acc) < 0)				// LT
		{
			condition = 1;
		}
		else if ((zlvc & 0x4) == 0 && (INT32)(tms.acc) > 0)		// GT
		{
			condition = 1;
		}
	}
	if (zlvc_mask & 0x2)		// OV-bit
	{
		if ((zlvc & 0x2) && tms.st0.ov)							// OV
		{
			condition = 1;
		}
		else if ((zlvc & 0x2) == 0 && tms.st0.ov == 0)			// NOV
		{
			condition = 1;
		}
	}
	if (zlvc_mask & 0x1)		// C-bit
	{
		if ((zlvc & 0x1) && tms.st1.c)							// C
		{
			condition = 1;
		}
		else if ((zlvc & 0x1) == 0 && tms.st1.c == 0)			// NC
		{
			condition = 1;
		}
	}

	return condition;
}

static int GET_TP_CONDITION(int tp)
{
	switch (tp)
	{
		case 0:		// BIO pin low
		{
			// TODO
			return 0;
		}
		case 1:		// TC = 1
		{
			return (tms.st1.tc == 1) ? 1 : 0;
		}
		case 2:		// TC = 0
		{
			return (tms.st1.tc == 0) ? 1 : 0;
		}
		case 3:		// always false
		{
			return 0;
		}
	}
	return 0;
}

INLINE INT32 PREG_PSCALER(INT32 preg)
{
	switch (tms.st1.pm & 3)
	{
		case 0:		// No shift
		{
			return preg;
		}
		case 1:		// Left-shifted 1 bit, LSB zero-filled
		{
			return preg << 1;
		}
		case 2:		// Left-shifted 4 bits, 4 LSBs zero-filled
		{
			return preg << 4;
		}
		case 3:		// Right-shifted 6 bits, sign-extended, 6 LSBs lost
		{
			return (INT32)(preg >> 6);
		}
	}
	return 0;
}



static void op_invalid(void)
{
	fatalerror("32051: invalid op at %08X", tms.pc-1);
}

static void op_group_be(void);
static void op_group_bf(void);

/*****************************************************************************/

static void op_abs(void)
{
	fatalerror("32051: unimplemented op abs at %08X", tms.pc-1);
}

static void op_adcb(void)
{
	fatalerror("32051: unimplemented op adcb at %08X", tms.pc-1);
}

static void op_add_mem(void)
{
	INT32 d;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int shift = (tms.op >> 8) & 0xf;

	if (tms.st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	tms.acc = ADD(tms.acc, d, 0);

	CYCLES(1);
}

static void op_add_simm(void)
{
	UINT16 imm = tms.op & 0xff;

	tms.acc = ADD(tms.acc, imm, 0);

	CYCLES(1);
}

static void op_add_limm(void)
{
	INT32 d;
	UINT16 imm = ROPCODE();
	int shift = tms.op & 0xf;

	if (tms.st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	tms.acc = ADD(tms.acc, d, 0);

	CYCLES(2);
}

static void op_add_s16_mem(void)
{
	fatalerror("32051: unimplemented op add s16 mem at %08X", tms.pc-1);
}

static void op_addb(void)
{
	tms.acc = ADD(tms.acc, tms.accb, 0);

	CYCLES(1);
}

static void op_addc(void)
{
	fatalerror("32051: unimplemented op addc at %08X", tms.pc-1);
}

static void op_adds(void)
{
	fatalerror("32051: unimplemented op adds at %08X", tms.pc-1);
}

static void op_addt(void)
{
	fatalerror("32051: unimplemented op addt at %08X", tms.pc-1);
}

static void op_and_mem(void)
{
	fatalerror("32051: unimplemented op and mem at %08X", tms.pc-1);
}

static void op_and_limm(void)
{
	UINT32 imm = ROPCODE();
	int shift = tms.op & 0xf;

	tms.acc &= imm << shift;

	CYCLES(2);
}

static void op_and_s16_limm(void)
{
	fatalerror("32051: unimplemented op and s16 limm at %08X", tms.pc-1);
}

static void op_andb(void)
{
	fatalerror("32051: unimplemented op andb at %08X", tms.pc-1);
}

static void op_bsar(void)
{
	int shift = (tms.op & 0xf) + 1;

	if (tms.st1.sxm)
	{
		tms.acc = (INT32)(tms.acc) >> shift;
	}
	else
	{
		tms.acc = (UINT32)(tms.acc) >> shift;
	}

	CYCLES(1);
}

static void op_cmpl(void)
{
	tms.acc = ~tms.acc;

	CYCLES(1);
}

static void op_crgt(void)
{
	if (tms.acc > tms.accb)
	{
		tms.accb = tms.acc;
		tms.st1.c = 1;
	}
	else if (tms.acc < tms.accb)
	{
		tms.acc = tms.accb;
		tms.st1.c = 0;
	}
	else
	{
		tms.st1.c = 1;
	}

	CYCLES(1);
}

static void op_crlt(void)
{
	if (tms.acc < tms.accb)
	{
		tms.accb = tms.acc;
		tms.st1.c = 1;
	}
	else if (tms.acc > tms.accb)
	{
		tms.acc = tms.accb;
		tms.st1.c = 0;
	}
	else
	{
		tms.st1.c = 0;
	}

	CYCLES(1);
}

static void op_exar(void)
{
	INT32 tmp = tms.acc;
	tms.acc = tms.accb;
	tms.accb = tmp;

	CYCLES(1);
}

static void op_lacb(void)
{
	tms.acc = tms.accb;

	CYCLES(1);
}

static void op_lacc_mem(void)
{
	int shift = (tms.op >> 8) & 0xf;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	if (tms.st1.sxm)
	{
		tms.acc = (INT32)(INT16)(data) << shift;
	}
	else
	{
		tms.acc = (UINT32)(UINT16)(data) << shift;
	}

	CYCLES(1);
}

static void op_lacc_limm(void)
{
	UINT16 imm = ROPCODE();
	int shift = tms.op & 0xf;

	if (tms.st1.sxm)
	{
		tms.acc = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		tms.acc = (UINT32)(UINT16)(imm) << shift;
	}

	CYCLES(1);
}

static void op_lacc_s16_mem(void)
{
	UINT16 ea = GET_ADDRESS();

	if (tms.st1.sxm)
	{
		tms.acc = (INT32)(INT16)(DM_READ16(ea)) << 16;
	}
	else
	{
		tms.acc = (UINT32)(DM_READ16(ea)) << 16;
	}

	CYCLES(1);
}

static void op_lacl_simm(void)
{
	tms.acc = tms.op & 0xff;

	CYCLES(1);
}

static void op_lacl_mem(void)
{
	UINT16 ea = GET_ADDRESS();
	tms.acc = DM_READ16(ea) & 0xffff;

	CYCLES(1);
}

static void op_lact(void)
{
	fatalerror("32051: unimplemented op lact at %08X", tms.pc-1);
}

static void op_lamm(void)
{
	UINT16 ea = GET_ADDRESS();
	ea &= 0x7f;

	tms.acc = DM_READ16(ea) & 0xffff;
	CYCLES(1);
}

static void op_neg(void)
{
	if ((UINT32)(tms.acc) == 0x80000000)
	{
		tms.st0.ov = 1;
		if (tms.st0.ovm)
		{
			tms.acc = 0x7fffffff;
		}
		else
		{
			tms.acc = 0x80000000;
		}
	}
	else
	{
		tms.acc = 0 - (UINT32)(tms.acc);

		if (tms.acc == 0)
		{
			tms.st1.c = 1;
		}
		else
		{
			tms.st1.c = 0;
		}
	}

	CYCLES(1);
}

static void op_norm(void)
{
	fatalerror("32051: unimplemented op norm at %08X", tms.pc-1);
}

static void op_or_mem(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	tms.acc |= (UINT32)(data);

	CYCLES(1);
}

static void op_or_limm(void)
{
	UINT32 imm = ROPCODE();
	int shift = tms.op & 0xf;

	tms.acc |= imm << shift;

	CYCLES(1);
}

static void op_or_s16_limm(void)
{
	fatalerror("32051: unimplemented op or s16 limm at %08X", tms.pc-1);
}

static void op_orb(void)
{
	tms.acc |= tms.accb;

	CYCLES(1);
}

static void op_rol(void)
{
	fatalerror("32051: unimplemented op rol at %08X", tms.pc-1);
}

static void op_rolb(void)
{
	UINT32 acc = tms.acc;
	UINT32 accb = tms.accb;
	UINT32 c = tms.st1.c & 1;

	tms.acc = (acc << 1) | ((accb >> 31) & 1);
	tms.accb = (accb << 1) | c;
	tms.st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

static void op_ror(void)
{
	fatalerror("32051: unimplemented op ror at %08X", tms.pc-1);
}

static void op_rorb(void)
{
	fatalerror("32051: unimplemented op rorb at %08X", tms.pc-1);
}

static void op_sacb(void)
{
	tms.accb = tms.acc;

	CYCLES(1);
}

static void op_sach(void)
{
	UINT16 ea = GET_ADDRESS();
	int shift = (tms.op >> 8) & 0x7;

	DM_WRITE16(ea, (UINT16)((tms.acc << shift) >> 16));
	CYCLES(1);
}

static void op_sacl(void)
{
	UINT16 ea = GET_ADDRESS();
	int shift = (tms.op >> 8) & 0x7;

	DM_WRITE16(ea, (UINT16)(tms.acc << shift));
	CYCLES(1);
}

static void op_samm(void)
{
	UINT16 ea = GET_ADDRESS();
	ea &= 0x7f;

	DM_WRITE16(ea, (UINT16)(tms.acc));
	CYCLES(1);
}

static void op_sath(void)
{
	fatalerror("32051: unimplemented op sath at %08X", tms.pc-1);
}

static void op_satl(void)
{
	fatalerror("32051: unimplemented op satl at %08X", tms.pc-1);
}

static void op_sbb(void)
{
	tms.acc = SUB(tms.acc, tms.accb, 0);

	CYCLES(1);
}

static void op_sbbb(void)
{
	fatalerror("32051: unimplemented op sbbb at %08X", tms.pc-1);
}

static void op_sfl(void)
{
	tms.st1.c = (tms.acc >> 31) & 1;
	tms.acc = tms.acc << 1;

	CYCLES(1);
}

static void op_sflb(void)
{
	UINT32 acc = tms.acc;
	UINT32 accb = tms.accb;

	tms.acc = (acc << 1) | ((accb >> 31) & 1);
	tms.accb = (accb << 1);
	tms.st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

static void op_sfr(void)
{
	tms.st1.c = tms.acc & 1;

	if (tms.st1.sxm)
	{
		tms.acc = (INT32)(tms.acc) >> 1;
	}
	else
	{
		tms.acc = (UINT32)(tms.acc) >> 1;
	}

	CYCLES(1);
}

static void op_sfrb(void)
{
	fatalerror("32051: unimplemented op sfrb at %08X", tms.pc-1);
}

static void op_sub_mem(void)
{
	INT32 d;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int shift = (tms.op >> 8) & 0xf;

	if (tms.st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	tms.acc = SUB(tms.acc, d, 0);

	CYCLES(1);
}

static void op_sub_s16_mem(void)
{
	fatalerror("32051: unimplemented op sub s16 mem at %08X", tms.pc-1);
}

static void op_sub_simm(void)
{
	UINT16 imm = tms.op & 0xff;

	tms.acc = SUB(tms.acc, imm, 0);

	CYCLES(1);
}

static void op_sub_limm(void)
{
	INT32 d;
	UINT16 imm = ROPCODE();
	int shift = tms.op & 0xf;

	if (tms.st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	tms.acc = SUB(tms.acc, d, 0);

	CYCLES(2);
}

static void op_subb(void)
{
	fatalerror("32051: unimplemented op subb at %08X", tms.pc-1);
}

static void op_subc(void)
{
	fatalerror("32051: unimplemented op subc at %08X", tms.pc-1);
}

static void op_subs(void)
{
	fatalerror("32051: unimplemented op subs at %08X", tms.pc-1);
}

static void op_subt(void)
{
	fatalerror("32051: unimplemented op subt at %08X", tms.pc-1);
}

static void op_xor_mem(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	tms.acc ^= (UINT32)(data);

	CYCLES(1);
}

static void op_xor_limm(void)
{
	UINT32 imm = ROPCODE();
	int shift = tms.op & 0xf;

	tms.acc ^= imm << shift;

	CYCLES(1);
}

static void op_xor_s16_limm(void)
{
	fatalerror("32051: unimplemented op xor s16 limm at %08X", tms.pc-1);
}

static void op_xorb(void)
{
	fatalerror("32051: unimplemented op xorb at %08X", tms.pc-1);
}

static void op_zalr(void)
{
	fatalerror("32051: unimplemented op zalr at %08X", tms.pc-1);
}

static void op_zap(void)
{
	tms.acc = 0;
	tms.preg = 0;

	CYCLES(1);
}

/*****************************************************************************/

static void op_adrk(void)
{
	UINT16 imm = tms.op & 0xff;
	UPDATE_AR(tms.st0.arp, imm);

	CYCLES(1);
}

static void op_cmpr(void)
{
	tms.st1.tc = 0;

	switch (tms.op & 0x3)
	{
		case 0:			// (CurrentAR) == ARCR
		{
			if (tms.ar[tms.st0.arp] == tms.arcr)
			{
				tms.st1.tc = 1;
			}
			break;
		}
		case 1:			// (CurrentAR) < ARCR
		{
			if (tms.ar[tms.st0.arp] < tms.arcr)
			{
				tms.st1.tc = 1;
			}
			break;
		}
		case 2:			// (CurrentAR) > ARCR
		{
			if (tms.ar[tms.st0.arp] > tms.arcr)
			{
				tms.st1.tc = 1;
			}
			break;
		}
		case 3:			// (CurrentAR) != ARCR
		{
			if (tms.ar[tms.st0.arp] != tms.arcr)
			{
				tms.st1.tc = 1;
			}
			break;
		}
	}

	CYCLES(1);
}

static void op_lar_mem(void)
{
	int arx = (tms.op >> 8) & 0x7;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	tms.ar[arx] = data;

	CYCLES(2);
}

static void op_lar_simm(void)
{
	int arx = (tms.op >> 8) & 0x7;
	tms.ar[arx] = tms.op & 0xff;

	CYCLES(2);
}

static void op_lar_limm(void)
{
	int arx = tms.op & 0x7;
	UINT16 imm = ROPCODE();
	tms.ar[arx] = imm;

	CYCLES(2);
}

static void op_ldp_mem(void)
{
	fatalerror("32051: unimplemented op ldp mem at %08X", tms.pc-1);
}

static void op_ldp_imm(void)
{
	tms.st0.dp = (tms.op & 0x1ff) << 7;
	CYCLES(2);
}

static void op_mar(void)
{
	// direct addressing is NOP
	if (tms.op & 0x80)
	{
		GET_ADDRESS();
	}
	CYCLES(1);
}

static void op_sar(void)
{
	int arx = (tms.op >> 8) & 0x7;
	UINT16 ar = tms.ar[arx];
	UINT16 ea = GET_ADDRESS();
	DM_WRITE16(ea, ar);

	CYCLES(1);
}

static void op_sbrk(void)
{
	UINT16 imm = tms.op & 0xff;
	UPDATE_AR(tms.st0.arp, -imm);

	CYCLES(1);
}

/*****************************************************************************/

static void op_b(void)
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();		// update AR/ARP

	CHANGE_PC(pma);
	CYCLES(4);
}

static void op_bacc(void)
{
	CHANGE_PC((UINT16)(tms.acc));

	CYCLES(4);
}

static void op_baccd(void)
{
	UINT16 pc = (UINT16)(tms.acc);

	delay_slot(tms.pc);
	CHANGE_PC(pc);

	CYCLES(2);
}

static void op_banz(void)
{
	UINT16 pma = ROPCODE();

	if (tms.ar[tms.st0.arp] != 0)
	{
		CHANGE_PC(pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}

	GET_ADDRESS();		// modify AR/ARP
}

static void op_banzd(void)
{
	fatalerror("32051: unimplemented op banzd at %08X", tms.pc-1);
}

static void op_bcnd(void)
{
	UINT16 pma = ROPCODE();

	int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
	int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);

	if (zlvc_condition || tp_condition)
	{
		CHANGE_PC(pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_bcndd(void)
{
	UINT16 pma = ROPCODE();

	int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
	int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);

	if (zlvc_condition || tp_condition)
	{
		delay_slot(tms.pc);
		CHANGE_PC(pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_bd(void)
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();		// update AR/ARP

	delay_slot(tms.pc);
	CHANGE_PC(pma);
	CYCLES(2);
}

static void op_cala(void)
{
	PUSH_STACK(tms.pc);

	CHANGE_PC(tms.acc);

	CYCLES(4);
}

static void op_calad(void)
{
	UINT16 pma = tms.acc;
	PUSH_STACK(tms.pc+2);

	delay_slot(tms.pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

static void op_call(void)
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();		// update AR/ARP
	PUSH_STACK(tms.pc);

	CHANGE_PC(pma);

	CYCLES(4);
}

static void op_calld(void)
{
	UINT16 pma = ROPCODE();
	GET_ADDRESS();		// update AR/ARP
	PUSH_STACK(tms.pc+2);

	delay_slot(tms.pc);
	CHANGE_PC(pma);

	CYCLES(4);
}

static void op_cc(void)
{
	fatalerror("32051: unimplemented op cc at %08X", tms.pc-1);
}

static void op_ccd(void)
{
	UINT16 pma = ROPCODE();
	int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
	int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);

	if (zlvc_condition || tp_condition)
	{
		PUSH_STACK(tms.pc+2);

		delay_slot(tms.pc);
		CHANGE_PC(pma);
	}

	CYCLES(2);
}

static void op_intr(void)
{
	fatalerror("32051: unimplemented op intr at %08X", tms.pc-1);
}

static void op_nmi(void)
{
	fatalerror("32051: unimplemented op nmi at %08X", tms.pc-1);
}

static void op_retc(void)
{
	int condition = 0;

	if ((tms.op & 0x3ff) == 0x300)		// RET
	{
		condition = 1;
	}
	else
	{
		int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
		int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);
		condition = zlvc_condition || tp_condition;
	}

	if (condition)
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

static void op_retcd(void)
{
	int condition = 0;

	if ((tms.op & 0x3ff) == 0x300)		// RETD
	{
		condition = 1;
	}
	else
	{
		int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
		int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);
		condition = zlvc_condition || tp_condition;
	}

	if (condition)
	{
		UINT16 pc = POP_STACK();
		delay_slot(tms.pc);
		CHANGE_PC(pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_rete(void)
{
	UINT16 pc = POP_STACK();
	CHANGE_PC(pc);

	tms.st0.intm = 0;

	restore_interrupt_context();

	CYCLES(4);
}

static void op_reti(void)
{
	fatalerror("32051: unimplemented op reti at %08X", tms.pc-1);
}

static void op_trap(void)
{
	fatalerror("32051: unimplemented op trap at %08X", tms.pc-1);
}

static void op_xc(void)
{
	int n = ((tms.op >> 12) & 0x1) + 1;
	int zlvc_condition = GET_ZLVC_CONDITION((tms.op >> 4) & 0xf, tms.op & 0xf);
	int tp_condition = GET_TP_CONDITION((tms.op >> 8) & 0x3);

	if (zlvc_condition || tp_condition)
	{
		CYCLES(1);
	}
	else
	{
		CHANGE_PC(tms.pc + n);
		CYCLES(1 + n);
	}
}

/*****************************************************************************/

static void op_bldd_slimm(void)
{
	UINT16 pfc = ROPCODE();

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_bldd_dlimm(void)
{
	UINT16 pfc = ROPCODE();

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_bldd_sbmar(void)
{
	fatalerror("32051: unimplemented op bldd sbmar at %08X", tms.pc-1);
}

static void op_bldd_dbmar(void)
{
	UINT16 pfc = tms.bmar;

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_bldp(void)
{
	UINT16 pfc = tms.bmar;

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(1);

		tms.rptc--;
	};
}

static void op_blpd_bmar(void)
{
	fatalerror("32051: unimplemented op bpld bmar at %08X", tms.pc-1);
}

static void op_blpd_imm(void)
{
	UINT16 pfc = ROPCODE();

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

/*****************************************************************************/

static void op_dmov(void)
{
	fatalerror("32051: unimplemented op dmov at %08X", tms.pc-1);
}

static void op_in(void)
{
	fatalerror("32051: unimplemented op in at %08X", tms.pc-1);
}

static void op_lmmr(void)
{
	UINT16 pfc = ROPCODE();

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(pfc);
		DM_WRITE16(ea & 0x7f, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_out(void)
{
	fatalerror("32051: unimplemented op out at %08X", tms.pc-1);
}

static void op_smmr(void)
{
	UINT16 pfc = ROPCODE();

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea & 0x7f);
		DM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_tblr(void)
{
	UINT16 pfc = (UINT16)(tms.acc);

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = PM_READ16(pfc);
		DM_WRITE16(ea, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

static void op_tblw(void)
{
	UINT16 pfc = (UINT16)(tms.acc);

	while (tms.rptc > -1)
	{
		UINT16 ea = GET_ADDRESS();
		UINT16 data = DM_READ16(ea);
		PM_WRITE16(pfc, data);
		pfc++;
		CYCLES(2);

		tms.rptc--;
	};
}

/*****************************************************************************/

static void op_apl_dbmr(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	data &= tms.dbmr;
	DM_WRITE16(ea, data);
	CYCLES(1);
}

static void op_apl_imm(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();
	UINT16 data = DM_READ16(ea);
	data &= imm;
	DM_WRITE16(ea, data);
	CYCLES(1);
}

static void op_cpl_dbmr(void)
{
	fatalerror("32051: unimplemented op cpl dbmr at %08X", tms.pc-1);
}

static void op_cpl_imm(void)
{
	UINT16 imm = ROPCODE();
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	if (data == imm)
	{
		tms.st1.tc = 1;
	}
	else
	{
		tms.st1.tc = 0;
	}

	CYCLES(1);
}

static void op_opl_dbmr(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	data |= tms.dbmr;
	DM_WRITE16(ea, data);
	CYCLES(1);
}

static void op_opl_imm(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();
	UINT16 data = DM_READ16(ea);
	data |= imm;
	DM_WRITE16(ea, data);
	CYCLES(1);
}

static void op_splk(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 imm = ROPCODE();

	DM_WRITE16(ea, imm);

	CYCLES(2);
}

static void op_xpl_dbmr(void)
{
	fatalerror("32051: unimplemented op xpl dbmr at %08X", tms.pc-1);
}

static void op_xpl_imm(void)
{
	fatalerror("32051: unimplemented op xpl imm at %08X", tms.pc-1);
}

static void op_apac(void)
{
	INT32 spreg = PREG_PSCALER(tms.preg);
	tms.acc = ADD(tms.acc, spreg, 0);

	CYCLES(1);
}

static void op_lph(void)
{
	fatalerror("32051: unimplemented op lph at %08X", tms.pc-1);
}

static void op_lt(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	tms.treg0 = data;

	CYCLES(1);
}

static void op_lta(void)
{
	INT32 spreg;
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);

	tms.treg0 = data;
	spreg = PREG_PSCALER(tms.preg);
	tms.acc = ADD(tms.acc, spreg, 0);

	CYCLES(1);
}

static void op_ltd(void)
{
	fatalerror("32051: unimplemented op ltd at %08X", tms.pc-1);
}

static void op_ltp(void)
{
	fatalerror("32051: unimplemented op ltp at %08X", tms.pc-1);
}

static void op_lts(void)
{
	fatalerror("32051: unimplemented op lts at %08X", tms.pc-1);
}

static void op_mac(void)
{
	fatalerror("32051: unimplemented op mac at %08X", tms.pc-1);
}

static void op_macd(void)
{
	fatalerror("32051: unimplemented op macd at %08X", tms.pc-1);
}

static void op_madd(void)
{
	fatalerror("32051: unimplemented op madd at %08X", tms.pc-1);
}

static void op_mads(void)
{
	fatalerror("32051: unimplemented op mads at %08X", tms.pc-1);
}

static void op_mpy_mem(void)
{
	UINT16 ea = GET_ADDRESS();
	INT16 data = DM_READ16(ea);

	tms.preg = (INT32)(data) * (INT32)(INT16)(tms.treg0);

	CYCLES(1);
}

static void op_mpy_simm(void)
{
	fatalerror("32051: unimplemented op mpy simm at %08X", tms.pc-1);
}

static void op_mpy_limm(void)
{
	fatalerror("32051: unimplemented op mpy limm at %08X", tms.pc-1);
}

static void op_mpya(void)
{
	fatalerror("32051: unimplemented op mpya at %08X", tms.pc-1);
}

static void op_mpys(void)
{
	fatalerror("32051: unimplemented op mpys at %08X", tms.pc-1);
}

static void op_mpyu(void)
{
	fatalerror("32051: unimplemented op mpyu at %08X", tms.pc-1);
}

static void op_pac(void)
{
	fatalerror("32051: unimplemented op pac at %08X", tms.pc-1);
}

static void op_spac(void)
{
	fatalerror("32051: unimplemented op spac at %08X", tms.pc-1);
}

static void op_sph(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 spreg = (UINT16)(PREG_PSCALER(tms.preg) >> 16);
	DM_WRITE16(ea, spreg);

	CYCLES(1);
}

static void op_spl(void)
{
	fatalerror("32051: unimplemented op spl at %08X", tms.pc-1);
}

static void op_spm(void)
{
	tms.st1.pm = tms.op & 0x3;

	CYCLES(1);
}

static void op_sqra(void)
{
	fatalerror("32051: unimplemented op sqra at %08X", tms.pc-1);
}

static void op_sqrs(void)
{
	fatalerror("32051: unimplemented op sqrs at %08X", tms.pc-1);
}

static void op_zpr(void)
{
	fatalerror("32051: unimplemented op zpr at %08X", tms.pc-1);
}

static void op_bit(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int bit = 15 - ((tms.op >> 8) & 0xf);

	if (data & (1 << bit))
	{
		tms.st1.tc = 1;
	}
	else
	{
		tms.st1.tc = 0;
	}

	CYCLES(1);
}

static void op_bitt(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	int bit = 15 - (tms.treg2 & 0xf);

	if (data & (1 << bit))
	{
		tms.st1.tc = 1;
	}
	else
	{
		tms.st1.tc = 0;
	}

	CYCLES(1);
}

static void op_clrc_ov(void)
{
	tms.st0.ovm = 0;

	CYCLES(1);
}

static void op_clrc_ext(void)
{
	tms.st1.sxm = 0;

	CYCLES(1);
}

static void op_clrc_hold(void)
{
	fatalerror("32051: unimplemented op clrc hold at %08X", tms.pc-1);
}

static void op_clrc_tc(void)
{
	fatalerror("32051: unimplemented op clrc tc at %08X", tms.pc-1);
}

static void op_clrc_carry(void)
{
	fatalerror("32051: unimplemented op clrc carry at %08X", tms.pc-1);
}

static void op_clrc_cnf(void)
{
	tms.st1.cnf = 0;

	CYCLES(1);
}

static void op_clrc_intm(void)
{
	tms.st0.intm = 0;

	check_interrupts();

	CYCLES(1);
}

static void op_clrc_xf(void)
{
	fatalerror("32051: unimplemented op clrc xf at %08X", tms.pc-1);
}

static void op_idle(void)
{
	fatalerror("32051: unimplemented op idle at %08X", tms.pc-1);
}

static void op_idle2(void)
{
	fatalerror("32051: unimplemented op idle2 at %08X", tms.pc-1);
}

static void op_lst_st0(void)
{
	fatalerror("32051: unimplemented op lst st0 at %08X", tms.pc-1);
}

static void op_lst_st1(void)
{
	fatalerror("32051: unimplemented op lst st1 at %08X", tms.pc-1);
}

static void op_pop(void)
{
	tms.acc = POP_STACK();

	CYCLES(1);
}

static void op_popd(void)
{
	fatalerror("32051: unimplemented op popd at %08X", tms.pc-1);
}

static void op_pshd(void)
{
	fatalerror("32051: unimplemented op pshd at %08X", tms.pc-1);
}

static void op_push(void)
{
	fatalerror("32051: unimplemented op push at %08X", tms.pc-1);
}

static void op_rpt_mem(void)
{
	UINT16 ea = GET_ADDRESS();
	UINT16 data = DM_READ16(ea);
	tms.rptc = data;
	tms.rpt_start = tms.pc;
	tms.rpt_end = tms.pc;

	CYCLES(1);
}

static void op_rpt_limm(void)
{
	tms.rptc = (UINT16)ROPCODE();
	tms.rpt_start = tms.pc;
	tms.rpt_end = tms.pc;

	CYCLES(2);
}

static void op_rpt_simm(void)
{
	tms.rptc = (tms.op & 0xff);
	tms.rpt_start = tms.pc;
	tms.rpt_end = tms.pc;

	CYCLES(1);
}

static void op_rptb(void)
{
	UINT16 pma = ROPCODE();
	tms.pmst.braf = 1;
	tms.pasr = tms.pc;
	tms.paer = pma + 1;

	CYCLES(2);
}

static void op_rptz(void)
{
	fatalerror("32051: unimplemented op rptz at %08X", tms.pc-1);
}

static void op_setc_ov(void)
{
	tms.st0.ovm = 1;

	CYCLES(1);
}

static void op_setc_ext(void)
{
	tms.st1.sxm = 1;

	CYCLES(1);
}

static void op_setc_hold(void)
{
	fatalerror("32051: unimplemented op setc hold at %08X", tms.pc-1);
}

static void op_setc_tc(void)
{
	fatalerror("32051: unimplemented op setc tc at %08X", tms.pc-1);
}

static void op_setc_carry(void)
{
	fatalerror("32051: unimplemented op setc carry at %08X", tms.pc-1);
}

static void op_setc_xf(void)
{
	fatalerror("32051: unimplemented op setc xf at %08X", tms.pc-1);
}

static void op_setc_cnf(void)
{
	tms.st1.cnf = 1;

	CYCLES(1);
}

static void op_setc_intm(void)
{
	tms.st0.intm = 1;

	check_interrupts();

	CYCLES(1);
}

static void op_sst_st0(void)
{
	fatalerror("32051: unimplemented op sst st0 at %08X", tms.pc-1);
}

static void op_sst_st1(void)
{
	fatalerror("32051: unimplemented op sst st1 at %08X", tms.pc-1);
}
