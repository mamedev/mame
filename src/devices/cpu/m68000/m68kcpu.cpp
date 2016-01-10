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
#include "m68kcpu.h"
#include "m68kops.h"

#include "m68kfpu.inc"
#include "m68kmmu.h"

extern void m68040_fpu_op0(m68000_base_device *m68k);
extern void m68040_fpu_op1(m68000_base_device *m68k);
extern void m68881_mmu_ops(m68000_base_device *m68k);

/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* Used by shift & rotate instructions */
const UINT8 m68ki_shift_8_table[65] =
{
	0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff
};
const UINT16 m68ki_shift_16_table[65] =
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
const UINT32 m68ki_shift_32_table[65] =
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
const UINT8 m68ki_exception_cycle_table[7][256] =
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

const UINT8 m68ki_ea_idx_cycle_table[64] =
{
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0, /* ..01.000 no memory indirect, base NULL             */
		5, /* ..01..01 memory indirect,    base NULL, outer NULL */
		7, /* ..01..10 memory indirect,    base NULL, outer 16   */
		7, /* ..01..11 memory indirect,    base NULL, outer 32   */
		0,  5,  7,  7,  0,  5,  7,  7,  0,  5,  7,  7,
		2, /* ..10.000 no memory indirect, base 16               */
		7, /* ..10..01 memory indirect,    base 16,   outer NULL */
		9, /* ..10..10 memory indirect,    base 16,   outer 16   */
		9, /* ..10..11 memory indirect,    base 16,   outer 32   */
		0,  7,  9,  9,  0,  7,  9,  9,  0,  7,  9,  9,
		6, /* ..11.000 no memory indirect, base 32               */
	11, /* ..11..01 memory indirect,    base 32,   outer NULL */
	13, /* ..11..10 memory indirect,    base 32,   outer 16   */
	13, /* ..11..11 memory indirect,    base 32,   outer 32   */
		0, 11, 13, 13,  0, 11, 13, 13,  0, 11, 13, 13
};



/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define MASK_ALL                (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 )
#define MASK_24BIT_SPACE            (CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020)
#define MASK_32BIT_SPACE            (CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 )
#define MASK_010_OR_LATER           (CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_FSCPU32 )
#define MASK_020_OR_LATER           (CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_FSCPU32 )
#define MASK_030_OR_LATER           (CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040)
#define MASK_040_OR_LATER           (CPU_TYPE_040 | CPU_TYPE_EC040)



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

static void set_irq_line(m68000_base_device *m68k, int irqline, int state)
{
	UINT32 old_level = m68k->int_level;
	UINT32 vstate = m68k->virq_state;
	UINT32 blevel;

	if(state == ASSERT_LINE)
		vstate |= 1 << irqline;
	else
		vstate &= ~(1 << irqline);
	m68k->virq_state = vstate;

	for(blevel = 7; blevel > 0; blevel--)
		if(vstate & (1 << blevel))
			break;

	m68k->int_level = blevel << 8;

	/* A transition from < 7 to 7 always interrupts (NMI) */
	/* Note: Level 7 can also level trigger like a normal IRQ */
	if(old_level != 0x0700 && m68k->int_level == 0x0700)
		m68k->nmi_pending = TRUE;
}

static void m68k_presave(m68000_base_device *m68k)
{
	m68k->save_sr = m68ki_get_sr(m68k);
	m68k->save_stopped = (m68k->stopped & STOP_LEVEL_STOP) != 0;
	m68k->save_halted  = (m68k->stopped & STOP_LEVEL_HALT) != 0;
}

static void m68k_postload(m68000_base_device *m68k)
{
	m68ki_set_sr_noint_nosp(m68k, m68k->save_sr);
	//fprintf(stderr, "Reloaded, pc=%x\n", REG_PC(m68k));
	m68k->stopped = (m68k->save_stopped ? STOP_LEVEL_STOP : 0) | (m68k->save_halted  ? STOP_LEVEL_HALT : 0);
	m68ki_jump(m68k, REG_PC(m68k));
}

