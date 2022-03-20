// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */

#if 0
static const char copyright_notice[] =
"MUSASHI\n"
"Version 4.95 (2012-02-19)\n"
"A portable Motorola M68xxx/CPU32/ColdFire processor emulation engine.\n"
"Copyright Karl Stenerud.  All rights reserved.\n"
;
#endif


/* ======================================================================== */
/* ================================= NOTES ================================ */
/* ======================================================================== */



/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include "emu.h"
#include "debugger.h"
#include "m68000.h"
#include "m68kdasm.h"

// Generated data

u16 m68000_base_device::m68ki_instruction_state_table[NUM_CPU_TYPES][0x10000]; /* opcode handler jump table */
unsigned char m68000_base_device::m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* Used by shift & rotate instructions */
const u8 m68000_base_device::m68ki_shift_8_table[65] =
{
	0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff
};
const u16 m68000_base_device::m68ki_shift_16_table[65] =
{
	0x0000, 0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
	0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff
};
const u32 m68000_base_device::m68ki_shift_32_table[65] =
{
	0x00000000, 0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 0xf8000000,
	0xfc000000, 0xfe000000, 0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
	0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 0xffff8000,
	0xffffc000, 0xffffe000, 0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
	0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 0xfffffff8,
	0xfffffffc, 0xfffffffe, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
};


/* Number of clock cycles to use for exception processing.
 * I used 4 for any vectors that are undocumented for processing times.
 */
