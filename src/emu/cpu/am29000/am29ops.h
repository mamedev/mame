// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    am29ops.h
    Am29000 instructions

***************************************************************************/

/***************************************************************************
    DEFINES
***************************************************************************/

#define IFLAG_ILLEGAL               (1 << 0)
#define IFLAG_SUPERVISOR_ONLY       (1 << 1)
#define IFLAG_RA_PRESENT            (1 << 2)
#define IFLAG_RB_PRESENT            (1 << 3)
#define IFLAG_RC_PRESENT            (1 << 4)

#define IFLAG_SPR_ACCESS            (1 << 6)
#define IFLAG_MEMORY_ACCESS         (1 << 8)
#define IFLAG_CONTROL               (1 << 9)


#define GET_RA_VAL                  (m_r[RA])
#define GET_RB_VAL                  (m_r[RB])

#define RA                          (get_abs_reg(m_exec_ir >>  8, m_ipa))
#define RB                          (get_abs_reg(m_exec_ir >>  0, m_ipb))
#define RC                          (get_abs_reg(m_exec_ir >> 16, m_ipc))

#define INST_SA                     ((m_exec_ir >> 8) & 0xff)
#define INST_VN                     ((m_exec_ir >> 16) & 0xff)
#define INST_M_BIT                  (m_exec_ir & (1 << 24))
#define INST_CE_BIT                 (m_exec_ir & (1 << 23))
#define INST_AS_BIT                 (m_exec_ir & (1 << 22))
#define INST_PA_BIT                 (m_exec_ir & (1 << 21))
#define INST_SB_BIT                 (m_exec_ir & (1 << 20))
#define INST_UA_BIT                 (m_exec_ir & (1 << 19))
#define INST_OPT_MASK               (7)
#define INST_OPT_SHIFT              (16)
#define INST_OPT_FIELD              (((m_exec_ir) >> INST_OPT_SHIFT) & INST_OPT_MASK)
#define INST_CNTL_MASK              (0x7f)
#define INST_CNTL_SHIFT             (16)

#define I8                          (m_exec_ir & 0xff)
#define I16                         (((m_exec_ir >> 8) & 0xff00) | (m_exec_ir & 0xff))
#define I16_ZEX                     ((UINT32)(I16))
#define I16_SEX                     ((INT32)(INT16)I16)
#define I16_OEX                     (0xffff0000 | I16)

#define JMP_ZEX                     (I16 << 2)
#define JMP_SEX                     ((INT32)(INT16)(((m_exec_ir >> 8) & 0xff00) | (m_exec_ir & 0xff)) << 2)

#define BOOLEAN_MASK                (1 << 31)
#define BOOLEAN_TRUE                (1 << 31)
#define BOOLEAN_FALSE               (0)

#define UNHANDLED_OP                fatalerror("Am29000: Unhandled inst %s at %x\n", __FUNCTION__, m_exec_pc);


/***************************************************************************
    ALU FLAG CALCULATION
***************************************************************************/

#define SET_ALU_Z(r)            m_alu &= ~ALU_Z; \
								m_alu |= (r == 0) << ALU_Z_SHIFT;

#define SET_ALU_N(r)            m_alu &= ~ALU_N; \
								m_alu |= ((UINT32)r & 0x80000000) >> (31 - ALU_N_SHIFT);

#define CALC_C_ADD(r, a)        ((UINT32)(r) < (UINT32)(a))

#define SET_ALU_C_ADD(r, a)     m_alu &= ~ALU_C; \
								m_alu |= CALC_C_ADD(r, a) << ALU_C_SHIFT;

#define CALC_C_SUB(a, b)        (!((UINT32)(a) < (UINT32)(b)))

#define SET_ALU_C_SUB(a, b)     m_alu &= ~ALU_C; \
								m_alu |= CALC_C_SUB(a, b) << ALU_C_SHIFT;

#define SET_ALU_V_ADD(r, a, b)  m_alu &= ~ALU_V; \
								m_alu |= (((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0)) << ALU_V_SHIFT;

#define SET_ALU_V_SUB(r, a, b)  m_alu &= ~ALU_V; \
								m_alu |= ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0) << ALU_V_SHIFT;