static void m68k_cause_bus_error(m68000_base_device *m68k)
{
	UINT32 sr;

	sr = m68ki_init_exception(m68k);

	m68k->run_mode = RUN_MODE_BERR_AERR_RESET;

	if (!CPU_TYPE_IS_010_PLUS(m68k->cpu_type))
	{
		/* Note: This is implemented for 68000 only! */
		m68ki_stack_frame_buserr(m68k, sr);
	}
	else if (CPU_TYPE_IS_010(m68k->cpu_type))
	{
		/* only the 68010 throws this unique type-1000 frame */
		m68ki_stack_frame_1000(m68k, REG_PPC(m68k), sr, EXCEPTION_BUS_ERROR);
	}
	else if (m68k->mmu_tmp_buserror_address == REG_PPC(m68k))
	{
		m68ki_stack_frame_1010(m68k, sr, EXCEPTION_BUS_ERROR, REG_PPC(m68k), m68k->mmu_tmp_buserror_address);
	}
	else
	{
		m68ki_stack_frame_1011(m68k, sr, EXCEPTION_BUS_ERROR, REG_PPC(m68k), m68k->mmu_tmp_buserror_address);
	}

	m68ki_jump_vector(m68k, EXCEPTION_BUS_ERROR);
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq1 )
{
	set_input_line( M68K_IRQ_1, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq2 )
{
	set_input_line( M68K_IRQ_2, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq3 )
{
	set_input_line( M68K_IRQ_3, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq4 )
{
	set_input_line( M68K_IRQ_4, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq5 )
{
	set_input_line( M68K_IRQ_5, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq6 )
{
	set_input_line( M68K_IRQ_6, state );
}

WRITE_LINE_MEMBER( m68000_base_device::write_irq7 )
{
	set_input_line( M68K_IRQ_7, state );
}

bool m68000_base_device::memory_translate(address_spacenum space, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	if (this)
	{
		/* 68040 needs to call the MMU even when disabled so transparent translation works */
		if ((space == AS_PROGRAM) && ((pmmu_enabled) || (CPU_TYPE_IS_040_PLUS(cpu_type))))
		{
			// FIXME: mmu_tmp_sr will be overwritten in pmmu_translate_addr_with_fc
			UINT16 temp_mmu_tmp_sr = mmu_tmp_sr;
			int mode = s_flag ? FUNCTION_CODE_SUPERVISOR_PROGRAM : FUNCTION_CODE_USER_PROGRAM;
//          UINT32 va=address;

			if (CPU_TYPE_IS_040_PLUS(cpu_type))
			{
				address = pmmu_translate_addr_with_fc_040(this, address, mode, 1);
			}
			else
			{
				address = pmmu_translate_addr_with_fc(this, address, mode, 1);
			}

			if ((mmu_tmp_sr & M68K_MMU_SR_INVALID) != 0) {
//              logerror("cpu_translate_m68k failed with mmu_sr=%04x va=%08x pa=%08x\n",mmu_tmp_sr,va ,address);
				address = 0;
			}

			mmu_tmp_sr = temp_mmu_tmp_sr;
		}
	}
	return TRUE;
}









inline void m68000_base_device::cpu_execute(void)
{
	initial_cycles = remaining_cycles;

	/* eat up any reset cycles */
	if (reset_cycles) {
		int rc = reset_cycles;
		reset_cycles = 0;
		remaining_cycles -= rc;

		if (remaining_cycles <= 0) return;
	}

	/* See if interrupts came in */
	m68ki_check_interrupts(this);

	/* Make sure we're not stopped */
	if(!stopped)
	{
		/* Return point if we had an address error */
		check_address_error:
		if (m_address_error==1)
		{
			m_address_error = 0;
			try {
				m68ki_exception_address_error(this);
			}
			catch(int error)
			{
				if (error==10)
				{
					m_address_error = 1;
					REG_PPC(this) = REG_PC(this);
					goto check_address_error;
				}
				else
					throw;
			}
			if(stopped)
			{
				if (remaining_cycles > 0)
					remaining_cycles = 0;
				return;
			}
		}


		/* Main loop.  Keep going until we run out of clock cycles */
		while (remaining_cycles > 0)
		{
			/* Set tracing accodring to T1. (T0 is done inside instruction) */
			m68ki_trace_t1(this); /* auto-disable (see m68kcpu.h) */

			/* Call external hook to peek at CPU */
			debugger_instruction_hook(this, REG_PC(this));

			/* call external instruction hook (independent of debug mode) */
			if (!instruction_hook.isnull())
				instruction_hook(*program, REG_PC(this), 0xffffffff);

			/* Record previous program counter */
			REG_PPC(this) = REG_PC(this);

			try
			{
			if (!pmmu_enabled)
			{
				run_mode = RUN_MODE_NORMAL;
				/* Read an instruction and call its handler */
				ir = m68ki_read_imm_16(this);
				jump_table[ir](this);
				remaining_cycles -= cyc_instruction[ir];
			}
			else
			{
				run_mode = RUN_MODE_NORMAL;
				// save CPU address registers values at start of instruction
				int i;
				UINT32 tmp_dar[16];

				for (i = 15; i >= 0; i--)
				{
					tmp_dar[i] = REG_DA(this)[i];
				}

				mmu_tmp_buserror_occurred = 0;

				/* Read an instruction and call its handler */
				ir = m68ki_read_imm_16(this);

				if (!mmu_tmp_buserror_occurred)
				{
					jump_table[ir](this);
					remaining_cycles -= cyc_instruction[ir];
				}

				if (mmu_tmp_buserror_occurred)
				{
					UINT32 sr;

					mmu_tmp_buserror_occurred = 0;

					// restore cpu address registers to value at start of instruction
					for (i = 15; i >= 0; i--)
					{
						if (REG_DA(this)[i] != tmp_dar[i])
						{
//                          logerror("PMMU: pc=%08x sp=%08x bus error: fixed %s[%d]: %08x -> %08x\n",
//                                  REG_PPC(this), REG_A(this)[7], i < 8 ? "D" : "A", i & 7, REG_DA(this)[i], tmp_dar[i]);
							REG_DA(this)[i] = tmp_dar[i];
						}
					}

					sr = m68ki_init_exception(this);

					run_mode = RUN_MODE_BERR_AERR_RESET;

					if (!CPU_TYPE_IS_020_PLUS(cpu_type))
					{
						/* Note: This is implemented for 68000 only! */
						m68ki_stack_frame_buserr(this, sr);
					}
					else if(!CPU_TYPE_IS_040_PLUS(cpu_type)) {
						if (mmu_tmp_buserror_address == REG_PPC(this))
						{
							m68ki_stack_frame_1010(this, sr, EXCEPTION_BUS_ERROR, REG_PPC(this), mmu_tmp_buserror_address);
						}
						else
						{
							m68ki_stack_frame_1011(this, sr, EXCEPTION_BUS_ERROR, REG_PPC(this), mmu_tmp_buserror_address);
						}
					}
					else
					{
						m68ki_stack_frame_0111(this, sr, EXCEPTION_BUS_ERROR, REG_PPC(this), mmu_tmp_buserror_address, true);
					}

					m68ki_jump_vector(this, EXCEPTION_BUS_ERROR);

					// TODO:
					/* Use up some clock cycles and undo the instruction's cycles */
					// remaining_cycles -= cyc_exception[EXCEPTION_BUS_ERROR] - cyc_instruction[ir];
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
			m68ki_exception_if_trace(this); /* auto-disable (see m68kcpu.h) */
		}

		/* set previous PC to current PC for the next entry into the loop */
		REG_PPC(this) = REG_PC(this);
	}
	else if (remaining_cycles > 0)
		remaining_cycles = 0;
}



void m68000_base_device::init_cpu_common(void)
{
	static UINT32 emulation_initialized = 0;

	//this = device;//deviceparam;
	program = &space(AS_PROGRAM);
	oprogram = has_space(AS_DECRYPTED_OPCODES) ? &space(AS_DECRYPTED_OPCODES) : program;
	int_ack_callback = device_irq_acknowledge_delegate(FUNC(m68000_base_device::standard_irq_callback_member), this);

	/* disable all MMUs */
	has_pmmu         = 0;
	has_hmmu         = 0;
	pmmu_enabled     = 0;
	hmmu_enabled     = 0;

	/* The first call to this function initializes the opcode handler jump table */
	if(!emulation_initialized)
	{
		m68ki_build_opcode_table();
		emulation_initialized = 1;
	}

	/* Note, D covers A because the dar array is common, REG_A(m68k)=REG_D(m68k)+8 */
	save_item(NAME(REG_D(this)));
	save_item(NAME(REG_PPC(this)));
	save_item(NAME(REG_PC(this)));
	save_item(NAME(REG_USP(this)));
	save_item(NAME(REG_ISP(this)));
	save_item(NAME(REG_MSP(this)));
	save_item(NAME(vbr));
	save_item(NAME(sfc));
	save_item(NAME(dfc));
	save_item(NAME(cacr));
	save_item(NAME(caar));
	save_item(NAME(save_sr));
	save_item(NAME(int_level));
	save_item(NAME(save_stopped));
	save_item(NAME(save_halted));
	save_item(NAME(pref_addr));
	save_item(NAME(pref_data));
	save_item(NAME(reset_cycles));
	save_item(NAME(virq_state));
	save_item(NAME(nmi_pending));
	save_item(NAME(has_pmmu));
	save_item(NAME(has_hmmu));
	save_item(NAME(pmmu_enabled));
	save_item(NAME(hmmu_enabled));

	save_item(NAME(mmu_crp_aptr));
	save_item(NAME(mmu_crp_limit));
	save_item(NAME(mmu_srp_aptr));
	save_item(NAME(mmu_srp_limit));
	save_item(NAME(mmu_urp_aptr));
	save_item(NAME(mmu_tc));
	save_item(NAME(mmu_sr));
	save_item(NAME(mmu_sr_040));
	save_item(NAME(mmu_atc_rr));
	save_item(NAME(mmu_tt0));
	save_item(NAME(mmu_tt1));
	save_item(NAME(mmu_itt0));
	save_item(NAME(mmu_itt1));
	save_item(NAME(mmu_dtt0));
	save_item(NAME(mmu_dtt1));
	save_item(NAME(mmu_acr0));
	save_item(NAME(mmu_acr1));
	save_item(NAME(mmu_acr2));
	save_item(NAME(mmu_acr3));
	save_item(NAME(mmu_last_page_entry));
	save_item(NAME(mmu_last_page_entry_addr));

	for (int i=0; i<MMU_ATC_ENTRIES;i++) {
		save_item(NAME(mmu_atc_tag[i]), i);
		save_item(NAME(mmu_atc_data[i]), i);
	}

	machine().save().register_presave(save_prepost_delegate(FUNC(m68k_presave), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(m68k_postload), this));

	m_icountptr = &remaining_cycles;
	remaining_cycles = 0;

}

void m68000_base_device::reset_cpu(void)
{
	/* Disable the PMMU/HMMU on reset, if any */
	pmmu_enabled = 0;
	hmmu_enabled = 0;

	mmu_tc = 0;
	mmu_tt0 = 0;
	mmu_tt1 = 0;

	/* Clear all stop levels and eat up all remaining cycles */
	stopped = 0;
	if (remaining_cycles > 0)
		remaining_cycles = 0;

	run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Turn off tracing */
	t1_flag = t0_flag = 0;
	m68ki_clear_trace(this);
	/* Interrupt mask to level 7 */
	int_mask = 0x0700;
	int_level = 0;
	virq_state = 0;
	/* Reset VBR */
	vbr = 0;
	/* Go to supervisor mode */
	m68ki_set_sm_flag(this, SFLAG_SET | MFLAG_CLEAR);

	/* Invalidate the prefetch queue */
	/* Set to arbitrary number since our first fetch is from 0 */
	pref_addr = 0x1000;

	/* Read the initial stack pointer and program counter */
	m68ki_jump(this, 0);
	REG_SP(this) = m68ki_read_imm_32(this);
	REG_PC(this) = m68ki_read_imm_32(this);
	m68ki_jump(this, REG_PC(this));

	run_mode = RUN_MODE_NORMAL;

	reset_cycles = cyc_exception[EXCEPTION_RESET];

	/* flush the MMU's cache */
	pmmu_atc_flush(this);

	if(CPU_TYPE_IS_EC020_PLUS(cpu_type))
	{
		// clear instruction cache
		m68ki_ic_clear(this);
	}
}



/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void m68000_base_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case M68K_SR:
		case STATE_GENFLAGS:
			m68ki_set_sr(this, iotemp);
			break;

		case M68K_ISP:
			if (s_flag && !m_flag)
				REG_SP(this) = iotemp;
			else
				REG_ISP(this) = iotemp;
			break;

		case M68K_USP:
			if (!s_flag)
				REG_SP(this) = iotemp;
			else
				REG_USP(this) = iotemp;
			break;

		case M68K_MSP:
			if (s_flag && m_flag)
				REG_SP(this) = iotemp;
			else
				REG_MSP(this) = iotemp;
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
			iotemp = m68ki_get_sr(this);
			break;

		case M68K_ISP:
			iotemp = (s_flag && !m_flag) ? REG_SP(this) : REG_ISP(this);
			break;

		case M68K_USP:
			iotemp = (!s_flag) ? REG_SP(this) : REG_USP(this);
			break;

		case M68K_MSP:
			iotemp = (s_flag && m_flag) ? REG_SP(this) : REG_MSP(this);
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
	UINT16 sr;

	switch (entry.index())
	{
		case M68K_FP0:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[0]));
			break;

		case M68K_FP1:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[1]));
			break;

		case M68K_FP2:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[2]));
			break;

		case M68K_FP3:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[3]));
			break;

		case M68K_FP4:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[4]));
			break;

		case M68K_FP5:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[5]));
			break;

		case M68K_FP6:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[6]));
			break;

		case M68K_FP7:
			strprintf(str,"%f", fx80_to_double(REG_FP(this)[7]));
			break;

		case STATE_GENFLAGS:
			sr = m68ki_get_sr(this);
			strprintf(str,"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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
	hmmu_enabled = enable;
}

