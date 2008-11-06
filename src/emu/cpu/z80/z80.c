/*****************************************************************************
 *
 *   z80.c
 *   Portable Z80 emulator V3.8
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *   Changes in 3.8 [Miodrag Milanovic]
 *   - Added MEMPTR register (according to informations provided 
 *     by Vladimir Kladov
 *   - BIT n,(HL) now return valid values due to use of MEMPTR
 *   - Fixed BIT 6,(XY+o) undocumented instructions
 *   Changes in 3.7 [Aaron Giles]
 *   - Changed NMI handling. NMIs are now latched in set_irq_state
 *     but are not taken there. Instead they are taken at the start of the
 *     execute loop.
 *   - Changed IRQ handling. IRQ state is set in set_irq_state but not taken
 *     except during the inner execute loop.
 *   - Removed x86 assembly hacks and obsolete timing loop catchers.
 *   Changes in 3.6
 *   - Got rid of the code that would inexactly emulate a Z80, i.e. removed
 *     all the #if Z80_EXACT #else branches.
 *   - Removed leading underscores from local register name shortcuts as
 *     this violates the C99 standard.
 *   - Renamed the registers inside the Z80 context to lower case to avoid
 *     ambiguities (shortcuts would have had the same names as the fields
 *     of the structure).
 *   Changes in 3.5
 *   - Implemented OTIR, INIR, etc. without look-up table for PF flag.
 *     [Ramsoft, Sean Young]
 *   Changes in 3.4
 *   - Removed Z80-MSX specific code as it's not needed any more.
 *   - Implemented DAA without look-up table [Ramsoft, Sean Young]
 *   Changes in 3.3
 *   - Fixed undocumented flags XF & YF in the non-asm versions of CP,
 *     and all the 16 bit arithmetic instructions. [Sean Young]
 *   Changes in 3.2
 *   - Fixed undocumented flags XF & YF of RRCA, and CF and HF of
 *     INI/IND/OUTI/OUTD/INIR/INDR/OTIR/OTDR [Sean Young]
 *   Changes in 3.1
 *   - removed the REPEAT_AT_ONCE execution of LDIR/CPIR etc. opcodes
 *     for readabilities sake and because the implementation was buggy
 *     (and I was not able to find the difference)
 *   Changes in 3.0
 *   - 'finished' switch to dynamically overrideable cycle count tables
 *   Changes in 2.9:
 *   - added methods to access and override the cycle count tables
 *   - fixed handling and timing of multiple DD/FD prefixed opcodes
 *   Changes in 2.8:
 *   - OUTI/OUTD/OTIR/OTDR also pre-decrement the B register now.
 *     This was wrong because of a bug fix on the wrong side
 *     (astrocade sound driver).
 *   Changes in 2.7:
 *    - removed z80_vm specific code, it's not needed (and never was).
 *   Changes in 2.6:
 *    - BUSY_LOOP_HACKS needed to call change_pc() earlier, before
 *      checking the opcodes at the new address, because otherwise they
 *      might access the old (wrong or even NULL) banked memory region.
 *      Thanks to Sean Young for finding this nasty bug.
 *   Changes in 2.5:
 *    - Burning cycles always adjusts the ICount by a multiple of 4.
 *    - In REPEAT_AT_ONCE cases the R register wasn't incremented twice
 *      per repetition as it should have been. Those repeated opcodes
 *      could also underflow the ICount.
 *    - Simplified TIME_LOOP_HACKS for BC and added two more for DE + HL
 *      timing loops. I think those hacks weren't endian safe before too.
 *   Changes in 2.4:
 *    - z80_reset zaps the entire context, sets IX and IY to 0xffff(!) and
 *      sets the Z flag. With these changes the Tehkan World Cup driver
 *      _seems_ to work again.
 *   Changes in 2.3:
 *    - External termination of the execution loop calls z80_burn() and
 *      z80_vm_burn() to burn an amount of cycles (R adjustment)
 *    - Shortcuts which burn CPU cycles (BUSY_LOOP_HACKS and TIME_LOOP_HACKS)
 *      now also adjust the R register depending on the skipped opcodes.
 *   Changes in 2.2:
 *    - Fixed bugs in CPL, SCF and CCF instructions flag handling.
 *    - Changed variable EA and ARG16() function to UINT32; this
 *      produces slightly more efficient code.
 *    - The DD/FD XY CB opcodes where XY is 40-7F and Y is not 6/E
 *      are changed to calls to the X6/XE opcodes to reduce object size.
 *      They're hardly ever used so this should not yield a speed penalty.
 *   New in 2.0:
 *    - Optional more exact Z80 emulation (#define Z80_EXACT 1) according
 *      to a detailed description by Sean Young which can be found at:
 *      http://www.msxnet.org/tech/z80-documented.pdf
 *****************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "driver.h"
#include "z80.h"
#include "z80daisy.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* execute main opcodes inside a big switch statement */
#ifndef BIG_SWITCH
#define BIG_SWITCH			1
#endif

/* big flags array for ADD/ADC/SUB/SBC/CP results */
#define BIG_FLAGS_ARRAY		1

/* on JP and JR opcodes check for tight loops */
#define BUSY_LOOP_HACKS		1


/****************************************************************************/
/* The Z80 registers. HALT is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(z80->r&127)|(z80->r2&128)      */
/****************************************************************************/
typedef struct _z80_state z80_state;
struct _z80_state
{
	PAIR	prvpc,pc,sp,af,bc,de,hl,ix,iy,memptr;
	PAIR	af2,bc2,de2,hl2;
	UINT8	r,r2,iff1,iff2,halt,im,i;
	UINT8	nmi_state;			/* nmi line state */
	UINT8	nmi_pending;		/* nmi pending */
	UINT8	irq_state;			/* irq line state */
	UINT8	after_ei;			/* are we in the EI shadow? */
	UINT32	ea;
	cpu_irq_callback irq_callback;
	const device_config *device;
	int		icount;
	z80_daisy_state *daisy;
};

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	PF
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PRVPC z80->prvpc.d		/* previous program counter */

#define PCD	z80->pc.d
#define PC z80->pc.w.l

#define SPD z80->sp.d
#define SP z80->sp.w.l

#define AFD z80->af.d
#define AF z80->af.w.l
#define A z80->af.b.h
#define F z80->af.b.l

#define BCD z80->bc.d
#define BC z80->bc.w.l
#define B z80->bc.b.h
#define C z80->bc.b.l

#define DED z80->de.d
#define DE z80->de.w.l
#define D z80->de.b.h
#define E z80->de.b.l

#define HLD z80->hl.d
#define HL z80->hl.w.l
#define H z80->hl.b.h
#define L z80->hl.b.l

#define IXD z80->ix.d
#define IX z80->ix.w.l
#define HX z80->ix.b.h
#define LX z80->ix.b.l

#define IYD z80->iy.d
#define IY z80->iy.w.l
#define HY z80->iy.b.h
#define LY z80->iy.b.l

#define MEMPTR z80->memptr.w.l
#define MEMPTR_H z80->memptr.b.h
#define MEMPTR_L z80->memptr.b.l

#define EA z80->ea


#define I z80->i
#define R z80->r
#define R2 z80->r2
#define IM z80->im
#define IFF1 z80->iff1
#define IFF2 z80->iff2
#define HALT z80->halt

static z80_state *token;

static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

#if BIG_FLAGS_ARRAY
static UINT8 *SZHVC_add = 0;
static UINT8 *SZHVC_sub = 0;
#endif

static const UINT8 cc_op[0x100] = {
 4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
 8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
 7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
 7,10,13, 6,11,11,10, 4, 7,11,13, 6, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 7, 7, 7, 7, 7, 7, 4, 7, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
 5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,
 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,
 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,
 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11};

static const UINT8 cc_cb[0x100] = {
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8};

static const UINT8 cc_ed[0x100] = {
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
12,12,15,20, 8,14, 8, 9,12,12,15,20, 8,14, 8, 9,
12,12,15,20, 8,14, 8,18,12,12,15,20, 8,14, 8,18,
12,12,15,20, 8,14, 8, 8,12,12,15,20, 8,14, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
16,16,16,16, 8, 8, 8, 8,16,16,16,16, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};

static const UINT8 cc_xy[0x100] = {
 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4,14,20,10, 9, 9, 9, 4, 4,15,20,10, 9, 9, 9, 4,
 4, 4, 4, 4,23,23,19, 4, 4,15, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 9, 9, 9, 9, 9, 9,19, 9, 9, 9, 9, 9, 9, 9,19, 9,
19,19,19,19,19,19, 4,19, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 9, 9,19, 4, 4, 4, 4, 4, 9, 9,19, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 4,14, 4,23, 4,15, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4,
 4, 4, 4, 4, 4, 4, 4, 4, 4,10, 4, 4, 4, 4, 4, 4};

