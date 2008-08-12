/***************************************************************************

    mb88xx.h
    Core implementation for the portable Fujitsu MB88xx series MCU emulator.

    Written by Ernesto Corvi

***************************************************************************/

#pragma once

#ifndef __MB88XX_H__
#define	__MB88XX_H__

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

typedef struct _mb88_cpu_core mb88_cpu_core;
struct _mb88_cpu_core
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

offs_t mb88_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif /* __MB88XX_H__ */