const u8 m68000_base_device::m68ki_exception_cycle_table[NUM_CPU_TYPES][256] =
{
	{ /* 000 */
			40, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			34, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			34, /*  9: Trace                                              */
			4, /* 10: 1010                                               */
			4, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			44, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			44, /* 24: Spurious Interrupt                                 */
			44, /* 25: Level 1 Interrupt Autovector                       */
			44, /* 26: Level 2 Interrupt Autovector                       */
			44, /* 27: Level 3 Interrupt Autovector                       */
			44, /* 28: Level 4 Interrupt Autovector                       */
			44, /* 29: Level 5 Interrupt Autovector                       */
			44, /* 30: Level 6 Interrupt Autovector                       */
			44, /* 31: Level 7 Interrupt Autovector                       */
			34, /* 32: TRAP #0                                            */
			34, /* 33: TRAP #1                                            */
			34, /* 34: TRAP #2                                            */
			34, /* 35: TRAP #3                                            */
			34, /* 36: TRAP #4                                            */
			34, /* 37: TRAP #5                                            */
			34, /* 38: TRAP #6                                            */
			34, /* 39: TRAP #7                                            */
			34, /* 40: TRAP #8                                            */
			34, /* 41: TRAP #9                                            */
			34, /* 42: TRAP #10                                           */
			34, /* 43: TRAP #11                                           */
			34, /* 44: TRAP #12                                           */
			34, /* 45: TRAP #13                                           */
			34, /* 46: TRAP #14                                           */
			34, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 070 - not even pretending to be correct */
			40, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
		126, /*  2: Bus Error                             (unemulated) */
		126, /*  3: Address Error                         (unemulated) */
			38, /*  4: Illegal Instruction                                */
			44, /*  5: Divide by Zero                                     */
			44, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			38, /*  8: Privilege Violation                                */
			38, /*  9: Trace                                              */
			4, /* 10: 1010                                               */
			4, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			44, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			46, /* 24: Spurious Interrupt                                 */
			46, /* 25: Level 1 Interrupt Autovector                       */
			46, /* 26: Level 2 Interrupt Autovector                       */
			46, /* 27: Level 3 Interrupt Autovector                       */
			46, /* 28: Level 4 Interrupt Autovector                       */
			46, /* 29: Level 5 Interrupt Autovector                       */
			46, /* 30: Level 6 Interrupt Autovector                       */
			46, /* 31: Level 7 Interrupt Autovector                       */
			38, /* 32: TRAP #0                                            */
			38, /* 33: TRAP #1                                            */
			38, /* 34: TRAP #2                                            */
			38, /* 35: TRAP #3                                            */
			38, /* 36: TRAP #4                                            */
			38, /* 37: TRAP #5                                            */
			38, /* 38: TRAP #6                                            */
			38, /* 39: TRAP #7                                            */
			38, /* 40: TRAP #8                                            */
			38, /* 41: TRAP #9                                            */
			38, /* 42: TRAP #10                                           */
			38, /* 43: TRAP #11                                           */
			38, /* 44: TRAP #12                                           */
			38, /* 45: TRAP #13                                           */
			38, /* 46: TRAP #14                                           */
			38, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 010 */
			40, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
		126, /*  2: Bus Error                             (unemulated) */
		126, /*  3: Address Error                         (unemulated) */
			38, /*  4: Illegal Instruction                                */
			44, /*  5: Divide by Zero                                     */
			44, /*  6: CHK                                                */
			34, /*  7: TRAPV                                              */
			38, /*  8: Privilege Violation                                */
			38, /*  9: Trace                                              */
			4, /* 10: 1010                                               */
			4, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			44, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			46, /* 24: Spurious Interrupt                                 */
			46, /* 25: Level 1 Interrupt Autovector                       */
			46, /* 26: Level 2 Interrupt Autovector                       */
			46, /* 27: Level 3 Interrupt Autovector                       */
			46, /* 28: Level 4 Interrupt Autovector                       */
			46, /* 29: Level 5 Interrupt Autovector                       */
			46, /* 30: Level 6 Interrupt Autovector                       */
			46, /* 31: Level 7 Interrupt Autovector                       */
			38, /* 32: TRAP #0                                            */
			38, /* 33: TRAP #1                                            */
			38, /* 34: TRAP #2                                            */
			38, /* 35: TRAP #3                                            */
			38, /* 36: TRAP #4                                            */
			38, /* 37: TRAP #5                                            */
			38, /* 38: TRAP #6                                            */
			38, /* 39: TRAP #7                                            */
			38, /* 40: TRAP #8                                            */
			38, /* 41: TRAP #9                                            */
			38, /* 42: TRAP #10                                           */
			38, /* 43: TRAP #11                                           */
			38, /* 44: TRAP #12                                           */
			38, /* 45: TRAP #13                                           */
			38, /* 46: TRAP #14                                           */
			38, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 020 */
			4, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			30, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			30, /* 24: Spurious Interrupt                                 */
			30, /* 25: Level 1 Interrupt Autovector                       */
			30, /* 26: Level 2 Interrupt Autovector                       */
			30, /* 27: Level 3 Interrupt Autovector                       */
			30, /* 28: Level 4 Interrupt Autovector                       */
			30, /* 29: Level 5 Interrupt Autovector                       */
			30, /* 30: Level 6 Interrupt Autovector                       */
			30, /* 31: Level 7 Interrupt Autovector                       */
			20, /* 32: TRAP #0                                            */
			20, /* 33: TRAP #1                                            */
			20, /* 34: TRAP #2                                            */
			20, /* 35: TRAP #3                                            */
			20, /* 36: TRAP #4                                            */
			20, /* 37: TRAP #5                                            */
			20, /* 38: TRAP #6                                            */
			20, /* 39: TRAP #7                                            */
			20, /* 40: TRAP #8                                            */
			20, /* 41: TRAP #9                                            */
			20, /* 42: TRAP #10                                           */
			20, /* 43: TRAP #11                                           */
			20, /* 44: TRAP #12                                           */
			20, /* 45: TRAP #13                                           */
			20, /* 46: TRAP #14                                           */
			20, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 030 - not correct */
			4, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			30, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			30, /* 24: Spurious Interrupt                                 */
			30, /* 25: Level 1 Interrupt Autovector                       */
			30, /* 26: Level 2 Interrupt Autovector                       */
			30, /* 27: Level 3 Interrupt Autovector                       */
			30, /* 28: Level 4 Interrupt Autovector                       */
			30, /* 29: Level 5 Interrupt Autovector                       */
			30, /* 30: Level 6 Interrupt Autovector                       */
			30, /* 31: Level 7 Interrupt Autovector                       */
			20, /* 32: TRAP #0                                            */
			20, /* 33: TRAP #1                                            */
			20, /* 34: TRAP #2                                            */
			20, /* 35: TRAP #3                                            */
			20, /* 36: TRAP #4                                            */
			20, /* 37: TRAP #5                                            */
			20, /* 38: TRAP #6                                            */
			20, /* 39: TRAP #7                                            */
			20, /* 40: TRAP #8                                            */
			20, /* 41: TRAP #9                                            */
			20, /* 42: TRAP #10                                           */
			20, /* 43: TRAP #11                                           */
			20, /* 44: TRAP #12                                           */
			20, /* 45: TRAP #13                                           */
			20, /* 46: TRAP #14                                           */
			20, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* 040 */ // TODO: these values are not correct
			4, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			30, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			30, /* 24: Spurious Interrupt                                 */
			30, /* 25: Level 1 Interrupt Autovector                       */
			30, /* 26: Level 2 Interrupt Autovector                       */
			30, /* 27: Level 3 Interrupt Autovector                       */
			30, /* 28: Level 4 Interrupt Autovector                       */
			30, /* 29: Level 5 Interrupt Autovector                       */
			30, /* 30: Level 6 Interrupt Autovector                       */
			30, /* 31: Level 7 Interrupt Autovector                       */
			20, /* 32: TRAP #0                                            */
			20, /* 33: TRAP #1                                            */
			20, /* 34: TRAP #2                                            */
			20, /* 35: TRAP #3                                            */
			20, /* 36: TRAP #4                                            */
			20, /* 37: TRAP #5                                            */
			20, /* 38: TRAP #6                                            */
			20, /* 39: TRAP #7                                            */
			20, /* 40: TRAP #8                                            */
			20, /* 41: TRAP #9                                            */
			20, /* 42: TRAP #10                                           */
			20, /* 43: TRAP #11                                           */
			20, /* 44: TRAP #12                                           */
			20, /* 45: TRAP #13                                           */
			20, /* 46: TRAP #14                                           */
			20, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* CPU32 */
			4, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			30, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			30, /* 24: Spurious Interrupt                                 */
			30, /* 25: Level 1 Interrupt Autovector                       */
			30, /* 26: Level 2 Interrupt Autovector                       */
			30, /* 27: Level 3 Interrupt Autovector                       */
			30, /* 28: Level 4 Interrupt Autovector                       */
			30, /* 29: Level 5 Interrupt Autovector                       */
			30, /* 30: Level 6 Interrupt Autovector                       */
			30, /* 31: Level 7 Interrupt Autovector                       */
			20, /* 32: TRAP #0                                            */
			20, /* 33: TRAP #1                                            */
			20, /* 34: TRAP #2                                            */
			20, /* 35: TRAP #3                                            */
			20, /* 36: TRAP #4                                            */
			20, /* 37: TRAP #5                                            */
			20, /* 38: TRAP #6                                            */
			20, /* 39: TRAP #7                                            */
			20, /* 40: TRAP #8                                            */
			20, /* 41: TRAP #9                                            */
			20, /* 42: TRAP #10                                           */
			20, /* 43: TRAP #11                                           */
			20, /* 44: TRAP #12                                           */
			20, /* 45: TRAP #13                                           */
			20, /* 46: TRAP #14                                           */
			20, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
	{ /* ColdFire - not correct */
			4, /*  0: Reset - Initial Stack Pointer                      */
			4, /*  1: Reset - Initial Program Counter                    */
			50, /*  2: Bus Error                             (unemulated) */
			50, /*  3: Address Error                         (unemulated) */
			20, /*  4: Illegal Instruction                                */
			38, /*  5: Divide by Zero                                     */
			40, /*  6: CHK                                                */
			20, /*  7: TRAPV                                              */
			34, /*  8: Privilege Violation                                */
			25, /*  9: Trace                                              */
			20, /* 10: 1010                                               */
			20, /* 11: 1111                                               */
			4, /* 12: RESERVED                                           */
			4, /* 13: Coprocessor Protocol Violation        (unemulated) */
			4, /* 14: Format Error                                       */
			30, /* 15: Uninitialized Interrupt                            */
			4, /* 16: RESERVED                                           */
			4, /* 17: RESERVED                                           */
			4, /* 18: RESERVED                                           */
			4, /* 19: RESERVED                                           */
			4, /* 20: RESERVED                                           */
			4, /* 21: RESERVED                                           */
			4, /* 22: RESERVED                                           */
			4, /* 23: RESERVED                                           */
			30, /* 24: Spurious Interrupt                                 */
			30, /* 25: Level 1 Interrupt Autovector                       */
			30, /* 26: Level 2 Interrupt Autovector                       */
			30, /* 27: Level 3 Interrupt Autovector                       */
			30, /* 28: Level 4 Interrupt Autovector                       */
			30, /* 29: Level 5 Interrupt Autovector                       */
			30, /* 30: Level 6 Interrupt Autovector                       */
			30, /* 31: Level 7 Interrupt Autovector                       */
			20, /* 32: TRAP #0                                            */
			20, /* 33: TRAP #1                                            */
			20, /* 34: TRAP #2                                            */
			20, /* 35: TRAP #3                                            */
			20, /* 36: TRAP #4                                            */
			20, /* 37: TRAP #5                                            */
			20, /* 38: TRAP #6                                            */
			20, /* 39: TRAP #7                                            */
			20, /* 40: TRAP #8                                            */
			20, /* 41: TRAP #9                                            */
			20, /* 42: TRAP #10                                           */
			20, /* 43: TRAP #11                                           */
			20, /* 44: TRAP #12                                           */
			20, /* 45: TRAP #13                                           */
			20, /* 46: TRAP #14                                           */
			20, /* 47: TRAP #15                                           */
			4, /* 48: FP Branch or Set on Unknown Condition (unemulated) */
			4, /* 49: FP Inexact Result                     (unemulated) */
			4, /* 50: FP Divide by Zero                     (unemulated) */
			4, /* 51: FP Underflow                          (unemulated) */
			4, /* 52: FP Operand Error                      (unemulated) */
			4, /* 53: FP Overflow                           (unemulated) */
			4, /* 54: FP Signaling NAN                      (unemulated) */
			4, /* 55: FP Unimplemented Data Type            (unemulated) */
			4, /* 56: MMU Configuration Error               (unemulated) */
			4, /* 57: MMU Illegal Operation Error           (unemulated) */
			4, /* 58: MMU Access Level Violation Error      (unemulated) */
			4, /* 59: RESERVED                                           */
			4, /* 60: RESERVED                                           */
			4, /* 61: RESERVED                                           */
			4, /* 62: RESERVED                                           */
			4, /* 63: RESERVED                                           */
				/* 64-255: User Defined                                   */
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
			4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
	},
};

const u8 m68000_base_device::m68ki_ea_idx_cycle_table[64] =
{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, /* ..01.000 no memory indirect, base nullptr             */
		5, /* ..01..01 memory indirect,    base nullptr, outer nullptr */
		7, /* ..01..10 memory indirect,    base nullptr, outer 16   */
		7, /* ..01..11 memory indirect,    base nullptr, outer 32   */
		0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,
		2, /* ..10.000 no memory indirect, base 16               */
		7, /* ..10..01 memory indirect,    base 16,   outer nullptr */
		9, /* ..10..10 memory indirect,    base 16,   outer 16   */
		9, /* ..10..11 memory indirect,    base 16,   outer 32   */
		0,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,
		6, /* ..11.000 no memory indirect, base 32               */
	11, /* ..11..01 memory indirect,    base 32,   outer nullptr */
	13, /* ..11..10 memory indirect,    base 32,   outer 16   */
	13, /* ..11..11 memory indirect,    base 32,   outer 32   */
		0, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13
};



/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define MASK_ALL                (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 | CPU_TYPE_SCC070 )
#define MASK_24BIT_SPACE            (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020)
#define MASK_32BIT_SPACE            (CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 )
#define MASK_010_OR_LATER           (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 )
#define MASK_020_OR_LATER           (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 )
#define MASK_030_OR_LATER           (CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040)
#define MASK_040_OR_LATER           (CPU_TYPE_040 | CPU_TYPE_EC040)



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

void m68000_base_device::set_irq_line(int irqline, int state)
{
	u32 old_level = m_int_level;
	u32 vstate = m_virq_state;
	u32 blevel;

	if(state == ASSERT_LINE)
		vstate |= 1 << irqline;
	else
		vstate &= ~(1 << irqline);
	m_virq_state = vstate;

	if(m_interrupt_mixer) {
		for(blevel = 7; blevel > 0; blevel--)
			if(vstate & (1 << blevel))
				break;
	} else
		blevel = vstate;

	m_int_level = blevel << 8;

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	// FIXME: This may cause unintended level 7 interrupts if one or two IPL lines are asserted
	// immediately before others are cleared. The actual 68000 imposes an input hold time.
	if(old_level != 0x0700 && m_int_level == 0x0700)
		m_nmi_pending = true;
}

void m68000_base_device::device_pre_save()
{
	m_save_sr = m68ki_get_sr();
	m_save_stopped = (m_stopped & STOP_LEVEL_STOP) != 0;
	m_save_halted  = (m_stopped & STOP_LEVEL_HALT) != 0;
}

void m68000_base_device::device_post_load()
{
	m68ki_set_sr_noint_nosp(m_save_sr);
	//fprintf(stderr, "Reloaded, pc=%x\n", REG_PC(m68k));
	m_stopped = (m_save_stopped ? STOP_LEVEL_STOP : 0) | (m_save_halted  ? STOP_LEVEL_HALT : 0);
	m68ki_jump(m_pc);
}

void m68000_base_device::m68k_cause_bus_error()
{
	m_mmu_tmp_buserror_fc = m_mmu_tmp_fc;
	m_mmu_tmp_buserror_rw = m_mmu_tmp_rw;
	m_mmu_tmp_buserror_sz = m_mmu_tmp_sz;

	// Halt the cpu on berr when writing the stack frame.
	if (m_run_mode == RUN_MODE_BERR_AERR_RESET_WSF)
	{
		m_stopped = STOP_LEVEL_HALT;
		return;
	}

	u32 sr = m68ki_init_exception(EXCEPTION_BUS_ERROR);

	m_run_mode = RUN_MODE_BERR_AERR_RESET_WSF;

	if (CPU_TYPE_IS_000())
	{
		/* Note: This is implemented for 68000 only! */
		m68ki_stack_frame_buserr(sr);
	}
	else if (CPU_TYPE_IS_010())
	{
		/* only the 68010 throws this unique type-1000 frame */
		m68ki_stack_frame_1000(m_ppc, sr, EXCEPTION_BUS_ERROR);
	}
	else if (CPU_TYPE_IS_070())
	{
		/* only the 68070 throws this unique type-1111 frame */
		m68ki_stack_frame_1111(m_ppc, sr, EXCEPTION_BUS_ERROR);
	}
	else if (m_mmu_tmp_buserror_address == m_ppc)
	{
		m68ki_stack_frame_1010(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
	}
	else
	{
		m68ki_stack_frame_1011(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
	}

	m68ki_jump_vector(EXCEPTION_BUS_ERROR);
	m_run_mode = RUN_MODE_BERR_AERR_RESET;
}

bool m68000_base_device::memory_translate(int space, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	{
		/* 68040 needs to call the MMU even when disabled so transparent translation works */
		if ((space == AS_PROGRAM) && ((m_pmmu_enabled) || (CPU_TYPE_IS_040_PLUS())))
		{
			// FIXME: m_mmu_tmp_sr will be overwritten in pmmu_translate_addr_with_fc
			u16 temp_mmu_tmp_sr = m_mmu_tmp_sr;
			int mode = m_s_flag ? FUNCTION_CODE_SUPERVISOR_PROGRAM : FUNCTION_CODE_USER_PROGRAM;
//          u32 va=address;

			if (CPU_TYPE_IS_040_PLUS())
			{
				address = pmmu_translate_addr_with_fc_040(address, mode, 1);
			}
			else
			{
				address = pmmu_translate_addr_with_fc<false, false>(address, mode, 1);
			}

			if ((m_mmu_tmp_sr & M68K_MMU_SR_INVALID) != 0) {
//              logerror("cpu_translate_m68k failed with mmu_sr=%04x va=%08x pa=%08x\n",m_mmu_tmp_sr,va ,address);
				address = 0;
			}

			m_mmu_tmp_sr = temp_mmu_tmp_sr;
		}
	}
	return true;
}









void m68000_base_device::execute_run()
{
	m_initial_cycles = m_icount;

	if (m_reset_cycles) {
		/* Read the initial stack pointer and program counter */
		REG_SP() = m68ki_read_imm_32();
		m_pc = m68ki_read_imm_32();
		m68ki_jump(m_pc);

		/* eat up any reset cycles */
		int rc = m_reset_cycles;
		m_reset_cycles = 0;
		m_icount -= rc;

		if (m_icount <= 0) return;
	}

	/* See if interrupts came in */
	m68ki_check_interrupts();

	/* Make sure we're not stopped */
	if(!m_stopped)
	{
		/* Return point if we had an address error */
		check_address_error:
		if (m_address_error==1)
		{
			m_address_error = 0;
			try {
				m68ki_exception_address_error();
			}
			catch(int error)
			{
				if (error==10)
				{
					m_address_error = 1;
					m_ppc = m_pc;
					goto check_address_error;
				}
				else
					throw;
			}
			if(m_stopped)
			{
				if (m_icount > 0)
					m_icount = 0;
				return;
			}
		}


		/* Main loop.  Keep going until we run out of clock cycles */
		while (m_icount > 0)
		{
			/* Set tracing accodring to T1. (T0 is done inside instruction) */
			m68ki_trace_t1(); /* auto-disable (see m68kcpu.h) */

			/* Record previous program counter */
			m_ppc = m_pc;

			/* Call external hook to peek at CPU */
			debugger_instruction_hook(m_pc);

			try
			{
			if (!m_pmmu_enabled)
			{
				m_run_mode = RUN_MODE_NORMAL;
				/* Read an instruction and call its handler */
				m_ir = m68ki_read_imm_16();
				u16 state = m_state_table[m_ir];
				(this->*m68k_handler_table[state])();
				m_icount -= m_cyc_instruction[m_ir];
			}
			else
			{
				m_run_mode = RUN_MODE_NORMAL;
				// save CPU address registers values at start of instruction
				int i;
				u32 tmp_dar[16];

				for (i = 15; i >= 0; i--)
				{
					tmp_dar[i] = REG_DA()[i];
				}

				m_mmu_tmp_buserror_occurred = 0;

				/* Read an instruction and call its handler */
				m_ir = m68ki_read_imm_16();

				if (!m_mmu_tmp_buserror_occurred)
				{
					u16 state = m_state_table[m_ir];
					(this->*m68k_handler_table[state])();
					m_icount -= m_cyc_instruction[m_ir];
				}

				if (m_mmu_tmp_buserror_occurred)
				{
					u32 sr;

					m_mmu_tmp_buserror_occurred = 0;

					// restore cpu address registers to value at start of instruction
					for (i = 15; i >= 0; i--)
					{
						if (REG_DA()[i] != tmp_dar[i])
						{
//                          logerror("PMMU: pc=%08x sp=%08x bus error: fixed %s[%d]: %08x -> %08x\n",
//                                  m_ppc, REG_A()[7], i < 8 ? "D" : "A", i & 7, REG_DA()[i], tmp_dar[i]);
							REG_DA()[i] = tmp_dar[i];
						}
					}

					sr = m68ki_init_exception(EXCEPTION_BUS_ERROR);

					m_run_mode = RUN_MODE_BERR_AERR_RESET;

					if (!CPU_TYPE_IS_020_PLUS())
					{
						if (CPU_TYPE_IS_010())
						{
							m68ki_stack_frame_1000(m_ppc, sr, EXCEPTION_BUS_ERROR);
						}
						else
						{
							/* Note: This is implemented for 68000 only! */
							m68ki_stack_frame_buserr(sr);
						}
					}
					else if(!CPU_TYPE_IS_040_PLUS()) {
						if (m_mmu_tmp_buserror_address == m_ppc)
						{
							m68ki_stack_frame_1010(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
						}
						else
						{
							m68ki_stack_frame_1011(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address);
						}
					}
					else
					{
						m68ki_stack_frame_0111(sr, EXCEPTION_BUS_ERROR, m_ppc, m_mmu_tmp_buserror_address, true);
					}

					m68ki_jump_vector(EXCEPTION_BUS_ERROR);

					// TODO:
					/* Use up some clock cycles and undo the instruction's cycles */
					// m_icount -= m_cyc_exception[EXCEPTION_BUS_ERROR] - m_cyc_instruction[m_ir];
				}
			}
			}
			catch (int error)
			{
				if (error==10)
				{
					m_address_error = 1;
					goto check_address_error;
				}
				else
					throw;
			}


			/* Trace m68k_exception, if necessary */
			m68ki_exception_if_trace(); /* auto-disable (see m68kcpu.h) */
		}

		/* set previous PC to current PC for the next entry into the loop */
		m_ppc = m_pc;
	}
	else if (m_icount > 0)
		m_icount = 0;
}



void m68000_base_device::init_cpu_common(void)
{
	static u32 emulation_initialized = 0;

	//this = device;//deviceparam;
	m_program = &space(AS_PROGRAM);
	m_oprogram = has_space(AS_OPCODES) ? &space(AS_OPCODES) : m_program;
	m_cpu_space = &space(m_cpu_space_id);

	/* disable all MMUs */
	m_has_pmmu         = 0;
	m_has_hmmu         = 0;
	m_pmmu_enabled     = 0;
	m_hmmu_enabled     = 0;

	/* The first call to this function initializes the opcode handler jump table */
	if(!emulation_initialized)
	{
		m68ki_build_opcode_table();
		emulation_initialized = 1;
	}

	/* Note, D covers A because the dar array is common, REG_A(m68k)=REG_D(m68k)+8 */
	save_item(NAME(REG_D()));
	save_item(NAME(m_ppc));
	save_item(NAME(m_pc));
	save_item(NAME(REG_USP()));
	save_item(NAME(REG_ISP()));
	save_item(NAME(REG_MSP()));
	save_item(NAME(m_vbr));
	save_item(NAME(m_sfc));
	save_item(NAME(m_dfc));
	save_item(NAME(m_cacr));
	save_item(NAME(m_caar));
	save_item(NAME(m_save_sr));
	save_item(NAME(m_int_level));
	save_item(NAME(m_save_stopped));
	save_item(NAME(m_save_halted));
	save_item(NAME(m_pref_addr));
	save_item(NAME(m_pref_data));
	save_item(NAME(m_reset_cycles));
	save_item(NAME(m_virq_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_has_pmmu));
	save_item(NAME(m_has_hmmu));
	save_item(NAME(m_pmmu_enabled));
	save_item(NAME(m_hmmu_enabled));

	save_item(NAME(m_mmu_crp_aptr));
	save_item(NAME(m_mmu_crp_limit));
	save_item(NAME(m_mmu_srp_aptr));
	save_item(NAME(m_mmu_srp_limit));
	save_item(NAME(m_mmu_urp_aptr));
	save_item(NAME(m_mmu_tc));
	save_item(NAME(m_mmu_sr));
	save_item(NAME(m_mmu_sr_040));
	save_item(NAME(m_mmu_atc_rr));
	save_item(NAME(m_mmu_tt0));
	save_item(NAME(m_mmu_tt1));
	save_item(NAME(m_mmu_itt0));
	save_item(NAME(m_mmu_itt1));
	save_item(NAME(m_mmu_dtt0));
	save_item(NAME(m_mmu_dtt1));
	save_item(NAME(m_mmu_acr0));
	save_item(NAME(m_mmu_acr1));
	save_item(NAME(m_mmu_acr2));
	save_item(NAME(m_mmu_acr3));
	save_item(NAME(m_mmu_last_page_entry));
	save_item(NAME(m_mmu_last_page_entry_addr));

	save_item(NAME(m_mmu_atc_tag));
	save_item(NAME(m_mmu_atc_data));

	set_icountptr(m_icount);
	m_icount = 0;

}

void m68000_base_device::device_reset()
{
	/* Disable the PMMU/HMMU on reset, if any */
	m_pmmu_enabled = 0;
	m_hmmu_enabled = 0;

	m_mmu_tc = 0;
	m_mmu_tt0 = 0;
	m_mmu_tt1 = 0;

	/* Clear all stop levels and eat up all remaining cycles */
	m_stopped = 0;
	if (m_icount > 0)
		m_icount = 0;

	m_run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Turn off tracing */
	m_t1_flag = m_t0_flag = 0;
	m68ki_clear_trace();
	/* Interrupt mask to level 7 */
	m_int_mask = 0x0700;
	/* Reset VBR */
	m_vbr = 0;
	/* Go to supervisor mode */
	m68ki_set_sm_flag(SFLAG_SET | MFLAG_CLEAR);

	/* Invalidate the prefetch queue */
	/* Set to arbitrary number since our first fetch is from 0 */
	m_pref_addr = 0x1000;

	m68ki_jump(0);
	m_run_mode = RUN_MODE_NORMAL;

	m_reset_cycles = m_cyc_exception[EXCEPTION_RESET];

	/* flush the MMU's cache */
	pmmu_atc_flush();

	if(CPU_TYPE_IS_EC020_PLUS())
	{
		// clear instruction cache
		m68ki_ic_clear();
	}
}



/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void m68000_base_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENPC:
			m_ppc = m_pc;
			break;

		case STATE_GENPCBASE:
			m_pc = m_ppc;
			break;

		case M68K_SR:
		case STATE_GENFLAGS:
			m68ki_set_sr(m_iotemp);
			break;

		case M68K_ISP:
			if (m_s_flag && !m_m_flag)
				REG_SP() = m_iotemp;
			else
				REG_ISP() = m_iotemp;
			break;

		case M68K_USP:
			if (!m_s_flag)
				REG_SP() = m_iotemp;
			else
				REG_USP() = m_iotemp;
			break;

		case M68K_MSP:
			if (m_s_flag && m_m_flag)
				REG_SP() = m_iotemp;
			else
				REG_MSP() = m_iotemp;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(this) called for unexpected value\n");
	}

}



void m68000_base_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case M68K_SR:
		case STATE_GENFLAGS:
			m_iotemp = m68ki_get_sr();
			break;

		case M68K_ISP:
			m_iotemp = (m_s_flag && !m_m_flag) ? REG_SP() : REG_ISP();
			break;

		case M68K_USP:
			m_iotemp = (!m_s_flag) ? REG_SP() : REG_USP();
			break;

		case M68K_MSP:
			m_iotemp = (m_s_flag && m_m_flag) ? REG_SP() : REG_MSP();
			break;

		case M68K_FP0:
		case M68K_FP1:
		case M68K_FP2:
		case M68K_FP3:
		case M68K_FP4:
		case M68K_FP5:
		case M68K_FP6:
		case M68K_FP7:
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(this) called for unexpected value\n");
	}
}

