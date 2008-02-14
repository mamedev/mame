/***************************************************************************

    asap.h
    Interface file for the portable Atari ASAP emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef _ASAP_H
#define _ASAP_H

#include "cpuintrf.h"


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	ASAP_PC=1,ASAP_PS,
	ASAP_R0,ASAP_R1,ASAP_R2,ASAP_R3,ASAP_R4,ASAP_R5,ASAP_R6,ASAP_R7,
	ASAP_R8,ASAP_R9,ASAP_R10,ASAP_R11,ASAP_R12,ASAP_R13,ASAP_R14,ASAP_R15,
	ASAP_R16,ASAP_R17,ASAP_R18,ASAP_R19,ASAP_R20,ASAP_R21,ASAP_R22,ASAP_R23,
	ASAP_R24,ASAP_R25,ASAP_R26,ASAP_R27,ASAP_R28,ASAP_R29,ASAP_R30,ASAP_R31
};


/***************************************************************************
    INTERRUPT CONSTANTS
***************************************************************************/

#define ASAP_IRQ0		0		/* IRQ0 */


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

extern void asap_get_info(UINT32 state, cpuinfo *info);

#endif /* _ASAP_H */
