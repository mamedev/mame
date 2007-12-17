#ifndef _MB86233_H
#define _MB86233_H

#include "cpuintrf.h"

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB86233_PC=1,
	MB86233_A,
	MB86233_B,
	MB86233_D,
	MB86233_P,
	MB86233_REP,
	MB86233_SP,
	MB86233_EB,
	MB86233_SHIFT,
	MB86233_FLAGS,
	MB86233_R0,
	MB86233_R1,
	MB86233_R2,
	MB86233_R3,
	MB86233_R4,
	MB86233_R5,
	MB86233_R6,
	MB86233_R7,
	MB86233_R8,
	MB86233_R9,
	MB86233_R10,
	MB86233_R11,
	MB86233_R12,
	MB86233_R13,
	MB86233_R14,
	MB86233_R15
};

/***************************************************************************
    STRUCTURES
***************************************************************************/

struct mb86233_config
{
	int			(*fifo_read_cb)( UINT32* data );
	void		(*fifo_write_cb)( UINT32 data );
};

extern void mb86233_get_info(UINT32 state, cpuinfo *info);

#endif /* _MB86233_H */
