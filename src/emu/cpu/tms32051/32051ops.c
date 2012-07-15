// stack is LIFO and is 8 levels deep, there is no stackpointer on the real chip
INLINE void PUSH_STACK(tms32051_state *cpustate, UINT16 pc)
{
	cpustate->pcstack_ptr = (cpustate->pcstack_ptr - 1) & 7;
	cpustate->pcstack[cpustate->pcstack_ptr] = pc;
}

INLINE UINT16 POP_STACK(tms32051_state *cpustate)
{
	UINT16 pc = cpustate->pcstack[cpustate->pcstack_ptr];
	cpustate->pcstack_ptr = (cpustate->pcstack_ptr + 1) & 7;
	cpustate->pcstack[(cpustate->pcstack_ptr + 7) & 7] = cpustate->pcstack[(cpustate->pcstack_ptr + 6) & 7];
	return pc;
}

INLINE INT32 SUB(tms32051_state *cpustate, UINT32 a, UINT32 b, int shift16)
{
	UINT32 res = a - b;

	// check overflow
	if ((a ^ b) & (a ^ res) & 0x80000000)
	{
		if (cpustate->st0.ovm)	// overflow saturation mode
		{
			res = ((INT32)(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		cpustate->st0.ov = 1;
	}

	// set carry
	if (!shift16)
	{
		// C is cleared if borrow was generated
		cpustate->st1.c = (b > a) ? 0 : 1;
	}
	else
	{
		// if 16-bit shift, C is cleared if borrow was generated, otherwise C is unaffected
		if (b > a)
		{
			cpustate->st1.c = 0;
		}
	}

	return (INT32)(res);
}

INLINE INT32 ADD(tms32051_state *cpustate, UINT32 a, UINT32 b, int shift16)
{
	UINT32 res = a + b;
	
	// check overflow
	if ((a ^ res) & (b ^ res) & 0x80000000)
	{
		if (cpustate->st0.ovm)	// overflow saturation mode
		{
			res = ((INT32)(res) < 0) ? 0x7fffffff : 0x80000000;
		}

		// set OV, this is a sticky flag
		cpustate->st0.ov = 1;
	}

	// check carry
	if (!shift16)
	{
		// C is set if carry was generated
		cpustate->st1.c = (((UINT64)(a) + (UINT64)(b)) & U64(0x100000000)) ? 1 : 0;
	}
	else
	{
		// if 16-bit shift, C is set carry was generated, otherwise C is unaffected
		if (((UINT64)(a) + (UINT64)(b)) & U64(0x100000000))
		{
			cpustate->st1.c = 1;
		}
	}

	return (INT32)(res);
}


INLINE void UPDATE_AR(tms32051_state *cpustate, int ar, int step)
{
	int cenb1 = (cpustate->cbcr >> 3) & 0x1;
	int car1 = cpustate->cbcr & 0x7;
	int cenb2 = (cpustate->cbcr >> 7) & 0x1;
	int car2 = (cpustate->cbcr >> 4) & 0x7;

	if (cenb1 && ar == car1)
	{
		// update circular buffer 1
		if (cpustate->ar[ar] == cpustate->cber1)
		{
			cpustate->ar[ar] = cpustate->cbsr1;
		}
		else
		{
			cpustate->ar[ar] += step;
		}
	}
	else if (cenb2 && ar == car2)
	{
		// update circular buffer 2
		if (cpustate->ar[ar] == cpustate->cber2)
		{
			cpustate->ar[ar] = cpustate->cbsr2;
		}
		else
		{
			cpustate->ar[ar] += step;
		}
	}
	else
	{
		cpustate->ar[ar] += step;
	}
}

INLINE void UPDATE_ARP(tms32051_state *cpustate, int nar)
{
	cpustate->st1.arb = cpustate->st0.arp;
	cpustate->st0.arp = nar;
}

static UINT16 GET_ADDRESS(tms32051_state *cpustate)
{
	if (cpustate->op & 0x80)		// Indirect Addressing
	{
		UINT16 ea;
		int arp = cpustate->st0.arp;
		int nar = cpustate->op & 0x7;

		ea = cpustate->ar[arp];

		switch ((cpustate->op >> 3) & 0xf)
		{
			case 0x0:	// *            (no operation)
			{
				break;
			}
			case 0x1:	// *, ARn       (NAR -> ARP)
			{
				UPDATE_ARP(cpustate, nar);
				break;
			}
			case 0x2:	// *-           ((CurrentAR)-1 -> CurrentAR)
			{
				UPDATE_AR(cpustate, arp, -1);
				break;
			}
			case 0x3:	// *-, ARn      ((CurrentAR)-1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(cpustate, arp, -1);
				UPDATE_ARP(cpustate, nar);
				break;
			}
			case 0x4:	// *+           ((CurrentAR)+1 -> CurrentAR)
			{
				UPDATE_AR(cpustate, arp, 1);
				break;
			}
			case 0x5:	// *+, ARn      ((CurrentAR)+1 -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(cpustate, arp, 1);
				UPDATE_ARP(cpustate, nar);
				break;
			}
			case 0xa:	// *0-          ((CurrentAR) - INDX)
			{
				UPDATE_AR(cpustate, arp, -cpustate->indx);
				break;
			}
			case 0xb:	// *0-, ARn     ((CurrentAR) - INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(cpustate, arp, -cpustate->indx);
				UPDATE_ARP(cpustate, nar);
				break;
			}
			case 0xc:	// *0+          ((CurrentAR) + INDX -> CurrentAR)
			{
				UPDATE_AR(cpustate, arp, cpustate->indx);
				break;
			}
			case 0xd:	// *0+, ARn     ((CurrentAR) + INDX -> CurrentAR, NAR -> ARP)
			{
				UPDATE_AR(cpustate, arp, cpustate->indx);
				UPDATE_ARP(cpustate, nar);
				break;
			}

			default:	fatalerror("32051: GET_ADDRESS: unimplemented indirect addressing mode %d at %04X (%04X)\n", (cpustate->op >> 3) & 0xf, cpustate->pc, cpustate->op);
		}

		return ea;
	}
	else					// Direct Addressing
	{
		return cpustate->st0.dp | (cpustate->op & 0x7f);
	}
}

