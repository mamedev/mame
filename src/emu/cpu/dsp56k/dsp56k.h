/***************************************************************************

    dsp56k.h
    Interface file for the portable Motorola/Freescale DSP56k emulator.
    Written by Andrew Gardner

***************************************************************************/


#pragma once

#ifndef __DSP56K_H__
#define __DSP56K_H__

#include "cpuintrf.h"

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/
enum
{
	// PCU
	DSP56K_PC=1,
	DSP56K_SR,
	DSP56K_LC,
	DSP56K_LA,
	DSP56K_SP,
	DSP56K_OMR,

	// ALU
	DSP56K_X, DSP56K_Y,
	DSP56K_A, DSP56K_B,

	// AGU
	DSP56K_R0,DSP56K_R1,DSP56K_R2,DSP56K_R3,
	DSP56K_N0,DSP56K_N1,DSP56K_N2,DSP56K_N3,
	DSP56K_M0,DSP56K_M1,DSP56K_M2,DSP56K_M3,
	DSP56K_TEMP,
	DSP56K_STATUS,

	// CPU STACK
	DSP56K_ST0,
	DSP56K_ST1,
	DSP56K_ST2,
	DSP56K_ST3,
	DSP56K_ST4,
	DSP56K_ST5,
	DSP56K_ST6,
	DSP56K_ST7,
	DSP56K_ST8,
	DSP56K_ST9,
	DSP56K_ST10,
	DSP56K_ST11,
	DSP56K_ST12,
	DSP56K_ST13,
	DSP56K_ST14,
	DSP56K_ST15
};

// IRQ Lines
// MODA and MODB are also known as IRQA and IRQB
#define DSP56K_IRQ_MODA  0
#define DSP56K_IRQ_MODB  1
#define DSP56K_IRQ_MODC  2
#define DSP56K_IRQ_RESET 3	/* Is this needed? */

// Needed for MAME
extern CPU_GET_INFO( dsp56k );
#define CPU_DSP56156 CPU_GET_INFO_NAME( dsp56k )


/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/
// 5-4 Host Interface
typedef struct
{
	// **** Dsp56k side **** //
	// Host Control Register
	UINT16* hcr;

	// Host Status Register
	UINT16* hsr;

	// Host Transmit/Receive Data
	UINT16* htrx;

	// **** Host CPU side **** //
	// Interrupt Control Register
	UINT8 icr;

	// Command Vector Register
	UINT8 cvr;

	// Interrupt Status Register
	UINT8 isr;

	// Interrupt Vector Register
	UINT8 ivr;

	// Transmit / Receive Registers
	UINT8 trxh;
	UINT8 trxl;

	// HACK - Host interface bootstrap write offset
	UINT16 bootstrap_offset;

} dsp56k_host_interface;

// 1-9 ALU
typedef struct
{
	// Four 16-bit input registers (can be accessed as 2 32-bit registers)
	PAIR x;
	PAIR y;

	// Two 32-bit accumulator registers + 8-bit accumulator extension registers
	PAIR64 a;
	PAIR64 b;

	// An accumulation shifter
	// One data bus shifter/limiter
	// A parallel, single cycle, non-pipelined Multiply-Accumulator (MAC) unit
	// Basics
} dsp56k_data_alu;

// 1-10 Address Generation Unit (AGU)
typedef struct
{
	// Four address registers
	UINT16 r0;
	UINT16 r1;
	UINT16 r2;
	UINT16 r3;

	// Four offset registers
	UINT16 n0;
	UINT16 n1;
	UINT16 n2;
	UINT16 n3;

	// Four modifier registers
	UINT16 m0;
	UINT16 m1;
	UINT16 m2;
	UINT16 m3;

	// Used in loop processing
	UINT16 temp;

	// FM.4-5 - hmmm?
	// UINT8 status;

	// Basics
} dsp56k_agu;

// 1-11 Program Control Unit (PCU)
typedef struct
{
	// Program Counter
	UINT16 pc;

	// Loop Address
	UINT16 la;

	// Loop Counter
	UINT16 lc;

	// Status Register
	UINT16 sr;

	// Operating Mode Register
	UINT16 omr;

	// Stack Pointer
	UINT16 sp;

	// Stack (TODO: 15-level?)
	PAIR ss[16];

	// Controls IRQ processing
	void (*service_interrupts)(void);

	// A list of pending interrupts (indices into dsp56k_interrupt_sources array)
	INT8 pending_interrupts[32];

	// Basics

	// Other PCU internals
	UINT16 reset_vector;

} dsp56k_pcu;

// 1-8 The dsp56156 CORE
typedef struct
{
	// PROGRAM CONTROLLER
	dsp56k_pcu PCU;

	// ADR ALU (AGU)
	dsp56k_agu AGU;

	// CLOCK GEN
	//static emu_timer *dsp56k_timer;   // 1-5, 1-8 - Clock gen

	// DATA ALU
	dsp56k_data_alu ALU;

	// OnCE

	// IBS and BITFIELD UNIT

	// Host Interface
	dsp56k_host_interface HI;

	// IRQ line states
	UINT8 modA_state;
	UINT8 modB_state;
	UINT8 modC_state;
	UINT8 reset_state;

	// HACK - Bootstrap mode state variable.
	UINT8 bootstrap_mode;

	UINT8	repFlag;	// Knowing if we're in a 'repeat' state (dunno how the processor does this)
	UINT32	repAddr;	// The address of the instruction to repeat...


	/* MAME internal stuff */
	int icount;

	UINT32			ppc;
	UINT32			op;
	int				interrupt_cycles;
	void			(*output_pins_changed)(UINT32 pins);
	const device_config *device;
	const address_space *program;
	const address_space *data;
} dsp56k_core;


/***************************************************************************
    PUBLIC FUNCTIONS - ACCESSIBLE TO DRIVERS
***************************************************************************/
void  dsp56k_host_interface_write(const device_config* device, UINT8 offset, UINT8 data);
UINT8 dsp56k_host_interface_read(const device_config* device, UINT8 offset);

UINT16 dsp56k_get_peripheral_memory(const device_config* device, UINT16 addr);

#endif /* __DSP56K_H__ */