#define GET_CARRY               ((m_alu >> ALU_C_SHIFT) & 1)



UINT32 am29000_cpu_device::read_spr(UINT32 idx)
{
	UINT32 val = 0;

	switch (idx)
	{
		case SPR_VAB:   val = m_vab;     break;
		case SPR_OPS:   val = m_ops;     break;
		case SPR_CPS:   val = m_cps;     break;
		case SPR_CFG:   val = m_cfg;     break;
		case SPR_CHA:   val = m_cha;     break;
		case SPR_CHD:   val = m_chd;     break;
		case SPR_CHC:   val = m_chc;     break;
		case SPR_RBP:   val = m_rbp;     break;
		case SPR_TMC:   val = m_tmc;     break;
		case SPR_TMR:   val = m_tmr;     break;
		case SPR_PC0:   val = m_pc0;     break;
		case SPR_PC1:   val = m_pc1;     break;
		case SPR_PC2:   val = m_pc2;     break;
		case SPR_MMU:   val = m_mmu;     break;
		case SPR_LRU:   val = m_lru;     break;
		case SPR_IPC:   val = m_ipc;     break;
		case SPR_IPA:   val = m_ipa;     break;
		case SPR_IPB:   val = m_ipb;     break;
		case SPR_Q:     val = m_q;       break;
		case SPR_ALU:   val = m_alu;     break;
		case SPR_BP:    val = GET_ALU_BP;       break;
		case SPR_FC:    val = GET_ALU_FC;       break;
		case SPR_CR:    val = GET_CHC_CR;       break;
		case SPR_FPE:   val = m_fpe;     break;
		case SPR_INTE:  val = m_inte;    break;
		case SPR_FPS:   val = m_fps;     break;
		default:
			logerror("Unknown SPR read (%d)\n", idx);
	}

	return val;
}


void am29000_cpu_device::write_spr(UINT32 idx, UINT32 val)
{
	switch (idx)
	{
		case SPR_VAB:   m_vab = val & (VAB_MASK << VAB_SHIFT);
						break;
		case SPR_OPS:   m_ops = val & (CPS_CA | CPS_IP | CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_RE |
						CPS_WM | CPS_PD | CPS_PI | CPS_SM | (CPS_IM_MASK << CPS_IM_SHIFT) | CPS_DI | CPS_DA);
						break;
		case SPR_CPS:   m_cps = val & (CPS_CA | CPS_IP | CPS_TE | CPS_TP | CPS_TU | CPS_FZ | CPS_LK | CPS_RE |
						CPS_WM | CPS_PD | CPS_PI | CPS_SM | (CPS_IM_MASK << CPS_IM_SHIFT) | CPS_DI | CPS_DA);
						break;
		case SPR_CFG:   m_cfg = val & (CFG_DW | CFG_VF | CFG_RV | CFG_BO | CFG_CP | CFG_CD);
						m_cfg |= PROCESSOR_REL_FIELD << CFG_PRL_SHIFT;
						break;
		case SPR_CHA:   m_cha = val;
						break;
		case SPR_CHD:   m_chd = val;
						break;
		case SPR_CHC:   m_chc = val;
						break;
		case SPR_RBP:   m_rbp = val & RBP_MASK;
						break;
		case SPR_TMC:   m_tmc = val & TCV_MASK;
						break;
		case SPR_TMR:   m_tmr = val & (TMR_OV | TMR_IN | TMR_IE | TMR_TRV_MASK);
						break;
		case SPR_PC0:   m_pc0 = val & PC_MASK;
						break;
		case SPR_PC1:   m_pc1 = val & PC_MASK;
						break;
		case SPR_PC2:   m_pc2 = val & PC_MASK;
						break;
		case SPR_MMU:   m_mmu = val & ((MMU_PS_MASK << MMU_PS_SHIFT) | MMU_PID_MASK);
						break;
		case SPR_LRU:   m_lru = val & (LRU_MASK << LRU_SHIFT);
						break;
		case SPR_IPC:   m_ipc = val;// & IPX_MASK;
						break;
		case SPR_IPA:   m_ipa = val;// & IPX_MASK;
						break;
		case SPR_IPB:   m_ipb = val;// & IPX_MASK;
						break;
		case SPR_Q:     m_q = val;
						break;
		case SPR_ALU:   m_alu = val & (ALU_DF | ALU_V | ALU_N | ALU_Z | ALU_C | (ALU_BP_MASK << ALU_BP_SHIFT) | (ALU_FC_MASK << ALU_FC_SHIFT));
						break;
		case SPR_BP:    m_alu &= ~(ALU_BP_MASK << ALU_BP_SHIFT);
						m_alu |= (val & ALU_BP_MASK) << ALU_BP_SHIFT;
						break;
		case SPR_FC:    m_alu &= ~(ALU_FC_MASK << ALU_FC_SHIFT);
						m_alu |= (val & ALU_FC_MASK) << ALU_FC_SHIFT;
						break;
		case SPR_CR:    m_chc &= ~(CHC_CR_MASK << CHC_CR_SHIFT);
						m_chc |= (val & CHC_CR_MASK) << CHC_CR_SHIFT;
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

void am29000_cpu_device::ADD()
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

	m_r[RC] = r;
}

void am29000_cpu_device::ADDS()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::ADDU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::ADDC()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::ADDCS()
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
		m_alu &= ~ALU_C;
		m_alu |= carry << ALU_C_SHIFT;
	}

	// TODO: Trap on signed overflow
	m_r[RC] = r;
}

