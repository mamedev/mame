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
	MCS48_STS,  /* UPI-41 systems only */
	MCS48_DBBO, /* UPI-41 systems only */
	MCS48_DBBI, /* UPI-41 systems only */

	MCS48_GENPC = STATE_GENPC,
	MCS48_GENSP = STATE_GENSP,
	MCS48_GENPCBASE = STATE_GENPCBASE
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
	MCS48_PORT_P0   = 0x100,    /* Not used */
	MCS48_PORT_P1   = 0x101,
	MCS48_PORT_P2   = 0x102,
	MCS48_PORT_T0   = 0x110,
	MCS48_PORT_T1   = 0x111,
	MCS48_PORT_BUS  = 0x120,
	MCS48_PORT_PROG = 0x121     /* PROG line to 8243 expander */
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
    MACROS
***************************************************************************/

#define MCS48_ALE_CLOCK(_clock) \
	attotime::from_hz(_clock/(3*5))

/* Official Intel MCS-48 parts */
DECLARE_LEGACY_CPU_DEVICE(I8021, i8021);            /* 1k internal ROM,      64 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8022, i8022);            /* 2k internal ROM,     128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8035, i8035);            /* external ROM,         64 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8048, i8048);            /* 1k internal ROM,      64 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8648, i8648);            /* 1k internal OTP ROM,  64 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8748, i8748);            /* 1k internal EEPROM,   64 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8039, i8039);            /* external ROM,        128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8049, i8049);            /* 2k internal ROM,     128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8749, i8749);            /* 2k internal EEPROM,  128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8040, i8040);            /* external ROM,        256 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8050, i8050);            /* 4k internal ROM,     256 bytes internal RAM */

/* Official Intel UPI-41 parts */
DECLARE_LEGACY_CPU_DEVICE(I8041, i8041);            /* 1k internal ROM,     128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8741, i8741);            /* 1k internal EEPROM,  128 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8042, i8042);            /* 2k internal ROM,     256 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8242, i8242);            /* 2k internal ROM,     256 bytes internal RAM */
DECLARE_LEGACY_CPU_DEVICE(I8742, i8742);            /* 2k internal EEPROM,  256 bytes internal RAM */

/* Clones */
DECLARE_LEGACY_CPU_DEVICE(MB8884, mb8884);          /* 8035 clone */
DECLARE_LEGACY_CPU_DEVICE(N7751, n7751);            /* 8048 clone */
DECLARE_LEGACY_CPU_DEVICE(M58715, m58715);          /* 8049 clone */



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* functions for talking to the input/output buffers on the UPI41-class chips */
UINT8 upi41_master_r(device_t *device, UINT8 a0);
void upi41_master_w(device_t *device, UINT8 a0, UINT8 data);


/* disassemblers */
CPU_DISASSEMBLE( mcs48 );
CPU_DISASSEMBLE( upi41 );


#endif  /* __MCS48_H__ */