static const UINT8 cc_xycb[0x100] = {
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const UINT8 cc_ex[0x100] = {
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* DJNZ */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NZ/JR Z */
 5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0,	/* JR NC/JR C */
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0,	/* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2};

static const UINT8 *cc[6];
#define Z80_TABLE_dd	Z80_TABLE_xy
#define Z80_TABLE_fd	Z80_TABLE_xy

static void take_interrupt(z80_state *z80);
static void z80_burn(int cycles);

typedef void (*funcptr)(z80_state *z80);

#define PROTOTYPES(tablename,prefix) \
	INLINE void prefix##_00(z80_state *z80); INLINE void prefix##_01(z80_state *z80); INLINE void prefix##_02(z80_state *z80); INLINE void prefix##_03(z80_state *z80); \
	INLINE void prefix##_04(z80_state *z80); INLINE void prefix##_05(z80_state *z80); INLINE void prefix##_06(z80_state *z80); INLINE void prefix##_07(z80_state *z80); \
	INLINE void prefix##_08(z80_state *z80); INLINE void prefix##_09(z80_state *z80); INLINE void prefix##_0a(z80_state *z80); INLINE void prefix##_0b(z80_state *z80); \
	INLINE void prefix##_0c(z80_state *z80); INLINE void prefix##_0d(z80_state *z80); INLINE void prefix##_0e(z80_state *z80); INLINE void prefix##_0f(z80_state *z80); \
	INLINE void prefix##_10(z80_state *z80); INLINE void prefix##_11(z80_state *z80); INLINE void prefix##_12(z80_state *z80); INLINE void prefix##_13(z80_state *z80); \
	INLINE void prefix##_14(z80_state *z80); INLINE void prefix##_15(z80_state *z80); INLINE void prefix##_16(z80_state *z80); INLINE void prefix##_17(z80_state *z80); \
	INLINE void prefix##_18(z80_state *z80); INLINE void prefix##_19(z80_state *z80); INLINE void prefix##_1a(z80_state *z80); INLINE void prefix##_1b(z80_state *z80); \
	INLINE void prefix##_1c(z80_state *z80); INLINE void prefix##_1d(z80_state *z80); INLINE void prefix##_1e(z80_state *z80); INLINE void prefix##_1f(z80_state *z80); \
	INLINE void prefix##_20(z80_state *z80); INLINE void prefix##_21(z80_state *z80); INLINE void prefix##_22(z80_state *z80); INLINE void prefix##_23(z80_state *z80); \
	INLINE void prefix##_24(z80_state *z80); INLINE void prefix##_25(z80_state *z80); INLINE void prefix##_26(z80_state *z80); INLINE void prefix##_27(z80_state *z80); \
	INLINE void prefix##_28(z80_state *z80); INLINE void prefix##_29(z80_state *z80); INLINE void prefix##_2a(z80_state *z80); INLINE void prefix##_2b(z80_state *z80); \
	INLINE void prefix##_2c(z80_state *z80); INLINE void prefix##_2d(z80_state *z80); INLINE void prefix##_2e(z80_state *z80); INLINE void prefix##_2f(z80_state *z80); \
	INLINE void prefix##_30(z80_state *z80); INLINE void prefix##_31(z80_state *z80); INLINE void prefix##_32(z80_state *z80); INLINE void prefix##_33(z80_state *z80); \
	INLINE void prefix##_34(z80_state *z80); INLINE void prefix##_35(z80_state *z80); INLINE void prefix##_36(z80_state *z80); INLINE void prefix##_37(z80_state *z80); \
	INLINE void prefix##_38(z80_state *z80); INLINE void prefix##_39(z80_state *z80); INLINE void prefix##_3a(z80_state *z80); INLINE void prefix##_3b(z80_state *z80); \
	INLINE void prefix##_3c(z80_state *z80); INLINE void prefix##_3d(z80_state *z80); INLINE void prefix##_3e(z80_state *z80); INLINE void prefix##_3f(z80_state *z80); \
	INLINE void prefix##_40(z80_state *z80); INLINE void prefix##_41(z80_state *z80); INLINE void prefix##_42(z80_state *z80); INLINE void prefix##_43(z80_state *z80); \
	INLINE void prefix##_44(z80_state *z80); INLINE void prefix##_45(z80_state *z80); INLINE void prefix##_46(z80_state *z80); INLINE void prefix##_47(z80_state *z80); \
	INLINE void prefix##_48(z80_state *z80); INLINE void prefix##_49(z80_state *z80); INLINE void prefix##_4a(z80_state *z80); INLINE void prefix##_4b(z80_state *z80); \
	INLINE void prefix##_4c(z80_state *z80); INLINE void prefix##_4d(z80_state *z80); INLINE void prefix##_4e(z80_state *z80); INLINE void prefix##_4f(z80_state *z80); \
	INLINE void prefix##_50(z80_state *z80); INLINE void prefix##_51(z80_state *z80); INLINE void prefix##_52(z80_state *z80); INLINE void prefix##_53(z80_state *z80); \
	INLINE void prefix##_54(z80_state *z80); INLINE void prefix##_55(z80_state *z80); INLINE void prefix##_56(z80_state *z80); INLINE void prefix##_57(z80_state *z80); \
	INLINE void prefix##_58(z80_state *z80); INLINE void prefix##_59(z80_state *z80); INLINE void prefix##_5a(z80_state *z80); INLINE void prefix##_5b(z80_state *z80); \
	INLINE void prefix##_5c(z80_state *z80); INLINE void prefix##_5d(z80_state *z80); INLINE void prefix##_5e(z80_state *z80); INLINE void prefix##_5f(z80_state *z80); \
	INLINE void prefix##_60(z80_state *z80); INLINE void prefix##_61(z80_state *z80); INLINE void prefix##_62(z80_state *z80); INLINE void prefix##_63(z80_state *z80); \
	INLINE void prefix##_64(z80_state *z80); INLINE void prefix##_65(z80_state *z80); INLINE void prefix##_66(z80_state *z80); INLINE void prefix##_67(z80_state *z80); \
	INLINE void prefix##_68(z80_state *z80); INLINE void prefix##_69(z80_state *z80); INLINE void prefix##_6a(z80_state *z80); INLINE void prefix##_6b(z80_state *z80); \
	INLINE void prefix##_6c(z80_state *z80); INLINE void prefix##_6d(z80_state *z80); INLINE void prefix##_6e(z80_state *z80); INLINE void prefix##_6f(z80_state *z80); \
	INLINE void prefix##_70(z80_state *z80); INLINE void prefix##_71(z80_state *z80); INLINE void prefix##_72(z80_state *z80); INLINE void prefix##_73(z80_state *z80); \
	INLINE void prefix##_74(z80_state *z80); INLINE void prefix##_75(z80_state *z80); INLINE void prefix##_76(z80_state *z80); INLINE void prefix##_77(z80_state *z80); \
	INLINE void prefix##_78(z80_state *z80); INLINE void prefix##_79(z80_state *z80); INLINE void prefix##_7a(z80_state *z80); INLINE void prefix##_7b(z80_state *z80); \
	INLINE void prefix##_7c(z80_state *z80); INLINE void prefix##_7d(z80_state *z80); INLINE void prefix##_7e(z80_state *z80); INLINE void prefix##_7f(z80_state *z80); \
	INLINE void prefix##_80(z80_state *z80); INLINE void prefix##_81(z80_state *z80); INLINE void prefix##_82(z80_state *z80); INLINE void prefix##_83(z80_state *z80); \
	INLINE void prefix##_84(z80_state *z80); INLINE void prefix##_85(z80_state *z80); INLINE void prefix##_86(z80_state *z80); INLINE void prefix##_87(z80_state *z80); \
	INLINE void prefix##_88(z80_state *z80); INLINE void prefix##_89(z80_state *z80); INLINE void prefix##_8a(z80_state *z80); INLINE void prefix##_8b(z80_state *z80); \
	INLINE void prefix##_8c(z80_state *z80); INLINE void prefix##_8d(z80_state *z80); INLINE void prefix##_8e(z80_state *z80); INLINE void prefix##_8f(z80_state *z80); \
	INLINE void prefix##_90(z80_state *z80); INLINE void prefix##_91(z80_state *z80); INLINE void prefix##_92(z80_state *z80); INLINE void prefix##_93(z80_state *z80); \
	INLINE void prefix##_94(z80_state *z80); INLINE void prefix##_95(z80_state *z80); INLINE void prefix##_96(z80_state *z80); INLINE void prefix##_97(z80_state *z80); \
	INLINE void prefix##_98(z80_state *z80); INLINE void prefix##_99(z80_state *z80); INLINE void prefix##_9a(z80_state *z80); INLINE void prefix##_9b(z80_state *z80); \
	INLINE void prefix##_9c(z80_state *z80); INLINE void prefix##_9d(z80_state *z80); INLINE void prefix##_9e(z80_state *z80); INLINE void prefix##_9f(z80_state *z80); \
	INLINE void prefix##_a0(z80_state *z80); INLINE void prefix##_a1(z80_state *z80); INLINE void prefix##_a2(z80_state *z80); INLINE void prefix##_a3(z80_state *z80); \
	INLINE void prefix##_a4(z80_state *z80); INLINE void prefix##_a5(z80_state *z80); INLINE void prefix##_a6(z80_state *z80); INLINE void prefix##_a7(z80_state *z80); \
	INLINE void prefix##_a8(z80_state *z80); INLINE void prefix##_a9(z80_state *z80); INLINE void prefix##_aa(z80_state *z80); INLINE void prefix##_ab(z80_state *z80); \
	INLINE void prefix##_ac(z80_state *z80); INLINE void prefix##_ad(z80_state *z80); INLINE void prefix##_ae(z80_state *z80); INLINE void prefix##_af(z80_state *z80); \
	INLINE void prefix##_b0(z80_state *z80); INLINE void prefix##_b1(z80_state *z80); INLINE void prefix##_b2(z80_state *z80); INLINE void prefix##_b3(z80_state *z80); \
	INLINE void prefix##_b4(z80_state *z80); INLINE void prefix##_b5(z80_state *z80); INLINE void prefix##_b6(z80_state *z80); INLINE void prefix##_b7(z80_state *z80); \
	INLINE void prefix##_b8(z80_state *z80); INLINE void prefix##_b9(z80_state *z80); INLINE void prefix##_ba(z80_state *z80); INLINE void prefix##_bb(z80_state *z80); \
	INLINE void prefix##_bc(z80_state *z80); INLINE void prefix##_bd(z80_state *z80); INLINE void prefix##_be(z80_state *z80); INLINE void prefix##_bf(z80_state *z80); \
	INLINE void prefix##_c0(z80_state *z80); INLINE void prefix##_c1(z80_state *z80); INLINE void prefix##_c2(z80_state *z80); INLINE void prefix##_c3(z80_state *z80); \
	INLINE void prefix##_c4(z80_state *z80); INLINE void prefix##_c5(z80_state *z80); INLINE void prefix##_c6(z80_state *z80); INLINE void prefix##_c7(z80_state *z80); \
	INLINE void prefix##_c8(z80_state *z80); INLINE void prefix##_c9(z80_state *z80); INLINE void prefix##_ca(z80_state *z80); INLINE void prefix##_cb(z80_state *z80); \
	INLINE void prefix##_cc(z80_state *z80); INLINE void prefix##_cd(z80_state *z80); INLINE void prefix##_ce(z80_state *z80); INLINE void prefix##_cf(z80_state *z80); \
	INLINE void prefix##_d0(z80_state *z80); INLINE void prefix##_d1(z80_state *z80); INLINE void prefix##_d2(z80_state *z80); INLINE void prefix##_d3(z80_state *z80); \
	INLINE void prefix##_d4(z80_state *z80); INLINE void prefix##_d5(z80_state *z80); INLINE void prefix##_d6(z80_state *z80); INLINE void prefix##_d7(z80_state *z80); \
	INLINE void prefix##_d8(z80_state *z80); INLINE void prefix##_d9(z80_state *z80); INLINE void prefix##_da(z80_state *z80); INLINE void prefix##_db(z80_state *z80); \
	INLINE void prefix##_dc(z80_state *z80); INLINE void prefix##_dd(z80_state *z80); INLINE void prefix##_de(z80_state *z80); INLINE void prefix##_df(z80_state *z80); \
	INLINE void prefix##_e0(z80_state *z80); INLINE void prefix##_e1(z80_state *z80); INLINE void prefix##_e2(z80_state *z80); INLINE void prefix##_e3(z80_state *z80); \
	INLINE void prefix##_e4(z80_state *z80); INLINE void prefix##_e5(z80_state *z80); INLINE void prefix##_e6(z80_state *z80); INLINE void prefix##_e7(z80_state *z80); \
	INLINE void prefix##_e8(z80_state *z80); INLINE void prefix##_e9(z80_state *z80); INLINE void prefix##_ea(z80_state *z80); INLINE void prefix##_eb(z80_state *z80); \
	INLINE void prefix##_ec(z80_state *z80); INLINE void prefix##_ed(z80_state *z80); INLINE void prefix##_ee(z80_state *z80); INLINE void prefix##_ef(z80_state *z80); \
	INLINE void prefix##_f0(z80_state *z80); INLINE void prefix##_f1(z80_state *z80); INLINE void prefix##_f2(z80_state *z80); INLINE void prefix##_f3(z80_state *z80); \
	INLINE void prefix##_f4(z80_state *z80); INLINE void prefix##_f5(z80_state *z80); INLINE void prefix##_f6(z80_state *z80); INLINE void prefix##_f7(z80_state *z80); \
	INLINE void prefix##_f8(z80_state *z80); INLINE void prefix##_f9(z80_state *z80); INLINE void prefix##_fa(z80_state *z80); INLINE void prefix##_fb(z80_state *z80); \
	INLINE void prefix##_fc(z80_state *z80); INLINE void prefix##_fd(z80_state *z80); INLINE void prefix##_fe(z80_state *z80); INLINE void prefix##_ff(z80_state *z80); \
static const funcptr tablename[0x100] = {	\
	prefix##_00,prefix##_01,prefix##_02,prefix##_03,prefix##_04,prefix##_05,prefix##_06,prefix##_07, \
	prefix##_08,prefix##_09,prefix##_0a,prefix##_0b,prefix##_0c,prefix##_0d,prefix##_0e,prefix##_0f, \
	prefix##_10,prefix##_11,prefix##_12,prefix##_13,prefix##_14,prefix##_15,prefix##_16,prefix##_17, \
	prefix##_18,prefix##_19,prefix##_1a,prefix##_1b,prefix##_1c,prefix##_1d,prefix##_1e,prefix##_1f, \
	prefix##_20,prefix##_21,prefix##_22,prefix##_23,prefix##_24,prefix##_25,prefix##_26,prefix##_27, \
	prefix##_28,prefix##_29,prefix##_2a,prefix##_2b,prefix##_2c,prefix##_2d,prefix##_2e,prefix##_2f, \
	prefix##_30,prefix##_31,prefix##_32,prefix##_33,prefix##_34,prefix##_35,prefix##_36,prefix##_37, \
	prefix##_38,prefix##_39,prefix##_3a,prefix##_3b,prefix##_3c,prefix##_3d,prefix##_3e,prefix##_3f, \
	prefix##_40,prefix##_41,prefix##_42,prefix##_43,prefix##_44,prefix##_45,prefix##_46,prefix##_47, \
	prefix##_48,prefix##_49,prefix##_4a,prefix##_4b,prefix##_4c,prefix##_4d,prefix##_4e,prefix##_4f, \
	prefix##_50,prefix##_51,prefix##_52,prefix##_53,prefix##_54,prefix##_55,prefix##_56,prefix##_57, \
	prefix##_58,prefix##_59,prefix##_5a,prefix##_5b,prefix##_5c,prefix##_5d,prefix##_5e,prefix##_5f, \
	prefix##_60,prefix##_61,prefix##_62,prefix##_63,prefix##_64,prefix##_65,prefix##_66,prefix##_67, \
	prefix##_68,prefix##_69,prefix##_6a,prefix##_6b,prefix##_6c,prefix##_6d,prefix##_6e,prefix##_6f, \
	prefix##_70,prefix##_71,prefix##_72,prefix##_73,prefix##_74,prefix##_75,prefix##_76,prefix##_77, \
	prefix##_78,prefix##_79,prefix##_7a,prefix##_7b,prefix##_7c,prefix##_7d,prefix##_7e,prefix##_7f, \
	prefix##_80,prefix##_81,prefix##_82,prefix##_83,prefix##_84,prefix##_85,prefix##_86,prefix##_87, \
	prefix##_88,prefix##_89,prefix##_8a,prefix##_8b,prefix##_8c,prefix##_8d,prefix##_8e,prefix##_8f, \
	prefix##_90,prefix##_91,prefix##_92,prefix##_93,prefix##_94,prefix##_95,prefix##_96,prefix##_97, \
	prefix##_98,prefix##_99,prefix##_9a,prefix##_9b,prefix##_9c,prefix##_9d,prefix##_9e,prefix##_9f, \
	prefix##_a0,prefix##_a1,prefix##_a2,prefix##_a3,prefix##_a4,prefix##_a5,prefix##_a6,prefix##_a7, \
	prefix##_a8,prefix##_a9,prefix##_aa,prefix##_ab,prefix##_ac,prefix##_ad,prefix##_ae,prefix##_af, \
	prefix##_b0,prefix##_b1,prefix##_b2,prefix##_b3,prefix##_b4,prefix##_b5,prefix##_b6,prefix##_b7, \
	prefix##_b8,prefix##_b9,prefix##_ba,prefix##_bb,prefix##_bc,prefix##_bd,prefix##_be,prefix##_bf, \
	prefix##_c0,prefix##_c1,prefix##_c2,prefix##_c3,prefix##_c4,prefix##_c5,prefix##_c6,prefix##_c7, \
	prefix##_c8,prefix##_c9,prefix##_ca,prefix##_cb,prefix##_cc,prefix##_cd,prefix##_ce,prefix##_cf, \
	prefix##_d0,prefix##_d1,prefix##_d2,prefix##_d3,prefix##_d4,prefix##_d5,prefix##_d6,prefix##_d7, \
	prefix##_d8,prefix##_d9,prefix##_da,prefix##_db,prefix##_dc,prefix##_dd,prefix##_de,prefix##_df, \
	prefix##_e0,prefix##_e1,prefix##_e2,prefix##_e3,prefix##_e4,prefix##_e5,prefix##_e6,prefix##_e7, \
	prefix##_e8,prefix##_e9,prefix##_ea,prefix##_eb,prefix##_ec,prefix##_ed,prefix##_ee,prefix##_ef, \
	prefix##_f0,prefix##_f1,prefix##_f2,prefix##_f3,prefix##_f4,prefix##_f5,prefix##_f6,prefix##_f7, \
	prefix##_f8,prefix##_f9,prefix##_fa,prefix##_fb,prefix##_fc,prefix##_fd,prefix##_fe,prefix##_ff  \
}

PROTOTYPES(Z80op,op);
PROTOTYPES(Z80cb,cb);
PROTOTYPES(Z80dd,dd);
PROTOTYPES(Z80ed,ed);
PROTOTYPES(Z80fd,fd);
PROTOTYPES(Z80xycb,xycb);

/****************************************************************************/
/* Burn an odd amount of cycles, that is instructions taking something      */
/* different from 4 T-states per opcode (and R increment)                   */
/****************************************************************************/
INLINE void BURNODD(z80_state *z80, int cycles, int opcodes, int cyclesum)
{
	if( cycles > 0 )
	{
		R += (cycles / cyclesum) * opcodes;
		z80->icount -= (cycles / cyclesum) * cyclesum;
	}
}

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(z80_state *z80)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) z80->icount -= cc[Z80_TABLE_##prefix][opcode]

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(z80,prefix,opcode)									\
{																\
	unsigned op = opcode;										\
	CC(prefix,op);												\
	(*Z80##prefix[op])(z80);									\
}

#if BIG_SWITCH
#define EXEC_INLINE(z80,prefix,opcode)							\
{																\
	unsigned op = opcode;										\
	CC(prefix,op);												\
	switch(op)													\
	{															\
	case 0x00:prefix##_##00(z80);break; case 0x01:prefix##_##01(z80);break; case 0x02:prefix##_##02(z80);break; case 0x03:prefix##_##03(z80);break; \
	case 0x04:prefix##_##04(z80);break; case 0x05:prefix##_##05(z80);break; case 0x06:prefix##_##06(z80);break; case 0x07:prefix##_##07(z80);break; \
	case 0x08:prefix##_##08(z80);break; case 0x09:prefix##_##09(z80);break; case 0x0a:prefix##_##0a(z80);break; case 0x0b:prefix##_##0b(z80);break; \
	case 0x0c:prefix##_##0c(z80);break; case 0x0d:prefix##_##0d(z80);break; case 0x0e:prefix##_##0e(z80);break; case 0x0f:prefix##_##0f(z80);break; \
	case 0x10:prefix##_##10(z80);break; case 0x11:prefix##_##11(z80);break; case 0x12:prefix##_##12(z80);break; case 0x13:prefix##_##13(z80);break; \
	case 0x14:prefix##_##14(z80);break; case 0x15:prefix##_##15(z80);break; case 0x16:prefix##_##16(z80);break; case 0x17:prefix##_##17(z80);break; \
	case 0x18:prefix##_##18(z80);break; case 0x19:prefix##_##19(z80);break; case 0x1a:prefix##_##1a(z80);break; case 0x1b:prefix##_##1b(z80);break; \
	case 0x1c:prefix##_##1c(z80);break; case 0x1d:prefix##_##1d(z80);break; case 0x1e:prefix##_##1e(z80);break; case 0x1f:prefix##_##1f(z80);break; \
	case 0x20:prefix##_##20(z80);break; case 0x21:prefix##_##21(z80);break; case 0x22:prefix##_##22(z80);break; case 0x23:prefix##_##23(z80);break; \
	case 0x24:prefix##_##24(z80);break; case 0x25:prefix##_##25(z80);break; case 0x26:prefix##_##26(z80);break; case 0x27:prefix##_##27(z80);break; \
	case 0x28:prefix##_##28(z80);break; case 0x29:prefix##_##29(z80);break; case 0x2a:prefix##_##2a(z80);break; case 0x2b:prefix##_##2b(z80);break; \
	case 0x2c:prefix##_##2c(z80);break; case 0x2d:prefix##_##2d(z80);break; case 0x2e:prefix##_##2e(z80);break; case 0x2f:prefix##_##2f(z80);break; \
	case 0x30:prefix##_##30(z80);break; case 0x31:prefix##_##31(z80);break; case 0x32:prefix##_##32(z80);break; case 0x33:prefix##_##33(z80);break; \
	case 0x34:prefix##_##34(z80);break; case 0x35:prefix##_##35(z80);break; case 0x36:prefix##_##36(z80);break; case 0x37:prefix##_##37(z80);break; \
	case 0x38:prefix##_##38(z80);break; case 0x39:prefix##_##39(z80);break; case 0x3a:prefix##_##3a(z80);break; case 0x3b:prefix##_##3b(z80);break; \
	case 0x3c:prefix##_##3c(z80);break; case 0x3d:prefix##_##3d(z80);break; case 0x3e:prefix##_##3e(z80);break; case 0x3f:prefix##_##3f(z80);break; \
	case 0x40:prefix##_##40(z80);break; case 0x41:prefix##_##41(z80);break; case 0x42:prefix##_##42(z80);break; case 0x43:prefix##_##43(z80);break; \
	case 0x44:prefix##_##44(z80);break; case 0x45:prefix##_##45(z80);break; case 0x46:prefix##_##46(z80);break; case 0x47:prefix##_##47(z80);break; \
	case 0x48:prefix##_##48(z80);break; case 0x49:prefix##_##49(z80);break; case 0x4a:prefix##_##4a(z80);break; case 0x4b:prefix##_##4b(z80);break; \
	case 0x4c:prefix##_##4c(z80);break; case 0x4d:prefix##_##4d(z80);break; case 0x4e:prefix##_##4e(z80);break; case 0x4f:prefix##_##4f(z80);break; \
	case 0x50:prefix##_##50(z80);break; case 0x51:prefix##_##51(z80);break; case 0x52:prefix##_##52(z80);break; case 0x53:prefix##_##53(z80);break; \
	case 0x54:prefix##_##54(z80);break; case 0x55:prefix##_##55(z80);break; case 0x56:prefix##_##56(z80);break; case 0x57:prefix##_##57(z80);break; \
	case 0x58:prefix##_##58(z80);break; case 0x59:prefix##_##59(z80);break; case 0x5a:prefix##_##5a(z80);break; case 0x5b:prefix##_##5b(z80);break; \
	case 0x5c:prefix##_##5c(z80);break; case 0x5d:prefix##_##5d(z80);break; case 0x5e:prefix##_##5e(z80);break; case 0x5f:prefix##_##5f(z80);break; \
	case 0x60:prefix##_##60(z80);break; case 0x61:prefix##_##61(z80);break; case 0x62:prefix##_##62(z80);break; case 0x63:prefix##_##63(z80);break; \
	case 0x64:prefix##_##64(z80);break; case 0x65:prefix##_##65(z80);break; case 0x66:prefix##_##66(z80);break; case 0x67:prefix##_##67(z80);break; \
	case 0x68:prefix##_##68(z80);break; case 0x69:prefix##_##69(z80);break; case 0x6a:prefix##_##6a(z80);break; case 0x6b:prefix##_##6b(z80);break; \
	case 0x6c:prefix##_##6c(z80);break; case 0x6d:prefix##_##6d(z80);break; case 0x6e:prefix##_##6e(z80);break; case 0x6f:prefix##_##6f(z80);break; \
	case 0x70:prefix##_##70(z80);break; case 0x71:prefix##_##71(z80);break; case 0x72:prefix##_##72(z80);break; case 0x73:prefix##_##73(z80);break; \
	case 0x74:prefix##_##74(z80);break; case 0x75:prefix##_##75(z80);break; case 0x76:prefix##_##76(z80);break; case 0x77:prefix##_##77(z80);break; \
	case 0x78:prefix##_##78(z80);break; case 0x79:prefix##_##79(z80);break; case 0x7a:prefix##_##7a(z80);break; case 0x7b:prefix##_##7b(z80);break; \
	case 0x7c:prefix##_##7c(z80);break; case 0x7d:prefix##_##7d(z80);break; case 0x7e:prefix##_##7e(z80);break; case 0x7f:prefix##_##7f(z80);break; \
	case 0x80:prefix##_##80(z80);break; case 0x81:prefix##_##81(z80);break; case 0x82:prefix##_##82(z80);break; case 0x83:prefix##_##83(z80);break; \
	case 0x84:prefix##_##84(z80);break; case 0x85:prefix##_##85(z80);break; case 0x86:prefix##_##86(z80);break; case 0x87:prefix##_##87(z80);break; \
	case 0x88:prefix##_##88(z80);break; case 0x89:prefix##_##89(z80);break; case 0x8a:prefix##_##8a(z80);break; case 0x8b:prefix##_##8b(z80);break; \
	case 0x8c:prefix##_##8c(z80);break; case 0x8d:prefix##_##8d(z80);break; case 0x8e:prefix##_##8e(z80);break; case 0x8f:prefix##_##8f(z80);break; \
	case 0x90:prefix##_##90(z80);break; case 0x91:prefix##_##91(z80);break; case 0x92:prefix##_##92(z80);break; case 0x93:prefix##_##93(z80);break; \
	case 0x94:prefix##_##94(z80);break; case 0x95:prefix##_##95(z80);break; case 0x96:prefix##_##96(z80);break; case 0x97:prefix##_##97(z80);break; \
	case 0x98:prefix##_##98(z80);break; case 0x99:prefix##_##99(z80);break; case 0x9a:prefix##_##9a(z80);break; case 0x9b:prefix##_##9b(z80);break; \
	case 0x9c:prefix##_##9c(z80);break; case 0x9d:prefix##_##9d(z80);break; case 0x9e:prefix##_##9e(z80);break; case 0x9f:prefix##_##9f(z80);break; \
	case 0xa0:prefix##_##a0(z80);break; case 0xa1:prefix##_##a1(z80);break; case 0xa2:prefix##_##a2(z80);break; case 0xa3:prefix##_##a3(z80);break; \
	case 0xa4:prefix##_##a4(z80);break; case 0xa5:prefix##_##a5(z80);break; case 0xa6:prefix##_##a6(z80);break; case 0xa7:prefix##_##a7(z80);break; \
	case 0xa8:prefix##_##a8(z80);break; case 0xa9:prefix##_##a9(z80);break; case 0xaa:prefix##_##aa(z80);break; case 0xab:prefix##_##ab(z80);break; \
	case 0xac:prefix##_##ac(z80);break; case 0xad:prefix##_##ad(z80);break; case 0xae:prefix##_##ae(z80);break; case 0xaf:prefix##_##af(z80);break; \
	case 0xb0:prefix##_##b0(z80);break; case 0xb1:prefix##_##b1(z80);break; case 0xb2:prefix##_##b2(z80);break; case 0xb3:prefix##_##b3(z80);break; \
	case 0xb4:prefix##_##b4(z80);break; case 0xb5:prefix##_##b5(z80);break; case 0xb6:prefix##_##b6(z80);break; case 0xb7:prefix##_##b7(z80);break; \
	case 0xb8:prefix##_##b8(z80);break; case 0xb9:prefix##_##b9(z80);break; case 0xba:prefix##_##ba(z80);break; case 0xbb:prefix##_##bb(z80);break; \
	case 0xbc:prefix##_##bc(z80);break; case 0xbd:prefix##_##bd(z80);break; case 0xbe:prefix##_##be(z80);break; case 0xbf:prefix##_##bf(z80);break; \
	case 0xc0:prefix##_##c0(z80);break; case 0xc1:prefix##_##c1(z80);break; case 0xc2:prefix##_##c2(z80);break; case 0xc3:prefix##_##c3(z80);break; \
	case 0xc4:prefix##_##c4(z80);break; case 0xc5:prefix##_##c5(z80);break; case 0xc6:prefix##_##c6(z80);break; case 0xc7:prefix##_##c7(z80);break; \
	case 0xc8:prefix##_##c8(z80);break; case 0xc9:prefix##_##c9(z80);break; case 0xca:prefix##_##ca(z80);break; case 0xcb:prefix##_##cb(z80);break; \
	case 0xcc:prefix##_##cc(z80);break; case 0xcd:prefix##_##cd(z80);break; case 0xce:prefix##_##ce(z80);break; case 0xcf:prefix##_##cf(z80);break; \
	case 0xd0:prefix##_##d0(z80);break; case 0xd1:prefix##_##d1(z80);break; case 0xd2:prefix##_##d2(z80);break; case 0xd3:prefix##_##d3(z80);break; \
	case 0xd4:prefix##_##d4(z80);break; case 0xd5:prefix##_##d5(z80);break; case 0xd6:prefix##_##d6(z80);break; case 0xd7:prefix##_##d7(z80);break; \
	case 0xd8:prefix##_##d8(z80);break; case 0xd9:prefix##_##d9(z80);break; case 0xda:prefix##_##da(z80);break; case 0xdb:prefix##_##db(z80);break; \
	case 0xdc:prefix##_##dc(z80);break; case 0xdd:prefix##_##dd(z80);break; case 0xde:prefix##_##de(z80);break; case 0xdf:prefix##_##df(z80);break; \
	case 0xe0:prefix##_##e0(z80);break; case 0xe1:prefix##_##e1(z80);break; case 0xe2:prefix##_##e2(z80);break; case 0xe3:prefix##_##e3(z80);break; \
	case 0xe4:prefix##_##e4(z80);break; case 0xe5:prefix##_##e5(z80);break; case 0xe6:prefix##_##e6(z80);break; case 0xe7:prefix##_##e7(z80);break; \
	case 0xe8:prefix##_##e8(z80);break; case 0xe9:prefix##_##e9(z80);break; case 0xea:prefix##_##ea(z80);break; case 0xeb:prefix##_##eb(z80);break; \
	case 0xec:prefix##_##ec(z80);break; case 0xed:prefix##_##ed(z80);break; case 0xee:prefix##_##ee(z80);break; case 0xef:prefix##_##ef(z80);break; \
	case 0xf0:prefix##_##f0(z80);break; case 0xf1:prefix##_##f1(z80);break; case 0xf2:prefix##_##f2(z80);break; case 0xf3:prefix##_##f3(z80);break; \
	case 0xf4:prefix##_##f4(z80);break; case 0xf5:prefix##_##f5(z80);break; case 0xf6:prefix##_##f6(z80);break; case 0xf7:prefix##_##f7(z80);break; \
	case 0xf8:prefix##_##f8(z80);break; case 0xf9:prefix##_##f9(z80);break; case 0xfa:prefix##_##fa(z80);break; case 0xfb:prefix##_##fb(z80);break; \
	case 0xfc:prefix##_##fc(z80);break; case 0xfd:prefix##_##fd(z80);break; case 0xfe:prefix##_##fe(z80);break; case 0xff:prefix##_##ff(z80);break; \
	}																																	\
}
#else
#define EXEC_INLINE EXEC
#endif


/***************************************************************
 * Enter HALT state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT {											\
	PC--;														\
	HALT = 1;													\
	if( z80->irq_state == CLEAR_LINE )							\
		z80_burn( z80->icount );								\
}

/***************************************************************
 * Leave HALT state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT {											\
	if( HALT )													\
	{															\
		HALT = 0;												\
		PC++;													\
	}															\
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(port)   ((UINT8)io_read_byte_8le(port))

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(port,value) io_write_byte_8le(port,value)

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(addr) (UINT8)program_read_byte_8le(addr)

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
INLINE void RM16( UINT32 addr, PAIR *r )
{
	r->b.l = RM(addr);
	r->b.h = RM((addr+1)&0xffff);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
#define WM(addr,value) program_write_byte_8le(addr,value)

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
INLINE void WM16( UINT32 addr, PAIR *r )
{
	WM(addr,r->b.l);
	WM((addr+1)&0xffff,r->b.h);
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
INLINE UINT8 ROP(z80_state *z80)
{
	unsigned pc = PCD;
	PC++;
	return cpu_readop(pc);
}

/****************************************************************
 * ARG(z80) is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
INLINE UINT8 ARG(z80_state *z80)
{
	unsigned pc = PCD;
	PC++;
	return cpu_readop_arg(pc);
}

INLINE UINT32 ARG16(z80_state *z80)
{
	unsigned pc = PCD;
	PC += 2;
	return cpu_readop_arg(pc) | (cpu_readop_arg((pc+1)&0xffff) << 8);
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX EA = (UINT32)(UINT16)(IX + (INT8)ARG(z80)); MEMPTR = EA
#define EAY EA = (UINT32)(UINT16)(IY + (INT8)ARG(z80)); MEMPTR = EA

/***************************************************************
 * POP
 ***************************************************************/
#define POP(DR) do { RM16( SPD, &z80->DR ); SP += 2; } while (0)

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(SR) do { SP -= 2; WM16( SPD, &z80->SR ); } while (0)

/***************************************************************
 * JP
 ***************************************************************/
#if BUSY_LOOP_HACKS
#define JP {													\
	unsigned oldpc = PCD-1;										\
	PCD = ARG16(z80);											\
	MEMPTR = PCD;												\
	change_pc(PCD);												\
	/* speed up busy loop */									\
	if( PCD == oldpc )											\
	{															\
		if( z80->irq_state == CLEAR_LINE )						\
			BURNODD( z80, z80->icount, 1, cc[Z80_TABLE_op][0xc3] );	\
	}															\
	else														\
	{															\
		UINT8 op = cpu_readop(PCD);								\
		if( PCD == oldpc-1 )									\
		{														\
			/* NOP - JP $-1 or EI - JP $-1 */					\
			if ( op == 0x00 || op == 0xfb )						\
			{													\
				if( z80->irq_state == CLEAR_LINE )				\
					BURNODD( z80, z80->icount-cc[Z80_TABLE_op][0x00], \
						2, cc[Z80_TABLE_op][0x00]+cc[Z80_TABLE_op][0xc3]); \
			}													\
		}														\
		else													\
		/* LD SP,#xxxx - JP $-3 (Galaga) */						\
		if( PCD == oldpc-3 && op == 0x31 )						\
		{														\
			if( z80->irq_state == CLEAR_LINE )					\
				BURNODD( z80, z80->icount-cc[Z80_TABLE_op][0x31],\
					2, cc[Z80_TABLE_op][0x31]+cc[Z80_TABLE_op][0xc3]); \
		}														\
	}															\
}
#else
#define JP {													\
	PCD = ARG16(z80);											\
	MEMPTR = PCD;												\
	change_pc(PCD);												\
}
#endif

/***************************************************************
 * JP_COND
 ***************************************************************/

#define JP_COND(cond)											\
	if( cond )													\
	{															\
		PCD = ARG16(z80);										\
		MEMPTR = PCD;											\
		change_pc(PCD);											\
	}															\
	else														\
	{															\
		MEMPTR = ARG16(z80); /* implicit do PC += 2 */			\
	}

/***************************************************************
 * JR
 ***************************************************************/
#define JR()													\
{																\
	unsigned oldpc = PCD-1;										\
	INT8 arg = (INT8)ARG(z80); /* ARG() also increments PC */	\
	PC += arg;				/* so don't do PC += ARG() */		\
	MEMPTR = PC;												\
	change_pc(PCD);												\
	/* speed up busy loop */									\
	if( PCD == oldpc )											\
	{															\
		if( z80->irq_state == CLEAR_LINE )						\
			BURNODD( z80, z80->icount, 1, cc[Z80_TABLE_op][0x18] );	\
	}															\
	else														\
	{															\
		UINT8 op = cpu_readop(PCD);								\
		if( PCD == oldpc-1 )									\
		{														\
			/* NOP - JR $-1 or EI - JR $-1 */					\
			if ( op == 0x00 || op == 0xfb )						\
			{													\
				if( z80->irq_state == CLEAR_LINE )				\
				   BURNODD( z80, z80->icount-cc[Z80_TABLE_op][0x00],	\
					   2, cc[Z80_TABLE_op][0x00]+cc[Z80_TABLE_op][0x18]); \
			}													\
		}														\
		else													\
		/* LD SP,#xxxx - JR $-3 */								\
		if( PCD == oldpc-3 && op == 0x31 )						\
		{														\
			if( z80->irq_state == CLEAR_LINE )					\
			   BURNODD( z80, z80->icount-cc[Z80_TABLE_op][0x31],	\
				   2, cc[Z80_TABLE_op][0x31]+cc[Z80_TABLE_op][0x18]); \
		}														\
	}															\
}

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(cond,opcode)									\
	if( cond )													\
	{															\
		INT8 arg = (INT8)ARG(z80); /* ARG() also increments PC */\
		PC += arg;				/* so don't do PC += ARG() */	\
		MEMPTR=PC;												\
		CC(ex,opcode);											\
		change_pc(PCD);											\
	}															\
	else PC++;													\

/***************************************************************
 * CALL
 ***************************************************************/
#define CALL()													\
	EA = ARG16(z80);											\
	MEMPTR = EA;												\
	PUSH( pc );													\
	PCD = EA;													\
	change_pc(PCD)

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(cond,opcode)									\
	if( cond )													\
	{															\
		EA = ARG16(z80);										\
		MEMPTR = EA;											\
		PUSH( pc );												\
		PCD = EA;												\
		CC(ex,opcode);											\
		change_pc(PCD);											\
	}															\
	else														\
	{															\
		MEMPTR = ARG16(z80);  /* implicit call PC+=2;	*/		\
	}

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(cond,opcode)									\
	if( cond )													\
	{															\
		POP( pc );												\
		MEMPTR = PC;											\
		change_pc(PCD);											\
		CC(ex,opcode);											\
	}

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN	{												\
	LOG(("Z80 #%d RETN IFF1:%d IFF2:%d\n", cpu_getactivecpu(), IFF1, IFF2)); \
	POP( pc );													\
	MEMPTR = PC;												\
	change_pc(PCD);												\
	IFF1 = IFF2;												\
}

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI	{												\
	POP( pc );													\
	MEMPTR = PC;												\
	change_pc(PCD);												\
/* according to http://www.msxnet.org/tech/z80-documented.pdf */\
	IFF1 = IFF2;												\
	if (z80->daisy)												\
		z80daisy_call_reti_device(z80->daisy);					\
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
#define LD_R_A {												\
	R = A;														\
	R2 = A & 0x80;				/* keep bit 7 of R */			\
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
#define LD_A_R {												\
	A = (R & 0x7f) | R2;										\
	F = (F & CF) | SZ[A] | ( IFF2 << 2 );						\
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
#define LD_I_A {												\
	I = A;														\
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
#define LD_A_I {												\
	A = I;														\
	F = (F & CF) | SZ[A] | ( IFF2 << 2 );						\
}

/***************************************************************
 * RST
 ***************************************************************/
#define RST(addr)												\
	PUSH( pc );													\
	PCD = addr;													\
	MEMPTR = PC;												\
	change_pc(PCD);							

/***************************************************************
 * INC  r8
 ***************************************************************/
INLINE UINT8 INC(z80_state *z80, UINT8 value)
{
	UINT8 res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
INLINE UINT8 DEC(z80_state *z80, UINT8 value)
{
	UINT8 res = value - 1;
	F = (F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA													\
	A = (A << 1) | (A >> 7);									\
	F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF))

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA													\
	F = (F & (SF | ZF | PF)) | (A & CF);						\
	A = (A >> 1) | (A << 7);									\
	F |= (A & (YF | XF) )

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA {													\
	UINT8 res = (A << 1) | (F & CF);							\
	UINT8 c = (A & 0x80) ? CF : 0;								\
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));			\
	A = res;													\
}

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA {													\
	UINT8 res = (A >> 1) | (F << 7);							\
	UINT8 c = (A & 0x01) ? CF : 0;								\
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));			\
	A = res;													\
}

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD {													\
	UINT8 n = RM(HL);											\
	MEMPTR = HL+1;												\
	WM( HL, (n >> 4) | (A << 4) );								\
	A = (A & 0xf0) | (n & 0x0f);								\
	F = (F & CF) | SZP[A];										\
}

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD {													\
	UINT8 n = RM(HL);											\
	MEMPTR = HL+1;												\
	WM( HL, (n << 4) | (A & 0x0f) );							\
	A = (A & 0xf0) | (n >> 4);									\
	F = (F & CF) | SZP[A];										\
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define ADD(value)												\
{																\
	UINT32 ah = AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) + value);					\
	F = SZHVC_add[ah | res];									\
	A = res;													\
}
#else
#define ADD(value)												\
{																\
	unsigned val = value;										\
	unsigned res = A + val;										\
	F = SZ[(UINT8)res] | ((res >> 8) & CF) |					\
		((A ^ res ^ val) & HF) |								\
		(((val ^ A ^ 0x80) & (val ^ res) & 0x80) >> 5);			\
	A = (UINT8)res;												\
}
#endif

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define ADC(value)												\
{																\
	UINT32 ah = AFD & 0xff00, c = AFD & 1;						\
	UINT32 res = (UINT8)((ah >> 8) + value + c);				\
	F = SZHVC_add[(c << 16) | ah | res];						\
	A = res;													\
}
#else
#define ADC(value)												\
{																\
	unsigned val = value;										\
	unsigned res = A + val + (F & CF);							\
	F = SZ[res & 0xff] | ((res >> 8) & CF) |					\
		((A ^ res ^ val) & HF) |								\
		(((val ^ A ^ 0x80) & (val ^ res) & 0x80) >> 5);			\
	A = res;													\
}
#endif

