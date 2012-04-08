/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */

#if 0
static const char copyright_notice[] =
"MUSASHI\n"
"Version 4.95 (2012-02-19)\n"
"A portable Motorola M68xxx/CPU32/ColdFire processor emulation engine.\n"
"Copyright Karl Stenerud.  All rights reserved.\n"
"\n"
"This code may be freely used for non-commercial purpooses as long as this\n"
"copyright notice remains unaltered in the source code and any binary files\n"
"containing this code in compiled form.\n"
"\n"
"All other licensing terms must be negotiated with the author\n"
"(Karl Stenerud).\n"
"\n"
"The latest version of this code can be obtained at:\n"
"http://kstenerud.cjb.net or http://mamedev.org/\n"
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
#include <setjmp.h>
#include "m68kcpu.h"
#include "m68kops.h"
#include "m68kfpu.c"

#include "m68kmmu.h"

extern void m68040_fpu_op0(m68ki_cpu_core *m68k);
extern void m68040_fpu_op1(m68ki_cpu_core *m68k);
extern void m68881_mmu_ops(m68ki_cpu_core *m68k);

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
	{ /* 68340 */
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

#define MASK_ALL				(CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_68340 )
#define MASK_24BIT_SPACE			(CPU_TYPE_000 | CPU_TYPE_008 | CPU_TYPE_010 | CPU_TYPE_EC020)
#define MASK_32BIT_SPACE			(CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_68340 )
#define MASK_010_OR_LATER			(CPU_TYPE_010 | CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040 | CPU_TYPE_68340)
#define MASK_020_OR_LATER			(CPU_TYPE_EC020 | CPU_TYPE_020 | CPU_TYPE_EC030 | CPU_TYPE_030 | CPU_TYPE_EC040 | CPU_TYPE_040 | CPU_TYPE_68340)
#define MASK_030_OR_LATER			(CPU_TYPE_030 | CPU_TYPE_EC030 | CPU_TYPE_040 | CPU_TYPE_EC040)
#define MASK_040_OR_LATER			(CPU_TYPE_040 | CPU_TYPE_EC040)



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

static void set_irq_line(m68ki_cpu_core *m68k, int irqline, int state)
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

static void m68k_presave(m68ki_cpu_core *m68k)
{
	m68k->save_sr = m68ki_get_sr(m68k);
	m68k->save_stopped = (m68k->stopped & STOP_LEVEL_STOP) != 0;
	m68k->save_halted  = (m68k->stopped & STOP_LEVEL_HALT) != 0;
}

static void m68k_postload(m68ki_cpu_core *m68k)
{
	m68ki_set_sr_noint_nosp(m68k, m68k->save_sr);
	m68k->stopped = m68k->save_stopped ? STOP_LEVEL_STOP : 0
		        | m68k->save_halted  ? STOP_LEVEL_HALT : 0;
	m68ki_jump(m68k, REG_PC(m68k));
}

static void m68k_cause_bus_error(m68ki_cpu_core *m68k)
{
	UINT32 sr;

	sr = m68ki_init_exception(m68k);

	m68k->run_mode = RUN_MODE_BERR_AERR_RESET;

	if (!CPU_TYPE_IS_020_PLUS(m68k->cpu_type))
	{
		/* Note: This is implemented for 68000 only! */
		m68ki_stack_frame_buserr(m68k, sr);
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

/* translate logical to physical addresses */
static CPU_TRANSLATE( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	/* only applies to the program address space and only does something if the MMU's enabled */
	if (m68k)
	{
		/* 68040 needs to call the MMU even when disabled so transparent translation works */
		if ((space == AS_PROGRAM) && ((m68k->pmmu_enabled) || (CPU_TYPE_IS_040_PLUS(m68k->cpu_type))))
		{
			// FIXME: mmu_tmp_sr will be overwritten in pmmu_translate_addr_with_fc
			UINT16 mmu_tmp_sr = m68k->mmu_tmp_sr;
			int mode = m68k->s_flag ? FUNCTION_CODE_SUPERVISOR_PROGRAM : FUNCTION_CODE_USER_PROGRAM;
//          UINT32 va=*address;

			if (CPU_TYPE_IS_040_PLUS(m68k->cpu_type))
			{
				*address = pmmu_translate_addr_with_fc_040(m68k, *address, mode, 1);
			}
			else
			{
				*address = pmmu_translate_addr_with_fc(m68k, *address, mode, 1);
			}

			if ((m68k->mmu_tmp_sr & M68K_MMU_SR_INVALID) != 0) {
//              logerror("cpu_translate_m68k failed with mmu_sr=%04x va=%08x pa=%08x\n",m68k->mmu_tmp_sr,va ,*address);
				*address = 0;
			}

			m68k->mmu_tmp_sr = mmu_tmp_sr;
		}
	}
	return TRUE;
}

/* translate logical to physical addresses for Apple HMMU */
static CPU_TRANSLATE( m68khmmu )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	/* only applies to the program address space and only does something if the MMU's enabled */
	if (m68k)
	{
		if ((space == AS_PROGRAM) && (m68k->hmmu_enabled))
		{
			*address = hmmu_translate_addr(m68k, *address);
		}
	}
	return TRUE;
}

/* Execute some instructions until we use up cycles clock cycles */
static CPU_EXECUTE( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	m68k->initial_cycles = m68k->remaining_cycles;

	/* eat up any reset cycles */
	if (m68k->reset_cycles) {
		int rc = m68k->reset_cycles;
		m68k->reset_cycles = 0;
		m68k->remaining_cycles -= rc;

		if (m68k->remaining_cycles <= 0) return;
	}

	/* See if interrupts came in */
	m68ki_check_interrupts(m68k);

	/* Make sure we're not stopped */
	if(!m68k->stopped)
	{
        /* Return point if we had an address error */
		m68ki_set_address_error_trap(m68k); /* auto-disable (see m68kcpu.h) */

		/* Main loop.  Keep going until we run out of clock cycles */
		while (m68k->remaining_cycles > 0)
		{
			/* Set tracing accodring to T1. (T0 is done inside instruction) */
			m68ki_trace_t1(m68k); /* auto-disable (see m68kcpu.h) */

			/* Call external hook to peek at CPU */
			debugger_instruction_hook(device, REG_PC(m68k));

			/* call external instruction hook (independent of debug mode) */
			if (m68k->instruction_hook != NULL)
				m68k->instruction_hook(device, REG_PC(m68k));

			/* Record previous program counter */
			REG_PPC(m68k) = REG_PC(m68k);

			if (!m68k->pmmu_enabled)
			{
				m68k->run_mode = RUN_MODE_NORMAL;
				/* Read an instruction and call its handler */
				m68k->ir = m68ki_read_imm_16(m68k);
				m68k->jump_table[m68k->ir](m68k);
				m68k->remaining_cycles -= m68k->cyc_instruction[m68k->ir];
			}
			else
			{
				m68k->run_mode = RUN_MODE_NORMAL;
				// save CPU address registers values at start of instruction
				int i;
				UINT32 tmp_dar[16];

				for (i = 15; i >= 0; i--)
				{
					tmp_dar[i] = REG_DA(m68k)[i];
				}

				m68k->mmu_tmp_buserror_occurred = 0;

				/* Read an instruction and call its handler */
				m68k->ir = m68ki_read_imm_16(m68k);

				if (!m68k->mmu_tmp_buserror_occurred)
				{
					m68k->jump_table[m68k->ir](m68k);
					m68k->remaining_cycles -= m68k->cyc_instruction[m68k->ir];
				}

				if (m68k->mmu_tmp_buserror_occurred)
				{
					UINT32 sr;

					m68k->mmu_tmp_buserror_occurred = 0;

					// restore cpu address registers to value at start of instruction
					for (i = 15; i >= 0; i--)
					{
						if (REG_DA(m68k)[i] != tmp_dar[i])
						{
//                          logerror("PMMU: pc=%08x sp=%08x bus error: fixed %s[%d]: %08x -> %08x\n",
//                                  REG_PPC(m68k), REG_A(m68k)[7], i < 8 ? "D" : "A", i & 7, REG_DA(m68k)[i], tmp_dar[i]);
							REG_DA(m68k)[i] = tmp_dar[i];
						}
					}

					sr = m68ki_init_exception(m68k);

					m68k->run_mode = RUN_MODE_BERR_AERR_RESET;

					if (!CPU_TYPE_IS_020_PLUS(m68k->cpu_type))
					{
						/* Note: This is implemented for 68000 only! */
						m68ki_stack_frame_buserr(m68k, sr);
					}
					else if(!CPU_TYPE_IS_040_PLUS(m68k->cpu_type)) {
						if (m68k->mmu_tmp_buserror_address == REG_PPC(m68k))
						{
							m68ki_stack_frame_1010(m68k, sr, EXCEPTION_BUS_ERROR, REG_PPC(m68k), m68k->mmu_tmp_buserror_address);
						}
						else
						{
							m68ki_stack_frame_1011(m68k, sr, EXCEPTION_BUS_ERROR, REG_PPC(m68k), m68k->mmu_tmp_buserror_address);
						}
					}
					else
					{
						m68ki_stack_frame_0111(m68k, sr, EXCEPTION_BUS_ERROR, REG_PPC(m68k), m68k->mmu_tmp_buserror_address, true);
					}

					m68ki_jump_vector(m68k, EXCEPTION_BUS_ERROR);

					// TODO:
					/* Use up some clock cycles and undo the instruction's cycles */
					// m68k->remaining_cycles -= m68k->cyc_exception[EXCEPTION_BUS_ERROR] - m68k->cyc_instruction[m68k->ir];
				}
			}

			/* Trace m68k_exception, if necessary */
			m68ki_exception_if_trace(m68k); /* auto-disable (see m68kcpu.h) */
		}

		/* set previous PC to current PC for the next entry into the loop */
		REG_PPC(m68k) = REG_PC(m68k);
	}
	else if (m68k->remaining_cycles > 0)
		m68k->remaining_cycles = 0;
}

static CPU_INIT( m68k )
{
	static UINT32 emulation_initialized = 0;
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	m68k->device = device;
	m68k->program = device->space(AS_PROGRAM);
	m68k->int_ack_callback = irqcallback;

	/* disable all MMUs */
	m68k->has_pmmu	       = 0;
	m68k->has_hmmu	       = 0;
	m68k->pmmu_enabled     = 0;
	m68k->hmmu_enabled     = 0;

	/* The first call to this function initializes the opcode handler jump table */
	if(!emulation_initialized)
	{
		m68ki_build_opcode_table();
		emulation_initialized = 1;
	}

	/* Note, D covers A because the dar array is common, REG_A(m68k)=REG_D(m68k)+8 */
	device->save_item(NAME(REG_D(m68k)));
	device->save_item(NAME(REG_PPC(m68k)));
	device->save_item(NAME(REG_PC(m68k)));
	device->save_item(NAME(REG_USP(m68k)));
	device->save_item(NAME(REG_ISP(m68k)));
	device->save_item(NAME(REG_MSP(m68k)));
	device->save_item(NAME(m68k->vbr));
	device->save_item(NAME(m68k->sfc));
	device->save_item(NAME(m68k->dfc));
	device->save_item(NAME(m68k->cacr));
	device->save_item(NAME(m68k->caar));
	device->save_item(NAME(m68k->save_sr));
	device->save_item(NAME(m68k->int_level));
	device->save_item(NAME(m68k->save_stopped));
	device->save_item(NAME(m68k->save_halted));
	device->save_item(NAME(m68k->pref_addr));
	device->save_item(NAME(m68k->pref_data));
	device->machine().save().register_presave(save_prepost_delegate(FUNC(m68k_presave), m68k));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(m68k_postload), m68k));
}

/* Pulse the RESET line on the CPU */
static CPU_RESET( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	/* Disable the PMMU/HMMU on reset, if any */
	m68k->pmmu_enabled = 0;
	m68k->hmmu_enabled = 0;

	m68k->mmu_tc = 0;
	m68k->mmu_tt0 = 0;
	m68k->mmu_tt1 = 0;

	/* Clear all stop levels and eat up all remaining cycles */
	m68k->stopped = 0;
	if (m68k->remaining_cycles > 0)
		m68k->remaining_cycles = 0;

	m68k->run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Turn off tracing */
	m68k->t1_flag = m68k->t0_flag = 0;
	m68ki_clear_trace(m68k);
	/* Interrupt mask to level 7 */
	m68k->int_mask = 0x0700;
	m68k->int_level = 0;
	m68k->virq_state = 0;
	/* Reset VBR */
	m68k->vbr = 0;
	/* Go to supervisor mode */
	m68ki_set_sm_flag(m68k, SFLAG_SET | MFLAG_CLEAR);

	/* Invalidate the prefetch queue */
	/* Set to arbitrary number since our first fetch is from 0 */
	m68k->pref_addr = 0x1000;

	/* Read the initial stack pointer and program counter */
	m68ki_jump(m68k, 0);
	REG_SP(m68k) = m68ki_read_imm_32(m68k);
	REG_PC(m68k) = m68ki_read_imm_32(m68k);
	m68ki_jump(m68k, REG_PC(m68k));

	m68k->run_mode = RUN_MODE_NORMAL;

	m68k->reset_cycles = m68k->cyc_exception[EXCEPTION_RESET];

	/* flush the MMU's cache */
	pmmu_atc_flush(m68k);

	if(CPU_TYPE_IS_EC020_PLUS(m68k->cpu_type))
	{
		// clear instruction cache
		m68ki_ic_clear(m68k);
	}

	// disable instruction hook
	m68k->instruction_hook = NULL;
}

static CPU_DISASSEMBLE( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, m68k->dasm_type);
}



/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	switch (entry.index())
	{
		case M68K_SR:
		case STATE_GENFLAGS:
			m68ki_set_sr(m68k, m68k->iotemp);
			break;

		case M68K_ISP:
			if (m68k->s_flag && !m68k->m_flag)
				REG_SP(m68k) = m68k->iotemp;
			else
				REG_ISP(m68k) = m68k->iotemp;
			break;

		case M68K_USP:
			if (!m68k->s_flag)
				REG_SP(m68k) = m68k->iotemp;
			else
				REG_USP(m68k) = m68k->iotemp;
			break;

		case M68K_MSP:
			if (m68k->s_flag && m68k->m_flag)
				REG_SP(m68k) = m68k->iotemp;
			else
				REG_MSP(m68k) = m68k->iotemp;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(m68k) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STATE( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	switch (entry.index())
	{
		case M68K_SR:
		case STATE_GENFLAGS:
			m68k->iotemp = m68ki_get_sr(m68k);
			break;

		case M68K_ISP:
			m68k->iotemp = (m68k->s_flag && !m68k->m_flag) ? REG_SP(m68k) : REG_ISP(m68k);
			break;

		case M68K_USP:
			m68k->iotemp = (!m68k->s_flag) ? REG_SP(m68k) : REG_USP(m68k);
			break;

		case M68K_MSP:
			m68k->iotemp = (m68k->s_flag && m68k->m_flag) ? REG_SP(m68k) : REG_MSP(m68k);
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
			fatalerror("CPU_EXPORT_STATE(m68k) called for unexpected value\n");
			break;
	}
}

static CPU_SET_INFO( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:
		case CPUINFO_INT_INPUT_STATE + 1:
		case CPUINFO_INT_INPUT_STATE + 2:
		case CPUINFO_INT_INPUT_STATE + 3:
		case CPUINFO_INT_INPUT_STATE + 4:
		case CPUINFO_INT_INPUT_STATE + 5:
		case CPUINFO_INT_INPUT_STATE + 6:
		case CPUINFO_INT_INPUT_STATE + 7:
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:
			set_irq_line(m68k, state - CPUINFO_INT_INPUT_STATE, info->i);
			break;

		case CPUINFO_INT_INPUT_STATE + M68K_LINE_BUSERROR:
			if (info->i == ASSERT_LINE)
			{
				m68k_cause_bus_error(m68k);
			}
			break;
	}
}

static CPU_EXPORT_STRING( m68k )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	UINT16 sr;

	switch (entry.index())
	{
		case M68K_FP0:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[0]));
			break;

		case M68K_FP1:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[1]));
			break;

		case M68K_FP2:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[2]));
			break;

		case M68K_FP3:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[3]));
			break;

		case M68K_FP4:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[4]));
			break;

		case M68K_FP5:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[5]));
			break;

		case M68K_FP6:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[6]));
			break;

		case M68K_FP7:
			string.printf("%f", fx80_to_double(REG_FP(m68k)[7]));
			break;

		case STATE_GENFLAGS:
			sr = m68ki_get_sr(m68k);
			string.printf("%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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

static CPU_GET_INFO( m68k )
{
	m68ki_cpu_core *m68k = (device != NULL && device->token() != NULL) ? m68k_get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68ki_cpu_core);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 16;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 24;							break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;							break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;  /* there is no level 0 */	break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = (m68k->virq_state >> 1) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = (m68k->virq_state >> 2) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = (m68k->virq_state >> 3) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = (m68k->virq_state >> 4) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = (m68k->virq_state >> 5) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = (m68k->virq_state >> 6) & 1;	break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = (m68k->virq_state >> 7) & 1;	break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(m68k);				break;
		case CPUINFO_FCT_INIT:			/* set per-core */										break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(m68k);						break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(m68k);					break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(m68k);			break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(m68k);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(m68k);		break;
		case CPUINFO_FCT_EXPORT_STRING: info->export_string = CPU_EXPORT_STRING_NAME(m68k);		break;
		case CPUINFO_FCT_TRANSLATE:	    	info->translate = CPU_TRANSLATE_NAME(m68k);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k->remaining_cycles;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							/* set per-core */						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "4.95");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB, FPU+MMU by RB+HO+OG)"); break;
	}
}


/* global access */

void m68k_set_encrypted_opcode_range(device_t *device, offs_t start, offs_t end)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->encrypted_start = start;
	m68k->encrypted_end = end;
}

void m68k_set_hmmu_enable(device_t *device, int enable)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	m68k->hmmu_enabled = enable;
}

void m68k_set_instruction_hook(device_t *device, instruction_hook_t ihook)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

    m68k->instruction_hook = ihook;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

UINT16 m68k_memory_interface::m68008_read_immediate_16(offs_t address)
{
	return (m_direct->read_decrypted_byte(address) << 8) | (m_direct->read_decrypted_byte(address + 1));
}

void m68k_memory_interface::init8(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = 0;

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::m68008_read_immediate_16), this);
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

UINT16 m68k_memory_interface::read_immediate_16(offs_t address)
{
	return m_direct->read_decrypted_word((address), opcode_xor);
}

UINT16 m68k_memory_interface::simple_read_immediate_16(offs_t address)
{
	return m_direct->read_decrypted_word(address);
}

void m68k_memory_interface::init16(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = 0;

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::simple_read_immediate_16), this);
	read8 = m68k_read8_delegate(FUNC(address_space::read_byte), &space);
	read16 = m68k_read16_delegate(FUNC(address_space::read_word), &space);
	read32 = m68k_read32_delegate(FUNC(address_space::read_dword), &space);
	write8 = m68k_write8_delegate(FUNC(address_space::write_byte), &space);
	write16 = m68k_write16_delegate(FUNC(address_space::write_word), &space);
	write32 = m68k_write32_delegate(FUNC(address_space::write_dword), &space);
}