void am29000_cpu_device::ADDCU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUB()
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
	m_r[RC] = r;
}

void am29000_cpu_device::SUBS()
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

	m_r[RC] = r;
}

void am29000_cpu_device::SUBU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBC()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBCS()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBCU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBR()
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

	m_r[RC] = r;
}

void am29000_cpu_device::SUBRS()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBRU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBRC()
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

	m_r[RC] = r;
}

void am29000_cpu_device::SUBRCS()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SUBRCU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::MULTIPLU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::MULTIPLY()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::MUL()
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 sign;

	if (m_q & 1)
	{
		r = a + b;
		sign = (r >> 31) ^ (((INT32)(~((a) ^ (b)) & ((a) ^ (r))) < 0));
	}
	else
	{
		r = b;
		sign = b >> 31;
	}

	v = ((((UINT64)r << 32) | m_q) >> 1) | ((UINT64)sign << 63);
	m_q = v & 0xffffffff;

	m_r[RC] = v >> 32;
}

void am29000_cpu_device::MULL()
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 sign;

	if (m_q & 1)
	{
		r = b - a;
		sign = (r >> 31) ^ ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0);
	}
	else
	{
		r = b;
		sign = b >> 31;
	}

	v = ((((UINT64)r << 32) | m_q) >> 1) | ((UINT64)sign << 63);
	m_q = v & 0xffffffff;

	m_r[RC] = v >> 32;
}

void am29000_cpu_device::MULU()
{
	/* TODO: Zero/Neg flags ? */
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r;
	UINT64 v;
	UINT32 c;

	if (m_q & 1)
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}
	else
	{
		r = b;
		c = 0;
	}

	v = ((((UINT64)r << 32) | m_q) >> 1) | ((UINT64)c << 63);
	m_q = v & 0xffffffff;

	m_r[RC] = v >> 32;
}

void am29000_cpu_device::DIVIDE()
{
	m_ipa = RA << IPX_SHIFT;
	m_ipb = RB << IPX_SHIFT;
	m_ipc = RC << IPX_SHIFT;

	SIGNAL_EXCEPTION(EXCEPTION_DIVIDE);
}

void am29000_cpu_device::DIVIDU()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::DIV0()
{
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT64 v;

	if (!FREEZE_MODE)
	{
		m_alu |= ALU_DF;
		SET_ALU_N(b);
	}

	v = (((UINT64)b << 32) | m_q) << 1;

	m_q = v & 0xffffffff;

	m_r[RC] = v >> 32;
}

