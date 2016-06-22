// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//================================================================
//
//  mb86901defs.h - Helpful #defines for emulating the MB86901
//                  series of SPARC processor.
//
//================================================================

#pragma once

#ifndef __MB86901_DEFS_H__
#define __MB86901_DEFS_H__

#define PSR_CWP_MASK		0x0000001f
#define PSR_ET_SHIFT		5
#define PSR_ET_MASK			0x00000020
#define PSR_PS_SHIFT		6
#define PSR_PS_MASK			0x00000040
#define PSR_S_SHIFT			7
#define PSR_S_MASK			0x00000080
#define PSR_PIL_SHIFT		8
#define PSR_PIL_MASK		0x00000f00
#define PSR_EF_SHIFT		12
#define PSR_EF_MASK			0x00001000
#define PSR_EC_SHIFT		13
#define PSR_EC_MASK			0x00002000
#define PSR_ICC_SHIFT		20
#define PSR_RES_MASK		0x000fc000
#define PSR_ICC_MASK		0x00f00000
#define PSR_N_MASK			0x00800000
#define PSR_Z_MASK			0x00400000
#define PSR_V_MASK			0x00200000
#define PSR_C_MASK			0x00100000
#define PSR_VER_SHIFT		24
#define PSR_VER_MASK		0x0f000000
#define PSR_VER				0
#define PSR_IMPL_SHIFT		28
#define PSR_IMPL_MASK		0xf0000000
#define PSR_IMPL			0
#define PSR_ZERO_MASK		(PSR_IMPL_MASK | PSR_VER_MASK | PSR_RES_MASK)

#define ICC_N_SET			(m_psr & PSR_N_MASK)
#define ICC_N				(ICC_N_SET ? 1 : 0)
#define ICC_N_CLEAR			(!ICC_N_SET)
#define SET_ICC_N_FLAG		do { m_psr |= PSR_N_MASK; } while(0);
#define CLEAR_ICC_N_FLAG	do { m_psr &= ~PSR_N_MASK; } while(0);

#define ICC_Z_SET			(m_psr & PSR_Z_MASK)
#define ICC_Z				(ICC_Z_SET ? 1 : 0)
#define ICC_Z_CLEAR			(!ICC_Z_SET)
#define SET_ICC_Z_FLAG		do { m_psr |= PSR_Z_MASK; } while(0);
#define CLEAR_ICC_Z_FLAG	do { m_psr &= ~PSR_Z_MASK; } while(0);

#define ICC_V_SET			(m_psr & PSR_V_MASK)
#define ICC_V				(ICC_V_SET ? 1 : 0)
#define ICC_V_CLEAR			(!ICC_V_SET)
#define SET_ICC_V_FLAG		do { m_psr |= PSR_V_MASK; } while(0);
#define CLEAR_ICC_V_FLAG	do { m_psr &= ~PSR_V_MASK; } while(0);

#define ICC_C_SET			(m_psr & PSR_C_MASK)
#define ICC_C				(ICC_C_SET ? 1 : 0)
#define ICC_C_CLEAR			(!ICC_C_SET)
#define SET_ICC_C_FLAG		do { m_psr |= PSR_C_MASK; } while(0);
#define CLEAR_ICC_C_FLAG	do { m_psr &= ~PSR_C_MASK; } while(0);

#define CLEAR_ICC			do { m_psr &= ~PSR_ICC_MASK; } while(0);

#define TEST_ICC_NZ(x)		do { m_psr &= ~PSR_ICC_MASK; m_psr |= (x & 0x80000000) ? PSR_N_MASK : 0; m_psr |= (x == 0) ? PSR_Z_MASK : 0; } while (0);

#define MAKE_PSR			do { m_psr = (m_impl << PSR_IMPL_SHIFT) | (m_ver << PSR_VER_SHIFT) | (m_icc << PSR_ICC_SHIFT) | (m_ec ? PSR_EC_MASK : 0) | (m_ef ? PSR_EF_MASK : 0) | (m_pil << PSR_PIL_SHIFT) | (m_s ? PSR_S_MASK : 0) | (m_ps ? PSR_PS_MASK : 0) | (m_et ? PSR_ET_MASK : 0) | m_cwp; } while(0);
#define BREAK_PSR			do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; m_ec = m_psr & PSR_EC_MASK; m_ef = m_psr & PSR_EF_MASK; m_pil = (m_psr & PSR_PIL_MASK) >> PSR_PIL_SHIFT; m_s = m_psr & PSR_S_MASK; m_ps = m_psr & PSR_PS_MASK; m_et = m_psr & PSR_ET_MASK; m_cwp = m_psr & PSR_CWP_MASK; } while(0);
#define MAKE_ICC			do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; } while(0);

