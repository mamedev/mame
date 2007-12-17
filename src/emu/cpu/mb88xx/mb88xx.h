/***************************************************************************

    mb88xx.h
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi

***************************************************************************/

#ifndef _MB88xx_H_
#define	_MB88xx_H_

#ifndef INLINE
#define INLINE static inline
#endif

#include "cpuintrf.h"

/***************************************************************************
    PORT ENUMERATION
***************************************************************************/

enum
{
	MB88_PORTK = 0,	/* input only, 4 bits */
	MB88_PORTO,		/* output only, PLA function output */
	MB88_PORTP,		/* 4 bits */
	MB88_PORTR0,	/* R0-R3, 4 bits */
	MB88_PORTR1,	/* R4-R7, 4 bits */
	MB88_PORTR2,	/* R8-R11, 4 bits */
	MB88_PORTR3		/* R12-R15, 4 bits */
};

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	MB88_PC=1,
	MB88_PA,
	MB88_FLAGS,
	MB88_SI,
	MB88_A,
	MB88_X,
	MB88_Y,
	MB88_PIO,
	MB88_TH,
	MB88_TL,
	MB88_SB
};

#define MB88_IRQ_LINE		0

/***************************************************************************
    CONFIG STRUCTURE
***************************************************************************/

struct MB88Config
{
	UINT8		*PLA_config;		/* PLA configuration (32 byte values), if NULL assume direct output */
};

/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

void mb88_get_info(UINT32 state, cpuinfo *info);
void mb8841_get_info(UINT32 state, cpuinfo *info);
void mb8842_get_info(UINT32 state, cpuinfo *info);
void mb8843_get_info(UINT32 state, cpuinfo *info);
void mb8844_get_info(UINT32 state, cpuinfo *info);

#ifdef MAME_DEBUG
offs_t mb88_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif
