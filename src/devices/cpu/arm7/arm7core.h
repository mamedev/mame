// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7core.h
 *   Portable ARM7TDMI Core Emulator
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************

 This file contains everything related to the arm7 core itself, and is presumed
 to be cpu implementation non-specific, ie, applies to only the core.

 ******************************************************************************/

#pragma once

#ifndef __ARM7CORE_H__
#define __ARM7CORE_H__

#define ARM7_MMU_ENABLE_HACK 0
#define ARM7_DEBUG_CORE 0


/****************************************************************************************************
 *  INTERRUPT LINES/EXCEPTIONS
 ***************************************************************************************************/
enum
{
	ARM7_IRQ_LINE=0, ARM7_FIRQ_LINE,
	ARM7_ABORT_EXCEPTION, ARM7_ABORT_PREFETCH_EXCEPTION, ARM7_UNDEFINE_EXCEPTION,
	ARM7_NUM_LINES
};
// Really there's only 1 ABORT Line.. and cpu decides whether it's during data fetch or prefetch, but we let the user specify

/****************************************************************************************************
 *  ARM7 CORE REGISTERS
 ***************************************************************************************************/
enum
{
	ARM7_PC = 0,
	ARM7_R0, ARM7_R1, ARM7_R2, ARM7_R3, ARM7_R4, ARM7_R5, ARM7_R6, ARM7_R7,
	ARM7_R8, ARM7_R9, ARM7_R10, ARM7_R11, ARM7_R12, ARM7_R13, ARM7_R14, ARM7_R15,
	ARM7_FR8, ARM7_FR9, ARM7_FR10, ARM7_FR11, ARM7_FR12, ARM7_FR13, ARM7_FR14,
	ARM7_IR13, ARM7_IR14, ARM7_SR13, ARM7_SR14, ARM7_FSPSR, ARM7_ISPSR, ARM7_SSPSR,
	ARM7_CPSR, ARM7_AR13, ARM7_AR14, ARM7_ASPSR, ARM7_UR13, ARM7_UR14, ARM7_USPSR
};

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
#define COPRO_TLB_UNMAPPED                  0
#define COPRO_TLB_LARGE_PAGE                1
#define COPRO_TLB_SMALL_PAGE                2
#define COPRO_TLB_TINY_PAGE                 3
#define COPRO_TLB_COARSE_TABLE              1
#define COPRO_TLB_SECTION_TABLE             2
#define COPRO_TLB_FINE_TABLE                3

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

enum
{
	eARM_ARCHFLAGS_T    = 1,        // Thumb present
	eARM_ARCHFLAGS_E    = 2,        // extended DSP operations present (only for v5+)
	eARM_ARCHFLAGS_J    = 4,        // "Jazelle" (direct execution of Java bytecode)
	eARM_ARCHFLAGS_MMU  = 8,        // has on-board MMU (traditional ARM style like the SA1110)
	eARM_ARCHFLAGS_SA   = 16,       // StrongARM extensions (enhanced TLB)
	eARM_ARCHFLAGS_XSCALE   = 32,       // XScale extensions (CP14, enhanced TLB)
	eARM_ARCHFLAGS_MODE26   = 64        // supports 26-bit backwards compatibility mode
};


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
	UINT32 m_r[NUM_REGS];
	UINT32 m_pendingIrq;
	UINT32 m_pendingFiq;
	UINT32 m_pendingAbtD;
	UINT32 m_pendingAbtP;
	UINT32 m_pendingUnd;
	UINT32 m_pendingSwi;
	int m_icount;
	endianness_t m_endian;
	address_space *m_program;
	direct_read_data *m_direct;

	/* Coprocessor Registers */
	UINT32 m_control;
	UINT32 m_tlbBase;
	UINT32 m_faultStatus[2];
	UINT32 m_faultAddress;
	UINT32 m_fcsePID;
	UINT32 m_domainAccessControl;

	UINT8 m_archRev;          // ARM architecture revision (3, 4, and 5 are valid)
	UINT8 m_archFlags;        // architecture flags

#if ARM7_MMU_ENABLE_HACK
	UINT32 mmu_enable_addr; // workaround for "MMU is enabled when PA != VA" problem
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