#define CWP					m_cwp
#define S					m_s
#define PS					m_ps

#define IS_SUPERVISOR		(m_psr & PSR_S_MASK)
#define IS_USER				(!IS_SUPERVISOR)

#define TRAPS_ENABLED		(m_psr & PSR_ET_MASK)
#define TRAPS_DISABLED		(!TRAPS_ENABLED)

#define PSR					m_psr
#define WIM					m_wim
#define TBR					m_tbr

#define OP		(op >> 30) // gangnam style
#define OP2		((op >> 22) & 7)
#define OP3		((op >> 19) & 63)
#define OPF		((op >> 5) & 0x1ff)
#define OPC		((op >> 5) & 0x1ff)
#define OPFLOW	((op >> 5) & 0x3f)

#define DISP30	(INT32(op << 2))
#define DISP22	(INT32(op << 10) >> 8)
#define DISP19	(INT32(op << 13) >> 11)
#define DISP16	(INT32(((op << 10) & 0xc0000000) | ((op << 16) & 0x3fff0000)) >> 14)
#define IMM22	(op << 10)
#define CONST22 (op & 0x3fffff)
#define SIMM13	(INT32(op << 19) >> 19)
#define SIMM11	(INT32(op << 21) >> 21)
#define SIMM10	(INT32(op << 22) >> 22)
#define IMM7	(op & 0x7f)
#define SIMM7	(INT32(op << 25) >> 25)
#define SIMM8	(INT32(op << 24) >> 24)
#define SHCNT32	(op & 31)
#define SHCNT64 (op & 63)
#define USEIMM	((op >> 13) & 1)
#define USEEXT	((op >> 12) & 1)


#define COND	((op >> 25) & 15)
#define RCOND	((op >> 10) & 7)
#define MOVCOND	((op >> 14) & 15)
#define PRED	((op >> 19) & 1)
#define ANNUL	((op >> 29) & 1)
#define BRCC	((op >> 20) & 3)
#define MOVCC	(((op >> 11) & 3) | ((op >> 16) & 4))
#define OPFCC	((op >> 11) & 7)
#define TCCCC	((op >> 11) & 3)
#define ASI		((op >> 5) & 255)
#define MMASK	(op & 15)
#define CMASK	((op >> 4) & 7)

#define RD		((op >> 25) & 31)
#define RS1		((op >> 14) & 31)
#define RS2		(op & 31)

#define FREG(x)	m_fpr[(x)]
#define FDREG	m_fpr[RD]
#define FSR		m_fsr

#define REG(x)	*m_regs[(x)]
#define RDREG	*m_regs[RD]
#define RS1REG	*m_regs[RS1]
#define RS2REG	*m_regs[RS2]
#define SET_RDREG(x)	if(RD) { RDREG = (x); }
#define ADDRESS	(USEIMM ? (RS1REG + SIMM13) : (RS1REG + RS2REG))

#define PC		m_pc
#define nPC		m_npc

#define Y		m_y

#define ET		m_et
#define EF		m_ef
#define EC		m_ec
#define PIL		m_pil

#define MAE			m_mae
#define HOLD_BUS	m_hold_bus

#define BIT31(x)	((x) & 0x80000000)

#define UPDATE_PC	true
#define PC_UPDATED	false

#define OP_TYPE0	0
#define OP_CALL		1
#define OP_ALU		2
#define OP_LDST		3

#define OP2_UNIMP	0
#define OP2_BICC	2
#define OP2_SETHI	4
#define OP2_FBFCC	6
#define OP2_CBCCC	7

#define OP3_ADD		0
#define OP3_AND		1
#define OP3_OR		2
#define OP3_XOR		3
#define OP3_SUB		4
#define OP3_ANDN	5
#define OP3_ORN		6
#define OP3_XNOR	7
#define OP3_ADDX	8
#define OP3_UMUL	10
#define OP3_SMUL	11
#define OP3_SUBX	12
#define OP3_UDIV	14
#define OP3_SDIV	15
#define OP3_ADDCC	16
#define OP3_ANDCC	17
#define OP3_ORCC	18
#define OP3_XORCC	19
#define OP3_SUBCC	20
#define OP3_ANDNCC	21
#define OP3_ORNCC	22
#define OP3_XNORCC	23
#define OP3_ADDXCC	24
#define OP3_UMULCC	26
#define OP3_SMULCC	27
#define OP3_SUBXCC	28
#define OP3_UDIVCC	30
#define OP3_SDIVCC	31
#define OP3_TADDCC	32
#define OP3_TSUBCC	33
#define OP3_TADDCCTV	34
#define OP3_TSUBCCTV	35
#define OP3_MULSCC	36
#define OP3_SLL		37
#define OP3_SRL		38
#define OP3_SRA		39
#define OP3_RDASR	40
#define OP3_RDPSR	41
#define OP3_RDWIM	42
#define OP3_RDTBR	43
#define OP3_WRASR	48
#define OP3_WRPSR	49
#define OP3_WRWIM	50
#define OP3_WRTBR	51
#define OP3_FPOP1	52
#define OP3_FPOP2	53
#define OP3_JMPL	56
#define OP3_RETT	57
#define OP3_TICC	58
#define OP3_SAVE	60
#define OP3_RESTORE	61

