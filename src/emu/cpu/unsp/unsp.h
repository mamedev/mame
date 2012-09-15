/**************************\
*
*   SunPlus u'nSP emulator
*
*    by Harmony
*
\**************************/

#pragma once

#ifndef __UNSP_H__
#define __UNSP_H__

typedef struct _unspimp_state unspimp_state;
struct unsp_state
{
	UINT16 r[16];
	UINT8 irq;
	UINT8 fiq;
	UINT16 curirq;
	UINT16 sirq;
	UINT8 sb;
	UINT8 saved_sb;

	legacy_cpu_device *device;
	address_space *program;
	int icount;

	unspimp_state *impstate;
};

enum
{
	UNSP_SP = 1,
	UNSP_R1,
	UNSP_R2,
	UNSP_R3,
	UNSP_R4,
	UNSP_BP,
	UNSP_SR,
	UNSP_PC,

	UNSP_GPR_COUNT = UNSP_PC,

	UNSP_IRQ,
	UNSP_FIQ,
	UNSP_SB,

};

enum
{
    UNSP_IRQ0_LINE = 0,
    UNSP_IRQ1_LINE,
    UNSP_IRQ2_LINE,
    UNSP_IRQ3_LINE,
    UNSP_IRQ4_LINE,
    UNSP_IRQ5_LINE,
    UNSP_IRQ6_LINE,
    UNSP_IRQ7_LINE,
    UNSP_FIQ_LINE,
    UNSP_BRK_LINE,

    UNSP_NUM_LINES
};

DECLARE_LEGACY_CPU_DEVICE(UNSP, unsp);
CPU_DISASSEMBLE( unsp );

#endif /* __UNSP_H__ */