/***************************************************************
 * SUB  n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define SUB(value)												\
{																\
	UINT32 ah = AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - value);					\
	F = SZHVC_sub[ah | res];									\
	A = res;													\
}
#else
#define SUB(value)												\
{																\
	unsigned val = value;										\
	unsigned res = A - val;										\
	F = SZ[res & 0xff] | ((res >> 8) & CF) | NF |				\
		((A ^ res ^ val) & HF) |								\
		(((val ^ A) & (A ^ res) & 0x80) >> 5);					\
	A = res;													\
}
#endif

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define SBC(value)												\
{																\
	UINT32 ah = AFD & 0xff00, c = AFD & 1;						\
	UINT32 res = (UINT8)((ah >> 8) - value - c);				\
	F = SZHVC_sub[(c<<16) | ah | res];							\
	A = res;													\
}
#else
#define SBC(value)												\
{																\
	unsigned val = value;										\
	unsigned res = A - val - (F & CF);							\
	F = SZ[res & 0xff] | ((res >> 8) & CF) | NF |				\
		((A ^ res ^ val) & HF) |								\
		(((val ^ A) & (A ^ res) & 0x80) >> 5);					\
	A = res;													\
}
#endif

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG {													\
	UINT8 value = A;											\
	A = 0;														\
	SUB(value);													\
}

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA {													\
	UINT8 cf, nf, hf, lo, hi, diff;								\
	cf = F & CF;												\
	nf = F & NF;												\
	hf = F & HF;												\
	lo = A & 15;												\
	hi = A / 16;												\
																\
	if (cf)														\
	{															\
		diff = (lo <= 9 && !hf) ? 0x60 : 0x66;					\
	}															\
	else														\
	{															\
		if (lo >= 10)											\
		{														\
			diff = hi <= 8 ? 0x06 : 0x66;						\
		}														\
		else													\
		{														\
			if (hi >= 10)										\
			{													\
				diff = hf ? 0x66 : 0x60;						\
			}													\
			else												\
			{													\
				diff = hf ? 0x06 : 0x00;						\
			}													\
		}														\
	}															\
	if (nf) A -= diff;											\
	else A += diff;												\
																\
	F = SZP[A] | (F & NF);										\
	if (cf || (lo <= 9 ? hi >= 10 : hi >= 9)) F |= CF;			\
	if (nf ? hf && lo <= 5 : lo >= 10)	F |= HF;				\
}

/***************************************************************
 * AND  n
 ***************************************************************/
#define AND(value)												\
	A &= value;													\
	F = SZP[A] | HF

/***************************************************************
 * OR   n
 ***************************************************************/
#define OR(value)												\
	A |= value;													\
	F = SZP[A]

/***************************************************************
 * XOR  n
 ***************************************************************/
#define XOR(value)												\
	A ^= value;													\
	F = SZP[A]

/***************************************************************
 * CP   n
 ***************************************************************/
#if BIG_FLAGS_ARRAY
#define CP(value)												\
{																\
	unsigned val = value;										\
	UINT32 ah = AFD & 0xff00;									\
	UINT32 res = (UINT8)((ah >> 8) - val);						\
	F = (SZHVC_sub[ah | res] & ~(YF | XF)) |					\
		(val & (YF | XF));										\
}
#else
#define CP(value)												\
{																\
	unsigned val = value;										\
	unsigned res = A - val;										\
	F = (SZ[res & 0xff] & (SF | ZF)) |							\
		(val & (YF | XF)) | ((res >> 8) & CF) | NF |			\
		((A ^ res ^ val) & HF) |								\
		((((val ^ A) & (A ^ res)) >> 5) & VF);					\
}
#endif

/***************************************************************
 * EX   AF,AF'
 ***************************************************************/
#define EX_AF {													\
	PAIR tmp;													\
	tmp = z80->af; z80->af = z80->af2; z80->af2 = tmp;			\
}

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
#define EX_DE_HL {												\
	PAIR tmp;													\
	tmp = z80->de; z80->de = z80->hl; z80->hl = tmp;			\
}

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX {													\
	PAIR tmp;													\
	tmp = z80->bc; z80->bc = z80->bc2; z80->bc2 = tmp;			\
	tmp = z80->de; z80->de = z80->de2; z80->de2 = tmp;			\
	tmp = z80->hl; z80->hl = z80->hl2; z80->hl2 = tmp;			\
}

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
#define EXSP(DR)												\
{																\
	PAIR tmp = { { 0, 0, 0, 0 } };								\
	RM16( SPD, &tmp );											\
	WM16( SPD, &z80->DR );										\
	z80->DR = tmp;												\
	MEMPTR = z80->DR.d;											\
}


/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(DR,SR)											\
{																\
	UINT32 res = z80->DR.d + z80->SR.d;							\
	MEMPTR = z80->DR.d + 1;										\
	F = (F & (SF | ZF | VF)) |									\
		(((z80->DR.d ^ res ^ z80->SR.d) >> 8) & HF) |			\
		((res >> 16) & CF) | ((res >> 8) & (YF | XF));			\
	z80->DR.w.l = (UINT16)res;									\
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(Reg)												\
{																\
	UINT32 res = HLD + z80->Reg.d + (F & CF);					\
	MEMPTR = HL + 1;											\
	F = (((HLD ^ res ^ z80->Reg.d) >> 8) & HF) |				\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) |							\
		((res & 0xffff) ? 0 : ZF) |								\
		(((z80->Reg.d ^ HLD ^ 0x8000) & (z80->Reg.d ^ res) & 0x8000) >> 13); \
	HL = (UINT16)res;											\
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(Reg)												\
{																\
	UINT32 res = HLD - z80->Reg.d - (F & CF);					\
	MEMPTR = HL + 1;											\
	F = (((HLD ^ res ^ z80->Reg.d) >> 8) & HF) | NF |			\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) |							\
		((res & 0xffff) ? 0 : ZF) |								\
		(((z80->Reg.d ^ HLD) & (HLD ^ res) &0x8000) >> 13);		\
	HL = (UINT16)res;											\
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
INLINE UINT8 RLC(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RRC  r8
 ***************************************************************/
INLINE UINT8 RRC(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
INLINE UINT8 RL(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (F & CF)) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
INLINE UINT8 RR(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (F << 7)) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLA  r8
 ***************************************************************/
INLINE UINT8 SLA(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRA  r8
 ***************************************************************/
INLINE UINT8 SRA(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (res & 0x80)) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SLL  r8
 ***************************************************************/
INLINE UINT8 SLL(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | 0x01) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
INLINE UINT8 SRL(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = (res >> 1) & 0xff;
	F = SZP[res] | c;
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(bit,reg)											\
	F = (F & CF) | HF | SZ_BIT[reg & (1<<bit)]

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
#define BIT_HL(bit,reg)											\
	F = (F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | (MEMPTR_H & (YF|XF))

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/

#define BIT_XY(bit,reg)											\
	F = (F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((EA>>8) & (YF|XF))

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
INLINE UINT8 RES(UINT8 bit, UINT8 value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
INLINE UINT8 SET(UINT8 bit, UINT8 value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
#define LDI {													\
	UINT8 io = RM(HL);											\
	WM( DE, io );												\
	F &= SF | ZF | CF;											\
	if( (A + io) & 0x02 ) F |= YF; /* bit 1 -> flag 5 */		\
	if( (A + io) & 0x08 ) F |= XF; /* bit 3 -> flag 3 */		\
	HL++; DE++; BC--;											\
	if( BC ) F |= VF;											\
}

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI {													\
	UINT8 val = RM(HL);											\
	UINT8 res = A - val;										\
	MEMPTR++;													\
	HL++; BC--;													\
	F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;	\
	if( F & HF ) res -= 1;										\
	if( res & 0x02 ) F |= YF; /* bit 1 -> flag 5 */				\
	if( res & 0x08 ) F |= XF; /* bit 3 -> flag 3 */				\
	if( BC ) F |= VF;											\
}

/***************************************************************
 * INI
 ***************************************************************/
#define INI {													\
	unsigned t;													\
	UINT8 io = IN(BC);											\
	MEMPTR = BC + 1;											\
	B--;														\
	WM( HL, io );												\
	HL++;														\
	F = SZ[B];													\
	t = (unsigned)((C + 1) & 0xff) + (unsigned)io;				\
	if( io & SF ) F |= NF;										\
	if( t & 0x100 ) F |= HF | CF;								\
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;						\
}

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI {													\
	unsigned t;													\
	UINT8 io = RM(HL);											\
	B--;														\
	MEMPTR = BC + 1;											\
	OUT( BC, io );												\
	HL++;														\
	F = SZ[B];													\
	t = (unsigned)L + (unsigned)io;								\
	if( io & SF ) F |= NF;										\
	if( t & 0x100 ) F |= HF | CF;								\
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;						\
}

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD {													\
	UINT8 io = RM(HL);											\
	WM( DE, io );												\
	F &= SF | ZF | CF;											\
	if( (A + io) & 0x02 ) F |= YF; /* bit 1 -> flag 5 */		\
	if( (A + io) & 0x08 ) F |= XF; /* bit 3 -> flag 3 */		\
	HL--; DE--; BC--;											\
	if( BC ) F |= VF;											\
}

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD {													\
	UINT8 val = RM(HL);											\
	UINT8 res = A - val;										\
	MEMPTR--;													\
	HL--; BC--;													\
	F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;	\
	if( F & HF ) res -= 1;										\
	if( res & 0x02 ) F |= YF; /* bit 1 -> flag 5 */				\
	if( res & 0x08 ) F |= XF; /* bit 3 -> flag 3 */				\
	if( BC ) F |= VF;											\
}

/***************************************************************
 * IND
 ***************************************************************/
#define IND {													\
	unsigned t;													\
	UINT8 io = IN(BC);											\
	MEMPTR = BC - 1;											\
	B--;														\
	WM( HL, io );												\
	HL--;														\
	F = SZ[B];													\
	t = ((unsigned)(C - 1) & 0xff) + (unsigned)io;				\
	if( io & SF ) F |= NF;										\
	if( t & 0x100 ) F |= HF | CF;								\
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;						\
}

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD {													\
	unsigned t;													\
	UINT8 io = RM(HL);											\
	B--;														\
	MEMPTR = BC - 1;											\
	OUT( BC, io );												\
	HL--;														\
	F = SZ[B];													\
	t = (unsigned)L + (unsigned)io;								\
	if( io & SF ) F |= NF;										\
	if( t & 0x100 ) F |= HF | CF;								\
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;						\
}

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR													\
	LDI;														\
	if( BC )													\
	{															\
		PC -= 2;												\
		MEMPTR = PC + 1;										\
		CC(ex,0xb0);											\
	}

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR													\
	CPI;														\
	if( BC && !(F & ZF) )										\
	{															\
		PC -= 2;												\
		MEMPTR = PC + 1;										\
		CC(ex,0xb1);											\
	}

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR													\
	INI;														\
	if( B )														\
	{															\
		PC -= 2;												\
		CC(ex,0xb2);											\
	}

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR													\
	OUTI;														\
	if( B )														\
	{															\
		PC -= 2;												\
		CC(ex,0xb3);											\
	}

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR													\
	LDD;														\
	if( BC )													\
	{															\
		PC -= 2;												\
		MEMPTR = PC + 1;										\
		CC(ex,0xb8);											\
	}

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR													\
	CPD;														\
	if( BC && !(F & ZF) )										\
	{															\
		PC -= 2;												\
		MEMPTR = PC + 1;										\
		CC(ex,0xb9);											\
	}

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR													\
	IND;														\
	if( B )														\
	{															\
		PC -= 2;												\
		CC(ex,0xba);											\
	}

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR													\
	OUTD;														\
	if( B )														\
	{															\
		PC -= 2;												\
		CC(ex,0xbb);											\
	}

/***************************************************************
 * EI
 ***************************************************************/
#define EI {													\
	IFF1 = IFF2 = 1;											\
	z80->after_ei = TRUE;										\
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { B = RLC(z80, B);										} /* RLC  B           */
OP(cb,01) { C = RLC(z80, C);										} /* RLC  C           */
OP(cb,02) { D = RLC(z80, D);										} /* RLC  D           */
OP(cb,03) { E = RLC(z80, E);										} /* RLC  E           */
OP(cb,04) { H = RLC(z80, H);										} /* RLC  H           */
OP(cb,05) { L = RLC(z80, L);										} /* RLC  L           */
OP(cb,06) { WM( HL, RLC(z80, RM(HL)) );								} /* RLC  (HL)        */
OP(cb,07) { A = RLC(z80, A);										} /* RLC  A           */

OP(cb,08) { B = RRC(z80, B);										} /* RRC  B           */
OP(cb,09) { C = RRC(z80, C);										} /* RRC  C           */
OP(cb,0a) { D = RRC(z80, D);										} /* RRC  D           */
OP(cb,0b) { E = RRC(z80, E);										} /* RRC  E           */
OP(cb,0c) { H = RRC(z80, H);										} /* RRC  H           */
OP(cb,0d) { L = RRC(z80, L);										} /* RRC  L           */
OP(cb,0e) { WM( HL, RRC(z80, RM(HL)) );								} /* RRC  (HL)        */
OP(cb,0f) { A = RRC(z80, A);										} /* RRC  A           */

OP(cb,10) { B = RL(z80, B);											} /* RL   B           */
OP(cb,11) { C = RL(z80, C);											} /* RL   C           */
OP(cb,12) { D = RL(z80, D);											} /* RL   D           */
OP(cb,13) { E = RL(z80, E);											} /* RL   E           */
OP(cb,14) { H = RL(z80, H);											} /* RL   H           */
OP(cb,15) { L = RL(z80, L);											} /* RL   L           */
OP(cb,16) { WM( HL, RL(z80, RM(HL)) );								} /* RL   (HL)        */
OP(cb,17) { A = RL(z80, A);											} /* RL   A           */

OP(cb,18) { B = RR(z80, B);											} /* RR   B           */
OP(cb,19) { C = RR(z80, C);											} /* RR   C           */
OP(cb,1a) { D = RR(z80, D);											} /* RR   D           */
OP(cb,1b) { E = RR(z80, E);											} /* RR   E           */
OP(cb,1c) { H = RR(z80, H);											} /* RR   H           */
OP(cb,1d) { L = RR(z80, L);											} /* RR   L           */
OP(cb,1e) { WM( HL, RR(z80, RM(HL)) );								} /* RR   (HL)        */
OP(cb,1f) { A = RR(z80, A);											} /* RR   A           */

OP(cb,20) { B = SLA(z80, B);										} /* SLA  B           */
OP(cb,21) { C = SLA(z80, C);										} /* SLA  C           */
OP(cb,22) { D = SLA(z80, D);										} /* SLA  D           */
OP(cb,23) { E = SLA(z80, E);										} /* SLA  E           */
OP(cb,24) { H = SLA(z80, H);										} /* SLA  H           */
OP(cb,25) { L = SLA(z80, L);										} /* SLA  L           */
OP(cb,26) { WM( HL, SLA(z80, RM(HL)) );								} /* SLA  (HL)        */
OP(cb,27) { A = SLA(z80, A);										} /* SLA  A           */

OP(cb,28) { B = SRA(z80, B);										} /* SRA  B           */
OP(cb,29) { C = SRA(z80, C);										} /* SRA  C           */
OP(cb,2a) { D = SRA(z80, D);										} /* SRA  D           */
OP(cb,2b) { E = SRA(z80, E);										} /* SRA  E           */
OP(cb,2c) { H = SRA(z80, H);										} /* SRA  H           */
OP(cb,2d) { L = SRA(z80, L);										} /* SRA  L           */
OP(cb,2e) { WM( HL, SRA(z80, RM(HL)) );								} /* SRA  (HL)        */
OP(cb,2f) { A = SRA(z80, A);										} /* SRA  A           */

OP(cb,30) { B = SLL(z80, B);										} /* SLL  B           */
OP(cb,31) { C = SLL(z80, C);										} /* SLL  C           */
OP(cb,32) { D = SLL(z80, D);										} /* SLL  D           */
OP(cb,33) { E = SLL(z80, E);										} /* SLL  E           */
OP(cb,34) { H = SLL(z80, H);										} /* SLL  H           */
OP(cb,35) { L = SLL(z80, L);										} /* SLL  L           */
OP(cb,36) { WM( HL, SLL(z80, RM(HL)) );								} /* SLL  (HL)        */
OP(cb,37) { A = SLL(z80, A);										} /* SLL  A           */

OP(cb,38) { B = SRL(z80, B);										} /* SRL  B           */
OP(cb,39) { C = SRL(z80, C);										} /* SRL  C           */
OP(cb,3a) { D = SRL(z80, D);										} /* SRL  D           */
OP(cb,3b) { E = SRL(z80, E);										} /* SRL  E           */
OP(cb,3c) { H = SRL(z80, H);										} /* SRL  H           */
OP(cb,3d) { L = SRL(z80, L);										} /* SRL  L           */
OP(cb,3e) { WM( HL, SRL(z80, RM(HL)) );								} /* SRL  (HL)        */
OP(cb,3f) { A = SRL(z80, A);										} /* SRL  A           */

OP(cb,40) { BIT(0,B);												} /* BIT  0,B         */
OP(cb,41) { BIT(0,C);												} /* BIT  0,C         */
OP(cb,42) { BIT(0,D);												} /* BIT  0,D         */
OP(cb,43) { BIT(0,E);												} /* BIT  0,E         */
OP(cb,44) { BIT(0,H);												} /* BIT  0,H         */
OP(cb,45) { BIT(0,L);												} /* BIT  0,L         */
OP(cb,46) { BIT_HL(0,RM(HL));										} /* BIT  0,(HL)      */
OP(cb,47) { BIT(0,A);												} /* BIT  0,A         */

OP(cb,48) { BIT(1,B);												} /* BIT  1,B         */
OP(cb,49) { BIT(1,C);												} /* BIT  1,C         */
OP(cb,4a) { BIT(1,D);												} /* BIT  1,D         */
OP(cb,4b) { BIT(1,E);												} /* BIT  1,E         */
OP(cb,4c) { BIT(1,H);												} /* BIT  1,H         */
OP(cb,4d) { BIT(1,L);												} /* BIT  1,L         */
OP(cb,4e) { BIT_HL(1,RM(HL));										} /* BIT  1,(HL)      */
OP(cb,4f) { BIT(1,A);												} /* BIT  1,A         */

OP(cb,50) { BIT(2,B);												} /* BIT  2,B         */
OP(cb,51) { BIT(2,C);												} /* BIT  2,C         */
OP(cb,52) { BIT(2,D);												} /* BIT  2,D         */
OP(cb,53) { BIT(2,E);												} /* BIT  2,E         */
OP(cb,54) { BIT(2,H);												} /* BIT  2,H         */
OP(cb,55) { BIT(2,L);												} /* BIT  2,L         */
OP(cb,56) { BIT_HL(2,RM(HL));										} /* BIT  2,(HL)      */
OP(cb,57) { BIT(2,A);												} /* BIT  2,A         */

OP(cb,58) { BIT(3,B);												} /* BIT  3,B         */
OP(cb,59) { BIT(3,C);												} /* BIT  3,C         */
OP(cb,5a) { BIT(3,D);												} /* BIT  3,D         */
OP(cb,5b) { BIT(3,E);												} /* BIT  3,E         */
OP(cb,5c) { BIT(3,H);												} /* BIT  3,H         */
OP(cb,5d) { BIT(3,L);												} /* BIT  3,L         */
OP(cb,5e) { BIT_HL(3,RM(HL));										} /* BIT  3,(HL)      */
OP(cb,5f) { BIT(3,A);												} /* BIT  3,A         */

OP(cb,60) { BIT(4,B);												} /* BIT  4,B         */
OP(cb,61) { BIT(4,C);												} /* BIT  4,C         */
OP(cb,62) { BIT(4,D);												} /* BIT  4,D         */
OP(cb,63) { BIT(4,E);												} /* BIT  4,E         */
OP(cb,64) { BIT(4,H);												} /* BIT  4,H         */
OP(cb,65) { BIT(4,L);												} /* BIT  4,L         */
OP(cb,66) { BIT_HL(4,RM(HL));										} /* BIT  4,(HL)      */
OP(cb,67) { BIT(4,A);												} /* BIT  4,A         */

OP(cb,68) { BIT(5,B);												} /* BIT  5,B         */
OP(cb,69) { BIT(5,C);												} /* BIT  5,C         */
OP(cb,6a) { BIT(5,D);												} /* BIT  5,D         */
OP(cb,6b) { BIT(5,E);												} /* BIT  5,E         */
OP(cb,6c) { BIT(5,H);												} /* BIT  5,H         */
OP(cb,6d) { BIT(5,L);												} /* BIT  5,L         */
OP(cb,6e) { BIT_HL(5,RM(HL));										} /* BIT  5,(HL)      */
OP(cb,6f) { BIT(5,A);												} /* BIT  5,A         */

OP(cb,70) { BIT(6,B);												} /* BIT  6,B         */
OP(cb,71) { BIT(6,C);												} /* BIT  6,C         */
OP(cb,72) { BIT(6,D);												} /* BIT  6,D         */
OP(cb,73) { BIT(6,E);												} /* BIT  6,E         */
OP(cb,74) { BIT(6,H);												} /* BIT  6,H         */
OP(cb,75) { BIT(6,L);												} /* BIT  6,L         */
OP(cb,76) { BIT_HL(6,RM(HL));										} /* BIT  6,(HL)      */
OP(cb,77) { BIT(6,A);												} /* BIT  6,A         */

OP(cb,78) { BIT(7,B);												} /* BIT  7,B         */
OP(cb,79) { BIT(7,C);												} /* BIT  7,C         */
OP(cb,7a) { BIT(7,D);												} /* BIT  7,D         */
OP(cb,7b) { BIT(7,E);												} /* BIT  7,E         */
OP(cb,7c) { BIT(7,H);												} /* BIT  7,H         */
OP(cb,7d) { BIT(7,L);												} /* BIT  7,L         */
OP(cb,7e) { BIT_HL(7,RM(HL));										} /* BIT  7,(HL)      */
OP(cb,7f) { BIT(7,A);												} /* BIT  7,A         */

OP(cb,80) { B = RES(0,B);											} /* RES  0,B         */
OP(cb,81) { C = RES(0,C);											} /* RES  0,C         */
OP(cb,82) { D = RES(0,D);											} /* RES  0,D         */
OP(cb,83) { E = RES(0,E);											} /* RES  0,E         */
OP(cb,84) { H = RES(0,H);											} /* RES  0,H         */
OP(cb,85) { L = RES(0,L);											} /* RES  0,L         */
OP(cb,86) { WM( HL, RES(0,RM(HL)) );								} /* RES  0,(HL)      */
OP(cb,87) { A = RES(0,A);											} /* RES  0,A         */

OP(cb,88) { B = RES(1,B);											} /* RES  1,B         */
OP(cb,89) { C = RES(1,C);											} /* RES  1,C         */
OP(cb,8a) { D = RES(1,D);											} /* RES  1,D         */
OP(cb,8b) { E = RES(1,E);											} /* RES  1,E         */
OP(cb,8c) { H = RES(1,H);											} /* RES  1,H         */
OP(cb,8d) { L = RES(1,L);											} /* RES  1,L         */
OP(cb,8e) { WM( HL, RES(1,RM(HL)) );								} /* RES  1,(HL)      */
OP(cb,8f) { A = RES(1,A);											} /* RES  1,A         */

OP(cb,90) { B = RES(2,B);											} /* RES  2,B         */
OP(cb,91) { C = RES(2,C);											} /* RES  2,C         */
OP(cb,92) { D = RES(2,D);											} /* RES  2,D         */
OP(cb,93) { E = RES(2,E);											} /* RES  2,E         */
OP(cb,94) { H = RES(2,H);											} /* RES  2,H         */
OP(cb,95) { L = RES(2,L);											} /* RES  2,L         */
OP(cb,96) { WM( HL, RES(2,RM(HL)) );								} /* RES  2,(HL)      */
OP(cb,97) { A = RES(2,A);											} /* RES  2,A         */

OP(cb,98) { B = RES(3,B);											} /* RES  3,B         */
OP(cb,99) { C = RES(3,C);											} /* RES  3,C         */
OP(cb,9a) { D = RES(3,D);											} /* RES  3,D         */
OP(cb,9b) { E = RES(3,E);											} /* RES  3,E         */
OP(cb,9c) { H = RES(3,H);											} /* RES  3,H         */
OP(cb,9d) { L = RES(3,L);											} /* RES  3,L         */
OP(cb,9e) { WM( HL, RES(3,RM(HL)) );								} /* RES  3,(HL)      */
OP(cb,9f) { A = RES(3,A);											} /* RES  3,A         */

OP(cb,a0) { B = RES(4,B);											} /* RES  4,B         */
OP(cb,a1) { C = RES(4,C);											} /* RES  4,C         */
OP(cb,a2) { D = RES(4,D);											} /* RES  4,D         */
OP(cb,a3) { E = RES(4,E);											} /* RES  4,E         */
OP(cb,a4) { H = RES(4,H);											} /* RES  4,H         */
OP(cb,a5) { L = RES(4,L);											} /* RES  4,L         */
OP(cb,a6) { WM( HL, RES(4,RM(HL)) );								} /* RES  4,(HL)      */
OP(cb,a7) { A = RES(4,A);											} /* RES  4,A         */

OP(cb,a8) { B = RES(5,B);											} /* RES  5,B         */
OP(cb,a9) { C = RES(5,C);											} /* RES  5,C         */
OP(cb,aa) { D = RES(5,D);											} /* RES  5,D         */
OP(cb,ab) { E = RES(5,E);											} /* RES  5,E         */
OP(cb,ac) { H = RES(5,H);											} /* RES  5,H         */
OP(cb,ad) { L = RES(5,L);											} /* RES  5,L         */
OP(cb,ae) { WM( HL, RES(5,RM(HL)) );								} /* RES  5,(HL)      */
OP(cb,af) { A = RES(5,A);											} /* RES  5,A         */

OP(cb,b0) { B = RES(6,B);											} /* RES  6,B         */
OP(cb,b1) { C = RES(6,C);											} /* RES  6,C         */
OP(cb,b2) { D = RES(6,D);											} /* RES  6,D         */
OP(cb,b3) { E = RES(6,E);											} /* RES  6,E         */
OP(cb,b4) { H = RES(6,H);											} /* RES  6,H         */
OP(cb,b5) { L = RES(6,L);											} /* RES  6,L         */
OP(cb,b6) { WM( HL, RES(6,RM(HL)) );								} /* RES  6,(HL)      */
OP(cb,b7) { A = RES(6,A);											} /* RES  6,A         */

OP(cb,b8) { B = RES(7,B);											} /* RES  7,B         */
OP(cb,b9) { C = RES(7,C);											} /* RES  7,C         */
OP(cb,ba) { D = RES(7,D);											} /* RES  7,D         */
OP(cb,bb) { E = RES(7,E);											} /* RES  7,E         */
OP(cb,bc) { H = RES(7,H);											} /* RES  7,H         */
OP(cb,bd) { L = RES(7,L);											} /* RES  7,L         */
OP(cb,be) { WM( HL, RES(7,RM(HL)) );								} /* RES  7,(HL)      */
OP(cb,bf) { A = RES(7,A);											} /* RES  7,A         */

OP(cb,c0) { B = SET(0,B);											} /* SET  0,B         */
OP(cb,c1) { C = SET(0,C);											} /* SET  0,C         */
OP(cb,c2) { D = SET(0,D);											} /* SET  0,D         */
OP(cb,c3) { E = SET(0,E);											} /* SET  0,E         */
OP(cb,c4) { H = SET(0,H);											} /* SET  0,H         */
OP(cb,c5) { L = SET(0,L);											} /* SET  0,L         */
OP(cb,c6) { WM( HL, SET(0,RM(HL)) );								} /* SET  0,(HL)      */
OP(cb,c7) { A = SET(0,A);											} /* SET  0,A         */

OP(cb,c8) { B = SET(1,B);											} /* SET  1,B         */
OP(cb,c9) { C = SET(1,C);											} /* SET  1,C         */
OP(cb,ca) { D = SET(1,D);											} /* SET  1,D         */
OP(cb,cb) { E = SET(1,E);											} /* SET  1,E         */
OP(cb,cc) { H = SET(1,H);											} /* SET  1,H         */
OP(cb,cd) { L = SET(1,L);											} /* SET  1,L         */
OP(cb,ce) { WM( HL, SET(1,RM(HL)) );								} /* SET  1,(HL)      */
OP(cb,cf) { A = SET(1,A);											} /* SET  1,A         */

OP(cb,d0) { B = SET(2,B);											} /* SET  2,B         */
OP(cb,d1) { C = SET(2,C);											} /* SET  2,C         */
OP(cb,d2) { D = SET(2,D);											} /* SET  2,D         */
OP(cb,d3) { E = SET(2,E);											} /* SET  2,E         */
OP(cb,d4) { H = SET(2,H);											} /* SET  2,H         */
OP(cb,d5) { L = SET(2,L);											} /* SET  2,L         */
OP(cb,d6) { WM( HL, SET(2,RM(HL)) );								} /* SET  2,(HL)      */
OP(cb,d7) { A = SET(2,A);											} /* SET  2,A         */

OP(cb,d8) { B = SET(3,B);											} /* SET  3,B         */
OP(cb,d9) { C = SET(3,C);											} /* SET  3,C         */
OP(cb,da) { D = SET(3,D);											} /* SET  3,D         */
OP(cb,db) { E = SET(3,E);											} /* SET  3,E         */
OP(cb,dc) { H = SET(3,H);											} /* SET  3,H         */
OP(cb,dd) { L = SET(3,L);											} /* SET  3,L         */
OP(cb,de) { WM( HL, SET(3,RM(HL)) );								} /* SET  3,(HL)      */
OP(cb,df) { A = SET(3,A);											} /* SET  3,A         */

OP(cb,e0) { B = SET(4,B);											} /* SET  4,B         */
OP(cb,e1) { C = SET(4,C);											} /* SET  4,C         */
OP(cb,e2) { D = SET(4,D);											} /* SET  4,D         */
OP(cb,e3) { E = SET(4,E);											} /* SET  4,E         */
OP(cb,e4) { H = SET(4,H);											} /* SET  4,H         */
OP(cb,e5) { L = SET(4,L);											} /* SET  4,L         */
OP(cb,e6) { WM( HL, SET(4,RM(HL)) );								} /* SET  4,(HL)      */
OP(cb,e7) { A = SET(4,A);											} /* SET  4,A         */

OP(cb,e8) { B = SET(5,B);											} /* SET  5,B         */
OP(cb,e9) { C = SET(5,C);											} /* SET  5,C         */
OP(cb,ea) { D = SET(5,D);											} /* SET  5,D         */
OP(cb,eb) { E = SET(5,E);											} /* SET  5,E         */
OP(cb,ec) { H = SET(5,H);											} /* SET  5,H         */
OP(cb,ed) { L = SET(5,L);											} /* SET  5,L         */
OP(cb,ee) { WM( HL, SET(5,RM(HL)) );								} /* SET  5,(HL)      */
OP(cb,ef) { A = SET(5,A);											} /* SET  5,A         */

OP(cb,f0) { B = SET(6,B);											} /* SET  6,B         */
OP(cb,f1) { C = SET(6,C);											} /* SET  6,C         */
OP(cb,f2) { D = SET(6,D);											} /* SET  6,D         */
OP(cb,f3) { E = SET(6,E);											} /* SET  6,E         */
OP(cb,f4) { H = SET(6,H);											} /* SET  6,H         */
OP(cb,f5) { L = SET(6,L);											} /* SET  6,L         */
OP(cb,f6) { WM( HL, SET(6,RM(HL)) );								} /* SET  6,(HL)      */
OP(cb,f7) { A = SET(6,A);											} /* SET  6,A         */

OP(cb,f8) { B = SET(7,B);											} /* SET  7,B         */
OP(cb,f9) { C = SET(7,C);											} /* SET  7,C         */
OP(cb,fa) { D = SET(7,D);											} /* SET  7,D         */
OP(cb,fb) { E = SET(7,E);											} /* SET  7,E         */
OP(cb,fc) { H = SET(7,H);											} /* SET  7,H         */
OP(cb,fd) { L = SET(7,L);											} /* SET  7,L         */
OP(cb,fe) { WM( HL, SET(7,RM(HL)) );								} /* SET  7,(HL)      */
OP(cb,ff) { A = SET(7,A);											} /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { B = RLC(z80,  RM(EA) ); WM( EA,B );					} /* RLC  B=(XY+o)    */
OP(xycb,01) { C = RLC(z80,  RM(EA) ); WM( EA,C );					} /* RLC  C=(XY+o)    */
OP(xycb,02) { D = RLC(z80,  RM(EA) ); WM( EA,D );					} /* RLC  D=(XY+o)    */
OP(xycb,03) { E = RLC(z80,  RM(EA) ); WM( EA,E );					} /* RLC  E=(XY+o)    */
OP(xycb,04) { H = RLC(z80,  RM(EA) ); WM( EA,H );					} /* RLC  H=(XY+o)    */
OP(xycb,05) { L = RLC(z80,  RM(EA) ); WM( EA,L );					} /* RLC  L=(XY+o)    */
OP(xycb,06) { WM( EA, RLC(z80,  RM(EA) ) );							} /* RLC  (XY+o)      */
OP(xycb,07) { A = RLC(z80,  RM(EA) ); WM( EA,A );					} /* RLC  A=(XY+o)    */

OP(xycb,08) { B = RRC(z80,  RM(EA) ); WM( EA,B );					} /* RRC  B=(XY+o)    */
OP(xycb,09) { C = RRC(z80,  RM(EA) ); WM( EA,C );					} /* RRC  C=(XY+o)    */
OP(xycb,0a) { D = RRC(z80,  RM(EA) ); WM( EA,D );					} /* RRC  D=(XY+o)    */
OP(xycb,0b) { E = RRC(z80,  RM(EA) ); WM( EA,E );					} /* RRC  E=(XY+o)    */
OP(xycb,0c) { H = RRC(z80,  RM(EA) ); WM( EA,H );					} /* RRC  H=(XY+o)    */
OP(xycb,0d) { L = RRC(z80,  RM(EA) ); WM( EA,L );					} /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM( EA,RRC(z80,  RM(EA) ) );							} /* RRC  (XY+o)      */
OP(xycb,0f) { A = RRC(z80,  RM(EA) ); WM( EA,A );					} /* RRC  A=(XY+o)    */

OP(xycb,10) { B = RL(z80,  RM(EA) ); WM( EA,B );					} /* RL   B=(XY+o)    */
OP(xycb,11) { C = RL(z80,  RM(EA) ); WM( EA,C );					} /* RL   C=(XY+o)    */
OP(xycb,12) { D = RL(z80,  RM(EA) ); WM( EA,D );					} /* RL   D=(XY+o)    */
OP(xycb,13) { E = RL(z80,  RM(EA) ); WM( EA,E );					} /* RL   E=(XY+o)    */
OP(xycb,14) { H = RL(z80,  RM(EA) ); WM( EA,H );					} /* RL   H=(XY+o)    */
OP(xycb,15) { L = RL(z80,  RM(EA) ); WM( EA,L );					} /* RL   L=(XY+o)    */
OP(xycb,16) { WM( EA,RL(z80,  RM(EA) ) );							} /* RL   (XY+o)      */
OP(xycb,17) { A = RL(z80,  RM(EA) ); WM( EA,A );					} /* RL   A=(XY+o)    */

OP(xycb,18) { B = RR(z80,  RM(EA) ); WM( EA,B );					} /* RR   B=(XY+o)    */
OP(xycb,19) { C = RR(z80,  RM(EA) ); WM( EA,C );					} /* RR   C=(XY+o)    */
OP(xycb,1a) { D = RR(z80,  RM(EA) ); WM( EA,D );					} /* RR   D=(XY+o)    */
OP(xycb,1b) { E = RR(z80,  RM(EA) ); WM( EA,E );					} /* RR   E=(XY+o)    */
OP(xycb,1c) { H = RR(z80,  RM(EA) ); WM( EA,H );					} /* RR   H=(XY+o)    */
OP(xycb,1d) { L = RR(z80,  RM(EA) ); WM( EA,L );					} /* RR   L=(XY+o)    */
OP(xycb,1e) { WM( EA,RR(z80,  RM(EA) ) );							} /* RR   (XY+o)      */
OP(xycb,1f) { A = RR(z80,  RM(EA) ); WM( EA,A );					} /* RR   A=(XY+o)    */

OP(xycb,20) { B = SLA(z80,  RM(EA) ); WM( EA,B );					} /* SLA  B=(XY+o)    */
OP(xycb,21) { C = SLA(z80,  RM(EA) ); WM( EA,C );					} /* SLA  C=(XY+o)    */
OP(xycb,22) { D = SLA(z80,  RM(EA) ); WM( EA,D );					} /* SLA  D=(XY+o)    */
OP(xycb,23) { E = SLA(z80,  RM(EA) ); WM( EA,E );					} /* SLA  E=(XY+o)    */
OP(xycb,24) { H = SLA(z80,  RM(EA) ); WM( EA,H );					} /* SLA  H=(XY+o)    */
OP(xycb,25) { L = SLA(z80,  RM(EA) ); WM( EA,L );					} /* SLA  L=(XY+o)    */
OP(xycb,26) { WM( EA,SLA(z80,  RM(EA) ) );							} /* SLA  (XY+o)      */
OP(xycb,27) { A = SLA(z80,  RM(EA) ); WM( EA,A );					} /* SLA  A=(XY+o)    */

OP(xycb,28) { B = SRA(z80,  RM(EA) ); WM( EA,B );					} /* SRA  B=(XY+o)    */
OP(xycb,29) { C = SRA(z80,  RM(EA) ); WM( EA,C );					} /* SRA  C=(XY+o)    */
OP(xycb,2a) { D = SRA(z80,  RM(EA) ); WM( EA,D );					} /* SRA  D=(XY+o)    */
OP(xycb,2b) { E = SRA(z80,  RM(EA) ); WM( EA,E );					} /* SRA  E=(XY+o)    */
OP(xycb,2c) { H = SRA(z80,  RM(EA) ); WM( EA,H );					} /* SRA  H=(XY+o)    */
OP(xycb,2d) { L = SRA(z80,  RM(EA) ); WM( EA,L );					} /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM( EA,SRA(z80,  RM(EA) ) );							} /* SRA  (XY+o)      */
OP(xycb,2f) { A = SRA(z80,  RM(EA) ); WM( EA,A );					} /* SRA  A=(XY+o)    */

OP(xycb,30) { B = SLL(z80,  RM(EA) ); WM( EA,B );					} /* SLL  B=(XY+o)    */
OP(xycb,31) { C = SLL(z80,  RM(EA) ); WM( EA,C );					} /* SLL  C=(XY+o)    */
OP(xycb,32) { D = SLL(z80,  RM(EA) ); WM( EA,D );					} /* SLL  D=(XY+o)    */
OP(xycb,33) { E = SLL(z80,  RM(EA) ); WM( EA,E );					} /* SLL  E=(XY+o)    */
OP(xycb,34) { H = SLL(z80,  RM(EA) ); WM( EA,H );					} /* SLL  H=(XY+o)    */
OP(xycb,35) { L = SLL(z80,  RM(EA) ); WM( EA,L );					} /* SLL  L=(XY+o)    */
OP(xycb,36) { WM( EA,SLL(z80,  RM(EA) ) );							} /* SLL  (XY+o)      */
OP(xycb,37) { A = SLL(z80,  RM(EA) ); WM( EA,A );					} /* SLL  A=(XY+o)    */

OP(xycb,38) { B = SRL(z80,  RM(EA) ); WM( EA,B );					} /* SRL  B=(XY+o)    */
OP(xycb,39) { C = SRL(z80,  RM(EA) ); WM( EA,C );					} /* SRL  C=(XY+o)    */
OP(xycb,3a) { D = SRL(z80,  RM(EA) ); WM( EA,D );					} /* SRL  D=(XY+o)    */
OP(xycb,3b) { E = SRL(z80,  RM(EA) ); WM( EA,E );					} /* SRL  E=(XY+o)    */
OP(xycb,3c) { H = SRL(z80,  RM(EA) ); WM( EA,H );					} /* SRL  H=(XY+o)    */
OP(xycb,3d) { L = SRL(z80,  RM(EA) ); WM( EA,L );					} /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM( EA,SRL(z80,  RM(EA) ) );							} /* SRL  (XY+o)      */
OP(xycb,3f) { A = SRL(z80,  RM(EA) ); WM( EA,A );					} /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,41) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,42) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,43) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,44) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,45) { xycb_46(z80);											} /* BIT  0,(XY+o)    */
OP(xycb,46) { BIT_XY(0,RM(EA));										} /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46(z80);											} /* BIT  0,(XY+o)    */