int m68307_calc_cs(m68ki_cpu_core *m68k, offs_t address)
{
	m68307_sim* sim = m68k->m68307SIM;

	for (int i=0;i<4;i++)
	{
		int br,amask,bra;
		br = sim->m_br[i] & 1;
		amask = ((sim->m_or[i]&0x1ffc)<<11);
		bra = ((sim->m_br[i] & 0x1ffc)<<11);
		if ((br) && ((address & amask) == bra)) return i+1;
	}
	return 0;
}



UINT16 m68k_memory_interface::simple_read_immediate_16_m68307(offs_t address)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	return m_direct->read_decrypted_word(address);
}

UINT8 m68k_memory_interface::read_byte_m68307(offs_t address)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	return m_space->read_byte(address);
}

UINT16 m68k_memory_interface::read_word_m68307(offs_t address)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	return m_space->read_word(address);
}

UINT32 m68k_memory_interface::read_dword_m68307(offs_t address)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	return m_space->read_dword(address);
}

void m68k_memory_interface::write_byte_m68307(offs_t address, UINT8 data)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	m_space->write_byte(address, data);
}

void m68k_memory_interface::write_word_m68307(offs_t address, UINT16 data)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	m_space->write_word(address, data);
}

void m68k_memory_interface::write_dword_m68307(offs_t address, UINT32 data)
{
//  m_cpustate->m68307_currentcs = m68307_calc_cs(m_cpustate, address);
	m_space->write_dword(address, data);
}




void m68k_memory_interface::init16_m68307(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = 0;

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::simple_read_immediate_16_m68307), this);
	read8 = m68k_read8_delegate(FUNC(m68k_memory_interface::read_byte_m68307), this);
	read16 = m68k_read16_delegate(FUNC(m68k_memory_interface::read_word_m68307), this);
	read32 = m68k_read32_delegate(FUNC(m68k_memory_interface::read_dword_m68307), this);
	write8 = m68k_write8_delegate(FUNC(m68k_memory_interface::write_byte_m68307), this);
	write16 = m68k_write16_delegate(FUNC(m68k_memory_interface::write_word_m68307), this);
	write32 = m68k_write32_delegate(FUNC(m68k_memory_interface::write_dword_m68307), this);
}

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

/* interface for 32-bit data bus (68EC020, 68020) */
void m68k_memory_interface::init32(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::read_immediate_16), this);
	read8 = m68k_read8_delegate(FUNC(address_space::read_byte), &space);
	read16 = m68k_read16_delegate(FUNC(address_space::read_word_unaligned), &space);
	read32 = m68k_read32_delegate(FUNC(address_space::read_dword_unaligned), &space);
	write8 = m68k_write8_delegate(FUNC(address_space::write_byte), &space);
	write16 = m68k_write16_delegate(FUNC(address_space::write_word_unaligned), &space);
	write32 = m68k_write32_delegate(FUNC(address_space::write_dword_unaligned), &space);
}

/* interface for 32-bit data bus with PMMU (68EC020, 68020) */
UINT8 m68k_memory_interface::read_byte_32_mmu(offs_t address)
{
	if (m_cpustate->pmmu_enabled)
	{
		address = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return ~0;
		}
	}

	return m_space->read_byte(address);
}

void m68k_memory_interface::write_byte_32_mmu(offs_t address, UINT8 data)
{
	if (m_cpustate->pmmu_enabled)
	{
		address = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return;
		}
	}

	m_space->write_byte(address, data);
}

