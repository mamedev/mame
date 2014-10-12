// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    32031ops.c

    TMS32031/2 emulator

***************************************************************************/


//**************************************************************************
//  COMPILE-TIME OPTIONS
//**************************************************************************

#define USE_FP              0



//**************************************************************************
//  MACROS
//**************************************************************************

#define IREG(rnum)          (m_r[rnum].i32[0])
#define FREGEXP(rnum)       (m_r[rnum].exponent())
#define FREGMAN(rnum)       (m_r[rnum].mantissa())

#define FP2LONG(rnum)       ((FREGEXP(rnum) << 24) | ((UINT32)FREGMAN(rnum) >> 8))
#define LONG2FP(rnum,v)     do { m_r[rnum].set_mantissa((v) << 8); m_r[rnum].set_exponent((INT32)(v) >> 24); } while (0)
#define SHORT2FP(rnum,v)    do { \
								if ((UINT16)(v) == 0x8000) { m_r[rnum].set_mantissa(0); m_r[rnum].set_exponent(-128); } \
								else { m_r[rnum].set_mantissa((v) << 20); m_r[rnum].set_exponent((INT16)(v) >> 12); } \
							} while (0)

#define DIRECT(op)              (((IREG(TMR_DP) & 0xff) << 16) | ((UINT16)op))
#define INDIRECT_D(op,o)        ((this->*s_indirect_d[((o) >> 3) & 31])(op,o))
#define INDIRECT_1(op,o)        ((this->*s_indirect_1[((o) >> 3) & 31])(op,o))
#define INDIRECT_1_DEF(op,o)    ((this->*s_indirect_1_def[((o) >> 3) & 31])(op,o,defptr))

#define SIGN(val)           ((val) & 0x80000000)

#define OVERFLOW_SUB(a,b,r) ((INT32)(((a) ^ (b)) & ((a) ^ (r))) < 0)
#define OVERFLOW_ADD(a,b,r) ((INT32)(((a) ^ (r)) & ((b) ^ (r))) < 0)

#define CLR_FLAGS(f)        do { IREG(TMR_ST) &= ~(f); } while (0)
#define CLR_NVUF()          CLR_FLAGS(NFLAG | VFLAG | UFFLAG)
#define CLR_NZVUF()         CLR_FLAGS(NFLAG | ZFLAG | VFLAG | UFFLAG)
#define CLR_NZCVUF()        CLR_FLAGS(NFLAG | ZFLAG | VFLAG | CFLAG | UFFLAG)

#define OR_C(flag)          do { IREG(TMR_ST) |= flag & CFLAG; } while (0)
#define OR_NZ(val)          do { IREG(TMR_ST) |= (((val) >> 28) & NFLAG) | (((val) == 0) << 2); } while (0)
#define OR_NZF(reg)         do { IREG(TMR_ST) |= ((reg.mantissa() >> 28) & NFLAG) | ((reg.exponent() == -128) << 2); } while (0)
#define OR_NUF(reg)         do { int temp = (reg.exponent() == -128) << 4; IREG(TMR_ST) |= ((reg.mantissa() >> 28) & NFLAG) | (temp) | (temp << 2); } while (0)
#define OR_V_SUB(a,b,r)     do { UINT32 temp = ((((a) ^ (b)) & ((a) ^ (r))) >> 30) & VFLAG; IREG(TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_V_ADD(a,b,r)     do { UINT32 temp = ((((a) ^ (r)) & ((b) ^ (r))) >> 30) & VFLAG; IREG(TMR_ST) |= temp | (temp << 4); } while (0)
#define OR_C_SUB(a,b,r)     do { IREG(TMR_ST) |= ((UINT32)(b) > (UINT32)(a)); } while (0)
#define OR_C_ADD(a,b,r)     do { IREG(TMR_ST) |= ((UINT32)(a) > (UINT32)(r)); } while (0)
#define OR_C_SBB(a,b,c)     do { INT64 temp = (INT64)(a) - (UINT32)(b) - (UINT32)(c); IREG(TMR_ST) |= (temp < 0); } while (0)
#define OR_C_ADC(a,b,c)     do { UINT64 temp = (UINT64)(a) + (UINT32)(b) + (UINT32)(c); IREG(TMR_ST) |= (temp > 0xffffffff); } while (0)

#define OVM()               (IREG(TMR_ST) & OVMFLAG)

#define DECLARE_DEF         UINT32 defval; UINT32 *defptr = &defval
#define UPDATE_DEF()        *defptr = defval



//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void tms3203x_device::illegal(UINT32 op)
{
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		logerror("Illegal op @ %06X: %08X (tbl=%03X)\n", m_pc - 1, op, op >> 21);
		debugger_break(machine());
	}
}


void tms3203x_device::unimplemented(UINT32 op)
{
	fatalerror("Unimplemented op @ %06X: %08X (tbl=%03X)\n", m_pc - 1, op, op >> 21);
}


inline void tms3203x_device::execute_one()
{
	UINT32 op = ROPCODE(m_pc);
	m_icount -= 2;  // 2 clocks per cycle
	m_pc++;
#if (TMS_3203X_LOG_OPCODE_USAGE)
	m_hits[op >> 21]++;
#endif
	(this->*s_tms32031ops[op >> 21])(op);
}


void tms3203x_device::update_special(int dreg)
{
	if (dreg == TMR_BK)
	{
		UINT32 temp = IREG(TMR_BK);
		m_bkmask = temp;
		while (temp >>= 1)
			m_bkmask |= temp;
	}
	else if (dreg == TMR_IOF)
	{
		if (IREG(TMR_IOF) & 0x002)
			m_xf0_cb((offs_t)0, (IREG(TMR_IOF) >> 2) & 1);
		if (IREG(TMR_IOF) & 0x020)
			m_xf1_cb((offs_t)0, (IREG(TMR_IOF) >> 6) & 1);
	}
	else if (dreg == TMR_ST || dreg == TMR_IF || dreg == TMR_IE)
		check_irqs();
}



//**************************************************************************
//  CONDITION CODES
//**************************************************************************

const UINT32 C_LO = 1 << 1;
const UINT32 C_LS = 1 << 2;
const UINT32 C_HI = 1 << 3;
const UINT32 C_HS = 1 << 4;
const UINT32 C_EQ = 1 << 5;
const UINT32 C_NE = 1 << 6;
const UINT32 C_LT = 1 << 7;
const UINT32 C_LE = 1 << 8;
const UINT32 C_GT = 1 << 9;
const UINT32 C_GE = 1 << 10;
const UINT32 C_NV = 1 << 12;
const UINT32 C_V = 1 << 13;
const UINT32 C_NUF = 1 << 14;
const UINT32 C_UF = 1 << 15;
const UINT32 C_NLV = 1 << 16;
const UINT32 C_LV = 1 << 17;
const UINT32 C_NLUF = 1 << 18;
const UINT32 C_LUF = 1 << 19;
const UINT32 C_ZUF = 1 << 20;

const UINT32 condition_table[0x80] =
{
/* ------- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_NLV | C_NLUF,
/* ------C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_NLV | C_NLUF,
/* -----V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_NLV | C_NLUF,
/* -----VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_NLV | C_NLUF,
/* ----Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ----Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ----ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ----ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ---N--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_NLUF,
/* ---N--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_NLUF,
/* ---N-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_NLUF,
/* ---N-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_NLUF,
/* ---NZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ---NZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ---NZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* ---NZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_NLUF | C_ZUF,
/* --U---- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U---C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U--V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U--VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U-Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U-Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U-ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --U-ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UN--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UN--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UN-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UN-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UNZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UNZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UNZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* --UNZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_NLUF | C_ZUF,
/* -v----- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_LV  | C_NLUF,
/* -v----C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_LV  | C_NLUF,
/* -v---V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_LV  | C_NLUF,
/* -v---VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_LV  | C_NLUF,
/* -v--Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v--Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v--ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v--ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v-N--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_NLUF,
/* -v-N--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_NLUF,
/* -v-N-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_NLUF,
/* -v-N-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_NLUF,
/* -v-NZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v-NZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v-NZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -v-NZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_NLUF | C_ZUF,
/* -vU---- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU---C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU--V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU--VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU-Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU-Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU-ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vU-ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUN--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUN--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUN-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUN-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUNZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUNZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUNZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* -vUNZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_NLUF | C_ZUF,
/* u------ */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_NLV | C_LUF,
/* u-----C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_NLV | C_LUF,
/* u----V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_NLV | C_LUF,
/* u----VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_NLV | C_LUF,
/* u---Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u---Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u---ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u---ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u--N--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_LUF,
/* u--N--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_LUF,
/* u--N-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_LUF,
/* u--N-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_LUF,
/* u--NZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u--NZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u--NZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u--NZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_NLV | C_LUF  | C_ZUF,
/* u-U---- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U---C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U--V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U--VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U-Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U-Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U-ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-U-ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UN--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UN--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UN-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UN-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UNZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UNZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UNZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* u-UNZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_NLV | C_LUF  | C_ZUF,
/* uv----- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_LV  | C_LUF,
/* uv----C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_NUF | C_LV  | C_LUF,
/* uv---V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_LV  | C_LUF,
/* uv---VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_NUF | C_LV  | C_LUF,
/* uv--Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv--Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv--ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv--ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv-N--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_LUF,
/* uv-N--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_LUF,
/* uv-N-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_LUF,
/* uv-N-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_LUF,
/* uv-NZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv-NZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv-NZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uv-NZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_NUF | C_LV  | C_LUF  | C_ZUF,
/* uvU---- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU---C */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU--V- */   1 | C_HI | C_HS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU--VC */   1 | C_LO | C_LS | C_NE | C_GT | C_GE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU-Z-- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU-Z-C */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU-ZV- */   1 | C_LS | C_HS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvU-ZVC */   1 | C_LO | C_LS | C_EQ | C_LE | C_GE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUN--- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUN--C */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUN-V- */   1 | C_HI | C_HS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUN-VC */   1 | C_LO | C_LS | C_NE | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUNZ-- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUNZ-C */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_NV | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUNZV- */   1 | C_LS | C_HS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
/* uvUNZVC */   1 | C_LO | C_LS | C_EQ | C_LT | C_LE | C_V  | C_UF  | C_LV  | C_LUF  | C_ZUF,
};

#define CONDITION_LO()      (IREG(TMR_ST) & CFLAG)
#define CONDITION_LS()      (IREG(TMR_ST) & (CFLAG | ZFLAG))
#define CONDITION_HI()      (!(IREG(TMR_ST) & (CFLAG | ZFLAG)))
#define CONDITION_HS()      (!(IREG(TMR_ST) & CFLAG))
#define CONDITION_EQ()      (IREG(TMR_ST) & ZFLAG)
#define CONDITION_NE()      (!(IREG(TMR_ST) & ZFLAG))
#define CONDITION_LT()      (IREG(TMR_ST) & NFLAG)
#define CONDITION_LE()      (IREG(TMR_ST) & (NFLAG | ZFLAG))
#define CONDITION_GT()      (!(IREG(TMR_ST) & (NFLAG | ZFLAG)))
#define CONDITION_GE()      (!(IREG(TMR_ST) & NFLAG))
#define CONDITION_NV()      (!(IREG(TMR_ST) & VFLAG))
#define CONDITION_V()       (IREG(TMR_ST) & VFLAG)
#define CONDITION_NUF()     (!(IREG(TMR_ST) & UFFLAG))
#define CONDITION_UF()      (IREG(TMR_ST) & UFFLAG)
#define CONDITION_NLV()     (!(IREG(TMR_ST) & LVFLAG))
#define CONDITION_LV()      (IREG(TMR_ST) & LVFLAG)
#define CONDITION_NLUF()    (!(IREG(TMR_ST) & LUFFLAG))
#define CONDITION_LUF()     (IREG(TMR_ST) & LUFFLAG)
#define CONDITION_ZUF()     (IREG(TMR_ST) & (UFFLAG | ZFLAG))

inline bool tms3203x_device::condition(int which)
{
	return (condition_table[IREG(TMR_ST) & (LUFFLAG | LVFLAG | UFFLAG | NFLAG | ZFLAG | VFLAG | CFLAG)] >> (which & 31)) & 1;
}



//**************************************************************************
//  FLOATING POINT HELPERS
//**************************************************************************

#if USE_FP
void tms3203x_device::double_to_dsp_with_flags(double val, tmsreg &result)
{
	int_double id;
	id.d = val;

	CLR_NZVUF();

	int mantissa = ((id.i[BYTE_XOR_BE(0)] & 0x000fffff) << 11) | ((id.i[BYTE_XOR_BE(1)] & 0xffe00000) >> 21);
	int exponent = ((id.i[BYTE_XOR_BE(0)] & 0x7ff00000) >> 20) - 1023;
	if (exponent <= -128)
	{
		result.set_mantissa(0);
		result.set_exponent(-128);
		IREG(TMR_ST) |= UFFLAG | LUFFLAG | ZFLAG;
	}
	else if (exponent > 127)
	{
		if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
			result.set_mantissa(0x7fffffff);
		else
		{
			result.set_mantissa(0x80000001);
			IREG(TMR_ST) |= NFLAG;
		}
		result.set_exponent(127);
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}
	else if (val == 0)
	{
		result.set_mantissa(0);
		result.set_exponent(-128);
		IREG(TMR_ST) |= ZFLAG;
	}
	else if ((INT32)id.i[BYTE_XOR_BE(0)] >= 0)
	{
		result.set_mantissa(mantissa);
		result.set_exponent(exponent);
	}
	else if (mantissa != 0)
	{
		result.set_mantissa(0x80000000 | -mantissa);
		result.set_exponent(exponent);
		IREG(TMR_ST) |= NFLAG;
	}
	else
	{
		result.set_mantissa(0x80000000);
		result.set_exponent(exponent - 1);
		IREG(TMR_ST) |= NFLAG;
	}
}
#endif

