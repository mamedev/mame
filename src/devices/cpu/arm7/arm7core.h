// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7core.h
 *   Portable ARM7TDMI Core Emulator
 *
 *   Copyright Steve Ellenoff
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************

 This file contains everything related to the arm7 core itself, and is presumed
 to be cpu implementation non-specific, ie, applies to only the core.

 ******************************************************************************/

#ifndef MAME_CPU_ARM7_ARM7CORE_H
#define MAME_CPU_ARM7_ARM7CORE_H

#pragma once

#include "arm7.h"

#define ARM7_MMU_ENABLE_HACK 0
#define ARM7_DEBUG_CORE 0


/****************************************************************************************************
 *  ARM7 CORE REGISTERS
 ***************************************************************************************************/

/* There are 36 Unique - 32 bit processor registers */
/* Each mode has 17 registers (except user & system, which have 16) */
/* This is a list of each *unique* register */
enum
{
	/* All modes have the following */
	eR0 = 0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
	eR8, eR9, eR10, eR11, eR12,
	eR13, /* Stack Pointer */
	eR14, /* Link Register (holds return address) */
	eR15, /* Program Counter */
	eCPSR, /* Current Status Program Register */

	/* Fast Interrupt - Bank switched registers */
	eR8_FIQ, eR9_FIQ, eR10_FIQ, eR11_FIQ, eR12_FIQ, eR13_FIQ, eR14_FIQ, eSPSR_FIQ,

	/* IRQ - Bank switched registers */
	eR13_IRQ, eR14_IRQ, eSPSR_IRQ,

	/* Supervisor/Service Mode - Bank switched registers */
	eR13_SVC, eR14_SVC, eSPSR_SVC,

	/* Abort Mode - Bank switched registers */
	eR13_ABT, eR14_ABT, eSPSR_ABT,

	/* Undefined Mode - Bank switched registers */
	eR13_UND, eR14_UND, eSPSR_UND,

	NUM_REGS
};

/* Coprocessor-related macros */
#define COPRO_DOMAIN_NO_ACCESS              0
#define COPRO_DOMAIN_CLIENT                 1
#define COPRO_DOMAIN_RESV                   2
#define COPRO_DOMAIN_MANAGER                3

#define COPRO_FAULT_NONE                    0
#define COPRO_FAULT_TRANSLATE_SECTION       5
#define COPRO_FAULT_TRANSLATE_PAGE          7
#define COPRO_FAULT_DOMAIN_SECTION          9
#define COPRO_FAULT_DOMAIN_PAGE             11
#define COPRO_FAULT_PERM_SECTION            13
#define COPRO_FAULT_PERM_PAGE               15

#define COPRO_TLB_BASE                      m_tlbBase
#define COPRO_TLB_BASE_MASK                 0xffffc000
#define COPRO_TLB_VADDR_FLTI_MASK           0xfff00000
#define COPRO_TLB_VADDR_FLTI_MASK_SHIFT     18
#define COPRO_TLB_VADDR_CSLTI_MASK          0x000ff000
#define COPRO_TLB_VADDR_CSLTI_MASK_SHIFT    10
#define COPRO_TLB_VADDR_FSLTI_MASK          0x000ffc00
#define COPRO_TLB_VADDR_FSLTI_MASK_SHIFT    8
#define COPRO_TLB_CFLD_ADDR_MASK            0xfffffc00
#define COPRO_TLB_CFLD_ADDR_MASK_SHIFT      10
#define COPRO_TLB_FPTB_ADDR_MASK            0xfffff000
#define COPRO_TLB_FPTB_ADDR_MASK_SHIFT      12
#define COPRO_TLB_SECTION_PAGE_MASK         0xfff00000
#define COPRO_TLB_LARGE_PAGE_MASK           0xffff0000
#define COPRO_TLB_SMALL_PAGE_MASK           0xfffff000
#define COPRO_TLB_TINY_PAGE_MASK            0xfffffc00
#define COPRO_TLB_STABLE_MASK               0xfff00000
#define COPRO_TLB_LSTABLE_MASK              0xfffff000
#define COPRO_TLB_TTABLE_MASK               0xfffffc00
#define COPRO_TLB_UNMAPPED                  0
#define COPRO_TLB_LARGE_PAGE                1
#define COPRO_TLB_SMALL_PAGE                2
#define COPRO_TLB_TINY_PAGE                 3
#define COPRO_TLB_COARSE_TABLE              1
#define COPRO_TLB_SECTION_TABLE             2
#define COPRO_TLB_FINE_TABLE                3
#define COPRO_TLB_TYPE_SECTION              0
#define COPRO_TLB_TYPE_LARGE                1
#define COPRO_TLB_TYPE_SMALL                2
#define COPRO_TLB_TYPE_TINY                 3

