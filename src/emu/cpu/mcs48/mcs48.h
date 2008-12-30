/***************************************************************************

    mcs48.c

    Intel MCS-48/UPI-41 Portable Emulator

    Copyright Mirko Buffoni
    Based on the original work Copyright Dan Boris, an 8048 emulator
    You are not allowed to distribute this software commercially

***************************************************************************/

#pragma once

#ifndef __MCS48_H__
#define __MCS48_H__

#include "cpuintrf.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register access indexes */
enum
{
	MCS48_PC,
	MCS48_PSW,
	MCS48_A,
	MCS48_TC,
	MCS48_TPRE,
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
	MCS48_EA,
	MCS48_STS,	/* UPI-41 systems only */
	MCS48_DBBO,	/* UPI-41 systems only */
	MCS48_DBBI,	/* UPI-41 systems only */
	
	MCS48_GENPC = REG_GENPC,
	MCS48_GENSP = REG_GENSP,
	MCS48_GENPCBASE = REG_GENPCBASE
};


/* I/O port access indexes */
enum
{
	MCS48_INPUT_IRQ = 0,
	UPI41_INPUT_IBF = 0,
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
	MCS48_PORT_BUS	= 0x120,
	MCS48_PORT_PROG	= 0x121		/* PROG line to 8243 expander */
};


/* 8243 expander operations */
enum
{
	MCS48_EXPANDER_OP_READ = 0,
	MCS48_EXPANDER_OP_WRITE = 1,
	MCS48_EXPANDER_OP_OR = 2,
	MCS48_EXPANDER_OP_AND = 3
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* MCS-48 variants with 64 bytes of internal RAM and up to 1k of internal ROM */
CPU_GET_INFO( i8035 );
CPU_GET_INFO( i8048 );
CPU_GET_INFO( i8648 );
CPU_GET_INFO( i8748 );
CPU_GET_INFO( mb8884 );
CPU_GET_INFO( n7751 );

#define CPU_I8035 CPU_GET_INFO_NAME( i8035 )
#define CPU_I8048 CPU_GET_INFO_NAME( i8048 )
#define CPU_I8648 CPU_GET_INFO_NAME( i8648 )
#define CPU_I8748 CPU_GET_INFO_NAME( i8748 )
#define CPU_MB8884 CPU_GET_INFO_NAME( mb8884 )
#define CPU_N7751 CPU_GET_INFO_NAME( n7751 )

/* MCS-48 variants with 128 bytes of internal RAM and up to 2k of internal ROM */
CPU_GET_INFO( i8039 );
CPU_GET_INFO( i8049 );
CPU_GET_INFO( i8749 );
CPU_GET_INFO( m58715 );

#define CPU_I8039 CPU_GET_INFO_NAME( i8039 )
#define CPU_I8049 CPU_GET_INFO_NAME( i8049 )
#define CPU_I8749 CPU_GET_INFO_NAME( i8749 )
#define CPU_M58715 CPU_GET_INFO_NAME( m58715 )


/* UPI-41 variants with 128 bytes of internal RAM and up to 1k of internal ROM */
CPU_GET_INFO( i8041 );
CPU_GET_INFO( i8741 );

#define CPU_I8041 CPU_GET_INFO_NAME( i8041 )
#define CPU_I8741 CPU_GET_INFO_NAME( i8741 )

/* UPI-41 variants with 256 bytes of internal RAM and up to 2k of internal ROM */
CPU_GET_INFO( i8042 );
CPU_GET_INFO( i8242 );
CPU_GET_INFO( i8742 );

#define CPU_I8042 CPU_GET_INFO_NAME( i8042 )
#define CPU_I8242 CPU_GET_INFO_NAME( i8242 )
#define CPU_I8742 CPU_GET_INFO_NAME( i8742 )


/* functions for talking to the input/output buffers on the UPI41-class chips */
UINT8 upi41_master_r(const device_config *device, UINT8 a0);
void upi41_master_w(const device_config *device, UINT8 a0, UINT8 data);


/* disassemblers */
CPU_DISASSEMBLE( mcs48 );
CPU_DISASSEMBLE( upi41 );


#endif  /* __MCS48_H__ */