INLINE int GET_ZLVC_CONDITION(tms32051_state *cpustate, int zlvc, int zlvc_mask)
{
	if (zlvc_mask & 0x2)		// OV-bit
	{
		if ((zlvc & 0x2) && cpustate->st0.ov)							// OV
		{
			// clear OV
			cpustate->st0.ov = 0;

			return 1;
		}
		else if ((zlvc & 0x2) == 0 && cpustate->st0.ov == 0)			// NOV
			return 1;
	}
	if (zlvc_mask & 0x1)		// C-bit
	{
		if ((zlvc & 0x1) && cpustate->st1.c)							// C
			return 1;
		else if ((zlvc & 0x1) == 0 && cpustate->st1.c == 0)			// NC
			return 1;
	}
	if (zlvc_mask & 0x8)		// Z-bit
	{
		if ((zlvc & 0x8) && (INT32)(cpustate->acc) == 0)				// EQ
			return 1;
		else if ((zlvc & 0x8) == 0 && (INT32)(cpustate->acc) != 0)	// NEQ
			return 1;
	}
	if (zlvc_mask & 0x4)		// L-bit
	{
		if ((zlvc & 0x4) && (INT32)(cpustate->acc) < 0)				// LT
			return 1;
		else if ((zlvc & 0x4) == 0 && (INT32)(cpustate->acc) > 0)		// GT
			return 1;
	}
	return 0;
}

INLINE int GET_TP_CONDITION(tms32051_state *cpustate, int tp)
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
			return cpustate->st1.tc;
		}
		case 2:		// TC = 0
		{
			return !cpustate->st1.tc;
		}
		case 3:		// always false
		{
			return 0;
		}
	}
	return 0;
}

INLINE INT32 PREG_PSCALER(tms32051_state *cpustate, INT32 preg)
{
	switch (cpustate->st1.pm & 3)
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



static void op_invalid(tms32051_state *cpustate)
{
	fatalerror("32051: invalid op at %08X", cpustate->pc-1);
}

static void op_group_be(tms32051_state *cpustate);
static void op_group_bf(tms32051_state *cpustate);

/*****************************************************************************/

static void op_abs(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op abs at %08X", cpustate->pc-1);
}

static void op_adcb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op adcb at %08X", cpustate->pc-1);
}

static void op_add_mem(tms32051_state *cpustate)
{
	INT32 d;
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	int shift = (cpustate->op >> 8) & 0xf;

	if (cpustate->st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	cpustate->acc = ADD(cpustate, cpustate->acc, d, 0);

	CYCLES(1);
}

static void op_add_simm(tms32051_state *cpustate)
{
	UINT16 imm = cpustate->op & 0xff;

	cpustate->acc = ADD(cpustate, cpustate->acc, imm, 0);

	CYCLES(1);
}

static void op_add_limm(tms32051_state *cpustate)
{
	INT32 d;
	UINT16 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	if (cpustate->st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	cpustate->acc = ADD(cpustate, cpustate->acc, d, 0);

	CYCLES(2);
}

static void op_add_s16_mem(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op add s16 mem at %08X", cpustate->pc-1);
}

static void op_addb(tms32051_state *cpustate)
{
	cpustate->acc = ADD(cpustate, cpustate->acc, cpustate->accb, 0);

	CYCLES(1);
}

static void op_addc(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op addc at %08X", cpustate->pc-1);
}

static void op_adds(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op adds at %08X", cpustate->pc-1);
}

static void op_addt(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op addt at %08X", cpustate->pc-1);
}

static void op_and_mem(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op and mem at %08X", cpustate->pc-1);
}

static void op_and_limm(tms32051_state *cpustate)
{
	UINT32 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	cpustate->acc &= imm << shift;

	CYCLES(2);
}

static void op_and_s16_limm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op and s16 limm at %08X", cpustate->pc-1);
}

static void op_andb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op andb at %08X", cpustate->pc-1);
}