void m68000_base_device::set_instruction_hook(read32_delegate ihook)
{
	instruction_hook = ihook;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

UINT16 m68000_base_device::m68008_read_immediate_16(offs_t address)
{
	return (m_odirect->read_byte(address) << 8) | (m_odirect->read_byte(address + 1));
}

void m68000_base_device::init8(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_direct = &space.direct();
	m_ospace = &ospace;
	m_odirect = &ospace.direct();
//  m_cpustate = this;
	opcode_xor = 0;

	readimm16 = m68k_readimm16_delegate(FUNC(m68000_base_device::m68008_read_immediate_16), this);
	read8 = m68k_read8_delegate(FUNC(address_space::read_byte), &space);
	read16 = m68k_read16_delegate(FUNC(address_space::read_word), &space);
	read32 = m68k_read32_delegate(FUNC(address_space::read_dword), &space);
	write8 = m68k_write8_delegate(FUNC(address_space::write_byte), &space);
	write16 = m68k_write16_delegate(FUNC(address_space::write_word), &space);
	write32 = m68k_write32_delegate(FUNC(address_space::write_dword), &space);
}

/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

UINT16 m68000_base_device::read_immediate_16(offs_t address)
{
	return m_odirect->read_word((address), opcode_xor);
}

UINT16 m68000_base_device::simple_read_immediate_16(offs_t address)
{
	return m_odirect->read_word(address);
}

void m68000_base_device::m68000_write_byte(offs_t address, UINT8 data)
{
	static const UINT16 masks[] = {0xff00, 0x00ff};

	m_space->write_word(address & ~1, data | (data << 8), masks[address & 1]);
}

void m68000_base_device::init16(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_direct = &space.direct();
	m_ospace = &ospace;
	m_odirect = &ospace.direct();

	opcode_xor = 0;

	readimm16 = m68k_readimm16_delegate(FUNC(m68000_base_device::simple_read_immediate_16), this);
	read8 = m68k_read8_delegate(FUNC(address_space::read_byte), &space);
	read16 = m68k_read16_delegate(FUNC(address_space::read_word), &space);
	read32 = m68k_read32_delegate(FUNC(address_space::read_dword), &space);
	write8 = m68k_write8_delegate(FUNC(m68000_base_device::m68000_write_byte), this);
	write16 = m68k_write16_delegate(FUNC(address_space::write_word), &space);
	write32 = m68k_write32_delegate(FUNC(address_space::write_dword), &space);
}





/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

/* interface for 32-bit data bus (68EC020, 68020) */
void m68000_base_device::init32(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_direct = &space.direct();
	m_ospace = &ospace;
	m_odirect = &ospace.direct();
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68000_base_device::read_immediate_16), this);
	read8 = m68k_read8_delegate(FUNC(address_space::read_byte), &space);
	read16 = m68k_read16_delegate(FUNC(address_space::read_word_unaligned), &space);
	read32 = m68k_read32_delegate(FUNC(address_space::read_dword_unaligned), &space);
	write8 = m68k_write8_delegate(FUNC(address_space::write_byte), &space);
	write16 = m68k_write16_delegate(FUNC(address_space::write_word_unaligned), &space);
	write32 = m68k_write32_delegate(FUNC(address_space::write_dword_unaligned), &space);
}