#define COPRO_CTRL                          m_control
#define COPRO_CTRL_MMU_EN                   0x00000001
#define COPRO_CTRL_ADDRFAULT_EN             0x00000002
#define COPRO_CTRL_DCACHE_EN                0x00000004
#define COPRO_CTRL_WRITEBUF_EN              0x00000008
#define COPRO_CTRL_ENDIAN                   0x00000080
#define COPRO_CTRL_SYSTEM                   0x00000100
#define COPRO_CTRL_ROM                      0x00000200
#define COPRO_CTRL_ICACHE_EN                0x00001000
#define COPRO_CTRL_INTVEC_ADJUST            0x00002000
#define COPRO_CTRL_ADDRFAULT_EN_SHIFT       1
#define COPRO_CTRL_DCACHE_EN_SHIFT          2
#define COPRO_CTRL_WRITEBUF_EN_SHIFT        3
#define COPRO_CTRL_ENDIAN_SHIFT             7
#define COPRO_CTRL_SYSTEM_SHIFT             8
#define COPRO_CTRL_ROM_SHIFT                9
#define COPRO_CTRL_ICACHE_EN_SHIFT          12
#define COPRO_CTRL_INTVEC_ADJUST_SHIFT      13
#define COPRO_CTRL_LITTLE_ENDIAN            0
#define COPRO_CTRL_BIG_ENDIAN               1
#define COPRO_CTRL_INTVEC_0                 0
#define COPRO_CTRL_INTVEC_F                 1
#define COPRO_CTRL_MASK                     0x0000338f

#define COPRO_DOMAIN_ACCESS_CONTROL         m_domainAccessControl

#define COPRO_FAULT_STATUS_D                m_faultStatus[0]
#define COPRO_FAULT_STATUS_P                m_faultStatus[1]

#define COPRO_FAULT_ADDRESS                 m_faultAddress

#define COPRO_FCSE_PID                      m_fcsePID

//#define ARM7_USE_DRC

/* forward declaration of implementation-specific state */
#ifndef ARM7_USE_DRC
struct arm7imp_state {};
#else
struct arm7imp_state;
#endif

/* CPU state struct */
struct arm_state
{
	uint32_t m_r[NUM_REGS];
	bool m_pendingIrq;
	bool m_pendingFiq;
	bool m_pendingAbtD;
	bool m_pendingAbtP;
	bool m_pendingUnd;
	bool m_pendingSwi;
	bool m_pending_interrupt;
	int m_icount;
	endianness_t m_endian;
	address_space *m_program;
	std::function<u32 (offs_t)> m_pr32;

	/* Coprocessor Registers */
	uint32_t m_control;
	uint32_t m_tlbBase;
	uint32_t m_tlb_base_mask;
	uint32_t m_faultStatus[2];
	uint32_t m_faultAddress;
	uint32_t m_fcsePID;
	uint32_t m_domainAccessControl;
	uint32_t m_decoded_access_control[16];

	uint8_t m_archRev;          // ARM architecture revision (3, 4, and 5 are valid)
	uint8_t m_archFlags;        // architecture flags

#if ARM7_MMU_ENABLE_HACK
	uint32_t mmu_enable_addr; // workaround for "MMU is enabled when PA != VA" problem
#endif
	arm7imp_state m_impstate;
};

/****************************************************************************************************
 *  VARIOUS INTERNAL STRUCS/DEFINES/ETC..
 ***************************************************************************************************/