OP(xycb,48) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,49) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,4a) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,4b) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,4c) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,4d) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */
OP(xycb,4e) { BIT_XY(1,RM(EA));										} /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e(z80);											} /* BIT  1,(XY+o)    */

OP(xycb,50) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,51) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,52) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,53) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,54) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,55) { xycb_56(z80);											} /* BIT  2,(XY+o)    */
OP(xycb,56) { BIT_XY(2,RM(EA));										} /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56(z80);											} /* BIT  2,(XY+o)    */

OP(xycb,58) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,59) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,5a) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,5b) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,5c) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,5d) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */
OP(xycb,5e) { BIT_XY(3,RM(EA));										} /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e(z80);											} /* BIT  3,(XY+o)    */

OP(xycb,60) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,61) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,62) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,63) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,64) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,65) { xycb_66(z80);											} /* BIT  4,(XY+o)    */
OP(xycb,66) { BIT_XY(4,RM(EA));										} /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66(z80);											} /* BIT  4,(XY+o)    */

OP(xycb,68) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,69) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,6a) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,6b) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,6c) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,6d) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */
OP(xycb,6e) { BIT_XY(5,RM(EA));										} /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e(z80);											} /* BIT  5,(XY+o)    */

OP(xycb,70) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,71) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,72) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,73) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,74) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,75) { xycb_76(z80);											} /* BIT  6,(XY+o)    */
OP(xycb,76) { BIT_XY(6,RM(EA));										} /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76(z80);											} /* BIT  6,(XY+o)    */

OP(xycb,78) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,79) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,7a) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,7b) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,7c) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,7d) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */
OP(xycb,7e) { BIT_XY(7,RM(EA));										} /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e(z80);											} /* BIT  7,(XY+o)    */

OP(xycb,80) { B = RES(0, RM(EA) ); WM( EA,B );						} /* RES  0,B=(XY+o)  */
OP(xycb,81) { C = RES(0, RM(EA) ); WM( EA,C );						} /* RES  0,C=(XY+o)  */
OP(xycb,82) { D = RES(0, RM(EA) ); WM( EA,D );						} /* RES  0,D=(XY+o)  */
OP(xycb,83) { E = RES(0, RM(EA) ); WM( EA,E );						} /* RES  0,E=(XY+o)  */
OP(xycb,84) { H = RES(0, RM(EA) ); WM( EA,H );						} /* RES  0,H=(XY+o)  */
OP(xycb,85) { L = RES(0, RM(EA) ); WM( EA,L );						} /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM( EA, RES(0,RM(EA)) );								} /* RES  0,(XY+o)    */
OP(xycb,87) { A = RES(0, RM(EA) ); WM( EA,A );						} /* RES  0,A=(XY+o)  */

OP(xycb,88) { B = RES(1, RM(EA) ); WM( EA,B );						} /* RES  1,B=(XY+o)  */
OP(xycb,89) { C = RES(1, RM(EA) ); WM( EA,C );						} /* RES  1,C=(XY+o)  */
OP(xycb,8a) { D = RES(1, RM(EA) ); WM( EA,D );						} /* RES  1,D=(XY+o)  */
OP(xycb,8b) { E = RES(1, RM(EA) ); WM( EA,E );						} /* RES  1,E=(XY+o)  */
OP(xycb,8c) { H = RES(1, RM(EA) ); WM( EA,H );						} /* RES  1,H=(XY+o)  */
OP(xycb,8d) { L = RES(1, RM(EA) ); WM( EA,L );						} /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM( EA, RES(1,RM(EA)) );								} /* RES  1,(XY+o)    */
OP(xycb,8f) { A = RES(1, RM(EA) ); WM( EA,A );						} /* RES  1,A=(XY+o)  */

OP(xycb,90) { B = RES(2, RM(EA) ); WM( EA,B );						} /* RES  2,B=(XY+o)  */
OP(xycb,91) { C = RES(2, RM(EA) ); WM( EA,C );						} /* RES  2,C=(XY+o)  */
OP(xycb,92) { D = RES(2, RM(EA) ); WM( EA,D );						} /* RES  2,D=(XY+o)  */
OP(xycb,93) { E = RES(2, RM(EA) ); WM( EA,E );						} /* RES  2,E=(XY+o)  */
OP(xycb,94) { H = RES(2, RM(EA) ); WM( EA,H );						} /* RES  2,H=(XY+o)  */
OP(xycb,95) { L = RES(2, RM(EA) ); WM( EA,L );						} /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM( EA, RES(2,RM(EA)) );								} /* RES  2,(XY+o)    */
OP(xycb,97) { A = RES(2, RM(EA) ); WM( EA,A );						} /* RES  2,A=(XY+o)  */

OP(xycb,98) { B = RES(3, RM(EA) ); WM( EA,B );						} /* RES  3,B=(XY+o)  */
OP(xycb,99) { C = RES(3, RM(EA) ); WM( EA,C );						} /* RES  3,C=(XY+o)  */
OP(xycb,9a) { D = RES(3, RM(EA) ); WM( EA,D );						} /* RES  3,D=(XY+o)  */
OP(xycb,9b) { E = RES(3, RM(EA) ); WM( EA,E );						} /* RES  3,E=(XY+o)  */
OP(xycb,9c) { H = RES(3, RM(EA) ); WM( EA,H );						} /* RES  3,H=(XY+o)  */
OP(xycb,9d) { L = RES(3, RM(EA) ); WM( EA,L );						} /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM( EA, RES(3,RM(EA)) );								} /* RES  3,(XY+o)    */
OP(xycb,9f) { A = RES(3, RM(EA) ); WM( EA,A );						} /* RES  3,A=(XY+o)  */

OP(xycb,a0) { B = RES(4, RM(EA) ); WM( EA,B );						} /* RES  4,B=(XY+o)  */
OP(xycb,a1) { C = RES(4, RM(EA) ); WM( EA,C );						} /* RES  4,C=(XY+o)  */
OP(xycb,a2) { D = RES(4, RM(EA) ); WM( EA,D );						} /* RES  4,D=(XY+o)  */
OP(xycb,a3) { E = RES(4, RM(EA) ); WM( EA,E );						} /* RES  4,E=(XY+o)  */
OP(xycb,a4) { H = RES(4, RM(EA) ); WM( EA,H );						} /* RES  4,H=(XY+o)  */
OP(xycb,a5) { L = RES(4, RM(EA) ); WM( EA,L );						} /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM( EA, RES(4,RM(EA)) );								} /* RES  4,(XY+o)    */
OP(xycb,a7) { A = RES(4, RM(EA) ); WM( EA,A );						} /* RES  4,A=(XY+o)  */

OP(xycb,a8) { B = RES(5, RM(EA) ); WM( EA,B );						} /* RES  5,B=(XY+o)  */
OP(xycb,a9) { C = RES(5, RM(EA) ); WM( EA,C );						} /* RES  5,C=(XY+o)  */
OP(xycb,aa) { D = RES(5, RM(EA) ); WM( EA,D );						} /* RES  5,D=(XY+o)  */
OP(xycb,ab) { E = RES(5, RM(EA) ); WM( EA,E );						} /* RES  5,E=(XY+o)  */
OP(xycb,ac) { H = RES(5, RM(EA) ); WM( EA,H );						} /* RES  5,H=(XY+o)  */
OP(xycb,ad) { L = RES(5, RM(EA) ); WM( EA,L );						} /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM( EA, RES(5,RM(EA)) );								} /* RES  5,(XY+o)    */
OP(xycb,af) { A = RES(5, RM(EA) ); WM( EA,A );						} /* RES  5,A=(XY+o)  */

OP(xycb,b0) { B = RES(6, RM(EA) ); WM( EA,B );						} /* RES  6,B=(XY+o)  */
OP(xycb,b1) { C = RES(6, RM(EA) ); WM( EA,C );						} /* RES  6,C=(XY+o)  */
OP(xycb,b2) { D = RES(6, RM(EA) ); WM( EA,D );						} /* RES  6,D=(XY+o)  */
OP(xycb,b3) { E = RES(6, RM(EA) ); WM( EA,E );						} /* RES  6,E=(XY+o)  */
OP(xycb,b4) { H = RES(6, RM(EA) ); WM( EA,H );						} /* RES  6,H=(XY+o)  */
OP(xycb,b5) { L = RES(6, RM(EA) ); WM( EA,L );						} /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM( EA, RES(6,RM(EA)) );								} /* RES  6,(XY+o)    */
OP(xycb,b7) { A = RES(6, RM(EA) ); WM( EA,A );						} /* RES  6,A=(XY+o)  */

OP(xycb,b8) { B = RES(7, RM(EA) ); WM( EA,B );						} /* RES  7,B=(XY+o)  */
OP(xycb,b9) { C = RES(7, RM(EA) ); WM( EA,C );						} /* RES  7,C=(XY+o)  */
OP(xycb,ba) { D = RES(7, RM(EA) ); WM( EA,D );						} /* RES  7,D=(XY+o)  */
OP(xycb,bb) { E = RES(7, RM(EA) ); WM( EA,E );						} /* RES  7,E=(XY+o)  */
OP(xycb,bc) { H = RES(7, RM(EA) ); WM( EA,H );						} /* RES  7,H=(XY+o)  */
OP(xycb,bd) { L = RES(7, RM(EA) ); WM( EA,L );						} /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM( EA, RES(7,RM(EA)) );								} /* RES  7,(XY+o)    */
OP(xycb,bf) { A = RES(7, RM(EA) ); WM( EA,A );						} /* RES  7,A=(XY+o)  */

OP(xycb,c0) { B = SET(0, RM(EA) ); WM( EA,B );						} /* SET  0,B=(XY+o)  */
OP(xycb,c1) { C = SET(0, RM(EA) ); WM( EA,C );						} /* SET  0,C=(XY+o)  */
OP(xycb,c2) { D = SET(0, RM(EA) ); WM( EA,D );						} /* SET  0,D=(XY+o)  */
OP(xycb,c3) { E = SET(0, RM(EA) ); WM( EA,E );						} /* SET  0,E=(XY+o)  */
OP(xycb,c4) { H = SET(0, RM(EA) ); WM( EA,H );						} /* SET  0,H=(XY+o)  */
OP(xycb,c5) { L = SET(0, RM(EA) ); WM( EA,L );						} /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM( EA, SET(0,RM(EA)) );								} /* SET  0,(XY+o)    */
OP(xycb,c7) { A = SET(0, RM(EA) ); WM( EA,A );						} /* SET  0,A=(XY+o)  */