void m68000_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	u16 sr;

	switch (entry.index())
	{
		case M68K_FP0:
			str = string_format("%f", fx80_to_double(m_fpr[0]));
			break;

		case M68K_FP1:
			str = string_format("%f", fx80_to_double(m_fpr[1]));
			break;

		case M68K_FP2:
			str = string_format("%f", fx80_to_double(m_fpr[2]));
			break;

		case M68K_FP3:
			str = string_format("%f", fx80_to_double(m_fpr[3]));
			break;

		case M68K_FP4:
			str = string_format("%f", fx80_to_double(m_fpr[4]));
			break;

		case M68K_FP5:
			str = string_format("%f", fx80_to_double(m_fpr[5]));
			break;

		case M68K_FP6:
			str = string_format("%f", fx80_to_double(m_fpr[6]));
			break;

		case M68K_FP7:
			str = string_format("%f", fx80_to_double(m_fpr[7]));
			break;

		case STATE_GENFLAGS:
			sr = m68ki_get_sr();
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? 't':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? 'M':'.',
				sr & 0x0800 ? '?':'.',
				sr & 0x0400 ? 'I':'.',
				sr & 0x0200 ? 'I':'.',
				sr & 0x0100 ? 'I':'.',
				sr & 0x0080 ? '?':'.',
				sr & 0x0040 ? '?':'.',
				sr & 0x0020 ? '?':'.',
				sr & 0x0010 ? 'X':'.',
				sr & 0x0008 ? 'N':'.',
				sr & 0x0004 ? 'Z':'.',
				sr & 0x0002 ? 'V':'.',
				sr & 0x0001 ? 'C':'.');
			break;
	}

}