#define N_MASK  ((UINT32)(1 << N_BIT)) /* Negative flag */
#define Z_MASK  ((UINT32)(1 << Z_BIT)) /* Zero flag */
#define C_MASK  ((UINT32)(1 << C_BIT)) /* Carry flag */
#define V_MASK  ((UINT32)(1 << V_BIT)) /* oVerflow flag */
#define Q_MASK  ((UINT32)(1 << Q_BIT)) /* signed overflow for QADD, MAC */
#define I_MASK  ((UINT32)(1 << I_BIT)) /* Interrupt request disable */
#define F_MASK  ((UINT32)(1 << F_BIT)) /* Fast interrupt request disable */
#define T_MASK  ((UINT32)(1 << T_BIT)) /* Thumb Mode flag */

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
// todo: use these in all places (including dasm file)
#define INSN_COND           ((UINT32)0xf0000000u)
#define INSN_SDT_L          ((UINT32)0x00100000u)
#define INSN_SDT_W          ((UINT32)0x00200000u)
#define INSN_SDT_B          ((UINT32)0x00400000u)
#define INSN_SDT_U          ((UINT32)0x00800000u)
#define INSN_SDT_P          ((UINT32)0x01000000u)
#define INSN_BDT_L          ((UINT32)0x00100000u)
#define INSN_BDT_W          ((UINT32)0x00200000u)
#define INSN_BDT_S          ((UINT32)0x00400000u)
#define INSN_BDT_U          ((UINT32)0x00800000u)
#define INSN_BDT_P          ((UINT32)0x01000000u)
#define INSN_BDT_REGS       ((UINT32)0x0000ffffu)
#define INSN_SDT_IMM        ((UINT32)0x00000fffu)
#define INSN_MUL_A          ((UINT32)0x00200000u)
#define INSN_MUL_RM         ((UINT32)0x0000000fu)
#define INSN_MUL_RS         ((UINT32)0x00000f00u)
#define INSN_MUL_RN         ((UINT32)0x0000f000u)
#define INSN_MUL_RD         ((UINT32)0x000f0000u)
#define INSN_I              ((UINT32)0x02000000u)
#define INSN_OPCODE         ((UINT32)0x01e00000u)
#define INSN_S              ((UINT32)0x00100000u)
#define INSN_BL             ((UINT32)0x01000000u)
#define INSN_BRANCH         ((UINT32)0x00ffffffu)
#define INSN_SWI            ((UINT32)0x00ffffffu)
#define INSN_RN             ((UINT32)0x000f0000u)
#define INSN_RD             ((UINT32)0x0000f000u)
#define INSN_OP2            ((UINT32)0x00000fffu)
#define INSN_OP2_SHIFT      ((UINT32)0x00000f80u)
#define INSN_OP2_SHIFT_TYPE ((UINT32)0x00000070u)
#define INSN_OP2_RM         ((UINT32)0x0000000fu)
#define INSN_OP2_ROTATE     ((UINT32)0x00000f00u)
#define INSN_OP2_IMM        ((UINT32)0x000000ffu)
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

#define INSN_COPRO_N        ((UINT32) 0x00100000u)
#define INSN_COPRO_CREG     ((UINT32) 0x000f0000u)
#define INSN_COPRO_AREG     ((UINT32) 0x0000f000u)
#define INSN_COPRO_CPNUM    ((UINT32) 0x00000f00u)
#define INSN_COPRO_OP2      ((UINT32) 0x000000e0u)
#define INSN_COPRO_OP3      ((UINT32) 0x0000000fu)
#define INSN_COPRO_N_SHIFT          20
#define INSN_COPRO_CREG_SHIFT       16
#define INSN_COPRO_AREG_SHIFT       12
#define INSN_COPRO_CPNUM_SHIFT      8
#define INSN_COPRO_OP2_SHIFT        5