// integer to floating point conversion
#if USE_FP
void tms3203x_device::int2float(tmsreg &srcdst)
{
	double val = srcdst.mantissa();
	double_to_dsp_with_flags(val, srcdst);
}
#else
void tms3203x_device::int2float(tmsreg &srcdst)
{
	UINT32 man = srcdst.mantissa();
	int exp, cnt;

	// never overflows or underflows
	CLR_NZVUF();

	// 0 always has exponent of -128
	if (man == 0)
	{
		man = 0x80000000;
		exp = -128;
	}

	// check for -1 here because count_leading_ones will infinite loop
	else if (man == (UINT32)-1)
	{
		man = 0;
		exp = -1;
	}

	// positive values; count leading zeros and shift
	else if ((INT32)man > 0)
	{
		cnt = count_leading_zeros(man);
		man <<= cnt;
		exp = 31 - cnt;
	}

	// negative values; count leading ones and shift
	else
	{
		cnt = count_leading_ones(man);
		man <<= cnt;
		exp = 31 - cnt;
	}

	// set the final results and compute NZ
	srcdst.set_mantissa(man ^ 0x80000000);
	srcdst.set_exponent(exp);
	OR_NZF(srcdst);
}
#endif


// floating point to integer conversion
#if USE_FP
void tms3203x_device::float2int(tmsreg &srcdst, int setflags)
{
	INT32 val;

	if (setflags) CLR_NZVUF();
	if (srcdst.exponent() > 30)
	{
		if ((INT32)srcdst.mantissa() >= 0)
			val = 0x7fffffff;
		else
			val = 0x80000000;
		if (setflags) IREG(TMR_ST) |= VFLAG | LVFLAG;
	}
	else
		val = floor(srcdst.as_double());
	srcdst.set_mantissa(val);
	if (setflags) OR_NZ(val);
}
#else
void tms3203x_device::float2int(tmsreg &srcdst, bool setflags)
{
	INT32 man = srcdst.mantissa();
	int shift = 31 - srcdst.exponent();

	// never underflows
	if (setflags) CLR_NZVUF();

	// if we've got too much to handle, overflow
	if (shift <= 0)
	{
		srcdst.set_mantissa((man >= 0) ? 0x7fffffff : 0x80000000);
		if (setflags) IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	// if we're too small, go to 0 or -1
	else if (shift > 31)
		srcdst.set_mantissa(man >> 31);

	// we're in the middle; shift it
	else
		srcdst.set_mantissa((man >> shift) ^ (1 << (31 - shift)));

	// set the NZ flags
	if (setflags) OR_NZ(srcdst.mantissa());
}
#endif


// compute the negative of a floating point value
#if USE_FP
void tms3203x_device::negf(tmsreg &dst, tmsreg tmsreg &src)
{
	double val = -src.as_double();
	double_to_dsp_with_flags(val, dst);
}
#else
void tms3203x_device::negf(tmsreg &dst, tmsreg &src)
{
	INT32 man = src.mantissa();

	CLR_NZVUF();

	if (src.exponent() == -128)
	{
		dst.set_mantissa(0);
		dst.set_exponent(-128);
	}
	else if ((man & 0x7fffffff) != 0)
	{
		dst.set_mantissa(-man);
		dst.set_exponent(src.exponent());
	}
	else
	{
		dst.set_mantissa(man ^ 0x80000000);
		if (man == 0)
			dst.set_exponent(src.exponent() - 1);
		else
			dst.set_exponent(src.exponent() + 1);
	}
	OR_NZF(dst);
}
#endif



// add two floating point values
#if USE_FP
void tms3203x_device::addf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	double val = src1.as_double() + src2.as_double();
	double_to_dsp_with_flags(val, dst);
}
#else
void tms3203x_device::addf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	// reset over/underflow conditions
	CLR_NZVUF();

	// first check for 0 operands
	if (src1.exponent() == -128)
	{
		dst = src2;
		OR_NZF(dst);
		return;
	}
	if (src2.exponent() == -128)
	{
		dst = src1;
		OR_NZF(dst);
		return;
	}

	// extract mantissas from 1.0.31 values to 1.1.31 values
	m1 = (INT64)src1.mantissa() ^ 0x80000000;
	m2 = (INT64)src2.mantissa() ^ 0x80000000;

	// normalize based on the exponent
	if (src1.exponent() > src2.exponent())
	{
		exp = src1.exponent();
		cnt = exp - src2.exponent();
		if (cnt >= 32)
		{
			dst = src1;
			OR_NZF(dst);
			return;
		}
		m2 >>= cnt;
	}
	else
	{
		exp = src2.exponent();
		cnt = exp - src1.exponent();
		if (cnt >= 32)
		{
			dst = src2;
			OR_NZF(dst);
			return;
		}
		m1 >>= cnt;
	}

	// add
	man = m1 + m2;

	// if the mantissa is zero, set the exponent appropriately
	if (man == 0 || exp == -128)
	{
		exp = -128;
		man = 0x80000000;
	}

	// if the mantissa is >= 2.0 or < -2.0, normalize
	else if (man >= ((INT64)2 << 31) || man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	// if the mantissa is < 1.0 and > -1.0, normalize
	else if (man < ((INT64)1 << 31) && man >= ((INT64)-1 << 31))
	{
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
	}

	// check for underflow
	if (exp <= -128)
	{
		man = 0x80000000;
		exp = -128;
		IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}

	// check for overflow
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	// store the result back, removing the implicit one and putting
	// back the sign bit
	dst.set_mantissa((UINT32)man ^ 0x80000000);
	dst.set_exponent(exp);
	OR_NZF(dst);
}
#endif


// subtract two floating point values
#if USE_FP
void tms3203x_device::subf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	double val = src1.as_double() - src2.as_double();
	double_to_dsp_with_flags(val, dst);
}
#else
void tms3203x_device::subf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	INT64 man;
	INT64 m1, m2;
	int exp, cnt;

	// reset over/underflow conditions
	CLR_NZVUF();

	// first check for 0 operands
	if (src2.exponent() == -128)
	{
		dst = src1;
		OR_NZF(dst);
		return;
	}

	// extract mantissas from 1.0.31 values to 1.1.31 values
	m1 = (INT64)src1.mantissa() ^ 0x80000000;
	m2 = (INT64)src2.mantissa() ^ 0x80000000;

	// normalize based on the exponent
	if (src1.exponent() > src2.exponent())
	{
		exp = src1.exponent();
		cnt = exp - src2.exponent();
		if (cnt >= 32)
		{
			dst = src1;
			OR_NZF(dst);
			return;
		}
		m2 >>= cnt;
	}
	else
	{
		exp = src2.exponent();
		cnt = exp - src1.exponent();
		if (cnt >= 32)
		{
			negf(dst, src2);
			return;
		}
		m1 >>= cnt;
	}

	// subtract
	man = m1 - m2;

	// if the mantissa is zero, set the exponent appropriately
	if (man == 0 || exp == -128)
	{
		exp = -128;
		man = 0x80000000;
	}

	// if the mantissa is >= 2.0 or < -2.0, normalize
	else if (man >= ((INT64)2 << 31) || man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	// if the mantissa is < 1.0 and > -1.0, normalize
	else if (man < ((INT64)1 << 31) && man >= ((INT64)-1 << 31))
	{
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
	}

	// check for underflow
	if (exp <= -128)
	{
		// make sure a 0 result doesn't set underflow
		if (man != 0 || exp < -128)
			IREG(TMR_ST) |= UFFLAG | LUFFLAG;
		man = 0x80000000;
		exp = -128;
	}

	// check for overflow
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	// store the result back, removing the implicit one and putting
	// back the sign bit
	dst.set_mantissa((UINT32)man ^ 0x80000000);
	dst.set_exponent(exp);
	OR_NZF(dst);
}
#endif


// multiply two floating point values
#if USE_FP
void tms3203x_device::mpyf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	double val = (double)src1.as_float() * (double)src2.as_float();
	double_to_dsp_with_flags(val, dst);
}
#else
void tms3203x_device::mpyf(tmsreg &dst, tmsreg &src1, tmsreg &src2)
{
	// reset over/underflow conditions
	CLR_NZVUF();

	// first check for 0 multipliers and return 0 in any case
	if (src1.exponent() == -128 || src2.exponent() == -128)
	{
		dst.set_mantissa(0);
		dst.set_exponent(-128);
		OR_NZF(dst);
		return;
	}

	// convert the mantissas from 1.0.31 numbers to 1.1.23 numbers
	INT32 m1 = (src1.mantissa() >> 8) ^ 0x800000;
	INT32 m2 = (src2.mantissa() >> 8) ^ 0x800000;

	// multiply the mantissas and add the exponents
	INT64 man = (INT64)m1 * (INT64)m2;
	int exp = src1.exponent() + src2.exponent();

	// chop off the low bits, going from 1.2.46 down to 1.2.31
	man >>= 46 - 31;

	// if the mantissa is zero, set the exponent appropriately
	if (man == 0)
	{
		exp = -128;
		man = 0x80000000;
	}

	// if the mantissa is >= 2.0 or <= -2.0, normalize
	else if (man >= ((INT64)2 << 31))
	{
		man >>= 1;
		exp++;
		if (man >= ((INT64)2 << 31))
		{
			man >>= 1;
			exp++;
		}
	}

	// if the mantissa is >= 2.0 or <= -2.0, normalize
	else if (man < ((INT64)-2 << 31))
	{
		man >>= 1;
		exp++;
	}

	// check for underflow
	if (exp <= -128)
	{
		man = 0x80000000;
		exp = -128;
		IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}

	// check for overflow
	else if (exp > 127)
	{
		man = (man < 0) ? 0x00000000 : 0xffffffff;
		exp = 127;
		IREG(TMR_ST) |= VFLAG | LVFLAG;
	}

	// store the result back, removing the implicit one and putting
	// back the sign bit
	dst.set_mantissa((UINT32)man ^ 0x80000000);
	dst.set_exponent(exp);
	OR_NZF(dst);
}
#endif


// normalize a floating point value
#if USE_FP
void tms3203x_device::norm(tmsreg &dst, tmsreg &src)
{
	fatalerror("norm not implemented\n");
}
#else
void tms3203x_device::norm(tmsreg &dst, tmsreg &src)
{
	INT32 man = src.mantissa();
	int exp = src.exponent();

	CLR_NZVUF();

	if (exp == -128 || man == 0)
	{
		dst.set_mantissa(0);
		dst.set_exponent(-128);
		if (man != 0)
			IREG(TMR_ST) |= UFFLAG | LUFFLAG;
	}
	else
	{
		int cnt;
		if (man > 0)
		{
			cnt = count_leading_zeros((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}
		else
		{
			cnt = count_leading_ones((UINT32)man);
			man <<= cnt;
			exp -= cnt;
		}

		// check for underflow
		if (exp <= -128)
		{
			man = 0x00000000;
			exp = -128;
			IREG(TMR_ST) |= UFFLAG | LUFFLAG;
		}
	}

	dst.set_mantissa(man);
	dst.set_exponent(exp);
	OR_NZF(dst);
}
#endif




//**************************************************************************
//  INDIRECT MEMORY REFS
//**************************************************************************

// immediate displacement variants

UINT32 tms3203x_device::mod00_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + (UINT8)op;
}

UINT32 tms3203x_device::mod01_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - (UINT8)op;
}

UINT32 tms3203x_device::mod02_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += (UINT8)op;
	return IREG(reg);
}

UINT32 tms3203x_device::mod03_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= (UINT8)op;
	return IREG(reg);
}

UINT32 tms3203x_device::mod04_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += (UINT8)op;
	return result;
}

UINT32 tms3203x_device::mod05_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= (UINT8)op;
	return result;
}

UINT32 tms3203x_device::mod06_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + (UINT8)op;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}

UINT32 tms3203x_device::mod07_d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - (UINT8)op;
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}


// immediate displacement variants (implied 1)

UINT32 tms3203x_device::mod00_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + 1;
}

UINT32 tms3203x_device::mod01_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - 1;
}

UINT32 tms3203x_device::mod02_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return ++IREG(reg);
}

UINT32 tms3203x_device::mod03_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return --IREG(reg);
}

UINT32 tms3203x_device::mod04_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg)++;
}

UINT32 tms3203x_device::mod05_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg)--;
}

UINT32 tms3203x_device::mod06_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + 1;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}

UINT32 tms3203x_device::mod07_1(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - 1;
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}


// IR0 displacement variants

UINT32 tms3203x_device::mod08(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR0);
}

UINT32 tms3203x_device::mod09(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR0);
}

UINT32 tms3203x_device::mod0a(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += IREG(TMR_IR0);
	return IREG(reg);
}

UINT32 tms3203x_device::mod0b(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= IREG(TMR_IR0);
	return IREG(reg);
}

UINT32 tms3203x_device::mod0c(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += IREG(TMR_IR0);
	return result;
}

UINT32 tms3203x_device::mod0d(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= IREG(TMR_IR0);
	return result;
}

UINT32 tms3203x_device::mod0e(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + IREG(TMR_IR0);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}

UINT32 tms3203x_device::mod0f(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - IREG(TMR_IR0);
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}


// IR1 displacement variants

UINT32 tms3203x_device::mod10(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR1);
}

UINT32 tms3203x_device::mod11(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR1);
}

UINT32 tms3203x_device::mod12(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) += IREG(TMR_IR1);
	return IREG(reg);
}

UINT32 tms3203x_device::mod13(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	IREG(reg) -= IREG(TMR_IR1);
	return IREG(reg);
}

UINT32 tms3203x_device::mod14(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) += IREG(TMR_IR1);
	return result;
}

UINT32 tms3203x_device::mod15(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	IREG(reg) -= IREG(TMR_IR1);
	return result;
}

UINT32 tms3203x_device::mod16(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + IREG(TMR_IR1);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}

UINT32 tms3203x_device::mod17(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - IREG(TMR_IR1);
	if (temp < 0)
		temp += IREG(TMR_BK);
	IREG(reg) = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	return result;
}


// special variants

UINT32 tms3203x_device::mod18(UINT32 op, UINT8 ar)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg);
}

UINT32 tms3203x_device::mod19(UINT32 op, UINT8 ar)
{
	unimplemented(op);
	return 0;
}

UINT32 tms3203x_device::modillegal(UINT32 op, UINT8 ar)
{
	illegal(op);
	return 0;
}


// immediate displacement variants (implied 1)

UINT32 tms3203x_device::mod00_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + 1;
}

UINT32 tms3203x_device::mod01_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - 1;
}