/* global access */

void m68000_base_device::set_hmmu_enable(int enable)
{
	m_hmmu_enabled = enable;
}

void m68000_base_device::set_fpu_enable(int enable)
{
	m_has_fpu = enable;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

void m68000_base_device::init8(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_ospace = &ospace;
	ospace.cache(m_oprogram8);
	space.specific(m_program8);

	m_readimm16 = [this](offs_t address) -> u16  { return m_oprogram8.read_word(address); };
	m_read8   = [this](offs_t address) -> u8     { return m_program8.read_byte(address); };
	m_read16  = [this](offs_t address) -> u16    { return m_program8.read_word(address); };
	m_read32  = [this](offs_t address) -> u32    { return m_program8.read_dword(address); };
	m_write8  = [this](offs_t address, u8 data)  { m_program8.write_byte(address, data); };
	m_write16 = [this](offs_t address, u16 data) { m_program8.write_word(address, data); };
	m_write32 = [this](offs_t address, u32 data) { m_program8.write_dword(address, data); };
}

/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

void m68000_base_device::init16(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_ospace = &ospace;
	ospace.cache(m_oprogram16);
	space.specific(m_program16);

	m_readimm16 = [this](offs_t address) -> u16  { return m_oprogram16.read_word(address); };
	m_read8   = [this](offs_t address) -> u8     { return m_program16.read_byte(address); };
	m_read16  = [this](offs_t address) -> u16    { return m_program16.read_word(address); };
	m_read32  = [this](offs_t address) -> u32    { return m_program16.read_dword(address); };
	m_write8  = [this](offs_t address, u8 data)  { m_program16.write_word(address & ~1, data | (data << 8), address & 1 ? 0x00ff : 0xff00); };
	m_write16 = [this](offs_t address, u16 data) { m_program16.write_word(address, data); };
	m_write32 = [this](offs_t address, u32 data) { m_program16.write_dword(address, data); };
}

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

static inline u32 dword_from_byte(u8 data) { return data * 0x01010101U; }
static inline u32 dword_from_word(u16 data) { return data * 0x00010001U; }
static inline u32 dword_from_unaligned_word(u16 data) { return u32(data) << 8 | ((data >> 8) * 0x01000001U); }

/* interface for 32-bit data bus (68EC020, 68020) */
void m68000_base_device::init32(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_ospace = &ospace;
	ospace.cache(m_oprogram32);
	space.specific(m_program32);

	m_readimm16 = [this](offs_t address) -> u16 { return m_oprogram32.read_word(address); };
	m_read8   = [this](offs_t address) -> u8     { return m_program32.read_byte(address); };
	m_read16  = [this](offs_t address) -> u16    { return m_program32.read_word_unaligned(address); };
	m_read32  = [this](offs_t address) -> u32    { return m_program32.read_dword_unaligned(address); };
	m_write8  = [this](offs_t address, u8 data)  {
		m_program32.write_dword(address & 0xfffffffcU, dword_from_byte(data), 0xff000000U >> 8 * (address & 3));
	};
	m_write16 = [this](offs_t address, u16 data)  {
		switch (address & 3) {
		case 0:
			m_program32.write_dword(address, dword_from_word(data), 0xffff0000U);
			break;

		case 1:
			m_program32.write_dword(address - 1, dword_from_unaligned_word(data), 0x00ffff00);
			break;

		case 2:
			m_program32.write_dword(address - 2, dword_from_word(data), 0x0000ffff);
			break;

		case 3:
			m_program32.write_dword(address - 3, dword_from_unaligned_word(data), 0x000000ff);
			m_program32.write_dword(address + 1, dword_from_byte(data & 0x00ff), 0xff000000U);
			break;
		}
	};
	m_write32 = [this](offs_t address, u32 data)  {
		switch (address & 3) {
		case 0:
			m_program32.write_dword(address, data, 0xffffffffU);
			break;

		case 1:
			m_program32.write_dword(address - 1, (data & 0xff000000U) | (data & 0xffffff00U) >> 8, 0x00ffffff);
			m_program32.write_dword(address + 3, dword_from_byte(data & 0x000000ff), 0xff000000U);
			break;

		case 2:
			m_program32.write_dword(address - 2, dword_from_word((data & 0xffff0000U) >> 16), 0x0000ffff);
			m_program32.write_dword(address + 2, dword_from_word(data & 0x0000ffff), 0xffff0000U);
			break;

		case 3:
			m_program32.write_dword(address - 3, dword_from_unaligned_word((data & 0xffff0000U) >> 16), 0x000000ff);
			m_program32.write_dword(address + 1, (data & 0x00ffffff) << 8 | (data & 0xff000000U) >> 24, 0xffffff00U);
			break;
		}
	};
}

/* interface for 32-bit data bus with PMMU */
void m68000_base_device::init32mmu(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_ospace = &ospace;
	ospace.cache(m_oprogram32);
	space.specific(m_program32);

	m_readimm16 = [this](offs_t address) -> u16 {
		if (m_pmmu_enabled) {
			address = pmmu_translate_addr(address, 1);
			if (m_mmu_tmp_buserror_occurred)
			return ~0;
		}

		return m_oprogram32.read_word(address);
	};

	m_read8   = [this](offs_t address) -> u8     {
		if (m_pmmu_enabled) {
				address = pmmu_translate_addr(address, 1);
				if (m_mmu_tmp_buserror_occurred)
					return ~0;
		}
		return m_program32.read_byte(address);
	};

	m_read16  = [this](offs_t address) -> u16    {
		if (m_pmmu_enabled) {
			u32 address0 = pmmu_translate_addr(address, 1);
			if (m_mmu_tmp_buserror_occurred)
				return ~0;
			if (WORD_ALIGNED(address))
				return m_program32.read_word(address0);
			u32 address1 = pmmu_translate_addr(address + 1, 1);
			if (m_mmu_tmp_buserror_occurred)
				return ~0;
			u16 result = m_program32.read_byte(address0) << 8;
			return result | m_program32.read_byte(address1);
		}
		return m_program32.read_word_unaligned(address);
	};

	m_read32  = [this](offs_t address) -> u32    {
		if (m_pmmu_enabled) {
			u32 address0 = pmmu_translate_addr(address, 1);
			if (m_mmu_tmp_buserror_occurred)
				return ~0;
			if ((address +3) & 0xfc)
				// not at page boundary; use default code
				address = address0;
			else if (DWORD_ALIGNED(address)) // 0
				return m_program32.read_dword(address0);
			else {
				u32 address2 = pmmu_translate_addr(address+2, 1);
				if (m_mmu_tmp_buserror_occurred)
					return ~0;
				if (WORD_ALIGNED(address)) { // 2
					u32 result = m_program32.read_word(address0) << 16;
					return result | m_program32.read_word(address2);
				}
				u32 address1 = pmmu_translate_addr(address+1, 1);
				u32 address3 = pmmu_translate_addr(address+3, 1);
				if (m_mmu_tmp_buserror_occurred)
					return ~0;
				u32 result = m_program32.read_byte(address0) << 24;
				result |= m_program32.read_word(address1) << 8;
				return result | m_program32.read_byte(address3);
			}
		}
		return m_program32.read_dword_unaligned(address);
	};

	m_write8  = [this](offs_t address, u8  data) {
		if (m_pmmu_enabled) {
			address = pmmu_translate_addr(address, 0);
			if (m_mmu_tmp_buserror_occurred)
				return;
		}
		m_program32.write_dword(address & 0xfffffffcU, dword_from_byte(data), 0xff000000U >> 8 * (address & 3));
	};

	m_write16 = [this](offs_t address, u16 data) {
		u32 address0 = address;
		if (m_pmmu_enabled) {
			address0 = pmmu_translate_addr(address0, 0);
			if (m_mmu_tmp_buserror_occurred)
				return;
		}
		switch (address & 3) {
		case 0:
			m_program32.write_dword(address0, dword_from_word(data), 0xffff0000U);
			break;

		case 1:
			m_program32.write_dword(address0 - 1, dword_from_unaligned_word(data), 0x00ffff00);
			break;

		case 2:
			m_program32.write_dword(address0 - 2, dword_from_word(data), 0x0000ffff);
			break;

		case 3:
		{
			u32 address1 = address + 1;
			if (m_pmmu_enabled) {
				address1 = pmmu_translate_addr(address1, 0);
				if (m_mmu_tmp_buserror_occurred)
					return;
			}
			m_program32.write_dword(address0 - 3, dword_from_unaligned_word(data), 0x000000ff);
			m_program32.write_dword(address1, dword_from_byte(data & 0x00ff), 0xff000000U);
			break;
		}
		}
	};

	m_write32 = [this](offs_t address, u32 data) {
		u32 address0 = address;
		if (m_pmmu_enabled) {
			address0 = pmmu_translate_addr(address0, 0);
			if (m_mmu_tmp_buserror_occurred)
				return;
		}
		switch (address & 3) {
		case 0:
			m_program32.write_dword(address0, data, 0xffffffffU);
			break;

		case 1:
		{
			u32 address3 = address + 3;
			if (m_pmmu_enabled) {
				address3 = pmmu_translate_addr(address3, 0);
				if (m_mmu_tmp_buserror_occurred)
					return;
			}
			m_program32.write_dword(address0 - 1, (data & 0xff000000U) | (data & 0xffffff00U) >> 8, 0x00ffffff);
			m_program32.write_dword(address3, dword_from_byte(data & 0x000000ff), 0xff000000U);
			break;
		}

		case 2:
		{
			u32 address2 = address + 2;
			if (m_pmmu_enabled) {
				address2 = pmmu_translate_addr(address2, 0);
				if (m_mmu_tmp_buserror_occurred)
					return;
			}
			m_program32.write_dword(address0 - 2, dword_from_word((data & 0xffff0000U) >> 16), 0x0000ffff);
			m_program32.write_dword(address2, dword_from_word(data & 0x0000ffff), 0xffff0000U);
			break;
		}

		case 3:
		{
			u32 address1 = address + 1;
			if (m_pmmu_enabled) {
				address1 = pmmu_translate_addr(address1, 0);
				if (m_mmu_tmp_buserror_occurred)
					return;
			}
			m_program32.write_dword(address0 - 3, dword_from_unaligned_word((data & 0xffff0000U) >> 16), 0x000000ff);
			m_program32.write_dword(address1, (data & 0x00ffffff) << 8 | (data & 0xff000000U) >> 24, 0xffffff00U);
			break;
		}
		}
	};
}

void m68000_base_device::init32hmmu(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_ospace = &ospace;
	ospace.cache(m_oprogram32);
	space.specific(m_program32);

	m_readimm16 = [this](offs_t address) -> u16 {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);
		return m_oprogram32.read_word(address);
	};

	m_read8   = [this](offs_t address) -> u8     {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);
		return m_program32.read_byte(address);
	};

	m_read16  = [this](offs_t address) -> u16    {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);
		if (WORD_ALIGNED(address))
			return m_program32.read_word(address);
		u16 result = m_program32.read_byte(address) << 8;
		return result | m_program32.read_byte(address + 1);
	};

	m_read32  = [this](offs_t address) -> u32    {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);

		if (DWORD_ALIGNED(address))
			return m_program32.read_dword(address);
		if (WORD_ALIGNED(address)) {
			u32 result = m_program32.read_word(address) << 16;
			return result | m_program32.read_word(address + 2);
		}
		u32 result = m_program32.read_byte(address) << 24;
		result |= m_program32.read_word(address + 1) << 8;
		return result | m_program32.read_byte(address + 3);
	};

	m_write8  = [this](offs_t address, u8 data)  {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);
		m_program32.write_byte(address, data);
	};

	m_write16 = [this](offs_t address, u16 data)  {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);
		if (WORD_ALIGNED(address)) {
			m_program32.write_word(address, data);
			return;
		}
		m_program32.write_byte(address, data >> 8);
		m_program32.write_byte(address + 1, data);
	};

	m_write32 = [this](offs_t address, u32 data)  {
		if (m_hmmu_enabled)
			address = hmmu_translate_addr(address);

		if (DWORD_ALIGNED(address)) {
			m_program32.write_dword(address, data);
			return;
		}
		if (WORD_ALIGNED(address)) {
			m_program32.write_word(address, data >> 16);
			m_program32.write_word(address + 2, data);
			return;
		}
		m_program32.write_byte(address, data >> 24);
		m_program32.write_word(address + 1, data >> 8);
		m_program32.write_byte(address + 3, data);
	};
}