#define OP3_LD		0
#define OP3_LDUB	1
#define OP3_LDUH	2
#define OP3_LDD		3
#define OP3_ST		4
#define OP3_STB		5
#define OP3_STH		6
#define OP3_STD		7
#define OP3_LDSB	9
#define OP3_LDSH	10
#define OP3_LDSTUB	13
#define OP3_SWAP	15
#define OP3_LDA		16
#define OP3_LDUBA	17
#define OP3_LDUHA	18
#define OP3_LDDA	19
#define OP3_STA		20
#define OP3_STBA	21
#define OP3_STHA	22
#define OP3_STDA	23
#define OP3_LDSBA	25
#define OP3_LDSHA	26
#define OP3_LDSTUBA	29
#define OP3_SWAPA	31
#define OP3_LDFPR	32
#define OP3_LDFSR	33
#define OP3_LDDFPR	35
#define OP3_STFPR	36
#define OP3_STFSR	37
#define OP3_STDFQ	38
#define OP3_STDFPR	39
#define OP3_LDCPR	40
#define OP3_LDCSR	41
#define OP3_LDDCPR	43
#define OP3_STCPR	44
#define OP3_STCSR	45
#define OP3_STDCQ	46
#define OP3_STDCPR	47
#define OP3_CPOP1	54
#define OP3_CPOP2	55

#define COND_BN		0
#define COND_BE		1
#define COND_BLE	2
#define COND_BL		3
#define COND_BLEU	4
#define COND_BCS	5
#define COND_BNEG	6
#define COND_BVS	7
#define COND_BA		8
#define COND_BNE	9
#define COND_BG		10
#define COND_BGE	11
#define COND_BGU	12
#define COND_BCC	13
#define COND_BPOS	14
#define COND_BVC	15

#define LDD		(OP3 == OP3_LDD)
#define LD		(OP3 == OP3_LD)
#define LDSH	(OP3 == OP3_LDSH)
#define LDUH	(OP3 == OP3_LDUH)
#define LDSB	(OP3 == OP3_LDSB)
#define LDUB	(OP3 == OP3_LDUB)
#define LDDF	(OP3 == OP3_LDDFPR)
#define LDF		(OP3 == OP3_LDFPR)
#define LDFSR	(OP3 == OP3_LDFSR)
#define LDDC	(OP3 == OP3_LDDCPR)
#define LDC		(OP3 == OP3_LDCPR)
#define LDCSR	(OP3 == OP3_LDCSR)
#define LDDA	(OP3 == OP3_LDDA)
#define LDA		(OP3 == OP3_LDA)
#define LDSHA	(OP3 == OP3_LDSHA)
#define LDUHA	(OP3 == OP3_LDUHA)
#define LDSBA	(OP3 == OP3_LDSBA)
#define LDUBA	(OP3 == OP3_LDUBA)

#define STD		(OP3 == OP3_STD)
#define ST		(OP3 == OP3_ST)
#define STH		(OP3 == OP3_STH)
#define STB		(OP3 == OP3_STB)
#define STDA	(OP3 == OP3_STDA)
#define STA		(OP3 == OP3_STA)
#define STHA	(OP3 == OP3_STHA)
#define STBA	(OP3 == OP3_STBA)
#define STF		(OP3 == OP3_STFPR)
#define STFSR	(OP3 == OP3_STFSR)
#define STDFQ	(OP3 == OP3_STDFQ)
#define STDF	(OP3 == OP3_STDFPR)
#define STC		(OP3 == OP3_STCPR)
#define STCSR	(OP3 == OP3_STCSR)
#define STDCQ	(OP3 == OP3_STDCQ)
#define STDC	(OP3 == OP3_STDCPR)

#define JMPL	(OP3 == OP3_JMPL)
#define TICC	(OP3 == OP3_TICC)
#define RETT	(OP3 == OP3_RETT)

#define LDSTUB	(OP3 == OP3_LDSTUB)
#define LDSTUBA	(OP3 == OP3_LDSTUBA)

#endif // __MB86901_DEFS_H__