UINT16 m68k_memory_interface::read_immediate_16_mmu(offs_t address)
{
	if (m_cpustate->pmmu_enabled)
	{
		address = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return ~0;
		}
	}

	return m_direct->read_decrypted_word((address), m_cpustate->memory.opcode_xor);
}

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT16 m68k_memory_interface::readword_d32_mmu(offs_t address)
{
	UINT16 result;

	if (m_cpustate->pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return ~0;
		} else if (!(address & 1)) {
			return m_space->read_word(address0);
		} else {
			UINT32 address1 = pmmu_translate_addr(m_cpustate, address + 1);
			if (m_cpustate->mmu_tmp_buserror_occurred) {
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
void m68k_memory_interface::writeword_d32_mmu(offs_t address, UINT16 data)
{
	if (m_cpustate->pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return;
		} else if (!(address & 1)) {
			m_space->write_word(address0, data);
			return;
		} else {
			UINT32 address1 = pmmu_translate_addr(m_cpustate, address + 1);
			if (m_cpustate->mmu_tmp_buserror_occurred) {
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
UINT32 m68k_memory_interface::readlong_d32_mmu(offs_t address)
{
	UINT32 result;

	if (m_cpustate->pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return ~0;
		} else if ((address +3) & 0xfc) {
			// not at page boundary; use default code
			address = address0;
		} else if (!(address & 3)) { // 0
			return m_space->read_dword(address0);
		} else {
			UINT32 address2 = pmmu_translate_addr(m_cpustate, address+2);
			if (m_cpustate->mmu_tmp_buserror_occurred) {
				return ~0;
			} else if (!(address & 1)) { // 2
				result = m_space->read_word(address0) << 16;
				return result | m_space->read_word(address2);
			} else {
				UINT32 address1 = pmmu_translate_addr(m_cpustate, address+1);
				UINT32 address3 = pmmu_translate_addr(m_cpustate, address+3);
				if (m_cpustate->mmu_tmp_buserror_occurred) {
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
void m68k_memory_interface::writelong_d32_mmu(offs_t address, UINT32 data)
{
	if (m_cpustate->pmmu_enabled)
	{
		UINT32 address0 = pmmu_translate_addr(m_cpustate, address);
		if (m_cpustate->mmu_tmp_buserror_occurred) {
			return;
		} else if ((address +3) & 0xfc) {
			// not at page boundary; use default code
			address = address0;
		} else if (!(address & 3)) { // 0
			m_space->write_dword(address0, data);
			return;
		} else {
			UINT32 address2 = pmmu_translate_addr(m_cpustate, address+2);
			if (m_cpustate->mmu_tmp_buserror_occurred) {
				return;
			} else if (!(address & 1)) { // 2
				m_space->write_word(address0, data >> 16);
				m_space->write_word(address2, data);
				return;
			} else {
				UINT32 address1 = pmmu_translate_addr(m_cpustate, address+1);
				UINT32 address3 = pmmu_translate_addr(m_cpustate, address+3);
				if (m_cpustate->mmu_tmp_buserror_occurred) {
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

void m68k_memory_interface::init32mmu(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::read_immediate_16_mmu), this);
	read8 = m68k_read8_delegate(FUNC(m68k_memory_interface::read_byte_32_mmu), this);
	read16 = m68k_read16_delegate(FUNC(m68k_memory_interface::readword_d32_mmu), this);
	read32 = m68k_read32_delegate(FUNC(m68k_memory_interface::readlong_d32_mmu), this);
	write8 = m68k_write8_delegate(FUNC(m68k_memory_interface::write_byte_32_mmu), this);
	write16 = m68k_write16_delegate(FUNC(m68k_memory_interface::writeword_d32_mmu), this);
	write32 = m68k_write32_delegate(FUNC(m68k_memory_interface::writelong_d32_mmu), this);
}


/* interface for 32-bit data bus with PMMU (68EC020, 68020) */
UINT8 m68k_memory_interface::read_byte_32_hmmu(offs_t address)
{
	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
	}

	return m_space->read_byte(address);
}

void m68k_memory_interface::write_byte_32_hmmu(offs_t address, UINT8 data)
{
	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
	}

	m_space->write_byte(address, data);
}

UINT16 m68k_memory_interface::read_immediate_16_hmmu(offs_t address)
{
	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
	}

	return m_direct->read_decrypted_word((address), m_cpustate->memory.opcode_xor);
}

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
UINT16 m68k_memory_interface::readword_d32_hmmu(offs_t address)
{
	UINT16 result;

	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
	}

	if (!(address & 1))
		return m_space->read_word(address);
	result = m_space->read_byte(address) << 8;
	return result | m_space->read_byte(address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
void m68k_memory_interface::writeword_d32_hmmu(offs_t address, UINT16 data)
{
	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
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
UINT32 m68k_memory_interface::readlong_d32_hmmu(offs_t address)
{
	UINT32 result;

	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
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
void m68k_memory_interface::writelong_d32_hmmu(offs_t address, UINT32 data)
{
	if (m_cpustate->hmmu_enabled)
	{
		address = hmmu_translate_addr(m_cpustate, address);
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

void m68k_memory_interface::init32hmmu(address_space &space)
{
	m_space = &space;
	m_direct = &space.direct();
	m_cpustate = m68k_get_safe_token(&space.device());
	opcode_xor = WORD_XOR_BE(0);

	readimm16 = m68k_readimm16_delegate(FUNC(m68k_memory_interface::read_immediate_16_hmmu), this);
	read8 = m68k_read8_delegate(FUNC(m68k_memory_interface::read_byte_32_hmmu), this);
	read16 = m68k_read16_delegate(FUNC(m68k_memory_interface::readword_d32_hmmu), this);
	read32 = m68k_read32_delegate(FUNC(m68k_memory_interface::readlong_d32_hmmu), this);
	write8 = m68k_write8_delegate(FUNC(m68k_memory_interface::write_byte_32_hmmu), this);
	write16 = m68k_write16_delegate(FUNC(m68k_memory_interface::writeword_d32_hmmu), this);
	write32 = m68k_write32_delegate(FUNC(m68k_memory_interface::writelong_d32_hmmu), this);
}

void m68k_set_reset_callback(device_t *device, m68k_reset_func callback)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->reset_instr_callback = callback;
}

void m68k_set_cmpild_callback(device_t *device, m68k_cmpild_func callback)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->cmpild_instr_callback = callback;
}

void m68k_set_rte_callback(device_t *device, m68k_rte_func callback)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->rte_instr_callback = callback;
}

void m68k_set_tas_callback(device_t *device, m68k_tas_func callback)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->tas_instr_callback = callback;
}

UINT16 m68k_get_fc(device_t *device)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	return m68k->mmu_tmp_fc;
}

void m68307_set_port_callbacks(device_t *device, m68307_porta_read_callback porta_r, m68307_porta_write_callback porta_w, m68307_portb_read_callback portb_r, m68307_portb_write_callback portb_w)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	m68k->m_m68307_porta_r = porta_r;
	m68k->m_m68307_porta_w = porta_w;
	m68k->m_m68307_portb_r = portb_r;
	m68k->m_m68307_portb_w = portb_w;
}


UINT16 m68307_get_cs(device_t *device, offs_t address)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	m68k->m68307_currentcs = m68307_calc_cs(m68k, address);

	return m68k->m68307_currentcs;
}

/****************************************************************************
 * State definition
 ****************************************************************************/

static void define_state(device_t *device)
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);
	UINT32 addrmask = (m68k->cpu_type & MASK_24BIT_SPACE) ? 0xffffff : 0xffffffff;

	device_state_interface *state;
	device->interface(state);
	state->state_add(M68K_PC,         "PC",        m68k->pc).mask(addrmask);
	state->state_add(STATE_GENPC,     "GENPC",     m68k->pc).mask(addrmask).noshow();
	state->state_add(STATE_GENPCBASE, "GENPCBASE", m68k->ppc).mask(addrmask).noshow();
	state->state_add(M68K_SP,         "SP",        m68k->dar[15]);
	state->state_add(STATE_GENSP,     "GENSP",     m68k->dar[15]).noshow();
	state->state_add(STATE_GENFLAGS,  "GENFLAGS",  m68k->iotemp).noshow().callimport().callexport().formatstr("%16s");
	state->state_add(M68K_ISP,        "ISP",       m68k->iotemp).callimport().callexport();
	state->state_add(M68K_USP,        "USP",       m68k->iotemp).callimport().callexport();
	if (m68k->cpu_type & MASK_020_OR_LATER)
		state->state_add(M68K_MSP,    "MSP",       m68k->iotemp).callimport().callexport();
	state->state_add(M68K_ISP,        "ISP",       m68k->iotemp).callimport().callexport();

	astring tempstr;
	for (int regnum = 0; regnum < 8; regnum++)
		state->state_add(M68K_D0 + regnum, tempstr.format("D%d", regnum), m68k->dar[regnum]);
	for (int regnum = 0; regnum < 8; regnum++)
		state->state_add(M68K_A0 + regnum, tempstr.format("A%d", regnum), m68k->dar[8 + regnum]);

	state->state_add(M68K_PREF_ADDR,  "PREF_ADDR", m68k->pref_addr).mask(addrmask);
	state->state_add(M68K_PREF_DATA,  "PREF_DATA", m68k->pref_data);

	if (m68k->cpu_type & MASK_010_OR_LATER)
	{
		state->state_add(M68K_SFC,    "SFC",       m68k->sfc).mask(0x7);
		state->state_add(M68K_DFC,    "DFC",       m68k->dfc).mask(0x7);
		state->state_add(M68K_VBR,    "VBR",       m68k->vbr);
	}

	if (m68k->cpu_type & MASK_020_OR_LATER)
	{
		state->state_add(M68K_CACR,   "CACR",      m68k->cacr);
		state->state_add(M68K_CAAR,   "CAAR",      m68k->caar);
	}

	if (m68k->cpu_type & MASK_030_OR_LATER)
	{
		for (int regnum = 0; regnum < 8; regnum++)
			state->state_add(M68K_FP0 + regnum, tempstr.format("FP%d", regnum), m68k->iotemp).callimport().callexport().formatstr("%10s");
		state->state_add(M68K_FPSR, "FPSR", m68k->fpsr);
		state->state_add(M68K_FPCR, "FPCR", m68k->fpcr);
	}
}


/****************************************************************************
 * 68000 section
 ****************************************************************************/

static CPU_INIT( m68000 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_000;
	m68k->dasm_type        = M68K_CPU_TYPE_68000;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init16(*m68k->program);
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[0];
	m68k->cyc_instruction  = m68ki_cycles[0];
	m68k->cyc_exception    = m68ki_exception_cycle_table[0];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 2;
	m68k->cyc_dbcc_f_noexp = -2;
	m68k->cyc_dbcc_f_exp   = 2;
	m68k->cyc_scc_r_true   = 2;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 3;
	m68k->cyc_shift        = 1;
	m68k->cyc_reset        = 132;
	m68k->has_pmmu	       = 0;
	m68k->has_hmmu	       = 0;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68000 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:						info->init = CPU_INIT_NAME(m68000);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "68000");						break;

		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}

static CPU_INIT( m68301 )
{
//  m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68000);

	/* there is a basic implementation of this in emu/machine/tmp68301.c but it should be moved here */

}

CPU_GET_INFO( m68301 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:						info->init = CPU_INIT_NAME(m68301);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "68301");						break;

		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}




void m68307_set_interrupt(device_t *device, int level, int vector)
{
	device_set_input_line_and_vector(device, level, HOLD_LINE, vector);
}

void m68307_timer0_interrupt(legacy_cpu_device *cpudev)
{
	m68ki_cpu_core* m68k = m68k_get_safe_token(cpudev);
	int prioritylevel = (m68k->m68307SIM->m_picr & 0x7000)>>12;
	int vector        = (m68k->m68307SIM->m_pivr & 0x00f0) | 0xa;
	m68307_set_interrupt(cpudev, prioritylevel, vector);
}

void m68307_timer1_interrupt(legacy_cpu_device *cpudev)
{
	m68ki_cpu_core* m68k = m68k_get_safe_token(cpudev);
	int prioritylevel = (m68k->m68307SIM->m_picr & 0x0700)>>8;
	int vector        = (m68k->m68307SIM->m_pivr & 0x00f0) | 0xb;
	m68307_set_interrupt(cpudev, prioritylevel, vector);
}

void m68307_serial_interrupt(legacy_cpu_device *cpudev)
{
	m68ki_cpu_core* m68k = m68k_get_safe_token(cpudev);
	int prioritylevel = (m68k->m68307SIM->m_picr & 0x0070)>>4;
	int vector        = (m68k->m68307SERIAL->m_uivr);
	m68307_set_interrupt(cpudev, prioritylevel, vector);
}

void m68307_mbus_interrupt(legacy_cpu_device *cpudev)
{
	m68ki_cpu_core* m68k = m68k_get_safe_token(cpudev);
	int prioritylevel = (m68k->m68307SIM->m_picr & 0x0007)>>0;
	int vector        = (m68k->m68307SIM->m_pivr & 0x00f0) | 0xd;
	m68307_set_interrupt(cpudev, prioritylevel, vector);
}

void m68307_licr2_interrupt(legacy_cpu_device *cpudev)
{
	m68ki_cpu_core* m68k = m68k_get_safe_token(cpudev);
	int prioritylevel = (m68k->m68307SIM->m_licr2 & 0x0007)>>0;
	int vector        = (m68k->m68307SIM->m_pivr & 0x00f0) | 0x9;
	m68k->m68307SIM->m_licr2 |= 0x8;


	m68307_set_interrupt(cpudev, prioritylevel, vector);
}



static CPU_INIT( m68307 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68000);

	/* basic CS logic, timers, mbus, serial logic
       set via remappable register
    */
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init16_m68307(*m68k->program);

	m68k->m68307SIM    = new m68307_sim();
	m68k->m68307MBUS   = new m68307_mbus();
	m68k->m68307SERIAL = new m68307_serial();
	m68k->m68307TIMER  = new m68307_timer();

	m68k->m68307TIMER->init(device);

	m68k->m68307SIM->reset();
	m68k->m68307MBUS->reset();
	m68k->m68307SERIAL->reset();
	m68k->m68307TIMER->reset();

	m68k->internal = device->space(AS_PROGRAM);
	m68k->m68307_base = 0xbfff;
	m68k->m68307_scrhigh = 0x0007;
	m68k->m68307_scrlow = 0xf010;

	m68307_set_port_callbacks(device, 0,0,0,0);
}

static READ16_HANDLER( m68307_internal_base_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68307_internal_base_r %08x, (%04x)\n", pc, offset*2,mem_mask);

	switch (offset<<1)
	{
		case 0x2: return m68k->m68307_base;
		case 0x4: return m68k->m68307_scrhigh;
		case 0x6: return m68k->m68307_scrlow;
	}

	logerror("(read was illegal?)\n");

	return 0x0000;
}

static WRITE16_HANDLER( m68307_internal_base_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68307_internal_base_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
	int base = 0;
	//int mask = 0;

	switch (offset<<1)
	{
		case 0x2:
			/* remove old internal handler */
			base = (m68k->m68307_base & 0x0fff) << 12;
			//mask = (m68k->m68307_base & 0xe000) >> 13;
			//if ( m68k->m68307_base & 0x1000 ) mask |= 7;
        	m68k->internal->unmap_readwrite(base+0x000, base+0x04f);
        	m68k->internal->unmap_readwrite(base+0x100, base+0x11f);
        	m68k->internal->unmap_readwrite(base+0x120, base+0x13f);
        	m68k->internal->unmap_readwrite(base+0x140, base+0x149);

			/* store new base address */
			COMBINE_DATA(&m68k->m68307_base);

			/* install new internal handler */
			base = (m68k->m68307_base & 0x0fff) << 12;
			//mask = (m68k->m68307_base & 0xe000) >> 13;
			//if ( m68k->m68307_base & 0x1000 ) mask |= 7;
			m68k->internal->install_legacy_readwrite_handler(base + 0x000, base + 0x04f, FUNC(m68307_internal_sim_r),    FUNC(m68307_internal_sim_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x100, base + 0x11f, FUNC(m68307_internal_serial_r), FUNC(m68307_internal_serial_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x120, base + 0x13f, FUNC(m68307_internal_timer_r),  FUNC(m68307_internal_timer_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x140, base + 0x149, FUNC(m68307_internal_mbus_r),   FUNC(m68307_internal_mbus_w));

			break;

		case 0x4:
			COMBINE_DATA(&m68k->m68307_scrhigh);
			break;

		case 0x6:
			COMBINE_DATA(&m68k->m68307_scrlow);
			break;

		default:
			logerror("(write was illegal?)\n");
			break;
	}
}

static ADDRESS_MAP_START( m68307_internal_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000000f0, 0x000000ff) AM_READWRITE_LEGACY(m68307_internal_base_r, m68307_internal_base_w)
ADDRESS_MAP_END

CPU_GET_INFO( m68307 )
{
	switch (state)
	{
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 24;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:						info->init = CPU_INIT_NAME(m68307);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "68307");						break;

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(m68307_internal_map); break;


		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}


/****************************************************************************
 * M68008 section
 ****************************************************************************/

static CPU_INIT( m68008 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_008;
	m68k->dasm_type        = M68K_CPU_TYPE_68008;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init8(*m68k->program);
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[0];
	m68k->cyc_instruction  = m68ki_cycles[0];
	m68k->cyc_exception    = m68ki_exception_cycle_table[0];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 2;
	m68k->cyc_dbcc_f_noexp = -2;
	m68k->cyc_dbcc_f_exp   = 2;
	m68k->cyc_scc_r_true   = 2;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 3;
	m68k->cyc_shift        = 1;
	m68k->cyc_reset        = 132;
	m68k->has_pmmu	       = 0;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68008 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 20;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68008);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68008");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

CPU_GET_INFO( m68008plcc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:	info->i = 22;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68008);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68008");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}


/****************************************************************************
 * M68010 section
 ****************************************************************************/

static CPU_INIT( m68010 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_010;
	m68k->dasm_type        = M68K_CPU_TYPE_68010;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init16(*m68k->program);
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[1];
	m68k->cyc_instruction  = m68ki_cycles[1];
	m68k->cyc_exception    = m68ki_exception_cycle_table[1];
	m68k->cyc_bcc_notake_b = -4;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 6;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 3;
	m68k->cyc_shift        = 1;
	m68k->cyc_reset        = 130;
	m68k->has_pmmu	       = 0;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68010 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68010);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68010");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}


/****************************************************************************
 * M68020 section
 ****************************************************************************/

static CPU_INIT( m68020 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_020;
	m68k->dasm_type        = M68K_CPU_TYPE_68020;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[2];
	m68k->cyc_instruction  = m68ki_cycles[2];
	m68k->cyc_exception    = m68ki_exception_cycle_table[2];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;

	define_state(device);
}

CPU_GET_INFO( m68020 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68020);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68020");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

// 68020 with 68851 PMMU
static CPU_INIT( m68020pmmu )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68020);

	m68k->has_pmmu	       = 1;
	m68k->has_fpu	       = 1;

// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32mmu(*m68k->program);
}

CPU_GET_INFO( m68020pmmu )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68020pmmu);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68020, 68851");				break;

		default:										CPU_GET_INFO_CALL(m68020);				break;
	}
}

// 68020 with Apple HMMU & 68881 FPU
static CPU_INIT( m68020hmmu )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68020);

	m68k->has_hmmu = 1;
	m68k->has_fpu  = 1;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32hmmu(*m68k->program);
}