// fault_addr = address to indicate fault at
// rw = 1 for read, 0 for write
// fc = 3-bit function code of access (usually you'd just put what m68k_get_fc() returns here)
void m68000_base_device::set_buserror_details(u32 fault_addr, u8 rw, u8 fc)
{
	m_aerr_address = fault_addr;
	m_aerr_write_mode = (rw << 4);
	m_aerr_fc = fc;
	m_mmu_tmp_buserror_address = fault_addr; // Hack for x68030
}

u16 m68000_base_device::get_fc()
{
	return m_mmu_tmp_fc;
}

/****************************************************************************
 * State definition
 ****************************************************************************/

void m68000_base_device::define_state(void)
{
	u32 addrmask = (m_cpu_type & MASK_24BIT_SPACE) ? 0xffffff : 0xffffffff;

	state_add(STATE_GENPC,     "PC",        m_pc).mask(addrmask).callimport();
	state_add(STATE_GENPCBASE, "CURPC",     m_ppc).mask(addrmask).callimport().noshow();
	state_add(M68K_SR,         "SR",        m_iotemp).callimport().callexport().mask(m_sr_mask);
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_iotemp).noshow().callimport().callexport().formatstr("%16s");
	state_add(M68K_SP,         "SP",        m_dar[15]);
	state_add(M68K_USP,        "USP",       m_iotemp).callimport().callexport();
	if (m_cpu_type & MASK_020_OR_LATER)
	{
		state_add(M68K_ISP,    "ISP",       m_iotemp).callimport().callexport();
		state_add(M68K_MSP,    "MSP",       m_iotemp).callimport().callexport();
	}
	else
		state_add(M68K_ISP,    "SSP",       m_iotemp).callimport().callexport();

	for (int regnum = 0; regnum < 8; regnum++) {
		state_add(M68K_D0 + regnum, string_format("D%d", regnum).c_str(), m_dar[regnum]);
	}
	for (int regnum = 0; regnum < 8; regnum++) {
		state_add(M68K_A0 + regnum, string_format("A%d", regnum).c_str(), m_dar[8 + regnum]);
	}

	state_add(M68K_IR,         "IR",        m_ir);
	state_add(M68K_PREF_ADDR,  "PREF_ADDR", m_pref_addr).mask(addrmask);
	state_add(M68K_PREF_DATA,  "PREF_DATA", m_pref_data);

	if (m_cpu_type & MASK_010_OR_LATER)
	{
		state_add(M68K_SFC,    "SFC",       m_sfc).mask(0x7);
		state_add(M68K_DFC,    "DFC",       m_dfc).mask(0x7);
		state_add(M68K_VBR,    "VBR",       m_vbr);
	}

	if (m_cpu_type & MASK_020_OR_LATER)
	{
		state_add(M68K_CACR,   "CACR",      m_cacr);
		state_add(M68K_CAAR,   "CAAR",      m_caar);
	}

	if (m_cpu_type & MASK_030_OR_LATER)
	{
		for (int regnum = 0; regnum < 8; regnum++) {
			state_add(M68K_FP0 + regnum, string_format("FP%d", regnum).c_str(), m_iotemp).callimport().callexport().formatstr("%10s");
		}
		state_add(M68K_FPSR, "FPSR", m_fpsr);
		state_add(M68K_FPCR, "FPCR", m_fpcr);
	}

	if (m_has_pmmu)
	{
		state_add(M68K_MMU_TC, "TC", m_mmu_tc);
		state_add(M68K_MMU_SR, "PSR", m_mmu_sr);

		if (m_cpu_type & (CPU_TYPE_010|CPU_TYPE_020)) // 68010/68020 + 68851 PMMU
		{
			state_add(M68K_CRP_LIMIT, "CRP_LIMIT", m_mmu_crp_limit);
			state_add(M68K_CRP_APTR, "CRP_APTR", m_mmu_crp_aptr);
			state_add(M68K_SRP_LIMIT, "SRP_LIMIT", m_mmu_srp_limit);
			state_add(M68K_SRP_APTR, "SRP_APTR", m_mmu_srp_aptr);
		}

		if (m_cpu_type & CPU_TYPE_030)
		{
			state_add(M68K_TT0, "TT0", m_mmu_tt0);
			state_add(M68K_TT1, "TT1", m_mmu_tt1);
			state_add(M68K_CRP_LIMIT, "CRP_LIMIT", m_mmu_crp_limit);
			state_add(M68K_CRP_APTR, "CRP_APTR", m_mmu_crp_aptr);
			state_add(M68K_SRP_LIMIT, "SRP_LIMIT", m_mmu_srp_limit);
			state_add(M68K_SRP_APTR, "SRP_APTR", m_mmu_srp_aptr);
		}

		if (m_cpu_type & CPU_TYPE_040)
		{
			state_add(M68K_ITT0, "ITT0", m_mmu_itt0);
			state_add(M68K_ITT1, "ITT1", m_mmu_itt1);
			state_add(M68K_DTT0, "DTT0", m_mmu_dtt0);
			state_add(M68K_DTT1, "DTT1", m_mmu_dtt1);
			state_add(M68K_URP_APTR, "URP", m_mmu_urp_aptr);
			state_add(M68K_SRP_APTR, "SRP", m_mmu_srp_aptr);
		}
	}
}



