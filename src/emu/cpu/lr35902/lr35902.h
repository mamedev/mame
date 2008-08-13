#pragma once

#ifndef __LR35902_H__
#define __LR35902_H__

#include "cpuintrf.h"

typedef struct _lr35902_cpu_core lr35902_cpu_core;
struct _lr35902_cpu_core
{
	const UINT16	*regs;
	UINT8	features;
	void	(*timer_fired_func)(int cycles);
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
extern void lr35902_get_info(UINT32 state, cpuinfo *info);

extern unsigned lr35902_dasm( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram );

#endif /* __LR35902_H__ */
