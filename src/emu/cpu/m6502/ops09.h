/*****************************************************************************
 *
 *   ops09.h
 *
 *   Copyright Peter Trauner, all rights reserved.
 *   documentation by michael steil mist@c64.org
 *   available at ftp://ftp.funet.fi/pub/cbm/c65
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#define ZPWH	cpustate->zp.w.h

#define EAWH	cpustate->ea.w.h

#define PBWH	cpustate->pc_bank.w.h
#define PB		cpustate->pc_bank.d

#define IBWH	cpustate->ind_bank.w.h
#define IB		cpustate->ind_bank.d

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#undef RDOP
#define RDOP() cpustate->direct->read_decrypted_byte((PCW++)|PB); cpustate->icount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#undef RDOPARG
#define RDOPARG() cpustate->direct->read_raw_byte((PCW++)|PB); cpustate->icount -= 1

/***************************************************************
 *  RDMEM   read memory
 ***************************************************************/
#undef RDMEM
#define RDMEM(addr) cpustate->space->read_byte(addr); cpustate->icount -= 1

/***************************************************************
 *  WRMEM   write memory
 ***************************************************************/
#undef WRMEM
#define WRMEM(addr,data) cpustate->space->write_byte(addr,data); cpustate->icount -= 1

/***************************************************************
 * push a register onto the stack
 ***************************************************************/
#undef PUSH
#define PUSH(Rg) WRMEM(SPD|PB, Rg); S--

/***************************************************************
 * pull a register from the stack
 ***************************************************************/
#undef PULL
#define PULL(Rg) S++; Rg = RDMEM(SPD|PB)


/***************************************************************
 *  EA = zero page address
 ***************************************************************/
#undef EA_ZPG
#define EA_ZPG													\
	ZPL = RDOPARG();											\
	ZPWH = PBWH;												\
    EAD = ZPD

/***************************************************************
 *  EA = zero page address + X
 ***************************************************************/
#undef EA_ZPX
#define EA_ZPX													\
	ZPL = X + RDOPARG();										\
	ZPWH = PBWH;												\
    EAD = ZPD

/***************************************************************
 *  EA = zero page address + Y
 ***************************************************************/
#undef EA_ZPY
#define EA_ZPY													\
	ZPL = Y + RDOPARG();										\
	ZPWH = PBWH;												\
    EAD = ZPD

/***************************************************************
 *  EA = absolute address
 ***************************************************************/
#undef EA_ABS
#define EA_ABS													\
	EAL = RDOPARG();											\
	EAH = RDOPARG();											\
    EAWH = PBWH

/***************************************************************
 *  EA = zero page + X indirect (pre indexed)
 ***************************************************************/
#undef EA_IDX
#define EA_IDX													\
	ZPL = X + RDOPARG();										\
	ZPWH=PBWH;													\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
    EAWH = PBWH

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 *  subtract 1 cycle if page boundary is crossed
 ***************************************************************/
#undef EA_IDY
#define EA_IDY													\
	ZPL = RDOPARG();											\
	ZPWH = PBWH;												\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	EAWH = PBWH;												\
    if (EAL + Y > 0xff)                                         \
		cpustate->icount--; 									\
	EAW += Y

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 *  subtract 1 cycle if page boundary is crossed
 ***************************************************************/
#undef EA_IDY_P
#define EA_IDY_P												\
	ZPL = RDOPARG();											\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	EAWH = PBWH;												\
	if (EAL + Y > 0xff) {										\
		RDMEM( ( EAH << 8 ) | ( ( EAL + Y ) & 0xff ) );			\
	}															\
	EAW += Y;

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 *  subtract 1 cycle if page boundary is crossed
 ***************************************************************/
#define EA_IDY_6509 											\
	ZPL = RDOPARG();											\
	ZPWH = PBWH;												\
	EAL = RDMEM(ZPD);											\
	ZPL++;														\
	EAH = RDMEM(ZPD);											\
	EAWH = IBWH;												\
    if (EAL + Y > 0xff)                                         \
		cpustate->icount--; 									\
	EAW += Y

/***************************************************************
 *  EA = indirect (only used by JMP)
 ***************************************************************/
#undef EA_IND
#define EA_IND													\
	EA_ABS; 													\
	tmp = RDMEM(EAD);											\
	EAL++;	/* booby trap: stay in same page! ;-) */			\
	EAH = RDMEM(EAD);											\
	EAL = tmp;
/*  EAWH = PBWH */

#define RD_IDY_6509	EA_IDY_6509; tmp = RDMEM(EAD)
#define WR_IDY_6509	EA_IDY_6509; WRMEM(EAD, tmp)

/***************************************************************
 *  BRA  branch relative
 *  extra cycle if page boundary is crossed
 ***************************************************************/
#undef BRA
#define BRA(cond)                                               \
	if (cond)													\
	{															\
		tmp = RDOPARG();										\
		EAW = PCW + (signed char)tmp;							\
		cpustate->icount -= (PCH == EAH) ? 1 : 2;				\
		PCD = EAD|PB;											\
	}															\
	else														\
	{															\
		PCW++;													\
		cpustate->icount -= 1;									\
	}

/* 6502 ********************************************************
 *  JSR Jump to subroutine
 *  decrement PC (sic!) push PC hi, push PC lo and set
 *  PC to the effective address
 ***************************************************************/
#undef JSR
#define JSR 													\
	EAL = RDOPARG();											\
	PUSH(PCH);													\
	PUSH(PCL);													\
	EAH = RDOPARG();											\
	EAWH = PBWH;												\
	PCD = EAD

/* 6510 ********************************************************
 *  KIL Illegal opcode
 * processor haltet, no hardware interrupt will help
 * only reset
 ***************************************************************/
#undef KIL
#define KIL 													\
	PCW--;														\
	logerror("M6509 KILL opcode %05x: %02x\n", PCD, cpustate->direct->read_decrypted_byte(PCD))