/****************
 CPU Inits
****************/


void m68000_base_device::init_cpu_m68000(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_000;

	init16(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[0];
	m_cyc_instruction  = m68ki_cycles[0];
	m_cyc_exception    = m68ki_exception_cycle_table[0];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 132;
	m_has_pmmu         = 0;
	m_has_hmmu         = 0;
	m_has_fpu          = 0;

	define_state();

}


void m68000_base_device::init_cpu_m68008(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_008;

	init8(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[0];
	m_cyc_instruction  = m68ki_cycles[0];
	m_cyc_exception    = m68ki_exception_cycle_table[0];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 2;
	m_cyc_dbcc_f_noexp = -2;
	m_cyc_dbcc_f_exp   = 2;
	m_cyc_scc_r_true   = 2;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 132;
	m_has_pmmu         = 0;
	m_has_fpu          = 0;

	define_state();

}


void m68000_base_device::init_cpu_m68010(void)
{
	init_cpu_common();
	m_cpu_type         = CPU_TYPE_010;

	init16(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[2];
	m_cyc_instruction  = m68ki_cycles[2];
	m_cyc_exception    = m68ki_exception_cycle_table[2];
	m_cyc_bcc_notake_b = -4;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 6;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 8;
	m_cyc_shift        = 2;
	m_cyc_reset        = 130;
	m_has_pmmu         = 0;
	m_has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68020(void)
{
	init_cpu_common();
	m_cpu_type         = CPU_TYPE_020;

	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[3];
	m_cyc_instruction  = m68ki_cycles[3];
	m_cyc_exception    = m68ki_exception_cycle_table[3];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;

	define_state();
}

void m68000_base_device::init_cpu_m68020fpu(void)
{
	init_cpu_m68020();

	m_has_fpu          = 1;
}

void m68000_base_device::init_cpu_m68020pmmu(void)
{
	init_cpu_m68020();

	m_has_pmmu         = 1;
	m_has_fpu          = 1;


	init32mmu(*m_program, *m_oprogram);
}



void m68000_base_device::init_cpu_m68020hmmu(void)
{
	init_cpu_m68020();

	m_has_hmmu = 1;
	m_has_fpu  = 1;


	init32hmmu(*m_program, *m_oprogram);
}

void m68000_base_device::init_cpu_m68ec020(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_EC020;


	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[3];
	m_cyc_instruction  = m68ki_cycles[3];
	m_cyc_exception    = m68ki_exception_cycle_table[3];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 0;
	m_has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68030(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_030;


	init32mmu(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[4];
	m_cyc_instruction  = m68ki_cycles[4];
	m_cyc_exception    = m68ki_exception_cycle_table[4];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 1;
	m_has_fpu          = 1;

	define_state();
}



void m68000_base_device::init_cpu_m68ec030(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_EC030;


	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[4];
	m_cyc_instruction  = m68ki_cycles[4];
	m_cyc_exception    = m68ki_exception_cycle_table[4];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 0;     /* EC030 lacks the PMMU and is effectively a die-shrink 68020 */
	m_has_fpu          = 1;

	define_state();
}



void m68000_base_device::init_cpu_m68040(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_040;


	init32mmu(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[5];
	m_cyc_instruction  = m68ki_cycles[5];
	m_cyc_exception    = m68ki_exception_cycle_table[5];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 1;
	m_has_fpu          = 1;

	define_state();
}


void m68000_base_device::init_cpu_m68ec040(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_EC040;


	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[5];
	m_cyc_instruction  = m68ki_cycles[5];
	m_cyc_exception    = m68ki_exception_cycle_table[5];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 0;
	m_has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68lc040(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_LC040;


	init32mmu(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[5];
	m_cyc_instruction  = m68ki_cycles[5];
	m_cyc_exception    = m68ki_exception_cycle_table[5];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;
	m_has_pmmu         = 1;
	m_has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_scc68070(void)
{
	init_cpu_common();
	m_cpu_type         = CPU_TYPE_SCC070;

	init16(*m_program, *m_oprogram);
	m_sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[1];
	m_cyc_instruction  = m68ki_cycles[1];
	m_cyc_exception    = m68ki_exception_cycle_table[1];
	m_cyc_bcc_notake_b = 0;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 3;
	m_cyc_dbcc_f_exp   = 3;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 7;
	m_cyc_movem_l      = 11;
	m_cyc_shift        = 3;
	m_cyc_reset        = 154;
	m_has_pmmu         = 0;
	m_has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_fscpu32(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_FSCPU32;


	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[6];
	m_cyc_instruction  = m68ki_cycles[6];
	m_cyc_exception    = m68ki_exception_cycle_table[6];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;

	define_state();
}



void m68000_base_device::init_cpu_coldfire(void)
{
	init_cpu_common();

	m_cpu_type         = CPU_TYPE_COLDFIRE;


	init32(*m_program, *m_oprogram);
	m_sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m_state_table      = m68ki_instruction_state_table[7];
	m_cyc_instruction  = m68ki_cycles[7];
	m_cyc_exception    = m68ki_exception_cycle_table[7];
	m_cyc_bcc_notake_b = -2;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp   = 4;
	m_cyc_scc_r_true   = 0;
	m_cyc_movem_w      = 4;
	m_cyc_movem_l      = 4;
	m_cyc_shift        = 1;
	m_cyc_reset        = 518;

	define_state();
}

std::unique_ptr<util::disasm_interface> m68000_base_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}

std::unique_ptr<util::disasm_interface> m68000_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}

std::unique_ptr<util::disasm_interface> m68008_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

std::unique_ptr<util::disasm_interface> m68008fn_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68008);
}

std::unique_ptr<util::disasm_interface> m68010_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68010);
}

std::unique_ptr<util::disasm_interface> m68ec020_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020fpu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020pmmu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68020hmmu_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68020);
}

std::unique_ptr<util::disasm_interface> m68ec030_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68030);
}

std::unique_ptr<util::disasm_interface> m68030_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68030);
}

