/***************************************************************************

    i860.h

    Interface file for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)
    Released for general non-commercial use under the MAME license
    with the additional requirement that you are free to use and
    redistribute this code in modified or unmodified form, provided
    you list me in the credits.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __I860_H__
#define __I860_H__


/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	I860_PC = 1,

	I860_FIR,
	I860_PSR,
	I860_DIRBASE,
	I860_DB,
	I860_FSR,
	I860_EPSR,

	I860_R0,  I860_R1,  I860_R2,  I860_R3,  I860_R4,  I860_R5,  I860_R6,  I860_R7,  I860_R8,  I860_R9,
	I860_R10, I860_R11, I860_R12, I860_R13, I860_R14, I860_R15, I860_R16, I860_R17, I860_R18, I860_R19,
	I860_R20, I860_R21, I860_R22, I860_R23, I860_R24, I860_R25, I860_R26, I860_R27, I860_R28, I860_R29,
	I860_R30, I860_R31,

	I860_F0,  I860_F1,  I860_F2,  I860_F3,  I860_F4,  I860_F5,  I860_F6,  I860_F7,  I860_F8,  I860_F9,
	I860_F10, I860_F11, I860_F12, I860_F13, I860_F14, I860_F15, I860_F16, I860_F17, I860_F18, I860_F19,
	I860_F20, I860_F21, I860_F22, I860_F23, I860_F24, I860_F25, I860_F26, I860_F27, I860_F28, I860_F29,
	I860_F30, I860_F31,

};

/* Needed for MAME */
DECLARE_LEGACY_CPU_DEVICE(I860, i860);


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* i860 state.  */
struct i860_state_t {
	/* Integer registers (32 x 32-bits).  */
	UINT32 iregs[32];

	/* Floating point registers (32 x 32-bits, 16 x 64 bits, or 8 x 128 bits).
	   When referenced as pairs or quads, the higher numbered registers
	   are the upper bits. E.g., double precision f0 is f1:f0.  */
	UINT8 frg[32 * 4];

	/* Control registers (6 x 32-bits).  */
	UINT32 cregs[6];

	/* Program counter (1 x 32-bits).  Reset starts at pc=0xffffff00.  */
	UINT32 pc;

	/* Special registers (4 x 64-bits).  */
	union
	{
		float s;
		double d;
	} KR, KI, T;
	UINT64 merge;

	/* The adder pipeline, always 3 stages.  */
	struct
	{
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Adder result precision (1 = dbl, 0 = sgl).  */
			char arp;
		} stat;
	} A[3];

	/* The multiplier pipeline. 3 stages for single precision, 2 stages
	   for double precision, and confusing for mixed precision.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Multiplier result precision (1 = dbl, 0 = sgl).  */
			char mrp;
		} stat;
	} M[3];

	/* The load pipeline, always 3 stages.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Load result precision (1 = dbl, 0 = sgl).  */
			char lrp;
		} stat;
	} L[3];

	/* The graphics/integer pipeline, always 1 stage.  */
	struct {
		/* The stage contents.  */
		union {
			float s;
			double d;
		} val;

		/* The stage status bits.  */
		struct {
			/* Integer/graphics result precision (1 = dbl, 0 = sgl).  */
			char irp;
		} stat;
	} G;

	/* Pins.  */
	int pin_bus_hold;
	int pin_reset;

	/*
	 * Other emulator state.
	 */
	int exiting_readmem;
	int exiting_ifetch;

	/* Indicate a control-flow instruction, so we know the PC is updated.  */
	int pc_updated;

	/* Indicate an instruction just generated a trap, so we know the PC
	   needs to go to the trap address.  */
	int pending_trap;

	/* This is 1 if the next fir load gets the trap address, otherwise
	   it is 0 to get the ld.c address.  This is set to 1 only when a
	   non-reset trap occurs.  */
	int fir_gets_trap_addr;

	/* Single stepping flag for internal use.  */
	int single_stepping;

	/*
	 * MAME-specific stuff.
	 */
	legacy_cpu_device *device;
	address_space *program;
	UINT32 ppc;
	int icount;

};

INLINE i860_state_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == I860);
	return (i860_state_t *)downcast<legacy_cpu_device *>(device)->token();
}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

/* This is the external interface for asserting an external interrupt
   to the i860.  */
extern void i860_gen_interrupt(i860_state_t*);

/* This is the external interface for asserting/deasserting a pin on
   the i860.  */
extern void i860_set_pin(device_t *, int, int);

/* Hard or soft reset.  */
extern void reset_i860(i860_state_t*);

/* i860 pins.  */
enum {
	DEC_PIN_BUS_HOLD,       /* Bus HOLD pin.      */
	DEC_PIN_RESET           /* System reset pin.  */
};


/*  TODO: THESE WILL BE REPLACED BY MAME FUNCTIONS
#define BYTE_REV32(t)   \
  do { \
    (t) = ((UINT32)(t) >> 16) | ((UINT32)(t) << 16); \
    (t) = (((UINT32)(t) >> 8) & 0x00ff00ff) | (((UINT32)(t) << 8) & 0xff00ff00); \
  } while (0);

#define BYTE_REV16(t)   \
  do { \
    (t) = (((UINT16)(t) >> 8) & 0x00ff) | (((UINT16)(t) << 8) & 0xff00); \
  } while (0);
#endif
*/

#endif /* __I860_H__ */