void am29000_cpu_device::DIV()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 c;
	UINT32 r;
	UINT64 r64;
	UINT32 df;

	if (m_alu & ALU_DF)
	{
		r = a - b;
		c = !((UINT32)(a) < (UINT32)(b));
	}
	else
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}


	df = (~(c ^ (m_alu >> ALU_DF_SHIFT) ^ (m_alu >> ALU_N_SHIFT)) & 1);

	if (!FREEZE_MODE)
	{
		m_alu &= ~ALU_DF;
		m_alu |= df << ALU_DF_SHIFT;
		SET_ALU_N(r);
	}

	r64 = ((((UINT64)r << 32) | m_q) << 1) | df;
	m_q = r64 & 0xffffffff;

	m_r[RC] = r64 >> 32;
}

void am29000_cpu_device::DIVL()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 c;
	UINT32 r;
	UINT32 df;

	if (m_alu & ALU_DF)
	{
		r = a - b;
		c = !((UINT32)(a) < (UINT32)(b));
	}
	else
	{
		r = a + b;
		c = (UINT32)(r) < (UINT32)(a);
	}

	df = (~(c ^ (m_alu >> ALU_DF_SHIFT) ^ (m_alu >> ALU_N_SHIFT)) & 1);

	if (!FREEZE_MODE)
	{
		m_alu &= ~ALU_DF;
		m_alu |= df << ALU_DF_SHIFT;
		SET_ALU_N(r);
	}

	m_q = (m_q << 1) | df;
	m_r[RC] = r;
}

void am29000_cpu_device::DIVREM()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;

	if (m_alu & ALU_DF)
		m_r[RC] = a;
	else
		m_r[RC] = a + b;
}


/***************************************************************************
    COMPARE
***************************************************************************/