CPU_GET_INFO( m68020hmmu )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:		info->init = CPU_INIT_NAME(m68020hmmu);			break;
		case CPUINFO_FCT_TRANSLATE:	info->translate = CPU_TRANSLATE_NAME(m68khmmu);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:		strcpy(info->s, "68020, Apple HMMU");			break;

		default:			CPU_GET_INFO_CALL(m68020);				break;
	}
}

/****************************************************************************
 * M680EC20 section
 ****************************************************************************/

static CPU_INIT( m68ec020 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_EC020;
	m68k->dasm_type        = M68K_CPU_TYPE_68EC020;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[2];
	m68k->cyc_instruction  = m68ki_cycles[2];
	m68k->cyc_exception    = m68ki_exception_cycle_table[2];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 0;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68ec020 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 24;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68ec020);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68EC020");				break;

		default:										CPU_GET_INFO_CALL(m68020);				break;
	}
}

/****************************************************************************
 * M68030 section
 ****************************************************************************/

static CPU_INIT( m68030 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_030;
	m68k->dasm_type        = M68K_CPU_TYPE_68030;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32mmu(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[3];
	m68k->cyc_instruction  = m68ki_cycles[3];
	m68k->cyc_exception    = m68ki_exception_cycle_table[3];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 1;
	m68k->has_fpu	       = 1;

	define_state(device);
}

CPU_GET_INFO( m68030 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68030);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68030");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}


/****************************************************************************
 * M680EC30 section
 ****************************************************************************/

static CPU_INIT( m68ec030 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_EC030;
	m68k->dasm_type        = M68K_CPU_TYPE_68EC030;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[3];
	m68k->cyc_instruction  = m68ki_cycles[3];
	m68k->cyc_exception    = m68ki_exception_cycle_table[3];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 0;		/* EC030 lacks the PMMU and is effectively a die-shrink 68020 */
	m68k->has_fpu	       = 1;

	define_state(device);
}

CPU_GET_INFO( m68ec030 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68ec030);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68EC030");				break;

		default:										CPU_GET_INFO_CALL(m68030);				break;
	}
}