static void op_bsar(tms32051_state *cpustate)
{
	int shift = (cpustate->op & 0xf) + 1;

	if (cpustate->st1.sxm)
	{
		cpustate->acc = (INT32)(cpustate->acc) >> shift;
	}
	else
	{
		cpustate->acc = (UINT32)(cpustate->acc) >> shift;
	}

	CYCLES(1);
}

static void op_cmpl(tms32051_state *cpustate)
{
	cpustate->acc = ~cpustate->acc;

	CYCLES(1);
}

static void op_crgt(tms32051_state *cpustate)
{
	if (cpustate->acc > cpustate->accb)
	{
		cpustate->accb = cpustate->acc;
		cpustate->st1.c = 1;
	}
	else if (cpustate->acc < cpustate->accb)
	{
		cpustate->acc = cpustate->accb;
		cpustate->st1.c = 0;
	}
	else
	{
		cpustate->st1.c = 1;
	}

	CYCLES(1);
}

static void op_crlt(tms32051_state *cpustate)
{
	if (cpustate->acc < cpustate->accb)
	{
		cpustate->accb = cpustate->acc;
		cpustate->st1.c = 1;
	}
	else if (cpustate->acc > cpustate->accb)
	{
		cpustate->acc = cpustate->accb;
		cpustate->st1.c = 0;
	}
	else
	{
		cpustate->st1.c = 0;
	}

	CYCLES(1);
}

static void op_exar(tms32051_state *cpustate)
{
	INT32 tmp = cpustate->acc;
	cpustate->acc = cpustate->accb;
	cpustate->accb = tmp;

	CYCLES(1);
}

static void op_lacb(tms32051_state *cpustate)
{
	cpustate->acc = cpustate->accb;

	CYCLES(1);
}

static void op_lacc_mem(tms32051_state *cpustate)
{
	int shift = (cpustate->op >> 8) & 0xf;
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	if (cpustate->st1.sxm)
	{
		cpustate->acc = (INT32)(INT16)(data) << shift;
	}
	else
	{
		cpustate->acc = (UINT32)(UINT16)(data) << shift;
	}

	CYCLES(1);
}

static void op_lacc_limm(tms32051_state *cpustate)
{
	UINT16 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	if (cpustate->st1.sxm)
	{
		cpustate->acc = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		cpustate->acc = (UINT32)(UINT16)(imm) << shift;
	}

	CYCLES(1);
}

static void op_lacc_s16_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);

	if (cpustate->st1.sxm)
	{
		cpustate->acc = (INT32)(INT16)(DM_READ16(cpustate, ea)) << 16;
	}
	else
	{
		cpustate->acc = (UINT32)(DM_READ16(cpustate, ea)) << 16;
	}

	CYCLES(1);
}

static void op_lacl_simm(tms32051_state *cpustate)
{
	cpustate->acc = cpustate->op & 0xff;

	CYCLES(1);
}

static void op_lacl_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	cpustate->acc = DM_READ16(cpustate, ea) & 0xffff;

	CYCLES(1);
}

static void op_lact(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op lact at %08X", cpustate->pc-1);
}

static void op_lamm(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	ea &= 0x7f;

	cpustate->acc = DM_READ16(cpustate, ea) & 0xffff;
	CYCLES(1);
}

static void op_neg(tms32051_state *cpustate)
{
	if ((UINT32)(cpustate->acc) == 0x80000000)
	{
		cpustate->st0.ov = 1;
		cpustate->st1.c = 0;
		cpustate->acc = (cpustate->st0.ovm) ? 0x7fffffff : 0x80000000;
	}
	else
	{
		cpustate->acc = 0 - (UINT32)(cpustate->acc);
		cpustate->st1.c = (cpustate->acc == 0) ? 1 : 0;
	}

	CYCLES(1);
}

static void op_norm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op norm at %08X", cpustate->pc-1);
}

static void op_or_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	cpustate->acc |= (UINT32)(data);

	CYCLES(1);
}