// Mode values come from bit 4-0 of CPSR, but we are ignoring bit 4 here, since bit 4 always = 1 for valid modes
enum
{
	eARM7_MODE_USER = 0x0,      // Bit: 4-0 = 10000
	eARM7_MODE_FIQ  = 0x1,      // Bit: 4-0 = 10001
	eARM7_MODE_IRQ  = 0x2,      // Bit: 4-0 = 10010
	eARM7_MODE_SVC  = 0x3,      // Bit: 4-0 = 10011
	eARM7_MODE_ABT  = 0x7,      // Bit: 4-0 = 10111
	eARM7_MODE_UND  = 0xb,      // Bit: 4-0 = 11011
	eARM7_MODE_SYS  = 0xf       // Bit: 4-0 = 11111
};

#define ARM7_NUM_MODES 0x10

static const int thumbCycles[256] =
{
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 1
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 2
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 3
	1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 4
	2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // 5
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 6
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 7
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 8
	2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,  // 9
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // a
	1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 4, 1, 1,  // b
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // c
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3,  // d
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  // e
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   // f
};

/* 17 processor registers are visible at any given time,
 * banked depending on processor mode.
 */

static const int sRegisterTable[ARM7_NUM_MODES][18] =
{
	{ /* USR */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13, eR14,
		eR15, eCPSR  // No SPSR in this mode
	},
	{ /* FIQ */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8_FIQ, eR9_FIQ, eR10_FIQ, eR11_FIQ, eR12_FIQ,
		eR13_FIQ, eR14_FIQ,
		eR15, eCPSR, eSPSR_FIQ
	},
	{ /* IRQ */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13_IRQ, eR14_IRQ,
		eR15, eCPSR, eSPSR_IRQ
	},
	{ /* SVC */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13_SVC, eR14_SVC,
		eR15, eCPSR, eSPSR_SVC
	},
	{0}, {0}, {0},        // values for modes 4,5,6 are not valid
	{ /* ABT */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13_ABT, eR14_ABT,
		eR15, eCPSR, eSPSR_ABT
	},
	{0}, {0}, {0},        // values for modes 8,9,a are not valid!
	{ /* UND */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13_UND, eR14_UND,
		eR15, eCPSR, eSPSR_UND
	},
	{0}, {0}, {0},        // values for modes c,d, e are not valid!
	{ /* SYS */
		eR0, eR1, eR2, eR3, eR4, eR5, eR6, eR7,
		eR8, eR9, eR10, eR11, eR12,
		eR13, eR14,
		eR15, eCPSR  // No SPSR in this mode
	}
};

#define N_BIT   31
#define Z_BIT   30
#define C_BIT   29
#define V_BIT   28
#define Q_BIT   27
#define I_BIT   7
#define F_BIT   6
#define T_BIT   5   // Thumb mode

#define N_MASK  ((uint32_t)(1 << N_BIT)) /* Negative flag */
#define Z_MASK  ((uint32_t)(1 << Z_BIT)) /* Zero flag */
#define C_MASK  ((uint32_t)(1 << C_BIT)) /* Carry flag */
#define V_MASK  ((uint32_t)(1 << V_BIT)) /* oVerflow flag */
#define Q_MASK  ((uint32_t)(1 << Q_BIT)) /* signed overflow for QADD, MAC */
#define I_MASK  ((uint32_t)(1 << I_BIT)) /* Interrupt request disable */
#define F_MASK  ((uint32_t)(1 << F_BIT)) /* Fast interrupt request disable */
#define T_MASK  ((uint32_t)(1 << T_BIT)) /* Thumb Mode flag */

#define N_IS_SET(pc)    ((pc) & N_MASK)
#define Z_IS_SET(pc)    ((pc) & Z_MASK)
#define C_IS_SET(pc)    ((pc) & C_MASK)
#define V_IS_SET(pc)    ((pc) & V_MASK)
#define Q_IS_SET(pc)    ((pc) & Q_MASK)
#define I_IS_SET(pc)    ((pc) & I_MASK)
#define F_IS_SET(pc)    ((pc) & F_MASK)
#define T_IS_SET(pc)    ((pc) & T_MASK)