/* interface for 32-bit data bus with PMMU (68EC020, 68020) */
UINT8 m68000_base_device::read_byte_32_mmu(offs_t address)
{
	if (pmmu_enabled)
	{
		address = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return ~0;
		}
	}

	return m_space->read_byte(address);
}

void m68000_base_device::write_byte_32_mmu(offs_t address, UINT8 data)
{
	if (pmmu_enabled)
	{
		address = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return;
		}
	}

	m_space->write_byte(address, data);
}

UINT16 m68000_base_device::read_immediate_16_mmu(offs_t address)
{
	if (pmmu_enabled)
	{
		address = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return ~0;
		}
	}

	return m_odirect->read_word((address), opcode_xor);
}

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT16 m68000_base_device::readword_d32_mmu(offs_t address)
{
	UINT16 result;

	if (pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return ~0;
		} else if (!(address & 1)) {
			return m_space->read_word(address0);
		} else {
			UINT32 address1 = pmmu_translate_addr(this, address + 1);
			if (mmu_tmp_buserror_occurred) {
				return ~0;
			} else {
				result = m_space->read_byte(address0) << 8;
				return result | m_space->read_byte(address1);
			}
		}
	}

	if (!(address & 1))
		return m_space->read_word(address);
	result = m_space->read_byte(address) << 8;
	return result | m_space->read_byte(address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
void m68000_base_device::writeword_d32_mmu(offs_t address, UINT16 data)
{
	if (pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return;
		} else if (!(address & 1)) {
			m_space->write_word(address0, data);
			return;
		} else {
			UINT32 address1 = pmmu_translate_addr(this, address + 1);
			if (mmu_tmp_buserror_occurred) {
				return;
			} else {
				m_space->write_byte(address0, data >> 8);
				m_space->write_byte(address1, data);
				return;
			}
		}
	}

	if (!(address & 1))
	{
		m_space->write_word(address, data);
		return;
	}
	m_space->write_byte(address, data >> 8);
	m_space->write_byte(address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT32 m68000_base_device::readlong_d32_mmu(offs_t address)
{
	UINT32 result;

	if (pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return ~0;
		} else if ((address +3) & 0xfc) {
			// not at page boundary; use default code
			address = address0;
		} else if (!(address & 3)) { // 0
			return m_space->read_dword(address0);
		} else {
			UINT32 address2 = pmmu_translate_addr(this, address+2);
			if (mmu_tmp_buserror_occurred) {
				return ~0;
			} else if (!(address & 1)) { // 2
				result = m_space->read_word(address0) << 16;
				return result | m_space->read_word(address2);
			} else {
				UINT32 address1 = pmmu_translate_addr(this, address+1);
				UINT32 address3 = pmmu_translate_addr(this, address+3);
				if (mmu_tmp_buserror_occurred) {
					return ~0;
				} else {
					result = m_space->read_byte(address0) << 24;
					result |= m_space->read_word(address1) << 8;
					return result | m_space->read_byte(address3);
				}
			}
		}
	}

	if (!(address & 3))
		return m_space->read_dword(address);
	else if (!(address & 1))
	{
		result = m_space->read_word(address) << 16;
		return result | m_space->read_word(address + 2);
	}
	result = m_space->read_byte(address) << 24;
	result |= m_space->read_word(address + 1) << 8;
	return result | m_space->read_byte(address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
void m68000_base_device::writelong_d32_mmu(offs_t address, UINT32 data)
{
	if (pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(this, address);
		if (mmu_tmp_buserror_occurred) {
			return;
		} else if ((address +3) & 0xfc) {
			// not at page boundary; use default code
			address = address0;
		} else if (!(address & 3)) { // 0
			m_space->write_dword(address0, data);
			return;
		} else {
			UINT32 address2 = pmmu_translate_addr(this, address+2);
			if (mmu_tmp_buserror_occurred) {
				return;
			} else if (!(address & 1)) { // 2
				m_space->write_word(address0, data >> 16);
				m_space->write_word(address2, data);
				return;
			} else {
				UINT32 address1 = pmmu_translate_addr(this, address+1);
				UINT32 address3 = pmmu_translate_addr(this, address+3);
				if (mmu_tmp_buserror_occurred) {
					return;
				} else {
					m_space->write_byte(address0, data >> 24);
					m_space->write_word(address1, data >> 8);
					m_space->write_byte(address3, data);
					return;
				}
			}
		}
	}

	if (!(address & 3))
	{
		m_space->write_dword(address, data);
		return;
	}
	else if (!(address & 1))
	{
		m_space->write_word(address, data >> 16);
		m_space->write_word(address + 2, data);
		return;
	}
	m_space->write_byte(address, data >> 24);
	m_space->write_word(address + 1, data >> 8);
	m_space->write_byte(address + 3, data);
}

void m68000_base_device::init32mmu(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_direct = &space.direct();
	m_ospace = &ospace;
	m_odirect = &ospace.direct();
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68000_base_device::read_immediate_16_mmu), this);
	read8 = m68k_read8_delegate(FUNC(m68000_base_device::read_byte_32_mmu), this);
	read16 = m68k_read16_delegate(FUNC(m68000_base_device::readword_d32_mmu), this);
	read32 = m68k_read32_delegate(FUNC(m68000_base_device::readlong_d32_mmu), this);
	write8 = m68k_write8_delegate(FUNC(m68000_base_device::write_byte_32_mmu), this);
	write16 = m68k_write16_delegate(FUNC(m68000_base_device::writeword_d32_mmu), this);
	write32 = m68k_write32_delegate(FUNC(m68000_base_device::writelong_d32_mmu), this);
}


/* interface for 32-bit data bus with PMMU (68EC020, 68020) */
UINT8 m68000_base_device::read_byte_32_hmmu(offs_t address)
{
	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	return m_space->read_byte(address);
}

void m68000_base_device::write_byte_32_hmmu(offs_t address, UINT8 data)
{
	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	m_space->write_byte(address, data);
}

UINT16 m68000_base_device::read_immediate_16_hmmu(offs_t address)
{
	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	return m_odirect->read_word((address), opcode_xor);
}

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT16 m68000_base_device::readword_d32_hmmu(offs_t address)
{
	UINT16 result;

	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	if (!(address & 1))
		return m_space->read_word(address);
	result = m_space->read_byte(address) << 8;
	return result | m_space->read_byte(address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
void m68000_base_device::writeword_d32_hmmu(offs_t address, UINT16 data)
{
	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	if (!(address & 1))
	{
		m_space->write_word(address, data);
		return;
	}
	m_space->write_byte(address, data >> 8);
	m_space->write_byte(address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT32 m68000_base_device::readlong_d32_hmmu(offs_t address)
{
	UINT32 result;

	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	if (!(address & 3))
		return m_space->read_dword(address);
	else if (!(address & 1))
	{
		result = m_space->read_word(address) << 16;
		return result | m_space->read_word(address + 2);
	}
	result = m_space->read_byte(address) << 24;
	result |= m_space->read_word(address + 1) << 8;
	return result | m_space->read_byte(address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
void m68000_base_device::writelong_d32_hmmu(offs_t address, UINT32 data)
{
	if (hmmu_enabled)
	{
		address = hmmu_translate_addr(this, address);
	}

	if (!(address & 3))
	{
		m_space->write_dword(address, data);
		return;
	}
	else if (!(address & 1))
	{
		m_space->write_word(address, data >> 16);
		m_space->write_word(address + 2, data);
		return;
	}
	m_space->write_byte(address, data >> 24);
	m_space->write_word(address + 1, data >> 8);
	m_space->write_byte(address + 3, data);
}

void m68000_base_device::init32hmmu(address_space &space, address_space &ospace)
{
	m_space = &space;
	m_direct = &space.direct();
	m_ospace = &ospace;
	m_odirect = &ospace.direct();
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68000_base_device::read_immediate_16_hmmu), this);
	read8 = m68k_read8_delegate(FUNC(m68000_base_device::read_byte_32_hmmu), this);
	read16 = m68k_read16_delegate(FUNC(m68000_base_device::readword_d32_hmmu), this);
	read32 = m68k_read32_delegate(FUNC(m68000_base_device::readlong_d32_hmmu), this);
	write8 = m68k_write8_delegate(FUNC(m68000_base_device::write_byte_32_hmmu), this);
	write16 = m68k_write16_delegate(FUNC(m68000_base_device::writeword_d32_hmmu), this);
	write32 = m68k_write32_delegate(FUNC(m68000_base_device::writelong_d32_hmmu), this);
}

void m68000_base_device::set_reset_callback(write_line_delegate callback)
{
	reset_instr_callback = callback;
}

// fault_addr = address to indicate fault at
// rw = 0 for read, 1 for write
// fc = 3-bit function code of access (usually you'd just put what m68k_get_fc() returns here)
void m68000_base_device::set_buserror_details(UINT32 fault_addr, UINT8 rw, UINT8 fc)
{
	aerr_address = fault_addr;
	aerr_write_mode = rw;
	aerr_fc = fc;
}

void m68000_base_device::set_cmpild_callback(write32_delegate callback)
{
	cmpild_instr_callback = callback;
}

void m68000_base_device::set_rte_callback(write_line_delegate callback)
{
	rte_instr_callback = callback;
}

void m68000_base_device::set_tas_write_callback(write8_delegate callback)
{
	tas_write_callback = callback;
}

UINT16 m68000_base_device::get_fc()
{
	return mmu_tmp_fc;
}

/****************************************************************************
 * State definition
 ****************************************************************************/

void m68000_base_device::define_state(void)
{
	UINT32 addrmask = (cpu_type & MASK_24BIT_SPACE) ? 0xffffff : 0xffffffff;

	state_add(M68K_PC,         "PC",        pc).mask(addrmask);
	state_add(STATE_GENPC,     "GENPC",     pc).mask(addrmask).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", ppc).mask(addrmask).noshow();
	state_add(M68K_SP,         "SP",        dar[15]);
	state_add(STATE_GENSP,     "GENSP",     dar[15]).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  iotemp).noshow().callimport().callexport().formatstr("%16s");
	state_add(M68K_ISP,        "ISP",       iotemp).callimport().callexport();
	state_add(M68K_USP,        "USP",       iotemp).callimport().callexport();
	if (cpu_type & MASK_020_OR_LATER)
		state_add(M68K_MSP,    "MSP",       iotemp).callimport().callexport();

	for (int regnum = 0; regnum < 8; regnum++) {
		state_add(M68K_D0 + regnum, strformat("D%d", regnum).c_str(), dar[regnum]);
	}
	for (int regnum = 0; regnum < 8; regnum++) {
		state_add(M68K_A0 + regnum, strformat("A%d", regnum).c_str(), dar[8 + regnum]);
	}

	state_add(M68K_PREF_ADDR,  "PREF_ADDR", pref_addr).mask(addrmask);
	state_add(M68K_PREF_DATA,  "PREF_DATA", pref_data);

	if (cpu_type & MASK_010_OR_LATER)
	{
		state_add(M68K_SFC,    "SFC",       sfc).mask(0x7);
		state_add(M68K_DFC,    "DFC",       dfc).mask(0x7);
		state_add(M68K_VBR,    "VBR",       vbr);
	}

	if (cpu_type & MASK_020_OR_LATER)
	{
		state_add(M68K_CACR,   "CACR",      cacr);
		state_add(M68K_CAAR,   "CAAR",      caar);
	}

	if (cpu_type & MASK_030_OR_LATER)
	{
		for (int regnum = 0; regnum < 8; regnum++) {
			state_add(M68K_FP0 + regnum, strformat("FP%d", regnum).c_str(), iotemp).callimport().callexport().formatstr("%10s");
		}
		state_add(M68K_FPSR, "FPSR", fpsr);
		state_add(M68K_FPCR, "FPCR", fpcr);
	}
}



/****************
 CPU Inits
****************/


void m68000_base_device::init_cpu_m68000(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_000;
//  dasm_type        = M68K_CPU_TYPE_68000;

	init16(*program, *oprogram);
	sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[0];
	cyc_instruction  = m68ki_cycles[0];
	cyc_exception    = m68ki_exception_cycle_table[0];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 2;
	cyc_dbcc_f_noexp = -2;
	cyc_dbcc_f_exp   = 2;
	cyc_scc_r_true   = 2;
	cyc_movem_w      = 2;
	cyc_movem_l      = 3;
	cyc_shift        = 1;
	cyc_reset        = 132;
	has_pmmu         = 0;
	has_hmmu         = 0;
	has_fpu          = 0;

	define_state();

}


void m68000_base_device::init_cpu_m68008(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_008;
//  dasm_type        = M68K_CPU_TYPE_68008;

	init8(*program, *oprogram);
	sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[0];
	cyc_instruction  = m68ki_cycles[0];
	cyc_exception    = m68ki_exception_cycle_table[0];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 2;
	cyc_dbcc_f_noexp = -2;
	cyc_dbcc_f_exp   = 2;
	cyc_scc_r_true   = 2;
	cyc_movem_w      = 2;
	cyc_movem_l      = 3;
	cyc_shift        = 1;
	cyc_reset        = 132;
	has_pmmu         = 0;
	has_fpu          = 0;

	define_state();

}


void m68000_base_device::init_cpu_m68010(void)
{
	init_cpu_common();
	cpu_type         = CPU_TYPE_010;
//  dasm_type        = M68K_CPU_TYPE_68010;

	init16(*program, *oprogram);
	sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[1];
	cyc_instruction  = m68ki_cycles[1];
	cyc_exception    = m68ki_exception_cycle_table[1];
	cyc_bcc_notake_b = -4;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 6;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 3;
	cyc_shift        = 1;
	cyc_reset        = 130;
	has_pmmu         = 0;
	has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68020(void)
{
	init_cpu_common();
	cpu_type         = CPU_TYPE_020;
//  dasm_type        = M68K_CPU_TYPE_68020;

	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[2];
	cyc_instruction  = m68ki_cycles[2];
	cyc_exception    = m68ki_exception_cycle_table[2];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;

	define_state();
}

void m68000_base_device::init_cpu_m68020fpu(void)
{
	init_cpu_m68020();

	has_fpu          = 1;
}

void m68000_base_device::init_cpu_m68020pmmu(void)
{
	init_cpu_m68020();

	has_pmmu         = 1;
	has_fpu          = 1;


	init32mmu(*program, *oprogram);
}



void m68000_base_device::init_cpu_m68020hmmu(void)
{
	init_cpu_m68020();

	has_hmmu = 1;
	has_fpu  = 1;


	init32hmmu(*program, *oprogram);
}

void m68000_base_device::init_cpu_m68ec020(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_EC020;
//  dasm_type        = M68K_CPU_TYPE_68EC020;


	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[2];
	cyc_instruction  = m68ki_cycles[2];
	cyc_exception    = m68ki_exception_cycle_table[2];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 0;
	has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68030(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_030;
//  dasm_type        = M68K_CPU_TYPE_68030;


	init32mmu(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[3];
	cyc_instruction  = m68ki_cycles[3];
	cyc_exception    = m68ki_exception_cycle_table[3];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 1;
	has_fpu          = 1;

	define_state();
}



void m68000_base_device::init_cpu_m68ec030(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_EC030;
//  dasm_type        = M68K_CPU_TYPE_68EC030;


	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[3];
	cyc_instruction  = m68ki_cycles[3];
	cyc_exception    = m68ki_exception_cycle_table[3];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 0;     /* EC030 lacks the PMMU and is effectively a die-shrink 68020 */
	has_fpu          = 1;

	define_state();
}



void m68000_base_device::init_cpu_m68040(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_040;
//  dasm_type        = M68K_CPU_TYPE_68040;


	init32mmu(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[4];
	cyc_instruction  = m68ki_cycles[4];
	cyc_exception    = m68ki_exception_cycle_table[4];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 1;
	has_fpu          = 1;

	define_state();
}


void m68000_base_device::init_cpu_m68ec040(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_EC040;
//  dasm_type        = M68K_CPU_TYPE_68EC040;


	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[4];
	cyc_instruction  = m68ki_cycles[4];
	cyc_exception    = m68ki_exception_cycle_table[4];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 0;
	has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_m68lc040(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_LC040;
//  dasm_type        = M68K_CPU_TYPE_68LC040;


	init32mmu(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[4];
	cyc_instruction  = m68ki_cycles[4];
	cyc_exception    = m68ki_exception_cycle_table[4];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;
	has_pmmu         = 1;
	has_fpu          = 0;

	define_state();
}


void m68000_base_device::init_cpu_scc68070(void)
{
	init_cpu_m68010();
	cpu_type         = CPU_TYPE_SCC070;
}


void m68000_base_device::init_cpu_fscpu32(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_FSCPU32;
//  dasm_type        = M68K_CPU_TYPE_FSCPU32;


	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[5];
	cyc_instruction  = m68ki_cycles[5];
	cyc_exception    = m68ki_exception_cycle_table[5];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;

	define_state();
}



void m68000_base_device::init_cpu_coldfire(void)
{
	init_cpu_common();

	cpu_type         = CPU_TYPE_COLDFIRE;
//  dasm_type        = M68K_CPU_TYPE_COLDFIRE;


	init32(*program, *oprogram);
	sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	jump_table       = m68ki_instruction_jump_table[6];
	cyc_instruction  = m68ki_cycles[6];
	cyc_exception    = m68ki_exception_cycle_table[6];
	cyc_bcc_notake_b = -2;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp   = 4;
	cyc_scc_r_true   = 0;
	cyc_movem_w      = 2;
	cyc_movem_l      = 2;
	cyc_shift        = 0;
	cyc_reset        = 518;

	define_state();
}

CPU_DISASSEMBLE( dasm_m68000 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68000);
}

CPU_DISASSEMBLE( dasm_m68008 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68008);
}

CPU_DISASSEMBLE( dasm_m68010 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68010);
}

CPU_DISASSEMBLE( dasm_m68020 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68020);
}

CPU_DISASSEMBLE( dasm_m68030 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68030);
}

CPU_DISASSEMBLE( dasm_m68ec030 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68EC030);
}

CPU_DISASSEMBLE( dasm_m68040 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68040);
}

CPU_DISASSEMBLE( dasm_m68ec040 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68EC040);
}

CPU_DISASSEMBLE( dasm_m68lc040 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68LC040);
}

CPU_DISASSEMBLE( dasm_fscpu32 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_FSCPU32);
}