static void op_or_limm(tms32051_state *cpustate)
{
	UINT32 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	cpustate->acc |= imm << shift;

	CYCLES(1);
}

static void op_or_s16_limm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op or s16 limm at %08X", cpustate->pc-1);
}

static void op_orb(tms32051_state *cpustate)
{
	cpustate->acc |= cpustate->accb;

	CYCLES(1);
}

static void op_rol(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op rol at %08X", cpustate->pc-1);
}

static void op_rolb(tms32051_state *cpustate)
{
	UINT32 acc = cpustate->acc;
	UINT32 accb = cpustate->accb;
	UINT32 c = cpustate->st1.c & 1;

	cpustate->acc = (acc << 1) | ((accb >> 31) & 1);
	cpustate->accb = (accb << 1) | c;
	cpustate->st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

static void op_ror(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op ror at %08X", cpustate->pc-1);
}

static void op_rorb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op rorb at %08X", cpustate->pc-1);
}

static void op_sacb(tms32051_state *cpustate)
{
	cpustate->accb = cpustate->acc;

	CYCLES(1);
}

static void op_sach(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	int shift = (cpustate->op >> 8) & 0x7;

	DM_WRITE16(cpustate, ea, (UINT16)((cpustate->acc << shift) >> 16));
	CYCLES(1);
}

static void op_sacl(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	int shift = (cpustate->op >> 8) & 0x7;

	DM_WRITE16(cpustate, ea, (UINT16)(cpustate->acc << shift));
	CYCLES(1);
}

static void op_samm(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	ea &= 0x7f;

	DM_WRITE16(cpustate, ea, (UINT16)(cpustate->acc));
	CYCLES(1);
}

static void op_sath(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sath at %08X", cpustate->pc-1);
}

static void op_satl(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op satl at %08X", cpustate->pc-1);
}

static void op_sbb(tms32051_state *cpustate)
{
	cpustate->acc = SUB(cpustate, cpustate->acc, cpustate->accb, 0);

	CYCLES(1);
}

static void op_sbbb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sbbb at %08X", cpustate->pc-1);
}

static void op_sfl(tms32051_state *cpustate)
{
	cpustate->st1.c = (cpustate->acc >> 31) & 1;
	cpustate->acc = cpustate->acc << 1;

	CYCLES(1);
}

static void op_sflb(tms32051_state *cpustate)
{
	UINT32 acc = cpustate->acc;
	UINT32 accb = cpustate->accb;

	cpustate->acc = (acc << 1) | ((accb >> 31) & 1);
	cpustate->accb = (accb << 1);
	cpustate->st1.c = (acc >> 31) & 1;

	CYCLES(1);
}

static void op_sfr(tms32051_state *cpustate)
{
	cpustate->st1.c = cpustate->acc & 1;

	if (cpustate->st1.sxm)
	{
		cpustate->acc = (INT32)(cpustate->acc) >> 1;
	}
	else
	{
		cpustate->acc = (UINT32)(cpustate->acc) >> 1;
	}

	CYCLES(1);
}

static void op_sfrb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sfrb at %08X", cpustate->pc-1);
}

static void op_sub_mem(tms32051_state *cpustate)
{
	INT32 d;
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	int shift = (cpustate->op >> 8) & 0xf;

	if (cpustate->st1.sxm)
	{
		d = (INT32)(INT16)(data) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(data) << shift;
	}

	cpustate->acc = SUB(cpustate, cpustate->acc, d, 0);

	CYCLES(1);
}

static void op_sub_s16_mem(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sub s16 mem at %08X", cpustate->pc-1);
}

static void op_sub_simm(tms32051_state *cpustate)
{
	UINT16 imm = cpustate->op & 0xff;

	cpustate->acc = SUB(cpustate, cpustate->acc, imm, 0);

	CYCLES(1);
}

static void op_sub_limm(tms32051_state *cpustate)
{
	INT32 d;
	UINT16 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	if (cpustate->st1.sxm)
	{
		d = (INT32)(INT16)(imm) << shift;
	}
	else
	{
		d = (UINT32)(UINT16)(imm) << shift;
	}

	cpustate->acc = SUB(cpustate, cpustate->acc, d, 0);

	CYCLES(2);
}

static void op_subb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op subb at %08X", cpustate->pc-1);
}

static void op_subc(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op subc at %08X", cpustate->pc-1);
}

static void op_subs(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op subs at %08X", cpustate->pc-1);
}

static void op_subt(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op subt at %08X", cpustate->pc-1);
}

static void op_xor_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	cpustate->acc ^= (UINT32)(data);

	CYCLES(1);
}

static void op_xor_limm(tms32051_state *cpustate)
{
	UINT32 imm = ROPCODE(cpustate);
	int shift = cpustate->op & 0xf;

	cpustate->acc ^= imm << shift;

	CYCLES(1);
}

