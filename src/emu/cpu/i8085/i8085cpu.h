// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, hap
/*******************************************************
 *
 *      Portable (hopefully ;-) 8085A emulator
 *
 *      Written by J. Buchmueller for use with MAME
 *
 *      Partially based on Z80Em by Marcel De Kogel
 *
 *      CPU related macros
 *
 *******************************************************/


#define SF              0x80
#define ZF              0x40
#define X5F             0x20
#define HF              0x10
#define X3F             0x08
#define PF              0x04
#define VF              0x02
#define CF              0x01

#define IM_SID          0x80
#define IM_I75          0x40
#define IM_I65          0x20
#define IM_I55          0x10
#define IM_IE           0x08
#define IM_M75          0x04
#define IM_M65          0x02
#define IM_M55          0x01

#define ADDR_TRAP       0x0024
#define ADDR_RST55      0x002c
#define ADDR_RST65      0x0034
#define ADDR_RST75      0x003c
#define ADDR_INTR       0x0038


#define M_MVI(R) R=ARG()

/* rotate */
#define M_RLC { \
	m_AF.b.h = (m_AF.b.h << 1) | (m_AF.b.h >> 7); \
	m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF); \
}

#define M_RRC { \
	m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF); \
	m_AF.b.h = (m_AF.b.h >> 1) | (m_AF.b.h << 7); \
}

#define M_RAL { \
	int c = m_AF.b.l&CF; \
	m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h >> 7); \
	m_AF.b.h = (m_AF.b.h << 1) | c; \
}

#define M_RAR { \
	int c = (m_AF.b.l&CF) << 7; \
	m_AF.b.l = (m_AF.b.l & 0xfe) | (m_AF.b.h & CF); \
	m_AF.b.h = (m_AF.b.h >> 1) | c; \
}

/* logical */
#define M_ORA(R) m_AF.b.h|=R; m_AF.b.l=ZSP[m_AF.b.h]
#define M_XRA(R) m_AF.b.h^=R; m_AF.b.l=ZSP[m_AF.b.h]
#define M_ANA(R) {UINT8 hc = ((m_AF.b.h | R)<<1) & HF; m_AF.b.h&=R; m_AF.b.l=ZSP[m_AF.b.h]; if(IS_8085()) { m_AF.b.l |= HF; } else {m_AF.b.l |= hc; } }

/* increase / decrease */
#define M_INR(R) {UINT8 hc = ((R & 0x0f) == 0x0f) ? HF : 0; ++R; m_AF.b.l= (m_AF.b.l & CF ) | ZSP[R] | hc; }
#define M_DCR(R) {UINT8 hc = ((R & 0x0f) != 0x00) ? HF : 0; --R; m_AF.b.l= (m_AF.b.l & CF ) | ZSP[R] | hc | VF; }

/* arithmetic */
#define M_ADD(R) { \
	int q = m_AF.b.h+R; \
	m_AF.b.l=ZSP[q&255]|((q>>8)&CF)|((m_AF.b.h^q^R)&HF); \
	m_AF.b.h=q; \
}

#define M_ADC(R) { \
	int q = m_AF.b.h+R+(m_AF.b.l&CF); \
	m_AF.b.l=ZSP[q&255]|((q>>8)&CF)|((m_AF.b.h^q^R)&HF); \
	m_AF.b.h=q; \
}

#define M_SUB(R) { \
	int q = m_AF.b.h-R; \
	m_AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
	m_AF.b.h=q; \
}

#define M_SBB(R) { \
	int q = m_AF.b.h-R-(m_AF.b.l&CF); \
	m_AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
	m_AF.b.h=q; \
}

#define M_CMP(R) { \
	int q = m_AF.b.h-R; \
	m_AF.b.l=ZSP[q&255]|((q>>8)&CF)|(~(m_AF.b.h^q^R)&HF)|VF; \
}

#define M_DAD(R) { \
	int q = m_HL.d + m_##R.d; \
	m_AF.b.l = (m_AF.b.l & ~CF) | (q>>16 & CF ); \
	m_HL.w.l = q; \
}

// DSUB is 8085-only, not sure if H flag handling is correct
#define M_DSUB() { \
	int q = m_HL.b.l-m_BC.b.l; \
	m_AF.b.l=ZS[q&255]|((q>>8)&CF)|VF| \
		((m_HL.b.l^q^m_BC.b.l)&HF)| \
		(((m_BC.b.l^m_HL.b.l)&(m_HL.b.l^q)&SF)>>5); \
	m_HL.b.l=q; \
	q = m_HL.b.h-m_BC.b.h-(m_AF.b.l&CF); \
	m_AF.b.l=ZS[q&255]|((q>>8)&CF)|VF| \
		((m_HL.b.h^q^m_BC.b.h)&HF)| \
		(((m_BC.b.h^m_HL.b.h)&(m_HL.b.h^q)&SF)>>5); \
	if (m_HL.b.l!=0) m_AF.b.l&=~ZF; \
}

/* i/o */
#define M_IN \
	m_STATUS = 0x42; \
	m_WZ.d=ARG(); \
	m_AF.b.h=m_io->read_byte(m_WZ.d);

#define M_OUT \
	m_STATUS = 0x10; \
	m_WZ.d=ARG(); \
	m_io->write_byte(m_WZ.d,m_AF.b.h)

/* stack */
#define M_PUSH(R) { \
	m_STATUS = 0x04; \
	m_program->write_byte(--m_SP.w.l, m_##R.b.h); \
	m_program->write_byte(--m_SP.w.l, m_##R.b.l); \
}

#define M_POP(R) { \
	m_STATUS = 0x86; \
	m_##R.b.l = m_program->read_byte(m_SP.w.l++); \
	m_##R.b.h = m_program->read_byte(m_SP.w.l++); \
}

/* jumps */
// On 8085 jump if condition is not satisfied is shorter
#define M_JMP(cc) { \
	if (cc) { \
		m_PC.w.l = ARG16(); \
	} else { \
		m_PC.w.l += 2; \
		m_icount += (IS_8085()) ? 3 : 0; \
	} \
}

// On 8085 call if condition is not satisfied is 9 ticks
#define M_CALL(cc) \
{ \
	if (cc) \
	{ \
		UINT16 a = ARG16(); \
		m_icount -= (IS_8085()) ? 7 : 6 ; \
		M_PUSH(PC); \
		m_PC.d = a; \
	} else { \
		m_PC.w.l += 2; \
		m_icount += (IS_8085()) ? 2 : 0; \
	} \
}

// conditional RET only
#define M_RET(cc) \
{ \
	if (cc) \
	{ \
		m_icount -= 6; \
		M_POP(PC); \
	} \
}

#define M_RST(nn) { \
	M_PUSH(PC); \
	m_PC.d = 8 * nn; \
}