UINT32 tms3203x_device::mod02_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) + 1;
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod03_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) - 1;
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod04_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) + 1;
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod05_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) - 1;
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod06_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + 1;
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}

UINT32 tms3203x_device::mod07_1_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - 1;
	if (temp < 0)
		temp += IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}


// IR0 displacement variants

UINT32 tms3203x_device::mod08_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR0);
}

UINT32 tms3203x_device::mod09_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR0);
}

UINT32 tms3203x_device::mod0a_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) + IREG(TMR_IR0);
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod0b_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) - IREG(TMR_IR0);
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod0c_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) + IREG(TMR_IR0);
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod0d_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) - IREG(TMR_IR0);
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod0e_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + IREG(TMR_IR0);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}

UINT32 tms3203x_device::mod0f_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - IREG(TMR_IR0);
	if (temp < 0)
		temp += IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}


// IR1 displacement variants

UINT32 tms3203x_device::mod10_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) + IREG(TMR_IR1);
}

UINT32 tms3203x_device::mod11_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg) - IREG(TMR_IR1);
}

UINT32 tms3203x_device::mod12_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) + IREG(TMR_IR1);
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod13_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 defval = IREG(reg) - IREG(TMR_IR1);
	*defptrptr = defval;
	defptrptr = &IREG(reg);
	return defval;
}

UINT32 tms3203x_device::mod14_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) + IREG(TMR_IR1);
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod15_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	*defptrptr = IREG(reg) - IREG(TMR_IR1);
	defptrptr = &IREG(reg);
	return IREG(reg);
}

UINT32 tms3203x_device::mod16_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) + IREG(TMR_IR1);
	if (temp >= IREG(TMR_BK))
		temp -= IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}

UINT32 tms3203x_device::mod17_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	UINT32 result = IREG(reg);
	INT32 temp = (result & m_bkmask) - IREG(TMR_IR1);
	if (temp < 0)
		temp += IREG(TMR_BK);
	*defptrptr = (IREG(reg) & ~m_bkmask) | (temp & m_bkmask);
	defptrptr = &IREG(reg);
	return result;
}

UINT32 tms3203x_device::mod18_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	int reg = TMR_AR0 + (ar & 7);
	return IREG(reg);
}

UINT32 tms3203x_device::mod19_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	unimplemented(op);
	return 0;
}

UINT32 tms3203x_device::modillegal_def(UINT32 op, UINT8 ar, UINT32 *&defptrptr)
{
	illegal(op);
	return 0;
}


/*-----------------------------------------------------*/

#define ABSF(dreg, sreg)                                                \
{                                                                       \
	INT32 man = FREGMAN(sreg);                                          \
	CLR_NZVUF();                                                        \
	m_r[dreg] = m_r[sreg];                              \
	if (man < 0)                                                        \
	{                                                                   \
		m_r[dreg].set_mantissa(~man);                           \
		if (man == (INT32)0x80000000 && FREGEXP(sreg) == 127)           \
			IREG(TMR_ST) |= VFLAG | LVFLAG;                             \
	}                                                                   \
	OR_NZF(m_r[dreg]);                                          \
}

void tms3203x_device::absf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	int sreg = op & 7;
	ABSF(dreg, sreg);
}

void tms3203x_device::absf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

void tms3203x_device::absf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	ABSF(dreg, TMR_TEMP1);
}

void tms3203x_device::absf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	ABSF(dreg, TMR_TEMP1);
}

/*-----------------------------------------------------*/

#define ABSI(dreg, src)                                             \
{                                                                   \
	UINT32 _res = ((INT32)src < 0) ? -src : src;                    \
	if (!OVM() || _res != 0x80000000)                                   \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = 0x7fffffff;                                    \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
		if (_res == 0x80000000)                                     \
			IREG(TMR_ST) |= VFLAG | LVFLAG;                         \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::absi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

void tms3203x_device::absi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

void tms3203x_device::absi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

void tms3203x_device::absi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	ABSI(dreg, src);
}

/*-----------------------------------------------------*/