static void op_xor_s16_limm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op xor s16 limm at %08X", cpustate->pc-1);
}

static void op_xorb(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op xorb at %08X", cpustate->pc-1);
}

static void op_zalr(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op zalr at %08X", cpustate->pc-1);
}

static void op_zap(tms32051_state *cpustate)
{
	cpustate->acc = 0;
	cpustate->preg = 0;

	CYCLES(1);
}

/*****************************************************************************/

static void op_adrk(tms32051_state *cpustate)
{
	UINT16 imm = cpustate->op & 0xff;
	UPDATE_AR(cpustate, cpustate->st0.arp, imm);

	CYCLES(1);
}

static void op_cmpr(tms32051_state *cpustate)
{
	cpustate->st1.tc = 0;

	switch (cpustate->op & 0x3)
	{
		case 0:			// (CurrentAR) == ARCR
		{
			if (cpustate->ar[cpustate->st0.arp] == cpustate->arcr)
			{
				cpustate->st1.tc = 1;
			}
			break;
		}
		case 1:			// (CurrentAR) < ARCR
		{
			if (cpustate->ar[cpustate->st0.arp] < cpustate->arcr)
			{
				cpustate->st1.tc = 1;
			}
			break;
		}
		case 2:			// (CurrentAR) > ARCR
		{
			if (cpustate->ar[cpustate->st0.arp] > cpustate->arcr)
			{
				cpustate->st1.tc = 1;
			}
			break;
		}
		case 3:			// (CurrentAR) != ARCR
		{
			if (cpustate->ar[cpustate->st0.arp] != cpustate->arcr)
			{
				cpustate->st1.tc = 1;
			}
			break;
		}
	}

	CYCLES(1);
}

static void op_lar_mem(tms32051_state *cpustate)
{
	int arx = (cpustate->op >> 8) & 0x7;
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	cpustate->ar[arx] = data;

	CYCLES(2);
}

static void op_lar_simm(tms32051_state *cpustate)
{
	int arx = (cpustate->op >> 8) & 0x7;
	cpustate->ar[arx] = cpustate->op & 0xff;

	CYCLES(2);
}

static void op_lar_limm(tms32051_state *cpustate)
{
	int arx = cpustate->op & 0x7;
	UINT16 imm = ROPCODE(cpustate);
	cpustate->ar[arx] = imm;

	CYCLES(2);
}

static void op_ldp_mem(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op ldp mem at %08X", cpustate->pc-1);
}

static void op_ldp_imm(tms32051_state *cpustate)
{
	cpustate->st0.dp = (cpustate->op & 0x1ff) << 7;
	CYCLES(2);
}

static void op_mar(tms32051_state *cpustate)
{
	// direct addressing is NOP
	if (cpustate->op & 0x80)
	{
		GET_ADDRESS(cpustate);
	}
	CYCLES(1);
}

static void op_sar(tms32051_state *cpustate)
{
	int arx = (cpustate->op >> 8) & 0x7;
	UINT16 ar = cpustate->ar[arx];
	UINT16 ea = GET_ADDRESS(cpustate);
	DM_WRITE16(cpustate, ea, ar);

	CYCLES(1);
}

static void op_sbrk(tms32051_state *cpustate)
{
	UINT16 imm = cpustate->op & 0xff;
	UPDATE_AR(cpustate, cpustate->st0.arp, -imm);

	CYCLES(1);
}

/*****************************************************************************/

static void op_b(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);
	GET_ADDRESS(cpustate);		// update AR/ARP

	CHANGE_PC(cpustate, pma);
	CYCLES(4);
}

static void op_bacc(tms32051_state *cpustate)
{
	CHANGE_PC(cpustate, (UINT16)(cpustate->acc));

	CYCLES(4);
}

static void op_baccd(tms32051_state *cpustate)
{
	UINT16 pc = (UINT16)(cpustate->acc);

	delay_slot(cpustate, cpustate->pc);
	CHANGE_PC(cpustate, pc);

	CYCLES(2);
}

static void op_banz(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);

	if (cpustate->ar[cpustate->st0.arp] != 0)
	{
		CHANGE_PC(cpustate, pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}

	GET_ADDRESS(cpustate);		// modify AR/ARP
}

static void op_banzd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op banzd at %08X", cpustate->pc-1);
}

