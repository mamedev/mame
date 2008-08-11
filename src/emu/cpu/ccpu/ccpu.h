/***************************************************************************

    ccpu.h
    Core implementation for the portable Cinematronics CPU emulator.

    Written by Aaron Giles
    Special thanks to Zonn Moore for his detailed documentation.

***************************************************************************/

#pragma once

#ifndef __CCPU_H__
#define	__CCPU_H__

#include "cpuintrf.h"


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	CCPU_PC=1,
	CCPU_FLAGS,
	CCPU_A,
	CCPU_B,
	CCPU_I,
	CCPU_J,
	CCPU_P,
	CCPU_X,
	CCPU_Y,
	CCPU_T
};



/***************************************************************************
    CONFIG STRUCTURE
***************************************************************************/

typedef struct _ccpu_config ccpu_config;
struct _ccpu_config
{
	UINT8		(*external_input)(void);		/* if NULL, assume JMI jumper is present */
	void		(*vector_callback)(INT16 sx, INT16 sy, INT16 ex, INT16 ey, UINT8 shift);
};



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void ccpu_get_info(UINT32 state, cpuinfo *info);
void ccpu_wdt_timer_trigger(void);

offs_t ccpu_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __CCPU_H__ */