std::unique_ptr<util::disasm_interface> m68ec040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

std::unique_ptr<util::disasm_interface> m68lc040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

std::unique_ptr<util::disasm_interface> m68040_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68040);
}

std::unique_ptr<util::disasm_interface> scc68070_base_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68000);
}

std::unique_ptr<util::disasm_interface> fscpu32_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_68340);
}

std::unique_ptr<util::disasm_interface> mcf5206e_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_COLDFIRE);
}


/* Service an interrupt request and start exception processing */
void m68000_base_device::m68ki_exception_interrupt(u32 int_level)
{
	u32 vector;
	u32 sr;
	u32 new_pc;

	if(CPU_TYPE_IS_000())
	{
		m_instr_mode = INSTRUCTION_NO;
	}

	/* Turn off the m_stopped state */
	m_stopped &= ~STOP_LEVEL_STOP;

	/* If we are halted, don't do anything */
	if(m_stopped)
		return;

	/* Inform the device than an interrupt is taken */
	if(m_interrupt_mixer)
		standard_irq_callback(int_level);
	else
		for(int i=0; i<3; i++)
			if(int_level & (1<<i))
				standard_irq_callback(i);

	/* Acknowledge the interrupt by reading the cpu space. */
	/* We require the handlers for autovector to return the correct
	   vector, including for spurious interrupts. */

	// 68000 and 68010 assert UDS as well as LDS for IACK cycles but disregard D8-D15
	// 68008 definitely reads only one byte from CPU space, and the 68020 byte-sizes the request
	if(CPU_TYPE_IS_EC020_PLUS() || m_cpu_type == CPU_TYPE_008)
		vector = m_cpu_space->read_byte(0xfffffff1 | (int_level << 1));
	else
		vector = m_cpu_space->read_word(0xfffffff0 | (int_level << 1)) & 0xff;

	/* Start exception processing */
	sr = m68ki_init_exception(vector);

	/* Set the interrupt mask to the level of the one being serviced */
	m_int_mask = int_level<<8;

	/* Get the new PC */
	new_pc = m68ki_read_data_32((vector<<2) + m_vbr);

	/* If vector is uninitialized, call the uninitialized interrupt vector */
	if(new_pc == 0)
		new_pc = m68ki_read_data_32((EXCEPTION_UNINITIALIZED_INTERRUPT<<2) + m_vbr);

	/* Generate a stack frame */
	m68ki_stack_frame_0000(m_pc, sr, vector);
	if(m_m_flag && CPU_TYPE_IS_EC020_PLUS())
	{
		/* Create throwaway frame */
		m68ki_set_sm_flag(m_s_flag);  /* clear M */
		sr |= 0x2000; /* Same as SR in master stack frame except S is forced high */
		m68ki_stack_frame_0001(m_pc, sr, vector);
	}

	m68ki_jump(new_pc);

	/* Defer cycle counting until later */
	m_icount -= m_cyc_exception[vector];
}


//-------------------------------------------------
//  m68000_base_device - constructor
//-------------------------------------------------

m68000_base_device::m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, internal_map),
		m_oprogram_config("decrypted_opcodes", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, internal_map),
		m_cpu_space_config("cpu_space", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, address_map_constructor(FUNC(m68000_base_device::default_autovectors_map), this)),
		m_interrupt_mixer(true),
		m_cpu_space_id(AS_CPU_SPACE),
		m_reset_instr_callback(*this),
		m_cmpild_instr_callback(*this),
		m_rte_instr_callback(*this),
		m_tas_write_callback(*this)
{
	clear_all();
}


m68000_base_device::m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, prg_address_bits),
		m_oprogram_config("decrypted_opcodes", ENDIANNESS_BIG, prg_data_width, prg_address_bits),
		m_cpu_space_config("cpu_space", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, address_map_constructor(FUNC(m68000_base_device::default_autovectors_map), this)),
		m_interrupt_mixer(true),
		m_cpu_space_id(AS_CPU_SPACE),
		m_reset_instr_callback(*this),
		m_cmpild_instr_callback(*this),
		m_rte_instr_callback(*this),
		m_tas_write_callback(*this)
{
	clear_all();
}

void m68000_base_device::clear_all()
{
	m_cpu_type= 0;
//
	for (auto & elem : m_dar)
		elem= 0;
	m_ppc= 0;
	m_pc= 0;
	for (auto & elem : m_sp)
		elem= 0;
	m_vbr= 0;
	m_sfc= 0;
	m_dfc= 0;
	m_cacr= 0;
	m_caar= 0;
	m_ir= 0;
//  for (int i=0;i<8;i++)
//      m_fpr[i]= 0;
	m_fpiar= 0;
	m_fpsr= 0;
	m_fpcr= 0;
	m_t1_flag= 0;
	m_t0_flag= 0;
	m_s_flag= 0;
	m_m_flag= 0;
	m_x_flag= 0;
	m_n_flag= 0;
	m_not_z_flag= 0;
	m_v_flag= 0;
	m_c_flag= 0;
	m_int_mask= 0;
	m_int_level= 0;
	m_stopped= 0;
	m_pref_addr= 0;
	m_pref_data= 0;
	m_sr_mask= 0;
	m_instr_mode= 0;
	m_run_mode= 0;
	m_has_pmmu= 0;
	m_has_hmmu= 0;
	m_pmmu_enabled= 0;
	m_hmmu_enabled= 0;
	m_has_fpu= 0;
	m_fpu_just_reset= 0;

	m_cyc_bcc_notake_b = 0;
	m_cyc_bcc_notake_w = 0;
	m_cyc_dbcc_f_noexp = 0;
	m_cyc_dbcc_f_exp = 0;
	m_cyc_scc_r_true = 0;
	m_cyc_movem_w = 1;
	m_cyc_movem_l = 1;
	m_cyc_shift = 1;
	m_cyc_reset = 0;

	m_initial_cycles = 0;
	m_icount = 0;
	m_reset_cycles = 0;
	m_tracing = 0;

	m_address_error = 0;

	m_aerr_address = 0;
	m_aerr_write_mode = 0;
	m_aerr_fc = 0;

	m_virq_state = 0;
	m_nmi_pending = 0;

	m_cyc_instruction = nullptr;
	m_cyc_exception = nullptr;

	m_program = nullptr;

	m_space = nullptr;
	m_ospace = nullptr;

	m_iotemp = 0;

	m_save_sr = 0;
	m_save_stopped = 0;
	m_save_halted = 0;


	m_mmu_crp_aptr = m_mmu_crp_limit = 0;
	m_mmu_srp_aptr = m_mmu_srp_limit = 0;
	m_mmu_urp_aptr = 0;
	m_mmu_tc = 0;
	m_mmu_sr = 0;
	m_mmu_sr_040 = 0;

	for (int i=0; i<MMU_ATC_ENTRIES;i++)
		m_mmu_atc_tag[i] = m_mmu_atc_data[i] = 0;

	m_mmu_atc_rr = 0;
	m_mmu_tt0 = m_mmu_tt1 = 0;
	m_mmu_itt0 = m_mmu_itt1 = m_mmu_dtt0 = m_mmu_dtt1 = 0;
	m_mmu_acr0 = m_mmu_acr1 = m_mmu_acr2 = m_mmu_acr3 = 0;
	m_mmu_tmp_sr = 0;
	m_mmu_tmp_fc = 0;
	m_mmu_tmp_rw = 0;
	m_mmu_tmp_buserror_address = 0;
	m_mmu_tmp_buserror_occurred = 0;
	m_mmu_tmp_buserror_fc = 0;
	m_mmu_tmp_buserror_rw = 0;

	for (int i=0;i<M68K_IC_SIZE;i++)
	{
		m_ic_address[i] = 0;
		m_ic_data[i] = 0;
		m_ic_valid[i] = false;
	}

	m_internal = nullptr;
}

