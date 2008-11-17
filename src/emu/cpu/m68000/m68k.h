#pragma once

#ifndef __M68K_H__
#define __M68K_H__

/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.32
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright Karl Stenerud.  All rights reserved.
 *
 * This code may be freely used for non-commercial purposes as long as this
 * copyright notice remains unaltered in the source code and any binary files
 * containing this code in compiled form.
 *
 * All other licensing terms must be negotiated with the author
 * (Karl Stenerud).
 *
 * The latest version of this code can be obtained at:
 * http://kstenerud.cjb.net
 */

/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */

/* ======================================================================== */

/* CPU types for use in m68k_set_cpu_type() */
enum
{
	M68K_CPU_TYPE_INVALID,
	M68K_CPU_TYPE_68000,
	M68K_CPU_TYPE_68008,
	M68K_CPU_TYPE_68010,
	M68K_CPU_TYPE_68EC020,
	M68K_CPU_TYPE_68020,
	M68K_CPU_TYPE_68030,	/* Supported by disassembler ONLY */
	M68K_CPU_TYPE_68040		/* Supported by disassembler ONLY */
};

/* Registers used by m68k_get_reg() and m68k_set_reg() */
enum _m68k_register_t
{
	/* Real registers */
	M68K_REG_D0,		/* Data registers */
	M68K_REG_D1,
	M68K_REG_D2,
	M68K_REG_D3,
	M68K_REG_D4,
	M68K_REG_D5,
	M68K_REG_D6,
	M68K_REG_D7,
	M68K_REG_A0,		/* Address registers */
	M68K_REG_A1,
	M68K_REG_A2,
	M68K_REG_A3,
	M68K_REG_A4,
	M68K_REG_A5,
	M68K_REG_A6,
	M68K_REG_A7,
	M68K_REG_PC,		/* Program Counter */
	M68K_REG_SR,		/* Status Register */
	M68K_REG_SP,		/* The current Stack Pointer (located in A7) */
	M68K_REG_USP,		/* User Stack Pointer */
	M68K_REG_ISP,		/* Interrupt Stack Pointer */
	M68K_REG_MSP,		/* Master Stack Pointer */
	M68K_REG_SFC,		/* Source Function Code */
	M68K_REG_DFC,		/* Destination Function Code */
	M68K_REG_VBR,		/* Vector Base Register */
	M68K_REG_CACR,		/* Cache Control Register */
	M68K_REG_CAAR,		/* Cache Address Register */

	/* Assumed registers */
	/* These are cheat registers which emulate the 1-longword prefetch
     * present in the 68000 and 68010.
     */
	M68K_REG_PREF_ADDR,	/* Last prefetch address */
	M68K_REG_PREF_DATA,	/* Last prefetch data */

	/* Convenience registers */
	M68K_REG_PPC,		/* Previous value in the program counter */
	M68K_REG_IR,		/* Instruction register */
	M68K_REG_CPU_TYPE	/* Type of CPU being run */
};
typedef enum _m68k_register_t m68k_register_t;



/* ======================================================================== */
/* ============================== CALLBACKS =============================== */
/* ======================================================================== */

/* These functions allow you to set callbacks to the host when specific events
 * occur.  Note that you must enable the corresponding value in m68kconf.h
 * in order for these to do anything useful.
 * Note: I have defined default callbacks which are used if you have enabled
 * the corresponding #define in m68kconf.h but either haven't assigned a
 * callback or have assigned a callback of NULL.
 */

/* Set the callback for an interrupt acknowledge.
 * The CPU will call the callback with the interrupt level being acknowledged.
 * The host program must return either a vector from 0x02-0xff, or one of the
 * special interrupt acknowledge values specified earlier in this header.
 * If this is not implemented, the CPU will always assume an autovectored
 * interrupt, and will automatically clear the interrupt request when it
 * services the interrupt.
 * Default behavior: return M68K_INT_ACK_AUTOVECTOR.
 */
void m68k_set_int_ack_callback(m68ki_cpu_core *m68k, void *param, int  (*callback)(void *param, int int_level));


/* Set the callback for a breakpoint acknowledge (68010+).
 * The CPU will call the callback with whatever was in the data field of the
 * BKPT instruction for 68020+, or 0 for 68010.
 * Default behavior: do nothing.
 */
void m68k_set_bkpt_ack_callback(m68ki_cpu_core *m68k, void (*callback)(unsigned int data));


/* Set the callback for the RESET instruction.
 * The CPU calls this callback every time it encounters a RESET instruction.
 * Default behavior: do nothing.
 */
void m68k_set_reset_instr_callback(m68ki_cpu_core *m68k, void  (*callback)(void));


/* Set the callback for the CMPI.L #v, Dn instruction.
 * The CPU calls this callback every time it encounters a CMPI.L #v, Dn instruction.
 * Default behavior: do nothing.
 */
void m68k_set_cmpild_instr_callback(m68ki_cpu_core *m68k, void  (*callback)(unsigned int val, int reg));


/* Set the callback for the RTE instruction.
 * The CPU calls this callback every time it encounters a RTE instruction.
 * Default behavior: do nothing.
 */
