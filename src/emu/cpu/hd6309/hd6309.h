/*** hd6309: Portable 6309 emulator ******************************************/

#pragma once

#ifndef __HD6309_H__
#define __HD6309_H__


enum
{
	HD6309_PC=1, HD6309_S, HD6309_CC ,HD6309_A, HD6309_B, HD6309_U, HD6309_X, HD6309_Y, HD6309_DP,
	HD6309_E, HD6309_F, HD6309_V, HD6309_MD
};

#define HD6309_IRQ_LINE 0   /* IRQ line number */
#define HD6309_FIRQ_LINE 1   /* FIRQ line number */


/* PUBLIC FUNCTIONS */
DECLARE_LEGACY_CPU_DEVICE(HD6309, hd6309);


CPU_DISASSEMBLE( hd6309 );

#endif /* __HD6309_H__ */