OP(xycb,c8) { B = SET(1, RM(EA) ); WM( EA,B );						} /* SET  1,B=(XY+o)  */
OP(xycb,c9) { C = SET(1, RM(EA) ); WM( EA,C );						} /* SET  1,C=(XY+o)  */
OP(xycb,ca) { D = SET(1, RM(EA) ); WM( EA,D );						} /* SET  1,D=(XY+o)  */
OP(xycb,cb) { E = SET(1, RM(EA) ); WM( EA,E );						} /* SET  1,E=(XY+o)  */
OP(xycb,cc) { H = SET(1, RM(EA) ); WM( EA,H );						} /* SET  1,H=(XY+o)  */
OP(xycb,cd) { L = SET(1, RM(EA) ); WM( EA,L );						} /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM( EA, SET(1,RM(EA)) );								} /* SET  1,(XY+o)    */
OP(xycb,cf) { A = SET(1, RM(EA) ); WM( EA,A );						} /* SET  1,A=(XY+o)  */

OP(xycb,d0) { B = SET(2, RM(EA) ); WM( EA,B );						} /* SET  2,B=(XY+o)  */
OP(xycb,d1) { C = SET(2, RM(EA) ); WM( EA,C );						} /* SET  2,C=(XY+o)  */
OP(xycb,d2) { D = SET(2, RM(EA) ); WM( EA,D );						} /* SET  2,D=(XY+o)  */
OP(xycb,d3) { E = SET(2, RM(EA) ); WM( EA,E );						} /* SET  2,E=(XY+o)  */
OP(xycb,d4) { H = SET(2, RM(EA) ); WM( EA,H );						} /* SET  2,H=(XY+o)  */
OP(xycb,d5) { L = SET(2, RM(EA) ); WM( EA,L );						} /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM( EA, SET(2,RM(EA)) );								} /* SET  2,(XY+o)    */
OP(xycb,d7) { A = SET(2, RM(EA) ); WM( EA,A );						} /* SET  2,A=(XY+o)  */

OP(xycb,d8) { B = SET(3, RM(EA) ); WM( EA,B );						} /* SET  3,B=(XY+o)  */
OP(xycb,d9) { C = SET(3, RM(EA) ); WM( EA,C );						} /* SET  3,C=(XY+o)  */
OP(xycb,da) { D = SET(3, RM(EA) ); WM( EA,D );						} /* SET  3,D=(XY+o)  */
OP(xycb,db) { E = SET(3, RM(EA) ); WM( EA,E );						} /* SET  3,E=(XY+o)  */
OP(xycb,dc) { H = SET(3, RM(EA) ); WM( EA,H );						} /* SET  3,H=(XY+o)  */
OP(xycb,dd) { L = SET(3, RM(EA) ); WM( EA,L );						} /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM( EA, SET(3,RM(EA)) );								} /* SET  3,(XY+o)    */
OP(xycb,df) { A = SET(3, RM(EA) ); WM( EA,A );						} /* SET  3,A=(XY+o)  */

OP(xycb,e0) { B = SET(4, RM(EA) ); WM( EA,B );						} /* SET  4,B=(XY+o)  */
OP(xycb,e1) { C = SET(4, RM(EA) ); WM( EA,C );						} /* SET  4,C=(XY+o)  */
OP(xycb,e2) { D = SET(4, RM(EA) ); WM( EA,D );						} /* SET  4,D=(XY+o)  */
OP(xycb,e3) { E = SET(4, RM(EA) ); WM( EA,E );						} /* SET  4,E=(XY+o)  */
OP(xycb,e4) { H = SET(4, RM(EA) ); WM( EA,H );						} /* SET  4,H=(XY+o)  */
OP(xycb,e5) { L = SET(4, RM(EA) ); WM( EA,L );						} /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM( EA, SET(4,RM(EA)) );								} /* SET  4,(XY+o)    */
OP(xycb,e7) { A = SET(4, RM(EA) ); WM( EA,A );						} /* SET  4,A=(XY+o)  */

OP(xycb,e8) { B = SET(5, RM(EA) ); WM( EA,B );						} /* SET  5,B=(XY+o)  */
OP(xycb,e9) { C = SET(5, RM(EA) ); WM( EA,C );						} /* SET  5,C=(XY+o)  */
OP(xycb,ea) { D = SET(5, RM(EA) ); WM( EA,D );						} /* SET  5,D=(XY+o)  */
OP(xycb,eb) { E = SET(5, RM(EA) ); WM( EA,E );						} /* SET  5,E=(XY+o)  */
OP(xycb,ec) { H = SET(5, RM(EA) ); WM( EA,H );						} /* SET  5,H=(XY+o)  */
OP(xycb,ed) { L = SET(5, RM(EA) ); WM( EA,L );						} /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM( EA, SET(5,RM(EA)) );								} /* SET  5,(XY+o)    */
OP(xycb,ef) { A = SET(5, RM(EA) ); WM( EA,A );						} /* SET  5,A=(XY+o)  */

OP(xycb,f0) { B = SET(6, RM(EA) ); WM( EA,B );						} /* SET  6,B=(XY+o)  */
OP(xycb,f1) { C = SET(6, RM(EA) ); WM( EA,C );						} /* SET  6,C=(XY+o)  */
OP(xycb,f2) { D = SET(6, RM(EA) ); WM( EA,D );						} /* SET  6,D=(XY+o)  */
OP(xycb,f3) { E = SET(6, RM(EA) ); WM( EA,E );						} /* SET  6,E=(XY+o)  */
OP(xycb,f4) { H = SET(6, RM(EA) ); WM( EA,H );						} /* SET  6,H=(XY+o)  */
OP(xycb,f5) { L = SET(6, RM(EA) ); WM( EA,L );						} /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM( EA, SET(6,RM(EA)) );								} /* SET  6,(XY+o)    */
OP(xycb,f7) { A = SET(6, RM(EA) ); WM( EA,A );						} /* SET  6,A=(XY+o)  */

OP(xycb,f8) { B = SET(7, RM(EA) ); WM( EA,B );						} /* SET  7,B=(XY+o)  */
OP(xycb,f9) { C = SET(7, RM(EA) ); WM( EA,C );						} /* SET  7,C=(XY+o)  */
OP(xycb,fa) { D = SET(7, RM(EA) ); WM( EA,D );						} /* SET  7,D=(XY+o)  */
OP(xycb,fb) { E = SET(7, RM(EA) ); WM( EA,E );						} /* SET  7,E=(XY+o)  */
OP(xycb,fc) { H = SET(7, RM(EA) ); WM( EA,H );						} /* SET  7,H=(XY+o)  */
OP(xycb,fd) { L = SET(7, RM(EA) ); WM( EA,L );						} /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM( EA, SET(7,RM(EA)) );								} /* SET  7,(XY+o)    */
OP(xycb,ff) { A = SET(7, RM(EA) ); WM( EA,A );						} /* SET  7,A=(XY+o)  */

OP(illegal,1) {
	logerror("Z80 #%d ill. opcode $%02x $%02x\n",
			cpu_getactivecpu(), cpu_readop((PCD-1)&0xffff), cpu_readop(PCD));
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP(dd,00) { illegal_1(z80); op_00(z80);								} /* DB   DD          */
OP(dd,01) { illegal_1(z80); op_01(z80);								} /* DB   DD          */
OP(dd,02) { illegal_1(z80); op_02(z80);								} /* DB   DD          */
OP(dd,03) { illegal_1(z80); op_03(z80);								} /* DB   DD          */
OP(dd,04) { illegal_1(z80); op_04(z80);								} /* DB   DD          */
OP(dd,05) { illegal_1(z80); op_05(z80);								} /* DB   DD          */
OP(dd,06) { illegal_1(z80); op_06(z80);								} /* DB   DD          */
OP(dd,07) { illegal_1(z80); op_07(z80);								} /* DB   DD          */

OP(dd,08) { illegal_1(z80); op_08(z80);								} /* DB   DD          */
OP(dd,09) { ADD16(ix,bc);											} /* ADD  IX,BC       */
OP(dd,0a) { illegal_1(z80); op_0a(z80);								} /* DB   DD          */
OP(dd,0b) { illegal_1(z80); op_0b(z80);								} /* DB   DD          */
OP(dd,0c) { illegal_1(z80); op_0c(z80);								} /* DB   DD          */
OP(dd,0d) { illegal_1(z80); op_0d(z80);								} /* DB   DD          */
OP(dd,0e) { illegal_1(z80); op_0e(z80);								} /* DB   DD          */
OP(dd,0f) { illegal_1(z80); op_0f(z80);								} /* DB   DD          */

OP(dd,10) { illegal_1(z80); op_10(z80);								} /* DB   DD          */
OP(dd,11) { illegal_1(z80); op_11(z80);								} /* DB   DD          */
OP(dd,12) { illegal_1(z80); op_12(z80);								} /* DB   DD          */
OP(dd,13) { illegal_1(z80); op_13(z80);								} /* DB   DD          */
OP(dd,14) { illegal_1(z80); op_14(z80);								} /* DB   DD          */
OP(dd,15) { illegal_1(z80); op_15(z80);								} /* DB   DD          */
OP(dd,16) { illegal_1(z80); op_16(z80);								} /* DB   DD          */
OP(dd,17) { illegal_1(z80); op_17(z80);								} /* DB   DD          */

OP(dd,18) { illegal_1(z80); op_18(z80);								} /* DB   DD          */
OP(dd,19) { ADD16(ix,de);											} /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(z80); op_1a(z80);								} /* DB   DD          */
OP(dd,1b) { illegal_1(z80); op_1b(z80);								} /* DB   DD          */
OP(dd,1c) { illegal_1(z80); op_1c(z80);								} /* DB   DD          */
OP(dd,1d) { illegal_1(z80); op_1d(z80);								} /* DB   DD          */
OP(dd,1e) { illegal_1(z80); op_1e(z80);								} /* DB   DD          */
OP(dd,1f) { illegal_1(z80); op_1f(z80);								} /* DB   DD          */

OP(dd,20) { illegal_1(z80); op_20(z80);								} /* DB   DD          */
OP(dd,21) { IX = ARG16(z80);										} /* LD   IX,w        */
OP(dd,22) { EA = ARG16(z80); WM16( EA, &z80->ix );	MEMPTR = EA+1;	} /* LD   (w),IX      */
OP(dd,23) { IX++;													} /* INC  IX          */
OP(dd,24) { HX = INC(z80, HX);										} /* INC  HX          */
OP(dd,25) { HX = DEC(z80, HX);										} /* DEC  HX          */
OP(dd,26) { HX = ARG(z80);											} /* LD   HX,n        */
OP(dd,27) { illegal_1(z80); op_27(z80);								} /* DB   DD          */

OP(dd,28) { illegal_1(z80); op_28(z80);								} /* DB   DD          */
OP(dd,29) { ADD16(ix,ix);											} /* ADD  IX,IX       */
OP(dd,2a) { EA = ARG16(z80); RM16( EA, &z80->ix );	MEMPTR = EA+1;	} /* LD   IX,(w)      */
OP(dd,2b) { IX--;													} /* DEC  IX          */
OP(dd,2c) { LX = INC(z80, LX);										} /* INC  LX          */
OP(dd,2d) { LX = DEC(z80, LX);										} /* DEC  LX          */
OP(dd,2e) { LX = ARG(z80);											} /* LD   LX,n        */
OP(dd,2f) { illegal_1(z80); op_2f(z80);								} /* DB   DD          */

OP(dd,30) { illegal_1(z80); op_30(z80);								} /* DB   DD          */
OP(dd,31) { illegal_1(z80); op_31(z80);								} /* DB   DD          */
OP(dd,32) { illegal_1(z80); op_32(z80);								} /* DB   DD          */
OP(dd,33) { illegal_1(z80); op_33(z80);								} /* DB   DD          */
OP(dd,34) { EAX; WM( EA, INC(z80, RM(EA)) );						} /* INC  (IX+o)      */
OP(dd,35) { EAX; WM( EA, DEC(z80, RM(EA)) );						} /* DEC  (IX+o)      */
OP(dd,36) { EAX; WM( EA, ARG(z80) );								} /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(z80); op_37(z80);								} /* DB   DD          */

OP(dd,38) { illegal_1(z80); op_38(z80);								} /* DB   DD          */
OP(dd,39) { ADD16(ix,sp);											} /* ADD  IX,SP       */
OP(dd,3a) { illegal_1(z80); op_3a(z80);								} /* DB   DD          */
OP(dd,3b) { illegal_1(z80); op_3b(z80);								} /* DB   DD          */
OP(dd,3c) { illegal_1(z80); op_3c(z80);								} /* DB   DD          */
OP(dd,3d) { illegal_1(z80); op_3d(z80);								} /* DB   DD          */
OP(dd,3e) { illegal_1(z80); op_3e(z80);								} /* DB   DD          */
OP(dd,3f) { illegal_1(z80); op_3f(z80);								} /* DB   DD          */

OP(dd,40) { illegal_1(z80); op_40(z80);								} /* DB   DD          */
OP(dd,41) { illegal_1(z80); op_41(z80);								} /* DB   DD          */
OP(dd,42) { illegal_1(z80); op_42(z80);								} /* DB   DD          */
OP(dd,43) { illegal_1(z80); op_43(z80);								} /* DB   DD          */
OP(dd,44) { B = HX;													} /* LD   B,HX        */
OP(dd,45) { B = LX;													} /* LD   B,LX        */
OP(dd,46) { EAX; B = RM(EA);										} /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(z80); op_47(z80);								} /* DB   DD          */

OP(dd,48) { illegal_1(z80); op_48(z80);								} /* DB   DD          */
OP(dd,49) { illegal_1(z80); op_49(z80);								} /* DB   DD          */
OP(dd,4a) { illegal_1(z80); op_4a(z80);								} /* DB   DD          */
OP(dd,4b) { illegal_1(z80); op_4b(z80);								} /* DB   DD          */
OP(dd,4c) { C = HX;													} /* LD   C,HX        */
OP(dd,4d) { C = LX;													} /* LD   C,LX        */
OP(dd,4e) { EAX; C = RM(EA);										} /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(z80); op_4f(z80);								} /* DB   DD          */

OP(dd,50) { illegal_1(z80); op_50(z80);								} /* DB   DD          */
OP(dd,51) { illegal_1(z80); op_51(z80);								} /* DB   DD          */
OP(dd,52) { illegal_1(z80); op_52(z80);								} /* DB   DD          */
OP(dd,53) { illegal_1(z80); op_53(z80);								} /* DB   DD          */
OP(dd,54) { D = HX;													} /* LD   D,HX        */
OP(dd,55) { D = LX;													} /* LD   D,LX        */
OP(dd,56) { EAX; D = RM(EA);										} /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(z80); op_57(z80);								} /* DB   DD          */

OP(dd,58) { illegal_1(z80); op_58(z80);								} /* DB   DD          */
OP(dd,59) { illegal_1(z80); op_59(z80);								} /* DB   DD          */
OP(dd,5a) { illegal_1(z80); op_5a(z80);								} /* DB   DD          */
OP(dd,5b) { illegal_1(z80); op_5b(z80);								} /* DB   DD          */
OP(dd,5c) { E = HX;													} /* LD   E,HX        */
OP(dd,5d) { E = LX;													} /* LD   E,LX        */
OP(dd,5e) { EAX; E = RM(EA);										} /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(z80); op_5f(z80);								} /* DB   DD          */

OP(dd,60) { HX = B;													} /* LD   HX,B        */
OP(dd,61) { HX = C;													} /* LD   HX,C        */
OP(dd,62) { HX = D;													} /* LD   HX,D        */
OP(dd,63) { HX = E;													} /* LD   HX,E        */
OP(dd,64) {															} /* LD   HX,HX       */
OP(dd,65) { HX = LX;												} /* LD   HX,LX       */
OP(dd,66) { EAX; H = RM(EA);										} /* LD   H,(IX+o)    */
OP(dd,67) { HX = A;													} /* LD   HX,A        */

OP(dd,68) { LX = B;													} /* LD   LX,B        */
OP(dd,69) { LX = C;													} /* LD   LX,C        */
OP(dd,6a) { LX = D;													} /* LD   LX,D        */
OP(dd,6b) { LX = E;													} /* LD   LX,E        */
OP(dd,6c) { LX = HX;												} /* LD   LX,HX       */
OP(dd,6d) {															} /* LD   LX,LX       */
OP(dd,6e) { EAX; L = RM(EA);										} /* LD   L,(IX+o)    */
OP(dd,6f) { LX = A;													} /* LD   LX,A        */

OP(dd,70) { EAX; WM( EA, B );										} /* LD   (IX+o),B    */
OP(dd,71) { EAX; WM( EA, C );										} /* LD   (IX+o),C    */
OP(dd,72) { EAX; WM( EA, D );										} /* LD   (IX+o),D    */
OP(dd,73) { EAX; WM( EA, E );										} /* LD   (IX+o),E    */
OP(dd,74) { EAX; WM( EA, H );										} /* LD   (IX+o),H    */
OP(dd,75) { EAX; WM( EA, L );										} /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(z80); op_76(z80);								} /* DB   DD          */
OP(dd,77) { EAX; WM( EA, A );										} /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(z80); op_78(z80);								} /* DB   DD          */
OP(dd,79) { illegal_1(z80); op_79(z80);								} /* DB   DD          */
OP(dd,7a) { illegal_1(z80); op_7a(z80);								} /* DB   DD          */
OP(dd,7b) { illegal_1(z80); op_7b(z80);								} /* DB   DD          */
OP(dd,7c) { A = HX;													} /* LD   A,HX        */
OP(dd,7d) { A = LX;													} /* LD   A,LX        */
OP(dd,7e) { EAX; A = RM(EA);										} /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(z80); op_7f(z80);								} /* DB   DD          */

OP(dd,80) { illegal_1(z80); op_80(z80);								} /* DB   DD          */
OP(dd,81) { illegal_1(z80); op_81(z80);								} /* DB   DD          */
OP(dd,82) { illegal_1(z80); op_82(z80);								} /* DB   DD          */
OP(dd,83) { illegal_1(z80); op_83(z80);								} /* DB   DD          */
OP(dd,84) { ADD(HX);												} /* ADD  A,HX        */
OP(dd,85) { ADD(LX);												} /* ADD  A,LX        */
OP(dd,86) { EAX; ADD(RM(EA));										} /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(z80); op_87(z80);								} /* DB   DD          */

OP(dd,88) { illegal_1(z80); op_88(z80);								} /* DB   DD          */
OP(dd,89) { illegal_1(z80); op_89(z80);								} /* DB   DD          */
OP(dd,8a) { illegal_1(z80); op_8a(z80);								} /* DB   DD          */
OP(dd,8b) { illegal_1(z80); op_8b(z80);								} /* DB   DD          */
OP(dd,8c) { ADC(HX);												} /* ADC  A,HX        */
OP(dd,8d) { ADC(LX);												} /* ADC  A,LX        */
OP(dd,8e) { EAX; ADC(RM(EA));										} /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(z80); op_8f(z80);								} /* DB   DD          */

OP(dd,90) { illegal_1(z80); op_90(z80);								} /* DB   DD          */
OP(dd,91) { illegal_1(z80); op_91(z80);								} /* DB   DD          */
OP(dd,92) { illegal_1(z80); op_92(z80);								} /* DB   DD          */
OP(dd,93) { illegal_1(z80); op_93(z80);								} /* DB   DD          */
OP(dd,94) { SUB(HX);												} /* SUB  HX          */
OP(dd,95) { SUB(LX);												} /* SUB  LX          */
OP(dd,96) { EAX; SUB(RM(EA));										} /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(z80); op_97(z80);								} /* DB   DD          */

OP(dd,98) { illegal_1(z80); op_98(z80);								} /* DB   DD          */
OP(dd,99) { illegal_1(z80); op_99(z80);								} /* DB   DD          */
OP(dd,9a) { illegal_1(z80); op_9a(z80);								} /* DB   DD          */
OP(dd,9b) { illegal_1(z80); op_9b(z80);								} /* DB   DD          */
OP(dd,9c) { SBC(HX);												} /* SBC  A,HX        */
OP(dd,9d) { SBC(LX);												} /* SBC  A,LX        */
OP(dd,9e) { EAX; SBC(RM(EA));										} /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(z80); op_9f(z80);								} /* DB   DD          */

OP(dd,a0) { illegal_1(z80); op_a0(z80);								} /* DB   DD          */
OP(dd,a1) { illegal_1(z80); op_a1(z80);								} /* DB   DD          */
OP(dd,a2) { illegal_1(z80); op_a2(z80);								} /* DB   DD          */
OP(dd,a3) { illegal_1(z80); op_a3(z80);								} /* DB   DD          */
OP(dd,a4) { AND(HX);												} /* AND  HX          */
OP(dd,a5) { AND(LX);												} /* AND  LX          */
OP(dd,a6) { EAX; AND(RM(EA));										} /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(z80); op_a7(z80);								} /* DB   DD          */

OP(dd,a8) { illegal_1(z80); op_a8(z80);								} /* DB   DD          */
OP(dd,a9) { illegal_1(z80); op_a9(z80);								} /* DB   DD          */
OP(dd,aa) { illegal_1(z80); op_aa(z80);								} /* DB   DD          */
OP(dd,ab) { illegal_1(z80); op_ab(z80);								} /* DB   DD          */
OP(dd,ac) { XOR(HX);												} /* XOR  HX          */
OP(dd,ad) { XOR(LX);												} /* XOR  LX          */
OP(dd,ae) { EAX; XOR(RM(EA));										} /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(z80); op_af(z80);								} /* DB   DD          */

OP(dd,b0) { illegal_1(z80); op_b0(z80);								} /* DB   DD          */
OP(dd,b1) { illegal_1(z80); op_b1(z80);								} /* DB   DD          */
OP(dd,b2) { illegal_1(z80); op_b2(z80);								} /* DB   DD          */
OP(dd,b3) { illegal_1(z80); op_b3(z80);								} /* DB   DD          */
OP(dd,b4) { OR(HX);													} /* OR   HX          */
OP(dd,b5) { OR(LX);													} /* OR   LX          */
OP(dd,b6) { EAX; OR(RM(EA));										} /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(z80); op_b7(z80);								} /* DB   DD          */

OP(dd,b8) { illegal_1(z80); op_b8(z80);								} /* DB   DD          */
OP(dd,b9) { illegal_1(z80); op_b9(z80);								} /* DB   DD          */
OP(dd,ba) { illegal_1(z80); op_ba(z80);								} /* DB   DD          */
OP(dd,bb) { illegal_1(z80); op_bb(z80);								} /* DB   DD          */
OP(dd,bc) { CP(HX);													} /* CP   HX          */
OP(dd,bd) { CP(LX);													} /* CP   LX          */
OP(dd,be) { EAX; CP(RM(EA));										} /* CP   (IX+o)      */
OP(dd,bf) { illegal_1(z80); op_bf(z80);								} /* DB   DD          */

OP(dd,c0) { illegal_1(z80); op_c0(z80);								} /* DB   DD          */
OP(dd,c1) { illegal_1(z80); op_c1(z80);								} /* DB   DD          */
OP(dd,c2) { illegal_1(z80); op_c2(z80);								} /* DB   DD          */
OP(dd,c3) { illegal_1(z80); op_c3(z80);								} /* DB   DD          */
OP(dd,c4) { illegal_1(z80); op_c4(z80);								} /* DB   DD          */
OP(dd,c5) { illegal_1(z80); op_c5(z80);								} /* DB   DD          */
OP(dd,c6) { illegal_1(z80); op_c6(z80);								} /* DB   DD          */
OP(dd,c7) { illegal_1(z80); op_c7(z80);								} /* DB   DD          */

OP(dd,c8) { illegal_1(z80); op_c8(z80);								} /* DB   DD          */
OP(dd,c9) { illegal_1(z80); op_c9(z80);								} /* DB   DD          */
OP(dd,ca) { illegal_1(z80); op_ca(z80);								} /* DB   DD          */
OP(dd,cb) { EAX; EXEC(z80,xycb,ARG(z80));							} /* **   DD CB xx    */
OP(dd,cc) { illegal_1(z80); op_cc(z80);								} /* DB   DD          */
OP(dd,cd) { illegal_1(z80); op_cd(z80);								} /* DB   DD          */
OP(dd,ce) { illegal_1(z80); op_ce(z80);								} /* DB   DD          */
OP(dd,cf) { illegal_1(z80); op_cf(z80);								} /* DB   DD          */

OP(dd,d0) { illegal_1(z80); op_d0(z80);								} /* DB   DD          */
OP(dd,d1) { illegal_1(z80); op_d1(z80);								} /* DB   DD          */
OP(dd,d2) { illegal_1(z80); op_d2(z80);								} /* DB   DD          */
OP(dd,d3) { illegal_1(z80); op_d3(z80);								} /* DB   DD          */
OP(dd,d4) { illegal_1(z80); op_d4(z80);								} /* DB   DD          */
OP(dd,d5) { illegal_1(z80); op_d5(z80);								} /* DB   DD          */
OP(dd,d6) { illegal_1(z80); op_d6(z80);								} /* DB   DD          */
OP(dd,d7) { illegal_1(z80); op_d7(z80);								} /* DB   DD          */

OP(dd,d8) { illegal_1(z80); op_d8(z80);								} /* DB   DD          */
OP(dd,d9) { illegal_1(z80); op_d9(z80);								} /* DB   DD          */
OP(dd,da) { illegal_1(z80); op_da(z80);								} /* DB   DD          */
OP(dd,db) { illegal_1(z80); op_db(z80);								} /* DB   DD          */
OP(dd,dc) { illegal_1(z80); op_dc(z80);								} /* DB   DD          */
OP(dd,dd) { illegal_1(z80); op_dd(z80);								} /* DB   DD          */
OP(dd,de) { illegal_1(z80); op_de(z80);								} /* DB   DD          */
OP(dd,df) { illegal_1(z80); op_df(z80);								} /* DB   DD          */

OP(dd,e0) { illegal_1(z80); op_e0(z80);								} /* DB   DD          */
OP(dd,e1) { POP( ix );												} /* POP  IX          */
OP(dd,e2) { illegal_1(z80); op_e2(z80);								} /* DB   DD          */
OP(dd,e3) { EXSP( ix );												} /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(z80); op_e4(z80);								} /* DB   DD          */
OP(dd,e5) { PUSH( ix );												} /* PUSH IX          */
OP(dd,e6) { illegal_1(z80); op_e6(z80);								} /* DB   DD          */
OP(dd,e7) { illegal_1(z80); op_e7(z80);								} /* DB   DD          */

