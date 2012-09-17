/***************************************************************************

    am29ops.h
    Am29000 instructions

***************************************************************************/

/***************************************************************************
    DEFINES
***************************************************************************/

#define IFLAG_ILLEGAL				(1 << 0)
#define IFLAG_SUPERVISOR_ONLY		(1 << 1)
#define IFLAG_RA_PRESENT			(1 << 2)
#define IFLAG_RB_PRESENT			(1 << 3)
#define IFLAG_RC_PRESENT			(1 << 4)

#define IFLAG_SPR_ACCESS			(1 << 6)
#define IFLAG_MEMORY_ACCESS			(1 << 8)
#define IFLAG_CONTROL				(1 << 9)


#define GET_RA_VAL					(am29000->r[RA])
#define GET_RB_VAL					(am29000->r[RB])

#define RA							(get_abs_reg(am29000, am29000->exec_ir >>  8, am29000->ipa))
#define RB							(get_abs_reg(am29000, am29000->exec_ir >>  0, am29000->ipb))
#define RC							(get_abs_reg(am29000, am29000->exec_ir >> 16, am29000->ipc))

#define INST_SA						((am29000->exec_ir >> 8) & 0xff)
#define INST_VN						((am29000->exec_ir >> 16) & 0xff)
#define INST_M_BIT					(am29000->exec_ir & (1 << 24))
#define INST_CE_BIT					(am29000->exec_ir & (1 << 23))
#define INST_AS_BIT					(am29000->exec_ir & (1 << 22))
#define INST_PA_BIT					(am29000->exec_ir & (1 << 21))
#define INST_SB_BIT					(am29000->exec_ir & (1 << 20))
#define INST_UA_BIT					(am29000->exec_ir & (1 << 19))
#define INST_OPT_MASK				(7)
#define INST_OPT_SHIFT				(16)
#define INST_OPT_FIELD				(((am29000->exec_ir) >> INST_OPT_SHIFT) & INST_OPT_MASK)
#define INST_CNTL_MASK				(0x7f)
#define INST_CNTL_SHIFT				(16)

#define I8							(am29000->exec_ir & 0xff)
#define I16							(((am29000->exec_ir >> 8) & 0xff00) | (am29000->exec_ir & 0xff))
#define I16_ZEX						((UINT32)(I16))
#define I16_SEX						((INT32)(INT16)I16)
#define I16_OEX						(0xffff0000 | I16)

#define JMP_ZEX						(I16 << 2)
#define JMP_SEX						((INT32)(INT16)(((am29000->exec_ir >> 8) & 0xff00) | (am29000->exec_ir & 0xff)) << 2)

#define BOOLEAN_MASK				(1 << 31)
#define BOOLEAN_TRUE				(1 << 31)
#define BOOLEAN_FALSE				(0)

#define UNHANDLED_OP				fatalerror("Am29000: Unhandled inst %s at %x\n", __FUNCTION__, am29000->exec_pc);


/***************************************************************************
    STRUCTS
***************************************************************************/

struct op_info
{
	void (*opcode)(am29000_state *);
	UINT32 flags;
};


/***************************************************************************
    ALU FLAG CALCULATION
***************************************************************************/

#define SET_ALU_Z(r)			am29000->alu &= ~ALU_Z; \
								am29000->alu |= (r == 0) << ALU_Z_SHIFT;

#define SET_ALU_N(r)			am29000->alu &= ~ALU_N; \
								am29000->alu |= ((UINT32)r & 0x80000000) >> (31 - ALU_N_SHIFT);

#define CALC_C_ADD(r, a)		((UINT32)(r) < (UINT32)(a))

#define SET_ALU_C_ADD(r, a)		am29000->alu &= ~ALU_C; \
								am29000->alu |= CALC_C_ADD(r, a) << ALU_C_SHIFT;

#define CALC_C_SUB(a, b)		(!((UINT32)(a) < (UINT32)(b)))

#define SET_ALU_C_SUB(a, b)		am29000->alu &= ~ALU_C; \
								am29000->alu |= CALC_C_SUB(a, b) << ALU_C_SHIFT;

#define SET_ALU_V_ADD(r, a, b)	am29000->alu &= ~ALU_V; \
								am29000->alu |= (((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0)) << ALU_V_SHIFT;

#define SET_ALU_V_SUB(r, a, b)	am29000->alu &= ~ALU_V; \
								am29000->alu |= ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0) << ALU_V_SHIFT;