CPU_DISASSEMBLE( dasm_coldfire )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_COLDFIRE);
}

offs_t m68000_base_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68000)(this, buffer, pc, oprom, opram, options); }
offs_t m68000_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68000)(this, buffer, pc, oprom, opram, options); }
offs_t m68301_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68000)(this, buffer, pc, oprom, opram, options); }
offs_t m68008_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68008)(this, buffer, pc, oprom, opram, options); }
offs_t m68008plcc_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68008)(this, buffer, pc, oprom, opram, options); }
offs_t m68010_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68010)(this, buffer, pc, oprom, opram, options); }
offs_t m68ec020_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68020)(this, buffer, pc, oprom, opram, options); }
offs_t m68020_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68020)(this, buffer, pc, oprom, opram, options); }
offs_t m68020fpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68020)(this, buffer, pc, oprom, opram, options); }
offs_t m68020pmmu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68020)(this, buffer, pc, oprom, opram, options); }
offs_t m68020hmmu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68020)(this, buffer, pc, oprom, opram, options); }
offs_t m68ec030_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68ec030)(this, buffer, pc, oprom, opram, options); }
offs_t m68030_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68030)(this, buffer, pc, oprom, opram, options); }
offs_t m68ec040_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68ec040)(this, buffer, pc, oprom, opram, options); }
offs_t m68lc040_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68lc040)(this, buffer, pc, oprom, opram, options); }
offs_t m68040_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68040)(this, buffer, pc, oprom, opram, options); }
offs_t scc68070_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_m68000)(this, buffer, pc, oprom, opram, options); }
offs_t fscpu32_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_fscpu32)(this, buffer, pc, oprom, opram, options); }
offs_t mcf5206e_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) { return CPU_DISASSEMBLE_NAME(dasm_coldfire)(this, buffer, pc, oprom, opram, options); }