#define N_IS_CLEAR(pc)  (!N_IS_SET(pc))
#define Z_IS_CLEAR(pc)  (!Z_IS_SET(pc))
#define C_IS_CLEAR(pc)  (!C_IS_SET(pc))
#define V_IS_CLEAR(pc)  (!V_IS_SET(pc))
#define Q_IS_CLEAR(pc)  (!Q_IS_SET(pc))
#define I_IS_CLEAR(pc)  (!I_IS_SET(pc))
#define F_IS_CLEAR(pc)  (!F_IS_SET(pc))
#define T_IS_CLEAR(pc)  (!T_IS_SET(pc))

/* Deconstructing an instruction */
// TODO: use these in all places (including dasm file)
#define INSN_COND           ((uint32_t)0xf0000000u)
#define INSN_SDT_L          ((uint32_t)0x00100000u)
#define INSN_SDT_W          ((uint32_t)0x00200000u)
#define INSN_SDT_B          ((uint32_t)0x00400000u)
#define INSN_SDT_U          ((uint32_t)0x00800000u)
#define INSN_SDT_P          ((uint32_t)0x01000000u)
#define INSN_BDT_L          ((uint32_t)0x00100000u)
#define INSN_BDT_W          ((uint32_t)0x00200000u)
#define INSN_BDT_S          ((uint32_t)0x00400000u)
#define INSN_BDT_U          ((uint32_t)0x00800000u)
#define INSN_BDT_P          ((uint32_t)0x01000000u)
#define INSN_BDT_REGS       ((uint32_t)0x0000ffffu)
#define INSN_SDT_IMM        ((uint32_t)0x00000fffu)
#define INSN_MUL_A          ((uint32_t)0x00200000u)
#define INSN_MUL_RM         ((uint32_t)0x0000000fu)
#define INSN_MUL_RS         ((uint32_t)0x00000f00u)
#define INSN_MUL_RN         ((uint32_t)0x0000f000u)
#define INSN_MUL_RD         ((uint32_t)0x000f0000u)
#define INSN_I              ((uint32_t)0x02000000u)
#define INSN_OPCODE         ((uint32_t)0x01e00000u)
#define INSN_S              ((uint32_t)0x00100000u)
#define INSN_BL             ((uint32_t)0x01000000u)
#define INSN_BRANCH         ((uint32_t)0x00ffffffu)
#define INSN_SWI            ((uint32_t)0x00ffffffu)
#define INSN_RN             ((uint32_t)0x000f0000u)
#define INSN_RD             ((uint32_t)0x0000f000u)
#define INSN_OP2            ((uint32_t)0x00000fffu)
#define INSN_OP2_SHIFT      ((uint32_t)0x00000f80u)
#define INSN_OP2_SHIFT_TYPE ((uint32_t)0x00000070u)
#define INSN_OP2_RM         ((uint32_t)0x0000000fu)
#define INSN_OP2_ROTATE     ((uint32_t)0x00000f00u)
#define INSN_OP2_IMM        ((uint32_t)0x000000ffu)
#define INSN_OP2_SHIFT_TYPE_SHIFT   4
#define INSN_OP2_SHIFT_SHIFT        7
#define INSN_OP2_ROTATE_SHIFT       8
#define INSN_MUL_RS_SHIFT           8
#define INSN_MUL_RN_SHIFT           12
#define INSN_MUL_RD_SHIFT           16
#define INSN_OPCODE_SHIFT           21
#define INSN_RN_SHIFT               16
#define INSN_RD_SHIFT               12
#define INSN_COND_SHIFT             28

#define INSN_COPRO_OP1      ((uint32_t) 0x00e00000u)
#define INSN_COPRO_N        ((uint32_t) 0x00100000u)
#define INSN_COPRO_CREG     ((uint32_t) 0x000f0000u)
#define INSN_COPRO_AREG     ((uint32_t) 0x0000f000u)
#define INSN_COPRO_CPNUM    ((uint32_t) 0x00000f00u)
#define INSN_COPRO_OP2      ((uint32_t) 0x000000e0u)
#define INSN_COPRO_OP3      ((uint32_t) 0x0000000fu)
#define INSN_COPRO_OP1_SHIFT        21
#define INSN_COPRO_N_SHIFT          20
#define INSN_COPRO_CREG_SHIFT       16
#define INSN_COPRO_AREG_SHIFT       12
#define INSN_COPRO_CPNUM_SHIFT      8
#define INSN_COPRO_OP2_SHIFT        5