void m68000_base_device::autovectors_map(address_map &map)
{
	// Eventually add the sync to E due to vpa
	// 8-bit handlers are used here to be compatible with all bus widths
	map(0x3, 0x3).lr8(NAME([] () -> u8 { return autovector(1); }));
	map(0x5, 0x5).lr8(NAME([] () -> u8 { return autovector(2); }));
	map(0x7, 0x7).lr8(NAME([] () -> u8 { return autovector(3); }));
	map(0x9, 0x9).lr8(NAME([] () -> u8 { return autovector(4); }));
	map(0xb, 0xb).lr8(NAME([] () -> u8 { return autovector(5); }));
	map(0xd, 0xd).lr8(NAME([] () -> u8 { return autovector(6); }));
	map(0xf, 0xf).lr8(NAME([] () -> u8 { return autovector(7); }));
}

void m68000_base_device::default_autovectors_map(address_map &map)
{
	if(m_cpu_space_id == AS_CPU_SPACE && !has_configured_map(AS_CPU_SPACE)) {
		offs_t mask = make_bitmask<offs_t>(m_program_config.addr_width());
		map(mask - 0xf, mask).m(*this, FUNC(m68000_base_device::autovectors_map));
	}
}

void m68000_base_device::device_start()
{
	m_reset_instr_callback.resolve();
	m_cmpild_instr_callback.resolve();
	m_rte_instr_callback.resolve();
	m_tas_write_callback.resolve();
}

void m68000_base_device::device_stop()
{
}




void m68000_base_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case M68K_IRQ_NONE:
		case M68K_IRQ_1:
		case M68K_IRQ_2:
		case M68K_IRQ_3:
		case M68K_IRQ_4:
		case M68K_IRQ_5:
		case M68K_IRQ_6:
		case M68K_IRQ_7:
		case INPUT_LINE_NMI:
			set_irq_line(inputnum, state);
			break;

		case M68K_LINE_BUSERROR:
			if (state == ASSERT_LINE)
			{
				m68k_cause_bus_error();
			}
			break;
	}
}


device_memory_interface::space_config_vector m68000_base_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		if(m_cpu_space_id == AS_CPU_SPACE)
			return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_OPCODES, &m_oprogram_config),
				std::make_pair(AS_CPU_SPACE, &m_cpu_space_config)
			};
		else
			return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_OPCODES, &m_oprogram_config)
			};
	else
		if(m_cpu_space_id == AS_CPU_SPACE)
			return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config),
				std::make_pair(AS_CPU_SPACE, &m_cpu_space_config)
			};
		else
			return space_config_vector {
				std::make_pair(AS_PROGRAM, &m_program_config)
			};
}



DEFINE_DEVICE_TYPE(M68000,      m68000_device,      "m68000",       "Motorola MC68000")
DEFINE_DEVICE_TYPE(M68008,      m68008_device,      "m68008",       "Motorola MC68008") // 48-pin plastic or ceramic DIP
DEFINE_DEVICE_TYPE(M68008FN,    m68008fn_device,    "m68008fn",     "Motorola MC68008FN") // 52-pin PLCC
DEFINE_DEVICE_TYPE(M68010,      m68010_device,      "m68010",       "Motorola MC68010")
DEFINE_DEVICE_TYPE(M68EC020,    m68ec020_device,    "m68ec020",     "Motorola MC68EC020")
DEFINE_DEVICE_TYPE(M68020,      m68020_device,      "m68020",       "Motorola MC68020")
DEFINE_DEVICE_TYPE(M68020FPU,   m68020fpu_device,   "m68020fpu",    "Motorola MC68020FPU")
DEFINE_DEVICE_TYPE(M68020PMMU,  m68020pmmu_device,  "m68020pmmu",   "Motorola MC68020PMMU")
DEFINE_DEVICE_TYPE(M68020HMMU,  m68020hmmu_device,  "m68020hmmu",   "Motorola MC68020HMMU")
DEFINE_DEVICE_TYPE(M68EC030,    m68ec030_device,    "m68ec030",     "Motorola MC68EC030")
DEFINE_DEVICE_TYPE(M68030,      m68030_device,      "m68030",       "Motorola MC68030")
DEFINE_DEVICE_TYPE(M68EC040,    m68ec040_device,    "m68ec040",     "Motorola MC68EC040")
DEFINE_DEVICE_TYPE(M68LC040,    m68lc040_device,    "m68lc040",     "Motorola MC68LC040")
DEFINE_DEVICE_TYPE(M68040,      m68040_device,      "m68040",       "Motorola MC68040")
DEFINE_DEVICE_TYPE(FSCPU32,     fscpu32_device,     "fscpu32",      "Freescale CPU32 Core")
DEFINE_DEVICE_TYPE(MCF5206E,    mcf5206e_device,    "mcf5206e",     "Freescale MCF5206E")

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_device(mconfig, M68000, tag, owner, clock)
{
}

m68000_device::m68000_device(const machine_config &mconfig, const device_type type, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, type, 16,24)
{
}

void m68000_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68000();
}

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map)
	: m68000_base_device(mconfig, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map)
{
}






/* m68008_device */

m68008_device::m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68008, 8,20)
{
}

void m68008_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68008();
}


m68008fn_device::m68008fn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68008FN, 8,22)
{
}

void m68008fn_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68008();
}



m68010_device::m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68010, 16,24)
{
}

void m68010_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68010();
}



m68020_device::m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68020, 32,32)
{
}

void m68020_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68020();
}


m68020fpu_device::m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68020FPU, 32,32)
{
}

void m68020fpu_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68020fpu();
}

// 68020 with 68851 PMMU
m68020pmmu_device::m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68020PMMU, 32,32)
{
}

void m68020pmmu_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68020pmmu();
}

bool m68020hmmu_device::memory_translate(int space, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	{
		if ((space == AS_PROGRAM) && (m_hmmu_enabled))
		{
			address = hmmu_translate_addr(address);
		}
	}
	return true;
}


// 68020 with Apple HMMU & 68881 FPU
//      case CPUINFO_FCT_TRANSLATE: info->translate = CPU_TRANSLATE_NAME(m68khmmu);     break;
m68020hmmu_device::m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68020HMMU, 32,32)
{
}

void m68020hmmu_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68020hmmu();
}


m68ec020_device::m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68EC020, 32,24)
{
}

void m68ec020_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68ec020();
}

m68030_device::m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68030, 32,32)
{
}

void m68030_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68030();
}

m68ec030_device::m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68EC030, 32,32)
{
}

void m68ec030_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68ec030();
}

m68040_device::m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68040, 32,32)
{
}


void m68040_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68040();
}



m68ec040_device::m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68EC040, 32,32)
{
}

void m68ec040_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68ec040();
}



m68lc040_device::m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, M68LC040, 32,32)
{
}

void m68lc040_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_m68lc040();
}


scc68070_base_device::scc68070_base_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
						const device_type type, address_map_constructor internal_map)
	: m68000_base_device(mconfig, tag, owner, clock, type, 16,32, internal_map)
{
}

void scc68070_base_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_scc68070();
}


fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, FSCPU32, 32,32)
{
}

fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock,
										const device_type type, u32 prg_data_width, u32 prg_address_bits, address_map_constructor internal_map)
	: m68000_base_device(mconfig, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map)
{
}


void fscpu32_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_fscpu32();
}



mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_base_device(mconfig, tag, owner, clock, MCF5206E, 32,32)
{
}

void mcf5206e_device::device_start()
{
	m68000_base_device::device_start();
	init_cpu_coldfire();
}

void m68000_base_device::m68ki_set_one(unsigned short opcode, u16 state, const opcode_handler_struct &s)
{
	for(int i=0; i<NUM_CPU_TYPES; i++)
		if(s.cycles[i] != 0xff) {
			m68ki_cycles[i][opcode] = s.cycles[i];
			m68ki_instruction_state_table[i][opcode] = state;
		}
}

void m68000_base_device::m68ki_build_opcode_table()
{
	for(int i = 0; i < 0x10000; i++)
	{
		/* default to illegal */
		for(int k=0;k<NUM_CPU_TYPES;k++)
		{
			m68ki_instruction_state_table[k][i] = m68k_state_illegal;
			m68ki_cycles[k][i] = 0;
		}
	}

	for(u16 state = 0; m68k_opcode_table[state].mask; state++)
	{
		const auto &os = m68k_opcode_table[state];
		u16 mask = os.mask;
		u16 extraval = 0;
		do {
			m68ki_set_one(os.match | extraval, state, os);
			extraval = ((extraval | mask) + 1) & ~mask;
		} while(extraval);
	}
}