#define GET_CARRY				((am29000->alu >> ALU_C_SHIFT) & 1)



static UINT32 read_spr(am29000_state *am29000, UINT32 idx)
{
	UINT32 val = 0;

	switch (idx)
	{
		case SPR_VAB:	val = am29000->vab;		break;
		case SPR_OPS:	val = am29000->ops;		break;
		case SPR_CPS:	val = am29000->cps;		break;
		case SPR_CFG:	val = am29000->cfg;		break;
		case SPR_CHA:	val = am29000->cha;		break;
		case SPR_CHD:	val = am29000->chd;		break;
		case SPR_CHC:	val = am29000->chc;		break;
		case SPR_RBP:	val = am29000->rbp;		break;
		case SPR_TMC:	val = am29000->tmc;		break;
		case SPR_TMR:	val = am29000->tmr;		break;
		case SPR_PC0:	val = am29000->pc0;		break;
		case SPR_PC1:	val = am29000->pc1;		break;
		case SPR_PC2:	val = am29000->pc2;		break;
		case SPR_MMU:	val = am29000->mmu;		break;
		case SPR_LRU:	val = am29000->lru;		break;
		case SPR_IPC:	val = am29000->ipc;		break;
		case SPR_IPA:	val = am29000->ipa;		break;
		case SPR_IPB:	val = am29000->ipb;		break;
		case SPR_Q:		val = am29000->q;		break;
		case SPR_ALU:	val = am29000->alu;		break;
		case SPR_BP:	val = GET_ALU_BP;		break;
		case SPR_FC:	val = GET_ALU_FC;		break;
		case SPR_CR:	val = GET_CHC_CR;		break;
		case SPR_FPE:	val = am29000->fpe;		break;
		case SPR_INTE:	val = am29000->inte;	break;
		case SPR_FPS:	val = am29000->fps;		break;
		default:
			logerror("Unknown SPR read (%d)\n", idx);
	}

	return val;
}


static void write_spr(am29000_state *am29000, UINT32 idx, UINT32 val)
{
	switch (idx)
	{
		case SPR_VAB:	am29000->vab = val & (VAB_MASK << VAB_SHIFT);
						break;
		case SPR_OPS:	am29000->ops = val & (CPS_CA | CPS_IP | CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_RE |
						CPS_WM | CPS_PD | CPS_PI | CPS_SM | (CPS_IM_MASK << CPS_IM_SHIFT) | CPS_DI | CPS_DA);
						break;
		case SPR_CPS:	am29000->cps = val & (CPS_CA | CPS_IP | CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_RE |
						CPS_WM | CPS_PD | CPS_PI | CPS_SM | (CPS_IM_MASK << CPS_IM_SHIFT) | CPS_DI | CPS_DA);
						break;
		case SPR_CFG:	am29000->cfg = val & (CFG_DW | CFG_VF | CFG_RV | CFG_BO | CFG_CP | CFG_CD);
						am29000->cfg |= PROCESSOR_REL_FIELD << CFG_PRL_SHIFT;
						break;
		case SPR_CHA:	am29000->cha = val;
						break;
		case SPR_CHD:	am29000->chd = val;
						break;
		case SPR_CHC:	am29000->chc = val;
						break;
		case SPR_RBP:	am29000->rbp = val & RBP_MASK;
						break;
		case SPR_TMC:	am29000->tmc = val & TCV_MASK;
						break;
		case SPR_TMR:	am29000->tmr = val & (TMR_OV | TMR_IN | TMR_IE | TMR_TRV_MASK);
						break;
		case SPR_PC0:	am29000->pc0 = val & PC_MASK;
						break;
		case SPR_PC1:	am29000->pc1 = val & PC_MASK;
						break;
		case SPR_PC2:	am29000->pc2 = val & PC_MASK;
						break;
		case SPR_MMU:	am29000->mmu = val & ((MMU_PS_MASK << MMU_PS_SHIFT) | MMU_PID_MASK);
						break;
		case SPR_LRU:	am29000->lru = val & (LRU_MASK << LRU_SHIFT);
						break;
		case SPR_IPC:	am29000->ipc = val;// & IPX_MASK;
						break;
		case SPR_IPA:	am29000->ipa = val;// & IPX_MASK;
						break;
		case SPR_IPB:	am29000->ipb = val;// & IPX_MASK;
						break;
		case SPR_Q:		am29000->q = val;
						break;
		case SPR_ALU:	am29000->alu = val & (ALU_DF | ALU_V | ALU_N | ALU_Z | ALU_C | (ALU_BP_MASK << ALU_BP_SHIFT) | (ALU_FC_MASK << ALU_FC_SHIFT));
						break;
		case SPR_BP:	am29000->alu &= ~(ALU_BP_MASK << ALU_BP_SHIFT);
						am29000->alu |= (val & ALU_BP_MASK) << ALU_BP_SHIFT;
						break;
		case SPR_FC:	am29000->alu &= ~(ALU_FC_MASK << ALU_FC_SHIFT);
						am29000->alu |= (val & ALU_FC_MASK) << ALU_FC_SHIFT;
						break;
		case SPR_CR:	am29000->chc &= ~(CHC_CR_MASK << CHC_CR_SHIFT);
						am29000->chc |= (val & CHC_CR_MASK) << CHC_CR_SHIFT;
						break;
//      case SPR_FPE:
//      case SPR_INTE:
//      case SPR_FPS:
		default: logerror("Unhandled SPR write (%d)\n", idx);
	}
}