#define ADDC(dreg, src1, src2)                                      \
{                                                                   \
	UINT32 _res = src1 + src2 + (IREG(TMR_ST) & CFLAG);             \
	if (!OVM() || !OVERFLOW_ADD(src1,src2,_res))                    \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;   \
	if (dreg < 8)                                                   \
	{                                                               \
		UINT32 tempc = IREG(TMR_ST) & CFLAG;                        \
		CLR_NZCVUF();                                               \
		OR_C_ADC(src1,src2,tempc);                                  \
		OR_V_ADD(src1,src2,_res);                                   \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::addc_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

void tms3203x_device::addc_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

void tms3203x_device::addc_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

void tms3203x_device::addc_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDC(dreg, dst, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::addf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	addf(m_r[dreg], m_r[dreg], m_r[op & 7]);
}

void tms3203x_device::addf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	addf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::addf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	addf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::addf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	addf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define ADDI(dreg, src1, src2)                                      \
{                                                                   \
	UINT32 _res = src1 + src2;                                      \
	if (!OVM() || !OVERFLOW_ADD(src1,src2,_res))                    \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;   \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZCVUF();                                               \
		OR_C_ADD(src1,src2,_res);                                   \
		OR_V_ADD(src1,src2,_res);                                   \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::addi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

void tms3203x_device::addi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

void tms3203x_device::addi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

void tms3203x_device::addi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ADDI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define AND(dreg, src1, src2)                                       \
{                                                                   \
	UINT32 _res = (src1) & (src2);                                  \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::and_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

void tms3203x_device::and_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

void tms3203x_device::and_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

void tms3203x_device::and_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	AND(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define ANDN(dreg, src1, src2)                                      \
{                                                                   \
	UINT32 _res = (src1) & ~(src2);                                 \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::andn_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

void tms3203x_device::andn_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

void tms3203x_device::andn_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

void tms3203x_device::andn_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	ANDN(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define ASH(dreg, src, count)                                       \
{                                                                   \
	UINT32 _res;                                                    \
	INT32 _count = (INT16)(count << 9) >> 9;    /* 7 LSBs */        \
	if (_count < 0)                                                 \
	{                                                               \
		if (_count >= -31)                                          \
			_res = (INT32)src >> -_count;                           \
		else                                                        \
			_res = (INT32)src >> 31;                                \
	}                                                               \
	else                                                            \
	{                                                               \
		if (_count <= 31)                                           \
			_res = (INT32)src << _count;                            \
		else                                                        \
			_res = 0;                                               \
	}                                                               \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZCVUF();                                               \
		OR_NZ(_res);                                                \
		if (_count < 0)                                             \
		{                                                           \
			if (_count >= -32)                                      \
				OR_C(((INT32)src >> (-_count - 1)) & 1);            \
			else                                                    \
				OR_C(((INT32)src >> 31) & 1);                       \
		}                                                           \
		else if (_count > 0)                                        \
		{                                                           \
			if (_count <= 32)                                       \
				OR_C(((UINT32)src << (_count - 1)) >> 31);          \
		}                                                           \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::ash_reg(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = IREG(op & 31);
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

void tms3203x_device::ash_dir(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(DIRECT(op));
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

void tms3203x_device::ash_ind(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(INDIRECT_D(op, op >> 8));
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

void tms3203x_device::ash_imm(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = op;
	UINT32 src = IREG(dreg);
	ASH(dreg, src, count);
}

/*-----------------------------------------------------*/

void tms3203x_device::cmpf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(m_r[TMR_TEMP2], m_r[dreg], m_r[op & 7]);
}

void tms3203x_device::cmpf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[TMR_TEMP2], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::cmpf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[TMR_TEMP2], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::cmpf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	subf(m_r[TMR_TEMP2], m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define CMPI(src1, src2)                                            \
{                                                                   \
	UINT32 _res = src1 - src2;                                      \
	CLR_NZCVUF();                                                   \
	OR_C_SUB(src1,src2,_res);                                       \
	OR_V_SUB(src1,src2,_res);                                       \
	OR_NZ(_res);                                                    \
}

void tms3203x_device::cmpi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	UINT32 dst = IREG((op >> 16) & 31);
	CMPI(dst, src);
}

void tms3203x_device::cmpi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	UINT32 dst = IREG((op >> 16) & 31);
	CMPI(dst, src);
}

void tms3203x_device::cmpi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	UINT32 dst = IREG((op >> 16) & 31);
	CMPI(dst, src);
}

void tms3203x_device::cmpi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	UINT32 dst = IREG((op >> 16) & 31);
	CMPI(dst, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::fix_reg(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	m_r[TMR_TEMP1] = m_r[op & 7];
	float2int(m_r[TMR_TEMP1], dreg < 8);
	m_r[dreg].set_mantissa(m_r[TMR_TEMP1].mantissa());
}

void tms3203x_device::fix_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	float2int(m_r[TMR_TEMP1], dreg < 8);
	m_r[dreg].set_mantissa(m_r[TMR_TEMP1].mantissa());
}

void tms3203x_device::fix_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	float2int(m_r[TMR_TEMP1], dreg < 8);
	m_r[dreg].set_mantissa(m_r[TMR_TEMP1].mantissa());
}

void tms3203x_device::fix_imm(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	SHORT2FP(TMR_TEMP1, op);
	float2int(m_r[TMR_TEMP1], dreg < 8);
	m_r[dreg].set_mantissa(m_r[TMR_TEMP1].mantissa());
}

/*-----------------------------------------------------*/

#define FLOAT(dreg, src)                                            \
{                                                                   \
	IREG(dreg) = src;                                               \
	int2float(m_r[dreg]);                                   \
}

void tms3203x_device::float_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

void tms3203x_device::float_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

void tms3203x_device::float_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

void tms3203x_device::float_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 7;
	FLOAT(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::idle(UINT32 op)
{
	m_is_idling = true;
	IREG(TMR_ST) |= GIEFLAG;
	check_irqs();
	if (m_is_idling)
		m_icount = 0;
}

/*-----------------------------------------------------*/

void tms3203x_device::lde_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	m_r[dreg].set_exponent(m_r[op & 7].exponent());
	if (m_r[dreg].exponent() == -128)
		m_r[dreg].set_mantissa(0);
}

void tms3203x_device::lde_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	m_r[dreg].set_exponent(m_r[TMR_TEMP1].exponent());
	if (m_r[dreg].exponent() == -128)
		m_r[dreg].set_mantissa(0);
}

void tms3203x_device::lde_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	m_r[dreg].set_exponent(m_r[TMR_TEMP1].exponent());
	if (m_r[dreg].exponent() == -128)
		m_r[dreg].set_mantissa(0);
}

void tms3203x_device::lde_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	m_r[dreg].set_exponent(m_r[TMR_TEMP1].exponent());
	if (m_r[dreg].exponent() == -128)
		m_r[dreg].set_mantissa(0);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	m_r[dreg] = m_r[op & 7];
	CLR_NZVUF();
	OR_NZF(m_r[dreg]);
}

void tms3203x_device::ldf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
	CLR_NZVUF();
	OR_NZF(m_r[dreg]);
}

void tms3203x_device::ldf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
	CLR_NZVUF();
	OR_NZF(m_r[dreg]);
}

void tms3203x_device::ldf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(dreg, op);
	CLR_NZVUF();
	OR_NZF(m_r[dreg]);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfi_dir(UINT32 op) { unimplemented(op); }
void tms3203x_device::ldfi_ind(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

#define LDI(dreg, src)                                              \
{                                                                   \
	IREG(dreg) = src;                                               \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(src);                                                 \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::ldi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

void tms3203x_device::ldi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

void tms3203x_device::ldi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

void tms3203x_device::ldi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	LDI(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldii_dir(UINT32 op) { unimplemented(op); }
void tms3203x_device::ldii_ind(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

void tms3203x_device::ldm_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	m_r[dreg].set_mantissa(m_r[op & 7].mantissa());
}

void tms3203x_device::ldm_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	m_r[dreg].set_mantissa(res);
}

void tms3203x_device::ldm_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	m_r[dreg].set_mantissa(res);
}

void tms3203x_device::ldm_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	m_r[dreg].set_mantissa(m_r[TMR_TEMP1].mantissa());
}

/*-----------------------------------------------------*/

#define LSH(dreg, src, count)                                       \
{                                                                   \
	UINT32 _res;                                                    \
	INT32 _count = (INT16)(count << 9) >> 9;    /* 7 LSBs */        \
	if (_count < 0)                                                 \
	{                                                               \
		if (_count >= -31)                                          \
			_res = (UINT32)src >> -_count;                          \
		else                                                        \
			_res = 0;                                               \
	}                                                               \
	else                                                            \
	{                                                               \
		if (_count <= 31)                                           \
			_res = (UINT32)src << _count;                           \
		else                                                        \
			_res = 0;                                               \
	}                                                               \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZCVUF();                                               \
		OR_NZ(_res);                                                \
		if (_count < 0)                                             \
		{                                                           \
			if (_count >= -32)                                      \
				OR_C(((UINT32)src >> (-_count - 1)) & 1);           \
		}                                                           \
		else if (_count > 0)                                        \
		{                                                           \
			if (_count <= 32)                                       \
				OR_C(((UINT32)src << (_count - 1)) >> 31);          \
		}                                                           \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::lsh_reg(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = IREG(op & 31);
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

void tms3203x_device::lsh_dir(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(DIRECT(op));
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

void tms3203x_device::lsh_ind(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = RMEM(INDIRECT_D(op, op >> 8));
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

void tms3203x_device::lsh_imm(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	int count = op;
	UINT32 src = IREG(dreg);
	LSH(dreg, src, count);
}

/*-----------------------------------------------------*/

void tms3203x_device::mpyf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	mpyf(m_r[dreg], m_r[dreg], m_r[op & 31]);
}

void tms3203x_device::mpyf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	mpyf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::mpyf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	LONG2FP(TMR_TEMP1, res);
	mpyf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::mpyf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	SHORT2FP(TMR_TEMP1, op);
	mpyf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define MPYI(dreg, src1, src2)                                      \
{                                                                   \
	INT64 _res = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);\
	if (!OVM() || (_res >= -(INT64)0x80000000 && _res <= (INT64)0x7fffffff))        \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = (_res < 0) ? 0x80000000 : 0x7fffffff;          \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ((UINT32)_res);                                        \
		if (_res < -(INT64)0x80000000 || _res > (INT64)0x7fffffff)  \
			IREG(TMR_ST) |= VFLAG | LVFLAG;                         \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::mpyi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

void tms3203x_device::mpyi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

void tms3203x_device::mpyi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

void tms3203x_device::mpyi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	MPYI(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define NEGB(dreg, src)                                             \
{                                                                   \
	UINT32 _res = 0 - src - (IREG(TMR_ST) & CFLAG);                 \
	if (!OVM() || !OVERFLOW_SUB(0,src,_res))                        \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;    \
	if (dreg < 8)                                                   \
	{                                                               \
		UINT32 tempc = IREG(TMR_ST) & CFLAG;                        \
		CLR_NZCVUF();                                               \
		OR_C_SBB(0,src,tempc);                                      \
		OR_V_SUB(0,src,_res);                                       \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::negb_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

void tms3203x_device::negb_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

void tms3203x_device::negb_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

void tms3203x_device::negb_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	NEGB(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::negf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	negf(m_r[dreg], m_r[op & 7]);
}

void tms3203x_device::negf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	negf(m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::negf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	negf(m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::negf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	negf(m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NEGI(dreg, src)                                             \
{                                                                   \
	UINT32 _res = 0 - src;                                          \
	if (!OVM() || !OVERFLOW_SUB(0,src,_res))                        \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src < 0) ? 0x80000000 : 0x7fffffff;    \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZCVUF();                                               \
		OR_C_SUB(0,src,_res);                                       \
		OR_V_SUB(0,src,_res);                                       \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::negi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

void tms3203x_device::negi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

void tms3203x_device::negi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

void tms3203x_device::negi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	NEGI(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::nop_reg(UINT32 op)
{
}

void tms3203x_device::nop_ind(UINT32 op)
{
	RMEM(INDIRECT_D(op, op >> 8));
}

/*-----------------------------------------------------*/

void tms3203x_device::norm_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	norm(m_r[dreg], m_r[op & 7]);
}

void tms3203x_device::norm_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	norm(m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::norm_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	norm(m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::norm_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	norm(m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define NOT(dreg, src)                                              \
{                                                                   \
	UINT32 _res = ~(src);                                           \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::not_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

void tms3203x_device::not_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

void tms3203x_device::not_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

void tms3203x_device::not_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	NOT(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::pop(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 val = RMEM(IREG(TMR_SP)--);
	IREG(dreg) = val;
	if (dreg < 8)
	{
		CLR_NZVUF();
		OR_NZ(val);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::popf(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	UINT32 val = RMEM(IREG(TMR_SP)--);
	LONG2FP(dreg, val);
	CLR_NZVUF();
	OR_NZF(m_r[dreg]);
}

void tms3203x_device::push(UINT32 op)
{
	WMEM(++IREG(TMR_SP), IREG((op >> 16) & 31));
}

void tms3203x_device::pushf(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	WMEM(++IREG(TMR_SP), FP2LONG(dreg));
}

/*-----------------------------------------------------*/

#define OR(dreg, src1, src2)                                        \
{                                                                   \
	UINT32 _res = (src1) | (src2);                                  \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::or_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

void tms3203x_device::or_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

void tms3203x_device::or_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

void tms3203x_device::or_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	OR(dreg, dst, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::maxspeed(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

#define RND(dreg)                                                   \
{                                                                   \
	INT32 man = FREGMAN(dreg);                                      \
	CLR_NVUF();                                                     \
	if (man < 0x7fffff80)                                           \
	{                                                               \
		m_r[dreg].set_mantissa(((UINT32)man + 0x80) & 0xffffff00);  \
		OR_NUF(m_r[dreg]);                                  \
	}                                                               \
	else if (FREGEXP(dreg) < 127)                                   \
	{                                                               \
		m_r[dreg].set_mantissa(((UINT32)man + 0x80) & 0x7fffff00);  \
		m_r[dreg].set_exponent(FREGEXP(dreg) + 1);          \
		OR_NUF(m_r[dreg]);                                  \
	}                                                               \
	else                                                            \
	{                                                               \
		m_r[dreg].set_mantissa(0x7fffff00);             \
		IREG(TMR_ST) |= VFLAG | LVFLAG;                             \
	}                                                               \
}

void tms3203x_device::rnd_reg(UINT32 op)
{
	int sreg = op & 7;
	int dreg = (op >> 16) & 7;
	m_r[dreg] = m_r[sreg];
	RND(dreg);
}

void tms3203x_device::rnd_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
	RND(dreg);
}

void tms3203x_device::rnd_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
	RND(dreg);
}

void tms3203x_device::rnd_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(dreg, op);
	RND(dreg);
}

/*-----------------------------------------------------*/

void tms3203x_device::rol(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res >> 31;
	res = (res << 1) | newcflag;
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::rolc(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res >> 31;
	res = (res << 1) | (IREG(TMR_ST) & CFLAG);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::ror(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res & 1;
	res = (res >> 1) | (newcflag << 31);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::rorc(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	UINT32 res = IREG(dreg);
	int newcflag = res & 1;
	res = (res >> 1) | ((IREG(TMR_ST) & CFLAG) << 31);
	IREG(dreg) = res;
	if (dreg < 8)
	{
		CLR_NZCVUF();
		OR_NZ(res);
		OR_C(newcflag);
	}
	else if (dreg >= TMR_BK)
		update_special(dreg);
}

/*-----------------------------------------------------*/

void tms3203x_device::rtps_reg(UINT32 op)
{
	IREG(TMR_RC) = IREG(op & 31);
	IREG(TMR_RS) = m_pc;
	IREG(TMR_RE) = m_pc;
	IREG(TMR_ST) |= RMFLAG;
	m_icount -= 3*2;
	m_delayed = true;
}

void tms3203x_device::rtps_dir(UINT32 op)
{
	IREG(TMR_RC) = RMEM(DIRECT(op));
	IREG(TMR_RS) = m_pc;
	IREG(TMR_RE) = m_pc;
	IREG(TMR_ST) |= RMFLAG;
	m_icount -= 3*2;
	m_delayed = true;
}

void tms3203x_device::rtps_ind(UINT32 op)
{
	IREG(TMR_RC) = RMEM(INDIRECT_D(op, op >> 8));
	IREG(TMR_RS) = m_pc;
	IREG(TMR_RE) = m_pc;
	IREG(TMR_ST) |= RMFLAG;
	m_icount -= 3*2;
	m_delayed = true;
}

void tms3203x_device::rtps_imm(UINT32 op)
{
	IREG(TMR_RC) = (UINT16)op;
	IREG(TMR_RS) = m_pc;
	IREG(TMR_RE) = m_pc;
	IREG(TMR_ST) |= RMFLAG;
	m_icount -= 3*2;
	m_delayed = true;
}

/*-----------------------------------------------------*/

void tms3203x_device::stf_dir(UINT32 op)
{
	WMEM(DIRECT(op), FP2LONG((op >> 16) & 7));
}

void tms3203x_device::stf_ind(UINT32 op)
{
	WMEM(INDIRECT_D(op, op >> 8), FP2LONG((op >> 16) & 7));
}

/*-----------------------------------------------------*/

void tms3203x_device::stfi_dir(UINT32 op) { unimplemented(op); }
void tms3203x_device::stfi_ind(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

void tms3203x_device::sti_dir(UINT32 op)
{
	WMEM(DIRECT(op), IREG((op >> 16) & 31));
}

void tms3203x_device::sti_ind(UINT32 op)
{
	WMEM(INDIRECT_D(op, op >> 8), IREG((op >> 16) & 31));
}

/*-----------------------------------------------------*/

void tms3203x_device::stii_dir(UINT32 op) { unimplemented(op); }
void tms3203x_device::stii_ind(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

void tms3203x_device::sigi(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

#define SUBB(dreg, src1, src2)                                      \
{                                                                   \
	UINT32 _res = src1 - src2 - (IREG(TMR_ST) & CFLAG);             \
	if (!OVM() || !OVERFLOW_SUB(src1,src2,_res))                    \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;   \
	if (dreg < 8)                                                   \
	{                                                               \
		UINT32 tempc = IREG(TMR_ST) & CFLAG;                        \
		CLR_NZCVUF();                                               \
		OR_C_SBB(src1,src2,tempc);                                  \
		OR_V_SUB(src1,src2,_res);                                   \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::subb_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

void tms3203x_device::subb_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

void tms3203x_device::subb_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

void tms3203x_device::subb_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, dst, src);
}

/*-----------------------------------------------------*/

#define SUBC(dreg, src)                                             \
{                                                                   \
	UINT32 dst = IREG(dreg);                                        \
	if (dst >= src)                                                 \
		IREG(dreg) = ((dst - src) << 1) | 1;                        \
	else                                                            \
		IREG(dreg) = dst << 1;                                      \
	if (dreg >= TMR_BK)                                             \
		update_special(dreg);                                       \
}

void tms3203x_device::subc_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

void tms3203x_device::subc_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

void tms3203x_device::subc_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

void tms3203x_device::subc_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	SUBC(dreg, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::subf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(m_r[dreg], m_r[dreg], m_r[op & 7]);
}

void tms3203x_device::subf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::subf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

void tms3203x_device::subf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	subf(m_r[dreg], m_r[dreg], m_r[TMR_TEMP1]);
}

/*-----------------------------------------------------*/

#define SUBI(dreg, src1, src2)                                      \
{                                                                   \
	UINT32 _res = src1 - src2;                                      \
	if (!OVM() || !OVERFLOW_SUB(src1,src2,_res))                    \
		IREG(dreg) = _res;                                          \
	else                                                            \
		IREG(dreg) = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;   \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZCVUF();                                               \
		OR_C_SUB(src1,src2,_res);                                   \
		OR_V_SUB(src1,src2,_res);                                   \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::subi_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

void tms3203x_device::subi_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

void tms3203x_device::subi_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

void tms3203x_device::subi_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, dst, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::subrb_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

void tms3203x_device::subrb_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

void tms3203x_device::subrb_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

void tms3203x_device::subrb_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBB(dreg, src, dst);
}

/*-----------------------------------------------------*/

void tms3203x_device::subrf_reg(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	subf(m_r[dreg], m_r[op & 7], m_r[dreg]);
}

void tms3203x_device::subrf_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[dreg], m_r[TMR_TEMP1], m_r[dreg]);
}

void tms3203x_device::subrf_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, res);
	subf(m_r[dreg], m_r[TMR_TEMP1], m_r[dreg]);
}

void tms3203x_device::subrf_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(TMR_TEMP1, op);
	subf(m_r[dreg], m_r[TMR_TEMP1], m_r[dreg]);
}

/*-----------------------------------------------------*/

void tms3203x_device::subri_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

void tms3203x_device::subri_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

void tms3203x_device::subri_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

void tms3203x_device::subri_imm(UINT32 op)
{
	UINT32 src = (INT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	SUBI(dreg, src, dst);
}

/*-----------------------------------------------------*/

#define TSTB(src1, src2)                                            \
{                                                                   \
	UINT32 _res = (src1) & (src2);                                  \
	CLR_NZVUF();                                                    \
	OR_NZ(_res);                                                    \
}

void tms3203x_device::tstb_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	UINT32 dst = IREG((op >> 16) & 31);
	TSTB(dst, src);
}

void tms3203x_device::tstb_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	UINT32 dst = IREG((op >> 16) & 31);
	TSTB(dst, src);
}

void tms3203x_device::tstb_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	UINT32 dst = IREG((op >> 16) & 31);
	TSTB(dst, src);
}

void tms3203x_device::tstb_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	UINT32 dst = IREG((op >> 16) & 31);
	TSTB(dst, src);
}

/*-----------------------------------------------------*/

#define XOR(dreg, src1, src2)                                       \
{                                                                   \
	UINT32 _res = (src1) ^ (src2);                                  \
	IREG(dreg) = _res;                                              \
	if (dreg < 8)                                                   \
	{                                                               \
		CLR_NZVUF();                                                \
		OR_NZ(_res);                                                \
	}                                                               \
	else if (dreg >= TMR_BK)                                        \
		update_special(dreg);                                       \
}

void tms3203x_device::xor_reg(UINT32 op)
{
	UINT32 src = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

void tms3203x_device::xor_dir(UINT32 op)
{
	UINT32 src = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

void tms3203x_device::xor_ind(UINT32 op)
{
	UINT32 src = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

void tms3203x_device::xor_imm(UINT32 op)
{
	UINT32 src = (UINT16)op;
	int dreg = (op >> 16) & 31;
	UINT32 dst = IREG(dreg);
	XOR(dreg, dst, src);
}

/*-----------------------------------------------------*/

void tms3203x_device::iack_dir(UINT32 op)
{
	offs_t addr = DIRECT(op);
	m_iack_cb(addr, ASSERT_LINE);
	RMEM(addr);
	m_iack_cb(addr, CLEAR_LINE);
}

void tms3203x_device::iack_ind(UINT32 op)
{
	offs_t addr = INDIRECT_D(op, op >> 8);
	m_iack_cb(addr, ASSERT_LINE);
	RMEM(addr);
	m_iack_cb(addr, CLEAR_LINE);
}

/*-----------------------------------------------------*/

void tms3203x_device::addc3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

void tms3203x_device::addc3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

void tms3203x_device::addc3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ADDC(dreg, src1, src2);
}

void tms3203x_device::addc3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ADDC(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::addf3_regreg(UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	addf(m_r[dreg], m_r[sreg1], m_r[sreg2]);
}

void tms3203x_device::addf3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	addf(m_r[dreg], m_r[TMR_TEMP1], m_r[sreg2]);
}

void tms3203x_device::addf3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	addf(m_r[dreg], m_r[sreg1], m_r[TMR_TEMP2]);
}

void tms3203x_device::addf3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	addf(m_r[dreg], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

void tms3203x_device::addi3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

void tms3203x_device::addi3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

void tms3203x_device::addi3_regind(UINT32 op)
{
	// Radikal Bikers confirms via ADDI3 AR3,*AR3++(1),R2 / SUB $0001,R2 sequence
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ADDI(dreg, src1, src2);
}

void tms3203x_device::addi3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ADDI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::and3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

void tms3203x_device::and3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

void tms3203x_device::and3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	AND(dreg, src1, src2);
}

void tms3203x_device::and3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	AND(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::andn3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

void tms3203x_device::andn3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

void tms3203x_device::andn3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ANDN(dreg, src1, src2);
}

void tms3203x_device::andn3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ANDN(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::ash3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

void tms3203x_device::ash3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

void tms3203x_device::ash3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	ASH(dreg, src1, src2);
}

void tms3203x_device::ash3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	ASH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::cmpf3_regreg(UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	subf(m_r[TMR_TEMP1], m_r[sreg1], m_r[sreg2]);
}

void tms3203x_device::cmpf3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	int sreg2 = op & 7;
	LONG2FP(TMR_TEMP1, src1);
	subf(m_r[TMR_TEMP1], m_r[TMR_TEMP1], m_r[sreg2]);
}

void tms3203x_device::cmpf3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int sreg1 = (op >> 8) & 7;
	LONG2FP(TMR_TEMP2, src2);
	subf(m_r[TMR_TEMP1], m_r[sreg1], m_r[TMR_TEMP2]);
}

void tms3203x_device::cmpf3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	subf(m_r[TMR_TEMP1], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

void tms3203x_device::cmpi3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	CMPI(src1, src2);
}

void tms3203x_device::cmpi3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	CMPI(src1, src2);
}

void tms3203x_device::cmpi3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	CMPI(src1, src2);
}

void tms3203x_device::cmpi3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UPDATE_DEF();
	CMPI(src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::lsh3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

void tms3203x_device::lsh3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

void tms3203x_device::lsh3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	LSH(dreg, src1, src2);
}

void tms3203x_device::lsh3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	LSH(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::mpyf3_regreg(UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	mpyf(m_r[dreg], m_r[sreg1], m_r[sreg2]);
}

void tms3203x_device::mpyf3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	mpyf(m_r[dreg], m_r[TMR_TEMP1], m_r[sreg2]);
}

void tms3203x_device::mpyf3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	mpyf(m_r[dreg], m_r[sreg1], m_r[TMR_TEMP2]);
}

void tms3203x_device::mpyf3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	mpyf(m_r[dreg], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

void tms3203x_device::mpyi3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

void tms3203x_device::mpyi3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

void tms3203x_device::mpyi3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	MPYI(dreg, src1, src2);
}

void tms3203x_device::mpyi3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	MPYI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::or3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

void tms3203x_device::or3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

void tms3203x_device::or3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	OR(dreg, src1, src2);
}

void tms3203x_device::or3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	OR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::subb3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

void tms3203x_device::subb3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

void tms3203x_device::subb3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	SUBB(dreg, src1, src2);
}

void tms3203x_device::subb3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	SUBB(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::subf3_regreg(UINT32 op)
{
	int sreg1 = (op >> 8) & 7;
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	subf(m_r[dreg], m_r[sreg1], m_r[sreg2]);
}

void tms3203x_device::subf3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	int sreg2 = op & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP1, src1);
	subf(m_r[dreg], m_r[TMR_TEMP1], m_r[sreg2]);
}

void tms3203x_device::subf3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int sreg1 = (op >> 8) & 7;
	int dreg = (op >> 16) & 7;
	LONG2FP(TMR_TEMP2, src2);
	subf(m_r[dreg], m_r[sreg1], m_r[TMR_TEMP2]);
}

void tms3203x_device::subf3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 7;
	UPDATE_DEF();
	LONG2FP(TMR_TEMP1, src1);
	LONG2FP(TMR_TEMP2, src2);
	subf(m_r[dreg], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
}

/*-----------------------------------------------------*/

void tms3203x_device::subi3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

void tms3203x_device::subi3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

void tms3203x_device::subi3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	SUBI(dreg, src1, src2);
}

void tms3203x_device::subi3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	SUBI(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::tstb3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	TSTB(src1, src2);
}

void tms3203x_device::tstb3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	TSTB(src1, src2);
}

void tms3203x_device::tstb3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	TSTB(src1, src2);
}

void tms3203x_device::tstb3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UPDATE_DEF();
	TSTB(src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::xor3_regreg(UINT32 op)
{
	UINT32 src1 = IREG((op >> 8) & 31);
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

void tms3203x_device::xor3_indreg(UINT32 op)
{
	UINT32 src1 = RMEM(INDIRECT_1(op, op >> 8));
	UINT32 src2 = IREG(op & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

void tms3203x_device::xor3_regind(UINT32 op)
{
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	UINT32 src1 = IREG((op >> 8) & 31);
	int dreg = (op >> 16) & 31;
	XOR(dreg, src1, src2);
}

void tms3203x_device::xor3_indind(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src1 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src2 = RMEM(INDIRECT_1(op, op));
	int dreg = (op >> 16) & 31;
	UPDATE_DEF();
	XOR(dreg, src1, src2);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfu_reg(UINT32 op)
{
	m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfu_dir(UINT32 op)
{
	UINT32 res = RMEM(DIRECT(op));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
}

void tms3203x_device::ldfu_ind(UINT32 op)
{
	UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
	int dreg = (op >> 16) & 7;
	LONG2FP(dreg, res);
}

void tms3203x_device::ldfu_imm(UINT32 op)
{
	int dreg = (op >> 16) & 7;
	SHORT2FP(dreg, op);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldflo_reg(UINT32 op)
{
	if (CONDITION_LO())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldflo_dir(UINT32 op)
{
	if (CONDITION_LO())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldflo_ind(UINT32 op)
{
	if (CONDITION_LO())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldflo_imm(UINT32 op)
{
	if (CONDITION_LO())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfls_reg(UINT32 op)
{
	if (CONDITION_LS())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfls_dir(UINT32 op)
{
	if (CONDITION_LS())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfls_ind(UINT32 op)
{
	if (CONDITION_LS())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfls_imm(UINT32 op)
{
	if (CONDITION_LS())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfhi_reg(UINT32 op)
{
	if (CONDITION_HI())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfhi_dir(UINT32 op)
{
	if (CONDITION_HI())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfhi_ind(UINT32 op)
{
	if (CONDITION_HI())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfhi_imm(UINT32 op)
{
	if (CONDITION_HI())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfhs_reg(UINT32 op)
{
	if (CONDITION_HS())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfhs_dir(UINT32 op)
{
	if (CONDITION_HS())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfhs_ind(UINT32 op)
{
	if (CONDITION_HS())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfhs_imm(UINT32 op)
{
	if (CONDITION_HS())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfeq_reg(UINT32 op)
{
	if (CONDITION_EQ())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfeq_dir(UINT32 op)
{
	if (CONDITION_EQ())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfeq_ind(UINT32 op)
{
	if (CONDITION_EQ())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfeq_imm(UINT32 op)
{
	if (CONDITION_EQ())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfne_reg(UINT32 op)
{
	if (CONDITION_NE())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfne_dir(UINT32 op)
{
	if (CONDITION_NE())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfne_ind(UINT32 op)
{
	if (CONDITION_NE())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfne_imm(UINT32 op)
{
	if (CONDITION_NE())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldflt_reg(UINT32 op)
{
	if (CONDITION_LT())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldflt_dir(UINT32 op)
{
	if (CONDITION_LT())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldflt_ind(UINT32 op)
{
	if (CONDITION_LT())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldflt_imm(UINT32 op)
{
	if (CONDITION_LT())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfle_reg(UINT32 op)
{
	if (CONDITION_LE())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfle_dir(UINT32 op)
{
	if (CONDITION_LE())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfle_ind(UINT32 op)
{
	if (CONDITION_LE())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfle_imm(UINT32 op)
{
	if (CONDITION_LE())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfgt_reg(UINT32 op)
{
	if (CONDITION_GT())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfgt_dir(UINT32 op)
{
	if (CONDITION_GT())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfgt_ind(UINT32 op)
{
	if (CONDITION_GT())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfgt_imm(UINT32 op)
{
	if (CONDITION_GT())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfge_reg(UINT32 op)
{
	if (CONDITION_GE())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfge_dir(UINT32 op)
{
	if (CONDITION_GE())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfge_ind(UINT32 op)
{
	if (CONDITION_GE())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfge_imm(UINT32 op)
{
	if (CONDITION_GE())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfnv_reg(UINT32 op)
{
	if (CONDITION_NV())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfnv_dir(UINT32 op)
{
	if (CONDITION_NV())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfnv_ind(UINT32 op)
{
	if (CONDITION_NV())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfnv_imm(UINT32 op)
{
	if (CONDITION_NV())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfv_reg(UINT32 op)
{
	if (CONDITION_V())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfv_dir(UINT32 op)
{
	if (CONDITION_V())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfv_ind(UINT32 op)
{
	if (CONDITION_V())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfv_imm(UINT32 op)
{
	if (CONDITION_V())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfnuf_reg(UINT32 op)
{
	if (CONDITION_NUF())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfnuf_dir(UINT32 op)
{
	if (CONDITION_NUF())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfnuf_ind(UINT32 op)
{
	if (CONDITION_NUF())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfnuf_imm(UINT32 op)
{
	if (CONDITION_NUF())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfuf_reg(UINT32 op)
{
	if (CONDITION_UF())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfuf_dir(UINT32 op)
{
	if (CONDITION_UF())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfuf_ind(UINT32 op)
{
	if (CONDITION_UF())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfuf_imm(UINT32 op)
{
	if (CONDITION_UF())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfnlv_reg(UINT32 op)
{
	if (CONDITION_NLV())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfnlv_dir(UINT32 op)
{
	if (CONDITION_NLV())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfnlv_ind(UINT32 op)
{
	if (CONDITION_NLV())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfnlv_imm(UINT32 op)
{
	if (CONDITION_NLV())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldflv_reg(UINT32 op)
{
	if (CONDITION_LV())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldflv_dir(UINT32 op)
{
	if (CONDITION_LV())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldflv_ind(UINT32 op)
{
	if (CONDITION_LV())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldflv_imm(UINT32 op)
{
	if (CONDITION_LV())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfnluf_reg(UINT32 op)
{
	if (CONDITION_NLUF())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfnluf_dir(UINT32 op)
{
	if (CONDITION_NLUF())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfnluf_ind(UINT32 op)
{
	if (CONDITION_NLUF())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfnluf_imm(UINT32 op)
{
	if (CONDITION_NLUF())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfluf_reg(UINT32 op)
{
	if (CONDITION_LUF())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfluf_dir(UINT32 op)
{
	if (CONDITION_LUF())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfluf_ind(UINT32 op)
{
	if (CONDITION_LUF())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfluf_imm(UINT32 op)
{
	if (CONDITION_LUF())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfzuf_reg(UINT32 op)
{
	if (CONDITION_ZUF())
		m_r[(op >> 16) & 7] = m_r[op & 7];
}

void tms3203x_device::ldfzuf_dir(UINT32 op)
{
	if (CONDITION_ZUF())
	{
		UINT32 res = RMEM(DIRECT(op));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
}

void tms3203x_device::ldfzuf_ind(UINT32 op)
{
	if (CONDITION_ZUF())
	{
		UINT32 res = RMEM(INDIRECT_D(op, op >> 8));
		int dreg = (op >> 16) & 7;
		LONG2FP(dreg, res);
	}
	else
		INDIRECT_D(op, op >> 8);
}

void tms3203x_device::ldfzuf_imm(UINT32 op)
{
	if (CONDITION_ZUF())
	{
		int dreg = (op >> 16) & 7;
		SHORT2FP(dreg, op);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldiu_reg(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(dreg) = IREG(op & 31);
	if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::ldiu_dir(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(dreg) = RMEM(DIRECT(op));
	if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::ldiu_ind(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(dreg) = RMEM(INDIRECT_D(op, op >> 8));
	if (dreg >= TMR_BK)
		update_special(dreg);
}

void tms3203x_device::ldiu_imm(UINT32 op)
{
	int dreg = (op >> 16) & 31;
	IREG(dreg) = (INT16)op;
	if (dreg >= TMR_BK)
		update_special(dreg);
}

/*-----------------------------------------------------*/

void tms3203x_device::ldilo_reg(UINT32 op)
{
	if (CONDITION_LO())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilo_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LO())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilo_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LO())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilo_imm(UINT32 op)
{
	if (CONDITION_LO())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldils_reg(UINT32 op)
{
	if (CONDITION_LS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldils_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldils_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldils_imm(UINT32 op)
{
	if (CONDITION_LS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldihi_reg(UINT32 op)
{
	if (CONDITION_HI())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihi_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_HI())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihi_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_HI())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihi_imm(UINT32 op)
{
	if (CONDITION_HI())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldihs_reg(UINT32 op)
{
	if (CONDITION_HS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihs_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_HS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihs_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_HS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldihs_imm(UINT32 op)
{
	if (CONDITION_HS())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldieq_reg(UINT32 op)
{
	if (CONDITION_EQ())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldieq_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_EQ())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldieq_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_EQ())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldieq_imm(UINT32 op)
{
	if (CONDITION_EQ())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldine_reg(UINT32 op)
{
	if (CONDITION_NE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldine_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_NE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldine_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_NE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldine_imm(UINT32 op)
{
	if (CONDITION_NE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldilt_reg(UINT32 op)
{
	if (CONDITION_LT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilt_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilt_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilt_imm(UINT32 op)
{
	if (CONDITION_LT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldile_reg(UINT32 op)
{
	if (CONDITION_LE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldile_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldile_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldile_imm(UINT32 op)
{
	if (CONDITION_LE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldigt_reg(UINT32 op)
{
	if (CONDITION_GT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldigt_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_GT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldigt_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_GT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldigt_imm(UINT32 op)
{
	if (CONDITION_GT())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldige_reg(UINT32 op)
{
	if (CONDITION_GE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldige_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_GE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldige_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_GE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldige_imm(UINT32 op)
{
	if (CONDITION_GE())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldinv_reg(UINT32 op)
{
	if (CONDITION_NV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinv_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_NV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinv_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_NV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinv_imm(UINT32 op)
{
	if (CONDITION_NV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldiuf_reg(UINT32 op)
{
	if (CONDITION_UF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiuf_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_UF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiuf_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_UF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiuf_imm(UINT32 op)
{
	if (CONDITION_UF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldinuf_reg(UINT32 op)
{
	if (CONDITION_NUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinuf_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_NUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinuf_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_NUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinuf_imm(UINT32 op)
{
	if (CONDITION_NUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldiv_reg(UINT32 op)
{
	if (CONDITION_V())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiv_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_V())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiv_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_V())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiv_imm(UINT32 op)
{
	if (CONDITION_V())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldinlv_reg(UINT32 op)
{
	if (CONDITION_NLV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinlv_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_NLV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinlv_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_NLV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinlv_imm(UINT32 op)
{
	if (CONDITION_NLV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldilv_reg(UINT32 op)
{
	if (CONDITION_LV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilv_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilv_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldilv_imm(UINT32 op)
{
	if (CONDITION_LV())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldinluf_reg(UINT32 op)
{
	if (CONDITION_NLUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinluf_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_NLUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinluf_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_NLUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldinluf_imm(UINT32 op)
{
	if (CONDITION_NLUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldiluf_reg(UINT32 op)
{
	if (CONDITION_LUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiluf_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_LUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiluf_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_LUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldiluf_imm(UINT32 op)
{
	if (CONDITION_LUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::ldizuf_reg(UINT32 op)
{
	if (CONDITION_ZUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = IREG(op & 31);
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldizuf_dir(UINT32 op)
{
	UINT32 val = RMEM(DIRECT(op));
	if (CONDITION_ZUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldizuf_ind(UINT32 op)
{
	UINT32 val = RMEM(INDIRECT_D(op, op >> 8));
	if (CONDITION_ZUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = val;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

void tms3203x_device::ldizuf_imm(UINT32 op)
{
	if (CONDITION_ZUF())
	{
		int dreg = (op >> 16) & 31;
		IREG(dreg) = (INT16)op;
		if (dreg >= TMR_BK)
			update_special(dreg);
	}
}

/*-----------------------------------------------------*/

inline void tms3203x_device::execute_delayed(UINT32 newpc)
{
	m_delayed = true;

	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		execute_one();
		execute_one();
		execute_one();
	}
	else
	{
		debugger_instruction_hook(this, m_pc);
		execute_one();
		debugger_instruction_hook(this, m_pc);
		execute_one();
		debugger_instruction_hook(this, m_pc);
		execute_one();
	}

	if (newpc != ~0)
		m_pc = newpc;

	m_delayed = false;
	if (m_irq_pending)
	{
		m_irq_pending = false;
		check_irqs();
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::br_imm(UINT32 op)
{
	m_pc = op & 0xffffff;
	m_icount -= 3*2;
}

void tms3203x_device::brd_imm(UINT32 op)
{
	execute_delayed(op & 0xffffff);
}

/*-----------------------------------------------------*/

void tms3203x_device::call_imm(UINT32 op)
{
	WMEM(++IREG(TMR_SP), m_pc);
	m_pc = op & 0xffffff;
	m_icount -= 3*2;
}

/*-----------------------------------------------------*/

void tms3203x_device::rptb_imm(UINT32 op)
{
	IREG(TMR_RS) = m_pc;
	IREG(TMR_RE) = op & 0xffffff;
	IREG(TMR_ST) |= RMFLAG;
	m_icount -= 3*2;
}

/*-----------------------------------------------------*/

void tms3203x_device::swi(UINT32 op) { unimplemented(op); }

/*-----------------------------------------------------*/

void tms3203x_device::brc_reg(UINT32 op)
{
	if (condition(op >> 16))
	{
		m_pc = IREG(op & 31);
		m_icount -= 3*2;
	}
}

void tms3203x_device::brcd_reg(UINT32 op)
{
	if (condition(op >> 16))
		execute_delayed(IREG(op & 31));
	else
		execute_delayed(~0);
}

void tms3203x_device::brc_imm(UINT32 op)
{
	if (condition(op >> 16))
	{
		m_pc += (INT16)op;
		m_icount -= 3*2;
	}
}

void tms3203x_device::brcd_imm(UINT32 op)
{
	if (condition(op >> 16))
		execute_delayed(m_pc + 2 + (INT16)op);
	else
		execute_delayed(~0);
}

/*-----------------------------------------------------*/

void tms3203x_device::dbc_reg(UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(op >> 16) && !(res & 0x800000))
	{
		m_pc = IREG(op & 31);
		m_icount -= 3*2;
	}
}

void tms3203x_device::dbcd_reg(UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(op >> 16) && !(res & 0x800000))
		execute_delayed(IREG(op & 31));
	else
		execute_delayed(~0);
}

void tms3203x_device::dbc_imm(UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(op >> 16) && !(res & 0x800000))
	{
		m_pc += (INT16)op;
		m_icount -= 3*2;
	}
}

void tms3203x_device::dbcd_imm(UINT32 op)
{
	int reg = TMR_AR0 + ((op >> 22) & 7);
	int res = (IREG(reg) - 1) & 0xffffff;
	IREG(reg) = res | (IREG(reg) & 0xff000000);
	if (condition(op >> 16) && !(res & 0x800000))
		execute_delayed(m_pc + 2 + (INT16)op);
	else
		execute_delayed(~0);
}

/*-----------------------------------------------------*/

void tms3203x_device::callc_reg(UINT32 op)
{
	if (condition(op >> 16))
	{
		WMEM(++IREG(TMR_SP), m_pc);
		m_pc = IREG(op & 31);
		m_icount -= 3*2;
	}
}

void tms3203x_device::callc_imm(UINT32 op)
{
	if (condition(op >> 16))
	{
		WMEM(++IREG(TMR_SP), m_pc);
		m_pc += (INT16)op;
		m_icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::trap(int trapnum)
{
	WMEM(++IREG(TMR_SP), m_pc);
	IREG(TMR_ST) &= ~GIEFLAG;
	if (m_chip_type == CHIP_TYPE_TMS32032)
		m_pc = RMEM(((IREG(TMR_IF) >> 16) << 8) + trapnum);
	else
		m_pc = RMEM(trapnum);
	m_icount -= 4*2;
}

void tms3203x_device::trapc(UINT32 op)
{
	if (condition(op >> 16))
		trap(op & 0x3f);
}

/*-----------------------------------------------------*/

void tms3203x_device::retic_reg(UINT32 op)
{
	if (condition(op >> 16))
	{
		m_pc = RMEM(IREG(TMR_SP)--);
		IREG(TMR_ST) |= GIEFLAG;
		m_icount -= 3*2;
		check_irqs();
	}
}

void tms3203x_device::retsc_reg(UINT32 op)
{
	if (condition(op >> 16))
	{
		m_pc = RMEM(IREG(TMR_SP)--);
		m_icount -= 3*2;
	}
}

/*-----------------------------------------------------*/

void tms3203x_device::mpyaddf_0(UINT32 op)
{
	// src3 * src4, src1 + src2
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
	addf(m_r[((op >> 22) & 1) | 2], m_r[(op >> 19) & 7], m_r[(op >> 16) & 7]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpyaddf_1(UINT32 op)
{
	// src3 * src1, src4 + src2
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[(op >> 19) & 7]);
	addf(m_r[((op >> 22) & 1) | 2], m_r[TMR_TEMP2], m_r[(op >> 16) & 7]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpyaddf_2(UINT32 op)
{
	// src1 * src2, src3 + src4
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[(op >> 19) & 7], m_r[(op >> 16) & 7]);
	addf(m_r[((op >> 22) & 1) | 2], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpyaddf_3(UINT32 op)
{
	// src3 * src1, src2 + src4
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[(op >> 19) & 7]);
	addf(m_r[((op >> 22) & 1) | 2], m_r[(op >> 16) & 7], m_r[TMR_TEMP2]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

void tms3203x_device::mpysubf_0(UINT32 op)
{
	// src3 * src4, src1 - src2
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
	subf(m_r[((op >> 22) & 1) | 2], m_r[(op >> 19) & 7], m_r[(op >> 16) & 7]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpysubf_1(UINT32 op)
{
	// src3 * src1, src4 - src2
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[(op >> 19) & 7]);
	subf(m_r[((op >> 22) & 1) | 2], m_r[TMR_TEMP2], m_r[(op >> 16) & 7]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpysubf_2(UINT32 op)
{
	// src1 * src2, src3 - src4
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[(op >> 19) & 7], m_r[(op >> 16) & 7]);
	subf(m_r[((op >> 22) & 1) | 2], m_r[TMR_TEMP1], m_r[TMR_TEMP2]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

void tms3203x_device::mpysubf_3(UINT32 op)
{
	// src3 * src1, src2 - src4
	DECLARE_DEF;
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	LONG2FP(TMR_TEMP1, src3);
	LONG2FP(TMR_TEMP2, src4);
	mpyf(m_r[TMR_TEMP3], m_r[TMR_TEMP1], m_r[(op >> 19) & 7]);
	subf(m_r[((op >> 22) & 1) | 2], m_r[(op >> 16) & 7], m_r[TMR_TEMP2]);
	m_r[(op >> 23) & 1] = m_r[TMR_TEMP3];
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

void tms3203x_device::mpyaddi_0(UINT32 op)
{
	// src3 * src4, src1 + src2
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 + src2;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpyaddi_1(UINT32 op)
{
	// src3 * src1, src4 + src2
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 + src2;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpyaddi_2(UINT32 op)
{
	// src1 * src2, src3 + src4
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 + src4;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpyaddi_3(UINT32 op)
{
	// src3 * src1, src2 + src4
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 + src4;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_ADD(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

void tms3203x_device::mpysubi_0(UINT32 op)
{
	// src3 * src4, src1 - src2
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src4 << 8) >> 8);
	UINT32 ares = src1 - src2;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src1,src2,ares))
			ares = ((INT32)src1 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpysubi_1(UINT32 op)
{
	// src3 * src1, src4 - src2
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src4 - src2;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src4,src2,ares))
			ares = ((INT32)src4 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpysubi_2(UINT32 op)
{
	// src1 * src2, src3 - src4
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src1 << 8) >> 8) * (INT64)((INT32)(src2 << 8) >> 8);
	UINT32 ares = src3 - src4;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src3,src4,ares))
			ares = ((INT32)src3 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

void tms3203x_device::mpysubi_3(UINT32 op)
{
	// src3 * src1, src2 - src4
	DECLARE_DEF;
	UINT32 src1 = IREG((op >> 19) & 7);
	UINT32 src2 = IREG((op >> 16) & 7);
	UINT32 src3 = RMEM(INDIRECT_1_DEF(op, op >> 8));
	UINT32 src4 = RMEM(INDIRECT_1(op, op));
	INT64 mres = (INT64)((INT32)(src3 << 8) >> 8) * (INT64)((INT32)(src1 << 8) >> 8);
	UINT32 ares = src2 - src4;

	CLR_NZVUF();
	if (OVM())
	{
		if (mres < -(INT64)0x80000000 || mres > (INT64)0x7fffffff)
			mres = (mres < 0) ? 0x80000000 : 0x7fffffff;
		if (OVERFLOW_SUB(src2,src4,ares))
			ares = ((INT32)src2 < 0) ? 0x80000000 : 0x7fffffff;
	}
	IREG((op >> 23) & 1) = mres;
	IREG(((op >> 22) & 1) | 2) = ares;
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

void tms3203x_device::stfstf(UINT32 op)
{
	DECLARE_DEF;
	WMEM(INDIRECT_1_DEF(op, op >> 8), FP2LONG((op >> 16) & 7));
	WMEM(INDIRECT_1(op, op), FP2LONG((op >> 22) & 7));
	UPDATE_DEF();
}

void tms3203x_device::stisti(UINT32 op)
{
	DECLARE_DEF;
	WMEM(INDIRECT_1_DEF(op, op >> 8), IREG((op >> 16) & 7));
	WMEM(INDIRECT_1(op, op), IREG((op >> 22) & 7));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

void tms3203x_device::ldfldf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 res;
	int dreg;

	res = RMEM(INDIRECT_1_DEF(op, op >> 8));
	dreg = (op >> 19) & 7;
	LONG2FP(dreg, res);
	res = RMEM(INDIRECT_1(op, op));
	dreg = (op >> 22) & 7;
	LONG2FP(dreg, res);
	UPDATE_DEF();
}

void tms3203x_device::ldildi(UINT32 op)
{
	DECLARE_DEF;
	IREG((op >> 19) & 7) = RMEM(INDIRECT_1_DEF(op, op >> 8));
	IREG((op >> 22) & 7) = RMEM(INDIRECT_1(op, op));
	UPDATE_DEF();
}

/*-----------------------------------------------------*/

//  src2 = ind(op)
//  dst2 = ind(op >> 8)
//  sreg3 = ((op >> 16) & 7)
//  sreg1 = ((op >> 19) & 7)
//  dreg1 = ((op >> 22) & 7)

void tms3203x_device::absfstf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(TMR_TEMP1, src2);
		ABSF(dreg, TMR_TEMP1);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::absisti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		ABSI(dreg, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::addf3stf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		LONG2FP(TMR_TEMP1, src2);
		addf(m_r[(op >> 22) & 7], m_r[(op >> 19) & 7], m_r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::addi3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		ADDI(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::and3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		AND(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::ash3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 count = IREG((op >> 19) & 7);
		ASH(dreg, src2, count);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::fixsti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(dreg, src2);
		float2int(m_r[dreg], 1);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::floatstf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		IREG(dreg) = src2;
		int2float(m_r[dreg]);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::ldfstf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		LONG2FP(dreg, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::ldisti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	IREG((op >> 22) & 7) = src2;
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::lsh3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 count = IREG((op >> 19) & 7);
		LSH(dreg, src2, count);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::mpyf3stf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		LONG2FP(TMR_TEMP1, src2);
		mpyf(m_r[(op >> 22) & 7], m_r[(op >> 19) & 7], m_r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::mpyi3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		MPYI(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::negfstf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		LONG2FP(TMR_TEMP1, src2);
		negf(m_r[(op >> 22) & 7], m_r[TMR_TEMP1]);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::negisti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		NEGI(dreg, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::notsti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		NOT(dreg, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::or3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		OR(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::subf3stf(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = FP2LONG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		LONG2FP(TMR_TEMP1, src2);
		subf(m_r[(op >> 22) & 7], m_r[TMR_TEMP1], m_r[(op >> 19) & 7]);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::subi3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		SUBI(dreg, src2, src1);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}

void tms3203x_device::xor3sti(UINT32 op)
{
	DECLARE_DEF;
	UINT32 src3 = IREG((op >> 16) & 7);
	UINT32 src2 = RMEM(INDIRECT_1_DEF(op, op));
	{
		int dreg = (op >> 22) & 7;
		UINT32 src1 = IREG((op >> 19) & 7);
		XOR(dreg, src1, src2);
	}
	WMEM(INDIRECT_1(op, op >> 8), src3);
	UPDATE_DEF();
}


//**************************************************************************
//  FUNCTION TABLE
//**************************************************************************

UINT32 (tms3203x_device::*const tms3203x_device::s_indirect_d[0x20])(UINT32, UINT8) =
{
	&tms3203x_device::mod00_d,      &tms3203x_device::mod01_d,      &tms3203x_device::mod02_d,      &tms3203x_device::mod03_d,
	&tms3203x_device::mod04_d,      &tms3203x_device::mod05_d,      &tms3203x_device::mod06_d,      &tms3203x_device::mod07_d,
	&tms3203x_device::mod08,        &tms3203x_device::mod09,        &tms3203x_device::mod0a,        &tms3203x_device::mod0b,
	&tms3203x_device::mod0c,        &tms3203x_device::mod0d,        &tms3203x_device::mod0e,        &tms3203x_device::mod0f,
	&tms3203x_device::mod10,        &tms3203x_device::mod11,        &tms3203x_device::mod12,        &tms3203x_device::mod13,
	&tms3203x_device::mod14,        &tms3203x_device::mod15,        &tms3203x_device::mod16,        &tms3203x_device::mod17,
	&tms3203x_device::mod18,        &tms3203x_device::mod19,        &tms3203x_device::modillegal,   &tms3203x_device::modillegal,
	&tms3203x_device::modillegal,   &tms3203x_device::modillegal,   &tms3203x_device::modillegal,   &tms3203x_device::modillegal
};


UINT32 (tms3203x_device::*const tms3203x_device::s_indirect_1[0x20])(UINT32, UINT8) =
{
	&tms3203x_device::mod00_1,      &tms3203x_device::mod01_1,      &tms3203x_device::mod02_1,      &tms3203x_device::mod03_1,
	&tms3203x_device::mod04_1,      &tms3203x_device::mod05_1,      &tms3203x_device::mod06_1,      &tms3203x_device::mod07_1,
	&tms3203x_device::mod08,        &tms3203x_device::mod09,        &tms3203x_device::mod0a,        &tms3203x_device::mod0b,
	&tms3203x_device::mod0c,        &tms3203x_device::mod0d,        &tms3203x_device::mod0e,        &tms3203x_device::mod0f,
	&tms3203x_device::mod10,        &tms3203x_device::mod11,        &tms3203x_device::mod12,        &tms3203x_device::mod13,
	&tms3203x_device::mod14,        &tms3203x_device::mod15,        &tms3203x_device::mod16,        &tms3203x_device::mod17,
	&tms3203x_device::mod18,        &tms3203x_device::mod19,        &tms3203x_device::modillegal,   &tms3203x_device::modillegal,
	&tms3203x_device::modillegal,   &tms3203x_device::modillegal,   &tms3203x_device::modillegal,   &tms3203x_device::modillegal
};


UINT32 (tms3203x_device::*const tms3203x_device::s_indirect_1_def[0x20])(UINT32, UINT8, UINT32 *&) =
{
	&tms3203x_device::mod00_1_def,  &tms3203x_device::mod01_1_def,  &tms3203x_device::mod02_1_def,  &tms3203x_device::mod03_1_def,
	&tms3203x_device::mod04_1_def,  &tms3203x_device::mod05_1_def,  &tms3203x_device::mod06_1_def,  &tms3203x_device::mod07_1_def,
	&tms3203x_device::mod08_def,    &tms3203x_device::mod09_def,    &tms3203x_device::mod0a_def,    &tms3203x_device::mod0b_def,
	&tms3203x_device::mod0c_def,    &tms3203x_device::mod0d_def,    &tms3203x_device::mod0e_def,    &tms3203x_device::mod0f_def,
	&tms3203x_device::mod10_def,    &tms3203x_device::mod11_def,    &tms3203x_device::mod12_def,    &tms3203x_device::mod13_def,
	&tms3203x_device::mod14_def,    &tms3203x_device::mod15_def,    &tms3203x_device::mod16_def,    &tms3203x_device::mod17_def,
	&tms3203x_device::mod18_def,    &tms3203x_device::mod19_def,    &tms3203x_device::modillegal_def,&tms3203x_device::modillegal_def,
	&tms3203x_device::modillegal_def,&tms3203x_device::modillegal_def,&tms3203x_device::modillegal_def,&tms3203x_device::modillegal_def
};

void (tms3203x_device::*const tms3203x_device::s_tms32031ops[])(UINT32 op) =
{
	&tms3203x_device::absf_reg,     &tms3203x_device::absf_dir,     &tms3203x_device::absf_ind,     &tms3203x_device::absf_imm,     // 0x00
	&tms3203x_device::absi_reg,     &tms3203x_device::absi_dir,     &tms3203x_device::absi_ind,     &tms3203x_device::absi_imm,
	&tms3203x_device::addc_reg,     &tms3203x_device::addc_dir,     &tms3203x_device::addc_ind,     &tms3203x_device::addc_imm,
	&tms3203x_device::addf_reg,     &tms3203x_device::addf_dir,     &tms3203x_device::addf_ind,     &tms3203x_device::addf_imm,
	&tms3203x_device::addi_reg,     &tms3203x_device::addi_dir,     &tms3203x_device::addi_ind,     &tms3203x_device::addi_imm,
	&tms3203x_device::and_reg,      &tms3203x_device::and_dir,      &tms3203x_device::and_ind,      &tms3203x_device::and_imm,
	&tms3203x_device::andn_reg,     &tms3203x_device::andn_dir,     &tms3203x_device::andn_ind,     &tms3203x_device::andn_imm,
	&tms3203x_device::ash_reg,      &tms3203x_device::ash_dir,      &tms3203x_device::ash_ind,      &tms3203x_device::ash_imm,
	&tms3203x_device::cmpf_reg,     &tms3203x_device::cmpf_dir,     &tms3203x_device::cmpf_ind,     &tms3203x_device::cmpf_imm,     // 0x08
	&tms3203x_device::cmpi_reg,     &tms3203x_device::cmpi_dir,     &tms3203x_device::cmpi_ind,     &tms3203x_device::cmpi_imm,
	&tms3203x_device::fix_reg,      &tms3203x_device::fix_dir,      &tms3203x_device::fix_ind,      &tms3203x_device::fix_imm,
	&tms3203x_device::float_reg,    &tms3203x_device::float_dir,    &tms3203x_device::float_ind,    &tms3203x_device::float_imm,
	&tms3203x_device::idle,         &tms3203x_device::idle,         &tms3203x_device::idle,         &tms3203x_device::idle,
	&tms3203x_device::lde_reg,      &tms3203x_device::lde_dir,      &tms3203x_device::lde_ind,      &tms3203x_device::lde_imm,
	&tms3203x_device::ldf_reg,      &tms3203x_device::ldf_dir,      &tms3203x_device::ldf_ind,      &tms3203x_device::ldf_imm,
	&tms3203x_device::illegal,      &tms3203x_device::ldfi_dir,     &tms3203x_device::ldfi_ind,     &tms3203x_device::illegal,
	&tms3203x_device::ldi_reg,      &tms3203x_device::ldi_dir,      &tms3203x_device::ldi_ind,      &tms3203x_device::ldi_imm,      // 0x10
	&tms3203x_device::illegal,      &tms3203x_device::ldii_dir,     &tms3203x_device::ldii_ind,     &tms3203x_device::illegal,
	&tms3203x_device::ldm_reg,      &tms3203x_device::ldm_dir,      &tms3203x_device::ldm_ind,      &tms3203x_device::ldm_imm,
	&tms3203x_device::lsh_reg,      &tms3203x_device::lsh_dir,      &tms3203x_device::lsh_ind,      &tms3203x_device::lsh_imm,
	&tms3203x_device::mpyf_reg,     &tms3203x_device::mpyf_dir,     &tms3203x_device::mpyf_ind,     &tms3203x_device::mpyf_imm,
	&tms3203x_device::mpyi_reg,     &tms3203x_device::mpyi_dir,     &tms3203x_device::mpyi_ind,     &tms3203x_device::mpyi_imm,
	&tms3203x_device::negb_reg,     &tms3203x_device::negb_dir,     &tms3203x_device::negb_ind,     &tms3203x_device::negb_imm,
	&tms3203x_device::negf_reg,     &tms3203x_device::negf_dir,     &tms3203x_device::negf_ind,     &tms3203x_device::negf_imm,
	&tms3203x_device::negi_reg,     &tms3203x_device::negi_dir,     &tms3203x_device::negi_ind,     &tms3203x_device::negi_imm,     // 0x18
	&tms3203x_device::nop_reg,      &tms3203x_device::illegal,      &tms3203x_device::nop_ind,      &tms3203x_device::illegal,
	&tms3203x_device::norm_reg,     &tms3203x_device::norm_dir,     &tms3203x_device::norm_ind,     &tms3203x_device::norm_imm,
	&tms3203x_device::not_reg,      &tms3203x_device::not_dir,      &tms3203x_device::not_ind,      &tms3203x_device::not_imm,
	&tms3203x_device::illegal,      &tms3203x_device::pop,          &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::popf,         &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::push,         &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::pushf,        &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::or_reg,       &tms3203x_device::or_dir,       &tms3203x_device::or_ind,       &tms3203x_device::or_imm,       // 0x20
	&tms3203x_device::maxspeed,     &tms3203x_device::maxspeed,     &tms3203x_device::maxspeed,     &tms3203x_device::maxspeed,
	&tms3203x_device::rnd_reg,      &tms3203x_device::rnd_dir,      &tms3203x_device::rnd_ind,      &tms3203x_device::rnd_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::rol,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::rolc,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::ror,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::rorc,
	&tms3203x_device::rtps_reg,     &tms3203x_device::rtps_dir,     &tms3203x_device::rtps_ind,     &tms3203x_device::rtps_imm,
	&tms3203x_device::illegal,      &tms3203x_device::stf_dir,      &tms3203x_device::stf_ind,      &tms3203x_device::illegal,      // 0x28
	&tms3203x_device::illegal,      &tms3203x_device::stfi_dir,     &tms3203x_device::stfi_ind,     &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::sti_dir,      &tms3203x_device::sti_ind,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::stii_dir,     &tms3203x_device::stii_ind,     &tms3203x_device::illegal,
	&tms3203x_device::sigi,         &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::subb_reg,     &tms3203x_device::subb_dir,     &tms3203x_device::subb_ind,     &tms3203x_device::subb_imm,
	&tms3203x_device::subc_reg,     &tms3203x_device::subc_dir,     &tms3203x_device::subc_ind,     &tms3203x_device::subc_imm,
	&tms3203x_device::subf_reg,     &tms3203x_device::subf_dir,     &tms3203x_device::subf_ind,     &tms3203x_device::subf_imm,
	&tms3203x_device::subi_reg,     &tms3203x_device::subi_dir,     &tms3203x_device::subi_ind,     &tms3203x_device::subi_imm,     // 0x30
	&tms3203x_device::subrb_reg,    &tms3203x_device::subrb_dir,    &tms3203x_device::subrb_ind,    &tms3203x_device::subrb_imm,
	&tms3203x_device::subrf_reg,    &tms3203x_device::subrf_dir,    &tms3203x_device::subrf_ind,    &tms3203x_device::subrf_imm,
	&tms3203x_device::subri_reg,    &tms3203x_device::subri_dir,    &tms3203x_device::subri_ind,    &tms3203x_device::subri_imm,
	&tms3203x_device::tstb_reg,     &tms3203x_device::tstb_dir,     &tms3203x_device::tstb_ind,     &tms3203x_device::tstb_imm,
	&tms3203x_device::xor_reg,      &tms3203x_device::xor_dir,      &tms3203x_device::xor_ind,      &tms3203x_device::xor_imm,
	&tms3203x_device::illegal,      &tms3203x_device::iack_dir,     &tms3203x_device::iack_ind,     &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x38
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::addc3_regreg, &tms3203x_device::addc3_indreg, &tms3203x_device::addc3_regind, &tms3203x_device::addc3_indind, // 0x40
	&tms3203x_device::addf3_regreg, &tms3203x_device::addf3_indreg, &tms3203x_device::addf3_regind, &tms3203x_device::addf3_indind,
	&tms3203x_device::addi3_regreg, &tms3203x_device::addi3_indreg, &tms3203x_device::addi3_regind, &tms3203x_device::addi3_indind,
	&tms3203x_device::and3_regreg,  &tms3203x_device::and3_indreg,  &tms3203x_device::and3_regind,  &tms3203x_device::and3_indind,
	&tms3203x_device::andn3_regreg, &tms3203x_device::andn3_indreg, &tms3203x_device::andn3_regind, &tms3203x_device::andn3_indind,
	&tms3203x_device::ash3_regreg,  &tms3203x_device::ash3_indreg,  &tms3203x_device::ash3_regind,  &tms3203x_device::ash3_indind,
	&tms3203x_device::cmpf3_regreg, &tms3203x_device::cmpf3_indreg, &tms3203x_device::cmpf3_regind, &tms3203x_device::cmpf3_indind,
	&tms3203x_device::cmpi3_regreg, &tms3203x_device::cmpi3_indreg, &tms3203x_device::cmpi3_regind, &tms3203x_device::cmpi3_indind,
	&tms3203x_device::lsh3_regreg,  &tms3203x_device::lsh3_indreg,  &tms3203x_device::lsh3_regind,  &tms3203x_device::lsh3_indind,  // 0x48
	&tms3203x_device::mpyf3_regreg, &tms3203x_device::mpyf3_indreg, &tms3203x_device::mpyf3_regind, &tms3203x_device::mpyf3_indind,
	&tms3203x_device::mpyi3_regreg, &tms3203x_device::mpyi3_indreg, &tms3203x_device::mpyi3_regind, &tms3203x_device::mpyi3_indind,
	&tms3203x_device::or3_regreg,   &tms3203x_device::or3_indreg,   &tms3203x_device::or3_regind,   &tms3203x_device::or3_indind,
	&tms3203x_device::subb3_regreg, &tms3203x_device::subb3_indreg, &tms3203x_device::subb3_regind, &tms3203x_device::subb3_indind,
	&tms3203x_device::subf3_regreg, &tms3203x_device::subf3_indreg, &tms3203x_device::subf3_regind, &tms3203x_device::subf3_indind,
	&tms3203x_device::subi3_regreg, &tms3203x_device::subi3_indreg, &tms3203x_device::subi3_regind, &tms3203x_device::subi3_indind,
	&tms3203x_device::tstb3_regreg, &tms3203x_device::tstb3_indreg, &tms3203x_device::tstb3_regind, &tms3203x_device::tstb3_indind,
	&tms3203x_device::xor3_regreg,  &tms3203x_device::xor3_indreg,  &tms3203x_device::xor3_regind,  &tms3203x_device::xor3_indind,  // 0x50
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x58
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x60
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x68
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x70
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x78
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::ldfu_reg,     &tms3203x_device::ldfu_dir,     &tms3203x_device::ldfu_ind,     &tms3203x_device::ldfu_imm,     // 0x80
	&tms3203x_device::ldflo_reg,    &tms3203x_device::ldflo_dir,    &tms3203x_device::ldflo_ind,    &tms3203x_device::ldflo_imm,
	&tms3203x_device::ldfls_reg,    &tms3203x_device::ldfls_dir,    &tms3203x_device::ldfls_ind,    &tms3203x_device::ldfls_imm,
	&tms3203x_device::ldfhi_reg,    &tms3203x_device::ldfhi_dir,    &tms3203x_device::ldfhi_ind,    &tms3203x_device::ldfhi_imm,
	&tms3203x_device::ldfhs_reg,    &tms3203x_device::ldfhs_dir,    &tms3203x_device::ldfhs_ind,    &tms3203x_device::ldfhs_imm,
	&tms3203x_device::ldfeq_reg,    &tms3203x_device::ldfeq_dir,    &tms3203x_device::ldfeq_ind,    &tms3203x_device::ldfeq_imm,
	&tms3203x_device::ldfne_reg,    &tms3203x_device::ldfne_dir,    &tms3203x_device::ldfne_ind,    &tms3203x_device::ldfne_imm,
	&tms3203x_device::ldflt_reg,    &tms3203x_device::ldflt_dir,    &tms3203x_device::ldflt_ind,    &tms3203x_device::ldflt_imm,
	&tms3203x_device::ldfle_reg,    &tms3203x_device::ldfle_dir,    &tms3203x_device::ldfle_ind,    &tms3203x_device::ldfle_imm,    // 0x88
	&tms3203x_device::ldfgt_reg,    &tms3203x_device::ldfgt_dir,    &tms3203x_device::ldfgt_ind,    &tms3203x_device::ldfgt_imm,
	&tms3203x_device::ldfge_reg,    &tms3203x_device::ldfge_dir,    &tms3203x_device::ldfge_ind,    &tms3203x_device::ldfge_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::ldfnv_reg,    &tms3203x_device::ldfnv_dir,    &tms3203x_device::ldfnv_ind,    &tms3203x_device::ldfnv_imm,
	&tms3203x_device::ldfv_reg,     &tms3203x_device::ldfv_dir,     &tms3203x_device::ldfv_ind,     &tms3203x_device::ldfv_imm,
	&tms3203x_device::ldfnuf_reg,   &tms3203x_device::ldfnuf_dir,   &tms3203x_device::ldfnuf_ind,   &tms3203x_device::ldfnuf_imm,
	&tms3203x_device::ldfuf_reg,    &tms3203x_device::ldfuf_dir,    &tms3203x_device::ldfuf_ind,    &tms3203x_device::ldfuf_imm,
	&tms3203x_device::ldfnlv_reg,   &tms3203x_device::ldfnlv_dir,   &tms3203x_device::ldfnlv_ind,   &tms3203x_device::ldfnlv_imm,   // 0x90
	&tms3203x_device::ldflv_reg,    &tms3203x_device::ldflv_dir,    &tms3203x_device::ldflv_ind,    &tms3203x_device::ldflv_imm,
	&tms3203x_device::ldfnluf_reg,  &tms3203x_device::ldfnluf_dir,  &tms3203x_device::ldfnluf_ind,  &tms3203x_device::ldfnluf_imm,
	&tms3203x_device::ldfluf_reg,   &tms3203x_device::ldfluf_dir,   &tms3203x_device::ldfluf_ind,   &tms3203x_device::ldfluf_imm,
	&tms3203x_device::ldfzuf_reg,   &tms3203x_device::ldfzuf_dir,   &tms3203x_device::ldfzuf_ind,   &tms3203x_device::ldfzuf_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x98
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::ldiu_reg,     &tms3203x_device::ldiu_dir,     &tms3203x_device::ldiu_ind,     &tms3203x_device::ldiu_imm,     // 0xa0
	&tms3203x_device::ldilo_reg,    &tms3203x_device::ldilo_dir,    &tms3203x_device::ldilo_ind,    &tms3203x_device::ldilo_imm,
	&tms3203x_device::ldils_reg,    &tms3203x_device::ldils_dir,    &tms3203x_device::ldils_ind,    &tms3203x_device::ldils_imm,
	&tms3203x_device::ldihi_reg,    &tms3203x_device::ldihi_dir,    &tms3203x_device::ldihi_ind,    &tms3203x_device::ldihi_imm,
	&tms3203x_device::ldihs_reg,    &tms3203x_device::ldihs_dir,    &tms3203x_device::ldihs_ind,    &tms3203x_device::ldihs_imm,
	&tms3203x_device::ldieq_reg,    &tms3203x_device::ldieq_dir,    &tms3203x_device::ldieq_ind,    &tms3203x_device::ldieq_imm,
	&tms3203x_device::ldine_reg,    &tms3203x_device::ldine_dir,    &tms3203x_device::ldine_ind,    &tms3203x_device::ldine_imm,
	&tms3203x_device::ldilt_reg,    &tms3203x_device::ldilt_dir,    &tms3203x_device::ldilt_ind,    &tms3203x_device::ldilt_imm,
	&tms3203x_device::ldile_reg,    &tms3203x_device::ldile_dir,    &tms3203x_device::ldile_ind,    &tms3203x_device::ldile_imm,    // 0xa8
	&tms3203x_device::ldigt_reg,    &tms3203x_device::ldigt_dir,    &tms3203x_device::ldigt_ind,    &tms3203x_device::ldigt_imm,
	&tms3203x_device::ldige_reg,    &tms3203x_device::ldige_dir,    &tms3203x_device::ldige_ind,    &tms3203x_device::ldige_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::ldinv_reg,    &tms3203x_device::ldinv_dir,    &tms3203x_device::ldinv_ind,    &tms3203x_device::ldinv_imm,
	&tms3203x_device::ldiv_reg,     &tms3203x_device::ldiv_dir,     &tms3203x_device::ldiv_ind,     &tms3203x_device::ldiv_imm,
	&tms3203x_device::ldinuf_reg,   &tms3203x_device::ldinuf_dir,   &tms3203x_device::ldinuf_ind,   &tms3203x_device::ldinuf_imm,
	&tms3203x_device::ldiuf_reg,    &tms3203x_device::ldiuf_dir,    &tms3203x_device::ldiuf_ind,    &tms3203x_device::ldiuf_imm,
	&tms3203x_device::ldinlv_reg,   &tms3203x_device::ldinlv_dir,   &tms3203x_device::ldinlv_ind,   &tms3203x_device::ldinlv_imm,   // 0xb0
	&tms3203x_device::ldilv_reg,    &tms3203x_device::ldilv_dir,    &tms3203x_device::ldilv_ind,    &tms3203x_device::ldilv_imm,
	&tms3203x_device::ldinluf_reg,  &tms3203x_device::ldinluf_dir,  &tms3203x_device::ldinluf_ind,  &tms3203x_device::ldinluf_imm,
	&tms3203x_device::ldiluf_reg,   &tms3203x_device::ldiluf_dir,   &tms3203x_device::ldiluf_ind,   &tms3203x_device::ldiluf_imm,
	&tms3203x_device::ldizuf_reg,   &tms3203x_device::ldizuf_dir,   &tms3203x_device::ldizuf_ind,   &tms3203x_device::ldizuf_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xb8
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::br_imm,       &tms3203x_device::br_imm,       &tms3203x_device::br_imm,       &tms3203x_device::br_imm,       // 0xc0
	&tms3203x_device::br_imm,       &tms3203x_device::br_imm,       &tms3203x_device::br_imm,       &tms3203x_device::br_imm,
	&tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,
	&tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,      &tms3203x_device::brd_imm,
	&tms3203x_device::call_imm,     &tms3203x_device::call_imm,     &tms3203x_device::call_imm,     &tms3203x_device::call_imm,
	&tms3203x_device::call_imm,     &tms3203x_device::call_imm,     &tms3203x_device::call_imm,     &tms3203x_device::call_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,     // 0xc8
	&tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,     &tms3203x_device::rptb_imm,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::swi,          &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::brc_reg,      &tms3203x_device::brcd_reg,     &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xd0
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::brc_imm,      &tms3203x_device::brcd_imm,     &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,     &tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,     // 0xd8
	&tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,     &tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,
	&tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,     &tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,
	&tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,     &tms3203x_device::dbc_reg,      &tms3203x_device::dbcd_reg,
	&tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,     &tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,
	&tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,     &tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,
	&tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,     &tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,
	&tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,     &tms3203x_device::dbc_imm,      &tms3203x_device::dbcd_imm,
	&tms3203x_device::callc_reg,    &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xe0
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::callc_imm,    &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::trapc,        &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xe8
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::retic_reg,    &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xf0
	&tms3203x_device::retsc_reg,    &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0xf8
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,    // 0x100
	&tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,    &tms3203x_device::mpyaddf_0,
	&tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,
	&tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,    &tms3203x_device::mpyaddf_1,
	&tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,
	&tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,    &tms3203x_device::mpyaddf_2,
	&tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,
	&tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,    &tms3203x_device::mpyaddf_3,
	&tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,    // 0x108
	&tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,    &tms3203x_device::mpysubf_0,
	&tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,
	&tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,    &tms3203x_device::mpysubf_1,
	&tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,
	&tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,    &tms3203x_device::mpysubf_2,
	&tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,
	&tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,    &tms3203x_device::mpysubf_3,
	&tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,    // 0x110
	&tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,    &tms3203x_device::mpyaddi_0,
	&tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,
	&tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,    &tms3203x_device::mpyaddi_1,
	&tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,
	&tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,    &tms3203x_device::mpyaddi_2,
	&tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,
	&tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,    &tms3203x_device::mpyaddi_3,
	&tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,    // 0x118
	&tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,    &tms3203x_device::mpysubi_0,
	&tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,
	&tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,    &tms3203x_device::mpysubi_1,
	&tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,
	&tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,    &tms3203x_device::mpysubi_2,
	&tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,
	&tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,    &tms3203x_device::mpysubi_3,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x120
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x128
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x130
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x138
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x140
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x148
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x150
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x158
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x160
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x168
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x170
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x178
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,

	&tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,       // 0x180
	&tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,
	&tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,
	&tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,       &tms3203x_device::stfstf,
	&tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,
	&tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,
	&tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,
	&tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,       &tms3203x_device::stisti,
	&tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       // 0x188
	&tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,
	&tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,
	&tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,       &tms3203x_device::ldfldf,
	&tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,
	&tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,
	&tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,
	&tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,       &tms3203x_device::ldildi,
	&tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,      // 0x190
	&tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,
	&tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,
	&tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,      &tms3203x_device::absfstf,
	&tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,
	&tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,
	&tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,
	&tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,      &tms3203x_device::absisti,
	&tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     // 0x198
	&tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,
	&tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,
	&tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,     &tms3203x_device::addf3stf,
	&tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,
	&tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,
	&tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,
	&tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,     &tms3203x_device::addi3sti,
	&tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,      // 0x1a0
	&tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,
	&tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,
	&tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,      &tms3203x_device::and3sti,
	&tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,
	&tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,
	&tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,
	&tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,      &tms3203x_device::ash3sti,
	&tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,       // 0x1a8
	&tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,
	&tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,
	&tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,       &tms3203x_device::fixsti,
	&tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,
	&tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,
	&tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,
	&tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,     &tms3203x_device::floatstf,
	&tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       // 0x1b0
	&tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,
	&tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,
	&tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,       &tms3203x_device::ldfstf,
	&tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,
	&tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,
	&tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,
	&tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,       &tms3203x_device::ldisti,
	&tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      // 0x1b8
	&tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,
	&tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,
	&tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,      &tms3203x_device::lsh3sti,
	&tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,
	&tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,
	&tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,
	&tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,     &tms3203x_device::mpyf3stf,

	&tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     // 0x1c0
	&tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,
	&tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,
	&tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,     &tms3203x_device::mpyi3sti,
	&tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,
	&tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,
	&tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,
	&tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,      &tms3203x_device::negfstf,
	&tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,      // 0x1c8
	&tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,
	&tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,
	&tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,      &tms3203x_device::negisti,
	&tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,
	&tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,
	&tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,
	&tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,       &tms3203x_device::notsti,
	&tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,       // 0x1d0
	&tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,
	&tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,
	&tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,       &tms3203x_device::or3sti,
	&tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,
	&tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,
	&tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,
	&tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,     &tms3203x_device::subf3stf,
	&tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     // 0x1d8
	&tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,
	&tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,
	&tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,     &tms3203x_device::subi3sti,
	&tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,
	&tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,
	&tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,
	&tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,      &tms3203x_device::xor3sti,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x1e0
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x1e8
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x1f0
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      // 0x1f8
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,
	&tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal,      &tms3203x_device::illegal
};
