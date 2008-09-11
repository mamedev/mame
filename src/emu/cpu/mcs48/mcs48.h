/***************************************************************************

    mcs48.c

    Intel MCS-48 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator
    You are not allowed to distribute this software commercially

***************************************************************************/

#pragma once

#ifndef __I8039_H__
#define __I8039_H__

#include "cpuintrf.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register access indexes */
enum
{
	MCS48_PC = 1,
	MCS48_PSW,
	MCS48_A,
	MCS48_TC,
	MCS48_P1,
	MCS48_P2,
	MCS48_R0,
	MCS48_R1,
	MCS48_R2,
	MCS48_R3,
	MCS48_R4,
	MCS48_R5,
	MCS48_R6,
	MCS48_R7,
	MCS48_EA
};


/* I/O port access indexes */
enum
{
	MCS48_INPUT_IRQ = 0,
	MCS48_INPUT_EA
};


/* special I/O space ports */
enum
{
	MCS48_PORT_P0	= 0x100,	/* Not used */
	MCS48_PORT_P1	= 0x101,
	MCS48_PORT_P2	= 0x102,
	MCS48_PORT_P4	= 0x104,
	MCS48_PORT_P5	= 0x105,
	MCS48_PORT_P6	= 0x106,
	MCS48_PORT_P7	= 0x107,
	MCS48_PORT_T0	= 0x110,
	MCS48_PORT_T1	= 0x111,
	MCS48_PORT_BUS	= 0x120
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* variants with 64 bytes of internal RAM and up to 1k of internal ROM */
void i8035_get_info(UINT32 state, cpuinfo *info);
void i8041_get_info(UINT32 state, cpuinfo *info);
void i8048_get_info(UINT32 state, cpuinfo *info);
void i8648_get_info(UINT32 state, cpuinfo *info);
void i8748_get_info(UINT32 state, cpuinfo *info);
void mb8884_get_info(UINT32 state, cpuinfo *info);
void n7751_get_info(UINT32 state, cpuinfo *info);

/* variants with 128 bytes of internal RAM and up to 2k of internal ROM */
void i8039_get_info(UINT32 state, cpuinfo *info);
void i8049_get_info(UINT32 state, cpuinfo *info);
void i8749_get_info(UINT32 state, cpuinfo *info);
void m58715_get_info(UINT32 state, cpuinfo *info);

/* disassembler */
offs_t mcs48_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif  /* __I8039_H__ */
