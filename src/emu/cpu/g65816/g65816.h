#pragma once

#ifndef __G65816_H__
#define __G65816_H__

#include "cpuintrf.h"
#include "debugger.h"
#include "g65816cm.h"

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright Karl Stenerud
All rights reserved.

Permission is granted to use this source code for non-commercial purposes.
To use this code for commercial purposes, you must get permission from the
author (Karl Stenerud) at karl@higashiyama-unet.ocn.ne.jp.


*/
/* ======================================================================== */
/* ============================= Configuration ============================ */
/* ======================================================================== */

/* GTE Microcircuits G65816 */

/* ======================================================================== */
/* =============================== DEFINES ================================ */
/* ======================================================================== */

/* Interrupt lines - used with g65816_set_irq_line() */
enum
{
	G65816_LINE_NONE,
	G65816_LINE_IRQ,
	G65816_LINE_NMI,
	G65816_LINE_ABORT,
	G65816_LINE_SO,
	G65816_LINE_RDY,
	G65816_LINE_RESET
};

#define G65816_INT_NONE G65816_LINE_NONE
#define G65816_INT_IRQ G65816_LINE_IRQ
#define G65816_INT_NMI G65816_LINE_NMI


/* Registers - used by g65816_set_reg() and g65816_get_reg() */
enum
{
	G65816_PC=1, G65816_S, G65816_P, G65816_A, G65816_X, G65816_Y,
	G65816_PB, G65816_DB, G65816_D, G65816_E,
	G65816_NMI_STATE, G65816_IRQ_STATE
};

enum
{
	CPUINFO_PTR_G65816_READVECTOR_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC
};

/* Main interface function */
CPU_GET_INFO( g65816 );
#define CPU_G65816 CPU_GET_INFO_NAME( g65816 )

#undef G65816_CALL_DEBUGGER
#define G65816_CALL_DEBUGGER(x) debugger_instruction_hook(cpustate->device, x)

#define g65816_read_8(addr) 			memory_read_byte_8be(cpustate->program, addr)
#define g65816_write_8(addr,data)		memory_write_byte_8be(cpustate->program, addr,data)
#define g65816_read_8_immediate(A)		memory_read_byte_8be(cpustate->program, A)
#define g65816_jumping(A)
#define g65816_branching(A)



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __G65816_H__ */
