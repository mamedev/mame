#pragma once

#ifndef __LR35902_H__
#define __LR35902_H__


typedef void (*lr35902_timer_fired_func)(running_device *device, int cycles);

typedef struct _lr35902_cpu_core lr35902_cpu_core;
struct _lr35902_cpu_core
{
	const UINT16	*regs;
	UINT8			features;
	lr35902_timer_fired_func timer_fired_func;
};

enum
{
	LR35902_PC=1, LR35902_SP, LR35902_AF, LR35902_BC, LR35902_DE, LR35902_HL,
	LR35902_IRQ_STATE,
	/* Pseudo registers to keep track of the interrupt statuses */
	LR35902_IE, LR35902_IF,
	/* Pseudo register to change and check the cpu operating speed */
	LR35902_SPEED,
};

#define LR35902_FEATURE_HALT_BUG	0x01

/****************************************************************************/
/* Return register contents                                                 */
/****************************************************************************/
DECLARE_LEGACY_CPU_DEVICE(LR35902, lr35902);

extern CPU_DISASSEMBLE( lr35902 );

#endif /* __LR35902_H__ */