OP(dd,e8) { illegal_1(z80); op_e8(z80);								} /* DB   DD          */
OP(dd,e9) { PC = IX; change_pc(PCD);								} /* JP   (IX)        */
OP(dd,ea) { illegal_1(z80); op_ea(z80);								} /* DB   DD          */
OP(dd,eb) { illegal_1(z80); op_eb(z80);								} /* DB   DD          */
OP(dd,ec) { illegal_1(z80); op_ec(z80);								} /* DB   DD          */
OP(dd,ed) { illegal_1(z80); op_ed(z80);								} /* DB   DD          */
OP(dd,ee) { illegal_1(z80); op_ee(z80);								} /* DB   DD          */
OP(dd,ef) { illegal_1(z80); op_ef(z80);								} /* DB   DD          */

OP(dd,f0) { illegal_1(z80); op_f0(z80);								} /* DB   DD          */
OP(dd,f1) { illegal_1(z80); op_f1(z80);								} /* DB   DD          */
OP(dd,f2) { illegal_1(z80); op_f2(z80);								} /* DB   DD          */
OP(dd,f3) { illegal_1(z80); op_f3(z80);								} /* DB   DD          */
OP(dd,f4) { illegal_1(z80); op_f4(z80);								} /* DB   DD          */
OP(dd,f5) { illegal_1(z80); op_f5(z80);								} /* DB   DD          */
OP(dd,f6) { illegal_1(z80); op_f6(z80);								} /* DB   DD          */
OP(dd,f7) { illegal_1(z80); op_f7(z80);								} /* DB   DD          */

OP(dd,f8) { illegal_1(z80); op_f8(z80);								} /* DB   DD          */
OP(dd,f9) { SP = IX;												} /* LD   SP,IX       */
OP(dd,fa) { illegal_1(z80); op_fa(z80);								} /* DB   DD          */
OP(dd,fb) { illegal_1(z80); op_fb(z80);								} /* DB   DD          */
OP(dd,fc) { illegal_1(z80); op_fc(z80);								} /* DB   DD          */
OP(dd,fd) { illegal_1(z80); op_fd(z80);								} /* DB   DD          */
OP(dd,fe) { illegal_1(z80); op_fe(z80);								} /* DB   DD          */
OP(dd,ff) { illegal_1(z80); op_ff(z80);								} /* DB   DD          */

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP(fd,00) { illegal_1(z80); op_00(z80);								} /* DB   FD          */
OP(fd,01) { illegal_1(z80); op_01(z80);								} /* DB   FD          */
OP(fd,02) { illegal_1(z80); op_02(z80);								} /* DB   FD          */
OP(fd,03) { illegal_1(z80); op_03(z80);								} /* DB   FD          */
OP(fd,04) { illegal_1(z80); op_04(z80);								} /* DB   FD          */
OP(fd,05) { illegal_1(z80); op_05(z80);								} /* DB   FD          */
OP(fd,06) { illegal_1(z80); op_06(z80);								} /* DB   FD          */
OP(fd,07) { illegal_1(z80); op_07(z80);								} /* DB   FD          */

OP(fd,08) { illegal_1(z80); op_08(z80);								} /* DB   FD          */
OP(fd,09) { ADD16(iy,bc);											} /* ADD  IY,BC       */
OP(fd,0a) { illegal_1(z80); op_0a(z80);								} /* DB   FD          */
OP(fd,0b) { illegal_1(z80); op_0b(z80);								} /* DB   FD          */
OP(fd,0c) { illegal_1(z80); op_0c(z80);								} /* DB   FD          */
OP(fd,0d) { illegal_1(z80); op_0d(z80);								} /* DB   FD          */
OP(fd,0e) { illegal_1(z80); op_0e(z80);								} /* DB   FD          */
OP(fd,0f) { illegal_1(z80); op_0f(z80);								} /* DB   FD          */

OP(fd,10) { illegal_1(z80); op_10(z80);								} /* DB   FD          */
OP(fd,11) { illegal_1(z80); op_11(z80);								} /* DB   FD          */
OP(fd,12) { illegal_1(z80); op_12(z80);								} /* DB   FD          */
OP(fd,13) { illegal_1(z80); op_13(z80);								} /* DB   FD          */
OP(fd,14) { illegal_1(z80); op_14(z80);								} /* DB   FD          */
OP(fd,15) { illegal_1(z80); op_15(z80);								} /* DB   FD          */
OP(fd,16) { illegal_1(z80); op_16(z80);								} /* DB   FD          */
OP(fd,17) { illegal_1(z80); op_17(z80);								} /* DB   FD          */

OP(fd,18) { illegal_1(z80); op_18(z80);								} /* DB   FD          */
OP(fd,19) { ADD16(iy,de);											} /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(z80); op_1a(z80);								} /* DB   FD          */
OP(fd,1b) { illegal_1(z80); op_1b(z80);								} /* DB   FD          */
OP(fd,1c) { illegal_1(z80); op_1c(z80);								} /* DB   FD          */
OP(fd,1d) { illegal_1(z80); op_1d(z80);								} /* DB   FD          */
OP(fd,1e) { illegal_1(z80); op_1e(z80);								} /* DB   FD          */
OP(fd,1f) { illegal_1(z80); op_1f(z80);								} /* DB   FD          */

OP(fd,20) { illegal_1(z80); op_20(z80);								} /* DB   FD          */
OP(fd,21) { IY = ARG16(z80);										} /* LD   IY,w        */
OP(fd,22) { EA = ARG16(z80); WM16( EA, &z80->iy ); MEMPTR = EA+1;	} /* LD   (w),IY      */
OP(fd,23) { IY++;													} /* INC  IY          */
OP(fd,24) { HY = INC(z80, HY);										} /* INC  HY          */
OP(fd,25) { HY = DEC(z80, HY);										} /* DEC  HY          */
OP(fd,26) { HY = ARG(z80);											} /* LD   HY,n        */
OP(fd,27) { illegal_1(z80); op_27(z80);								} /* DB   FD          */

OP(fd,28) { illegal_1(z80); op_28(z80);								} /* DB   FD          */
OP(fd,29) { ADD16(iy,iy);											} /* ADD  IY,IY       */
OP(fd,2a) { EA = ARG16(z80); RM16( EA, &z80->iy ); MEMPTR = EA+1;	} /* LD   IY,(w)      */
OP(fd,2b) { IY--;													} /* DEC  IY          */
OP(fd,2c) { LY = INC(z80, LY);										} /* INC  LY          */
OP(fd,2d) { LY = DEC(z80, LY);										} /* DEC  LY          */
OP(fd,2e) { LY = ARG(z80);											} /* LD   LY,n        */
OP(fd,2f) { illegal_1(z80); op_2f(z80);								} /* DB   FD          */

OP(fd,30) { illegal_1(z80); op_30(z80);								} /* DB   FD          */
OP(fd,31) { illegal_1(z80); op_31(z80);								} /* DB   FD          */
OP(fd,32) { illegal_1(z80); op_32(z80);								} /* DB   FD          */
OP(fd,33) { illegal_1(z80); op_33(z80);								} /* DB   FD          */
OP(fd,34) { EAY; WM( EA, INC(z80, RM(EA)) );						} /* INC  (IY+o)      */
OP(fd,35) { EAY; WM( EA, DEC(z80, RM(EA)) );						} /* DEC  (IY+o)      */
OP(fd,36) { EAY; WM( EA, ARG(z80) );								} /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(z80); op_37(z80);								} /* DB   FD          */

OP(fd,38) { illegal_1(z80); op_38(z80);								} /* DB   FD          */
OP(fd,39) { ADD16(iy,sp);											} /* ADD  IY,SP       */
OP(fd,3a) { illegal_1(z80); op_3a(z80);								} /* DB   FD          */
OP(fd,3b) { illegal_1(z80); op_3b(z80);								} /* DB   FD          */
OP(fd,3c) { illegal_1(z80); op_3c(z80);								} /* DB   FD          */
OP(fd,3d) { illegal_1(z80); op_3d(z80);								} /* DB   FD          */
OP(fd,3e) { illegal_1(z80); op_3e(z80);								} /* DB   FD          */
OP(fd,3f) { illegal_1(z80); op_3f(z80);								} /* DB   FD          */

OP(fd,40) { illegal_1(z80); op_40(z80);								} /* DB   FD          */
OP(fd,41) { illegal_1(z80); op_41(z80);								} /* DB   FD          */
OP(fd,42) { illegal_1(z80); op_42(z80);								} /* DB   FD          */
OP(fd,43) { illegal_1(z80); op_43(z80);								} /* DB   FD          */
OP(fd,44) { B = HY;													} /* LD   B,HY        */
OP(fd,45) { B = LY;													} /* LD   B,LY        */
OP(fd,46) { EAY; B = RM(EA);										} /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(z80); op_47(z80);								} /* DB   FD          */

OP(fd,48) { illegal_1(z80); op_48(z80);								} /* DB   FD          */
OP(fd,49) { illegal_1(z80); op_49(z80);								} /* DB   FD          */
OP(fd,4a) { illegal_1(z80); op_4a(z80);								} /* DB   FD          */
OP(fd,4b) { illegal_1(z80); op_4b(z80);								} /* DB   FD          */
OP(fd,4c) { C = HY;													} /* LD   C,HY        */
OP(fd,4d) { C = LY;													} /* LD   C,LY        */
OP(fd,4e) { EAY; C = RM(EA);										} /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(z80); op_4f(z80);								} /* DB   FD          */

OP(fd,50) { illegal_1(z80); op_50(z80);								} /* DB   FD          */
OP(fd,51) { illegal_1(z80); op_51(z80);								} /* DB   FD          */
OP(fd,52) { illegal_1(z80); op_52(z80);								} /* DB   FD          */
OP(fd,53) { illegal_1(z80); op_53(z80);								} /* DB   FD          */
OP(fd,54) { D = HY;													} /* LD   D,HY        */
OP(fd,55) { D = LY;													} /* LD   D,LY        */
OP(fd,56) { EAY; D = RM(EA);										} /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(z80); op_57(z80);								} /* DB   FD          */

OP(fd,58) { illegal_1(z80); op_58(z80);								} /* DB   FD          */
OP(fd,59) { illegal_1(z80); op_59(z80);								} /* DB   FD          */
OP(fd,5a) { illegal_1(z80); op_5a(z80);								} /* DB   FD          */
OP(fd,5b) { illegal_1(z80); op_5b(z80);								} /* DB   FD          */
OP(fd,5c) { E = HY;													} /* LD   E,HY        */
OP(fd,5d) { E = LY;													} /* LD   E,LY        */
OP(fd,5e) { EAY; E = RM(EA);										} /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(z80); op_5f(z80);								} /* DB   FD          */

OP(fd,60) { HY = B;													} /* LD   HY,B        */
OP(fd,61) { HY = C;													} /* LD   HY,C        */
OP(fd,62) { HY = D;													} /* LD   HY,D        */
OP(fd,63) { HY = E;													} /* LD   HY,E        */
OP(fd,64) {															} /* LD   HY,HY       */
OP(fd,65) { HY = LY;												} /* LD   HY,LY       */
OP(fd,66) { EAY; H = RM(EA);										} /* LD   H,(IY+o)    */
OP(fd,67) { HY = A;													} /* LD   HY,A        */

OP(fd,68) { LY = B;													} /* LD   LY,B        */
OP(fd,69) { LY = C;													} /* LD   LY,C        */
OP(fd,6a) { LY = D;													} /* LD   LY,D        */
OP(fd,6b) { LY = E;													} /* LD   LY,E        */
OP(fd,6c) { LY = HY;												} /* LD   LY,HY       */
OP(fd,6d) {															} /* LD   LY,LY       */
OP(fd,6e) { EAY; L = RM(EA);										} /* LD   L,(IY+o)    */
OP(fd,6f) { LY = A;													} /* LD   LY,A        */

OP(fd,70) { EAY; WM( EA, B );										} /* LD   (IY+o),B    */
OP(fd,71) { EAY; WM( EA, C );										} /* LD   (IY+o),C    */
OP(fd,72) { EAY; WM( EA, D );										} /* LD   (IY+o),D    */
OP(fd,73) { EAY; WM( EA, E );										} /* LD   (IY+o),E    */
OP(fd,74) { EAY; WM( EA, H );										} /* LD   (IY+o),H    */
OP(fd,75) { EAY; WM( EA, L );										} /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(z80); op_76(z80);								} /* DB   FD          */
OP(fd,77) { EAY; WM( EA, A );										} /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(z80); op_78(z80);								} /* DB   FD          */
OP(fd,79) { illegal_1(z80); op_79(z80);								} /* DB   FD          */
OP(fd,7a) { illegal_1(z80); op_7a(z80);								} /* DB   FD          */
OP(fd,7b) { illegal_1(z80); op_7b(z80);								} /* DB   FD          */
OP(fd,7c) { A = HY;													} /* LD   A,HY        */
OP(fd,7d) { A = LY;													} /* LD   A,LY        */
OP(fd,7e) { EAY; A = RM(EA);										} /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(z80); op_7f(z80);								} /* DB   FD          */

OP(fd,80) { illegal_1(z80); op_80(z80);								} /* DB   FD          */
OP(fd,81) { illegal_1(z80); op_81(z80);								} /* DB   FD          */
OP(fd,82) { illegal_1(z80); op_82(z80);								} /* DB   FD          */
OP(fd,83) { illegal_1(z80); op_83(z80);								} /* DB   FD          */
OP(fd,84) { ADD(HY);												} /* ADD  A,HY        */
OP(fd,85) { ADD(LY);												} /* ADD  A,LY        */
OP(fd,86) { EAY; ADD(RM(EA));										} /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(z80); op_87(z80);								} /* DB   FD          */

OP(fd,88) { illegal_1(z80); op_88(z80);								} /* DB   FD          */
OP(fd,89) { illegal_1(z80); op_89(z80);								} /* DB   FD          */
OP(fd,8a) { illegal_1(z80); op_8a(z80);								} /* DB   FD          */
OP(fd,8b) { illegal_1(z80); op_8b(z80);								} /* DB   FD          */
OP(fd,8c) { ADC(HY);												} /* ADC  A,HY        */
OP(fd,8d) { ADC(LY);												} /* ADC  A,LY        */
OP(fd,8e) { EAY; ADC(RM(EA));										} /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(z80); op_8f(z80);								} /* DB   FD          */

OP(fd,90) { illegal_1(z80); op_90(z80);								} /* DB   FD          */
OP(fd,91) { illegal_1(z80); op_91(z80);								} /* DB   FD          */
OP(fd,92) { illegal_1(z80); op_92(z80);								} /* DB   FD          */
OP(fd,93) { illegal_1(z80); op_93(z80);								} /* DB   FD          */
OP(fd,94) { SUB(HY);												} /* SUB  HY          */
OP(fd,95) { SUB(LY);												} /* SUB  LY          */
OP(fd,96) { EAY; SUB(RM(EA));										} /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(z80); op_97(z80);								} /* DB   FD          */

OP(fd,98) { illegal_1(z80); op_98(z80);								} /* DB   FD          */
OP(fd,99) { illegal_1(z80); op_99(z80);								} /* DB   FD          */
OP(fd,9a) { illegal_1(z80); op_9a(z80);								} /* DB   FD          */
OP(fd,9b) { illegal_1(z80); op_9b(z80);								} /* DB   FD          */
OP(fd,9c) { SBC(HY);												} /* SBC  A,HY        */
OP(fd,9d) { SBC(LY);												} /* SBC  A,LY        */
OP(fd,9e) { EAY; SBC(RM(EA));										} /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(z80); op_9f(z80);								} /* DB   FD          */

OP(fd,a0) { illegal_1(z80); op_a0(z80);								} /* DB   FD          */
OP(fd,a1) { illegal_1(z80); op_a1(z80);								} /* DB   FD          */
OP(fd,a2) { illegal_1(z80); op_a2(z80);								} /* DB   FD          */
OP(fd,a3) { illegal_1(z80); op_a3(z80);								} /* DB   FD          */
OP(fd,a4) { AND(HY);												} /* AND  HY          */
OP(fd,a5) { AND(LY);												} /* AND  LY          */
OP(fd,a6) { EAY; AND(RM(EA));										} /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(z80); op_a7(z80);								} /* DB   FD          */

OP(fd,a8) { illegal_1(z80); op_a8(z80);								} /* DB   FD          */
OP(fd,a9) { illegal_1(z80); op_a9(z80);								} /* DB   FD          */
OP(fd,aa) { illegal_1(z80); op_aa(z80);								} /* DB   FD          */
OP(fd,ab) { illegal_1(z80); op_ab(z80);								} /* DB   FD          */
OP(fd,ac) { XOR(HY);												} /* XOR  HY          */
OP(fd,ad) { XOR(LY);												} /* XOR  LY          */
OP(fd,ae) { EAY; XOR(RM(EA));										} /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(z80); op_af(z80);								} /* DB   FD          */

OP(fd,b0) { illegal_1(z80); op_b0(z80);								} /* DB   FD          */
OP(fd,b1) { illegal_1(z80); op_b1(z80);								} /* DB   FD          */
OP(fd,b2) { illegal_1(z80); op_b2(z80);								} /* DB   FD          */
OP(fd,b3) { illegal_1(z80); op_b3(z80);								} /* DB   FD          */
OP(fd,b4) { OR(HY);													} /* OR   HY          */
OP(fd,b5) { OR(LY);													} /* OR   LY          */
OP(fd,b6) { EAY; OR(RM(EA));										} /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(z80); op_b7(z80);								} /* DB   FD          */

OP(fd,b8) { illegal_1(z80); op_b8(z80);								} /* DB   FD          */
OP(fd,b9) { illegal_1(z80); op_b9(z80);								} /* DB   FD          */
OP(fd,ba) { illegal_1(z80); op_ba(z80);								} /* DB   FD          */
OP(fd,bb) { illegal_1(z80); op_bb(z80);								} /* DB   FD          */
OP(fd,bc) { CP(HY);													} /* CP   HY          */
OP(fd,bd) { CP(LY);													} /* CP   LY          */
OP(fd,be) { EAY; CP(RM(EA));										} /* CP   (IY+o)      */
OP(fd,bf) { illegal_1(z80); op_bf(z80);								} /* DB   FD          */

OP(fd,c0) { illegal_1(z80); op_c0(z80);								} /* DB   FD          */
OP(fd,c1) { illegal_1(z80); op_c1(z80);								} /* DB   FD          */
OP(fd,c2) { illegal_1(z80); op_c2(z80);								} /* DB   FD          */
OP(fd,c3) { illegal_1(z80); op_c3(z80);								} /* DB   FD          */
OP(fd,c4) { illegal_1(z80); op_c4(z80);								} /* DB   FD          */
OP(fd,c5) { illegal_1(z80); op_c5(z80);								} /* DB   FD          */
OP(fd,c6) { illegal_1(z80); op_c6(z80);								} /* DB   FD          */
OP(fd,c7) { illegal_1(z80); op_c7(z80);								} /* DB   FD          */

OP(fd,c8) { illegal_1(z80); op_c8(z80);								} /* DB   FD          */
OP(fd,c9) { illegal_1(z80); op_c9(z80);								} /* DB   FD          */
OP(fd,ca) { illegal_1(z80); op_ca(z80);								} /* DB   FD          */
OP(fd,cb) { EAY; EXEC(z80,xycb,ARG(z80));							} /* **   FD CB xx    */
OP(fd,cc) { illegal_1(z80); op_cc(z80);								} /* DB   FD          */
OP(fd,cd) { illegal_1(z80); op_cd(z80);								} /* DB   FD          */
OP(fd,ce) { illegal_1(z80); op_ce(z80);								} /* DB   FD          */
OP(fd,cf) { illegal_1(z80); op_cf(z80);								} /* DB   FD          */

OP(fd,d0) { illegal_1(z80); op_d0(z80);								} /* DB   FD          */
OP(fd,d1) { illegal_1(z80); op_d1(z80);								} /* DB   FD          */
OP(fd,d2) { illegal_1(z80); op_d2(z80);								} /* DB   FD          */
OP(fd,d3) { illegal_1(z80); op_d3(z80);								} /* DB   FD          */
OP(fd,d4) { illegal_1(z80); op_d4(z80);								} /* DB   FD          */
OP(fd,d5) { illegal_1(z80); op_d5(z80);								} /* DB   FD          */
OP(fd,d6) { illegal_1(z80); op_d6(z80);								} /* DB   FD          */
OP(fd,d7) { illegal_1(z80); op_d7(z80);								} /* DB   FD          */

OP(fd,d8) { illegal_1(z80); op_d8(z80);								} /* DB   FD          */
OP(fd,d9) { illegal_1(z80); op_d9(z80);								} /* DB   FD          */
OP(fd,da) { illegal_1(z80); op_da(z80);								} /* DB   FD          */
OP(fd,db) { illegal_1(z80); op_db(z80);								} /* DB   FD          */
OP(fd,dc) { illegal_1(z80); op_dc(z80);								} /* DB   FD          */
OP(fd,dd) { illegal_1(z80); op_dd(z80);								} /* DB   FD          */
OP(fd,de) { illegal_1(z80); op_de(z80);								} /* DB   FD          */
OP(fd,df) { illegal_1(z80); op_df(z80);								} /* DB   FD          */

OP(fd,e0) { illegal_1(z80); op_e0(z80);								} /* DB   FD          */
OP(fd,e1) { POP( iy );												} /* POP  IY          */
OP(fd,e2) { illegal_1(z80); op_e2(z80);								} /* DB   FD          */
OP(fd,e3) { EXSP( iy );												} /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(z80); op_e4(z80);								} /* DB   FD          */
OP(fd,e5) { PUSH( iy );												} /* PUSH IY          */
OP(fd,e6) { illegal_1(z80); op_e6(z80);								} /* DB   FD          */
OP(fd,e7) { illegal_1(z80); op_e7(z80);								} /* DB   FD          */

OP(fd,e8) { illegal_1(z80); op_e8(z80);								} /* DB   FD          */
OP(fd,e9) { PC = IY; change_pc(PCD);								} /* JP   (IY)        */
OP(fd,ea) { illegal_1(z80); op_ea(z80);								} /* DB   FD          */
OP(fd,eb) { illegal_1(z80); op_eb(z80);								} /* DB   FD          */
OP(fd,ec) { illegal_1(z80); op_ec(z80);								} /* DB   FD          */
OP(fd,ed) { illegal_1(z80); op_ed(z80);								} /* DB   FD          */
OP(fd,ee) { illegal_1(z80); op_ee(z80);								} /* DB   FD          */
OP(fd,ef) { illegal_1(z80); op_ef(z80);								} /* DB   FD          */

OP(fd,f0) { illegal_1(z80); op_f0(z80);								} /* DB   FD          */
OP(fd,f1) { illegal_1(z80); op_f1(z80);								} /* DB   FD          */
OP(fd,f2) { illegal_1(z80); op_f2(z80);								} /* DB   FD          */
OP(fd,f3) { illegal_1(z80); op_f3(z80);								} /* DB   FD          */
OP(fd,f4) { illegal_1(z80); op_f4(z80);								} /* DB   FD          */
OP(fd,f5) { illegal_1(z80); op_f5(z80);								} /* DB   FD          */
OP(fd,f6) { illegal_1(z80); op_f6(z80);								} /* DB   FD          */
OP(fd,f7) { illegal_1(z80); op_f7(z80);								} /* DB   FD          */

OP(fd,f8) { illegal_1(z80); op_f8(z80);								} /* DB   FD          */
OP(fd,f9) { SP = IY;												} /* LD   SP,IY       */
OP(fd,fa) { illegal_1(z80); op_fa(z80);								} /* DB   FD          */
OP(fd,fb) { illegal_1(z80); op_fb(z80);								} /* DB   FD          */
OP(fd,fc) { illegal_1(z80); op_fc(z80);								} /* DB   FD          */
OP(fd,fd) { illegal_1(z80); op_fd(z80);								} /* DB   FD          */
OP(fd,fe) { illegal_1(z80); op_fe(z80);								} /* DB   FD          */
OP(fd,ff) { illegal_1(z80); op_ff(z80);								} /* DB   FD          */

OP(illegal,2)
{
	logerror("Z80 #%d ill. opcode $ed $%02x\n",
			cpu_getactivecpu(), cpu_readop((PCD-1)&0xffff));
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP(ed,00) { illegal_2(z80);											} /* DB   ED          */
OP(ed,01) { illegal_2(z80);											} /* DB   ED          */
OP(ed,02) { illegal_2(z80);											} /* DB   ED          */
OP(ed,03) { illegal_2(z80);											} /* DB   ED          */
OP(ed,04) { illegal_2(z80);											} /* DB   ED          */
OP(ed,05) { illegal_2(z80);											} /* DB   ED          */
OP(ed,06) { illegal_2(z80);											} /* DB   ED          */
OP(ed,07) { illegal_2(z80);											} /* DB   ED          */

OP(ed,08) { illegal_2(z80);											} /* DB   ED          */
OP(ed,09) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,0f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,10) { illegal_2(z80);											} /* DB   ED          */
OP(ed,11) { illegal_2(z80);											} /* DB   ED          */
OP(ed,12) { illegal_2(z80);											} /* DB   ED          */
OP(ed,13) { illegal_2(z80);											} /* DB   ED          */
OP(ed,14) { illegal_2(z80);											} /* DB   ED          */
OP(ed,15) { illegal_2(z80);											} /* DB   ED          */
OP(ed,16) { illegal_2(z80);											} /* DB   ED          */
OP(ed,17) { illegal_2(z80);											} /* DB   ED          */