static void op_bcnd(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);

	if (GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		CHANGE_PC(cpustate, pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_bcndd(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);

	if (GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		delay_slot(cpustate, cpustate->pc);
		CHANGE_PC(cpustate, pma);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_bd(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);
	GET_ADDRESS(cpustate);		// update AR/ARP

	delay_slot(cpustate, cpustate->pc);
	CHANGE_PC(cpustate, pma);
	CYCLES(2);
}

static void op_cala(tms32051_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->pc);

	CHANGE_PC(cpustate, cpustate->acc);

	CYCLES(4);
}

static void op_calad(tms32051_state *cpustate)
{
	UINT16 pma = cpustate->acc;
	PUSH_STACK(cpustate, cpustate->pc+2);

	delay_slot(cpustate, cpustate->pc);
	CHANGE_PC(cpustate, pma);

	CYCLES(4);
}

static void op_call(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);
	GET_ADDRESS(cpustate);		// update AR/ARP
	PUSH_STACK(cpustate, cpustate->pc);

	CHANGE_PC(cpustate, pma);

	CYCLES(4);
}

static void op_calld(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);
	GET_ADDRESS(cpustate);		// update AR/ARP
	PUSH_STACK(cpustate, cpustate->pc+2);

	delay_slot(cpustate, cpustate->pc);
	CHANGE_PC(cpustate, pma);

	CYCLES(4);
}

static void op_cc(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op cc at %08X", cpustate->pc-1);
}

static void op_ccd(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);

	if (GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		PUSH_STACK(cpustate, cpustate->pc+2);

		delay_slot(cpustate, cpustate->pc);
		CHANGE_PC(cpustate, pma);
	}

	CYCLES(2);
}

static void op_intr(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op intr at %08X", cpustate->pc-1);
}

static void op_nmi(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op nmi at %08X", cpustate->pc-1);
}