void m68k_set_rte_instr_callback(m68ki_cpu_core *m68k, void  (*callback)(void));

/* Set the callback for the TAS instruction.
 * The CPU calls this callback every time it encounters a TAS instruction.
 * Default behavior: return 1, allow writeback.
 */
void m68k_set_tas_instr_callback(m68ki_cpu_core *m68k, int  (*callback)(void));



/* Set the callback for CPU function code changes.
 * You must enable M68K_EMULATE_FC in m68kconf.h.
 * The CPU calls this callback with the function code before every memory
 * access to set the CPU's function code according to what kind of memory
 * access it is (supervisor/user, program/data and such).
 * Default behavior: do nothing.
 */
void m68k_set_fc_callback(m68ki_cpu_core *m68k, void  (*callback)(unsigned int new_fc));



/* ======================================================================== */
/* ====================== FUNCTIONS TO ACCESS THE CPU ===================== */
/* ======================================================================== */

/* Use this function to set the CPU type you want to emulate.
 * Currently supported types are: M68K_CPU_TYPE_68000, M68K_CPU_TYPE_68008,
 * M68K_CPU_TYPE_68010, M68K_CPU_TYPE_EC020, and M68K_CPU_TYPE_68020.
 */
void m68k_set_cpu_type(m68ki_cpu_core *m68k, unsigned int cpu_type);

/* Do whatever initialisations the core requires.  Should be called
 * at least once at init time.
 */
void m68k_init(m68ki_cpu_core *m68k);

/* Pulse the RESET pin on the CPU.
 * You *MUST* reset the CPU at least once to initialize the emulation
 * Note: If you didn't call m68k_set_cpu_type() before resetting
 *       the CPU for the first time, the CPU will be set to
 *       M68K_CPU_TYPE_68000.
 */
void m68k_pulse_reset(m68ki_cpu_core *m68k);

/* execute num_cycles worth of instructions.  returns number of cycles used */
int m68k_execute(m68ki_cpu_core *m68k, int num_cycles);

/* These functions let you read/write/modify the number of cycles left to run
 * while m68k_execute() is running.
 * These are useful if the 68k accesses a memory-mapped port on another device
 * that requires immediate processing by another CPU.
 */
int m68k_cycles_run(m68ki_cpu_core *m68k);              /* Number of cycles run so far */
int m68k_cycles_remaining(m68ki_cpu_core *m68k);        /* Number of cycles left */
void m68k_modify_timeslice(m68ki_cpu_core *m68k, int cycles); /* Modify cycles left */
void m68k_end_timeslice(m68ki_cpu_core *m68k);          /* End timeslice now */

/* Set the IPL0-IPL2 pins on the CPU (IRQ).
 * A transition from < 7 to 7 will cause a non-maskable interrupt (NMI).
 * Setting IRQ to 0 will clear an interrupt request.
 */
void m68k_set_irq(m68ki_cpu_core *m68k, unsigned int int_level);

/* Set the virtual irq lines, where the highest level
 * active line is automatically selected.  If you use this function,
 * do not use m68k_set_irq.
 */
void m68k_set_virq(m68ki_cpu_core *m68k, unsigned int level, unsigned int active);
unsigned int m68k_get_virq(m68ki_cpu_core *m68k, unsigned int level);

/* Halt the CPU as if you pulsed the HALT pin. */
void m68k_pulse_halt(m68ki_cpu_core *m68k);


/* Context switching to allow multiple CPUs */

/* Get the size of the cpu context in bytes */
unsigned int m68k_context_size(void);

/* Get a cpu context */
unsigned int m68k_get_context(void* dst);

/* set the current cpu context */
void m68k_set_context(void* dst);

/* Register the CPU state information */
void m68k_state_register(m68ki_cpu_core *m68k, const char *type);


/* Peek at the internals of a CPU context.  This can either be a context
 * retrieved using m68k_get_context() or the currently running context.
 * If context is NULL, the currently running CPU context will be used.
 */
unsigned int m68k_get_reg(m68ki_cpu_core *m68k, m68k_register_t regnum);

/* Poke values into the internals of the currently running CPU context */
void m68k_set_reg(m68ki_cpu_core *m68k, m68k_register_t reg, unsigned int value);

/* Check if an instruction is valid for the specified CPU type */
unsigned int m68k_is_valid_instruction(unsigned int instruction, unsigned int cpu_type);

/* Disassemble 1 instruction using the epecified CPU type at pc.  Stores
 * disassembly in str_buff and returns the size of the instruction in bytes.
 */
unsigned int m68k_disassemble(char* str_buff, unsigned int pc, unsigned int cpu_type);

/* Same as above but accepts raw opcode data directly rather than fetching
 * via the read/write interfaces.
 */
unsigned int m68k_disassemble_raw(char* str_buff, unsigned int pc, const unsigned char* opdata, const unsigned char* argdata, unsigned int cpu_type);


/* ======================================================================== */
/* ============================== MAME STUFF ============================== */
/* ======================================================================== */

#include "m68kmame.h"


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __M68K_H__ */
