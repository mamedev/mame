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
CPU_GET_INFO( i8035 );
CPU_GET_INFO( i8048 );
CPU_GET_INFO( i8648 );
CPU_GET_INFO( i8748 );
CPU_GET_INFO( mb8884 );
CPU_GET_INFO( n7751 );

/* variants with 128 bytes of internal RAM and up to 2k of internal ROM */
CPU_GET_INFO( i8039 );
CPU_GET_INFO( i8049 );
CPU_GET_INFO( i8749 );
CPU_GET_INFO( m58715 );

/* disassembler */
CPU_DISASSEMBLE( mcs48 );

#endif  /* __I8039_H__ */