/***************************************************************************
    INSTRUCTIONS
***************************************************************************/

/***************************************************************************
    INTEGER ARITHMETIC
***************************************************************************/

static void ADD(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a + b;

	if (!FREEZE_MODE)
	{
		SET_ALU_V_ADD(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);
		SET_ALU_C_ADD(r, a);
	}

	am29000->r[RC] = r;
}

static void ADDS(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void ADDU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void ADDC(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void ADDCS(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a + b + GET_CARRY;

	if (!FREEZE_MODE)
	{
		UINT32 carry = GET_CARRY;
		UINT32 tmp = a + b;

		SET_ALU_V_ADD(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);

		carry = CALC_C_ADD(tmp, a) || CALC_C_ADD(tmp + carry, carry);
		am29000->alu &= ~ALU_C;
		am29000->alu |= carry << ALU_C_SHIFT;
	}

	// TODO: Trap on signed overflow
	am29000->r[RC] = r;
}

static void ADDCU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUB(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r = a - b;

	if (!FREEZE_MODE)
	{
		SET_ALU_V_SUB(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);
		SET_ALU_C_SUB(a, b);
	}

	// TODO: Trap on unsigned overflow
	am29000->r[RC] = r;
}

static void SUBS(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r = a - b;

	if (!FREEZE_MODE)
	{
		SET_ALU_V_SUB(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);
		SET_ALU_C_SUB(a, b);
	}

	if ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0)
		SIGNAL_EXCEPTION(EXCEPTION_OUT_OF_RANGE);

	am29000->r[RC] = r;
}

static void SUBU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBC(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBCS(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBCU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBR(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r = b - a;

	if (!FREEZE_MODE)
	{
		SET_ALU_V_SUB(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);
		SET_ALU_C_SUB(a, b);
	}

	am29000->r[RC] = r;
}

static void SUBRS(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBRU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBRC(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r = b - a - 1 + GET_CARRY;

	if (!FREEZE_MODE)
	{
		SET_ALU_V_SUB(r, a, b);
		SET_ALU_Z(r);
		SET_ALU_N(r);
		SET_ALU_C_SUB(a, b);
	}

	am29000->r[RC] = r;
}

static void SUBRCS(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SUBRCU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void MULTIPLU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void MULTIPLY(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void MUL(am29000_state *am29000)
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 sign;

	if (am29000->q & 1)
	{
		r = a + b;
		sign = (r >> 31) ^ (((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0));
	}
	else
	{
		r = b;
		sign = b >> 31;
	}

	v = ((((UINT64)r << 32) | am29000->q) >> 1) | ((UINT64)sign << 63);
	am29000->q = v & 0xffffffff;

	am29000->r[RC] = v >> 32;
}

static void MULL(am29000_state *am29000)
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 sign;

	if (am29000->q & 1)
	{
		r = b - a;
		sign = (r >> 31) ^ ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0);
	}
	else
	{
		r = b;
		sign = b >> 31;
	}

	v = ((((UINT64)r << 32) | am29000->q) >> 1) | ((UINT64)sign << 63);
	am29000->q = v & 0xffffffff;

	am29000->r[RC] = v >> 32;
}

static void MULU(am29000_state *am29000)
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 c;

	if (am29000->q & 1)
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}
	else
	{
		r = b;
		c = 0;
	}

	v = ((((UINT64)r << 32) | am29000->q) >> 1) | ((UINT64)c << 63);
	am29000->q = v & 0xffffffff;

	am29000->r[RC] = v >> 32;
}

static void DIVIDE(am29000_state *am29000)
{
	am29000->ipa = RA << IPX_SHIFT;
	am29000->ipb = RB << IPX_SHIFT;
	am29000->ipc = RC << IPX_SHIFT;

	SIGNAL_EXCEPTION(EXCEPTION_DIVIDE);
}

static void DIVIDU(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void DIV0(am29000_state *am29000)
{
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT64 v;

	if (!FREEZE_MODE)
	{
		am29000->alu |= ALU_DF;
		SET_ALU_N(b);
	}

	v = (((UINT64)b << 32) | am29000->q) << 1;

	am29000->q = v & 0xffffffff;

	am29000->r[RC] = v >> 32;
}

static void DIV(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 c;
	UINT32 r;
	UINT64 r64;
	UINT32 df;

	if (am29000->alu & ALU_DF)
	{
		r = a - b;
		c = !((UINT32)(a) < (UINT32)(b));
	}
	else
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}


	df = (~(c ^ (am29000->alu >> ALU_DF_SHIFT) ^ (am29000->alu >> ALU_N_SHIFT)) & 1);

	if (!FREEZE_MODE)
	{
		am29000->alu &= ~ALU_DF;
		am29000->alu |= df << ALU_DF_SHIFT;
		SET_ALU_N(r);
	}

	r64 = ((((UINT64)r << 32) | am29000->q) << 1) | df;
	am29000->q = r64 & 0xffffffff;

	am29000->r[RC] = r64 >> 32;
}

static void DIVL(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 c;
	UINT32 r;
	UINT32 df;

	if (am29000->alu & ALU_DF)
	{
		r = a - b;
		c = !((UINT32)(a) < (UINT32)(b));
	}
	else
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}

	df = (~(c ^ (am29000->alu >> ALU_DF_SHIFT) ^ (am29000->alu >> ALU_N_SHIFT)) & 1);

	if (!FREEZE_MODE)
	{
		am29000->alu &= ~ALU_DF;
		am29000->alu |= df << ALU_DF_SHIFT;
		SET_ALU_N(r);
	}

	am29000->q = (am29000->q << 1) | df;
	am29000->r[RC] = r;
}

static void DIVREM(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;

	if (am29000->alu & ALU_DF)
		am29000->r[RC] = a;
	else
		am29000->r[RC] = a + b;
}


/***************************************************************************
    COMPARE
***************************************************************************/

static void CPEQ(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a == b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPNEQ(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a != b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPLT(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a < (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPLTU(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a < (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPLE(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a <= (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPLEU(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a <= (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPGT(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a > (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPGTU(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a > (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPGE(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a >= (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPGEU(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a >= (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void CPBYTE(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r =
			((a & 0xff000000) == (b & 0xff000000)) ||
			((a & 0x00ff0000) == (b & 0x00ff0000)) ||
			((a & 0x0000ff00) == (b & 0x0000ff00)) ||
			((a & 0x000000ff) == (b & 0x000000ff))
			? BOOLEAN_TRUE : BOOLEAN_FALSE;

	am29000->r[RC] = r;
}

static void ASEQ(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!(GET_RA_VAL == GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASNEQ(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!(GET_RA_VAL != GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASLT(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL < (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASLTU(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL < (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASLE(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL <= (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASLEU(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL <= (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASGT(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL > (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASGTU(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL > (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASGE(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL >= (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

static void ASGEU(am29000_state *am29000)
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL >= (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}


/***************************************************************************
    LOGICAL
***************************************************************************/

static void AND(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a & b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void ANDN(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a & ~b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void NAND(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a & b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void OR(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a | b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void NOR(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a | b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void XOR(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a ^ b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}

static void XNOR(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a ^ b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	am29000->r[RC] = r;
}


/***************************************************************************
    SHIFT
***************************************************************************/

static void SLL(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a << b;

	am29000->r[RC] = r;
}

static void SRL(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a >> b;

	am29000->r[RC] = r;
}

static void SRA(am29000_state *am29000)
{
	INT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a >> b;

	am29000->r[RC] = r;
}

static void EXTRACT(am29000_state *am29000)
{
	INT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL);
	UINT64 r;

	r = (((UINT64)a << 32) | b) << GET_ALU_FC;

	am29000->r[RC] = r >> 32;
}


/***************************************************************************
    DATA MOVEMENT
***************************************************************************/

static void LOAD(am29000_state *am29000)
{
	UINT32 addr = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r;

	if (INST_UA_BIT)
		fatalerror("Am29000: UA bit set on LOAD\n");

	if (INST_CE_BIT)
	{
		logerror("Am29000: Attempting a co-processor LOAD!\n");
		r = 0;
	}
	else
	{
		if (!INST_PA_BIT && !(am29000->cps & CPS_PD))
		{
			fatalerror("Am29000: Address translation on LOAD\n");
		}
		else
		{
			if (USER_MODE)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}

			r = am29000->data->read_dword(addr);
		}
	}

//  if (opt & 2)
//      logerror("Am29000: Half word LOAD\n");

	if (!FREEZE_MODE)
	{
		am29000->chc = ((am29000->exec_ir << 8) & 0xff) |
						CHC_LS |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		am29000->cha = addr;
		am29000->chd = r;

		if (!(am29000->cfg & CFG_DW) && (am29000->exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	am29000->r[RA] = r;

	if (am29000->cfg & CFG_DW)
		logerror("DW ON A STORE");
}

static void LOADL(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void LOADSET(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void LOADM(am29000_state *am29000)
{
	UINT32 addr = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r;

	if (INST_UA_BIT)
		fatalerror("Am29000: UA bit set on LOAD\n");

	if (INST_CE_BIT)
	{
		logerror("Am29000: Attempting a co-processor LOAD!\n");
		r = 0;
	}
	else
	{
		if (!INST_PA_BIT && !(am29000->cps & CPS_PD))
		{
			fatalerror("Am29000: Address translation on LOAD\n");
		}
		else
		{
			if (USER_MODE)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}

			r = am29000->data->read_dword(addr);
		}
	}

	if (!FREEZE_MODE)
	{
		// TODO
		am29000->chc &= (CHC_CR_MASK << CHC_CR_SHIFT);
		am29000->chc |= ((am29000->exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		am29000->cha = addr;
		am29000->chd = r; // ?????

		if (!(am29000->cfg & CFG_DW) && (am29000->exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	r = RA;

	{
		int cnt;
		for (cnt = 0; cnt <= GET_CHC_CR; ++cnt)
		{
			am29000->r[r] = am29000->data->read_dword(addr);

//          SET_CHC_CR(cnt - 1);
			addr += 4;

			if (++r == 256)
				r = 128;
		}
	}
}

static void STORE(am29000_state *am29000)
{
	UINT32 addr = INST_M_BIT ? I8: GET_RB_VAL;
//  UINT32 r;

	if (INST_UA_BIT)
		fatalerror("Am29000: UA bit set on LOAD\n");

	if (INST_CE_BIT)
	{
		logerror("Am29000: Attempting a co-processor LOAD!\n");
//      r = 0;
	}
	else
	{
		if (!INST_PA_BIT && !(am29000->cps & CPS_PD))
		{
			fatalerror("Am29000: Address translation on LOAD\n");
		}
		else
		{
			if (USER_MODE)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}

		}
	}

	am29000->data->write_dword(addr, am29000->r[RA]);

	if (!FREEZE_MODE)
	{
		am29000->chc = ((am29000->exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		am29000->cha = addr;

		if (!(am29000->cfg & CFG_DW) && (am29000->exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	if (am29000->cfg & CFG_DW)
		logerror("DW ON A STORE");
}

static void STOREL(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void STOREM(am29000_state *am29000)
{
	UINT32 addr = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r;

	if (INST_UA_BIT)
		fatalerror("Am29000: UA bit set on LOAD\n");

	if (INST_CE_BIT)
	{
		logerror("Am29000: Attempting a co-processor LOAD!\n");
		r = 0;
	}
	else
	{
		if (!INST_PA_BIT && !(am29000->cps & CPS_PD))
		{
			fatalerror("Am29000: Address translation on LOAD\n");
		}
		else
		{
			if (USER_MODE)
			{
				SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
				return;
			}

		}
	}

	if (!FREEZE_MODE)
	{
		// TODO
		am29000->chc &= (CHC_CR_MASK << CHC_CR_SHIFT);
		am29000->chc |= ((am29000->exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		am29000->cha = addr;

		if (!(am29000->cfg & CFG_DW) && (am29000->exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	r = RA;

	{
		int cnt;
		for (cnt = 0; cnt <= GET_CHC_CR; ++cnt)
		{
			am29000->data->write_dword(addr, am29000->r[r]);

//          SET_CHC_CR(cnt - 1);
			addr += 4;

			if (++r == 256)
				r = 128;
		}
	}
}

static void EXBYTE(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 bp = GET_ALU_BP;
	UINT8 srcbyte;
	UINT32 r;

	if (am29000->cfg & CFG_BO)
		srcbyte = a >> 8 * bp;
	else
		srcbyte = a >> (8 * (3 - bp));

	r = (b & 0xffffff00) | srcbyte;

	am29000->r[RC] = r;
}

static void EXHW(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 wp = ((am29000->alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT16 srcword;
	UINT32 r;

	if (am29000->cfg & CFG_BO)
		srcword = a >> 16 * wp;
	else
		srcword = a >> (16 * (1 - wp));

	r = (b & 0xffff0000) | srcword;

	am29000->r[RC] = r;
}

static void EXHWS(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 wp = ((am29000->alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT16 srcword;
	UINT32 r;

	if (am29000->cfg & CFG_BO)
		srcword = a >> 16 * wp;
	else
		srcword = a >> (16 * (1 - wp));

	r = (INT32)(INT16)srcword;

	am29000->r[RC] = r;
}

static void INBYTE(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 bp = GET_ALU_BP;
	UINT8 shift = (am29000->cfg & CFG_BO) ? 8 * bp : (8 * (3 - bp));
	UINT32 r;

	r = (a & ~(0xff << shift)) | ((b & 0xff) << shift);

	am29000->r[RC] = r;
}

static void INHW(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 wp = ((am29000->alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT32 shift = (am29000->cfg & CFG_BO) ? 16 * wp : (16 * (1 - wp));
	UINT32 r;

	r = (a & ~(0xffff << shift)) | ((b & 0xffff) << shift);

	am29000->r[RC] = r;
}

static void MFSR(am29000_state *am29000)
{
	am29000->r[RC] = read_spr(am29000, INST_SA);
}

static void MFTLB(am29000_state *am29000)
{
	am29000->r[RC] = am29000->tlb[GET_RA_VAL & 0x7f];
}

static void MTSR(am29000_state *am29000)
{
	write_spr(am29000, INST_SA, GET_RB_VAL);
}

static void MTSRIM(am29000_state *am29000)
{
	write_spr(am29000, INST_SA, I16_ZEX);
}

static void MTTLB(am29000_state *am29000)
{
	am29000->tlb[GET_RA_VAL & 0x7f] = GET_RB_VAL;
}


/***************************************************************************
    CONSTANT
***************************************************************************/

static void CONST(am29000_state *am29000)
{
	am29000->r[RA] = I16_ZEX;
}

static void CONSTH(am29000_state *am29000)
{
	am29000->r[RA] = (I16 << 16) | GET_RA_VAL;
}

static void CONSTN(am29000_state *am29000)
{
	am29000->r[RA] = I16_OEX;
}


/***************************************************************************
    BRANCH INSTRUCTIONS
***************************************************************************/

static void CALL(am29000_state *am29000)
{
	UINT32 ret = am29000->next_pc;

	if (INST_M_BIT)
		am29000->next_pc = JMP_ZEX;
	else
		am29000->next_pc = am29000->exec_pc + JMP_SEX;

	am29000->r[RA] = ret;
am29000->next_pl_flags |= PFLAG_JUMP;
}

static void CALLI(am29000_state *am29000)
{
	UINT32 ret = am29000->next_pc;
	am29000->next_pc = GET_RB_VAL;
	am29000->r[RA] = ret;
	am29000->next_pl_flags |= PFLAG_JUMP;
}

static void JMP(am29000_state *am29000)
{
	if (INST_M_BIT)
		am29000->next_pc = JMP_ZEX;
	else
		am29000->next_pc = am29000->exec_pc + JMP_SEX;

	am29000->next_pl_flags |= PFLAG_JUMP;
}

static void JMPI(am29000_state *am29000)
{
	am29000->next_pc = GET_RB_VAL;

	am29000->next_pl_flags |= PFLAG_JUMP;
}

static void JMPT(am29000_state *am29000)
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_TRUE)
	{
		if (INST_M_BIT)
			am29000->next_pc = JMP_ZEX;
		else
			am29000->next_pc = am29000->exec_pc + JMP_SEX;

		am29000->next_pl_flags |= PFLAG_JUMP;
	}
}

static void JMPTI(am29000_state *am29000)
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_TRUE)
	{
		am29000->next_pc = GET_RB_VAL;
		am29000->next_pl_flags |= PFLAG_JUMP;
	}
}

static void JMPF(am29000_state *am29000)
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		if (INST_M_BIT)
			am29000->next_pc = JMP_ZEX;
		else
			am29000->next_pc = am29000->exec_pc + JMP_SEX;

		am29000->next_pl_flags |= PFLAG_JUMP;
	}
}

static void JMPFI(am29000_state *am29000)
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		am29000->next_pc = GET_RB_VAL;
		am29000->next_pl_flags |= PFLAG_JUMP;
	}
}

static void JMPFDEC(am29000_state *am29000)
{
	UINT32 a = GET_RA_VAL;

	if ((a & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		if (INST_M_BIT)
			am29000->next_pc = JMP_ZEX;
		else
			am29000->next_pc = am29000->exec_pc + JMP_SEX;

		am29000->next_pl_flags |= PFLAG_JUMP;
	}

	am29000->r[RA] = a - 1;
}


/***************************************************************************
    MISCELLANEOUS INSTRUCTIONS
***************************************************************************/

static void CLZ(am29000_state *am29000)
{
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;

	am29000->r[RC] = count_leading_zeros(b);
}

static void SETIP(am29000_state *am29000)
{
	am29000->ipa = RA << IPX_SHIFT;
	am29000->ipb = RB << IPX_SHIFT;
	am29000->ipc = RC << IPX_SHIFT;
}

static void EMULATE(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void INV(am29000_state *am29000)
{
	/* Nothing to do yet */
}

static void IRET(am29000_state *am29000)
{
	am29000->iret_pc = am29000->pc0;
	am29000->next_pc = am29000->pc1;
	am29000->cps = am29000->ops;
	am29000->next_pl_flags = PFLAG_IRET;
}

static void IRETINV(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void HALT(am29000_state *am29000)
{
	UNHANDLED_OP;
}


static void ILLEGAL(am29000_state *am29000)
{
	fatalerror("Am29000: Executed illegal instruction - this should never happen! %x (%x)\n", am29000->pc2, am29000->exec_pc);
}



/***************************************************************************
    UNHANDLED
***************************************************************************/

static void CONVERT(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void SQRT(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void CLASS(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void MULTM(am29000_state *am29000)
{
	UNHANDLED_OP;
}

static void MULTMU(am29000_state *am29000)
{
	UNHANDLED_OP;
}


const op_info op_table[256] =
{
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ CONSTN,   IFLAG_RA_PRESENT                                                   },
	{ CONSTH,   IFLAG_RA_PRESENT                                                   },
	{ CONST,    IFLAG_RA_PRESENT                                                   },
	{ MTSRIM,   0                                                                  },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ LOADL,    0                                                                  },
	{ LOADL,    0                                                                  },
	{ CLZ,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT                                },
	{ CLZ,      IFLAG_RC_PRESENT                                                   },
	{ EXBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ EXBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ INBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ INBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ STOREL,   0                                                                  },
	{ STOREL,   0                                                                  },
	{ ADDS,     0                                                                  },
	{ ADDS,     0                                                                  },
	{ ADDU,     0                                                                  },
	{ ADDU,     0                                                                  },
	{ ADD,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ ADD,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ LOAD,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ LOAD,     IFLAG_RA_PRESENT                                                   },
	{ ADDCS,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ ADDCS,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ADDCU,    0                                                                  },
	{ ADDCU,    0                                                                  },
	{ ADDC,     0                                                                  },
	{ ADDC,     0                                                                  },
	{ STORE,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ STORE,    IFLAG_RA_PRESENT                                                   },
	{ SUBS,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SUBS,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ SUBU,     0                                                                  },
	{ SUBU,     0                                                                  },
	{ SUB,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SUB,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ LOADSET,  0                                                                  },
	{ LOADSET,  0                                                                  },
	{ SUBCS,    0                                                                  },
	{ SUBCS,    0                                                                  },
	{ SUBCU,    0                                                                  },
	{ SUBCU,    0                                                                  },
	{ SUBC,     0                                                                  },
	{ SUBC,     0                                                                  },
	{ CPBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ SUBRS,    0                                                                  },
	{ SUBRS,    0                                                                  },
	{ SUBRU,    0                                                                  },
	{ SUBRU,    0                                                                  },
	{ SUBR,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SUBR,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ LOADM,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ LOADM,    IFLAG_RA_PRESENT                                                   },
	{ SUBRCS,   0                                                                  },
	{ SUBRCS,   0                                                                  },
	{ SUBRCU,   0                                                                  },
	{ SUBRCU,   0                                                                  },
	{ SUBRC,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SUBRC,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ STOREM,   IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ STOREM,   IFLAG_RA_PRESENT                                                   },
	{ CPLT,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPLT,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPLTU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPLTU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPLE,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPLE,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPLEU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPLEU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPGT,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPGT,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPGTU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPGTU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPGE,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPGE,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPGEU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPGEU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ASLT,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASLT,     IFLAG_RA_PRESENT                                                   },
	{ ASLTU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASLTU,    IFLAG_RA_PRESENT                                                   },
	{ ASLE,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASLE,     IFLAG_RA_PRESENT                                                   },
	{ ASLEU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASLEU,    IFLAG_RA_PRESENT                                                   },
	{ ASGT,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASGT,     IFLAG_RA_PRESENT                                                   },
	{ ASGTU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASGTU,    IFLAG_RA_PRESENT                                                   },
	{ ASGE,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASGE,     IFLAG_RA_PRESENT                                                   },
	{ ASGEU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASGEU,    IFLAG_RA_PRESENT                                                   },
	{ CPEQ,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPEQ,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ CPNEQ,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ CPNEQ,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ MUL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ MUL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ MULL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ MULL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ DIV0,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT                                },
	{ DIV0,     IFLAG_RC_PRESENT                                                   },
	{ DIV,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ DIV,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ DIVL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ DIVL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ DIVREM,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ DIVREM,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ASEQ,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASEQ,     IFLAG_RA_PRESENT                                                   },
	{ ASNEQ,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ASNEQ,    IFLAG_RA_PRESENT                                                   },
	{ MULU,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ MULU,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ INHW,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ INHW,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ EXTRACT,  IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ EXTRACT,  IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ EXHW,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ EXHW,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ EXHWS,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ SLL,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SLL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ SRL,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SRL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ SRA,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ SRA,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ IRET,     IFLAG_SUPERVISOR_ONLY                                              },
	{ HALT,     IFLAG_SUPERVISOR_ONLY                                              },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ IRETINV,  IFLAG_SUPERVISOR_ONLY                                              },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ AND,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ AND,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ OR,       IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ OR,       IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ XOR,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ XOR,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ XNOR,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ XNOR,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ NOR,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ NOR,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ NAND,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ NAND,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ ANDN,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ ANDN,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ SETIP,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT | IFLAG_RC_PRESENT             },
	{ INV,      IFLAG_SUPERVISOR_ONLY                                              },
	{ JMP,      0                                                                  },
	{ JMP,      0                                                                  },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPF,     IFLAG_RA_PRESENT                                                   },
	{ JMPF,     IFLAG_RA_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ CALL,     IFLAG_RA_PRESENT                                                   },
	{ CALL,     IFLAG_RA_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPT,     IFLAG_RA_PRESENT                                                   },
	{ JMPT,     IFLAG_RA_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPFDEC,  IFLAG_RA_PRESENT                                                   },
	{ JMPFDEC,  IFLAG_RA_PRESENT                                                   },
	{ MFTLB,    IFLAG_SUPERVISOR_ONLY | IFLAG_RC_PRESENT | IFLAG_RA_PRESENT        },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ MTTLB,    IFLAG_SUPERVISOR_ONLY | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT        },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPI,     IFLAG_RB_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPFI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ MFSR,     IFLAG_RC_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ CALLI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ JMPTI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ MTSR,     IFLAG_RB_PRESENT                                                   },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ EMULATE,  0                                                                  },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ MULTM,    0                                                                  },
	{ MULTMU,   0                                                                  },
	{ MULTIPLY, 0                                                                  },
	{ DIVIDE,   0                                                                  },
	{ MULTIPLU, 0                                                                  },
	{ DIVIDU,   0                                                                  },
	{ CONVERT,  0                                                                  },
	{ SQRT,     0                                                                  },
	{ CLASS,    0                                                                  },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ ILLEGAL,  IFLAG_ILLEGAL                                                      },
	// FEQ
	// DEQ
	// TODO! etc
};