#define THUMB_INSN_TYPE     ((UINT16)0xf000)
#define THUMB_COND_TYPE     ((UINT16)0x0f00)
#define THUMB_GROUP4_TYPE   ((UINT16)0x0c00)
#define THUMB_GROUP5_TYPE   ((UINT16)0x0e00)
#define THUMB_GROUP5_RM     ((UINT16)0x01c0)
#define THUMB_GROUP5_RN     ((UINT16)0x0038)
#define THUMB_GROUP5_RD     ((UINT16)0x0007)
#define THUMB_ADDSUB_RNIMM  ((UINT16)0x01c0)
#define THUMB_ADDSUB_RS     ((UINT16)0x0038)
#define THUMB_ADDSUB_RD     ((UINT16)0x0007)
#define THUMB_INSN_CMP      ((UINT16)0x0800)
#define THUMB_INSN_SUB      ((UINT16)0x0800)
#define THUMB_INSN_IMM_RD   ((UINT16)0x0700)
#define THUMB_INSN_IMM_S    ((UINT16)0x0080)
#define THUMB_INSN_IMM      ((UINT16)0x00ff)
#define THUMB_INSN_ADDSUB   ((UINT16)0x0800)
#define THUMB_ADDSUB_TYPE   ((UINT16)0x0600)
#define THUMB_HIREG_OP      ((UINT16)0x0300)
#define THUMB_HIREG_H       ((UINT16)0x00c0)
#define THUMB_HIREG_RS      ((UINT16)0x0038)
#define THUMB_HIREG_RD      ((UINT16)0x0007)
#define THUMB_STACKOP_TYPE  ((UINT16)0x0f00)
#define THUMB_STACKOP_L     ((UINT16)0x0800)
#define THUMB_STACKOP_RD    ((UINT16)0x0700)
#define THUMB_ALUOP_TYPE    ((UINT16)0x03c0)
#define THUMB_BLOP_LO       ((UINT16)0x0800)
#define THUMB_BLOP_OFFS     ((UINT16)0x07ff)
#define THUMB_SHIFT_R       ((UINT16)0x0800)
#define THUMB_SHIFT_AMT     ((UINT16)0x07c0)
#define THUMB_HALFOP_L      ((UINT16)0x0800)
#define THUMB_HALFOP_OFFS   ((UINT16)0x07c0)
#define THUMB_BRANCH_OFFS   ((UINT16)0x07ff)
#define THUMB_LSOP_L        ((UINT16)0x0800)
#define THUMB_LSOP_OFFS     ((UINT16)0x07c0)
#define THUMB_MULTLS        ((UINT16)0x0800)
#define THUMB_MULTLS_BASE   ((UINT16)0x0700)
#define THUMB_RELADDR_SP    ((UINT16)0x0800)
#define THUMB_RELADDR_RD    ((UINT16)0x0700)
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

#define LSL(v, s) ((v) << (s))
#define LSR(v, s) ((v) >> (s))
#define ROL(v, s) (LSL((v), (s)) | (LSR((v), 32u - (s))))
#define ROR(v, s) (LSR((v), (s)) | (LSL((v), 32u - (s))))

/* Convenience Macros */
#define R15                     m_r[eR15]
#define SPSR                    17                     // SPSR is always the 18th register in our 0 based array sRegisterTable[][18]
#define GET_CPSR                m_r[eCPSR]
#define SET_CPSR(v)             set_cpsr(v)
#define MODE_FLAG               0xF                    // Mode bits are 4:0 of CPSR, but we ignore bit 4.
#define GET_MODE                (GET_CPSR & MODE_FLAG)
#define SIGN_BIT                ((UINT32)(1 << 31))
#define SIGN_BITS_DIFFER(a, b)  (((a) ^ (b)) >> 31)
/* I really don't know why these were set to 16-bit, the thumb registers are still 32-bit ... */
#define THUMB_SIGN_BIT               ((UINT32)(1 << 31))
#define THUMB_SIGN_BITS_DIFFER(a, b) (((a)^(b)) >> 31)

#define SR_MODE32               0x10

#define MODE32                  (GET_CPSR & SR_MODE32)
#define MODE26                  (!(GET_CPSR & SR_MODE32))
#define GET_PC                  (MODE32 ? R15 : R15 & 0x03FFFFFC)

#define ARM7_TLB_ABORT_D (1 << 0)
#define ARM7_TLB_ABORT_P (1 << 1)
#define ARM7_TLB_READ    (1 << 2)
#define ARM7_TLB_WRITE   (1 << 3)

/* At one point I thought these needed to be cpu implementation specific, but they don't.. */
#define GET_REGISTER(reg)       GetRegister(reg)
#define SET_REGISTER(reg, val)  SetRegister(reg, val)
#define GET_MODE_REGISTER(mode, reg)       GetModeRegister(mode, reg)
#define SET_MODE_REGISTER(mode, reg, val)  SetModeRegister(mode, reg, val)
#define ARM7_CHECKIRQ           arm7_check_irq_state()


/* ARM flavors */
enum arm_flavor
{
	/* ARM7 variants */
	ARM_TYPE_ARM7,
	ARM_TYPE_ARM7BE,
	ARM_TYPE_ARM7500,
	ARM_TYPE_PXA255,
	ARM_TYPE_SA1110,

	/* ARM9 variants */
	ARM_TYPE_ARM9,
	ARM_TYPE_ARM920T
};

#endif /* __ARM7CORE_H__ */
