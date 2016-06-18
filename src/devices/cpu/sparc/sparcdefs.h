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

#define GET_OPCODE				0; { m_asi = m_insn_asi; op = m_spaces[m_insn_asi]->read_dword(m_pc); }

#define LOAD_SBA(asi,addr)		(((INT32)m_spaces[asi]->read_byte(addr) << 24) >> 24)
#define LOAD_UBA(asi,addr)		m_spaces[asi]->read_byte(addr)
#define LOAD_SHA(asi,addr)		(((INT32)m_spaces[asi]->read_word(addr) << 16) >> 16)
#define LOAD_UHA(asi,addr)		m_spaces[asi]->read_word(addr)
#define LOAD_WA(asi,addr)		m_spaces[asi]->read_dword(addr)
#define LOAD_DA(asi,addr)		(((UINT64)m_spaces[asi]->read_dword(addr) << 32) | m_spaces[asi]->read_dword(addr+4))
#define STORE_BA(asi,addr,data)	{ m_spaces[asi]->write_byte(addr,data); }
#define STORE_HA(asi,addr,data)	{ m_spaces[asi]->write_word(addr,data); }
#define STORE_WA(asi,addr,data)	{ m_spaces[asi]->write_dword(addr,data); }
#define STORE_DA(asi,addr,data)	{ m_spaces[asi]->write_dword(addr,(UINT32)(data >> 32)); m_spaces[asi]->write_dword(addr+4,(UINT32)data); }

#define LOAD_SB(addr)			{ m_asi = m_data_asi; LOAD_SBA(m_data_asi,addr); }
#define LOAD_UB(addr)			{ m_asi = m_data_asi; LOAD_UBA(m_data_asi,addr); }
#define LOAD_SH(addr)			{ m_asi = m_data_asi; LOAD_SHA(m_data_asi,addr); }
#define LOAD_UH(addr)			{ m_asi = m_data_asi; LOAD_UHA(m_data_asi,addr); }
#define LOAD_W(addr)			{ m_asi = m_data_asi; LOAD_WA(m_data_asi,addr); }
#define LOAD_D(addr)			{ m_asi = m_data_asi; LOAD_DA(m_data_asi,addr); }
#define STORE_B(addr,data)		{ m_asi = m_data_asi; STORE_BA(m_data_asi,addr); }
#define STORE_H(addr,data)		{ m_asi = m_data_asi; STORE_HA(m_data_asi,addr); }
#define STORE_W(addr,data)		{ m_asi = m_data_asi; STORE_WA(m_data_asi,addr); }
#define STORE_D(addr,data)		{ m_asi = m_data_asi; STORE_DA(m_data_asi,addr); }

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
#define ICC_N_CLEAR			(!ICC_N_SET)
#define SET_ICC_N_FLAG		do { m_psr |= PSR_N_MASK; } while(0);
#define CLEAR_ICC_N_FLAG	do { m_psr &= ~PSR_N_MASK; } while(0);

#define ICC_Z_SET			(m_psr & PSR_Z_MASK)
#define ICC_Z_CLEAR			(!ICC_Z_SET)
#define SET_ICC_Z_FLAG		do { m_psr |= PSR_Z_MASK; } while(0);
#define CLEAR_ICC_Z_FLAG	do { m_psr &= ~PSR_Z_MASK; } while(0);

#define ICC_V_SET			(m_psr & PSR_V_MASK)
#define ICC_V_CLEAR			(!ICC_B_SET)
#define SET_ICC_V_FLAG		do { m_psr |= PSR_V_MASK; } while(0);
#define CLEAR_ICC_V_FLAG	do { m_psr &= ~PSR_V_MASK; } while(0);

#define ICC_C_SET			(m_psr & PSR_C_MASK)
#define ICC_C_CLEAR			(!ICC_C_SET)
#define SET_ICC_C_FLAG		do { m_psr |= PSR_C_MASK; } while(0);
#define CLEAR_ICC_C_FLAG	do { m_psr &= ~PSR_C_MASK; } while(0);

#define CLEAR_ICC			do { m_psr &= ~PSR_ICC_MASK; } while(0);

#define TEST_ICC_NZ(x)		do { m_psr &= ~PSR_ICC_MASK; m_psr |= (x & 0x80000000) ? PSR_N_MASK : 0; m_psr |= (x == 0) ? PSR_Z_MASK : 0; } while (0);

#define MAKE_PSR			do { m_psr = (m_impl << PSR_IMPL_SHIFT) | (m_ver << PSR_VER_SHIFT) | (m_icc << PSR_ICC_SHIFT) | (m_ec ? PSR_EC_MASK : 0) | (m_ef ? PSR_EF_MASK : 0) | (m_pil << PSR_PIL_SHIFT) | (m_s ? PSR_S_MASK : 0) | (m_ps ? PSR_PS_MASK : 0) | (m_et ? PSR_ET_MASK : 0) | m_cwp; } while(0);
#define BREAK_PSR			do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; m_ec = m_psr & PSR_EC_MASK; m_ef = m_psr & PSR_EF_MASK; m_pil = (m_psr & PSR_PIL_MASK) >> PSR_PIL_SHIFT; m_s = m_psr & PSR_S_MASK; m_ps = m_psr & PSR_PS_MASK; m_et = m_psr & PSR_ET_MASK; m_cwp = m_psr & PSR_CWP_MASK; } while(0);
#define MAKE_ICC			do { m_icc = (m_psr & PSR_ICC_MASK) >> PSR_ICC_SHIFT; } while(0);

#define IS_SUPERVISOR		(m_psr & PSR_S_MASK)
#define IS_USER				(!IS_SUPERVISOR)

#define OP		(op >> 30) // gangnam style
#define OP2		((op >> 22) & 7)
#define OP3		((op >> 19) & 63)
#define OPF		((op >> 5) & 0x1ff)

#define DISP30	(op << 2)
#define DISP22	(((INT32)(op << 10)) >> 8)
#define IMM22	(op << 10)
#define CONST22 (op & 0x3fffff)
#define SIMM13	(((INT32)(op << 19)) >> 19)
#define IMM7	(op & 0x7f)

#define OPIMM	(op & 0x00002000)
#define USEIMM	((op >> 13) & 1)

#define COND	((op >> 25) & 15)
#define ANNUL	((op >> 29) & 1)
#define ASI		((op >> 5) & 0xff)

#define RD		((op >> 25) & 31)
#define RS1		((op >> 14) & 31)
#define RS2		(op & 31)

#define REG(x)	*m_regs[x]
#define RDREG	*m_regs[RD]
#define RS1REG	*m_regs[RS1]
#define RS2REG	*m_regs[RS2]
#define SET_RDREG(x)	if(RD) { RDREG = x; }
#define ADDRESS	(OPIMM ? (RS1REG + SIMM13) : (RS1REG + RS2REG))

#define PC		m_pc
#define nPC		m_npc

#endif // __MB86901_DEFS_H__