void am29000_cpu_device::CPEQ()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a == b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPNEQ()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a != b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPLT()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a < (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPLTU()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a < (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPLE()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a <= (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPLEU()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a <= (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPGT()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a > (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPGTU()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a > (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPGE()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (INT32)a >= (INT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPGEU()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = (UINT32)a >= (UINT32)b ? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::CPBYTE()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8 : GET_RB_VAL;
	UINT32 r =
			((a & 0xff000000) == (b & 0xff000000)) ||
			((a & 0x00ff0000) == (b & 0x00ff0000)) ||
			((a & 0x0000ff00) == (b & 0x0000ff00)) ||
			((a & 0x000000ff) == (b & 0x000000ff))
			? BOOLEAN_TRUE : BOOLEAN_FALSE;

	m_r[RC] = r;
}

void am29000_cpu_device::ASEQ()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!(GET_RA_VAL == GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASNEQ()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!(GET_RA_VAL != GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASLT()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL < (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASLTU()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL < (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASLE()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL <= (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASLEU()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL <= (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASGT()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL > (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASGTU()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL > (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASGE()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((INT32)GET_RA_VAL >= (INT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}

void am29000_cpu_device::ASGEU()
{
	if (USER_MODE && INST_VN < 64)
		SIGNAL_EXCEPTION(EXCEPTION_PROTECTION_VIOLATION);
	else if (!((UINT32)GET_RA_VAL >= (UINT32)GET_RB_VAL))
		SIGNAL_EXCEPTION(INST_VN);
}


/***************************************************************************
    LOGICAL
***************************************************************************/

void am29000_cpu_device::AND()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a & b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::ANDN()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a & ~b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::NAND()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a & b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::OR()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a | b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::NOR()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a | b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::XOR()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = a ^ b;

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}

void am29000_cpu_device::XNOR()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 r = ~(a ^ b);

	if (!FREEZE_MODE)
	{
		SET_ALU_Z(r);
		SET_ALU_N(r);
	}

	m_r[RC] = r;
}


/***************************************************************************
    SHIFT
***************************************************************************/

void am29000_cpu_device::SLL()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a << b;

	m_r[RC] = r;
}

void am29000_cpu_device::SRL()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a >> b;

	m_r[RC] = r;
}

void am29000_cpu_device::SRA()
{
	INT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL) & 0x1f;
	UINT32 r = a >> b;

	m_r[RC] = r;
}

void am29000_cpu_device::EXTRACT()
{
	INT32 a = GET_RA_VAL;
	UINT32 b = (INST_M_BIT ? I8: GET_RB_VAL);
	UINT64 r;

	r = (((UINT64)a << 32) | b) << GET_ALU_FC;

	m_r[RC] = r >> 32;
}


/***************************************************************************
    DATA MOVEMENT
***************************************************************************/

void am29000_cpu_device::LOAD()
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
		if (!INST_PA_BIT && !(m_cps & CPS_PD))
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

			r = m_data->read_dword(addr);
		}
	}

//  if (opt & 2)
//      logerror("Am29000: Half word LOAD\n");

	if (!FREEZE_MODE)
	{
		m_chc = ((m_exec_ir << 8) & 0xff) |
						CHC_LS |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		m_cha = addr;
		m_chd = r;

		if (!(m_cfg & CFG_DW) && (m_exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	m_r[RA] = r;

	if (m_cfg & CFG_DW)
		logerror("DW ON A STORE");
}

void am29000_cpu_device::LOADL()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::LOADSET()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::LOADM()
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
		if (!INST_PA_BIT && !(m_cps & CPS_PD))
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

			r = m_data->read_dword(addr);
		}
	}

	if (!FREEZE_MODE)
	{
		// TODO
		m_chc &= (CHC_CR_MASK << CHC_CR_SHIFT);
		m_chc |= ((m_exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		m_cha = addr;
		m_chd = r; // ?????

		if (!(m_cfg & CFG_DW) && (m_exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	r = RA;

	{
		int cnt;
		for (cnt = 0; cnt <= GET_CHC_CR; ++cnt)
		{
			m_r[r] = m_data->read_dword(addr);

//          SET_CHC_CR(cnt - 1);
			addr += 4;

			if (++r == 256)
				r = 128;
		}
	}
}

void am29000_cpu_device::STORE()
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
		if (!INST_PA_BIT && !(m_cps & CPS_PD))
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

	m_data->write_dword(addr, m_r[RA]);

	if (!FREEZE_MODE)
	{
		m_chc = ((m_exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		m_cha = addr;

		if (!(m_cfg & CFG_DW) && (m_exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	if (m_cfg & CFG_DW)
		logerror("DW ON A STORE");
}

void am29000_cpu_device::STOREL()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::STOREM()
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
		if (!INST_PA_BIT && !(m_cps & CPS_PD))
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
		m_chc &= (CHC_CR_MASK << CHC_CR_SHIFT);
		m_chc |= ((m_exec_ir << 8) & 0xff) |
						RA << CHC_TR_SHIFT |
						CHC_CV;

		m_cha = addr;

		if (!(m_cfg & CFG_DW) && (m_exec_ir & INST_SB_BIT))
			SET_ALU_BP(addr & 3);
	}

	r = RA;

	{
		int cnt;
		for (cnt = 0; cnt <= GET_CHC_CR; ++cnt)
		{
			m_data->write_dword(addr, m_r[r]);

//          SET_CHC_CR(cnt - 1);
			addr += 4;

			if (++r == 256)
				r = 128;
		}
	}
}

void am29000_cpu_device::EXBYTE()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 bp = GET_ALU_BP;
	UINT8 srcbyte;
	UINT32 r;

	if (m_cfg & CFG_BO)
		srcbyte = a >> 8 * bp;
	else
		srcbyte = a >> (8 * (3 - bp));

	r = (b & 0xffffff00) | srcbyte;

	m_r[RC] = r;
}

void am29000_cpu_device::EXHW()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 wp = ((m_alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT16 srcword;
	UINT32 r;

	if (m_cfg & CFG_BO)
		srcword = a >> 16 * wp;
	else
		srcword = a >> (16 * (1 - wp));

	r = (b & 0xffff0000) | srcword;

	m_r[RC] = r;
}

void am29000_cpu_device::EXHWS()
{
	UINT32 a = GET_RA_VAL;
	UINT32 wp = ((m_alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT16 srcword;
	UINT32 r;

	if (m_cfg & CFG_BO)
		srcword = a >> 16 * wp;
	else
		srcword = a >> (16 * (1 - wp));

	r = (INT32)(INT16)srcword;

	m_r[RC] = r;
}

void am29000_cpu_device::INBYTE()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 bp = GET_ALU_BP;
	UINT8 shift = (m_cfg & CFG_BO) ? 8 * bp : (8 * (3 - bp));
	UINT32 r;

	r = (a & ~(0xff << shift)) | ((b & 0xff) << shift);

	m_r[RC] = r;
}

void am29000_cpu_device::INHW()
{
	UINT32 a = GET_RA_VAL;
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;
	UINT32 wp = ((m_alu >> ALU_BP_SHIFT) & ALU_BP_MASK) >> 1;
	UINT32 shift = (m_cfg & CFG_BO) ? 16 * wp : (16 * (1 - wp));
	UINT32 r;

	r = (a & ~(0xffff << shift)) | ((b & 0xffff) << shift);

	m_r[RC] = r;
}

void am29000_cpu_device::MFSR()
{
	m_r[RC] = read_spr(INST_SA);
}

void am29000_cpu_device::MFTLB()
{
	m_r[RC] = m_tlb[GET_RA_VAL & 0x7f];
}

void am29000_cpu_device::MTSR()
{
	write_spr(INST_SA, GET_RB_VAL);
}

void am29000_cpu_device::MTSRIM()
{
	write_spr(INST_SA, I16_ZEX);
}

void am29000_cpu_device::MTTLB()
{
	m_tlb[GET_RA_VAL & 0x7f] = GET_RB_VAL;
}


/***************************************************************************
    CONSTANT
***************************************************************************/

void am29000_cpu_device::CONST()
{
	m_r[RA] = I16_ZEX;
}

void am29000_cpu_device::CONSTH()
{
	m_r[RA] = (I16 << 16) | GET_RA_VAL;
}

void am29000_cpu_device::CONSTN()
{
	m_r[RA] = I16_OEX;
}


/***************************************************************************
    BRANCH INSTRUCTIONS
***************************************************************************/

void am29000_cpu_device::CALL()
{
	UINT32 ret = m_next_pc;

	if (INST_M_BIT)
		m_next_pc = JMP_ZEX;
	else
		m_next_pc = m_exec_pc + JMP_SEX;

	m_r[RA] = ret;
m_next_pl_flags |= PFLAG_JUMP;
}

void am29000_cpu_device::CALLI()
{
	UINT32 ret = m_next_pc;
	m_next_pc = GET_RB_VAL;
	m_r[RA] = ret;
	m_next_pl_flags |= PFLAG_JUMP;
}

void am29000_cpu_device::JMP()
{
	if (INST_M_BIT)
		m_next_pc = JMP_ZEX;
	else
		m_next_pc = m_exec_pc + JMP_SEX;

	m_next_pl_flags |= PFLAG_JUMP;
}

void am29000_cpu_device::JMPI()
{
	m_next_pc = GET_RB_VAL;

	m_next_pl_flags |= PFLAG_JUMP;
}

void am29000_cpu_device::JMPT()
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_TRUE)
	{
		if (INST_M_BIT)
			m_next_pc = JMP_ZEX;
		else
			m_next_pc = m_exec_pc + JMP_SEX;

		m_next_pl_flags |= PFLAG_JUMP;
	}
}

void am29000_cpu_device::JMPTI()
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_TRUE)
	{
		m_next_pc = GET_RB_VAL;
		m_next_pl_flags |= PFLAG_JUMP;
	}
}

void am29000_cpu_device::JMPF()
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		if (INST_M_BIT)
			m_next_pc = JMP_ZEX;
		else
			m_next_pc = m_exec_pc + JMP_SEX;

		m_next_pl_flags |= PFLAG_JUMP;
	}
}

void am29000_cpu_device::JMPFI()
{
	if ((GET_RA_VAL & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		m_next_pc = GET_RB_VAL;
		m_next_pl_flags |= PFLAG_JUMP;
	}
}

void am29000_cpu_device::JMPFDEC()
{
	UINT32 a = GET_RA_VAL;

	if ((a & BOOLEAN_MASK) == BOOLEAN_FALSE)
	{
		if (INST_M_BIT)
			m_next_pc = JMP_ZEX;
		else
			m_next_pc = m_exec_pc + JMP_SEX;

		m_next_pl_flags |= PFLAG_JUMP;
	}

	m_r[RA] = a - 1;
}


/***************************************************************************
    MISCELLANEOUS INSTRUCTIONS
***************************************************************************/

void am29000_cpu_device::CLZ()
{
	UINT32 b = INST_M_BIT ? I8: GET_RB_VAL;

	m_r[RC] = count_leading_zeros(b);
}

void am29000_cpu_device::SETIP()
{
	m_ipa = RA << IPX_SHIFT;
	m_ipb = RB << IPX_SHIFT;
	m_ipc = RC << IPX_SHIFT;
}

void am29000_cpu_device::EMULATE()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::INV()
{
	/* Nothing to do yet */
}

void am29000_cpu_device::IRET()
{
	m_iret_pc = m_pc0;
	m_next_pc = m_pc1;
	m_cps = m_ops;
	m_next_pl_flags = PFLAG_IRET;
}

void am29000_cpu_device::IRETINV()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::HALT()
{
	UNHANDLED_OP;
}


void am29000_cpu_device::ILLEGAL()
{
	fatalerror("Am29000: Executed illegal instruction - this should never happen! %x (%x)\n", m_pc2, m_exec_pc);
}



/***************************************************************************
    UNHANDLED
***************************************************************************/

void am29000_cpu_device::CONVERT()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::SQRT()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::CLASS()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::MULTM()
{
	UNHANDLED_OP;
}

void am29000_cpu_device::MULTMU()
{
	UNHANDLED_OP;
}


const am29000_cpu_device::op_info am29000_cpu_device::op_table[256] =
{
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::CONSTN,   IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::CONSTH,   IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::CONST,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::MTSRIM,   0                                                                  },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::LOADL,    0                                                                  },
	{ &am29000_cpu_device::LOADL,    0                                                                  },
	{ &am29000_cpu_device::CLZ,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::CLZ,      IFLAG_RC_PRESENT                                                   },
	{ &am29000_cpu_device::EXBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::EXBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::INBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::INBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::STOREL,   0                                                                  },
	{ &am29000_cpu_device::STOREL,   0                                                                  },
	{ &am29000_cpu_device::ADDS,     0                                                                  },
	{ &am29000_cpu_device::ADDS,     0                                                                  },
	{ &am29000_cpu_device::ADDU,     0                                                                  },
	{ &am29000_cpu_device::ADDU,     0                                                                  },
	{ &am29000_cpu_device::ADD,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::ADD,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::LOAD,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::LOAD,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ADDCS,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::ADDCS,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ADDCU,    0                                                                  },
	{ &am29000_cpu_device::ADDCU,    0                                                                  },
	{ &am29000_cpu_device::ADDC,     0                                                                  },
	{ &am29000_cpu_device::ADDC,     0                                                                  },
	{ &am29000_cpu_device::STORE,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::STORE,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::SUBS,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SUBS,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::SUBU,     0                                                                  },
	{ &am29000_cpu_device::SUBU,     0                                                                  },
	{ &am29000_cpu_device::SUB,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SUB,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::LOADSET,  0                                                                  },
	{ &am29000_cpu_device::LOADSET,  0                                                                  },
	{ &am29000_cpu_device::SUBCS,    0                                                                  },
	{ &am29000_cpu_device::SUBCS,    0                                                                  },
	{ &am29000_cpu_device::SUBCU,    0                                                                  },
	{ &am29000_cpu_device::SUBCU,    0                                                                  },
	{ &am29000_cpu_device::SUBC,     0                                                                  },
	{ &am29000_cpu_device::SUBC,     0                                                                  },
	{ &am29000_cpu_device::CPBYTE,   IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPBYTE,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::SUBRS,    0                                                                  },
	{ &am29000_cpu_device::SUBRS,    0                                                                  },
	{ &am29000_cpu_device::SUBRU,    0                                                                  },
	{ &am29000_cpu_device::SUBRU,    0                                                                  },
	{ &am29000_cpu_device::SUBR,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SUBR,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::LOADM,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::LOADM,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::SUBRCS,   0                                                                  },
	{ &am29000_cpu_device::SUBRCS,   0                                                                  },
	{ &am29000_cpu_device::SUBRCU,   0                                                                  },
	{ &am29000_cpu_device::SUBRCU,   0                                                                  },
	{ &am29000_cpu_device::SUBRC,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SUBRC,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::STOREM,   IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::STOREM,   IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::CPLT,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPLT,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPLTU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPLTU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPLE,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPLE,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPLEU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPLEU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPGT,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPGT,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPGTU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPGTU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPGE,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPGE,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPGEU,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPGEU,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ASLT,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASLT,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASLTU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASLTU,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASLE,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASLE,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASLEU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASLEU,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASGT,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASGT,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASGTU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASGTU,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASGE,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASGE,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASGEU,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASGEU,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::CPEQ,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPEQ,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::CPNEQ,    IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::CPNEQ,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::MUL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::MUL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::MULL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::MULL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::DIV0,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::DIV0,     IFLAG_RC_PRESENT                                                   },
	{ &am29000_cpu_device::DIV,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::DIV,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::DIVL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::DIVL,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::DIVREM,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::DIVREM,   IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ASEQ,     IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASEQ,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ASNEQ,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ASNEQ,    IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::MULU,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT             },
	{ &am29000_cpu_device::MULU,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::INHW,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::INHW,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::EXTRACT,  IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::EXTRACT,  IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::EXHW,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::EXHW,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::EXHWS,    IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::SLL,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SLL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::SRL,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SRL,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::SRA,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::SRA,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::IRET,     IFLAG_SUPERVISOR_ONLY                                              },
	{ &am29000_cpu_device::HALT,     IFLAG_SUPERVISOR_ONLY                                              },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::IRETINV,  IFLAG_SUPERVISOR_ONLY                                              },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::AND,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::AND,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::OR,       IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::OR,       IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::XOR,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::XOR,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::XNOR,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::XNOR,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::NOR,      IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::NOR,      IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::NAND,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::NAND,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::ANDN,     IFLAG_RC_PRESENT | IFLAG_RB_PRESENT | IFLAG_RA_PRESENT             },
	{ &am29000_cpu_device::ANDN,     IFLAG_RC_PRESENT | IFLAG_RA_PRESENT                                },
	{ &am29000_cpu_device::SETIP,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT | IFLAG_RC_PRESENT             },
	{ &am29000_cpu_device::INV,      IFLAG_SUPERVISOR_ONLY                                              },
	{ &am29000_cpu_device::JMP,      0                                                                  },
	{ &am29000_cpu_device::JMP,      0                                                                  },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPF,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::JMPF,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::CALL,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::CALL,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPT,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::JMPT,     IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPFDEC,  IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::JMPFDEC,  IFLAG_RA_PRESENT                                                   },
	{ &am29000_cpu_device::MFTLB,    IFLAG_SUPERVISOR_ONLY | IFLAG_RC_PRESENT | IFLAG_RA_PRESENT        },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::MTTLB,    IFLAG_SUPERVISOR_ONLY | IFLAG_RA_PRESENT | IFLAG_RB_PRESENT        },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPI,     IFLAG_RB_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPFI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::MFSR,     IFLAG_RC_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::CALLI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::JMPTI,    IFLAG_RA_PRESENT | IFLAG_RB_PRESENT                                },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::MTSR,     IFLAG_RB_PRESENT                                                   },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::EMULATE,  0                                                                  },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::MULTM,    0                                                                  },
	{ &am29000_cpu_device::MULTMU,   0                                                                  },
	{ &am29000_cpu_device::MULTIPLY, 0                                                                  },
	{ &am29000_cpu_device::DIVIDE,   0                                                                  },
	{ &am29000_cpu_device::MULTIPLU, 0                                                                  },
	{ &am29000_cpu_device::DIVIDU,   0                                                                  },
	{ &am29000_cpu_device::CONVERT,  0                                                                  },
	{ &am29000_cpu_device::SQRT,     0                                                                  },
	{ &am29000_cpu_device::CLASS,    0                                                                  },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	{ &am29000_cpu_device::ILLEGAL,  IFLAG_ILLEGAL                                                      },
	// FEQ
	// DEQ
	// TODO! etc
};