OP(ed,18) { illegal_2(z80);											} /* DB   ED          */
OP(ed,19) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,1f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,20) { illegal_2(z80);											} /* DB   ED          */
OP(ed,21) { illegal_2(z80);											} /* DB   ED          */
OP(ed,22) { illegal_2(z80);											} /* DB   ED          */
OP(ed,23) { illegal_2(z80);											} /* DB   ED          */
OP(ed,24) { illegal_2(z80);											} /* DB   ED          */
OP(ed,25) { illegal_2(z80);											} /* DB   ED          */
OP(ed,26) { illegal_2(z80);											} /* DB   ED          */
OP(ed,27) { illegal_2(z80);											} /* DB   ED          */

OP(ed,28) { illegal_2(z80);											} /* DB   ED          */
OP(ed,29) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,2f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,30) { illegal_2(z80);											} /* DB   ED          */
OP(ed,31) { illegal_2(z80);											} /* DB   ED          */
OP(ed,32) { illegal_2(z80);											} /* DB   ED          */
OP(ed,33) { illegal_2(z80);											} /* DB   ED          */
OP(ed,34) { illegal_2(z80);											} /* DB   ED          */
OP(ed,35) { illegal_2(z80);											} /* DB   ED          */
OP(ed,36) { illegal_2(z80);											} /* DB   ED          */
OP(ed,37) { illegal_2(z80);											} /* DB   ED          */

OP(ed,38) { illegal_2(z80);											} /* DB   ED          */
OP(ed,39) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,3f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,40) { B = IN(BC); F = (F & CF) | SZP[B];						} /* IN   B,(C)       */
OP(ed,41) { OUT(BC, B);												} /* OUT  (C),B       */
OP(ed,42) { SBC16( bc );											} /* SBC  HL,BC       */
OP(ed,43) { EA = ARG16(z80); WM16( EA, &z80->bc ); MEMPTR = EA+1;	} /* LD   (w),BC      */
OP(ed,44) { NEG;													} /* NEG              */
OP(ed,45) { RETN;													} /* RETN;            */
OP(ed,46) { IM = 0;													} /* IM   0           */
OP(ed,47) { LD_I_A;													} /* LD   I,A         */

OP(ed,48) { C = IN(BC); F = (F & CF) | SZP[C];						} /* IN   C,(C)       */
OP(ed,49) { OUT(BC, C);												} /* OUT  (C),C       */
OP(ed,4a) { ADC16( bc );											} /* ADC  HL,BC       */
OP(ed,4b) { EA = ARG16(z80); RM16( EA, &z80->bc ); MEMPTR = EA+1;	} /* LD   BC,(w)      */
OP(ed,4c) { NEG;													} /* NEG              */
OP(ed,4d) { RETI;													} /* RETI             */
OP(ed,4e) { IM = 0;													} /* IM   0           */
OP(ed,4f) { LD_R_A;													} /* LD   R,A         */

OP(ed,50) { D = IN(BC); F = (F & CF) | SZP[D];						} /* IN   D,(C)       */
OP(ed,51) { OUT(BC, D);												} /* OUT  (C),D       */
OP(ed,52) { SBC16( de );											} /* SBC  HL,DE       */
OP(ed,53) { EA = ARG16(z80); WM16( EA, &z80->de ); MEMPTR = EA+1;	} /* LD   (w),DE      */
OP(ed,54) { NEG;													} /* NEG              */
OP(ed,55) { RETN;													} /* RETN;            */
OP(ed,56) { IM = 1;													} /* IM   1           */
OP(ed,57) { LD_A_I;													} /* LD   A,I         */

OP(ed,58) { E = IN(BC); F = (F & CF) | SZP[E];						} /* IN   E,(C)       */
OP(ed,59) { OUT(BC, E);												} /* OUT  (C),E       */
OP(ed,5a) { ADC16( de );											} /* ADC  HL,DE       */
OP(ed,5b) { EA = ARG16(z80); RM16( EA, &z80->de ); MEMPTR = EA+1;	} /* LD   DE,(w)      */
OP(ed,5c) { NEG;													} /* NEG              */
OP(ed,5d) { RETI;													} /* RETI             */
OP(ed,5e) { IM = 2;													} /* IM   2           */
OP(ed,5f) { LD_A_R;													} /* LD   A,R         */

OP(ed,60) { H = IN(BC); F = (F & CF) | SZP[H];						} /* IN   H,(C)       */
OP(ed,61) { OUT(BC, H);												} /* OUT  (C),H       */
OP(ed,62) { SBC16( hl );											} /* SBC  HL,HL       */
OP(ed,63) { EA = ARG16(z80); WM16( EA, &z80->hl ); MEMPTR = EA+1;	} /* LD   (w),HL      */
OP(ed,64) { NEG;													} /* NEG              */
OP(ed,65) { RETN;													} /* RETN;            */
OP(ed,66) { IM = 0;													} /* IM   0           */
OP(ed,67) { RRD;													} /* RRD  (HL)        */

OP(ed,68) { L = IN(BC); F = (F & CF) | SZP[L];						} /* IN   L,(C)       */
OP(ed,69) { OUT(BC, L);												} /* OUT  (C),L       */
OP(ed,6a) { ADC16( hl );											} /* ADC  HL,HL       */
OP(ed,6b) { EA = ARG16(z80); RM16( EA, &z80->hl ); MEMPTR = EA+1;	} /* LD   HL,(w)      */
OP(ed,6c) { NEG;													} /* NEG              */
OP(ed,6d) { RETI;													} /* RETI             */
OP(ed,6e) { IM = 0;													} /* IM   0           */
OP(ed,6f) { RLD;													} /* RLD  (HL)        */

OP(ed,70) { UINT8 res = IN(BC); F = (F & CF) | SZP[res];			} /* IN   0,(C)       */
OP(ed,71) { OUT(BC, 0);												} /* OUT  (C),0       */
OP(ed,72) { SBC16( sp );											} /* SBC  HL,SP       */
OP(ed,73) { EA = ARG16(z80); WM16( EA, &z80->sp ); MEMPTR = EA+1; 	} /* LD   (w),SP      */
OP(ed,74) { NEG;													} /* NEG              */
OP(ed,75) { RETN;													} /* RETN;            */
OP(ed,76) { IM = 1;													} /* IM   1           */
OP(ed,77) { illegal_2(z80);											} /* DB   ED,77       */

OP(ed,78) { A = IN(BC); F = (F & CF) | SZP[A];	MEMPTR = BC + 1;	} /* IN   A,(C)       */
OP(ed,79) { OUT(BC, A);	MEMPTR = BC + 1;							} /* OUT  (C),A       */
OP(ed,7a) { ADC16( sp );											} /* ADC  HL,SP       */
OP(ed,7b) { EA = ARG16(z80); RM16( EA, &z80->sp ); MEMPTR = EA+1; 	} /* LD   SP,(w)      */
OP(ed,7c) { NEG;													} /* NEG              */
OP(ed,7d) { RETI;													} /* RETI             */
OP(ed,7e) { IM = 2;													} /* IM   2           */
OP(ed,7f) { illegal_2(z80);											} /* DB   ED,7F       */

OP(ed,80) { illegal_2(z80);											} /* DB   ED          */
OP(ed,81) { illegal_2(z80);											} /* DB   ED          */
OP(ed,82) { illegal_2(z80);											} /* DB   ED          */
OP(ed,83) { illegal_2(z80);											} /* DB   ED          */
OP(ed,84) { illegal_2(z80);											} /* DB   ED          */
OP(ed,85) { illegal_2(z80);											} /* DB   ED          */
OP(ed,86) { illegal_2(z80);											} /* DB   ED          */
OP(ed,87) { illegal_2(z80);											} /* DB   ED          */

OP(ed,88) { illegal_2(z80);											} /* DB   ED          */
OP(ed,89) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,8f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,90) { illegal_2(z80);											} /* DB   ED          */
OP(ed,91) { illegal_2(z80);											} /* DB   ED          */
OP(ed,92) { illegal_2(z80);											} /* DB   ED          */
OP(ed,93) { illegal_2(z80);											} /* DB   ED          */
OP(ed,94) { illegal_2(z80);											} /* DB   ED          */
OP(ed,95) { illegal_2(z80);											} /* DB   ED          */
OP(ed,96) { illegal_2(z80);											} /* DB   ED          */
OP(ed,97) { illegal_2(z80);											} /* DB   ED          */

OP(ed,98) { illegal_2(z80);											} /* DB   ED          */
OP(ed,99) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9a) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9b) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9c) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9d) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9e) { illegal_2(z80);											} /* DB   ED          */
OP(ed,9f) { illegal_2(z80);											} /* DB   ED          */

OP(ed,a0) { LDI;													} /* LDI              */
OP(ed,a1) { CPI;													} /* CPI              */
OP(ed,a2) { INI;													} /* INI              */
OP(ed,a3) { OUTI;													} /* OUTI             */
OP(ed,a4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,a5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,a6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,a7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,a8) { LDD;													} /* LDD              */
OP(ed,a9) { CPD;													} /* CPD              */
OP(ed,aa) { IND;													} /* IND              */
OP(ed,ab) { OUTD; 													} /* OUTD             */
OP(ed,ac) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ad) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ae) { illegal_2(z80);											} /* DB   ED          */
OP(ed,af) { illegal_2(z80);											} /* DB   ED          */

OP(ed,b0) { LDIR;													} /* LDIR             */
OP(ed,b1) { CPIR;													} /* CPIR             */
OP(ed,b2) { INIR;													} /* INIR             */
OP(ed,b3) { OTIR;													} /* OTIR             */
OP(ed,b4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,b5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,b6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,b7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,b8) { LDDR;													} /* LDDR             */
OP(ed,b9) { CPDR;													} /* CPDR             */
OP(ed,ba) { INDR;													} /* INDR             */
OP(ed,bb) { OTDR;													} /* OTDR             */
OP(ed,bc) { illegal_2(z80);											} /* DB   ED          */
OP(ed,bd) { illegal_2(z80);											} /* DB   ED          */
OP(ed,be) { illegal_2(z80);											} /* DB   ED          */
OP(ed,bf) { illegal_2(z80);											} /* DB   ED          */

OP(ed,c0) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c1) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c2) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c3) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,c8) { illegal_2(z80);											} /* DB   ED          */
OP(ed,c9) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ca) { illegal_2(z80);											} /* DB   ED          */
OP(ed,cb) { illegal_2(z80);											} /* DB   ED          */
OP(ed,cc) { illegal_2(z80);											} /* DB   ED          */
OP(ed,cd) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ce) { illegal_2(z80);											} /* DB   ED          */
OP(ed,cf) { illegal_2(z80);											} /* DB   ED          */

OP(ed,d0) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d1) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d2) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d3) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,d8) { illegal_2(z80);											} /* DB   ED          */
OP(ed,d9) { illegal_2(z80);											} /* DB   ED          */
OP(ed,da) { illegal_2(z80);											} /* DB   ED          */
OP(ed,db) { illegal_2(z80);											} /* DB   ED          */
OP(ed,dc) { illegal_2(z80);											} /* DB   ED          */
OP(ed,dd) { illegal_2(z80);											} /* DB   ED          */
OP(ed,de) { illegal_2(z80);											} /* DB   ED          */
OP(ed,df) { illegal_2(z80);											} /* DB   ED          */

OP(ed,e0) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e1) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e2) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e3) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,e8) { illegal_2(z80);											} /* DB   ED          */
OP(ed,e9) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ea) { illegal_2(z80);											} /* DB   ED          */
OP(ed,eb) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ec) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ed) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ee) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ef) { illegal_2(z80);											} /* DB   ED          */

OP(ed,f0) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f1) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f2) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f3) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f4) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f5) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f6) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f7) { illegal_2(z80);											} /* DB   ED          */

OP(ed,f8) { illegal_2(z80);											} /* DB   ED          */
OP(ed,f9) { illegal_2(z80);											} /* DB   ED          */
OP(ed,fa) { illegal_2(z80);											} /* DB   ED          */
OP(ed,fb) { illegal_2(z80);											} /* DB   ED          */
OP(ed,fc) { illegal_2(z80);											} /* DB   ED          */
OP(ed,fd) { illegal_2(z80);											} /* DB   ED          */
OP(ed,fe) { illegal_2(z80);											} /* DB   ED          */
OP(ed,ff) { illegal_2(z80);											} /* DB   ED          */


/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {															} /* NOP              */
OP(op,01) { BC = ARG16(z80);										} /* LD   BC,w        */
OP(op,02) { WM(BC,A); MEMPTR_L = (BC + 1) & 0xFF;  MEMPTR_H = A;	} /* LD   (BC),A      */
OP(op,03) { BC++;													} /* INC  BC          */
OP(op,04) { B = INC(z80, B);										} /* INC  B           */
OP(op,05) { B = DEC(z80, B);										} /* DEC  B           */
OP(op,06) { B = ARG(z80);											} /* LD   B,n         */
OP(op,07) { RLCA;													} /* RLCA             */

OP(op,08) { EX_AF;													} /* EX   AF,AF'      */
OP(op,09) { ADD16(hl, bc);											} /* ADD  HL,BC       */
OP(op,0a) { A = RM( BC );	MEMPTR=BC+1;    						} /* LD   A,(BC)      */
OP(op,0b) { BC--; 													} /* DEC  BC          */
OP(op,0c) { C = INC(z80, C);										} /* INC  C           */
OP(op,0d) { C = DEC(z80, C);										} /* DEC  C           */
OP(op,0e) { C = ARG(z80);											} /* LD   C,n         */
OP(op,0f) { RRCA;													} /* RRCA             */

OP(op,10) { B--; JR_COND( B, 0x10 );								} /* DJNZ o           */
OP(op,11) { DE = ARG16(z80);										} /* LD   DE,w        */
OP(op,12) { WM(DE,A); MEMPTR_L = (DE + 1) & 0xFF;  MEMPTR_H = A;	} /* LD   (DE),A      */
OP(op,13) { DE++;													} /* INC  DE          */
OP(op,14) { D = INC(z80, D);										} /* INC  D           */
OP(op,15) { D = DEC(z80, D);										} /* DEC  D           */
OP(op,16) { D = ARG(z80);											} /* LD   D,n         */
OP(op,17) { RLA;													} /* RLA              */

OP(op,18) { JR();													} /* JR   o           */
OP(op,19) { ADD16(hl, de);											} /* ADD  HL,DE       */
OP(op,1a) { A = RM( DE ); MEMPTR=DE+1; 								} /* LD   A,(DE)      */
OP(op,1b) { DE--; 													} /* DEC  DE          */
OP(op,1c) { E = INC(z80, E);										} /* INC  E           */
OP(op,1d) { E = DEC(z80, E);										} /* DEC  E           */
OP(op,1e) { E = ARG(z80);											} /* LD   E,n         */
OP(op,1f) { RRA;													} /* RRA              */

OP(op,20) { JR_COND( !(F & ZF), 0x20 );								} /* JR   NZ,o        */
OP(op,21) { HL = ARG16(z80);										} /* LD   HL,w        */
OP(op,22) { EA = ARG16(z80); WM16( EA, &z80->hl );	MEMPTR = EA+1;	} /* LD   (w),HL      */
OP(op,23) { HL++;													} /* INC  HL          */
OP(op,24) { H = INC(z80, H);										} /* INC  H           */
OP(op,25) { H = DEC(z80, H);										} /* DEC  H           */
OP(op,26) { H = ARG(z80);											} /* LD   H,n         */
OP(op,27) { DAA;													} /* DAA              */

OP(op,28) { JR_COND( F & ZF, 0x28 );								} /* JR   Z,o         */
OP(op,29) { ADD16(hl, hl);											} /* ADD  HL,HL       */
OP(op,2a) { EA = ARG16(z80); RM16( EA, &z80->hl );	MEMPTR = EA+1;	} /* LD   HL,(w)      */
OP(op,2b) { HL--; 													} /* DEC  HL          */
OP(op,2c) { L = INC(z80, L);										} /* INC  L           */
OP(op,2d) { L = DEC(z80, L);										} /* DEC  L           */
OP(op,2e) { L = ARG(z80);											} /* LD   L,n         */
OP(op,2f) { A ^= 0xff; F = (F&(SF|ZF|PF|CF))|HF|NF|(A&(YF|XF));		} /* CPL              */

OP(op,30) { JR_COND( !(F & CF), 0x30 );								} /* JR   NC,o        */
OP(op,31) { SP = ARG16(z80);										} /* LD   SP,w        */
OP(op,32) { EA=ARG16(z80);WM(EA,A);MEMPTR_L=(EA+1)&0xFF;MEMPTR_H=A;	} /* LD   (w),A       */
OP(op,33) { SP++;													} /* INC  SP          */
OP(op,34) { WM( HL, INC(z80, RM(HL)) );								} /* INC  (HL)        */
OP(op,35) { WM( HL, DEC(z80, RM(HL)) );								} /* DEC  (HL)        */
OP(op,36) { WM( HL, ARG(z80) );										} /* LD   (HL),n      */
OP(op,37) { F = (F & (SF|ZF|PF)) | CF | (A & (YF|XF));				} /* SCF              */