/* Service an interrupt request and start exception processing */
void m68000_base_device::m68ki_exception_interrupt(m68000_base_device *m68k, UINT32 int_level)
{
	UINT32 vector;
	UINT32 sr;
	UINT32 new_pc;

	if(CPU_TYPE_IS_000(cpu_type))
	{
		instr_mode = INSTRUCTION_NO;
	}

	/* Turn off the stopped state */
	stopped &= ~STOP_LEVEL_STOP;

	/* If we are halted, don't do anything */
	if(stopped)
		return;

	/* Acknowledge the interrupt */
	vector = int_ack_callback(*this, int_level);

	/* Get the interrupt vector */
	if(vector == M68K_INT_ACK_AUTOVECTOR)
		/* Use the autovectors.  This is the most commonly used implementation */
		vector = EXCEPTION_INTERRUPT_AUTOVECTOR+int_level;
	else if(vector == M68K_INT_ACK_SPURIOUS)
		/* Called if no devices respond to the interrupt acknowledge */
		vector = EXCEPTION_SPURIOUS_INTERRUPT;
	else if(vector > 255)
		return;

	/* Start exception processing */
	sr = m68ki_init_exception(m68k);

	/* Set the interrupt mask to the level of the one being serviced */
	int_mask = int_level<<8;

	/* Get the new PC */
	new_pc = m68ki_read_data_32(this, (vector<<2) + vbr);

	/* If vector is uninitialized, call the uninitialized interrupt vector */
	if(new_pc == 0)
		new_pc = m68ki_read_data_32(this, (EXCEPTION_UNINITIALIZED_INTERRUPT<<2) + vbr);

	/* Generate a stack frame */
	m68ki_stack_frame_0000(this, REG_PC(m68k), sr, vector);
	if(m_flag && CPU_TYPE_IS_EC020_PLUS(cpu_type))
	{
		/* Create throwaway frame */
		m68ki_set_sm_flag(this, s_flag);  /* clear M */
		sr |= 0x2000; /* Same as SR in master stack frame except S is forced high */
		m68ki_stack_frame_0001(this, REG_PC(m68k), sr, vector);
	}

	m68ki_jump(this, new_pc);

	/* Defer cycle counting until later */
	remaining_cycles -= cyc_exception[vector];
}


