#ifndef z80gb_H
#define z80gb_H

#include "cpuintrf.h"

extern int z80gb_ICount;

typedef struct {
	UINT16	*regs;
	UINT8	features;
	void	(*timer_callback)(int cycles);
} Z80GB_CONFIG;

enum {
	Z80GB_PC=1, Z80GB_SP, Z80GB_AF, Z80GB_BC, Z80GB_DE, Z80GB_HL,
	Z80GB_IRQ_STATE,
	/* Pseudo registers to keep track of the interrupt statuses */
	Z80GB_IE, Z80GB_IF,
	/* Pseudo register to change and check the cpu operating speed */
	Z80GB_SPEED,
};

#define Z80GB_FEATURE_HALT_BUG	0x01

/****************************************************************************/
/* Return register contents                                                 */
/****************************************************************************/
extern void z80gb_get_info(UINT32 state, cpuinfo *info);

#ifdef MAME_DEBUG
extern unsigned z80gb_dasm( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram );
#endif /* MAME_DEBUG */

#endif