#define THUMB_INSN_TYPE     ((uint16_t)0xf000)
#define THUMB_COND_TYPE     ((uint16_t)0x0f00)
#define THUMB_GROUP4_TYPE   ((uint16_t)0x0c00)
#define THUMB_GROUP5_TYPE   ((uint16_t)0x0e00)
#define THUMB_GROUP5_RM     ((uint16_t)0x01c0)
#define THUMB_GROUP5_RN     ((uint16_t)0x0038)
#define THUMB_GROUP5_RD     ((uint16_t)0x0007)
#define THUMB_ADDSUB_RNIMM  ((uint16_t)0x01c0)
#define THUMB_ADDSUB_RS     ((uint16_t)0x0038)
#define THUMB_ADDSUB_RD     ((uint16_t)0x0007)
#define THUMB_INSN_CMP      ((uint16_t)0x0800)
#define THUMB_INSN_SUB      ((uint16_t)0x0800)
#define THUMB_INSN_IMM_RD   ((uint16_t)0x0700)
#define THUMB_INSN_IMM_S    ((uint16_t)0x0080)
#define THUMB_INSN_IMM      ((uint16_t)0x00ff)
#define THUMB_INSN_ADDSUB   ((uint16_t)0x0800)
#define THUMB_ADDSUB_TYPE   ((uint16_t)0x0600)
#define THUMB_HIREG_OP      ((uint16_t)0x0300)
#define THUMB_HIREG_H       ((uint16_t)0x00c0)
#define THUMB_HIREG_RS      ((uint16_t)0x0038)
#define THUMB_HIREG_RD      ((uint16_t)0x0007)
#define THUMB_STACKOP_TYPE  ((uint16_t)0x0f00)
#define THUMB_STACKOP_L     ((uint16_t)0x0800)
#define THUMB_STACKOP_RD    ((uint16_t)0x0700)
#define THUMB_ALUOP_TYPE    ((uint16_t)0x03c0)
#define THUMB_BLOP_LO       ((uint16_t)0x0800)
#define THUMB_BLOP_OFFS     ((uint16_t)0x07ff)
#define THUMB_SHIFT_R       ((uint16_t)0x0800)
#define THUMB_SHIFT_AMT     ((uint16_t)0x07c0)
#define THUMB_HALFOP_L      ((uint16_t)0x0800)
#define THUMB_HALFOP_OFFS   ((uint16_t)0x07c0)
#define THUMB_BRANCH_OFFS   ((uint16_t)0x07ff)
#define THUMB_LSOP_L        ((uint16_t)0x0800)
#define THUMB_LSOP_OFFS     ((uint16_t)0x07c0)
#define THUMB_MULTLS        ((uint16_t)0x0800)
#define THUMB_MULTLS_BASE   ((uint16_t)0x0700)
#define THUMB_RELADDR_SP    ((uint16_t)0x0800)
#define THUMB_RELADDR_RD    ((uint16_t)0x0700)
#define THUMB_INSN_TYPE_SHIFT       12
#define THUMB_COND_TYPE_SHIFT       8
#define THUMB_GROUP4_TYPE_SHIFT     10
#define THUMB_GROUP5_TYPE_SHIFT     9
#define THUMB_ADDSUB_TYPE_SHIFT     9
#define THUMB_INSN_IMM_RD_SHIFT     8
#define THUMB_STACKOP_TYPE_SHIFT    8
#define THUMB_HIREG_OP_SHIFT        8
#define THUMB_STACKOP_RD_SHIFT      8
#define THUMB_MULTLS_BASE_SHIFT     8
#define THUMB_RELADDR_RD_SHIFT      8
#define THUMB_HIREG_H_SHIFT         6
#define THUMB_HIREG_RS_SHIFT        3
#define THUMB_ALUOP_TYPE_SHIFT      6
#define THUMB_SHIFT_AMT_SHIFT       6
#define THUMB_HALFOP_OFFS_SHIFT     6
#define THUMB_LSOP_OFFS_SHIFT       6
#define THUMB_GROUP5_RM_SHIFT       6
#define THUMB_GROUP5_RN_SHIFT       3
#define THUMB_GROUP5_RD_SHIFT       0
#define THUMB_ADDSUB_RNIMM_SHIFT    6
#define THUMB_ADDSUB_RS_SHIFT       3
#define THUMB_ADDSUB_RD_SHIFT       0