const device_type M68K = &device_creator<m68000_base_device>;

//-------------------------------------------------
//  h6280_device - constructor
//-------------------------------------------------

m68000_base_device::m68000_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, M68K, "M68K", tag, owner, clock, "m68k", __FILE__),
		m_program_config("program", ENDIANNESS_BIG, 16, 24),
		m_oprogram_config("decrypted_opcodes", ENDIANNESS_BIG, 16, 24)
{
	clear_all();
}




m68000_base_device::m68000_base_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
										const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, internal_map),
		m_oprogram_config("decrypted_opcodes", ENDIANNESS_BIG, prg_data_width, prg_address_bits, 0, internal_map)
{
	clear_all();
}


m68000_base_device::m68000_base_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
										const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, prg_address_bits),
		m_oprogram_config("decrypted_opcodes", ENDIANNESS_BIG, prg_data_width, prg_address_bits)
{
	clear_all();
}

void m68000_base_device::clear_all()
{
	cpu_type= 0;
//  dasm_type= 0;
	for (auto & elem : dar)
		elem= 0;
	ppc= 0;
	pc= 0;
	for (auto & elem : sp)
		elem= 0;
	vbr= 0;
	sfc= 0;
	dfc= 0;
	cacr= 0;
	caar= 0;
	ir= 0;
//  for (int i=0;i<8;i++)
//      fpr[i]= 0;
	fpiar= 0;
	fpsr= 0;
	fpcr= 0;
	t1_flag= 0;
	t0_flag= 0;
	s_flag= 0;
	m_flag= 0;
	x_flag= 0;
	n_flag= 0;
	not_z_flag= 0;
	v_flag= 0;
	c_flag= 0;
	int_mask= 0;
	int_level= 0;
	stopped= 0;
	pref_addr= 0;
	pref_data= 0;
	sr_mask= 0;
	instr_mode= 0;
	run_mode= 0;
	has_pmmu= 0;
	has_hmmu= 0;
	pmmu_enabled= 0;
	hmmu_enabled= 0;
	has_fpu= 0;
	fpu_just_reset= 0;

	cyc_bcc_notake_b = 0;
	cyc_bcc_notake_w = 0;
	cyc_dbcc_f_noexp = 0;
	cyc_dbcc_f_exp = 0;
	cyc_scc_r_true = 0;
	cyc_movem_w = 0;
	cyc_movem_l = 0;
	cyc_shift = 0;
	cyc_reset = 0;

	initial_cycles = 0;
	remaining_cycles = 0;
	reset_cycles = 0;
	tracing = 0;

	m_address_error = 0;

	aerr_address = 0;
	aerr_write_mode = 0;
	aerr_fc = 0;

	virq_state = 0;
	nmi_pending = 0;

	cyc_instruction = nullptr;
	cyc_exception = nullptr;

	int_ack_callback = device_irq_acknowledge_delegate();
	program = nullptr;

	opcode_xor = 0;
//  readimm16 = 0;
//  read8 = 0;
//  read16 = 0;
//  read32 = 0;
//  write8 = 0;
//  write16 = 0;
//  write32 = 0;

	m_space = nullptr;
	m_direct = nullptr;


	iotemp = 0;

	save_sr = 0;
	save_stopped = 0;
	save_halted = 0;


	mmu_crp_aptr = mmu_crp_limit = 0;
	mmu_srp_aptr = mmu_srp_limit = 0;
	mmu_urp_aptr = 0;
	mmu_tc = 0;
	mmu_sr = 0;
	mmu_sr_040 = 0;

	for (int i=0; i<MMU_ATC_ENTRIES;i++)
		mmu_atc_tag[i] = mmu_atc_data[i] = 0;

	mmu_atc_rr = 0;
	mmu_tt0 = mmu_tt1 = 0;
	mmu_itt0 = mmu_itt1 = mmu_dtt0 = mmu_dtt1 = 0;
	mmu_acr0= mmu_acr1 = mmu_acr2 = mmu_acr3 = 0;
	mmu_tmp_sr = 0;
	mmu_tmp_fc = 0;
	mmu_tmp_rw = 0;
	mmu_tmp_buserror_address = 0;
	mmu_tmp_buserror_occurred = 0;
	mmu_tmp_buserror_fc = 0;
	mmu_tmp_buserror_rw = 0;

	for (int i=0;i<M68K_IC_SIZE;i++)
	{
		ic_address[i] = 0;
		ic_data[i] = 0;
		ic_valid[i] = false;
	}

	internal = nullptr;
}


