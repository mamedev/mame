/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */

#if 0
static const char copyright_notice[] =
"MUSASHI\n"
"Version 4.00 (2008-11-17)\n"
"A portable Motorola M680x0 processor emulation engine.\n"
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
"http://kstenerud.cjb.net\n"
;
#endif


/* ======================================================================== */
/* ================================= NOTES ================================ */
/* ======================================================================== */



/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include <setjmp.h>
#include "m68kcpu.h"
#include "m68kops.h"
#include "m68kfpu.c"
#include "debugger.h"

extern void m68040_fpu_op0(m68ki_cpu_core *m68k);
extern void m68040_fpu_op1(m68ki_cpu_core *m68k);

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
const UINT8 m68ki_exception_cycle_table[4][256] =
{
	{ /* 000 */
		  4, /*  0: Reset - Initial Stack Pointer                      */
		  4, /*  1: Reset - Initial Program Counter                    */
		 50, /*  2: Bus Error                             (unemulated) */
		 50, /*  3: Address Error                         (unemulated) */
		 34, /*  4: Illegal Instruction                                */
		 38, /*  5: Divide by Zero -- ASG: changed from 42             */
		 40, /*  6: CHK -- ASG: chanaged from 44                       */
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
		 34, /* 32: TRAP #0 -- ASG: chanaged from 38                   */
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
		  4, /*  0: Reset - Initial Stack Pointer                      */
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
	}
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

static void m68k_presave(running_machine *machine, void *param)
{
	m68ki_cpu_core *m68k = param;
	m68k->save_sr = m68ki_get_sr(m68k);
	m68k->save_stopped = (m68k->stopped & STOP_LEVEL_STOP) != 0;
	m68k->save_halted  = (m68k->stopped & STOP_LEVEL_HALT) != 0;
}

static void m68k_postload(running_machine *machine, void *param)
{
	m68ki_cpu_core *m68k = param;
	m68ki_set_sr_noint_nosp(m68k, m68k->save_sr);
	m68k->stopped = m68k->save_stopped ? STOP_LEVEL_STOP : 0
		        | m68k->save_halted  ? STOP_LEVEL_HALT : 0;
	m68ki_jump(m68k, REG_PC);
}


/* Execute some instructions until we use up cycles clock cycles */
static CPU_EXECUTE( m68k )
{
	m68ki_cpu_core *m68k = device->token;

	/* Set our pool of clock cycles available */
	SET_CYCLES(m68k, cycles);
	m68k->initial_cycles = cycles;

	/* See if interrupts came in */
	m68ki_check_interrupts(m68k);

	/* Make sure we're not stopped */
	if(!m68k->stopped)
	{
		/* Return point if we had an address error */
		m68ki_set_address_error_trap(m68k); /* auto-disable (see m68kcpu.h) */

		/* Main loop.  Keep going until we run out of clock cycles */
		do
		{
			/* Set tracing accodring to T1. (T0 is done inside instruction) */
			m68ki_trace_t1(); /* auto-disable (see m68kcpu.h) */

			/* Call external hook to peek at CPU */
			debugger_instruction_hook(device, REG_PC);

			/* Record previous program counter */
			REG_PPC = REG_PC;

			/* Read an instruction and call its handler */
			m68k->ir = m68ki_read_imm_16(m68k);
			m68ki_instruction_jump_table[m68k->ir](m68k);
			USE_CYCLES(m68k, m68k->cyc_instruction[m68k->ir]);

			/* Trace m68k_exception, if necessary */
			m68ki_exception_if_trace(); /* auto-disable (see m68kcpu.h) */
		} while(GET_CYCLES(m68k) > 0);

		/* set previous PC to current PC for the next entry into the loop */
		REG_PPC = REG_PC;
	}
	else
		SET_CYCLES(m68k, 0);

	/* return how many clocks we used */
	return m68k->initial_cycles - GET_CYCLES(m68k);
}

static CPU_INIT( m68k )
{
	static UINT32 emulation_initialized = 0;
	m68ki_cpu_core *m68k = device->token;

	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->int_ack_callback = irqcallback;

	/* The first call to this function initializes the opcode handler jump table */
	if(!emulation_initialized)
	{
		m68ki_build_opcode_table();
		emulation_initialized = 1;
	}

	/* Note, D covers A because the dar array is common, REG_A=REG_D+8 */
	state_save_register_item_array("m68k", device->tag, 0, REG_D);
	state_save_register_item("m68k", device->tag, 0, REG_PPC);
	state_save_register_item("m68k", device->tag, 0, REG_PC);
	state_save_register_item("m68k", device->tag, 0, REG_USP);
	state_save_register_item("m68k", device->tag, 0, REG_ISP);
	state_save_register_item("m68k", device->tag, 0, REG_MSP);
	state_save_register_item("m68k", device->tag, 0, m68k->vbr);
	state_save_register_item("m68k", device->tag, 0, m68k->sfc);
	state_save_register_item("m68k", device->tag, 0, m68k->dfc);
	state_save_register_item("m68k", device->tag, 0, m68k->cacr);
	state_save_register_item("m68k", device->tag, 0, m68k->caar);
	state_save_register_item("m68k", device->tag, 0, m68k->save_sr);
	state_save_register_item("m68k", device->tag, 0, m68k->int_level);
	state_save_register_item("m68k", device->tag, 0, m68k->save_stopped);
	state_save_register_item("m68k", device->tag, 0, m68k->save_halted);
	state_save_register_item("m68k", device->tag, 0, m68k->pref_addr);
	state_save_register_item("m68k", device->tag, 0, m68k->pref_data);
	state_save_register_presave(device->machine, m68k_presave, m68k);
	state_save_register_postload(device->machine, m68k_postload, m68k);
}

/* Pulse the RESET line on the CPU */
static CPU_RESET( m68k )
{
	m68ki_cpu_core *m68k = device->token;

	/* Clear all stop levels and eat up all remaining cycles */
	m68k->stopped = 0;
	SET_CYCLES(m68k, 0);

	m68k->run_mode = RUN_MODE_BERR_AERR_RESET;

	/* Turn off tracing */
	m68k->t1_flag = m68k->t0_flag = 0;
	m68ki_clear_trace();
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
	REG_SP = m68ki_read_imm_32(m68k);
	REG_PC = m68ki_read_imm_32(m68k);
	m68ki_jump(m68k, REG_PC);

	m68k->run_mode = RUN_MODE_NORMAL;
}

static CPU_GET_CONTEXT( m68k )
{
}

static CPU_SET_CONTEXT( m68k )
{
}

static CPU_DISASSEMBLE( m68k )
{
	m68ki_cpu_core *m68k = device->token;
	return m68k_disassemble_raw(buffer, pc, oprom, opram, m68k->dasm_type);
}

static CPU_GET_INFO( m68k )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(m68ki_cpu_core);				break;
		case CPUINFO_INT_INPUT_LINES:				info->i = 8;									break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:		info->i = -1;									break;
		case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_BE;							break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;									break;
		case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;									break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i = 2;									break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i = 10;									break;
		case CPUINFO_INT_MIN_CYCLES:				info->i = 4;									break;
		case CPUINFO_INT_MAX_CYCLES:				info->i = 158;									break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;						break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;						break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;						break;

		case CPUINFO_INT_INPUT_STATE + 0:			info->i = 0;  /* there is no level 0 */			break;
		case CPUINFO_INT_INPUT_STATE + 1:			info->i = (m68k->virq_state >> 1) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 2:			info->i = (m68k->virq_state >> 2) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 3:			info->i = (m68k->virq_state >> 3) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 4:			info->i = (m68k->virq_state >> 4) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 5:			info->i = (m68k->virq_state >> 5) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 6:			info->i = (m68k->virq_state >> 6) & 1;			break;
		case CPUINFO_INT_INPUT_STATE + 7:			info->i = (m68k->virq_state >> 7) & 1;			break;

		case CPUINFO_INT_PREVIOUSPC:				info->i = REG_PPC;								break;

		case CPUINFO_INT_PC:						info->i = REG_PC & 0x00ffffff; 					break;
		case CPUINFO_INT_REGISTER + M68K_PC:		info->i = REG_PC;								break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:		info->i = REG_SP;								break;
		case CPUINFO_INT_REGISTER + M68K_ISP:		info->i = (m68k->s_flag && !m68k->m_flag) ? REG_SP : REG_ISP; break;
		case CPUINFO_INT_REGISTER + M68K_USP:		info->i = m68k->s_flag ? REG_USP : REG_SP; 		break;
		case CPUINFO_INT_REGISTER + M68K_SR:		info->i = m68ki_get_sr(m68k);					break;
		case CPUINFO_INT_REGISTER + M68K_D0:		info->i = REG_D[0];								break;
		case CPUINFO_INT_REGISTER + M68K_D1:		info->i = REG_D[1]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D2:		info->i = REG_D[2]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D3:		info->i = REG_D[3]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D4:		info->i = REG_D[4]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D5:		info->i = REG_D[5]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D6:		info->i = REG_D[6]; 							break;
		case CPUINFO_INT_REGISTER + M68K_D7:		info->i = REG_D[7]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A0:		info->i = REG_A[0]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A1:		info->i = REG_A[1]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A2:		info->i = REG_A[2]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A3:		info->i = REG_A[3]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A4:		info->i = REG_A[4]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A5:		info->i = REG_A[5]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A6:		info->i = REG_A[6]; 							break;
		case CPUINFO_INT_REGISTER + M68K_A7:		info->i = REG_A[7]; 							break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:	info->i = m68k->pref_addr; 						break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:	info->i = m68k->pref_data; 						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					/* set per-core */								break;
		case CPUINFO_PTR_GET_CONTEXT:				info->getcontext = CPU_GET_CONTEXT_NAME(m68k);	break;
		case CPUINFO_PTR_SET_CONTEXT:				info->setcontext = CPU_SET_CONTEXT_NAME(m68k);	break;
		case CPUINFO_PTR_INIT:						/* set per-core */								break;
		case CPUINFO_PTR_RESET:						info->reset = CPU_RESET_NAME(m68k);				break;
		case CPUINFO_PTR_EXECUTE:					info->execute = CPU_EXECUTE_NAME(m68k);			break;
		case CPUINFO_PTR_DISASSEMBLE:				info->disassemble = CPU_DISASSEMBLE_NAME(m68k);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:		info->icount = &m68k->remaining_cycles;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						/* set per-core */								break;
		case CPUINFO_STR_CORE_FAMILY:				strcpy(info->s, "Motorola 68K");				break;
		case CPUINFO_STR_CORE_VERSION:				strcpy(info->s, "4.00");						break;
		case CPUINFO_STR_CORE_FILE:					strcpy(info->s, __FILE__);						break;
		case CPUINFO_STR_CORE_CREDITS:				strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68ki_get_sr(m68k);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? '?':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? '?':'.',
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

		case CPUINFO_STR_REGISTER + M68K_PC:		sprintf(info->s, "PC :%08X", REG_PC); 			break;
		case CPUINFO_STR_REGISTER + M68K_SR:  		sprintf(info->s, "SR :%04X", m68ki_get_sr(m68k)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  		sprintf(info->s, "SP :%08X", REG_SP); 			break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 		sprintf(info->s, "ISP:%08X", (m68k->s_flag && !m68k->m_flag) ? REG_SP : REG_ISP); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 		sprintf(info->s, "USP:%08X", m68k->s_flag ? REG_USP : REG_SP); break;
		case CPUINFO_STR_REGISTER + M68K_D0:		sprintf(info->s, "D0 :%08X", REG_D[0]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D1:		sprintf(info->s, "D1 :%08X", REG_D[1]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D2:		sprintf(info->s, "D2 :%08X", REG_D[2]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D3:		sprintf(info->s, "D3 :%08X", REG_D[3]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D4:		sprintf(info->s, "D4 :%08X", REG_D[4]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D5:		sprintf(info->s, "D5 :%08X", REG_D[5]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D6:		sprintf(info->s, "D6 :%08X", REG_D[6]); 		break;
		case CPUINFO_STR_REGISTER + M68K_D7:		sprintf(info->s, "D7 :%08X", REG_D[7]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A0:		sprintf(info->s, "A0 :%08X", REG_A[0]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A1:		sprintf(info->s, "A1 :%08X", REG_A[1]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A2:		sprintf(info->s, "A2 :%08X", REG_A[2]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A3:		sprintf(info->s, "A3 :%08X", REG_A[3]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A4:		sprintf(info->s, "A4 :%08X", REG_A[4]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A5:		sprintf(info->s, "A5 :%08X", REG_A[5]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A6:		sprintf(info->s, "A6 :%08X", REG_A[6]); 		break;
		case CPUINFO_STR_REGISTER + M68K_A7:		sprintf(info->s, "A7 :%08X", REG_A[7]); 		break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:	sprintf(info->s, "PAR:%08X", m68k->pref_addr); 	break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:	sprintf(info->s, "PDA:%08X", m68k->pref_data); 	break;
	}
}

static CPU_SET_INFO( m68k )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(m68k, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(m68k, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(m68k, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(m68k, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(m68k, 4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(m68k, 5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(m68k, 6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(m68k, 7, info->i);					break;

		case CPUINFO_INT_PC:  						m68ki_jump(m68k, info->i & 0x00ffffff); 		break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68ki_jump(m68k, info->i);						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		REG_SP = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		if(m68k->s_flag && !m68k->m_flag)
														REG_SP = info->i;
													else
														REG_ISP = info->i;
													break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		if(m68k->s_flag)
														REG_USP = info->i;
													else
														REG_SP = info->i;
													break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68ki_set_sr(m68k, info->i);					break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		REG_D[0] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		REG_D[1] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		REG_D[2] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		REG_D[3] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		REG_D[4] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		REG_D[5] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		REG_D[6] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		REG_D[7] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		REG_A[0] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		REG_A[1] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		REG_A[2] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		REG_A[3] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		REG_A[4] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		REG_A[5] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		REG_A[6] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		REG_A[7] = info->i;								break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:	m68k->pref_addr = info->i;						break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k->reset_instr_callback = (m68k_reset_func)info->f; break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k->cmpild_instr_callback = (m68k_cmpild_func)info->f; break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k->rte_instr_callback = (m68k_rte_func)info->f; break;
		case CPUINFO_PTR_M68K_TAS_CALLBACK:			m68k->tas_instr_callback = (m68k_tas_func)info->f; break;
	}
}


/* global access */

void m68k_set_encrypted_opcode_range(const device_config *device, offs_t start, offs_t end)
{
	m68ki_cpu_core *m68k = device->token;
	m68k->encrypted_start = start;
	m68k->encrypted_end = end;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

static UINT16 m68008_read_immediate_16(const address_space *space, offs_t address)
{
	offs_t addr = address;
	return (memory_decrypted_read_byte(space, addr) << 8) | (memory_decrypted_read_byte(space, addr + 1));
}

/* interface for 20/22-bit address bus, 8-bit data bus (68008) */
static const m68k_memory_interface interface_d8 =
{
	0,
	m68008_read_immediate_16,
	memory_read_byte_8be,
	memory_read_word_8be,
	memory_read_dword_8be,
	memory_write_byte_8be,
	memory_write_word_8be,
	memory_write_dword_8be
};

/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

static UINT16 read_immediate_16(const address_space *space, offs_t address)
{
	m68ki_cpu_core *m68k = space->cpu->token;
	return memory_decrypted_read_word(space, (address) ^ m68k->memory.opcode_xor);
}

static UINT16 simple_read_immediate_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_word(space, address);
}

/* interface for 24-bit address bus, 16-bit data bus (68000, 68010) */
static const m68k_memory_interface interface_d16 =
{
	0,
	simple_read_immediate_16,
	memory_read_byte_16be,
	memory_read_word_16be,
	memory_read_dword_16be,
	memory_write_byte_16be,
	memory_write_word_16be,
	memory_write_dword_16be
};

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT16 readword_d32(const address_space *space, offs_t address)
{
	UINT16 result;

	if (!(address & 1))
		return memory_read_word_32be(space, address);
	result = memory_read_byte_32be(space, address) << 8;
	return result | memory_read_byte_32be(space, address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writeword_d32(const address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
	{
		memory_write_word_32be(space, address, data);
		return;
	}
	memory_write_byte_32be(space, address, data >> 8);
	memory_write_byte_32be(space, address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT32 readlong_d32(const address_space *space, offs_t address)
{
	UINT32 result;

	if (!(address & 3))
		return memory_read_dword_32be(space, address);
	else if (!(address & 1))
	{
		result = memory_read_word_32be(space, address) << 16;
		return result | memory_read_word_32be(space, address + 2);
	}
	result = memory_read_byte_32be(space, address) << 24;
	result |= memory_read_word_32be(space, address + 1) << 8;
	return result | memory_read_byte_32be(space, address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writelong_d32(const address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 3))
	{
		memory_write_dword_32be(space, address, data);
		return;
	}
	else if (!(address & 1))
	{
		memory_write_word_32be(space, address, data >> 16);
		memory_write_word_32be(space, address + 2, data);
		return;
	}
	memory_write_byte_32be(space, address, data >> 24);
	memory_write_word_32be(space, address + 1, data >> 8);
	memory_write_byte_32be(space, address + 3, data);
}

/* interface for 32-bit data bus (68EC020, 68020) */
static const m68k_memory_interface interface_d32 =
{
	WORD_XOR_BE(0),
	read_immediate_16,
	memory_read_byte_32be,
	readword_d32,
	readlong_d32,
	memory_write_byte_32be,
	writeword_d32,
	writelong_d32
};



/****************************************************************************
 * 68000 section
 ****************************************************************************/

static CPU_INIT( m68000 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_000;
	m68k->dasm_type        = M68K_CPU_TYPE_68000;
	m68k->memory           = interface_d16;
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68000 )
{
	CPU_SET_INFO_CALL(m68k);
}

CPU_GET_INFO( m68000 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(m68000);		break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(m68000);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "68000");						break;

		default: 									CPU_GET_INFO_CALL(m68k);						break;
	}
}


/****************************************************************************
 * M68008 section
 ****************************************************************************/

static CPU_INIT( m68008 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_008;
	m68k->dasm_type        = M68K_CPU_TYPE_68008;
	m68k->memory           = interface_d8;
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68008 )
{
	CPU_SET_INFO_CALL(m68k);
}

CPU_GET_INFO( m68008 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 22;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(m68008);		break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(m68008);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "68008");						break;

		default: 									CPU_GET_INFO_CALL(m68k);						break;
	}
}


/****************************************************************************
 * M68010 section
 ****************************************************************************/

static CPU_INIT( m68010 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_010;
	m68k->dasm_type        = M68K_CPU_TYPE_68010;
	m68k->memory           = interface_d16;
	m68k->sr_mask          = 0xa71f; /* T1 -- S  -- -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68010 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  		m68k->vbr = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		m68k->sfc = info->i & 7;						break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		m68k->dfc = info->i & 7;						break;

		default:									CPU_SET_INFO_CALL(m68k);						break;
	}
}

CPU_GET_INFO( m68010 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  		info->i = m68k->vbr;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		info->i = m68k->sfc;							break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		info->i = m68k->dfc;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(m68010);		break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(m68010);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "68010");						break;
		case CPUINFO_STR_REGISTER + M68K_SFC:		sprintf(info->s, "SFC:%X",   m68k->sfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_DFC:		sprintf(info->s, "DFC:%X",   m68k->dfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_VBR:		sprintf(info->s, "VBR:%08X", m68k->vbr); 		break;

		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}


/****************************************************************************
 * M68020 section
 ****************************************************************************/

static CPU_INIT( m68020 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_020;
	m68k->dasm_type        = M68K_CPU_TYPE_68020;
	m68k->memory           = interface_d32;
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68020 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:  						m68ki_jump(m68k, info->i);					 	break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		if(m68k->s_flag && m68k->m_flag)
														REG_SP = info->i;
													else
														REG_MSP = info->i;
													break;
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k->cacr = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k->caar = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_VBR:  		m68k->vbr = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		m68k->sfc = info->i & 7;						break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		m68k->dfc = info->i & 7;						break;

		default:									CPU_SET_INFO_CALL(m68k);						break;
	}
}

CPU_GET_INFO( m68020 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i = 20;									break;
		case CPUINFO_INT_MIN_CYCLES:				info->i = 2;									break;
		case CPUINFO_INT_MAX_CYCLES:				info->i = 158;									break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;						break;

		case CPUINFO_INT_PC:						info->i = REG_PC;								break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		info->i = (m68k->s_flag && m68k->m_flag) ? REG_SP : REG_MSP; break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 		info->i = m68k->cacr;							break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 		info->i = m68k->caar;							break;
		case CPUINFO_INT_REGISTER + M68K_VBR:  		info->i = m68k->vbr;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		info->i = m68k->sfc;							break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		info->i = m68k->dfc;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(m68020);		break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(m68020);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "68020");						break;

		case CPUINFO_STR_FLAGS:
			sr = m68ki_get_sr(m68k);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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

		case CPUINFO_STR_REGISTER + M68K_MSP:		sprintf(info->s, "MSP:%08X", (m68k->s_flag && m68k->m_flag) ? REG_SP : REG_MSP); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:		sprintf(info->s, "CCR:%08X", m68k->cacr); 		break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:		sprintf(info->s, "CAR:%08X", m68k->caar); 		break;
		case CPUINFO_STR_REGISTER + M68K_SFC:		sprintf(info->s, "SFC:%X",   m68k->sfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_DFC:		sprintf(info->s, "DFC:%X",   m68k->dfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_VBR:		sprintf(info->s, "VBR:%08X", m68k->vbr); 		break;

		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}


/****************************************************************************
 * M680EC20 section
 ****************************************************************************/

static CPU_INIT( m68ec020 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_EC020;
	m68k->dasm_type        = M68K_CPU_TYPE_68EC020;
	m68k->memory           = interface_d32;
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68ec020 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:  						m68ki_jump(m68k, info->i & 0x00ffffff); 		break;

		default:									CPU_SET_INFO_CALL(m68020);						break;
	}
}

CPU_GET_INFO( m68ec020 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;						break;
		case CPUINFO_INT_PC:						info->i = REG_PC & 0x00ffffff; 					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = CPU_SET_INFO_NAME(m68ec020);	break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(m68ec020);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "68EC020");						break;

		default:									CPU_GET_INFO_CALL(m68020);						break;
	}
}

/****************************************************************************
 * M68040 section
 ****************************************************************************/

static CPU_INIT( m68040 )
{
	m68ki_cpu_core *m68k = device->token;

	CPU_INIT_CALL(m68k);

	m68k->cpu_type         = CPU_TYPE_040;
	m68k->dasm_type        = M68K_CPU_TYPE_68040;
	m68k->memory           = interface_d32;
	m68k->sr_mask          = 0xf71f; /* T1 T0 S  M  -- I2 I1 I0 -- -- -- X  N  Z  V  C  */
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
}

static CPU_SET_INFO( m68040 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:  						m68ki_jump(m68k, info->i);					 	break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		if(m68k->s_flag && m68k->m_flag)
														REG_SP = info->i;
													else
														REG_MSP = info->i;
													break;
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k->cacr = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k->caar = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_VBR:  		m68k->vbr = info->i;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		m68k->sfc = info->i & 7;						break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		m68k->dfc = info->i & 7;						break;

		default:									CPU_SET_INFO_CALL(m68k);						break;
	}
}

CPU_GET_INFO( m68040 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i = 20;								break;
		case CPUINFO_INT_MIN_CYCLES:				info->i = 2;								break;
		case CPUINFO_INT_MAX_CYCLES:				info->i = 158;								break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;

		case CPUINFO_INT_PC:						info->i = REG_PC;								break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		info->i = (m68k->s_flag && m68k->m_flag) ? REG_SP : REG_MSP; break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 		info->i = m68k->cacr;							break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 		info->i = m68k->caar;							break;
		case CPUINFO_INT_REGISTER + M68K_VBR:  		info->i = m68k->vbr;							break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  		info->i = m68k->sfc;							break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  		info->i = m68k->dfc;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68040);		break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68040);				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68040");				break;

		case CPUINFO_STR_FLAGS:
			sr = m68ki_get_sr(m68k);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
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

		case CPUINFO_STR_REGISTER + M68K_MSP:		sprintf(info->s, "MSP:%08X", (m68k->s_flag && m68k->m_flag) ? REG_SP : REG_MSP); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:		sprintf(info->s, "CCR:%08X", m68k->cacr); 		break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:		sprintf(info->s, "CAR:%08X", m68k->caar); 		break;
		case CPUINFO_STR_REGISTER + M68K_SFC:		sprintf(info->s, "SFC:%X",   m68k->sfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_DFC:		sprintf(info->s, "DFC:%X",   m68k->dfc); 		break;
		case CPUINFO_STR_REGISTER + M68K_VBR:		sprintf(info->s, "VBR:%08X", m68k->vbr); 		break;

		default:									CPU_GET_INFO_CALL(m68k);						break;
	}
}