/****************************************************************************
 * M68040 section
 ****************************************************************************/

static CPU_INIT( m68040 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_040;
	m68k->dasm_type        = M68K_CPU_TYPE_68040;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32mmu(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[4];
	m68k->cyc_instruction  = m68ki_cycles[4];
	m68k->cyc_exception    = m68ki_exception_cycle_table[4];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 1;
	m68k->has_fpu	       = 1;

	define_state(device);
}

CPU_GET_INFO( m68040 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68040);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68040");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

/****************************************************************************
 * M68EC040 section
 ****************************************************************************/

static CPU_INIT( m68ec040 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_EC040;
	m68k->dasm_type        = M68K_CPU_TYPE_68EC040;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[4];
	m68k->cyc_instruction  = m68ki_cycles[4];
	m68k->cyc_exception    = m68ki_exception_cycle_table[4];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 0;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68ec040 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68ec040);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68EC040");				break;

		default:										CPU_GET_INFO_CALL(m68040);				break;
	}
}

/****************************************************************************
 * M68LC040 section
 ****************************************************************************/

static CPU_INIT( m68lc040 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_LC040;
	m68k->dasm_type        = M68K_CPU_TYPE_68LC040;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32mmu(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[4];
	m68k->cyc_instruction  = m68ki_cycles[4];
	m68k->cyc_exception    = m68ki_exception_cycle_table[4];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;
	m68k->has_pmmu	       = 1;
	m68k->has_fpu	       = 0;

	define_state(device);
}

CPU_GET_INFO( m68lc040 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68lc040);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "68LC040");				break;

		default:										CPU_GET_INFO_CALL(m68040);				break;
	}
}