void m68000_base_device::execute_run()
{
	cpu_execute();
}

void m68000_base_device::device_start()
{
}

void m68000_base_device::device_reset()
{
	reset_cpu();
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
			set_irq_line(this, inputnum, state);
			break;

		case M68K_LINE_BUSERROR:
			if (state == ASSERT_LINE)
			{
				m68k_cause_bus_error(this);
			}
			break;
	}
}


const address_space_config *m68000_base_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &m_oprogram_config : nullptr;
	default:                   return nullptr;
	}
}



const device_type M68000 = &device_creator<m68000_device>;
const device_type M68301 = &device_creator<m68301_device>;
const device_type M68008 = &device_creator<m68008_device>;
const device_type M68008PLCC = &device_creator<m68008plcc_device>;
const device_type M68010 = &device_creator<m68010_device>;
const device_type M68EC020 = &device_creator<m68ec020_device>;
const device_type M68020 = &device_creator<m68020_device>;
const device_type M68020FPU = &device_creator<m68020fpu_device>;
const device_type M68020PMMU = &device_creator<m68020pmmu_device>;
const device_type M68020HMMU = &device_creator<m68020hmmu_device>;
const device_type M68EC030 = &device_creator<m68ec030_device>;
const device_type M68030 = &device_creator<m68030_device>;
const device_type M68EC040 = &device_creator<m68ec040_device>;
const device_type M68LC040 = &device_creator<m68lc040_device>;
const device_type M68040 = &device_creator<m68040_device>;
const device_type SCC68070 = &device_creator<scc68070_device>;
const device_type FSCPU32 = &device_creator<fscpu32_device>;
const device_type MCF5206E = &device_creator<mcf5206e_device>;

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68000", tag, owner, clock, M68000, 16,24, "m68000", __FILE__)
{
}

m68000_device::m68000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: m68000_base_device(mconfig, "M68000", tag, owner, clock, M68000, 16,24, shortname, source)
{
}

void m68000_device::device_start()
{
	init_cpu_m68000();
}

m68000_device::m68000_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
										const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source)
	: m68000_base_device(mconfig, name, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map, shortname, source)
{
}





m68301_device::m68301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68301", tag, owner, clock, M68301, 16,24, "m68301", __FILE__)
{
}


void m68301_device::device_start()
{
	init_cpu_m68000();
}






/* m68008_device */

m68008_device::m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68008", tag, owner, clock, M68008, 8,20, "m68008", __FILE__)
{
}

void m68008_device::device_start()
{
	init_cpu_m68008();
}


m68008plcc_device::m68008plcc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68008PLCC", tag, owner, clock, M68008, 8,22, "m68008plcc", __FILE__)
{
}

void m68008plcc_device::device_start()
{
	init_cpu_m68008();
}



m68010_device::m68010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68010", tag, owner, clock, M68010, 16,24, "m68010", __FILE__)
{
}

void m68010_device::device_start()
{
	init_cpu_m68010();
}



m68020_device::m68020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68020", tag, owner, clock, M68020, 32,32, "m68020", __FILE__)
{
}

void m68020_device::device_start()
{
	init_cpu_m68020();
}


m68020fpu_device::m68020fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68020FPU", tag, owner, clock, M68020, 32,32, "m68020fpu", __FILE__)
{
}

void m68020fpu_device::device_start()
{
	init_cpu_m68020fpu();
}

// 68020 with 68851 PMMU
m68020pmmu_device::m68020pmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68020PMMU", tag, owner, clock, M68020PMMU, 32,32, "m68020pmmu", __FILE__)
{
}

void m68020pmmu_device::device_start()
{
	init_cpu_m68020pmmu();
}

bool m68020hmmu_device::memory_translate(address_spacenum space, int intention, offs_t &address)
{
	/* only applies to the program address space and only does something if the MMU's enabled */
	if (this)
	{
		if ((space == AS_PROGRAM) && (hmmu_enabled))
		{
			address = hmmu_translate_addr(this, address);
		}
	}
	return TRUE;
}


// 68020 with Apple HMMU & 68881 FPU
//      case CPUINFO_FCT_TRANSLATE: info->translate = CPU_TRANSLATE_NAME(m68khmmu);     break;
m68020hmmu_device::m68020hmmu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68020HMMU", tag, owner, clock, M68020HMMU, 32,32, "m68020hmmu", __FILE__)
{
}

void m68020hmmu_device::device_start()
{
	init_cpu_m68020hmmu();
}


m68ec020_device::m68ec020_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68EC020", tag, owner, clock, M68EC020, 32,24, "m68ec020", __FILE__)
{
}

void m68ec020_device::device_start()
{
	init_cpu_m68ec020();
}

m68030_device::m68030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68030", tag, owner, clock, M68030, 32,32, "m68030", __FILE__)
{
}

void m68030_device::device_start()
{
	init_cpu_m68030();
}

m68ec030_device::m68ec030_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68EC030", tag, owner, clock, M68EC030, 32,32, "m68ec030", __FILE__)
{
}

void m68ec030_device::device_start()
{
	init_cpu_m68ec030();
}

m68040_device::m68040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68040", tag, owner, clock, M68040, 32,32, "m68040", __FILE__)
{
}


void m68040_device::device_start()
{
	init_cpu_m68040();
}



m68ec040_device::m68ec040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68EC040", tag, owner, clock, M68EC040, 32,32, "m68ec040", __FILE__)
{
}

void m68ec040_device::device_start()
{
	init_cpu_m68ec040();
}



m68lc040_device::m68lc040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "M68LC040", tag, owner, clock, M68LC040, 32,32, "m68lc040", __FILE__)
{
}

void m68lc040_device::device_start()
{
	init_cpu_m68lc040();
}


scc68070_device::scc68070_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "SCC68070", tag, owner, clock, SCC68070, 16,32, "scc68070", __FILE__)
{
}

void scc68070_device::device_start()
{
	init_cpu_scc68070();
}


fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "Freescale CPU32 Core", tag, owner, clock, FSCPU32, 32,32, "fscpu32", __FILE__)
{
}

fscpu32_device::fscpu32_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
										const device_type type, UINT32 prg_data_width, UINT32 prg_address_bits, address_map_constructor internal_map, const char *shortname, const char *source)
	: m68000_base_device(mconfig, name, tag, owner, clock, type, prg_data_width, prg_address_bits, internal_map, shortname, source)
{
}


void fscpu32_device::device_start()
{
	init_cpu_fscpu32();
}



mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: m68000_base_device(mconfig, "MCF5206E", tag, owner, clock, MCF5206E, 32,32, "mcf5206e", __FILE__)
{
}

void mcf5206e_device::device_start()
{
	init_cpu_coldfire();
}
