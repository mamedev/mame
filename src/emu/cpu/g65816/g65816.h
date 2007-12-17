#ifndef HEADER__G65816
#define HEADER__G65816

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright (c) 2000 Karl Stenerud
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



/* ======================================================================== */
/* =================== Functions Implemented by the Host ================== */
/* ======================================================================== */

/* Read data from RAM */
unsigned int g65816_read_8(unsigned int address);

/* Read data from ROM */
unsigned int g65816_read_8_immediate(unsigned int address);

/* Write data to RAM */
void g65816_write_8(unsigned int address, unsigned int value);

/* Notification of PC changes */
void g65816_jumping(unsigned int new_pc);
void g65816_branching(unsigned int new_pc);



/* ======================================================================== */
/* ================================= MAME ================================= */
/* ======================================================================== */

#include "cpuintrf.h"
#include "debugger.h"

enum
{
	CPUINFO_PTR_G65816_READVECTOR_CALLBACK = CPUINFO_PTR_CPU_SPECIFIC
};

/* Main interface function */
void g65816_get_info(UINT32 state, cpuinfo *info);

#undef G65816_CALL_DEBUGGER
#define G65816_CALL_DEBUGGER CALL_MAME_DEBUG

#define g65816_read_8(addr) 			program_read_byte_8(addr)
#define g65816_write_8(addr,data)		program_write_byte_8(addr,data)
#define g65816_read_8_immediate(A)		program_read_byte_8(A)
#define g65816_jumping(A)				change_pc(A)
#define g65816_branching(A)



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* HEADER__G65816 */