/****************************************************************************
 * SCC-68070 section
 ****************************************************************************/

static CPU_INIT( scc68070 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68010);

	m68k->cpu_type         = CPU_TYPE_SCC070;
}

CPU_GET_INFO( scc68070 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(scc68070);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "SCC68070");			break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

/****************************************************************************
 * Freescale M68340 section
 ****************************************************************************/

static READ32_HANDLER( m68340_internal_base_r )
{


	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68340_internal_base_r %08x, (%04x)\n", pc, offset*4,mem_mask);
	return 0x55555555;
}

static WRITE32_HANDLER( m68340_internal_base_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68340_internal_base_w %08x, %04x (%04x)\n", pc, offset*4,data,mem_mask);

	// other conditions?
	if (m68k->dfc==0x7)
	{
		// unmap old modules
		if (m68k->m68340_base&1)
		{
			int base = m68k->m68340_base & 0xfffff000;

			m68k->internal->unmap_readwrite(base + 0x000, base + 0x05f);
			m68k->internal->unmap_readwrite(base + 0x600, base + 0x67f);
			m68k->internal->unmap_readwrite(base + 0x700, base + 0x723);
			m68k->internal->unmap_readwrite(base + 0x780, base + 0x7bf);

		}

		COMBINE_DATA(&m68k->m68340_base);
		logerror("%08x m68340_internal_base_w %08x, %04x (%04x) (m68340_base write)\n", pc, offset*4,data,mem_mask);

		// map new modules
		if (m68k->m68340_base&1)
		{
			int base = m68k->m68340_base & 0xfffff000;

			m68k->internal->install_legacy_readwrite_handler(base + 0x000, base + 0x05f, FUNC(m68340_internal_sim_r),    FUNC(m68340_internal_sim_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x600, base + 0x67f, FUNC(m68340_internal_timer_r),  FUNC(m68340_internal_timer_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x700, base + 0x723, FUNC(m68340_internal_serial_r), FUNC(m68340_internal_serial_w));
			m68k->internal->install_legacy_readwrite_handler(base + 0x780, base + 0x7bf, FUNC(m68340_internal_dma_r),    FUNC(m68340_internal_dma_w));

		}

	}
	else
	{
		logerror("%08x m68340_internal_base_w %08x, %04x (%04x) (should fall through?)\n", pc, offset*4,data,mem_mask);
	}



}


static ADDRESS_MAP_START( m68340_internal_map, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0x0003ff00, 0x0003ff03) AM_READWRITE_LEGACY( m68340_internal_base_r, m68340_internal_base_w)
ADDRESS_MAP_END

static CPU_INIT( m68340 )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_68340;
	m68k->dasm_type        = M68K_CPU_TYPE_68340;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[5];
	m68k->cyc_instruction  = m68ki_cycles[5];
	m68k->cyc_exception    = m68ki_exception_cycle_table[5];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;

	m68k->m68340SIM    = new m68340_sim();
	m68k->m68340DMA    = new m68340_dma();
	m68k->m68340SERIAL = new m68340_serial();
	m68k->m68340TIMER  = new m68340_timer();

	m68k->m68340SIM->reset();
	m68k->m68340DMA->reset();
	m68k->m68340SERIAL->reset();
	m68k->m68340TIMER->reset();

	m68k->m68340_base = 0x00000000;

	m68k->internal = device->space(AS_PROGRAM);

	define_state(device);
}

CPU_GET_INFO( m68340 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(m68340_internal_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(m68340);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Freescale 68340");				break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

/*
  ColdFire

*/

static CPU_INIT( coldfire )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(device);

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_COLDFIRE;
	m68k->dasm_type        = M68K_CPU_TYPE_COLDFIRE;
// hack alert: we use placement new to ensure we are properly initialized
// because we live in the device state which is allocated as bytes
// remove me when we have a real C++ device
	new(&m68k->memory) m68k_memory_interface;
	m68k->memory.init32(*m68k->program);
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
	m68k->jump_table       = m68ki_instruction_jump_table[6];
	m68k->cyc_instruction  = m68ki_cycles[6];
	m68k->cyc_exception    = m68ki_exception_cycle_table[6];
	m68k->cyc_bcc_notake_b = -2;
	m68k->cyc_bcc_notake_w = 0;
	m68k->cyc_dbcc_f_noexp = 0;
	m68k->cyc_dbcc_f_exp   = 4;
	m68k->cyc_scc_r_true   = 0;
	m68k->cyc_movem_w      = 2;
	m68k->cyc_movem_l      = 2;
	m68k->cyc_shift        = 0;
	m68k->cyc_reset        = 518;

	define_state(device);
}

CPU_GET_INFO( mcf5206e )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 32;							break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 32;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(coldfire);						break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "MCF5206E");			break;

		default:										CPU_GET_INFO_CALL(m68k);				break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(M68000, m68000);
DEFINE_LEGACY_CPU_DEVICE(M68301, m68301);
DEFINE_LEGACY_CPU_DEVICE(M68307, m68307);
DEFINE_LEGACY_CPU_DEVICE(M68008, m68008);
DEFINE_LEGACY_CPU_DEVICE(M68008PLCC, m68008plcc);
DEFINE_LEGACY_CPU_DEVICE(M68010, m68010);
DEFINE_LEGACY_CPU_DEVICE(M68EC020, m68ec020);
DEFINE_LEGACY_CPU_DEVICE(M68020, m68020);
DEFINE_LEGACY_CPU_DEVICE(M68020PMMU, m68020pmmu);
DEFINE_LEGACY_CPU_DEVICE(M68020HMMU, m68020hmmu);
DEFINE_LEGACY_CPU_DEVICE(M68EC030, m68ec030);
DEFINE_LEGACY_CPU_DEVICE(M68030, m68030);
DEFINE_LEGACY_CPU_DEVICE(M68EC040, m68ec040);
DEFINE_LEGACY_CPU_DEVICE(M68LC040, m68lc040);
DEFINE_LEGACY_CPU_DEVICE(M68040, m68040);
DEFINE_LEGACY_CPU_DEVICE(SCC68070, scc68070);
DEFINE_LEGACY_CPU_DEVICE(M68340, m68340);
DEFINE_LEGACY_CPU_DEVICE(MCF5206E, mcf5206e);