static void op_retc(tms32051_state *cpustate)
{
	if ((cpustate->op & 0x3ff) == 0x300 || GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		UINT16 pc = POP_STACK(cpustate);
		CHANGE_PC(cpustate, pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_retcd(tms32051_state *cpustate)
{
	if ((cpustate->op & 0x3ff) == 0x300 || GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		UINT16 pc = POP_STACK(cpustate);
		delay_slot(cpustate, cpustate->pc);
		CHANGE_PC(cpustate, pc);
		CYCLES(4);
	}
	else
	{
		CYCLES(2);
	}
}

static void op_rete(tms32051_state *cpustate)
{
	UINT16 pc = POP_STACK(cpustate);
	CHANGE_PC(cpustate, pc);

	cpustate->st0.intm = 0;

	restore_interrupt_context(cpustate);

	CYCLES(4);
}

static void op_reti(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op reti at %08X", cpustate->pc-1);
}

static void op_trap(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op trap at %08X", cpustate->pc-1);
}

static void op_xc(tms32051_state *cpustate)
{
	if (GET_ZLVC_CONDITION(cpustate, (cpustate->op >> 4) & 0xf, cpustate->op & 0xf) || GET_TP_CONDITION(cpustate, (cpustate->op >> 8) & 0x3))
	{
		CYCLES(1);
	}
	else
	{
		int n = ((cpustate->op >> 12) & 0x1) + 1;
		CHANGE_PC(cpustate, cpustate->pc + n);
		CYCLES(1 + n);
	}
}

/*****************************************************************************/

static void op_bldd_slimm(tms32051_state *cpustate)
{
	UINT16 pfc = ROPCODE(cpustate);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, pfc);
		DM_WRITE16(cpustate, ea, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_bldd_dlimm(tms32051_state *cpustate)
{
	UINT16 pfc = ROPCODE(cpustate);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, ea);
		DM_WRITE16(cpustate, pfc, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_bldd_sbmar(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op bldd sbmar at %08X", cpustate->pc-1);
}

static void op_bldd_dbmar(tms32051_state *cpustate)
{
	UINT16 pfc = cpustate->bmar;

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, ea);
		DM_WRITE16(cpustate, pfc, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_bldp(tms32051_state *cpustate)
{
	UINT16 pfc = cpustate->bmar;

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, ea);
		PM_WRITE16(cpustate, pfc, data);
		pfc++;
		CYCLES(1);

		cpustate->rptc--;
	};
}

static void op_blpd_bmar(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op bpld bmar at %08X", cpustate->pc-1);
}

static void op_blpd_imm(tms32051_state *cpustate)
{
	UINT16 pfc = ROPCODE(cpustate);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = PM_READ16(cpustate, pfc);
		DM_WRITE16(cpustate, ea, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

/*****************************************************************************/

static void op_dmov(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op dmov at %08X", cpustate->pc-1);
}

static void op_in(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op in at %08X", cpustate->pc-1);
}

static void op_lmmr(tms32051_state *cpustate)
{
	UINT16 pfc = ROPCODE(cpustate);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, pfc);
		DM_WRITE16(cpustate, ea & 0x7f, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_out(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op out at %08X", cpustate->pc-1);
}

static void op_smmr(tms32051_state *cpustate)
{
	UINT16 pfc = ROPCODE(cpustate);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, ea & 0x7f);
		DM_WRITE16(cpustate, pfc, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_tblr(tms32051_state *cpustate)
{
	UINT16 pfc = (UINT16)(cpustate->acc);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = PM_READ16(cpustate, pfc);
		DM_WRITE16(cpustate, ea, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

static void op_tblw(tms32051_state *cpustate)
{
	UINT16 pfc = (UINT16)(cpustate->acc);

	while (cpustate->rptc > -1)
	{
		UINT16 ea = GET_ADDRESS(cpustate);
		UINT16 data = DM_READ16(cpustate, ea);
		PM_WRITE16(cpustate, pfc, data);
		pfc++;
		CYCLES(2);

		cpustate->rptc--;
	};
}

/*****************************************************************************/

static void op_apl_dbmr(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	data &= cpustate->dbmr;
	DM_WRITE16(cpustate, ea, data);
	CYCLES(1);
}

static void op_apl_imm(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 imm = ROPCODE(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	data &= imm;
	DM_WRITE16(cpustate, ea, data);
	CYCLES(1);
}

static void op_cpl_dbmr(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op cpl dbmr at %08X", cpustate->pc-1);
}

static void op_cpl_imm(tms32051_state *cpustate)
{
	UINT16 imm = ROPCODE(cpustate);
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	if (data == imm)
	{
		cpustate->st1.tc = 1;
	}
	else
	{
		cpustate->st1.tc = 0;
	}

	CYCLES(1);
}

static void op_opl_dbmr(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	data |= cpustate->dbmr;
	DM_WRITE16(cpustate, ea, data);
	CYCLES(1);
}

static void op_opl_imm(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 imm = ROPCODE(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	data |= imm;
	DM_WRITE16(cpustate, ea, data);
	CYCLES(1);
}

static void op_splk(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 imm = ROPCODE(cpustate);

	DM_WRITE16(cpustate, ea, imm);

	CYCLES(2);
}

static void op_xpl_dbmr(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op xpl dbmr at %08X", cpustate->pc-1);
}

static void op_xpl_imm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op xpl imm at %08X", cpustate->pc-1);
}

static void op_apac(tms32051_state *cpustate)
{
	INT32 spreg = PREG_PSCALER(cpustate, cpustate->preg);
	cpustate->acc = ADD(cpustate, cpustate->acc, spreg, 0);

	CYCLES(1);
}

static void op_lph(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op lph at %08X", cpustate->pc-1);
}

static void op_lt(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	cpustate->treg0 = data;

	CYCLES(1);
}

static void op_lta(tms32051_state *cpustate)
{
	INT32 spreg;
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);

	cpustate->treg0 = data;
	spreg = PREG_PSCALER(cpustate, cpustate->preg);
	cpustate->acc = ADD(cpustate, cpustate->acc, spreg, 0);

	CYCLES(1);
}

static void op_ltd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op ltd at %08X", cpustate->pc-1);
}

static void op_ltp(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op ltp at %08X", cpustate->pc-1);
}

static void op_lts(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op lts at %08X", cpustate->pc-1);
}

static void op_mac(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mac at %08X", cpustate->pc-1);
}

static void op_macd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op macd at %08X", cpustate->pc-1);
}

static void op_madd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op madd at %08X", cpustate->pc-1);
}

static void op_mads(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mads at %08X", cpustate->pc-1);
}

static void op_mpy_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	INT16 data = DM_READ16(cpustate, ea);

	cpustate->preg = (INT32)(data) * (INT32)(INT16)(cpustate->treg0);

	CYCLES(1);
}

static void op_mpy_simm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mpy simm at %08X", cpustate->pc-1);
}

static void op_mpy_limm(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mpy limm at %08X", cpustate->pc-1);
}

static void op_mpya(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mpya at %08X", cpustate->pc-1);
}

static void op_mpys(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mpys at %08X", cpustate->pc-1);
}

static void op_mpyu(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op mpyu at %08X", cpustate->pc-1);
}

static void op_pac(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op pac at %08X", cpustate->pc-1);
}

static void op_spac(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op spac at %08X", cpustate->pc-1);
}

static void op_sph(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 spreg = (UINT16)(PREG_PSCALER(cpustate, cpustate->preg) >> 16);
	DM_WRITE16(cpustate, ea, spreg);

	CYCLES(1);
}

static void op_spl(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op spl at %08X", cpustate->pc-1);
}

static void op_spm(tms32051_state *cpustate)
{
	cpustate->st1.pm = cpustate->op & 0x3;

	CYCLES(1);
}

static void op_sqra(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sqra at %08X", cpustate->pc-1);
}

static void op_sqrs(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sqrs at %08X", cpustate->pc-1);
}

static void op_zpr(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op zpr at %08X", cpustate->pc-1);
}

static void op_bit(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	int bit = 15 - ((cpustate->op >> 8) & 0xf);

	if (data & (1 << bit))
	{
		cpustate->st1.tc = 1;
	}
	else
	{
		cpustate->st1.tc = 0;
	}

	CYCLES(1);
}

static void op_bitt(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	int bit = 15 - (cpustate->treg2 & 0xf);

	if (data & (1 << bit))
	{
		cpustate->st1.tc = 1;
	}
	else
	{
		cpustate->st1.tc = 0;
	}

	CYCLES(1);
}

static void op_clrc_ov(tms32051_state *cpustate)
{
	cpustate->st0.ovm = 0;

	CYCLES(1);
}

static void op_clrc_ext(tms32051_state *cpustate)
{
	cpustate->st1.sxm = 0;

	CYCLES(1);
}

static void op_clrc_hold(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op clrc hold at %08X", cpustate->pc-1);
}

static void op_clrc_tc(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op clrc tc at %08X", cpustate->pc-1);
}

static void op_clrc_carry(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op clrc carry at %08X", cpustate->pc-1);
}

static void op_clrc_cnf(tms32051_state *cpustate)
{
	cpustate->st1.cnf = 0;

	CYCLES(1);
}

static void op_clrc_intm(tms32051_state *cpustate)
{
	cpustate->st0.intm = 0;

	check_interrupts(cpustate);

	CYCLES(1);
}

static void op_clrc_xf(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op clrc xf at %08X", cpustate->pc-1);
}

static void op_idle(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op idle at %08X", cpustate->pc-1);
}

static void op_idle2(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op idle2 at %08X", cpustate->pc-1);
}

static void op_lst_st0(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op lst st0 at %08X", cpustate->pc-1);
}

static void op_lst_st1(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op lst st1 at %08X", cpustate->pc-1);
}

static void op_pop(tms32051_state *cpustate)
{
	cpustate->acc = POP_STACK(cpustate);

	CYCLES(1);
}

static void op_popd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op popd at %08X", cpustate->pc-1);
}

static void op_pshd(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op pshd at %08X", cpustate->pc-1);
}

static void op_push(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op push at %08X", cpustate->pc-1);
}

static void op_rpt_mem(tms32051_state *cpustate)
{
	UINT16 ea = GET_ADDRESS(cpustate);
	UINT16 data = DM_READ16(cpustate, ea);
	cpustate->rptc = data;
	cpustate->rpt_start = cpustate->pc;
	cpustate->rpt_end = cpustate->pc;

	CYCLES(1);
}

static void op_rpt_limm(tms32051_state *cpustate)
{
	cpustate->rptc = (UINT16)ROPCODE(cpustate);
	cpustate->rpt_start = cpustate->pc;
	cpustate->rpt_end = cpustate->pc;

	CYCLES(2);
}

static void op_rpt_simm(tms32051_state *cpustate)
{
	cpustate->rptc = (cpustate->op & 0xff);
	cpustate->rpt_start = cpustate->pc;
	cpustate->rpt_end = cpustate->pc;

	CYCLES(1);
}

static void op_rptb(tms32051_state *cpustate)
{
	UINT16 pma = ROPCODE(cpustate);
	cpustate->pmst.braf = 1;
	cpustate->pasr = cpustate->pc;
	cpustate->paer = pma + 1;

	CYCLES(2);
}

static void op_rptz(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op rptz at %08X", cpustate->pc-1);
}

static void op_setc_ov(tms32051_state *cpustate)
{
	cpustate->st0.ovm = 1;

	CYCLES(1);
}

static void op_setc_ext(tms32051_state *cpustate)
{
	cpustate->st1.sxm = 1;

	CYCLES(1);
}

static void op_setc_hold(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op setc hold at %08X", cpustate->pc-1);
}

static void op_setc_tc(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op setc tc at %08X", cpustate->pc-1);
}

static void op_setc_carry(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op setc carry at %08X", cpustate->pc-1);
}

static void op_setc_xf(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op setc xf at %08X", cpustate->pc-1);
}

static void op_setc_cnf(tms32051_state *cpustate)
{
	cpustate->st1.cnf = 1;

	CYCLES(1);
}

static void op_setc_intm(tms32051_state *cpustate)
{
	cpustate->st0.intm = 1;

	check_interrupts(cpustate);

	CYCLES(1);
}

static void op_sst_st0(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sst st0 at %08X", cpustate->pc-1);
}

static void op_sst_st1(tms32051_state *cpustate)
{
	fatalerror("32051: unimplemented op sst st1 at %08X", cpustate->pc-1);
}