enum
{
	OPCODE_AND, /* 0000 */
	OPCODE_EOR, /* 0001 */
	OPCODE_SUB, /* 0010 */
	OPCODE_RSB, /* 0011 */
	OPCODE_ADD, /* 0100 */
	OPCODE_ADC, /* 0101 */
	OPCODE_SBC, /* 0110 */
	OPCODE_RSC, /* 0111 */
	OPCODE_TST, /* 1000 */
	OPCODE_TEQ, /* 1001 */
	OPCODE_CMP, /* 1010 */
	OPCODE_CMN, /* 1011 */
	OPCODE_ORR, /* 1100 */
	OPCODE_MOV, /* 1101 */
	OPCODE_BIC, /* 1110 */
	OPCODE_MVN  /* 1111 */
};

enum
{
	COND_EQ = 0,          /*  Z           equal                   */
	COND_NE,              /* ~Z           not equal               */
	COND_CS, COND_HS = 2, /*  C           unsigned higher or same */
	COND_CC, COND_LO = 3, /* ~C           unsigned lower          */
	COND_MI,              /*  N           negative                */
	COND_PL,              /* ~N           positive or zero        */
	COND_VS,              /*  V           overflow                */
	COND_VC,              /* ~V           no overflow             */
	COND_HI,              /*  C && ~Z     unsigned higher         */
	COND_LS,              /* ~C ||  Z     unsigned lower or same  */
	COND_GE,              /*  N == V      greater or equal        */
	COND_LT,              /*  N != V      less than               */
	COND_GT,              /* ~Z && N == V greater than            */
	COND_LE,              /*  Z || N != V less than or equal      */
	COND_AL,              /*  1           always                  */
	COND_NV               /*  0           never                   */
};

/* Convenience Macros */
#define R15                     m_r[eR15]
#define SPSR                    17                     // SPSR is always the 18th register in our 0 based array sRegisterTable[][18]
#define GET_CPSR                m_r[eCPSR]
#define MODE_FLAG               0xF                    // Mode bits are 4:0 of CPSR, but we ignore bit 4.
#define GET_MODE                (GET_CPSR & MODE_FLAG)
#define SIGN_BIT                ((uint32_t)(1 << 31))
#define SIGN_BITS_DIFFER(a, b)  (((a) ^ (b)) >> 31)
/* I really don't know why these were set to 16-bit, the thumb registers are still 32-bit ... */
#define THUMB_SIGN_BIT               ((uint32_t)(1 << 31))
#define THUMB_SIGN_BITS_DIFFER(a, b) (((a)^(b)) >> 31)

#define SR_MODE32               0x10

#define MODE32                  (GET_CPSR & SR_MODE32)
#define MODE26                  (!(GET_CPSR & SR_MODE32))
#define GET_PC                  (MODE32 ? R15 : R15 & 0x03FFFFFC)

#define ARM7_TLB_ABORT_D (1 << 0)
#define ARM7_TLB_ABORT_P (1 << 1)
#define ARM7_TLB_READ    (1 << 2)
#define ARM7_TLB_WRITE   (1 << 3)