OP(op,38) { JR_COND( F & CF, 0x38 );								} /* JR   C,o         */
OP(op,39) { ADD16(hl, sp);											} /* ADD  HL,SP       */
OP(op,3a) { EA = ARG16(z80); A = RM( EA );	MEMPTR=EA+1; 			} /* LD   A,(w)       */
OP(op,3b) { SP--;													} /* DEC  SP          */
OP(op,3c) { A = INC(z80, A);										} /* INC  A           */
OP(op,3d) { A = DEC(z80, A);										} /* DEC  A           */
OP(op,3e) { A = ARG(z80);											} /* LD   A,n         */
OP(op,3f) { F = ((F&(SF|ZF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;		} /* CCF              */

OP(op,40) {															} /* LD   B,B         */
OP(op,41) { B = C;													} /* LD   B,C         */
OP(op,42) { B = D;													} /* LD   B,D         */
OP(op,43) { B = E;													} /* LD   B,E         */
OP(op,44) { B = H;													} /* LD   B,H         */
OP(op,45) { B = L;													} /* LD   B,L         */
OP(op,46) { B = RM(HL);												} /* LD   B,(HL)      */
OP(op,47) { B = A;													} /* LD   B,A         */

OP(op,48) { C = B;													} /* LD   C,B         */
OP(op,49) {															} /* LD   C,C         */
OP(op,4a) { C = D;													} /* LD   C,D         */
OP(op,4b) { C = E;													} /* LD   C,E         */
OP(op,4c) { C = H;													} /* LD   C,H         */
OP(op,4d) { C = L;													} /* LD   C,L         */
OP(op,4e) { C = RM(HL);												} /* LD   C,(HL)      */
OP(op,4f) { C = A;													} /* LD   C,A         */

OP(op,50) { D = B;													} /* LD   D,B         */
OP(op,51) { D = C;													} /* LD   D,C         */
OP(op,52) {															} /* LD   D,D         */
OP(op,53) { D = E;													} /* LD   D,E         */
OP(op,54) { D = H;													} /* LD   D,H         */
OP(op,55) { D = L;													} /* LD   D,L         */
OP(op,56) { D = RM(HL);												} /* LD   D,(HL)      */
OP(op,57) { D = A;													} /* LD   D,A         */

OP(op,58) { E = B;													} /* LD   E,B         */
OP(op,59) { E = C;													} /* LD   E,C         */
OP(op,5a) { E = D;													} /* LD   E,D         */
OP(op,5b) {															} /* LD   E,E         */
OP(op,5c) { E = H;													} /* LD   E,H         */
OP(op,5d) { E = L;													} /* LD   E,L         */
OP(op,5e) { E = RM(HL);												} /* LD   E,(HL)      */
OP(op,5f) { E = A;													} /* LD   E,A         */

OP(op,60) { H = B;													} /* LD   H,B         */
OP(op,61) { H = C;													} /* LD   H,C         */
OP(op,62) { H = D;													} /* LD   H,D         */
OP(op,63) { H = E;													} /* LD   H,E         */
OP(op,64) {															} /* LD   H,H         */
OP(op,65) { H = L;													} /* LD   H,L         */
OP(op,66) { H = RM(HL);												} /* LD   H,(HL)      */
OP(op,67) { H = A;													} /* LD   H,A         */

OP(op,68) { L = B;													} /* LD   L,B         */
OP(op,69) { L = C;													} /* LD   L,C         */
OP(op,6a) { L = D;													} /* LD   L,D         */
OP(op,6b) { L = E;													} /* LD   L,E         */
OP(op,6c) { L = H;													} /* LD   L,H         */
OP(op,6d) {															} /* LD   L,L         */
OP(op,6e) { L = RM(HL);												} /* LD   L,(HL)      */
OP(op,6f) { L = A;													} /* LD   L,A         */

OP(op,70) { WM( HL, B );											} /* LD   (HL),B      */
OP(op,71) { WM( HL, C );											} /* LD   (HL),C      */
OP(op,72) { WM( HL, D );											} /* LD   (HL),D      */
OP(op,73) { WM( HL, E );											} /* LD   (HL),E      */
OP(op,74) { WM( HL, H );											} /* LD   (HL),H      */
OP(op,75) { WM( HL, L );											} /* LD   (HL),L      */
OP(op,76) { ENTER_HALT;												} /* HALT             */
OP(op,77) { WM( HL, A );											} /* LD   (HL),A      */

OP(op,78) { A = B;													} /* LD   A,B         */
OP(op,79) { A = C;													} /* LD   A,C         */
OP(op,7a) { A = D;													} /* LD   A,D         */
OP(op,7b) { A = E;													} /* LD   A,E         */
OP(op,7c) { A = H;													} /* LD   A,H         */
OP(op,7d) { A = L;													} /* LD   A,L         */
OP(op,7e) { A = RM(HL);												} /* LD   A,(HL)      */
OP(op,7f) {															} /* LD   A,A         */

OP(op,80) { ADD(B);													} /* ADD  A,B         */
OP(op,81) { ADD(C);													} /* ADD  A,C         */
OP(op,82) { ADD(D);													} /* ADD  A,D         */
OP(op,83) { ADD(E);													} /* ADD  A,E         */
OP(op,84) { ADD(H);													} /* ADD  A,H         */
OP(op,85) { ADD(L);													} /* ADD  A,L         */
OP(op,86) { ADD(RM(HL));											} /* ADD  A,(HL)      */
OP(op,87) { ADD(A);													} /* ADD  A,A         */

OP(op,88) { ADC(B);													} /* ADC  A,B         */
OP(op,89) { ADC(C);													} /* ADC  A,C         */
OP(op,8a) { ADC(D);													} /* ADC  A,D         */
OP(op,8b) { ADC(E);													} /* ADC  A,E         */
OP(op,8c) { ADC(H);													} /* ADC  A,H         */
OP(op,8d) { ADC(L);													} /* ADC  A,L         */
OP(op,8e) { ADC(RM(HL));											} /* ADC  A,(HL)      */
OP(op,8f) { ADC(A);													} /* ADC  A,A         */

OP(op,90) { SUB(B);													} /* SUB  B           */
OP(op,91) { SUB(C);													} /* SUB  C           */
OP(op,92) { SUB(D);													} /* SUB  D           */
OP(op,93) { SUB(E);													} /* SUB  E           */
OP(op,94) { SUB(H);													} /* SUB  H           */
OP(op,95) { SUB(L);													} /* SUB  L           */
OP(op,96) { SUB(RM(HL));											} /* SUB  (HL)        */
OP(op,97) { SUB(A);													} /* SUB  A           */

OP(op,98) { SBC(B);													} /* SBC  A,B         */
OP(op,99) { SBC(C);													} /* SBC  A,C         */
OP(op,9a) { SBC(D);													} /* SBC  A,D         */
OP(op,9b) { SBC(E);													} /* SBC  A,E         */
OP(op,9c) { SBC(H);													} /* SBC  A,H         */
OP(op,9d) { SBC(L);													} /* SBC  A,L         */
OP(op,9e) { SBC(RM(HL));											} /* SBC  A,(HL)      */
OP(op,9f) { SBC(A);													} /* SBC  A,A         */

OP(op,a0) { AND(B);													} /* AND  B           */
OP(op,a1) { AND(C);													} /* AND  C           */
OP(op,a2) { AND(D);													} /* AND  D           */
OP(op,a3) { AND(E);													} /* AND  E           */
OP(op,a4) { AND(H);													} /* AND  H           */
OP(op,a5) { AND(L);													} /* AND  L           */
OP(op,a6) { AND(RM(HL));											} /* AND  (HL)        */
OP(op,a7) { AND(A);													} /* AND  A           */

OP(op,a8) { XOR(B);													} /* XOR  B           */
OP(op,a9) { XOR(C);													} /* XOR  C           */
OP(op,aa) { XOR(D);													} /* XOR  D           */
OP(op,ab) { XOR(E);													} /* XOR  E           */
OP(op,ac) { XOR(H);													} /* XOR  H           */
OP(op,ad) { XOR(L);													} /* XOR  L           */
OP(op,ae) { XOR(RM(HL));											} /* XOR  (HL)        */
OP(op,af) { XOR(A);													} /* XOR  A           */

OP(op,b0) { OR(B);													} /* OR   B           */
OP(op,b1) { OR(C);													} /* OR   C           */
OP(op,b2) { OR(D);													} /* OR   D           */
OP(op,b3) { OR(E);													} /* OR   E           */
OP(op,b4) { OR(H);													} /* OR   H           */
OP(op,b5) { OR(L);													} /* OR   L           */
OP(op,b6) { OR(RM(HL));												} /* OR   (HL)        */
OP(op,b7) { OR(A);													} /* OR   A           */

OP(op,b8) { CP(B);													} /* CP   B           */
OP(op,b9) { CP(C);													} /* CP   C           */
OP(op,ba) { CP(D);													} /* CP   D           */
OP(op,bb) { CP(E);													} /* CP   E           */
OP(op,bc) { CP(H);													} /* CP   H           */
OP(op,bd) { CP(L);													} /* CP   L           */
OP(op,be) { CP(RM(HL));												} /* CP   (HL)        */
OP(op,bf) { CP(A);													} /* CP   A           */

OP(op,c0) { RET_COND( !(F & ZF), 0xc0 );							} /* RET  NZ          */
OP(op,c1) { POP( bc );												} /* POP  BC          */
OP(op,c2) { JP_COND( !(F & ZF) );									} /* JP   NZ,a        */
OP(op,c3) { JP;														} /* JP   a           */
OP(op,c4) { CALL_COND( !(F & ZF), 0xc4 );							} /* CALL NZ,a        */
OP(op,c5) { PUSH( bc );												} /* PUSH BC          */
OP(op,c6) { ADD(ARG(z80));											} /* ADD  A,n         */
OP(op,c7) { RST(0x00);												} /* RST  0           */

OP(op,c8) { RET_COND( F & ZF, 0xc8 );								} /* RET  Z           */
OP(op,c9) { POP( pc ); change_pc(PCD); MEMPTR=PCD;					} /* RET              */
OP(op,ca) { JP_COND( F & ZF );										} /* JP   Z,a         */
OP(op,cb) { R++; EXEC(z80,cb,ROP(z80));								} /* **** CB xx       */
OP(op,cc) { CALL_COND( F & ZF, 0xcc );								} /* CALL Z,a         */
OP(op,cd) { CALL();													} /* CALL a           */
OP(op,ce) { ADC(ARG(z80));											} /* ADC  A,n         */
OP(op,cf) { RST(0x08);												} /* RST  1           */

OP(op,d0) { RET_COND( !(F & CF), 0xd0 );							} /* RET  NC          */
OP(op,d1) { POP( de );												} /* POP  DE          */
OP(op,d2) { JP_COND( !(F & CF) );									} /* JP   NC,a        */
OP(op,d3) { unsigned n = ARG(z80) | (A << 8); OUT( n, A );	MEMPTR_L = ((n & 0xff) + 1) & 0xff;  MEMPTR_H = A;	} /* OUT  (n),A       */
OP(op,d4) { CALL_COND( !(F & CF), 0xd4 );							} /* CALL NC,a        */
OP(op,d5) { PUSH( de );												} /* PUSH DE          */
OP(op,d6) { SUB(ARG(z80));											} /* SUB  n           */
OP(op,d7) { RST(0x10);												} /* RST  2           */

OP(op,d8) { RET_COND( F & CF, 0xd8 );								} /* RET  C           */
OP(op,d9) { EXX;													} /* EXX              */
OP(op,da) { JP_COND( F & CF );										} /* JP   C,a         */
OP(op,db) { unsigned n = ARG(z80) | (A << 8); A = IN( n );	MEMPTR = n + 1;	} /* IN   A,(n)       */
OP(op,dc) { CALL_COND( F & CF, 0xdc );								} /* CALL C,a         */
OP(op,dd) { R++; EXEC(z80,dd,ROP(z80));								} /* **** DD xx       */
OP(op,de) { SBC(ARG(z80));											} /* SBC  A,n         */
OP(op,df) { RST(0x18);												} /* RST  3           */

OP(op,e0) { RET_COND( !(F & PF), 0xe0 );							} /* RET  PO          */
OP(op,e1) { POP( hl );												} /* POP  HL          */
OP(op,e2) { JP_COND( !(F & PF) );									} /* JP   PO,a        */
OP(op,e3) { EXSP( hl );												} /* EX   HL,(SP)     */
OP(op,e4) { CALL_COND( !(F & PF), 0xe4 );							} /* CALL PO,a        */
OP(op,e5) { PUSH( hl );												} /* PUSH HL          */
OP(op,e6) { AND(ARG(z80));											} /* AND  n           */
OP(op,e7) { RST(0x20);												} /* RST  4           */

OP(op,e8) { RET_COND( F & PF, 0xe8 );								} /* RET  PE          */
OP(op,e9) { PC = HL; change_pc(PCD);								} /* JP   (HL)        */
OP(op,ea) { JP_COND( F & PF );										} /* JP   PE,a        */
OP(op,eb) { EX_DE_HL;												} /* EX   DE,HL       */
OP(op,ec) { CALL_COND( F & PF, 0xec );								} /* CALL PE,a        */
OP(op,ed) { R++; EXEC(z80,ed,ROP(z80));								} /* **** ED xx       */
OP(op,ee) { XOR(ARG(z80));											} /* XOR  n           */
OP(op,ef) { RST(0x28);												} /* RST  5           */

OP(op,f0) { RET_COND( !(F & SF), 0xf0 );							} /* RET  P           */
OP(op,f1) { POP( af );												} /* POP  AF          */
OP(op,f2) { JP_COND( !(F & SF) );									} /* JP   P,a         */
OP(op,f3) { IFF1 = IFF2 = 0;										} /* DI               */
OP(op,f4) { CALL_COND( !(F & SF), 0xf4 );							} /* CALL P,a         */
OP(op,f5) { PUSH( af );												} /* PUSH AF          */
OP(op,f6) { OR(ARG(z80));											} /* OR   n           */
OP(op,f7) { RST(0x30);												} /* RST  6           */

OP(op,f8) { RET_COND( F & SF, 0xf8 );								} /* RET  M           */
OP(op,f9) { SP = HL;												} /* LD   SP,HL       */
OP(op,fa) { JP_COND(F & SF);										} /* JP   M,a         */
OP(op,fb) { EI;														} /* EI               */
OP(op,fc) { CALL_COND( F & SF, 0xfc );								} /* CALL M,a         */
OP(op,fd) { R++; EXEC(z80,fd,ROP(z80));								} /* **** FD xx       */
OP(op,fe) { CP(ARG(z80));											} /* CP   n           */
OP(op,ff) { RST(0x38);												} /* RST  7           */


static void take_interrupt(z80_state *z80)
{
	int irq_vector;

	/* there isn't a valid previous program counter */
	PRVPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT;

	/* Clear both interrupt flip flops */
	IFF1 = IFF2 = 0;

	/* Daisy chain mode? If so, call the requesting device */
	if (z80->daisy)
		irq_vector = z80daisy_call_ack_device(z80->daisy);

	/* else call back the cpu interface to retrieve the vector */
	else
		irq_vector = (*z80->irq_callback)(z80->device, 0);

	LOG(("Z80 #%d single int. irq_vector $%02x\n", cpu_getactivecpu(), irq_vector));

	/* Interrupt mode 2. Call [z80->i:databyte] */
	if( IM == 2 )
	{
		irq_vector = (irq_vector & 0xff) | (I << 8);
		PUSH( pc );
		RM16( irq_vector, &z80->pc );
		LOG(("Z80 #%d IM2 [$%04x] = $%04x\n",cpu_getactivecpu() , irq_vector, PCD));
		/* CALL opcode timing */
		z80->icount -= cc[Z80_TABLE_op][0xcd];
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( IM == 1 )
	{
		LOG(("Z80 #%d IM1 $0038\n",cpu_getactivecpu() ));
		PUSH( pc );
		PCD = 0x0038;
		/* RST $38 + 'interrupt latency' cycles */
		z80->icount -= cc[Z80_TABLE_op][0xff] + cc[Z80_TABLE_ex][0xff];
	}
	else
	{
		/* Interrupt mode 0. We check for CALL and JP instructions, */
		/* if neither of these were found we assume a 1 byte opcode */
		/* was placed on the databus                                */
		LOG(("Z80 #%d IM0 $%04x\n",cpu_getactivecpu() , irq_vector));
		switch (irq_vector & 0xff0000)
		{
			case 0xcd0000:	/* call */
				PUSH( pc );
				PCD = irq_vector & 0xffff;
				 /* CALL $xxxx + 'interrupt latency' cycles */
				z80->icount -= cc[Z80_TABLE_op][0xcd] + cc[Z80_TABLE_ex][0xff];
				break;
			case 0xc30000:	/* jump */
				PCD = irq_vector & 0xffff;
				/* JP $xxxx + 2 cycles */
				z80->icount -= cc[Z80_TABLE_op][0xc3] + cc[Z80_TABLE_ex][0xff];
				break;
			default:		/* rst (or other opcodes?) */
				PUSH( pc );
				PCD = irq_vector & 0x0038;
				/* RST $xx + 2 cycles */
				z80->icount -= cc[Z80_TABLE_op][PCD] + cc[Z80_TABLE_ex][PCD];
				break;
		}
	}
	change_pc(PCD);
	MEMPTR=PCD;
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
static CPU_INIT( z80 )
{
	z80_state *z80 = device->token;
	int i, p;
	
	token = device->token;	// temporary
	
	/* setup cycle tables */
	cc[Z80_TABLE_op] = cc_op;
	cc[Z80_TABLE_cb] = cc_cb;
	cc[Z80_TABLE_ed] = cc_ed;
	cc[Z80_TABLE_xy] = cc_xy;
	cc[Z80_TABLE_xycb] = cc_xycb;
	cc[Z80_TABLE_ex] = cc_ex;

#if BIG_FLAGS_ARRAY
	if( !SZHVC_add || !SZHVC_sub )
	{
		int oldval, newval, val;
		UINT8 *padd, *padc, *psub, *psbc;
		/* allocate big flag arrays once */
		SZHVC_add = (UINT8 *)malloc(2*256*256);
		SZHVC_sub = (UINT8 *)malloc(2*256*256);
		if( !SZHVC_add || !SZHVC_sub )
		{
			fatalerror("Z80: failed to allocate 2 * 128K flags arrays!!!");
		}
		padd = &SZHVC_add[	0*256];
		padc = &SZHVC_add[256*256];
		psub = &SZHVC_sub[	0*256];
		psbc = &SZHVC_sub[256*256];
		for (oldval = 0; oldval < 256; oldval++)
		{
			for (newval = 0; newval < 256; newval++)
			{
				/* add or adc w/o carry set */
				val = newval - oldval;
				*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
				if( newval < oldval ) *padd |= CF;
				if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
				padd++;

				/* adc with carry set */
				val = newval - oldval - 1;
				*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
				if( newval <= oldval ) *padc |= CF;
				if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
				padc++;

				/* cp, sub or sbc w/o carry set */
				val = oldval - newval;
				*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
				if( newval > oldval ) *psub |= CF;
				if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
				psub++;

				/* sbc with carry set */
				val = oldval - newval - 1;
				*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
				if( (newval & 0x0f) >= (oldval & 0x0f) ) *psbc |= HF;
				if( newval >= oldval ) *psbc |= CF;
				if( (val^oldval) & (oldval^newval) & 0x80 ) *psbc |= VF;
				psbc++;
			}
		}
	}
#endif
	for (i = 0; i < 256; i++)
	{
		p = 0;
		if( i&0x01 ) ++p;
		if( i&0x02 ) ++p;
		if( i&0x04 ) ++p;
		if( i&0x08 ) ++p;
		if( i&0x10 ) ++p;
		if( i&0x20 ) ++p;
		if( i&0x40 ) ++p;
		if( i&0x80 ) ++p;
		SZ[i] = i ? i & SF : ZF;
		SZ[i] |= (i & (YF | XF));		/* undocumented flag bits 5+3 */
		SZ_BIT[i] = i ? i & SF : ZF | PF;
		SZ_BIT[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
		SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
		SZHV_inc[i] = SZ[i];
		if( i == 0x80 ) SZHV_inc[i] |= VF;
		if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
		SZHV_dec[i] = SZ[i] | NF;
		if( i == 0x7f ) SZHV_dec[i] |= VF;
		if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
	}

	state_save_register_item("z80", index, z80->prvpc.w.l);
	state_save_register_item("z80", index, z80->pc.w.l);
	state_save_register_item("z80", index, z80->sp.w.l);
	state_save_register_item("z80", index, z80->af.w.l);
	state_save_register_item("z80", index, z80->bc.w.l);
	state_save_register_item("z80", index, z80->de.w.l);
	state_save_register_item("z80", index, z80->hl.w.l);
	state_save_register_item("z80", index, z80->ix.w.l);
	state_save_register_item("z80", index, z80->iy.w.l);
	state_save_register_item("z80", index, z80->memptr.w.l);
	state_save_register_item("z80", index, z80->af2.w.l);
	state_save_register_item("z80", index, z80->bc2.w.l);
	state_save_register_item("z80", index, z80->de2.w.l);
	state_save_register_item("z80", index, z80->hl2.w.l);
	state_save_register_item("z80", index, z80->r);
	state_save_register_item("z80", index, z80->r2);
	state_save_register_item("z80", index, z80->iff1);
	state_save_register_item("z80", index, z80->iff2);
	state_save_register_item("z80", index, z80->halt);
	state_save_register_item("z80", index, z80->im);
	state_save_register_item("z80", index, z80->i);
	state_save_register_item("z80", index, z80->nmi_state);
	state_save_register_item("z80", index, z80->nmi_pending);
	state_save_register_item("z80", index, z80->irq_state);
	state_save_register_item("z80", index, z80->after_ei);

	/* Reset registers to their initial values */
	memset(z80, 0, sizeof(*z80));
	if (config != NULL)
		z80->daisy = z80daisy_init(Machine, Machine->config->cpu[cpu_getactivecpu()].tag, config);
	z80->irq_callback = irqcallback;
	z80->device = device;
	IX = IY = 0xffff; /* IX and IY are FFFF after a reset! */
	F = ZF;			/* Zero flag is set */
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
static CPU_RESET( z80 )
{
	z80_state *z80 = device->token;
	
	PC = 0x0000;
	I = 0;
	R = 0;
	R2 = 0;
	z80->nmi_state = CLEAR_LINE;
	z80->nmi_pending = FALSE;
	z80->irq_state = CLEAR_LINE;
	z80->after_ei = FALSE;

	if (z80->daisy)
		z80daisy_reset(z80->daisy);

	change_pc(PCD);
	MEMPTR=PCD;
}

static CPU_EXIT( z80 )
{
#if BIG_FLAGS_ARRAY
	if (SZHVC_add) free(SZHVC_add);
	SZHVC_add = NULL;
	if (SZHVC_sub) free(SZHVC_sub);
	SZHVC_sub = NULL;
#endif
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
static CPU_EXECUTE( z80 )
{
	z80_state *z80 = device->token;
	
	z80->icount = cycles;

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (z80->nmi_pending)
	{
		LOG(("Z80 #%d take NMI\n", cpu_getactivecpu()));
		PRVPC = -1;			/* there isn't a valid previous program counter */
		LEAVE_HALT;			/* Check if processor was halted */

		IFF1 = 0;
		PUSH( pc );
		PCD = 0x0066;
		change_pc(PCD);
		MEMPTR=PCD;
		z80->icount -= 11;
		z80->nmi_pending = FALSE;
	}

	do
	{
		/* check for IRQs before each instruction */
		if (z80->irq_state != CLEAR_LINE && IFF1 && !z80->after_ei)
			take_interrupt(z80);
		z80->after_ei = FALSE;

		PRVPC = PCD;
		debugger_instruction_hook(Machine, PCD);
		R++;
		EXEC_INLINE(z80,op,ROP(z80));
	} while( z80->icount > 0 );

	return cycles - z80->icount;
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
static void z80_burn(int cycles)
{
	z80_state *z80 = token;
	
	if( cycles > 0 )
	{
		/* NOP takes 4 cycles per instruction */
		int n = (cycles + 3) / 4;
		R += n;
		z80->icount -= 4 * n;
	}
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
static void z80_get_context (void *dst)
{
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static void z80_set_context (void *src)
{
	z80_state *z80;
	if( src )
		token = src;
	z80 = token;
	change_pc(PCD);	
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(z80_state *z80, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* mark an NMI pending on the rising edge */
		if (z80->nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			z80->nmi_pending = TRUE;
		z80->nmi_state = state;
	}
	else
	{
		/* update the IRQ state via the daisy chain */
		z80->irq_state = state;
		if (z80->daisy)
			z80->irq_state = z80daisy_update_irq_state(z80->daisy);

		/* the main execute loop will take the interrupt */
	}
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void z80_set_info(UINT32 state, cpuinfo *info)
{
	z80_state *z80 = token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:		set_irq_line(z80, INPUT_LINE_NMI, info->i); break;
		case CPUINFO_INT_INPUT_STATE + 0:					set_irq_line(z80, 0, info->i);		break;

		case CPUINFO_INT_PC:								PC = info->i; change_pc(PCD);		break;
		case CPUINFO_INT_REGISTER + Z80_PC:					z80->pc.w.l = info->i;				break;
		case CPUINFO_INT_SP:								SP = info->i;						break;
		case CPUINFO_INT_REGISTER + Z80_SP:					z80->sp.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_A:					z80->af.b.h = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_B:					z80->bc.b.h = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_C:					z80->bc.b.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_D:					z80->de.b.h = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_E:					z80->de.b.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_H:					z80->hl.b.h = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_L:					z80->hl.b.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_AF:					z80->af.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_BC:					z80->bc.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_DE:					z80->de.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_HL:					z80->hl.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_IX:					z80->ix.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_IY:					z80->iy.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_MEMPTR:				z80->memptr.w.l = info->i;			break;
		case CPUINFO_INT_REGISTER + Z80_R:					z80->r = info->i; z80->r2 = info->i & 0x80; break;
		case CPUINFO_INT_REGISTER + Z80_I:					z80->i = info->i;					break;
		case CPUINFO_INT_REGISTER + Z80_AF2:				z80->af2.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_BC2:				z80->bc2.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_DE2:				z80->de2.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_HL2:				z80->hl2.w.l = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_IM:					z80->im = info->i;					break;
		case CPUINFO_INT_REGISTER + Z80_IFF1:				z80->iff1 = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_IFF2:				z80->iff2 = info->i;				break;
		case CPUINFO_INT_REGISTER + Z80_HALT:				z80->halt = info->i;				break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_op:	cc[Z80_TABLE_op] = info->p;			break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_cb:	cc[Z80_TABLE_cb] = info->p;			break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ed:	cc[Z80_TABLE_ed] = info->p;			break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_xy:	cc[Z80_TABLE_xy] = info->p;			break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_xycb:	cc[Z80_TABLE_xycb] = info->p;		break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ex:	cc[Z80_TABLE_ex] = info->p;			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void z80_get_info(UINT32 state, cpuinfo *info)
{
	z80_state *z80 = token;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:				info->i = sizeof(z80_state);				break;
		case CPUINFO_INT_INPUT_LINES:				info->i = 1;								break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:		info->i = 0xff;								break;
		case CPUINFO_INT_ENDIANNESS:				info->i = CPU_IS_LE;						break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:			info->i = 1;								break;
		case CPUINFO_INT_CLOCK_DIVIDER:				info->i = 1;								break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:		info->i = 1;								break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:		info->i = 4;								break;
		case CPUINFO_INT_MIN_CYCLES:				info->i = 2;								break;
		case CPUINFO_INT_MAX_CYCLES:				info->i = 16;								break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = z80->nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + 0:			info->i = z80->irq_state;					break;

		case CPUINFO_INT_PREVIOUSPC:				info->i = z80->prvpc.w.l;					break;

		case CPUINFO_INT_PC:						info->i = PCD;								break;
		case CPUINFO_INT_REGISTER + Z80_PC:			info->i = z80->pc.w.l;						break;
		case CPUINFO_INT_SP:						info->i = SPD;								break;
		case CPUINFO_INT_REGISTER + Z80_SP:			info->i = z80->sp.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_A:			info->i = z80->af.b.h;						break;
		case CPUINFO_INT_REGISTER + Z80_B:			info->i = z80->bc.b.h;						break;
		case CPUINFO_INT_REGISTER + Z80_C:			info->i = z80->bc.b.l;						break;
		case CPUINFO_INT_REGISTER + Z80_D:			info->i = z80->de.b.h;						break;
		case CPUINFO_INT_REGISTER + Z80_E:			info->i = z80->de.b.l;						break;
		case CPUINFO_INT_REGISTER + Z80_H:			info->i = z80->hl.b.h;						break;
		case CPUINFO_INT_REGISTER + Z80_L:			info->i = z80->hl.b.l;						break;
		case CPUINFO_INT_REGISTER + Z80_AF:			info->i = z80->af.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_BC:			info->i = z80->bc.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_DE:			info->i = z80->de.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_HL:			info->i = z80->hl.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_IX:			info->i = z80->ix.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_IY:			info->i = z80->iy.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_MEMPTR:		info->i = z80->memptr.w.l;					break;
		case CPUINFO_INT_REGISTER + Z80_R:			info->i = (z80->r & 0x7f) | (z80->r2 & 0x80);break;
		case CPUINFO_INT_REGISTER + Z80_I:			info->i = z80->i;							break;
		case CPUINFO_INT_REGISTER + Z80_AF2:		info->i = z80->af2.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_BC2:		info->i = z80->bc2.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_DE2:		info->i = z80->de2.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_HL2:		info->i = z80->hl2.w.l;						break;
		case CPUINFO_INT_REGISTER + Z80_IM:			info->i = z80->im;							break;
		case CPUINFO_INT_REGISTER + Z80_IFF1:		info->i = z80->iff1;						break;
		case CPUINFO_INT_REGISTER + Z80_IFF2:		info->i = z80->iff2;						break;
		case CPUINFO_INT_REGISTER + Z80_HALT:		info->i = z80->halt;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:					info->setinfo = z80_set_info;				break;
		case CPUINFO_PTR_GET_CONTEXT:				info->getcontext = z80_get_context;			break;
		case CPUINFO_PTR_SET_CONTEXT:				info->setcontext = z80_set_context;			break;
		case CPUINFO_PTR_INIT:						info->init = CPU_INIT_NAME(z80);						break;
		case CPUINFO_PTR_RESET:						info->reset = CPU_RESET_NAME(z80);					break;
		case CPUINFO_PTR_EXIT:						info->exit = CPU_EXIT_NAME(z80);						break;
		case CPUINFO_PTR_EXECUTE:					info->execute = CPU_EXECUTE_NAME(z80);				break;
		case CPUINFO_PTR_BURN:						info->burn = NULL;							break;
		case CPUINFO_PTR_DISASSEMBLE:				info->disassemble = z80_dasm;				break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:				info->icount = &z80->icount;		break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_op:	info->p = (void *)cc[Z80_TABLE_op];	break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_cb:	info->p = (void *)cc[Z80_TABLE_cb];	break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ed:	info->p = (void *)cc[Z80_TABLE_ed];	break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_xy:	info->p = (void *)cc[Z80_TABLE_xy];	break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_xycb:	info->p = (void *)cc[Z80_TABLE_xycb]; break;
		case CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ex:	info->p = (void *)cc[Z80_TABLE_ex];	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "Z80");						break;
		case CPUINFO_STR_CORE_FAMILY:				strcpy(info->s, "Zilog Z80");				break;
		case CPUINFO_STR_CORE_VERSION:				strcpy(info->s, "3.8");						break;
		case CPUINFO_STR_CORE_FILE:					strcpy(info->s, __FILE__);					break;
		case CPUINFO_STR_CORE_CREDITS:				strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				z80->af.b.l & 0x80 ? 'S':'.',
				z80->af.b.l & 0x40 ? 'Z':'.',
				z80->af.b.l & 0x20 ? '5':'.',
				z80->af.b.l & 0x10 ? 'H':'.',
				z80->af.b.l & 0x08 ? '3':'.',
				z80->af.b.l & 0x04 ? 'P':'.',
				z80->af.b.l & 0x02 ? 'N':'.',
				z80->af.b.l & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + Z80_PC:			sprintf(info->s, "PC:%04X", z80->pc.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_SP:			sprintf(info->s, "SP:%04X", z80->sp.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_A:			sprintf(info->s, "~A:%02X", z80->af.b.h);	break;
		case CPUINFO_STR_REGISTER + Z80_B:			sprintf(info->s, "~B:%02X", z80->bc.b.h);	break;
		case CPUINFO_STR_REGISTER + Z80_C:			sprintf(info->s, "~C:%02X", z80->bc.b.l);	break;
		case CPUINFO_STR_REGISTER + Z80_D:			sprintf(info->s, "~D:%02X", z80->de.b.h);	break;
		case CPUINFO_STR_REGISTER + Z80_E:			sprintf(info->s, "~E:%02X", z80->de.b.l);	break;
		case CPUINFO_STR_REGISTER + Z80_H:			sprintf(info->s, "~H:%02X", z80->hl.b.h);	break;
		case CPUINFO_STR_REGISTER + Z80_L:			sprintf(info->s, "~L:%02X", z80->hl.b.l);	break;
		case CPUINFO_STR_REGISTER + Z80_AF:			sprintf(info->s, "AF:%04X", z80->af.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_BC:			sprintf(info->s, "BC:%04X", z80->bc.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_DE:			sprintf(info->s, "DE:%04X", z80->de.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_HL:			sprintf(info->s, "HL:%04X", z80->hl.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_IX:			sprintf(info->s, "IX:%04X", z80->ix.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_IY:			sprintf(info->s, "IY:%04X", z80->iy.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_MEMPTR:		sprintf(info->s, "MEMPTR:%04X", z80->memptr.w.l);  break;
		case CPUINFO_STR_REGISTER + Z80_R:			sprintf(info->s, "R:%02X", (z80->r & 0x7f) | (z80->r2 & 0x80)); break;
		case CPUINFO_STR_REGISTER + Z80_I:			sprintf(info->s, "I:%02X", z80->i);			break;
		case CPUINFO_STR_REGISTER + Z80_AF2:		sprintf(info->s, "AF2:%04X", z80->af2.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_BC2:		sprintf(info->s, "BC2:%04X", z80->bc2.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_DE2:		sprintf(info->s, "DE2:%04X", z80->de2.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_HL2:		sprintf(info->s, "HL2:%04X", z80->hl2.w.l);	break;
		case CPUINFO_STR_REGISTER + Z80_IM:			sprintf(info->s, "IM:%X", z80->im);			break;
		case CPUINFO_STR_REGISTER + Z80_IFF1:		sprintf(info->s, "IFF1:%X", z80->iff1);		break;
		case CPUINFO_STR_REGISTER + Z80_IFF2:		sprintf(info->s, "IFF2:%X", z80->iff2);		break;
		case CPUINFO_STR_REGISTER + Z80_HALT:		sprintf(info->s, "HALT:%X", z80->halt);		break;
	}
}