/* ARM flavors */
enum arm7_cpu_device::arm_arch_flag : uint32_t
{
	ARCHFLAG_T        = 1U << 0,    // Thumb present
	ARCHFLAG_E        = 1U << 1,    // extended DSP operations present (only for v5+)
	ARCHFLAG_J        = 1U << 2,    // "Jazelle" (direct execution of Java bytecode)
	ARCHFLAG_MMU      = 1U << 3,    // has on-board MMU (traditional ARM style like the SA1110)
	ARCHFLAG_SA       = 1U << 4,    // StrongARM extensions (enhanced TLB)
	ARCHFLAG_XSCALE   = 1U << 5,    // XScale extensions (CP14, enhanced TLB)
	ARCHFLAG_MODE26   = 1U << 6,    // supports 26-bit backwards compatibility mode
	ARCHFLAG_K        = 1U << 7,    // enhanced MMU extensions present (only for v6)
	ARCHFLAG_T2       = 1U << 8,    // Thumb-2 present
};

enum arm7_cpu_device::arm_copro_id : uint32_t
{
	ARM9_COPRO_ID_STEP_SA1100_A = 0,
	ARM9_COPRO_ID_STEP_SA1100_B = 1,
	ARM9_COPRO_ID_STEP_SA1100_C = 2,
	ARM9_COPRO_ID_STEP_SA1100_D = 8,
	ARM9_COPRO_ID_STEP_SA1100_E = 9,
	ARM9_COPRO_ID_STEP_SA1100_G = 11,

	ARM9_COPRO_ID_STEP_SA1110_A0 = 0,
	ARM9_COPRO_ID_STEP_SA1110_B0 = 4,
	ARM9_COPRO_ID_STEP_SA1110_B1 = 5,
	ARM9_COPRO_ID_STEP_SA1110_B2 = 6,
	ARM9_COPRO_ID_STEP_SA1110_B4 = 8,

	ARM9_COPRO_ID_STEP_PXA255_A0 = 6,

	ARM9_COPRO_ID_STEP_ARM946_A0 = 1,

	ARM9_COPRO_ID_STEP_ARM1176JZF_S_R0P0 = 0,
	ARM9_COPRO_ID_STEP_ARM1176JZF_S_R0P7 = 7,

	ARM9_COPRO_ID_PART_ARM1176JZF_S = 0xb76 << 4,
	ARM9_COPRO_ID_PART_SA1100 = 0xa11 << 4,
	ARM9_COPRO_ID_PART_SA1110 = 0xb11 << 4,
	ARM9_COPRO_ID_PART_ARM946 = 0x946 << 4,
	ARM9_COPRO_ID_PART_ARM920 = 0x920 << 4,
	ARM9_COPRO_ID_PART_ARM710 = 0x710 << 4,
	ARM9_COPRO_ID_PART_PXA250 = 0x200 << 4,
	ARM9_COPRO_ID_PART_PXA255 = 0x2d0 << 4,
	ARM9_COPRO_ID_PART_PXA270 = 0x411 << 4,
	ARM9_COPRO_ID_PART_GENERICARM7 = 0x700 << 4,

	ARM9_COPRO_ID_PXA255_CORE_REV_SHIFT = 10,

	ARM9_COPRO_ID_ARCH_V4     = 0x01 << 16,
	ARM9_COPRO_ID_ARCH_V4T    = 0x02 << 16,
	ARM9_COPRO_ID_ARCH_V5     = 0x03 << 16,
	ARM9_COPRO_ID_ARCH_V5T    = 0x04 << 16,
	ARM9_COPRO_ID_ARCH_V5TE   = 0x05 << 16,
	ARM9_COPRO_ID_ARCH_V5TEJ  = 0x06 << 16,
	ARM9_COPRO_ID_ARCH_V6     = 0x07 << 16,
	ARM9_COPRO_ID_ARCH_CPUID  = 0x0f << 16,

	ARM9_COPRO_ID_SPEC_REV0   = 0x00 << 20,
	ARM9_COPRO_ID_SPEC_REV1   = 0x01 << 20,

	ARM9_COPRO_ID_MFR_ARM = 0x41 << 24,
	ARM9_COPRO_ID_MFR_DEC = 0x44 << 24,
	ARM9_COPRO_ID_MFR_INTEL = 0x69 << 24
};

#endif // MAME_CPU_ARM7_ARM7CORE_H
