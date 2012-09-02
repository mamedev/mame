/*****************************************************************************
 *
 *   z80.c
 *   Portable Z80 emulator V3.9
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
 *   TODO:
 *    - Interrupt mode 0 should be able to execute arbitrary opcodes
 *    - If LD A,I or LD A,R is interrupted, P/V flag gets reset, even if IFF2
 *      was set before this instruction (implemented, but not enabled: we need
 *      document Z80 types first, see below)
 *    - Ideally, the tiny differences between Z80 types should be supported,
 *      currently known differences:
 *       - LD A,I/R P/V flag reset glitch is fixed on CMOS Z80
 *       - OUT (C),0 outputs 0 on NMOS Z80, $FF on CMOS Z80
 *       - SCF/CCF X/Y flags is ((flags | A) & 0x28) on SGS/SHARP/ZiLOG NMOS Z80,
 *         (flags & A & 0x28) on NEC NMOS Z80, other models unknown.
 *         However, people from the Speccy scene mention that SCF/CCF X/Y results
 *         are inconsistant and may be influenced by I and R registers.
 *      This Z80 emulator assumes a ZiLOG NMOS model.
 *
 *   Changes in 3.9:
 *    - Fixed cycle counts for LD IYL/IXL/IYH/IXH,n [Marshmellow]
 *    - Fixed X/Y flags in CCF/SCF/BIT, ZEXALL is happy now [hap]
 *    - Simplified DAA, renamed MEMPTR (3.8) to WZ, added TODO [hap]
 *    - Fixed IM2 interrupt cycles [eke]
 *   Changes in 3.8 [Miodrag Milanovic]:
 *    - Added MEMPTR register (according to informations provided
 *      by Vladimir Kladov
 *    - BIT n,(HL) now return valid values due to use of MEMPTR
 *    - Fixed BIT 6,(XY+o) undocumented instructions
 *   Changes in 3.7 [Aaron Giles]:
 *    - Changed NMI handling. NMIs are now latched in set_irq_state
 *      but are not taken there. Instead they are taken at the start of the
 *      execute loop.
 *    - Changed IRQ handling. IRQ state is set in set_irq_state but not taken
 *      except during the inner execute loop.
 *    - Removed x86 assembly hacks and obsolete timing loop catchers.
 *   Changes in 3.6:
 *    - Got rid of the code that would inexactly emulate a Z80, i.e. removed
 *      all the #if Z80_EXACT #else branches.
 *    - Removed leading underscores from local register name shortcuts as
 *      this violates the C99 standard.
 *    - Renamed the registers inside the Z80 context to lower case to avoid
 *      ambiguities (shortcuts would have had the same names as the fields
 *      of the structure).
 *   Changes in 3.5:
 *    - Implemented OTIR, INIR, etc. without look-up table for PF flag.
 *      [Ramsoft, Sean Young]
 *   Changes in 3.4:
 *    - Removed Z80-MSX specific code as it's not needed any more.
 *    - Implemented DAA without look-up table [Ramsoft, Sean Young]
 *   Changes in 3.3:
 *    - Fixed undocumented flags XF & YF in the non-asm versions of CP,
 *      and all the 16 bit arithmetic instructions. [Sean Young]
 *   Changes in 3.2:
 *    - Fixed undocumented flags XF & YF of RRCA, and CF and HF of
 *      INI/IND/OUTI/OUTD/INIR/INDR/OTIR/OTDR [Sean Young]
 *   Changes in 3.1:
 *    - removed the REPEAT_AT_ONCE execution of LDIR/CPIR etc. opcodes
 *      for readabilities sake and because the implementation was buggy
 *      (and i was not able to find the difference)
 *   Changes in 3.0:
 *    - 'finished' switch to dynamically overrideable cycle count tables
 *   Changes in 2.9:
 *    - added methods to access and override the cycle count tables
 *    - fixed handling and timing of multiple DD/FD prefixed opcodes
 *   Changes in 2.8:
 *    - OUTI/OUTD/OTIR/OTDR also pre-decrement the B register now.
 *      This was wrong because of a bug fix on the wrong side
 *      (astrocade sound driver).
 *   Changes in 2.7:
 *    - removed z80_vm specific code, it's not needed (and never was).
 *   Changes in 2.6:
 *    - BUSY_LOOP_HACKS needed to call change_pc() earlier, before
 *      checking the opcodes at the new address, because otherwise they
 *      might access the old (wrong or even NULL) banked memory region.
 *      Thanks to Sean Young for finding this nasty bug.
 *   Changes in 2.5:
 *    - Burning cycles always adjusts the ICount by a multiple of 4.
 *    - In REPEAT_AT_ONCE cases the r register wasn't incremented twice
 *      per repetition as it should have been. Those repeated opcodes
 *      could also underflow the ICount.
 *    - Simplified TIME_LOOP_HACKS for BC and added two more for DE + HL
 *      timing loops. i think those hacks weren't endian safe before too.
 *   Changes in 2.4:
 *    - z80_reset zaps the entire context, sets IX and IY to 0xffff(!) and
 *      sets the Z flag. With these changes the Tehkan World Cup driver
 *      _seems_ to work again.
 *   Changes in 2.3:
 *    - External termination of the execution loop calls z80_burn() and
 *      z80_vm_burn() to burn an amount of cycles (r adjustment)
 *    - Shortcuts which burn CPU cycles (BUSY_LOOP_HACKS and TIME_LOOP_HACKS)
 *      now also adjust the r register depending on the skipped opcodes.
 *   Changes in 2.2:
 *    - Fixed bugs in CPL, SCF and CCF instructions flag handling.
 *    - Changed variable ea and ARG16() function to UINT32; this
 *      produces slightly more efficient code.
 *    - The DD/FD XY CB opcodes where XY is 40-7F and Y is not 6/E
 *      are changed to calls to the X6/XE opcodes to reduce object size.
 *      They're hardly ever used so this should not yield a speed penalty.
 *   New in 2.0:
 *    - Optional more exact Z80 emulation (#define Z80_EXACT 1) according
 *      to a detailed description by Sean Young which can be found at:
 *      http://www.msxnet.org/tech/z80-documented.pdf
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "z80.h"
#include "z80daisy.h"

#define VERBOSE				0

/* On an NMOS Z80, if LD A,I or LD A,R is interrupted, P/V flag gets reset,
   even if IFF2 was set before this instruction. This issue was fixed on
   the CMOS Z80, so until knowing (most) Z80 types on hardware, it's disabled */
#define HAS_LDAIR_QUIRK		0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* execute main opcodes inside a big switch statement */
#ifndef BIG_SWITCH
#define BIG_SWITCH			1
#endif


/****************************************************************************/
/* The Z80 registers. halt is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(r&127)|(r2&128)    */
/****************************************************************************/
typedef struct _z80_state z80_state;
struct _z80_state
{
	PAIR			prvpc,pc,sp,af,bc,de,hl,ix,iy,wz;
	PAIR			af2,bc2,de2,hl2;
	UINT8			r,r2,iff1,iff2,halt,im,i;
	UINT8			nmi_state;			/* nmi line state */
	UINT8			nmi_pending;		/* nmi pending */
	UINT8			irq_state;			/* irq line state */
	UINT8			nsc800_irq_state[4];/* state of NSC800 restart interrupts A, B, C */
	int				wait_state;			// wait line state
	int				busrq_state;		// bus request line state
	UINT8			after_ei;			/* are we in the EI shadow? */
	UINT8			after_ldair;		/* same, but for LD A,I or LD A,R */
	UINT32			ea;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int				icount;
	z80_daisy_chain daisy;
	UINT8			rtemp;
	const UINT8 *	cc_op;
	const UINT8 *	cc_cb;
	const UINT8 *	cc_ed;
	const UINT8 *	cc_xy;
	const UINT8 *	cc_xycb;
	const UINT8 *	cc_ex;
};

INLINE z80_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == Z80 || device->type() == NSC800);
	return (z80_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define CF		0x01
#define NF		0x02
#define PF		0x04
#define VF		PF
#define XF		0x08
#define HF		0x10
#define YF		0x20
#define ZF		0x40
#define SF		0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PRVPC	prvpc.d		/* previous program counter */

#define PCD		pc.d
#define PC		pc.w.l

#define SPD 	sp.d
#define SP		sp.w.l

#define AFD 	af.d
#define AF		af.w.l
#define A		af.b.h
#define F		af.b.l

#define BCD 	bc.d
#define BC		bc.w.l
#define B		bc.b.h
#define C		bc.b.l

#define DED 	de.d
#define DE		de.w.l
#define D		de.b.h
#define E		de.b.l

#define HLD 	hl.d
#define HL		hl.w.l
#define H		hl.b.h
#define L		hl.b.l

#define IXD 	ix.d
#define IX		ix.w.l
#define HX		ix.b.h
#define LX		ix.b.l

#define IYD 	iy.d
#define IY		iy.w.l
#define HY		iy.b.h
#define LY		iy.b.l

#define WZ		wz.w.l
#define WZ_H	wz.b.h
#define WZ_L	wz.b.l


static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static UINT8 *SZHVC_add = 0;
static UINT8 *SZHVC_sub = 0;

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
 5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11,	/* cb -> cc_cb */
 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11,	/* dd -> cc_xy */
 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11,	/* ed -> cc_ed */
 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11		/* fd -> cc_xy */
};

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
 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8
};

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
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

/* ix/iy: with the exception of (i+offset) opcodes, t-states are main_opcode_table + 4 */
static const UINT8 cc_xy[0x100] = {
 4+4,10+4, 7+4, 6+4, 4+4, 4+4, 7+4, 4+4, 4+4,11+4, 7+4, 6+4, 4+4, 4+4, 7+4, 4+4,
 8+4,10+4, 7+4, 6+4, 4+4, 4+4, 7+4, 4+4,12+4,11+4, 7+4, 6+4, 4+4, 4+4, 7+4, 4+4,
 7+4,10+4,16+4, 6+4, 4+4, 4+4, 7+4, 4+4, 7+4,11+4,16+4, 6+4, 4+4, 4+4, 7+4, 4+4,
 7+4,10+4,13+4, 6+4,23  ,23  ,19  , 4+4, 7+4,11+4,13+4, 6+4, 4+4, 4+4, 7+4, 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
19  ,19  ,19  ,19  ,19  ,19  , 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4, 4+4, 4+4, 4+4, 4+4, 4+4, 4+4,19  , 4+4,
 5+4,10+4,10+4,10+4,10+4,11+4, 7+4,11+4, 5+4,10+4,10+4, 0  ,10+4,17+4, 7+4,11+4,	/* cb -> cc_xycb */
 5+4,10+4,10+4,11+4,10+4,11+4, 7+4,11+4, 5+4, 4+4,10+4,11+4,10+4, 4+4, 7+4,11+4,
 5+4,10+4,10+4,19+4,10+4,11+4, 7+4,11+4, 5+4, 4+4,10+4, 4+4,10+4, 4+4, 7+4,11+4,
 5+4,10+4,10+4, 4+4,10+4,11+4, 7+4,11+4, 5+4, 6+4,10+4, 4+4,10+4, 4+4, 7+4,11+4
};

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
23,23,23,23,23,23,23,23,23,23,23,23,23,23,23,23
};

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
 6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
};

#define cc_dd	cc_xy
#define cc_fd	cc_xy

static void take_interrupt(z80_state *z80);
static void take_interrupt_nsc800(z80_state *z80);

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

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode)  INLINE void prefix##_##opcode(z80_state *z80)

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(Z,prefix,opcode)	do { (Z)->icount -= z80->cc_##prefix[opcode]; } while (0)

/***************************************************************
 * execute an opcode
 ***************************************************************/
#define EXEC(Z,prefix,opcode)									\
{																\
	unsigned op = opcode;										\
	CC(Z,prefix,op);											\
	(*Z80##prefix[op])(Z);										\
}

#if BIG_SWITCH
#define EXEC_INLINE(Z,prefix,opcode)							\
{																\
	unsigned op = opcode;										\
	CC(Z,prefix,op);											\
	switch(op)													\
	{															\
	case 0x00:prefix##_##00(Z);break; case 0x01:prefix##_##01(Z);break; case 0x02:prefix##_##02(Z);break; case 0x03:prefix##_##03(Z);break; \
	case 0x04:prefix##_##04(Z);break; case 0x05:prefix##_##05(Z);break; case 0x06:prefix##_##06(Z);break; case 0x07:prefix##_##07(Z);break; \
	case 0x08:prefix##_##08(Z);break; case 0x09:prefix##_##09(Z);break; case 0x0a:prefix##_##0a(Z);break; case 0x0b:prefix##_##0b(Z);break; \
	case 0x0c:prefix##_##0c(Z);break; case 0x0d:prefix##_##0d(Z);break; case 0x0e:prefix##_##0e(Z);break; case 0x0f:prefix##_##0f(Z);break; \
	case 0x10:prefix##_##10(Z);break; case 0x11:prefix##_##11(Z);break; case 0x12:prefix##_##12(Z);break; case 0x13:prefix##_##13(Z);break; \
	case 0x14:prefix##_##14(Z);break; case 0x15:prefix##_##15(Z);break; case 0x16:prefix##_##16(Z);break; case 0x17:prefix##_##17(Z);break; \
	case 0x18:prefix##_##18(Z);break; case 0x19:prefix##_##19(Z);break; case 0x1a:prefix##_##1a(Z);break; case 0x1b:prefix##_##1b(Z);break; \
	case 0x1c:prefix##_##1c(Z);break; case 0x1d:prefix##_##1d(Z);break; case 0x1e:prefix##_##1e(Z);break; case 0x1f:prefix##_##1f(Z);break; \
	case 0x20:prefix##_##20(Z);break; case 0x21:prefix##_##21(Z);break; case 0x22:prefix##_##22(Z);break; case 0x23:prefix##_##23(Z);break; \
	case 0x24:prefix##_##24(Z);break; case 0x25:prefix##_##25(Z);break; case 0x26:prefix##_##26(Z);break; case 0x27:prefix##_##27(Z);break; \
	case 0x28:prefix##_##28(Z);break; case 0x29:prefix##_##29(Z);break; case 0x2a:prefix##_##2a(Z);break; case 0x2b:prefix##_##2b(Z);break; \
	case 0x2c:prefix##_##2c(Z);break; case 0x2d:prefix##_##2d(Z);break; case 0x2e:prefix##_##2e(Z);break; case 0x2f:prefix##_##2f(Z);break; \
	case 0x30:prefix##_##30(Z);break; case 0x31:prefix##_##31(Z);break; case 0x32:prefix##_##32(Z);break; case 0x33:prefix##_##33(Z);break; \
	case 0x34:prefix##_##34(Z);break; case 0x35:prefix##_##35(Z);break; case 0x36:prefix##_##36(Z);break; case 0x37:prefix##_##37(Z);break; \
	case 0x38:prefix##_##38(Z);break; case 0x39:prefix##_##39(Z);break; case 0x3a:prefix##_##3a(Z);break; case 0x3b:prefix##_##3b(Z);break; \
	case 0x3c:prefix##_##3c(Z);break; case 0x3d:prefix##_##3d(Z);break; case 0x3e:prefix##_##3e(Z);break; case 0x3f:prefix##_##3f(Z);break; \
	case 0x40:prefix##_##40(Z);break; case 0x41:prefix##_##41(Z);break; case 0x42:prefix##_##42(Z);break; case 0x43:prefix##_##43(Z);break; \
	case 0x44:prefix##_##44(Z);break; case 0x45:prefix##_##45(Z);break; case 0x46:prefix##_##46(Z);break; case 0x47:prefix##_##47(Z);break; \
	case 0x48:prefix##_##48(Z);break; case 0x49:prefix##_##49(Z);break; case 0x4a:prefix##_##4a(Z);break; case 0x4b:prefix##_##4b(Z);break; \
	case 0x4c:prefix##_##4c(Z);break; case 0x4d:prefix##_##4d(Z);break; case 0x4e:prefix##_##4e(Z);break; case 0x4f:prefix##_##4f(Z);break; \
	case 0x50:prefix##_##50(Z);break; case 0x51:prefix##_##51(Z);break; case 0x52:prefix##_##52(Z);break; case 0x53:prefix##_##53(Z);break; \
	case 0x54:prefix##_##54(Z);break; case 0x55:prefix##_##55(Z);break; case 0x56:prefix##_##56(Z);break; case 0x57:prefix##_##57(Z);break; \
	case 0x58:prefix##_##58(Z);break; case 0x59:prefix##_##59(Z);break; case 0x5a:prefix##_##5a(Z);break; case 0x5b:prefix##_##5b(Z);break; \
	case 0x5c:prefix##_##5c(Z);break; case 0x5d:prefix##_##5d(Z);break; case 0x5e:prefix##_##5e(Z);break; case 0x5f:prefix##_##5f(Z);break; \
	case 0x60:prefix##_##60(Z);break; case 0x61:prefix##_##61(Z);break; case 0x62:prefix##_##62(Z);break; case 0x63:prefix##_##63(Z);break; \
	case 0x64:prefix##_##64(Z);break; case 0x65:prefix##_##65(Z);break; case 0x66:prefix##_##66(Z);break; case 0x67:prefix##_##67(Z);break; \
	case 0x68:prefix##_##68(Z);break; case 0x69:prefix##_##69(Z);break; case 0x6a:prefix##_##6a(Z);break; case 0x6b:prefix##_##6b(Z);break; \
	case 0x6c:prefix##_##6c(Z);break; case 0x6d:prefix##_##6d(Z);break; case 0x6e:prefix##_##6e(Z);break; case 0x6f:prefix##_##6f(Z);break; \
	case 0x70:prefix##_##70(Z);break; case 0x71:prefix##_##71(Z);break; case 0x72:prefix##_##72(Z);break; case 0x73:prefix##_##73(Z);break; \
	case 0x74:prefix##_##74(Z);break; case 0x75:prefix##_##75(Z);break; case 0x76:prefix##_##76(Z);break; case 0x77:prefix##_##77(Z);break; \
	case 0x78:prefix##_##78(Z);break; case 0x79:prefix##_##79(Z);break; case 0x7a:prefix##_##7a(Z);break; case 0x7b:prefix##_##7b(Z);break; \
	case 0x7c:prefix##_##7c(Z);break; case 0x7d:prefix##_##7d(Z);break; case 0x7e:prefix##_##7e(Z);break; case 0x7f:prefix##_##7f(Z);break; \
	case 0x80:prefix##_##80(Z);break; case 0x81:prefix##_##81(Z);break; case 0x82:prefix##_##82(Z);break; case 0x83:prefix##_##83(Z);break; \
	case 0x84:prefix##_##84(Z);break; case 0x85:prefix##_##85(Z);break; case 0x86:prefix##_##86(Z);break; case 0x87:prefix##_##87(Z);break; \
	case 0x88:prefix##_##88(Z);break; case 0x89:prefix##_##89(Z);break; case 0x8a:prefix##_##8a(Z);break; case 0x8b:prefix##_##8b(Z);break; \
	case 0x8c:prefix##_##8c(Z);break; case 0x8d:prefix##_##8d(Z);break; case 0x8e:prefix##_##8e(Z);break; case 0x8f:prefix##_##8f(Z);break; \
	case 0x90:prefix##_##90(Z);break; case 0x91:prefix##_##91(Z);break; case 0x92:prefix##_##92(Z);break; case 0x93:prefix##_##93(Z);break; \
	case 0x94:prefix##_##94(Z);break; case 0x95:prefix##_##95(Z);break; case 0x96:prefix##_##96(Z);break; case 0x97:prefix##_##97(Z);break; \
	case 0x98:prefix##_##98(Z);break; case 0x99:prefix##_##99(Z);break; case 0x9a:prefix##_##9a(Z);break; case 0x9b:prefix##_##9b(Z);break; \
	case 0x9c:prefix##_##9c(Z);break; case 0x9d:prefix##_##9d(Z);break; case 0x9e:prefix##_##9e(Z);break; case 0x9f:prefix##_##9f(Z);break; \
	case 0xa0:prefix##_##a0(Z);break; case 0xa1:prefix##_##a1(Z);break; case 0xa2:prefix##_##a2(Z);break; case 0xa3:prefix##_##a3(Z);break; \
	case 0xa4:prefix##_##a4(Z);break; case 0xa5:prefix##_##a5(Z);break; case 0xa6:prefix##_##a6(Z);break; case 0xa7:prefix##_##a7(Z);break; \
	case 0xa8:prefix##_##a8(Z);break; case 0xa9:prefix##_##a9(Z);break; case 0xaa:prefix##_##aa(Z);break; case 0xab:prefix##_##ab(Z);break; \
	case 0xac:prefix##_##ac(Z);break; case 0xad:prefix##_##ad(Z);break; case 0xae:prefix##_##ae(Z);break; case 0xaf:prefix##_##af(Z);break; \
	case 0xb0:prefix##_##b0(Z);break; case 0xb1:prefix##_##b1(Z);break; case 0xb2:prefix##_##b2(Z);break; case 0xb3:prefix##_##b3(Z);break; \
	case 0xb4:prefix##_##b4(Z);break; case 0xb5:prefix##_##b5(Z);break; case 0xb6:prefix##_##b6(Z);break; case 0xb7:prefix##_##b7(Z);break; \
	case 0xb8:prefix##_##b8(Z);break; case 0xb9:prefix##_##b9(Z);break; case 0xba:prefix##_##ba(Z);break; case 0xbb:prefix##_##bb(Z);break; \
	case 0xbc:prefix##_##bc(Z);break; case 0xbd:prefix##_##bd(Z);break; case 0xbe:prefix##_##be(Z);break; case 0xbf:prefix##_##bf(Z);break; \
	case 0xc0:prefix##_##c0(Z);break; case 0xc1:prefix##_##c1(Z);break; case 0xc2:prefix##_##c2(Z);break; case 0xc3:prefix##_##c3(Z);break; \
	case 0xc4:prefix##_##c4(Z);break; case 0xc5:prefix##_##c5(Z);break; case 0xc6:prefix##_##c6(Z);break; case 0xc7:prefix##_##c7(Z);break; \
	case 0xc8:prefix##_##c8(Z);break; case 0xc9:prefix##_##c9(Z);break; case 0xca:prefix##_##ca(Z);break; case 0xcb:prefix##_##cb(Z);break; \
	case 0xcc:prefix##_##cc(Z);break; case 0xcd:prefix##_##cd(Z);break; case 0xce:prefix##_##ce(Z);break; case 0xcf:prefix##_##cf(Z);break; \
	case 0xd0:prefix##_##d0(Z);break; case 0xd1:prefix##_##d1(Z);break; case 0xd2:prefix##_##d2(Z);break; case 0xd3:prefix##_##d3(Z);break; \
	case 0xd4:prefix##_##d4(Z);break; case 0xd5:prefix##_##d5(Z);break; case 0xd6:prefix##_##d6(Z);break; case 0xd7:prefix##_##d7(Z);break; \
	case 0xd8:prefix##_##d8(Z);break; case 0xd9:prefix##_##d9(Z);break; case 0xda:prefix##_##da(Z);break; case 0xdb:prefix##_##db(Z);break; \
	case 0xdc:prefix##_##dc(Z);break; case 0xdd:prefix##_##dd(Z);break; case 0xde:prefix##_##de(Z);break; case 0xdf:prefix##_##df(Z);break; \
	case 0xe0:prefix##_##e0(Z);break; case 0xe1:prefix##_##e1(Z);break; case 0xe2:prefix##_##e2(Z);break; case 0xe3:prefix##_##e3(Z);break; \
	case 0xe4:prefix##_##e4(Z);break; case 0xe5:prefix##_##e5(Z);break; case 0xe6:prefix##_##e6(Z);break; case 0xe7:prefix##_##e7(Z);break; \
	case 0xe8:prefix##_##e8(Z);break; case 0xe9:prefix##_##e9(Z);break; case 0xea:prefix##_##ea(Z);break; case 0xeb:prefix##_##eb(Z);break; \
	case 0xec:prefix##_##ec(Z);break; case 0xed:prefix##_##ed(Z);break; case 0xee:prefix##_##ee(Z);break; case 0xef:prefix##_##ef(Z);break; \
	case 0xf0:prefix##_##f0(Z);break; case 0xf1:prefix##_##f1(Z);break; case 0xf2:prefix##_##f2(Z);break; case 0xf3:prefix##_##f3(Z);break; \
	case 0xf4:prefix##_##f4(Z);break; case 0xf5:prefix##_##f5(Z);break; case 0xf6:prefix##_##f6(Z);break; case 0xf7:prefix##_##f7(Z);break; \
	case 0xf8:prefix##_##f8(Z);break; case 0xf9:prefix##_##f9(Z);break; case 0xfa:prefix##_##fa(Z);break; case 0xfb:prefix##_##fb(Z);break; \
	case 0xfc:prefix##_##fc(Z);break; case 0xfd:prefix##_##fd(Z);break; case 0xfe:prefix##_##fe(Z);break; case 0xff:prefix##_##ff(Z);break; \
	}																																	\
}
#else
#define EXEC_INLINE EXEC
#endif


/***************************************************************
 * Enter halt state; write 1 to fake port on first execution
 ***************************************************************/
#define ENTER_HALT(Z) do {										\
	(Z)->PC--;													\
	(Z)->halt = 1;												\
} while (0)

/***************************************************************
 * Leave halt state; write 0 to fake port
 ***************************************************************/
#define LEAVE_HALT(Z) do {										\
	if( (Z)->halt )												\
	{															\
		(Z)->halt = 0;											\
		(Z)->PC++;												\
	}															\
} while (0)

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
#define IN(Z,port)  		(Z)->io->read_byte(port)

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
#define OUT(Z,port,value)	(Z)->io->write_byte(port, value)

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
#define RM(Z,addr)			(Z)->program->read_byte(addr)

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
INLINE void RM16(z80_state *z80, UINT32 addr, PAIR *r)
{
	r->b.l = RM(z80, addr);
	r->b.h = RM(z80, (addr+1)&0xffff);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
#define WM(Z,addr,value)	(Z)->program->write_byte(addr, value)

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
INLINE void WM16(z80_state *z80, UINT32 addr, PAIR *r)
{
	WM(z80, addr, r->b.l);
	WM(z80, (addr+1)&0xffff, r->b.h);
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
INLINE UINT8 ROP(z80_state *z80)
{
	unsigned pc = z80->PCD;
	z80->PC++;
	return z80->direct->read_decrypted_byte(pc);
}

/****************************************************************
 * ARG(z80) is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
INLINE UINT8 ARG(z80_state *z80)
{
	unsigned pc = z80->PCD;
	z80->PC++;
	return z80->direct->read_raw_byte(pc);
}

INLINE UINT32 ARG16(z80_state *z80)
{
	unsigned pc = z80->PCD;
	z80->PC += 2;
	return z80->direct->read_raw_byte(pc) | (z80->direct->read_raw_byte((pc+1)&0xffff) << 8);
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
#define EAX(Z)		do { (Z)->ea = (UINT32)(UINT16)((Z)->IX + (INT8)ARG(Z)); (Z)->WZ = (Z)->ea; } while (0)
#define EAY(Z)		do { (Z)->ea = (UINT32)(UINT16)((Z)->IY + (INT8)ARG(Z)); (Z)->WZ = (Z)->ea; } while (0)

/***************************************************************
 * POP
 ***************************************************************/
#define POP(Z,DR)	do { RM16((Z), (Z)->SPD, &(Z)->DR); (Z)->SP += 2; } while (0)

/***************************************************************
 * PUSH
 ***************************************************************/
#define PUSH(Z,SR)	do { (Z)->SP -= 2; WM16((Z), (Z)->SPD, &(Z)->SR); } while (0)

/***************************************************************
 * JP
 ***************************************************************/
#define JP(Z) do {												\
	(Z)->PCD = ARG16(Z);										\
	(Z)->WZ = (Z)->PCD;											\
} while (0)

/***************************************************************
 * JP_COND
 ***************************************************************/
#define JP_COND(Z, cond) do {									\
	if (cond)													\
	{															\
		(Z)->PCD = ARG16(Z);									\
		(Z)->WZ = (Z)->PCD;										\
	}															\
	else														\
	{															\
		(Z)->WZ = ARG16(Z); /* implicit do PC += 2 */			\
	}															\
} while (0)

/***************************************************************
 * JR
 ***************************************************************/
#define JR(Z) do {												\
	INT8 arg = (INT8)ARG(Z);	/* ARG() also increments PC */	\
	(Z)->PC += arg;				/* so don't do PC += ARG() */	\
	(Z)->WZ = (Z)->PC;											\
} while (0)

/***************************************************************
 * JR_COND
 ***************************************************************/
#define JR_COND(Z, cond, opcode) do {							\
	if (cond)													\
	{															\
		JR(Z);													\
		CC(Z, ex, opcode);										\
	}															\
	else (Z)->PC++;												\
} while (0)

/***************************************************************
 * CALL
 ***************************************************************/
#define CALL(Z) do {											\
	(Z)->ea = ARG16(Z);											\
	(Z)->WZ = (Z)->ea;											\
	PUSH((Z), pc);												\
	(Z)->PCD = (Z)->ea;											\
} while (0)

/***************************************************************
 * CALL_COND
 ***************************************************************/
#define CALL_COND(Z, cond, opcode) do {							\
	if (cond)													\
	{															\
		(Z)->ea = ARG16(Z);										\
		(Z)->WZ = (Z)->ea;										\
		PUSH((Z), pc);											\
		(Z)->PCD = (Z)->ea;										\
		CC(Z, ex, opcode);										\
	}															\
	else														\
	{															\
		z80->WZ = ARG16(z80);  /* implicit call PC+=2;   */		\
	}															\
} while (0)

/***************************************************************
 * RET_COND
 ***************************************************************/
#define RET_COND(Z, cond, opcode) do {							\
	if (cond)													\
	{															\
		POP((Z), pc);											\
		(Z)->WZ = (Z)->PC;										\
		CC(Z, ex, opcode);										\
	}															\
} while (0)

/***************************************************************
 * RETN
 ***************************************************************/
#define RETN(Z) do {											\
	LOG(("Z80 '%s' RETN z80->iff1:%d z80->iff2:%d\n",			\
		(Z)->device->tag(), (Z)->iff1, (Z)->iff2));		\
	POP((Z), pc);												\
	(Z)->WZ = (Z)->PC;											\
	(Z)->iff1 = (Z)->iff2;										\
} while (0)

/***************************************************************
 * RETI
 ***************************************************************/
#define RETI(Z) do {											\
	POP((Z), pc);												\
	(Z)->WZ = (Z)->PC;											\
/* according to http://www.msxnet.org/tech/z80-documented.pdf */\
	(Z)->iff1 = (Z)->iff2;										\
	(Z)->daisy.call_reti_device();								\
} while (0)

/***************************************************************
 * LD   R,A
 ***************************************************************/
#define LD_R_A(Z) do {											\
	(Z)->r = (Z)->A;											\
	(Z)->r2 = (Z)->A & 0x80;			/* keep bit 7 of r */	\
} while (0)

/***************************************************************
 * LD   A,R
 ***************************************************************/
#define LD_A_R(Z) do {											\
	(Z)->A = ((Z)->r & 0x7f) | (Z)->r2;							\
	(Z)->F = ((Z)->F & CF) | SZ[(Z)->A] | ((Z)->iff2 << 2);		\
	(Z)->after_ldair = TRUE;									\
} while (0)

/***************************************************************
 * LD   I,A
 ***************************************************************/
#define LD_I_A(Z) do {											\
	(Z)->i = (Z)->A;											\
} while (0)

/***************************************************************
 * LD   A,I
 ***************************************************************/
#define LD_A_I(Z) do {											\
	(Z)->A = (Z)->i;											\
	(Z)->F = ((Z)->F & CF) | SZ[(Z)->A] | ((Z)->iff2 << 2);		\
	(Z)->after_ldair = TRUE;									\
} while (0)

/***************************************************************
 * RST
 ***************************************************************/
#define RST(Z, addr) do {										\
	PUSH((Z), pc);												\
	(Z)->PCD = addr;											\
	(Z)->WZ = (Z)->PC;											\
} while (0)

/***************************************************************
 * INC  r8
 ***************************************************************/
INLINE UINT8 INC(z80_state *z80, UINT8 value)
{
	UINT8 res = value + 1;
	z80->F = (z80->F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
INLINE UINT8 DEC(z80_state *z80, UINT8 value)
{
	UINT8 res = value - 1;
	z80->F = (z80->F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
#define RLCA(Z) do {											\
	(Z)->A = ((Z)->A << 1) | ((Z)->A >> 7);						\
	(Z)->F = ((Z)->F & (SF | ZF | PF)) | ((Z)->A & (YF | XF | CF)); \
} while (0)

/***************************************************************
 * RRCA
 ***************************************************************/
#define RRCA(Z) do {											\
	(Z)->F = ((Z)->F & (SF | ZF | PF)) | ((Z)->A & CF);			\
	(Z)->A = ((Z)->A >> 1) | ((Z)->A << 7);						\
	(Z)->F |= ((Z)->A & (YF | XF));								\
} while (0)

/***************************************************************
 * RLA
 ***************************************************************/
#define RLA(Z) do {												\
	UINT8 res = ((Z)->A << 1) | ((Z)->F & CF);					\
	UINT8 c = ((Z)->A & 0x80) ? CF : 0;							\
	(Z)->F = ((Z)->F & (SF | ZF | PF)) | c | (res & (YF | XF));	\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * RRA
 ***************************************************************/
#define RRA(Z) do {												\
	UINT8 res = ((Z)->A >> 1) | ((Z)->F << 7);					\
	UINT8 c = ((Z)->A & 0x01) ? CF : 0;							\
	(Z)->F = ((Z)->F & (SF | ZF | PF)) | c | (res & (YF | XF));	\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * RRD
 ***************************************************************/
#define RRD(Z) do {												\
	UINT8 n = RM((Z), (Z)->HL);									\
	(Z)->WZ = (Z)->HL+1;										\
	WM((Z), (Z)->HL, (n >> 4) | ((Z)->A << 4));					\
	(Z)->A = ((Z)->A & 0xf0) | (n & 0x0f);						\
	(Z)->F = ((Z)->F & CF) | SZP[(Z)->A];						\
} while (0)

/***************************************************************
 * RLD
 ***************************************************************/
#define RLD(Z) do {												\
	UINT8 n = RM((Z), (Z)->HL);									\
	(Z)->WZ = (Z)->HL+1;										\
	WM((Z), (Z)->HL, (n << 4) | ((Z)->A & 0x0f));				\
	(Z)->A = ((Z)->A & 0xf0) | (n >> 4);						\
	(Z)->F = ((Z)->F & CF) | SZP[(Z)->A];						\
} while (0)

/***************************************************************
 * ADD  A,n
 ***************************************************************/
#define ADD(Z, value) do {										\
	UINT32 ah = (Z)->AFD & 0xff00;								\
	UINT32 res = (UINT8)((ah >> 8) + value);					\
	(Z)->F = SZHVC_add[ah | res];								\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * ADC  A,n
 ***************************************************************/
#define ADC(Z, value) do {										\
	UINT32 ah = (Z)->AFD & 0xff00, c = (Z)->AFD & 1;			\
	UINT32 res = (UINT8)((ah >> 8) + value + c);				\
	(Z)->F = SZHVC_add[(c << 16) | ah | res];					\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * SUB  n
 ***************************************************************/
#define SUB(Z, value) do {										\
	UINT32 ah = (Z)->AFD & 0xff00;								\
	UINT32 res = (UINT8)((ah >> 8) - value);					\
	(Z)->F = SZHVC_sub[ah | res];								\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * SBC  A,n
 ***************************************************************/
#define SBC(Z, value) do {										\
	UINT32 ah = (Z)->AFD & 0xff00, c = (Z)->AFD & 1;			\
	UINT32 res = (UINT8)((ah >> 8) - value - c);				\
	(Z)->F = SZHVC_sub[(c<<16) | ah | res];						\
	(Z)->A = res;												\
} while (0)

/***************************************************************
 * NEG
 ***************************************************************/
#define NEG(Z) do {												\
	UINT8 value = (Z)->A;										\
	(Z)->A = 0;													\
	SUB(Z, value);												\
} while (0)

/***************************************************************
 * DAA
 ***************************************************************/
#define DAA(Z) do {												\
	UINT8 a = (Z)->A;											\
	if ((Z)->F & NF) {											\
		if (((Z)->F&HF) | (((Z)->A&0xf)>9)) a-=6;				\
		if (((Z)->F&CF) | ((Z)->A>0x99)) a-=0x60;				\
	}															\
	else {														\
		if (((Z)->F&HF) | (((Z)->A&0xf)>9)) a+=6;				\
		if (((Z)->F&CF) | ((Z)->A>0x99)) a+=0x60;				\
	}															\
																\
	(Z)->F = ((Z)->F&(CF|NF)) | ((Z)->A>0x99) | (((Z)->A^a)&HF) | SZP[a]; \
	(Z)->A = a;													\
} while (0)

/***************************************************************
 * AND  n
 ***************************************************************/
#define AND(Z, value) do {										\
	(Z)->A &= value;											\
	(Z)->F = SZP[(Z)->A] | HF;									\
} while (0)

/***************************************************************
 * OR   n
 ***************************************************************/
#define OR(Z, value) do {										\
	(Z)->A |= value;											\
	(Z)->F = SZP[(Z)->A];										\
} while (0)

/***************************************************************
 * XOR  n
 ***************************************************************/
#define XOR(Z, value) do {										\
	(Z)->A ^= value;											\
	(Z)->F = SZP[(Z)->A];										\
} while (0)

/***************************************************************
 * CP   n
 ***************************************************************/
#define CP(Z, value) do {										\
	unsigned val = value;										\
	UINT32 ah = (Z)->AFD & 0xff00;								\
	UINT32 res = (UINT8)((ah >> 8) - val);						\
	(Z)->F = (SZHVC_sub[ah | res] & ~(YF | XF)) |				\
		(val & (YF | XF));										\
} while (0)

/***************************************************************
 * EX   AF,AF'
 ***************************************************************/
#define EX_AF(Z) do {											\
	PAIR tmp;													\
	tmp = (Z)->af; (Z)->af = (Z)->af2; (Z)->af2 = tmp;			\
} while (0)

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
#define EX_DE_HL(Z) do {										\
	PAIR tmp;													\
	tmp = (Z)->de; (Z)->de = (Z)->hl; (Z)->hl = tmp;			\
} while (0)

/***************************************************************
 * EXX
 ***************************************************************/
#define EXX(Z) do {												\
	PAIR tmp;													\
	tmp = (Z)->bc; (Z)->bc = (Z)->bc2; (Z)->bc2 = tmp;			\
	tmp = (Z)->de; (Z)->de = (Z)->de2; (Z)->de2 = tmp;			\
	tmp = (Z)->hl; (Z)->hl = (Z)->hl2; (Z)->hl2 = tmp;			\
} while (0)

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
#define EXSP(Z, DR) do {										\
	PAIR tmp = { { 0, 0, 0, 0 } };								\
	RM16((Z), (Z)->SPD, &tmp);									\
	WM16((Z), (Z)->SPD, &(Z)->DR);								\
	(Z)->DR = tmp;												\
	(Z)->WZ = (Z)->DR.d;										\
} while (0)

/***************************************************************
 * ADD16
 ***************************************************************/
#define ADD16(Z, DR, SR) do {									\
	UINT32 res = (Z)->DR.d + (Z)->SR.d;							\
	(Z)->WZ = (Z)->DR.d + 1;									\
	(Z)->F = ((Z)->F & (SF | ZF | VF)) |						\
		((((Z)->DR.d ^ res ^ (Z)->SR.d) >> 8) & HF) |			\
		((res >> 16) & CF) | ((res >> 8) & (YF | XF));			\
	(Z)->DR.w.l = (UINT16)res;									\
} while (0)

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
#define ADC16(Z, Reg) do {										\
	UINT32 res = (Z)->HLD + (Z)->Reg.d + ((Z)->F & CF);			\
	(Z)->WZ = (Z)->HL + 1;										\
	(Z)->F = ((((Z)->HLD ^ res ^ (Z)->Reg.d) >> 8) & HF) |		\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) |							\
		((res & 0xffff) ? 0 : ZF) |								\
		((((Z)->Reg.d ^ (Z)->HLD ^ 0x8000) & ((Z)->Reg.d ^ res) & 0x8000) >> 13); \
	(Z)->HL = (UINT16)res;										\
} while (0)

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
#define SBC16(Z, Reg) do {										\
	UINT32 res = (Z)->HLD - (Z)->Reg.d - ((Z)->F & CF);			\
	(Z)->WZ = (Z)->HL + 1;										\
	(Z)->F = ((((Z)->HLD ^ res ^ (Z)->Reg.d) >> 8) & HF) | NF |	\
		((res >> 16) & CF) |									\
		((res >> 8) & (SF | YF | XF)) |							\
		((res & 0xffff) ? 0 : ZF) |								\
		((((Z)->Reg.d ^ (Z)->HLD) & ((Z)->HLD ^ res) &0x8000) >> 13); \
	(Z)->HL = (UINT16)res;										\
} while (0)

/***************************************************************
 * RLC  r8
 ***************************************************************/
INLINE UINT8 RLC(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	z80->F = SZP[res] | c;
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
	z80->F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RL   r8
 ***************************************************************/
INLINE UINT8 RL(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (z80->F & CF)) & 0xff;
	z80->F = SZP[res] | c;
	return res;
}

/***************************************************************
 * RR   r8
 ***************************************************************/
INLINE UINT8 RR(z80_state *z80, UINT8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x01) ? CF : 0;
	res = ((res >> 1) | (z80->F << 7)) & 0xff;
	z80->F = SZP[res] | c;
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
	z80->F = SZP[res] | c;
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
	z80->F = SZP[res] | c;
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
	z80->F = SZP[res] | c;
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
	z80->F = SZP[res] | c;
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
#undef BIT
#define BIT(Z, bit, reg) do {									\
	(Z)->F = ((Z)->F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | (reg & (YF|XF)); \
} while (0)

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
#define BIT_HL(Z, bit, reg) do {								\
	(Z)->F = ((Z)->F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | ((Z)->WZ_H & (YF|XF)); \
} while (0)

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
#define BIT_XY(Z, bit, reg) do {								\
	(Z)->F = ((Z)->F & CF) | HF | (SZ_BIT[reg & (1<<bit)] & ~(YF|XF)) | (((Z)->ea>>8) & (YF|XF)); \
} while (0)

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
#define LDI(Z) do {												\
	UINT8 io = RM((Z), (Z)->HL);								\
	WM((Z), (Z)->DE, io);										\
	(Z)->F &= SF | ZF | CF;										\
	if (((Z)->A + io) & 0x02) (Z)->F |= YF; /* bit 1 -> flag 5 */ \
	if (((Z)->A + io) & 0x08) (Z)->F |= XF; /* bit 3 -> flag 3 */ \
	(Z)->HL++; (Z)->DE++; (Z)->BC--;							\
	if((Z)->BC) (Z)->F |= VF;									\
} while (0)

/***************************************************************
 * CPI
 ***************************************************************/
#define CPI(Z) do {												\
	UINT8 val = RM((Z), (Z)->HL);								\
	UINT8 res = (Z)->A - val;									\
	(Z)->WZ++;													\
	(Z)->HL++; (Z)->BC--;										\
	(Z)->F = ((Z)->F & CF) | (SZ[res]&~(YF|XF)) | (((Z)->A^val^res)&HF) | NF; \
	if ((Z)->F & HF) res -= 1;									\
	if (res & 0x02) (Z)->F |= YF; /* bit 1 -> flag 5 */			\
	if (res & 0x08) (Z)->F |= XF; /* bit 3 -> flag 3 */			\
	if ((Z)->BC) (Z)->F |= VF;									\
} while (0)

/***************************************************************
 * INI
 ***************************************************************/
#define INI(Z) do {												\
	unsigned t;													\
	UINT8 io = IN((Z), (Z)->BC);								\
	(Z)->WZ = (Z)->BC + 1;										\
	(Z)->B--;													\
	WM((Z), (Z)->HL, io);										\
	(Z)->HL++;													\
	(Z)->F = SZ[(Z)->B];										\
	t = (unsigned)(((Z)->C + 1) & 0xff) + (unsigned)io;			\
	if (io & SF) (Z)->F |= NF;									\
	if (t & 0x100) (Z)->F |= HF | CF;							\
	(Z)->F |= SZP[(UINT8)(t & 0x07) ^ (Z)->B] & PF;				\
} while (0)

/***************************************************************
 * OUTI
 ***************************************************************/
#define OUTI(Z) do {											\
	unsigned t;													\
	UINT8 io = RM((Z), (Z)->HL);								\
	(Z)->B--;													\
	(Z)->WZ = (Z)->BC + 1;										\
	OUT((Z), (Z)->BC, io);										\
	(Z)->HL++;													\
	(Z)->F = SZ[(Z)->B];										\
	t = (unsigned)(Z)->L + (unsigned)io;						\
	if (io & SF) (Z)->F |= NF;									\
	if (t & 0x100) (Z)->F |= HF | CF;							\
	(Z)->F |= SZP[(UINT8)(t & 0x07) ^ (Z)->B] & PF;				\
} while (0)

/***************************************************************
 * LDD
 ***************************************************************/
#define LDD(Z) do {												\
	UINT8 io = RM((Z), (Z)->HL);								\
	WM((Z), (Z)->DE, io);										\
	(Z)->F &= SF | ZF | CF;										\
	if (((Z)->A + io) & 0x02) (Z)->F |= YF; /* bit 1 -> flag 5 */ \
	if (((Z)->A + io) & 0x08) (Z)->F |= XF; /* bit 3 -> flag 3 */ \
	(Z)->HL--; (Z)->DE--; (Z)->BC--;							\
	if ((Z)->BC) (Z)->F |= VF;									\
} while (0)

/***************************************************************
 * CPD
 ***************************************************************/
#define CPD(Z) do {												\
	UINT8 val = RM((Z), (Z)->HL);								\
	UINT8 res = (Z)->A - val;									\
	(Z)->WZ--;													\
	(Z)->HL--; (Z)->BC--;										\
	(Z)->F = ((Z)->F & CF) | (SZ[res]&~(YF|XF)) | (((Z)->A^val^res)&HF) | NF; \
	if ((Z)->F & HF) res -= 1;									\
	if (res & 0x02) (Z)->F |= YF; /* bit 1 -> flag 5 */			\
	if (res & 0x08) (Z)->F |= XF; /* bit 3 -> flag 3 */			\
	if ((Z)->BC) (Z)->F |= VF;									\
} while (0)

/***************************************************************
 * IND
 ***************************************************************/
#define IND(Z) do {												\
	unsigned t;													\
	UINT8 io = IN((Z), (Z)->BC);								\
	(Z)->WZ = (Z)->BC - 1;										\
	(Z)->B--;													\
	WM((Z), (Z)->HL, io);										\
	(Z)->HL--;													\
	(Z)->F = SZ[(Z)->B];										\
	t = ((unsigned)((Z)->C - 1) & 0xff) + (unsigned)io;			\
	if (io & SF) (Z)->F |= NF;									\
	if (t & 0x100) (Z)->F |= HF | CF;							\
	(Z)->F |= SZP[(UINT8)(t & 0x07) ^ (Z)->B] & PF;				\
} while (0)

/***************************************************************
 * OUTD
 ***************************************************************/
#define OUTD(Z) do {											\
	unsigned t;													\
	UINT8 io = RM((Z), (Z)->HL);								\
	(Z)->B--;													\
	(Z)->WZ = (Z)->BC - 1;										\
	OUT((Z), (Z)->BC, io);										\
	(Z)->HL--;													\
	(Z)->F = SZ[(Z)->B];										\
	t = (unsigned)(Z)->L + (unsigned)io;						\
	if (io & SF) (Z)->F |= NF;									\
	if (t & 0x100) (Z)->F |= HF | CF;							\
	(Z)->F |= SZP[(UINT8)(t & 0x07) ^ (Z)->B] & PF;				\
} while (0)

/***************************************************************
 * LDIR
 ***************************************************************/
#define LDIR(Z) do {											\
	LDI(Z);														\
	if ((Z)->BC != 0)											\
	{															\
		(Z)->PC -= 2;											\
		(Z)->WZ = (Z)->PC + 1;									\
		CC(Z, ex, 0xb0);										\
	}															\
} while (0)

/***************************************************************
 * CPIR
 ***************************************************************/
#define CPIR(Z) do {											\
	CPI(Z);														\
	if ((Z)->BC != 0 && !((Z)->F & ZF))							\
	{															\
		(Z)->PC -= 2;											\
		(Z)->WZ = (Z)->PC + 1;									\
		CC(Z, ex, 0xb1);										\
	}															\
} while (0)

/***************************************************************
 * INIR
 ***************************************************************/
#define INIR(Z) do {											\
	INI(Z);														\
	if ((Z)->B != 0)											\
	{															\
		(Z)->PC -= 2;											\
		CC(Z, ex, 0xb2);										\
	}															\
} while (0)

/***************************************************************
 * OTIR
 ***************************************************************/
#define OTIR(Z) do {											\
	OUTI(Z);													\
	if ((Z)->B != 0)											\
	{															\
		(Z)->PC -= 2;											\
		CC(Z, ex, 0xb3);										\
	}															\
} while (0)

/***************************************************************
 * LDDR
 ***************************************************************/
#define LDDR(Z) do {											\
	LDD(Z);														\
	if ((Z)->BC != 0)											\
	{															\
		(Z)->PC -= 2;											\
		(Z)->WZ = (Z)->PC + 1;									\
		CC(Z, ex, 0xb8);										\
	}															\
} while (0)

/***************************************************************
 * CPDR
 ***************************************************************/
#define CPDR(Z) do {											\
	CPD(Z);														\
	if ((Z)->BC != 0 && !((Z)->F & ZF))							\
	{															\
		(Z)->PC -= 2;											\
		(Z)->WZ = (Z)->PC + 1;									\
		CC(Z, ex, 0xb9);										\
	}															\
} while (0)

/***************************************************************
 * INDR
 ***************************************************************/
#define INDR(Z) do {											\
	IND(Z);														\
	if ((Z)->B != 0)											\
	{															\
		(Z)->PC -= 2;											\
		CC(Z, ex, 0xba);										\
	}															\
} while (0)

/***************************************************************
 * OTDR
 ***************************************************************/
#define OTDR(Z) do {											\
	OUTD(Z);													\
	if ((Z)->B != 0)											\
	{															\
		(Z)->PC -= 2;											\
		CC(Z, ex, 0xbb);										\
	}															\
} while (0)

/***************************************************************
 * EI
 ***************************************************************/
#define EI(Z) do {												\
	(Z)->iff1 = (Z)->iff2 = 1;									\
	(Z)->after_ei = TRUE;										\
} while (0)

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { z80->B = RLC(z80, z80->B);												} /* RLC  B           */
OP(cb,01) { z80->C = RLC(z80, z80->C);												} /* RLC  C           */
OP(cb,02) { z80->D = RLC(z80, z80->D);												} /* RLC  D           */
OP(cb,03) { z80->E = RLC(z80, z80->E);												} /* RLC  E           */
OP(cb,04) { z80->H = RLC(z80, z80->H);												} /* RLC  H           */
OP(cb,05) { z80->L = RLC(z80, z80->L);												} /* RLC  L           */
OP(cb,06) { WM(z80, z80->HL, RLC(z80, RM(z80, z80->HL)));							} /* RLC  (HL)        */
OP(cb,07) { z80->A = RLC(z80, z80->A);												} /* RLC  A           */

OP(cb,08) { z80->B = RRC(z80, z80->B);												} /* RRC  B           */
OP(cb,09) { z80->C = RRC(z80, z80->C);												} /* RRC  C           */
OP(cb,0a) { z80->D = RRC(z80, z80->D);												} /* RRC  D           */
OP(cb,0b) { z80->E = RRC(z80, z80->E);												} /* RRC  E           */
OP(cb,0c) { z80->H = RRC(z80, z80->H);												} /* RRC  H           */
OP(cb,0d) { z80->L = RRC(z80, z80->L);												} /* RRC  L           */
OP(cb,0e) { WM(z80, z80->HL, RRC(z80, RM(z80, z80->HL)));							} /* RRC  (HL)        */
OP(cb,0f) { z80->A = RRC(z80, z80->A);												} /* RRC  A           */

OP(cb,10) { z80->B = RL(z80, z80->B);												} /* RL   B           */
OP(cb,11) { z80->C = RL(z80, z80->C);												} /* RL   C           */
OP(cb,12) { z80->D = RL(z80, z80->D);												} /* RL   D           */
OP(cb,13) { z80->E = RL(z80, z80->E);												} /* RL   E           */
OP(cb,14) { z80->H = RL(z80, z80->H);												} /* RL   H           */
OP(cb,15) { z80->L = RL(z80, z80->L);												} /* RL   L           */
OP(cb,16) { WM(z80, z80->HL, RL(z80, RM(z80, z80->HL)));							} /* RL   (HL)        */
OP(cb,17) { z80->A = RL(z80, z80->A);												} /* RL   A           */

OP(cb,18) { z80->B = RR(z80, z80->B);												} /* RR   B           */
OP(cb,19) { z80->C = RR(z80, z80->C);												} /* RR   C           */
OP(cb,1a) { z80->D = RR(z80, z80->D);												} /* RR   D           */
OP(cb,1b) { z80->E = RR(z80, z80->E);												} /* RR   E           */
OP(cb,1c) { z80->H = RR(z80, z80->H);												} /* RR   H           */
OP(cb,1d) { z80->L = RR(z80, z80->L);												} /* RR   L           */
OP(cb,1e) { WM(z80, z80->HL, RR(z80, RM(z80, z80->HL)));							} /* RR   (HL)        */
OP(cb,1f) { z80->A = RR(z80, z80->A);												} /* RR   A           */

OP(cb,20) { z80->B = SLA(z80, z80->B);												} /* SLA  B           */
OP(cb,21) { z80->C = SLA(z80, z80->C);												} /* SLA  C           */
OP(cb,22) { z80->D = SLA(z80, z80->D);												} /* SLA  D           */
OP(cb,23) { z80->E = SLA(z80, z80->E);												} /* SLA  E           */
OP(cb,24) { z80->H = SLA(z80, z80->H);												} /* SLA  H           */
OP(cb,25) { z80->L = SLA(z80, z80->L);												} /* SLA  L           */
OP(cb,26) { WM(z80, z80->HL, SLA(z80, RM(z80, z80->HL)));							} /* SLA  (HL)        */
OP(cb,27) { z80->A = SLA(z80, z80->A);												} /* SLA  A           */

OP(cb,28) { z80->B = SRA(z80, z80->B);												} /* SRA  B           */
OP(cb,29) { z80->C = SRA(z80, z80->C);												} /* SRA  C           */
OP(cb,2a) { z80->D = SRA(z80, z80->D);												} /* SRA  D           */
OP(cb,2b) { z80->E = SRA(z80, z80->E);												} /* SRA  E           */
OP(cb,2c) { z80->H = SRA(z80, z80->H);												} /* SRA  H           */
OP(cb,2d) { z80->L = SRA(z80, z80->L);												} /* SRA  L           */
OP(cb,2e) { WM(z80, z80->HL, SRA(z80, RM(z80, z80->HL)));							} /* SRA  (HL)        */
OP(cb,2f) { z80->A = SRA(z80, z80->A);												} /* SRA  A           */

OP(cb,30) { z80->B = SLL(z80, z80->B);												} /* SLL  B           */
OP(cb,31) { z80->C = SLL(z80, z80->C);												} /* SLL  C           */
OP(cb,32) { z80->D = SLL(z80, z80->D);												} /* SLL  D           */
OP(cb,33) { z80->E = SLL(z80, z80->E);												} /* SLL  E           */
OP(cb,34) { z80->H = SLL(z80, z80->H);												} /* SLL  H           */
OP(cb,35) { z80->L = SLL(z80, z80->L);												} /* SLL  L           */
OP(cb,36) { WM(z80, z80->HL, SLL(z80, RM(z80, z80->HL)));							} /* SLL  (HL)        */
OP(cb,37) { z80->A = SLL(z80, z80->A);												} /* SLL  A           */

OP(cb,38) { z80->B = SRL(z80, z80->B);												} /* SRL  B           */
OP(cb,39) { z80->C = SRL(z80, z80->C);												} /* SRL  C           */
OP(cb,3a) { z80->D = SRL(z80, z80->D);												} /* SRL  D           */
OP(cb,3b) { z80->E = SRL(z80, z80->E);												} /* SRL  E           */
OP(cb,3c) { z80->H = SRL(z80, z80->H);												} /* SRL  H           */
OP(cb,3d) { z80->L = SRL(z80, z80->L);												} /* SRL  L           */
OP(cb,3e) { WM(z80, z80->HL, SRL(z80, RM(z80, z80->HL)));							} /* SRL  (HL)        */
OP(cb,3f) { z80->A = SRL(z80, z80->A);												} /* SRL  A           */

OP(cb,40) { BIT(z80, 0, z80->B);													} /* BIT  0,B         */
OP(cb,41) { BIT(z80, 0, z80->C);													} /* BIT  0,C         */
OP(cb,42) { BIT(z80, 0, z80->D);													} /* BIT  0,D         */
OP(cb,43) { BIT(z80, 0, z80->E);													} /* BIT  0,E         */
OP(cb,44) { BIT(z80, 0, z80->H);													} /* BIT  0,H         */
OP(cb,45) { BIT(z80, 0, z80->L);													} /* BIT  0,L         */
OP(cb,46) { BIT_HL(z80, 0, RM(z80, z80->HL));										} /* BIT  0,(HL)      */
OP(cb,47) { BIT(z80, 0, z80->A);													} /* BIT  0,A         */

OP(cb,48) { BIT(z80, 1, z80->B);													} /* BIT  1,B         */
OP(cb,49) { BIT(z80, 1, z80->C);													} /* BIT  1,C         */
OP(cb,4a) { BIT(z80, 1, z80->D);													} /* BIT  1,D         */
OP(cb,4b) { BIT(z80, 1, z80->E);													} /* BIT  1,E         */
OP(cb,4c) { BIT(z80, 1, z80->H);													} /* BIT  1,H         */
OP(cb,4d) { BIT(z80, 1, z80->L);													} /* BIT  1,L         */
OP(cb,4e) { BIT_HL(z80, 1, RM(z80, z80->HL));										} /* BIT  1,(HL)      */
OP(cb,4f) { BIT(z80, 1, z80->A);													} /* BIT  1,A         */

OP(cb,50) { BIT(z80, 2, z80->B);													} /* BIT  2,B         */
OP(cb,51) { BIT(z80, 2, z80->C);													} /* BIT  2,C         */
OP(cb,52) { BIT(z80, 2, z80->D);													} /* BIT  2,D         */
OP(cb,53) { BIT(z80, 2, z80->E);													} /* BIT  2,E         */
OP(cb,54) { BIT(z80, 2, z80->H);													} /* BIT  2,H         */
OP(cb,55) { BIT(z80, 2, z80->L);													} /* BIT  2,L         */
OP(cb,56) { BIT_HL(z80, 2, RM(z80, z80->HL));										} /* BIT  2,(HL)      */
OP(cb,57) { BIT(z80, 2, z80->A);													} /* BIT  2,A         */

OP(cb,58) { BIT(z80, 3, z80->B);													} /* BIT  3,B         */
OP(cb,59) { BIT(z80, 3, z80->C);													} /* BIT  3,C         */
OP(cb,5a) { BIT(z80, 3, z80->D);													} /* BIT  3,D         */
OP(cb,5b) { BIT(z80, 3, z80->E);													} /* BIT  3,E         */
OP(cb,5c) { BIT(z80, 3, z80->H);													} /* BIT  3,H         */
OP(cb,5d) { BIT(z80, 3, z80->L);													} /* BIT  3,L         */
OP(cb,5e) { BIT_HL(z80, 3, RM(z80, z80->HL));										} /* BIT  3,(HL)      */
OP(cb,5f) { BIT(z80, 3, z80->A);													} /* BIT  3,A         */

OP(cb,60) { BIT(z80, 4, z80->B);													} /* BIT  4,B         */
OP(cb,61) { BIT(z80, 4, z80->C);													} /* BIT  4,C         */
OP(cb,62) { BIT(z80, 4, z80->D);													} /* BIT  4,D         */
OP(cb,63) { BIT(z80, 4, z80->E);													} /* BIT  4,E         */
OP(cb,64) { BIT(z80, 4, z80->H);													} /* BIT  4,H         */
OP(cb,65) { BIT(z80, 4, z80->L);													} /* BIT  4,L         */
OP(cb,66) { BIT_HL(z80, 4, RM(z80, z80->HL));										} /* BIT  4,(HL)      */
OP(cb,67) { BIT(z80, 4, z80->A);													} /* BIT  4,A         */

OP(cb,68) { BIT(z80, 5, z80->B);													} /* BIT  5,B         */
OP(cb,69) { BIT(z80, 5, z80->C);													} /* BIT  5,C         */
OP(cb,6a) { BIT(z80, 5, z80->D);													} /* BIT  5,D         */
OP(cb,6b) { BIT(z80, 5, z80->E);													} /* BIT  5,E         */
OP(cb,6c) { BIT(z80, 5, z80->H);													} /* BIT  5,H         */
OP(cb,6d) { BIT(z80, 5, z80->L);													} /* BIT  5,L         */
OP(cb,6e) { BIT_HL(z80, 5, RM(z80, z80->HL));										} /* BIT  5,(HL)      */
OP(cb,6f) { BIT(z80, 5, z80->A);													} /* BIT  5,A         */

OP(cb,70) { BIT(z80, 6, z80->B);													} /* BIT  6,B         */
OP(cb,71) { BIT(z80, 6, z80->C);													} /* BIT  6,C         */
OP(cb,72) { BIT(z80, 6, z80->D);													} /* BIT  6,D         */
OP(cb,73) { BIT(z80, 6, z80->E);													} /* BIT  6,E         */
OP(cb,74) { BIT(z80, 6, z80->H);													} /* BIT  6,H         */
OP(cb,75) { BIT(z80, 6, z80->L);													} /* BIT  6,L         */
OP(cb,76) { BIT_HL(z80, 6, RM(z80, z80->HL));										} /* BIT  6,(HL)      */
OP(cb,77) { BIT(z80, 6, z80->A);													} /* BIT  6,A         */

OP(cb,78) { BIT(z80, 7, z80->B);													} /* BIT  7,B         */
OP(cb,79) { BIT(z80, 7, z80->C);													} /* BIT  7,C         */
OP(cb,7a) { BIT(z80, 7, z80->D);													} /* BIT  7,D         */
OP(cb,7b) { BIT(z80, 7, z80->E);													} /* BIT  7,E         */
OP(cb,7c) { BIT(z80, 7, z80->H);													} /* BIT  7,H         */
OP(cb,7d) { BIT(z80, 7, z80->L);													} /* BIT  7,L         */
OP(cb,7e) { BIT_HL(z80, 7, RM(z80, z80->HL));										} /* BIT  7,(HL)      */
OP(cb,7f) { BIT(z80, 7, z80->A);													} /* BIT  7,A         */

OP(cb,80) { z80->B = RES(0, z80->B);												} /* RES  0,B         */
OP(cb,81) { z80->C = RES(0, z80->C);												} /* RES  0,C         */
OP(cb,82) { z80->D = RES(0, z80->D);												} /* RES  0,D         */
OP(cb,83) { z80->E = RES(0, z80->E);												} /* RES  0,E         */
OP(cb,84) { z80->H = RES(0, z80->H);												} /* RES  0,H         */
OP(cb,85) { z80->L = RES(0, z80->L);												} /* RES  0,L         */
OP(cb,86) { WM(z80, z80->HL, RES(0, RM(z80, z80->HL)));								} /* RES  0,(HL)      */
OP(cb,87) { z80->A = RES(0, z80->A);												} /* RES  0,A         */

OP(cb,88) { z80->B = RES(1, z80->B);												} /* RES  1,B         */
OP(cb,89) { z80->C = RES(1, z80->C);												} /* RES  1,C         */
OP(cb,8a) { z80->D = RES(1, z80->D);												} /* RES  1,D         */
OP(cb,8b) { z80->E = RES(1, z80->E);												} /* RES  1,E         */
OP(cb,8c) { z80->H = RES(1, z80->H);												} /* RES  1,H         */
OP(cb,8d) { z80->L = RES(1, z80->L);												} /* RES  1,L         */
OP(cb,8e) { WM(z80, z80->HL, RES(1, RM(z80, z80->HL)));								} /* RES  1,(HL)      */
OP(cb,8f) { z80->A = RES(1, z80->A);												} /* RES  1,A         */

OP(cb,90) { z80->B = RES(2, z80->B);												} /* RES  2,B         */
OP(cb,91) { z80->C = RES(2, z80->C);												} /* RES  2,C         */
OP(cb,92) { z80->D = RES(2, z80->D);												} /* RES  2,D         */
OP(cb,93) { z80->E = RES(2, z80->E);												} /* RES  2,E         */
OP(cb,94) { z80->H = RES(2, z80->H);												} /* RES  2,H         */
OP(cb,95) { z80->L = RES(2, z80->L);												} /* RES  2,L         */
OP(cb,96) { WM(z80, z80->HL, RES(2, RM(z80, z80->HL)));								} /* RES  2,(HL)      */
OP(cb,97) { z80->A = RES(2, z80->A);												} /* RES  2,A         */

OP(cb,98) { z80->B = RES(3, z80->B);												} /* RES  3,B         */
OP(cb,99) { z80->C = RES(3, z80->C);												} /* RES  3,C         */
OP(cb,9a) { z80->D = RES(3, z80->D);												} /* RES  3,D         */
OP(cb,9b) { z80->E = RES(3, z80->E);												} /* RES  3,E         */
OP(cb,9c) { z80->H = RES(3, z80->H);												} /* RES  3,H         */
OP(cb,9d) { z80->L = RES(3, z80->L);												} /* RES  3,L         */
OP(cb,9e) { WM(z80, z80->HL, RES(3, RM(z80, z80->HL)));								} /* RES  3,(HL)      */
OP(cb,9f) { z80->A = RES(3, z80->A);												} /* RES  3,A         */

OP(cb,a0) { z80->B = RES(4,	z80->B);												} /* RES  4,B         */
OP(cb,a1) { z80->C = RES(4,	z80->C);												} /* RES  4,C         */
OP(cb,a2) { z80->D = RES(4,	z80->D);												} /* RES  4,D         */
OP(cb,a3) { z80->E = RES(4,	z80->E);												} /* RES  4,E         */
OP(cb,a4) { z80->H = RES(4,	z80->H);												} /* RES  4,H         */
OP(cb,a5) { z80->L = RES(4,	z80->L);												} /* RES  4,L         */
OP(cb,a6) { WM(z80, z80->HL, RES(4,	RM(z80, z80->HL)));								} /* RES  4,(HL)      */
OP(cb,a7) { z80->A = RES(4,	z80->A);												} /* RES  4,A         */

OP(cb,a8) { z80->B = RES(5, z80->B);												} /* RES  5,B         */
OP(cb,a9) { z80->C = RES(5, z80->C);												} /* RES  5,C         */
OP(cb,aa) { z80->D = RES(5, z80->D);												} /* RES  5,D         */
OP(cb,ab) { z80->E = RES(5, z80->E);												} /* RES  5,E         */
OP(cb,ac) { z80->H = RES(5, z80->H);												} /* RES  5,H         */
OP(cb,ad) { z80->L = RES(5, z80->L);												} /* RES  5,L         */
OP(cb,ae) { WM(z80, z80->HL, RES(5, RM(z80, z80->HL)));								} /* RES  5,(HL)      */
OP(cb,af) { z80->A = RES(5, z80->A);												} /* RES  5,A         */

OP(cb,b0) { z80->B = RES(6, z80->B);												} /* RES  6,B         */
OP(cb,b1) { z80->C = RES(6, z80->C);												} /* RES  6,C         */
OP(cb,b2) { z80->D = RES(6, z80->D);												} /* RES  6,D         */
OP(cb,b3) { z80->E = RES(6, z80->E);												} /* RES  6,E         */
OP(cb,b4) { z80->H = RES(6, z80->H);												} /* RES  6,H         */
OP(cb,b5) { z80->L = RES(6, z80->L);												} /* RES  6,L         */
OP(cb,b6) { WM(z80, z80->HL, RES(6, RM(z80, z80->HL)));								} /* RES  6,(HL)      */
OP(cb,b7) { z80->A = RES(6, z80->A);												} /* RES  6,A         */

OP(cb,b8) { z80->B = RES(7, z80->B);												} /* RES  7,B         */
OP(cb,b9) { z80->C = RES(7, z80->C);												} /* RES  7,C         */
OP(cb,ba) { z80->D = RES(7, z80->D);												} /* RES  7,D         */
OP(cb,bb) { z80->E = RES(7, z80->E);												} /* RES  7,E         */
OP(cb,bc) { z80->H = RES(7, z80->H);												} /* RES  7,H         */
OP(cb,bd) { z80->L = RES(7, z80->L);												} /* RES  7,L         */
OP(cb,be) { WM(z80, z80->HL, RES(7, RM(z80, z80->HL)));								} /* RES  7,(HL)      */
OP(cb,bf) { z80->A = RES(7, z80->A);												} /* RES  7,A         */

OP(cb,c0) { z80->B = SET(0, z80->B);												} /* SET  0,B         */
OP(cb,c1) { z80->C = SET(0, z80->C);												} /* SET  0,C         */
OP(cb,c2) { z80->D = SET(0, z80->D);												} /* SET  0,D         */
OP(cb,c3) { z80->E = SET(0, z80->E);												} /* SET  0,E         */
OP(cb,c4) { z80->H = SET(0, z80->H);												} /* SET  0,H         */
OP(cb,c5) { z80->L = SET(0, z80->L);												} /* SET  0,L         */
OP(cb,c6) { WM(z80, z80->HL, SET(0, RM(z80, z80->HL)));								} /* SET  0,(HL)      */
OP(cb,c7) { z80->A = SET(0, z80->A);												} /* SET  0,A         */

OP(cb,c8) { z80->B = SET(1, z80->B);												} /* SET  1,B         */
OP(cb,c9) { z80->C = SET(1, z80->C);												} /* SET  1,C         */
OP(cb,ca) { z80->D = SET(1, z80->D);												} /* SET  1,D         */
OP(cb,cb) { z80->E = SET(1, z80->E);												} /* SET  1,E         */
OP(cb,cc) { z80->H = SET(1, z80->H);												} /* SET  1,H         */
OP(cb,cd) { z80->L = SET(1, z80->L);												} /* SET  1,L         */
OP(cb,ce) { WM(z80, z80->HL, SET(1, RM(z80, z80->HL)));								} /* SET  1,(HL)      */
OP(cb,cf) { z80->A = SET(1, z80->A);												} /* SET  1,A         */

OP(cb,d0) { z80->B = SET(2, z80->B);												} /* SET  2,B         */
OP(cb,d1) { z80->C = SET(2, z80->C);												} /* SET  2,C         */
OP(cb,d2) { z80->D = SET(2, z80->D);												} /* SET  2,D         */
OP(cb,d3) { z80->E = SET(2, z80->E);												} /* SET  2,E         */
OP(cb,d4) { z80->H = SET(2, z80->H);												} /* SET  2,H         */
OP(cb,d5) { z80->L = SET(2, z80->L);												} /* SET  2,L         */
OP(cb,d6) { WM(z80, z80->HL, SET(2, RM(z80, z80->HL)));								} /* SET  2,(HL)      */
OP(cb,d7) { z80->A = SET(2, z80->A);												} /* SET  2,A         */

OP(cb,d8) { z80->B = SET(3, z80->B);												} /* SET  3,B         */
OP(cb,d9) { z80->C = SET(3, z80->C);												} /* SET  3,C         */
OP(cb,da) { z80->D = SET(3, z80->D);												} /* SET  3,D         */
OP(cb,db) { z80->E = SET(3, z80->E);												} /* SET  3,E         */
OP(cb,dc) { z80->H = SET(3, z80->H);												} /* SET  3,H         */
OP(cb,dd) { z80->L = SET(3, z80->L);												} /* SET  3,L         */
OP(cb,de) { WM(z80, z80->HL, SET(3, RM(z80, z80->HL)));								} /* SET  3,(HL)      */
OP(cb,df) { z80->A = SET(3, z80->A);												} /* SET  3,A         */

OP(cb,e0) { z80->B = SET(4, z80->B);												} /* SET  4,B         */
OP(cb,e1) { z80->C = SET(4, z80->C);												} /* SET  4,C         */
OP(cb,e2) { z80->D = SET(4, z80->D);												} /* SET  4,D         */
OP(cb,e3) { z80->E = SET(4, z80->E);												} /* SET  4,E         */
OP(cb,e4) { z80->H = SET(4, z80->H);												} /* SET  4,H         */
OP(cb,e5) { z80->L = SET(4, z80->L);												} /* SET  4,L         */
OP(cb,e6) { WM(z80, z80->HL, SET(4, RM(z80, z80->HL)));								} /* SET  4,(HL)      */
OP(cb,e7) { z80->A = SET(4, z80->A);												} /* SET  4,A         */

OP(cb,e8) { z80->B = SET(5, z80->B);												} /* SET  5,B         */
OP(cb,e9) { z80->C = SET(5, z80->C);												} /* SET  5,C         */
OP(cb,ea) { z80->D = SET(5, z80->D);												} /* SET  5,D         */
OP(cb,eb) { z80->E = SET(5, z80->E);												} /* SET  5,E         */
OP(cb,ec) { z80->H = SET(5, z80->H);												} /* SET  5,H         */
OP(cb,ed) { z80->L = SET(5, z80->L);												} /* SET  5,L         */
OP(cb,ee) { WM(z80, z80->HL, SET(5, RM(z80, z80->HL)));								} /* SET  5,(HL)      */
OP(cb,ef) { z80->A = SET(5, z80->A);												} /* SET  5,A         */

OP(cb,f0) { z80->B = SET(6, z80->B);												} /* SET  6,B         */
OP(cb,f1) { z80->C = SET(6, z80->C);												} /* SET  6,C         */
OP(cb,f2) { z80->D = SET(6, z80->D);												} /* SET  6,D         */
OP(cb,f3) { z80->E = SET(6, z80->E);												} /* SET  6,E         */
OP(cb,f4) { z80->H = SET(6, z80->H);												} /* SET  6,H         */
OP(cb,f5) { z80->L = SET(6, z80->L);												} /* SET  6,L         */
OP(cb,f6) { WM(z80, z80->HL, SET(6, RM(z80, z80->HL)));								} /* SET  6,(HL)      */
OP(cb,f7) { z80->A = SET(6, z80->A);												} /* SET  6,A         */

OP(cb,f8) { z80->B = SET(7, z80->B);												} /* SET  7,B         */
OP(cb,f9) { z80->C = SET(7, z80->C);												} /* SET  7,C         */
OP(cb,fa) { z80->D = SET(7, z80->D);												} /* SET  7,D         */
OP(cb,fb) { z80->E = SET(7, z80->E);												} /* SET  7,E         */
OP(cb,fc) { z80->H = SET(7, z80->H);												} /* SET  7,H         */
OP(cb,fd) { z80->L = SET(7, z80->L);												} /* SET  7,L         */
OP(cb,fe) { WM(z80, z80->HL, SET(7, RM(z80, z80->HL)));								} /* SET  7,(HL)      */
OP(cb,ff) { z80->A = SET(7, z80->A);												} /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { z80->B = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RLC  B=(XY+o)    */
OP(xycb,01) { z80->C = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RLC  C=(XY+o)    */
OP(xycb,02) { z80->D = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RLC  D=(XY+o)    */
OP(xycb,03) { z80->E = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RLC  E=(XY+o)    */
OP(xycb,04) { z80->H = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RLC  H=(XY+o)    */
OP(xycb,05) { z80->L = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RLC  L=(XY+o)    */
OP(xycb,06) { WM(z80, z80->ea, RLC(z80, RM(z80, z80->ea)));							} /* RLC  (XY+o)      */
OP(xycb,07) { z80->A = RLC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RLC  A=(XY+o)    */

OP(xycb,08) { z80->B = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RRC  B=(XY+o)    */
OP(xycb,09) { z80->C = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RRC  C=(XY+o)    */
OP(xycb,0a) { z80->D = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RRC  D=(XY+o)    */
OP(xycb,0b) { z80->E = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RRC  E=(XY+o)    */
OP(xycb,0c) { z80->H = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RRC  H=(XY+o)    */
OP(xycb,0d) { z80->L = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM(z80, z80->ea,RRC(z80, RM(z80, z80->ea)));							} /* RRC  (XY+o)      */
OP(xycb,0f) { z80->A = RRC(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RRC  A=(XY+o)    */

OP(xycb,10) { z80->B = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RL   B=(XY+o)    */
OP(xycb,11) { z80->C = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RL   C=(XY+o)    */
OP(xycb,12) { z80->D = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RL   D=(XY+o)    */
OP(xycb,13) { z80->E = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RL   E=(XY+o)    */
OP(xycb,14) { z80->H = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RL   H=(XY+o)    */
OP(xycb,15) { z80->L = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RL   L=(XY+o)    */
OP(xycb,16) { WM(z80, z80->ea,RL(z80, RM(z80, z80->ea)));							} /* RL   (XY+o)      */
OP(xycb,17) { z80->A = RL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RL   A=(XY+o)    */

OP(xycb,18) { z80->B = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RR   B=(XY+o)    */
OP(xycb,19) { z80->C = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RR   C=(XY+o)    */
OP(xycb,1a) { z80->D = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RR   D=(XY+o)    */
OP(xycb,1b) { z80->E = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RR   E=(XY+o)    */
OP(xycb,1c) { z80->H = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RR   H=(XY+o)    */
OP(xycb,1d) { z80->L = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RR   L=(XY+o)    */
OP(xycb,1e) { WM(z80, z80->ea,RR(z80, RM(z80, z80->ea)));							} /* RR   (XY+o)      */
OP(xycb,1f) { z80->A = RR(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RR   A=(XY+o)    */

OP(xycb,20) { z80->B = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SLA  B=(XY+o)    */
OP(xycb,21) { z80->C = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SLA  C=(XY+o)    */
OP(xycb,22) { z80->D = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SLA  D=(XY+o)    */
OP(xycb,23) { z80->E = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SLA  E=(XY+o)    */
OP(xycb,24) { z80->H = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SLA  H=(XY+o)    */
OP(xycb,25) { z80->L = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SLA  L=(XY+o)    */
OP(xycb,26) { WM(z80, z80->ea,SLA(z80, RM(z80, z80->ea)));							} /* SLA  (XY+o)      */
OP(xycb,27) { z80->A = SLA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SLA  A=(XY+o)    */

OP(xycb,28) { z80->B = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SRA  B=(XY+o)    */
OP(xycb,29) { z80->C = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SRA  C=(XY+o)    */
OP(xycb,2a) { z80->D = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SRA  D=(XY+o)    */
OP(xycb,2b) { z80->E = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SRA  E=(XY+o)    */
OP(xycb,2c) { z80->H = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SRA  H=(XY+o)    */
OP(xycb,2d) { z80->L = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM(z80, z80->ea,SRA(z80, RM(z80, z80->ea)));							} /* SRA  (XY+o)      */
OP(xycb,2f) { z80->A = SRA(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SRA  A=(XY+o)    */

OP(xycb,30) { z80->B = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SLL  B=(XY+o)    */
OP(xycb,31) { z80->C = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SLL  C=(XY+o)    */
OP(xycb,32) { z80->D = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SLL  D=(XY+o)    */
OP(xycb,33) { z80->E = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SLL  E=(XY+o)    */
OP(xycb,34) { z80->H = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SLL  H=(XY+o)    */
OP(xycb,35) { z80->L = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SLL  L=(XY+o)    */
OP(xycb,36) { WM(z80, z80->ea,SLL(z80, RM(z80, z80->ea)));							} /* SLL  (XY+o)      */
OP(xycb,37) { z80->A = SLL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SLL  A=(XY+o)    */

OP(xycb,38) { z80->B = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SRL  B=(XY+o)    */
OP(xycb,39) { z80->C = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SRL  C=(XY+o)    */
OP(xycb,3a) { z80->D = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SRL  D=(XY+o)    */
OP(xycb,3b) { z80->E = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SRL  E=(XY+o)    */
OP(xycb,3c) { z80->H = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SRL  H=(XY+o)    */
OP(xycb,3d) { z80->L = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM(z80, z80->ea,SRL(z80, RM(z80, z80->ea)));							} /* SRL  (XY+o)      */
OP(xycb,3f) { z80->A = SRL(z80, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,41) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,42) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,43) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,44) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,45) { xycb_46(z80);															} /* BIT  0,(XY+o)    */
OP(xycb,46) { BIT_XY(z80, 0, RM(z80, z80->ea));										} /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46(z80);															} /* BIT  0,(XY+o)    */

OP(xycb,48) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,49) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,4a) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,4b) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,4c) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,4d) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */
OP(xycb,4e) { BIT_XY(z80, 1, RM(z80, z80->ea));										} /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e(z80);															} /* BIT  1,(XY+o)    */

OP(xycb,50) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,51) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,52) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,53) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,54) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,55) { xycb_56(z80);															} /* BIT  2,(XY+o)    */
OP(xycb,56) { BIT_XY(z80, 2, RM(z80, z80->ea));										} /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56(z80);															} /* BIT  2,(XY+o)    */

OP(xycb,58) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,59) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,5a) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,5b) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,5c) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,5d) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */
OP(xycb,5e) { BIT_XY(z80, 3, RM(z80, z80->ea));										} /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e(z80);															} /* BIT  3,(XY+o)    */

OP(xycb,60) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,61) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,62) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,63) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,64) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,65) { xycb_66(z80);															} /* BIT  4,(XY+o)    */
OP(xycb,66) { BIT_XY(z80, 4, RM(z80, z80->ea));										} /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66(z80);															} /* BIT  4,(XY+o)    */

OP(xycb,68) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,69) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,6a) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,6b) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,6c) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,6d) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */
OP(xycb,6e) { BIT_XY(z80, 5, RM(z80, z80->ea));										} /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e(z80);															} /* BIT  5,(XY+o)    */

OP(xycb,70) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,71) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,72) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,73) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,74) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,75) { xycb_76(z80);															} /* BIT  6,(XY+o)    */
OP(xycb,76) { BIT_XY(z80, 6, RM(z80, z80->ea));										} /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76(z80);															} /* BIT  6,(XY+o)    */

OP(xycb,78) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,79) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,7a) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,7b) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,7c) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,7d) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */
OP(xycb,7e) { BIT_XY(z80, 7, RM(z80, z80->ea));										} /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e(z80);															} /* BIT  7,(XY+o)    */

OP(xycb,80) { z80->B = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  0,B=(XY+o)  */
OP(xycb,81) { z80->C = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  0,C=(XY+o)  */
OP(xycb,82) { z80->D = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  0,D=(XY+o)  */
OP(xycb,83) { z80->E = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  0,E=(XY+o)  */
OP(xycb,84) { z80->H = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  0,H=(XY+o)  */
OP(xycb,85) { z80->L = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM(z80, z80->ea, RES(0, RM(z80, z80->ea)));							} /* RES  0,(XY+o)    */
OP(xycb,87) { z80->A = RES(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  0,A=(XY+o)  */

OP(xycb,88) { z80->B = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  1,B=(XY+o)  */
OP(xycb,89) { z80->C = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  1,C=(XY+o)  */
OP(xycb,8a) { z80->D = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  1,D=(XY+o)  */
OP(xycb,8b) { z80->E = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  1,E=(XY+o)  */
OP(xycb,8c) { z80->H = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  1,H=(XY+o)  */
OP(xycb,8d) { z80->L = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM(z80, z80->ea, RES(1, RM(z80, z80->ea)));							} /* RES  1,(XY+o)    */
OP(xycb,8f) { z80->A = RES(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  1,A=(XY+o)  */

OP(xycb,90) { z80->B = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  2,B=(XY+o)  */
OP(xycb,91) { z80->C = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  2,C=(XY+o)  */
OP(xycb,92) { z80->D = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  2,D=(XY+o)  */
OP(xycb,93) { z80->E = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  2,E=(XY+o)  */
OP(xycb,94) { z80->H = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  2,H=(XY+o)  */
OP(xycb,95) { z80->L = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM(z80, z80->ea, RES(2, RM(z80, z80->ea)));							} /* RES  2,(XY+o)    */
OP(xycb,97) { z80->A = RES(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  2,A=(XY+o)  */

OP(xycb,98) { z80->B = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  3,B=(XY+o)  */
OP(xycb,99) { z80->C = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  3,C=(XY+o)  */
OP(xycb,9a) { z80->D = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  3,D=(XY+o)  */
OP(xycb,9b) { z80->E = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  3,E=(XY+o)  */
OP(xycb,9c) { z80->H = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  3,H=(XY+o)  */
OP(xycb,9d) { z80->L = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM(z80, z80->ea, RES(3, RM(z80, z80->ea)));							} /* RES  3,(XY+o)    */
OP(xycb,9f) { z80->A = RES(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  3,A=(XY+o)  */

OP(xycb,a0) { z80->B = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  4,B=(XY+o)  */
OP(xycb,a1) { z80->C = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  4,C=(XY+o)  */
OP(xycb,a2) { z80->D = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  4,D=(XY+o)  */
OP(xycb,a3) { z80->E = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  4,E=(XY+o)  */
OP(xycb,a4) { z80->H = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  4,H=(XY+o)  */
OP(xycb,a5) { z80->L = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM(z80, z80->ea, RES(4, RM(z80, z80->ea)));							} /* RES  4,(XY+o)    */
OP(xycb,a7) { z80->A = RES(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  4,A=(XY+o)  */

OP(xycb,a8) { z80->B = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  5,B=(XY+o)  */
OP(xycb,a9) { z80->C = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  5,C=(XY+o)  */
OP(xycb,aa) { z80->D = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  5,D=(XY+o)  */
OP(xycb,ab) { z80->E = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  5,E=(XY+o)  */
OP(xycb,ac) { z80->H = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  5,H=(XY+o)  */
OP(xycb,ad) { z80->L = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM(z80, z80->ea, RES(5, RM(z80, z80->ea)));							} /* RES  5,(XY+o)    */
OP(xycb,af) { z80->A = RES(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  5,A=(XY+o)  */

OP(xycb,b0) { z80->B = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  6,B=(XY+o)  */
OP(xycb,b1) { z80->C = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  6,C=(XY+o)  */
OP(xycb,b2) { z80->D = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  6,D=(XY+o)  */
OP(xycb,b3) { z80->E = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  6,E=(XY+o)  */
OP(xycb,b4) { z80->H = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  6,H=(XY+o)  */
OP(xycb,b5) { z80->L = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM(z80, z80->ea, RES(6, RM(z80, z80->ea)));							} /* RES  6,(XY+o)    */
OP(xycb,b7) { z80->A = RES(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  6,A=(XY+o)  */

OP(xycb,b8) { z80->B = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* RES  7,B=(XY+o)  */
OP(xycb,b9) { z80->C = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* RES  7,C=(XY+o)  */
OP(xycb,ba) { z80->D = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* RES  7,D=(XY+o)  */
OP(xycb,bb) { z80->E = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* RES  7,E=(XY+o)  */
OP(xycb,bc) { z80->H = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* RES  7,H=(XY+o)  */
OP(xycb,bd) { z80->L = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM(z80, z80->ea, RES(7, RM(z80, z80->ea)));							} /* RES  7,(XY+o)    */
OP(xycb,bf) { z80->A = RES(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* RES  7,A=(XY+o)  */

OP(xycb,c0) { z80->B = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  0,B=(XY+o)  */
OP(xycb,c1) { z80->C = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  0,C=(XY+o)  */
OP(xycb,c2) { z80->D = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  0,D=(XY+o)  */
OP(xycb,c3) { z80->E = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  0,E=(XY+o)  */
OP(xycb,c4) { z80->H = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  0,H=(XY+o)  */
OP(xycb,c5) { z80->L = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM(z80, z80->ea, SET(0, RM(z80, z80->ea)));							} /* SET  0,(XY+o)    */
OP(xycb,c7) { z80->A = SET(0, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  0,A=(XY+o)  */

OP(xycb,c8) { z80->B = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  1,B=(XY+o)  */
OP(xycb,c9) { z80->C = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  1,C=(XY+o)  */
OP(xycb,ca) { z80->D = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  1,D=(XY+o)  */
OP(xycb,cb) { z80->E = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  1,E=(XY+o)  */
OP(xycb,cc) { z80->H = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  1,H=(XY+o)  */
OP(xycb,cd) { z80->L = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM(z80, z80->ea, SET(1, RM(z80, z80->ea)));							} /* SET  1,(XY+o)    */
OP(xycb,cf) { z80->A = SET(1, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  1,A=(XY+o)  */

OP(xycb,d0) { z80->B = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  2,B=(XY+o)  */
OP(xycb,d1) { z80->C = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  2,C=(XY+o)  */
OP(xycb,d2) { z80->D = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  2,D=(XY+o)  */
OP(xycb,d3) { z80->E = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  2,E=(XY+o)  */
OP(xycb,d4) { z80->H = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  2,H=(XY+o)  */
OP(xycb,d5) { z80->L = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM(z80, z80->ea, SET(2, RM(z80, z80->ea)));							} /* SET  2,(XY+o)    */
OP(xycb,d7) { z80->A = SET(2, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  2,A=(XY+o)  */

OP(xycb,d8) { z80->B = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  3,B=(XY+o)  */
OP(xycb,d9) { z80->C = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  3,C=(XY+o)  */
OP(xycb,da) { z80->D = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  3,D=(XY+o)  */
OP(xycb,db) { z80->E = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  3,E=(XY+o)  */
OP(xycb,dc) { z80->H = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  3,H=(XY+o)  */
OP(xycb,dd) { z80->L = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM(z80, z80->ea, SET(3, RM(z80, z80->ea)));							} /* SET  3,(XY+o)    */
OP(xycb,df) { z80->A = SET(3, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  3,A=(XY+o)  */

OP(xycb,e0) { z80->B = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  4,B=(XY+o)  */
OP(xycb,e1) { z80->C = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  4,C=(XY+o)  */
OP(xycb,e2) { z80->D = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  4,D=(XY+o)  */
OP(xycb,e3) { z80->E = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  4,E=(XY+o)  */
OP(xycb,e4) { z80->H = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  4,H=(XY+o)  */
OP(xycb,e5) { z80->L = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM(z80, z80->ea, SET(4, RM(z80, z80->ea)));							} /* SET  4,(XY+o)    */
OP(xycb,e7) { z80->A = SET(4, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  4,A=(XY+o)  */

OP(xycb,e8) { z80->B = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  5,B=(XY+o)  */
OP(xycb,e9) { z80->C = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  5,C=(XY+o)  */
OP(xycb,ea) { z80->D = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  5,D=(XY+o)  */
OP(xycb,eb) { z80->E = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  5,E=(XY+o)  */
OP(xycb,ec) { z80->H = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  5,H=(XY+o)  */
OP(xycb,ed) { z80->L = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM(z80, z80->ea, SET(5, RM(z80, z80->ea)));							} /* SET  5,(XY+o)    */
OP(xycb,ef) { z80->A = SET(5, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  5,A=(XY+o)  */

OP(xycb,f0) { z80->B = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  6,B=(XY+o)  */
OP(xycb,f1) { z80->C = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  6,C=(XY+o)  */
OP(xycb,f2) { z80->D = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  6,D=(XY+o)  */
OP(xycb,f3) { z80->E = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  6,E=(XY+o)  */
OP(xycb,f4) { z80->H = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  6,H=(XY+o)  */
OP(xycb,f5) { z80->L = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM(z80, z80->ea, SET(6, RM(z80, z80->ea)));							} /* SET  6,(XY+o)    */
OP(xycb,f7) { z80->A = SET(6, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  6,A=(XY+o)  */

OP(xycb,f8) { z80->B = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->B);			} /* SET  7,B=(XY+o)  */
OP(xycb,f9) { z80->C = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->C);			} /* SET  7,C=(XY+o)  */
OP(xycb,fa) { z80->D = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->D);			} /* SET  7,D=(XY+o)  */
OP(xycb,fb) { z80->E = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->E);			} /* SET  7,E=(XY+o)  */
OP(xycb,fc) { z80->H = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->H);			} /* SET  7,H=(XY+o)  */
OP(xycb,fd) { z80->L = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->L);			} /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM(z80, z80->ea, SET(7, RM(z80, z80->ea)));							} /* SET  7,(XY+o)    */
OP(xycb,ff) { z80->A = SET(7, RM(z80, z80->ea)); WM(z80, z80->ea,z80->A);			} /* SET  7,A=(XY+o)  */

OP(illegal,1) {
	logerror("Z80 '%s' ill. opcode $%02x $%02x\n",
			z80->device->tag(), z80->direct->read_decrypted_byte((z80->PCD-1)&0xffff), z80->direct->read_decrypted_byte(z80->PCD));
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP(dd,00) { illegal_1(z80); op_00(z80);												} /* DB   DD          */
OP(dd,01) { illegal_1(z80); op_01(z80);												} /* DB   DD          */
OP(dd,02) { illegal_1(z80); op_02(z80);												} /* DB   DD          */
OP(dd,03) { illegal_1(z80); op_03(z80);												} /* DB   DD          */
OP(dd,04) { illegal_1(z80); op_04(z80);												} /* DB   DD          */
OP(dd,05) { illegal_1(z80); op_05(z80);												} /* DB   DD          */
OP(dd,06) { illegal_1(z80); op_06(z80);												} /* DB   DD          */
OP(dd,07) { illegal_1(z80); op_07(z80);												} /* DB   DD          */

OP(dd,08) { illegal_1(z80); op_08(z80);												} /* DB   DD          */
OP(dd,09) { ADD16(z80, ix, bc);														} /* ADD  IX,BC       */
OP(dd,0a) { illegal_1(z80); op_0a(z80);												} /* DB   DD          */
OP(dd,0b) { illegal_1(z80); op_0b(z80);												} /* DB   DD          */
OP(dd,0c) { illegal_1(z80); op_0c(z80);												} /* DB   DD          */
OP(dd,0d) { illegal_1(z80); op_0d(z80);												} /* DB   DD          */
OP(dd,0e) { illegal_1(z80); op_0e(z80);												} /* DB   DD          */
OP(dd,0f) { illegal_1(z80); op_0f(z80);												} /* DB   DD          */

OP(dd,10) { illegal_1(z80); op_10(z80);												} /* DB   DD          */
OP(dd,11) { illegal_1(z80); op_11(z80);												} /* DB   DD          */
OP(dd,12) { illegal_1(z80); op_12(z80);												} /* DB   DD          */
OP(dd,13) { illegal_1(z80); op_13(z80);												} /* DB   DD          */
OP(dd,14) { illegal_1(z80); op_14(z80);												} /* DB   DD          */
OP(dd,15) { illegal_1(z80); op_15(z80);												} /* DB   DD          */
OP(dd,16) { illegal_1(z80); op_16(z80);												} /* DB   DD          */
OP(dd,17) { illegal_1(z80); op_17(z80);												} /* DB   DD          */

OP(dd,18) { illegal_1(z80); op_18(z80);												} /* DB   DD          */
OP(dd,19) { ADD16(z80, ix, de);														} /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(z80); op_1a(z80);												} /* DB   DD          */
OP(dd,1b) { illegal_1(z80); op_1b(z80);												} /* DB   DD          */
OP(dd,1c) { illegal_1(z80); op_1c(z80);												} /* DB   DD          */
OP(dd,1d) { illegal_1(z80); op_1d(z80);												} /* DB   DD          */
OP(dd,1e) { illegal_1(z80); op_1e(z80);												} /* DB   DD          */
OP(dd,1f) { illegal_1(z80); op_1f(z80);												} /* DB   DD          */

OP(dd,20) { illegal_1(z80); op_20(z80);												} /* DB   DD          */
OP(dd,21) { z80->IX = ARG16(z80);													} /* LD   IX,w        */
OP(dd,22) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->ix);	z80->WZ = z80->ea+1;} /* LD   (w),IX      */
OP(dd,23) { z80->IX++;																} /* INC  IX          */
OP(dd,24) { z80->HX = INC(z80, z80->HX);											} /* INC  HX          */
OP(dd,25) { z80->HX = DEC(z80, z80->HX);											} /* DEC  HX          */
OP(dd,26) { z80->HX = ARG(z80);														} /* LD   HX,n        */
OP(dd,27) { illegal_1(z80); op_27(z80);												} /* DB   DD          */

OP(dd,28) { illegal_1(z80); op_28(z80);												} /* DB   DD          */
OP(dd,29) { ADD16(z80, ix, ix);														} /* ADD  IX,IX       */
OP(dd,2a) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->ix);	z80->WZ = z80->ea+1;} /* LD   IX,(w)      */
OP(dd,2b) { z80->IX--;																} /* DEC  IX          */
OP(dd,2c) { z80->LX = INC(z80, z80->LX);											} /* INC  LX          */
OP(dd,2d) { z80->LX = DEC(z80, z80->LX);											} /* DEC  LX          */
OP(dd,2e) { z80->LX = ARG(z80);														} /* LD   LX,n        */
OP(dd,2f) { illegal_1(z80); op_2f(z80);												} /* DB   DD          */

OP(dd,30) { illegal_1(z80); op_30(z80);												} /* DB   DD          */
OP(dd,31) { illegal_1(z80); op_31(z80);												} /* DB   DD          */
OP(dd,32) { illegal_1(z80); op_32(z80);												} /* DB   DD          */
OP(dd,33) { illegal_1(z80); op_33(z80);												} /* DB   DD          */
OP(dd,34) { EAX(z80); WM(z80, z80->ea, INC(z80, RM(z80, z80->ea)));					} /* INC  (IX+o)      */
OP(dd,35) { EAX(z80); WM(z80, z80->ea, DEC(z80, RM(z80, z80->ea)));					} /* DEC  (IX+o)      */
OP(dd,36) { EAX(z80); WM(z80, z80->ea, ARG(z80));									} /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(z80); op_37(z80);												} /* DB   DD          */

OP(dd,38) { illegal_1(z80); op_38(z80);												} /* DB   DD          */
OP(dd,39) { ADD16(z80, ix, sp);														} /* ADD  IX,SP       */
OP(dd,3a) { illegal_1(z80); op_3a(z80);												} /* DB   DD          */
OP(dd,3b) { illegal_1(z80); op_3b(z80);												} /* DB   DD          */
OP(dd,3c) { illegal_1(z80); op_3c(z80);												} /* DB   DD          */
OP(dd,3d) { illegal_1(z80); op_3d(z80);												} /* DB   DD          */
OP(dd,3e) { illegal_1(z80); op_3e(z80);												} /* DB   DD          */
OP(dd,3f) { illegal_1(z80); op_3f(z80);												} /* DB   DD          */

OP(dd,40) { illegal_1(z80); op_40(z80);												} /* DB   DD          */
OP(dd,41) { illegal_1(z80); op_41(z80);												} /* DB   DD          */
OP(dd,42) { illegal_1(z80); op_42(z80);												} /* DB   DD          */
OP(dd,43) { illegal_1(z80); op_43(z80);												} /* DB   DD          */
OP(dd,44) { z80->B = z80->HX;														} /* LD   B,HX        */
OP(dd,45) { z80->B = z80->LX;														} /* LD   B,LX        */
OP(dd,46) { EAX(z80); z80->B = RM(z80, z80->ea);									} /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(z80); op_47(z80);												} /* DB   DD          */

OP(dd,48) { illegal_1(z80); op_48(z80);												} /* DB   DD          */
OP(dd,49) { illegal_1(z80); op_49(z80);												} /* DB   DD          */
OP(dd,4a) { illegal_1(z80); op_4a(z80);												} /* DB   DD          */
OP(dd,4b) { illegal_1(z80); op_4b(z80);												} /* DB   DD          */
OP(dd,4c) { z80->C = z80->HX;														} /* LD   C,HX        */
OP(dd,4d) { z80->C = z80->LX;														} /* LD   C,LX        */
OP(dd,4e) { EAX(z80); z80->C = RM(z80, z80->ea);									} /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(z80); op_4f(z80);												} /* DB   DD          */

OP(dd,50) { illegal_1(z80); op_50(z80);												} /* DB   DD          */
OP(dd,51) { illegal_1(z80); op_51(z80);												} /* DB   DD          */
OP(dd,52) { illegal_1(z80); op_52(z80);												} /* DB   DD          */
OP(dd,53) { illegal_1(z80); op_53(z80);												} /* DB   DD          */
OP(dd,54) { z80->D = z80->HX;														} /* LD   D,HX        */
OP(dd,55) { z80->D = z80->LX;														} /* LD   D,LX        */
OP(dd,56) { EAX(z80); z80->D = RM(z80, z80->ea);									} /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(z80); op_57(z80);												} /* DB   DD          */

OP(dd,58) { illegal_1(z80); op_58(z80);												} /* DB   DD          */
OP(dd,59) { illegal_1(z80); op_59(z80);												} /* DB   DD          */
OP(dd,5a) { illegal_1(z80); op_5a(z80);												} /* DB   DD          */
OP(dd,5b) { illegal_1(z80); op_5b(z80);												} /* DB   DD          */
OP(dd,5c) { z80->E = z80->HX;														} /* LD   E,HX        */
OP(dd,5d) { z80->E = z80->LX;														} /* LD   E,LX        */
OP(dd,5e) { EAX(z80); z80->E = RM(z80, z80->ea);									} /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(z80); op_5f(z80);												} /* DB   DD          */

OP(dd,60) { z80->HX = z80->B;														} /* LD   HX,B        */
OP(dd,61) { z80->HX = z80->C;														} /* LD   HX,C        */
OP(dd,62) { z80->HX = z80->D;														} /* LD   HX,D        */
OP(dd,63) { z80->HX = z80->E;														} /* LD   HX,E        */
OP(dd,64) {																			} /* LD   HX,HX       */
OP(dd,65) { z80->HX = z80->LX;														} /* LD   HX,LX       */
OP(dd,66) { EAX(z80); z80->H = RM(z80, z80->ea);									} /* LD   H,(IX+o)    */
OP(dd,67) { z80->HX = z80->A;														} /* LD   HX,A        */

OP(dd,68) { z80->LX = z80->B;														} /* LD   LX,B        */
OP(dd,69) { z80->LX = z80->C;														} /* LD   LX,C        */
OP(dd,6a) { z80->LX = z80->D;														} /* LD   LX,D        */
OP(dd,6b) { z80->LX = z80->E;														} /* LD   LX,E        */
OP(dd,6c) { z80->LX = z80->HX;														} /* LD   LX,HX       */
OP(dd,6d) {																			} /* LD   LX,LX       */
OP(dd,6e) { EAX(z80); z80->L = RM(z80, z80->ea);									} /* LD   L,(IX+o)    */
OP(dd,6f) { z80->LX = z80->A;														} /* LD   LX,A        */

OP(dd,70) { EAX(z80); WM(z80, z80->ea, z80->B);										} /* LD   (IX+o),B    */
OP(dd,71) { EAX(z80); WM(z80, z80->ea, z80->C);										} /* LD   (IX+o),C    */
OP(dd,72) { EAX(z80); WM(z80, z80->ea, z80->D);										} /* LD   (IX+o),D    */
OP(dd,73) { EAX(z80); WM(z80, z80->ea, z80->E);										} /* LD   (IX+o),E    */
OP(dd,74) { EAX(z80); WM(z80, z80->ea, z80->H);										} /* LD   (IX+o),H    */
OP(dd,75) { EAX(z80); WM(z80, z80->ea, z80->L);										} /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(z80); op_76(z80);												} /* DB   DD          */
OP(dd,77) { EAX(z80); WM(z80, z80->ea, z80->A);										} /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(z80); op_78(z80);												} /* DB   DD          */
OP(dd,79) { illegal_1(z80); op_79(z80);												} /* DB   DD          */
OP(dd,7a) { illegal_1(z80); op_7a(z80);												} /* DB   DD          */
OP(dd,7b) { illegal_1(z80); op_7b(z80);												} /* DB   DD          */
OP(dd,7c) { z80->A = z80->HX;														} /* LD   A,HX        */
OP(dd,7d) { z80->A = z80->LX;														} /* LD   A,LX        */
OP(dd,7e) { EAX(z80); z80->A = RM(z80, z80->ea);									} /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(z80); op_7f(z80);												} /* DB   DD          */

OP(dd,80) { illegal_1(z80); op_80(z80);												} /* DB   DD          */
OP(dd,81) { illegal_1(z80); op_81(z80);												} /* DB   DD          */
OP(dd,82) { illegal_1(z80); op_82(z80);												} /* DB   DD          */
OP(dd,83) { illegal_1(z80); op_83(z80);												} /* DB   DD          */
OP(dd,84) { ADD(z80, z80->HX);														} /* ADD  A,HX        */
OP(dd,85) { ADD(z80, z80->LX);														} /* ADD  A,LX        */
OP(dd,86) { EAX(z80); ADD(z80, RM(z80, z80->ea));									} /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(z80); op_87(z80);												} /* DB   DD          */

OP(dd,88) { illegal_1(z80); op_88(z80);												} /* DB   DD          */
OP(dd,89) { illegal_1(z80); op_89(z80);												} /* DB   DD          */
OP(dd,8a) { illegal_1(z80); op_8a(z80);												} /* DB   DD          */
OP(dd,8b) { illegal_1(z80); op_8b(z80);												} /* DB   DD          */
OP(dd,8c) { ADC(z80, z80->HX);														} /* ADC  A,HX        */
OP(dd,8d) { ADC(z80, z80->LX);														} /* ADC  A,LX        */
OP(dd,8e) { EAX(z80); ADC(z80, RM(z80, z80->ea));									} /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(z80); op_8f(z80);												} /* DB   DD          */

OP(dd,90) { illegal_1(z80); op_90(z80);												} /* DB   DD          */
OP(dd,91) { illegal_1(z80); op_91(z80);												} /* DB   DD          */
OP(dd,92) { illegal_1(z80); op_92(z80);												} /* DB   DD          */
OP(dd,93) { illegal_1(z80); op_93(z80);												} /* DB   DD          */
OP(dd,94) { SUB(z80, z80->HX);														} /* SUB  HX          */
OP(dd,95) { SUB(z80, z80->LX);														} /* SUB  LX          */
OP(dd,96) { EAX(z80); SUB(z80, RM(z80, z80->ea));									} /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(z80); op_97(z80);												} /* DB   DD          */

OP(dd,98) { illegal_1(z80); op_98(z80);												} /* DB   DD          */
OP(dd,99) { illegal_1(z80); op_99(z80);												} /* DB   DD          */
OP(dd,9a) { illegal_1(z80); op_9a(z80);												} /* DB   DD          */
OP(dd,9b) { illegal_1(z80); op_9b(z80);												} /* DB   DD          */
OP(dd,9c) { SBC(z80, z80->HX);														} /* SBC  A,HX        */
OP(dd,9d) { SBC(z80, z80->LX);														} /* SBC  A,LX        */
OP(dd,9e) { EAX(z80); SBC(z80, RM(z80, z80->ea));									} /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(z80); op_9f(z80);												} /* DB   DD          */

OP(dd,a0) { illegal_1(z80); op_a0(z80);												} /* DB   DD          */
OP(dd,a1) { illegal_1(z80); op_a1(z80);												} /* DB   DD          */
OP(dd,a2) { illegal_1(z80); op_a2(z80);												} /* DB   DD          */
OP(dd,a3) { illegal_1(z80); op_a3(z80);												} /* DB   DD          */
OP(dd,a4) { AND(z80, z80->HX);														} /* AND  HX          */
OP(dd,a5) { AND(z80, z80->LX);														} /* AND  LX          */
OP(dd,a6) { EAX(z80); AND(z80, RM(z80, z80->ea));									} /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(z80); op_a7(z80);												} /* DB   DD          */

OP(dd,a8) { illegal_1(z80); op_a8(z80);												} /* DB   DD          */
OP(dd,a9) { illegal_1(z80); op_a9(z80);												} /* DB   DD          */
OP(dd,aa) { illegal_1(z80); op_aa(z80);												} /* DB   DD          */
OP(dd,ab) { illegal_1(z80); op_ab(z80);												} /* DB   DD          */
OP(dd,ac) { XOR(z80, z80->HX);														} /* XOR  HX          */
OP(dd,ad) { XOR(z80, z80->LX);														} /* XOR  LX          */
OP(dd,ae) { EAX(z80); XOR(z80, RM(z80, z80->ea));									} /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(z80); op_af(z80);												} /* DB   DD          */

OP(dd,b0) { illegal_1(z80); op_b0(z80);												} /* DB   DD          */
OP(dd,b1) { illegal_1(z80); op_b1(z80);												} /* DB   DD          */
OP(dd,b2) { illegal_1(z80); op_b2(z80);												} /* DB   DD          */
OP(dd,b3) { illegal_1(z80); op_b3(z80);												} /* DB   DD          */
OP(dd,b4) { OR(z80, z80->HX);														} /* OR   HX          */
OP(dd,b5) { OR(z80, z80->LX);														} /* OR   LX          */
OP(dd,b6) { EAX(z80); OR(z80, RM(z80, z80->ea));									} /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(z80); op_b7(z80);												} /* DB   DD          */

OP(dd,b8) { illegal_1(z80); op_b8(z80);												} /* DB   DD          */
OP(dd,b9) { illegal_1(z80); op_b9(z80);												} /* DB   DD          */
OP(dd,ba) { illegal_1(z80); op_ba(z80);												} /* DB   DD          */
OP(dd,bb) { illegal_1(z80); op_bb(z80);												} /* DB   DD          */
OP(dd,bc) { CP(z80, z80->HX);														} /* CP   HX          */
OP(dd,bd) { CP(z80, z80->LX);														} /* CP   LX          */
OP(dd,be) { EAX(z80); CP(z80, RM(z80, z80->ea));									} /* CP   (IX+o)      */
OP(dd,bf) { illegal_1(z80); op_bf(z80);												} /* DB   DD          */

OP(dd,c0) { illegal_1(z80); op_c0(z80);												} /* DB   DD          */
OP(dd,c1) { illegal_1(z80); op_c1(z80);												} /* DB   DD          */
OP(dd,c2) { illegal_1(z80); op_c2(z80);												} /* DB   DD          */
OP(dd,c3) { illegal_1(z80); op_c3(z80);												} /* DB   DD          */
OP(dd,c4) { illegal_1(z80); op_c4(z80);												} /* DB   DD          */
OP(dd,c5) { illegal_1(z80); op_c5(z80);												} /* DB   DD          */
OP(dd,c6) { illegal_1(z80); op_c6(z80);												} /* DB   DD          */
OP(dd,c7) { illegal_1(z80); op_c7(z80);												} /* DB   DD          */

OP(dd,c8) { illegal_1(z80); op_c8(z80);												} /* DB   DD          */
OP(dd,c9) { illegal_1(z80); op_c9(z80);												} /* DB   DD          */
OP(dd,ca) { illegal_1(z80); op_ca(z80);												} /* DB   DD          */
OP(dd,cb) { EAX(z80); EXEC(z80,xycb,ARG(z80));										} /* **   DD CB xx    */
OP(dd,cc) { illegal_1(z80); op_cc(z80);												} /* DB   DD          */
OP(dd,cd) { illegal_1(z80); op_cd(z80);												} /* DB   DD          */
OP(dd,ce) { illegal_1(z80); op_ce(z80);												} /* DB   DD          */
OP(dd,cf) { illegal_1(z80); op_cf(z80);												} /* DB   DD          */

OP(dd,d0) { illegal_1(z80); op_d0(z80);												} /* DB   DD          */
OP(dd,d1) { illegal_1(z80); op_d1(z80);												} /* DB   DD          */
OP(dd,d2) { illegal_1(z80); op_d2(z80);												} /* DB   DD          */
OP(dd,d3) { illegal_1(z80); op_d3(z80);												} /* DB   DD          */
OP(dd,d4) { illegal_1(z80); op_d4(z80);												} /* DB   DD          */
OP(dd,d5) { illegal_1(z80); op_d5(z80);												} /* DB   DD          */
OP(dd,d6) { illegal_1(z80); op_d6(z80);												} /* DB   DD          */
OP(dd,d7) { illegal_1(z80); op_d7(z80);												} /* DB   DD          */

OP(dd,d8) { illegal_1(z80); op_d8(z80);												} /* DB   DD          */
OP(dd,d9) { illegal_1(z80); op_d9(z80);												} /* DB   DD          */
OP(dd,da) { illegal_1(z80); op_da(z80);												} /* DB   DD          */
OP(dd,db) { illegal_1(z80); op_db(z80);												} /* DB   DD          */
OP(dd,dc) { illegal_1(z80); op_dc(z80);												} /* DB   DD          */
OP(dd,dd) { illegal_1(z80); op_dd(z80);												} /* DB   DD          */
OP(dd,de) { illegal_1(z80); op_de(z80);												} /* DB   DD          */
OP(dd,df) { illegal_1(z80); op_df(z80);												} /* DB   DD          */

OP(dd,e0) { illegal_1(z80); op_e0(z80);												} /* DB   DD          */
OP(dd,e1) { POP(z80, ix);															} /* POP  IX          */
OP(dd,e2) { illegal_1(z80); op_e2(z80);												} /* DB   DD          */
OP(dd,e3) { EXSP(z80, ix);															} /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(z80); op_e4(z80);												} /* DB   DD          */
OP(dd,e5) { PUSH(z80, ix);															} /* PUSH IX          */
OP(dd,e6) { illegal_1(z80); op_e6(z80);												} /* DB   DD          */
OP(dd,e7) { illegal_1(z80); op_e7(z80);												} /* DB   DD          */

OP(dd,e8) { illegal_1(z80); op_e8(z80);												} /* DB   DD          */
OP(dd,e9) { z80->PC = z80->IX;														} /* JP   (IX)        */
OP(dd,ea) { illegal_1(z80); op_ea(z80);												} /* DB   DD          */
OP(dd,eb) { illegal_1(z80); op_eb(z80);												} /* DB   DD          */
OP(dd,ec) { illegal_1(z80); op_ec(z80);												} /* DB   DD          */
OP(dd,ed) { illegal_1(z80); op_ed(z80);												} /* DB   DD          */
OP(dd,ee) { illegal_1(z80); op_ee(z80);												} /* DB   DD          */
OP(dd,ef) { illegal_1(z80); op_ef(z80);												} /* DB   DD          */

OP(dd,f0) { illegal_1(z80); op_f0(z80);												} /* DB   DD          */
OP(dd,f1) { illegal_1(z80); op_f1(z80);												} /* DB   DD          */
OP(dd,f2) { illegal_1(z80); op_f2(z80);												} /* DB   DD          */
OP(dd,f3) { illegal_1(z80); op_f3(z80);												} /* DB   DD          */
OP(dd,f4) { illegal_1(z80); op_f4(z80);												} /* DB   DD          */
OP(dd,f5) { illegal_1(z80); op_f5(z80);												} /* DB   DD          */
OP(dd,f6) { illegal_1(z80); op_f6(z80);												} /* DB   DD          */
OP(dd,f7) { illegal_1(z80); op_f7(z80);												} /* DB   DD          */

OP(dd,f8) { illegal_1(z80); op_f8(z80);												} /* DB   DD          */
OP(dd,f9) { z80->SP = z80->IX;														} /* LD   SP,IX       */
OP(dd,fa) { illegal_1(z80); op_fa(z80);												} /* DB   DD          */
OP(dd,fb) { illegal_1(z80); op_fb(z80);												} /* DB   DD          */
OP(dd,fc) { illegal_1(z80); op_fc(z80);												} /* DB   DD          */
OP(dd,fd) { illegal_1(z80); op_fd(z80);												} /* DB   DD          */
OP(dd,fe) { illegal_1(z80); op_fe(z80);												} /* DB   DD          */
OP(dd,ff) { illegal_1(z80); op_ff(z80);												} /* DB   DD          */

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP(fd,00) { illegal_1(z80); op_00(z80);												} /* DB   FD          */
OP(fd,01) { illegal_1(z80); op_01(z80);												} /* DB   FD          */
OP(fd,02) { illegal_1(z80); op_02(z80);												} /* DB   FD          */
OP(fd,03) { illegal_1(z80); op_03(z80);												} /* DB   FD          */
OP(fd,04) { illegal_1(z80); op_04(z80);												} /* DB   FD          */
OP(fd,05) { illegal_1(z80); op_05(z80);												} /* DB   FD          */
OP(fd,06) { illegal_1(z80); op_06(z80);												} /* DB   FD          */
OP(fd,07) { illegal_1(z80); op_07(z80);												} /* DB   FD          */

OP(fd,08) { illegal_1(z80); op_08(z80);												} /* DB   FD          */
OP(fd,09) { ADD16(z80, iy, bc);														} /* ADD  IY,BC       */
OP(fd,0a) { illegal_1(z80); op_0a(z80);												} /* DB   FD          */
OP(fd,0b) { illegal_1(z80); op_0b(z80);												} /* DB   FD          */
OP(fd,0c) { illegal_1(z80); op_0c(z80);												} /* DB   FD          */
OP(fd,0d) { illegal_1(z80); op_0d(z80);												} /* DB   FD          */
OP(fd,0e) { illegal_1(z80); op_0e(z80);												} /* DB   FD          */
OP(fd,0f) { illegal_1(z80); op_0f(z80);												} /* DB   FD          */

OP(fd,10) { illegal_1(z80); op_10(z80);												} /* DB   FD          */
OP(fd,11) { illegal_1(z80); op_11(z80);												} /* DB   FD          */
OP(fd,12) { illegal_1(z80); op_12(z80);												} /* DB   FD          */
OP(fd,13) { illegal_1(z80); op_13(z80);												} /* DB   FD          */
OP(fd,14) { illegal_1(z80); op_14(z80);												} /* DB   FD          */
OP(fd,15) { illegal_1(z80); op_15(z80);												} /* DB   FD          */
OP(fd,16) { illegal_1(z80); op_16(z80);												} /* DB   FD          */
OP(fd,17) { illegal_1(z80); op_17(z80);												} /* DB   FD          */

OP(fd,18) { illegal_1(z80); op_18(z80);												} /* DB   FD          */
OP(fd,19) { ADD16(z80, iy, de);														} /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(z80); op_1a(z80);												} /* DB   FD          */
OP(fd,1b) { illegal_1(z80); op_1b(z80);												} /* DB   FD          */
OP(fd,1c) { illegal_1(z80); op_1c(z80);												} /* DB   FD          */
OP(fd,1d) { illegal_1(z80); op_1d(z80);												} /* DB   FD          */
OP(fd,1e) { illegal_1(z80); op_1e(z80);												} /* DB   FD          */
OP(fd,1f) { illegal_1(z80); op_1f(z80);												} /* DB   FD          */

OP(fd,20) { illegal_1(z80); op_20(z80);												} /* DB   FD          */
OP(fd,21) { z80->IY = ARG16(z80);													} /* LD   IY,w        */
OP(fd,22) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->iy); z80->WZ = z80->ea+1;} /* LD   (w),IY      */
OP(fd,23) { z80->IY++;																} /* INC  IY          */
OP(fd,24) { z80->HY = INC(z80, z80->HY);											} /* INC  HY          */
OP(fd,25) { z80->HY = DEC(z80, z80->HY);											} /* DEC  HY          */
OP(fd,26) { z80->HY = ARG(z80);														} /* LD   HY,n        */
OP(fd,27) { illegal_1(z80); op_27(z80);												} /* DB   FD          */

OP(fd,28) { illegal_1(z80); op_28(z80);												} /* DB   FD          */
OP(fd,29) { ADD16(z80, iy, iy);														} /* ADD  IY,IY       */
OP(fd,2a) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->iy); z80->WZ = z80->ea+1;} /* LD   IY,(w)      */
OP(fd,2b) { z80->IY--;																} /* DEC  IY          */
OP(fd,2c) { z80->LY = INC(z80, z80->LY);											} /* INC  LY          */
OP(fd,2d) { z80->LY = DEC(z80, z80->LY);											} /* DEC  LY          */
OP(fd,2e) { z80->LY = ARG(z80);														} /* LD   LY,n        */
OP(fd,2f) { illegal_1(z80); op_2f(z80);												} /* DB   FD          */

OP(fd,30) { illegal_1(z80); op_30(z80);												} /* DB   FD          */
OP(fd,31) { illegal_1(z80); op_31(z80);												} /* DB   FD          */
OP(fd,32) { illegal_1(z80); op_32(z80);												} /* DB   FD          */
OP(fd,33) { illegal_1(z80); op_33(z80);												} /* DB   FD          */
OP(fd,34) { EAY(z80); WM(z80, z80->ea, INC(z80, RM(z80, z80->ea)));					} /* INC  (IY+o)      */
OP(fd,35) { EAY(z80); WM(z80, z80->ea, DEC(z80, RM(z80, z80->ea)));					} /* DEC  (IY+o)      */
OP(fd,36) { EAY(z80); WM(z80, z80->ea, ARG(z80));									} /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(z80); op_37(z80);												} /* DB   FD          */

OP(fd,38) { illegal_1(z80); op_38(z80);												} /* DB   FD          */
OP(fd,39) { ADD16(z80, iy, sp);														} /* ADD  IY,SP       */
OP(fd,3a) { illegal_1(z80); op_3a(z80);												} /* DB   FD          */
OP(fd,3b) { illegal_1(z80); op_3b(z80);												} /* DB   FD          */
OP(fd,3c) { illegal_1(z80); op_3c(z80);												} /* DB   FD          */
OP(fd,3d) { illegal_1(z80); op_3d(z80);												} /* DB   FD          */
OP(fd,3e) { illegal_1(z80); op_3e(z80);												} /* DB   FD          */
OP(fd,3f) { illegal_1(z80); op_3f(z80);												} /* DB   FD          */

OP(fd,40) { illegal_1(z80); op_40(z80);												} /* DB   FD          */
OP(fd,41) { illegal_1(z80); op_41(z80);												} /* DB   FD          */
OP(fd,42) { illegal_1(z80); op_42(z80);												} /* DB   FD          */
OP(fd,43) { illegal_1(z80); op_43(z80);												} /* DB   FD          */
OP(fd,44) { z80->B = z80->HY;														} /* LD   B,HY        */
OP(fd,45) { z80->B = z80->LY;														} /* LD   B,LY        */
OP(fd,46) { EAY(z80); z80->B = RM(z80, z80->ea);									} /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(z80); op_47(z80);												} /* DB   FD          */

OP(fd,48) { illegal_1(z80); op_48(z80);												} /* DB   FD          */
OP(fd,49) { illegal_1(z80); op_49(z80);												} /* DB   FD          */
OP(fd,4a) { illegal_1(z80); op_4a(z80);												} /* DB   FD          */
OP(fd,4b) { illegal_1(z80); op_4b(z80);												} /* DB   FD          */
OP(fd,4c) { z80->C = z80->HY;														} /* LD   C,HY        */
OP(fd,4d) { z80->C = z80->LY;														} /* LD   C,LY        */
OP(fd,4e) { EAY(z80); z80->C = RM(z80, z80->ea);									} /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(z80); op_4f(z80);												} /* DB   FD          */

OP(fd,50) { illegal_1(z80); op_50(z80);												} /* DB   FD          */
OP(fd,51) { illegal_1(z80); op_51(z80);												} /* DB   FD          */
OP(fd,52) { illegal_1(z80); op_52(z80);												} /* DB   FD          */
OP(fd,53) { illegal_1(z80); op_53(z80);												} /* DB   FD          */
OP(fd,54) { z80->D = z80->HY;														} /* LD   D,HY        */
OP(fd,55) { z80->D = z80->LY;														} /* LD   D,LY        */
OP(fd,56) { EAY(z80); z80->D = RM(z80, z80->ea);									} /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(z80); op_57(z80);												} /* DB   FD          */

OP(fd,58) { illegal_1(z80); op_58(z80);												} /* DB   FD          */
OP(fd,59) { illegal_1(z80); op_59(z80);												} /* DB   FD          */
OP(fd,5a) { illegal_1(z80); op_5a(z80);												} /* DB   FD          */
OP(fd,5b) { illegal_1(z80); op_5b(z80);												} /* DB   FD          */
OP(fd,5c) { z80->E = z80->HY;														} /* LD   E,HY        */
OP(fd,5d) { z80->E = z80->LY;														} /* LD   E,LY        */
OP(fd,5e) { EAY(z80); z80->E = RM(z80, z80->ea);									} /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(z80); op_5f(z80);												} /* DB   FD          */

OP(fd,60) { z80->HY = z80->B;														} /* LD   HY,B        */
OP(fd,61) { z80->HY = z80->C;														} /* LD   HY,C        */
OP(fd,62) { z80->HY = z80->D;														} /* LD   HY,D        */
OP(fd,63) { z80->HY = z80->E;														} /* LD   HY,E        */
OP(fd,64) {																			} /* LD   HY,HY       */
OP(fd,65) { z80->HY = z80->LY;														} /* LD   HY,LY       */
OP(fd,66) { EAY(z80); z80->H = RM(z80, z80->ea);									} /* LD   H,(IY+o)    */
OP(fd,67) { z80->HY = z80->A;														} /* LD   HY,A        */

OP(fd,68) { z80->LY = z80->B;														} /* LD   LY,B        */
OP(fd,69) { z80->LY = z80->C;														} /* LD   LY,C        */
OP(fd,6a) { z80->LY = z80->D;														} /* LD   LY,D        */
OP(fd,6b) { z80->LY = z80->E;														} /* LD   LY,E        */
OP(fd,6c) { z80->LY = z80->HY;														} /* LD   LY,HY       */
OP(fd,6d) {																			} /* LD   LY,LY       */
OP(fd,6e) { EAY(z80); z80->L = RM(z80, z80->ea);									} /* LD   L,(IY+o)    */
OP(fd,6f) { z80->LY = z80->A;														} /* LD   LY,A        */

OP(fd,70) { EAY(z80); WM(z80, z80->ea, z80->B);										} /* LD   (IY+o),B    */
OP(fd,71) { EAY(z80); WM(z80, z80->ea, z80->C);										} /* LD   (IY+o),C    */
OP(fd,72) { EAY(z80); WM(z80, z80->ea, z80->D);										} /* LD   (IY+o),D    */
OP(fd,73) { EAY(z80); WM(z80, z80->ea, z80->E);										} /* LD   (IY+o),E    */
OP(fd,74) { EAY(z80); WM(z80, z80->ea, z80->H);										} /* LD   (IY+o),H    */
OP(fd,75) { EAY(z80); WM(z80, z80->ea, z80->L);										} /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(z80); op_76(z80);												} /* DB   FD          */
OP(fd,77) { EAY(z80); WM(z80, z80->ea, z80->A);										} /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(z80); op_78(z80);												} /* DB   FD          */
OP(fd,79) { illegal_1(z80); op_79(z80);												} /* DB   FD          */
OP(fd,7a) { illegal_1(z80); op_7a(z80);												} /* DB   FD          */
OP(fd,7b) { illegal_1(z80); op_7b(z80);												} /* DB   FD          */
OP(fd,7c) { z80->A = z80->HY;														} /* LD   A,HY        */
OP(fd,7d) { z80->A = z80->LY;														} /* LD   A,LY        */
OP(fd,7e) { EAY(z80); z80->A = RM(z80, z80->ea);									} /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(z80); op_7f(z80);												} /* DB   FD          */

OP(fd,80) { illegal_1(z80); op_80(z80);												} /* DB   FD          */
OP(fd,81) { illegal_1(z80); op_81(z80);												} /* DB   FD          */
OP(fd,82) { illegal_1(z80); op_82(z80);												} /* DB   FD          */
OP(fd,83) { illegal_1(z80); op_83(z80);												} /* DB   FD          */
OP(fd,84) { ADD(z80, z80->HY);														} /* ADD  A,HY        */
OP(fd,85) { ADD(z80, z80->LY);														} /* ADD  A,LY        */
OP(fd,86) { EAY(z80); ADD(z80, RM(z80, z80->ea));									} /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(z80); op_87(z80);												} /* DB   FD          */

OP(fd,88) { illegal_1(z80); op_88(z80);												} /* DB   FD          */
OP(fd,89) { illegal_1(z80); op_89(z80);												} /* DB   FD          */
OP(fd,8a) { illegal_1(z80); op_8a(z80);												} /* DB   FD          */
OP(fd,8b) { illegal_1(z80); op_8b(z80);												} /* DB   FD          */
OP(fd,8c) { ADC(z80, z80->HY);														} /* ADC  A,HY        */
OP(fd,8d) { ADC(z80, z80->LY);														} /* ADC  A,LY        */
OP(fd,8e) { EAY(z80); ADC(z80, RM(z80, z80->ea));									} /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(z80); op_8f(z80);												} /* DB   FD          */

OP(fd,90) { illegal_1(z80); op_90(z80);												} /* DB   FD          */
OP(fd,91) { illegal_1(z80); op_91(z80);												} /* DB   FD          */
OP(fd,92) { illegal_1(z80); op_92(z80);												} /* DB   FD          */
OP(fd,93) { illegal_1(z80); op_93(z80);												} /* DB   FD          */
OP(fd,94) { SUB(z80, z80->HY);														} /* SUB  HY          */
OP(fd,95) { SUB(z80, z80->LY);														} /* SUB  LY          */
OP(fd,96) { EAY(z80); SUB(z80, RM(z80, z80->ea));									} /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(z80); op_97(z80);												} /* DB   FD          */

OP(fd,98) { illegal_1(z80); op_98(z80);												} /* DB   FD          */
OP(fd,99) { illegal_1(z80); op_99(z80);												} /* DB   FD          */
OP(fd,9a) { illegal_1(z80); op_9a(z80);												} /* DB   FD          */
OP(fd,9b) { illegal_1(z80); op_9b(z80);												} /* DB   FD          */
OP(fd,9c) { SBC(z80, z80->HY);														} /* SBC  A,HY        */
OP(fd,9d) { SBC(z80, z80->LY);														} /* SBC  A,LY        */
OP(fd,9e) { EAY(z80); SBC(z80, RM(z80, z80->ea));									} /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(z80); op_9f(z80);												} /* DB   FD          */

OP(fd,a0) { illegal_1(z80); op_a0(z80);												} /* DB   FD          */
OP(fd,a1) { illegal_1(z80); op_a1(z80);												} /* DB   FD          */
OP(fd,a2) { illegal_1(z80); op_a2(z80);												} /* DB   FD          */
OP(fd,a3) { illegal_1(z80); op_a3(z80);												} /* DB   FD          */
OP(fd,a4) { AND(z80, z80->HY);														} /* AND  HY          */
OP(fd,a5) { AND(z80, z80->LY);														} /* AND  LY          */
OP(fd,a6) { EAY(z80); AND(z80, RM(z80, z80->ea));									} /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(z80); op_a7(z80);												} /* DB   FD          */

OP(fd,a8) { illegal_1(z80); op_a8(z80);												} /* DB   FD          */
OP(fd,a9) { illegal_1(z80); op_a9(z80);												} /* DB   FD          */
OP(fd,aa) { illegal_1(z80); op_aa(z80);												} /* DB   FD          */
OP(fd,ab) { illegal_1(z80); op_ab(z80);												} /* DB   FD          */
OP(fd,ac) { XOR(z80, z80->HY);														} /* XOR  HY          */
OP(fd,ad) { XOR(z80, z80->LY);														} /* XOR  LY          */
OP(fd,ae) { EAY(z80); XOR(z80, RM(z80, z80->ea));									} /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(z80); op_af(z80);												} /* DB   FD          */

OP(fd,b0) { illegal_1(z80); op_b0(z80);												} /* DB   FD          */
OP(fd,b1) { illegal_1(z80); op_b1(z80);												} /* DB   FD          */
OP(fd,b2) { illegal_1(z80); op_b2(z80);												} /* DB   FD          */
OP(fd,b3) { illegal_1(z80); op_b3(z80);												} /* DB   FD          */
OP(fd,b4) { OR(z80, z80->HY);														} /* OR   HY          */
OP(fd,b5) { OR(z80, z80->LY);														} /* OR   LY          */
OP(fd,b6) { EAY(z80); OR(z80, RM(z80, z80->ea));									} /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(z80); op_b7(z80);												} /* DB   FD          */

OP(fd,b8) { illegal_1(z80); op_b8(z80);												} /* DB   FD          */
OP(fd,b9) { illegal_1(z80); op_b9(z80);												} /* DB   FD          */
OP(fd,ba) { illegal_1(z80); op_ba(z80);												} /* DB   FD          */
OP(fd,bb) { illegal_1(z80); op_bb(z80);												} /* DB   FD          */
OP(fd,bc) { CP(z80, z80->HY);														} /* CP   HY          */
OP(fd,bd) { CP(z80, z80->LY);														} /* CP   LY          */
OP(fd,be) { EAY(z80); CP(z80, RM(z80, z80->ea));									} /* CP   (IY+o)      */
OP(fd,bf) { illegal_1(z80); op_bf(z80);												} /* DB   FD          */

OP(fd,c0) { illegal_1(z80); op_c0(z80);												} /* DB   FD          */
OP(fd,c1) { illegal_1(z80); op_c1(z80);												} /* DB   FD          */
OP(fd,c2) { illegal_1(z80); op_c2(z80);												} /* DB   FD          */
OP(fd,c3) { illegal_1(z80); op_c3(z80);												} /* DB   FD          */
OP(fd,c4) { illegal_1(z80); op_c4(z80);												} /* DB   FD          */
OP(fd,c5) { illegal_1(z80); op_c5(z80);												} /* DB   FD          */
OP(fd,c6) { illegal_1(z80); op_c6(z80);												} /* DB   FD          */
OP(fd,c7) { illegal_1(z80); op_c7(z80);												} /* DB   FD          */

OP(fd,c8) { illegal_1(z80); op_c8(z80);												} /* DB   FD          */
OP(fd,c9) { illegal_1(z80); op_c9(z80);												} /* DB   FD          */
OP(fd,ca) { illegal_1(z80); op_ca(z80);												} /* DB   FD          */
OP(fd,cb) { EAY(z80); EXEC(z80,xycb,ARG(z80));										} /* **   FD CB xx    */
OP(fd,cc) { illegal_1(z80); op_cc(z80);												} /* DB   FD          */
OP(fd,cd) { illegal_1(z80); op_cd(z80);												} /* DB   FD          */
OP(fd,ce) { illegal_1(z80); op_ce(z80);												} /* DB   FD          */
OP(fd,cf) { illegal_1(z80); op_cf(z80);												} /* DB   FD          */

OP(fd,d0) { illegal_1(z80); op_d0(z80);												} /* DB   FD          */
OP(fd,d1) { illegal_1(z80); op_d1(z80);												} /* DB   FD          */
OP(fd,d2) { illegal_1(z80); op_d2(z80);												} /* DB   FD          */
OP(fd,d3) { illegal_1(z80); op_d3(z80);												} /* DB   FD          */
OP(fd,d4) { illegal_1(z80); op_d4(z80);												} /* DB   FD          */
OP(fd,d5) { illegal_1(z80); op_d5(z80);												} /* DB   FD          */
OP(fd,d6) { illegal_1(z80); op_d6(z80);												} /* DB   FD          */
OP(fd,d7) { illegal_1(z80); op_d7(z80);												} /* DB   FD          */

OP(fd,d8) { illegal_1(z80); op_d8(z80);												} /* DB   FD          */
OP(fd,d9) { illegal_1(z80); op_d9(z80);												} /* DB   FD          */
OP(fd,da) { illegal_1(z80); op_da(z80);												} /* DB   FD          */
OP(fd,db) { illegal_1(z80); op_db(z80);												} /* DB   FD          */
OP(fd,dc) { illegal_1(z80); op_dc(z80);												} /* DB   FD          */
OP(fd,dd) { illegal_1(z80); op_dd(z80);												} /* DB   FD          */
OP(fd,de) { illegal_1(z80); op_de(z80);												} /* DB   FD          */
OP(fd,df) { illegal_1(z80); op_df(z80);												} /* DB   FD          */

OP(fd,e0) { illegal_1(z80); op_e0(z80);												} /* DB   FD          */
OP(fd,e1) { POP(z80, iy);															} /* POP  IY          */
OP(fd,e2) { illegal_1(z80); op_e2(z80);												} /* DB   FD          */
OP(fd,e3) { EXSP(z80, iy);															} /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(z80); op_e4(z80);												} /* DB   FD          */
OP(fd,e5) { PUSH(z80, iy);															} /* PUSH IY          */
OP(fd,e6) { illegal_1(z80); op_e6(z80);												} /* DB   FD          */
OP(fd,e7) { illegal_1(z80); op_e7(z80);												} /* DB   FD          */

OP(fd,e8) { illegal_1(z80); op_e8(z80);												} /* DB   FD          */
OP(fd,e9) { z80->PC = z80->IY;														} /* JP   (IY)        */
OP(fd,ea) { illegal_1(z80); op_ea(z80);												} /* DB   FD          */
OP(fd,eb) { illegal_1(z80); op_eb(z80);												} /* DB   FD          */
OP(fd,ec) { illegal_1(z80); op_ec(z80);												} /* DB   FD          */
OP(fd,ed) { illegal_1(z80); op_ed(z80);												} /* DB   FD          */
OP(fd,ee) { illegal_1(z80); op_ee(z80);												} /* DB   FD          */
OP(fd,ef) { illegal_1(z80); op_ef(z80);												} /* DB   FD          */

OP(fd,f0) { illegal_1(z80); op_f0(z80);												} /* DB   FD          */
OP(fd,f1) { illegal_1(z80); op_f1(z80);												} /* DB   FD          */
OP(fd,f2) { illegal_1(z80); op_f2(z80);												} /* DB   FD          */
OP(fd,f3) { illegal_1(z80); op_f3(z80);												} /* DB   FD          */
OP(fd,f4) { illegal_1(z80); op_f4(z80);												} /* DB   FD          */
OP(fd,f5) { illegal_1(z80); op_f5(z80);												} /* DB   FD          */
OP(fd,f6) { illegal_1(z80); op_f6(z80);												} /* DB   FD          */
OP(fd,f7) { illegal_1(z80); op_f7(z80);												} /* DB   FD          */

OP(fd,f8) { illegal_1(z80); op_f8(z80);												} /* DB   FD          */
OP(fd,f9) { z80->SP = z80->IY;														} /* LD   SP,IY       */
OP(fd,fa) { illegal_1(z80); op_fa(z80);												} /* DB   FD          */
OP(fd,fb) { illegal_1(z80); op_fb(z80);												} /* DB   FD          */
OP(fd,fc) { illegal_1(z80); op_fc(z80);												} /* DB   FD          */
OP(fd,fd) { illegal_1(z80); op_fd(z80);												} /* DB   FD          */
OP(fd,fe) { illegal_1(z80); op_fe(z80);												} /* DB   FD          */
OP(fd,ff) { illegal_1(z80); op_ff(z80);												} /* DB   FD          */

OP(illegal,2)
{
	logerror("Z80 '%s' ill. opcode $ed $%02x\n",
			z80->device->tag(), z80->direct->read_decrypted_byte((z80->PCD-1)&0xffff));
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP(ed,00) { illegal_2(z80);															} /* DB   ED          */
OP(ed,01) { illegal_2(z80);															} /* DB   ED          */
OP(ed,02) { illegal_2(z80);															} /* DB   ED          */
OP(ed,03) { illegal_2(z80);															} /* DB   ED          */
OP(ed,04) { illegal_2(z80);															} /* DB   ED          */
OP(ed,05) { illegal_2(z80);															} /* DB   ED          */
OP(ed,06) { illegal_2(z80);															} /* DB   ED          */
OP(ed,07) { illegal_2(z80);															} /* DB   ED          */

OP(ed,08) { illegal_2(z80);															} /* DB   ED          */
OP(ed,09) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,0f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,10) { illegal_2(z80);															} /* DB   ED          */
OP(ed,11) { illegal_2(z80);															} /* DB   ED          */
OP(ed,12) { illegal_2(z80);															} /* DB   ED          */
OP(ed,13) { illegal_2(z80);															} /* DB   ED          */
OP(ed,14) { illegal_2(z80);															} /* DB   ED          */
OP(ed,15) { illegal_2(z80);															} /* DB   ED          */
OP(ed,16) { illegal_2(z80);															} /* DB   ED          */
OP(ed,17) { illegal_2(z80);															} /* DB   ED          */

OP(ed,18) { illegal_2(z80);															} /* DB   ED          */
OP(ed,19) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,1f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,20) { illegal_2(z80);															} /* DB   ED          */
OP(ed,21) { illegal_2(z80);															} /* DB   ED          */
OP(ed,22) { illegal_2(z80);															} /* DB   ED          */
OP(ed,23) { illegal_2(z80);															} /* DB   ED          */
OP(ed,24) { illegal_2(z80);															} /* DB   ED          */
OP(ed,25) { illegal_2(z80);															} /* DB   ED          */
OP(ed,26) { illegal_2(z80);															} /* DB   ED          */
OP(ed,27) { illegal_2(z80);															} /* DB   ED          */

OP(ed,28) { illegal_2(z80);															} /* DB   ED          */
OP(ed,29) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,2f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,30) { illegal_2(z80);															} /* DB   ED          */
OP(ed,31) { illegal_2(z80);															} /* DB   ED          */
OP(ed,32) { illegal_2(z80);															} /* DB   ED          */
OP(ed,33) { illegal_2(z80);															} /* DB   ED          */
OP(ed,34) { illegal_2(z80);															} /* DB   ED          */
OP(ed,35) { illegal_2(z80);															} /* DB   ED          */
OP(ed,36) { illegal_2(z80);															} /* DB   ED          */
OP(ed,37) { illegal_2(z80);															} /* DB   ED          */

OP(ed,38) { illegal_2(z80);															} /* DB   ED          */
OP(ed,39) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,3f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,40) { z80->B = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->B];		} /* IN   B,(C)       */
OP(ed,41) { OUT(z80, z80->BC, z80->B);												} /* OUT  (C),B       */
OP(ed,42) { SBC16(z80, bc);															} /* SBC  HL,BC       */
OP(ed,43) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->bc); z80->WZ = z80->ea+1;} /* LD   (w),BC      */
OP(ed,44) { NEG(z80);																} /* NEG              */
OP(ed,45) { RETN(z80);																} /* RETN             */
OP(ed,46) { z80->im = 0;															} /* im   0           */
OP(ed,47) { LD_I_A(z80);															} /* LD   i,A         */

OP(ed,48) { z80->C = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->C];		} /* IN   C,(C)       */
OP(ed,49) { OUT(z80, z80->BC, z80->C);												} /* OUT  (C),C       */
OP(ed,4a) { ADC16(z80, bc);															} /* ADC  HL,BC       */
OP(ed,4b) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->bc); z80->WZ = z80->ea+1;} /* LD   BC,(w)      */
OP(ed,4c) { NEG(z80);																} /* NEG              */
OP(ed,4d) { RETI(z80);																} /* RETI             */
OP(ed,4e) { z80->im = 0;															} /* im   0           */
OP(ed,4f) { LD_R_A(z80);															} /* LD   r,A         */

OP(ed,50) { z80->D = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->D];		} /* IN   D,(C)       */
OP(ed,51) { OUT(z80, z80->BC, z80->D);												} /* OUT  (C),D       */
OP(ed,52) { SBC16(z80, de);															} /* SBC  HL,DE       */
OP(ed,53) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->de); z80->WZ = z80->ea+1;} /* LD   (w),DE      */
OP(ed,54) { NEG(z80);																} /* NEG              */
OP(ed,55) { RETN(z80);																} /* RETN             */
OP(ed,56) { z80->im = 1;															} /* im   1           */
OP(ed,57) { LD_A_I(z80);															} /* LD   A,i         */

OP(ed,58) { z80->E = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->E];		} /* IN   E,(C)       */
OP(ed,59) { OUT(z80, z80->BC, z80->E);												} /* OUT  (C),E       */
OP(ed,5a) { ADC16(z80, de);															} /* ADC  HL,DE       */
OP(ed,5b) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->de); z80->WZ = z80->ea+1;} /* LD   DE,(w)      */
OP(ed,5c) { NEG(z80);																} /* NEG              */
OP(ed,5d) { RETI(z80);																} /* RETI             */
OP(ed,5e) { z80->im = 2;															} /* im   2           */
OP(ed,5f) { LD_A_R(z80);															} /* LD   A,r         */

OP(ed,60) { z80->H = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->H];		} /* IN   H,(C)       */
OP(ed,61) { OUT(z80, z80->BC, z80->H);												} /* OUT  (C),H       */
OP(ed,62) { SBC16(z80, hl);															} /* SBC  HL,HL       */
OP(ed,63) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->hl); z80->WZ = z80->ea+1;} /* LD   (w),HL      */
OP(ed,64) { NEG(z80);																} /* NEG              */
OP(ed,65) { RETN(z80);																} /* RETN             */
OP(ed,66) { z80->im = 0;															} /* im   0           */
OP(ed,67) { RRD(z80);																} /* RRD  (HL)        */

OP(ed,68) { z80->L = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->L];		} /* IN   L,(C)       */
OP(ed,69) { OUT(z80, z80->BC, z80->L);												} /* OUT  (C),L       */
OP(ed,6a) { ADC16(z80, hl);															} /* ADC  HL,HL       */
OP(ed,6b) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->hl); z80->WZ = z80->ea+1;} /* LD   HL,(w)      */
OP(ed,6c) { NEG(z80);																} /* NEG              */
OP(ed,6d) { RETI(z80);																} /* RETI             */
OP(ed,6e) { z80->im = 0;															} /* im   0           */
OP(ed,6f) { RLD(z80);																} /* RLD  (HL)        */

OP(ed,70) { UINT8 res = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[res];		} /* IN   0,(C)       */
OP(ed,71) { OUT(z80, z80->BC, 0);													} /* OUT  (C),0       */
OP(ed,72) { SBC16(z80, sp);															} /* SBC  HL,SP       */
OP(ed,73) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->sp); z80->WZ = z80->ea+1;} /* LD   (w),SP      */
OP(ed,74) { NEG(z80);																} /* NEG              */
OP(ed,75) { RETN(z80);																} /* RETN             */
OP(ed,76) { z80->im = 1;															} /* im   1           */
OP(ed,77) { illegal_2(z80);															} /* DB   ED,77       */

OP(ed,78) { z80->A = IN(z80, z80->BC); z80->F = (z80->F & CF) | SZP[z80->A]; z80->WZ = z80->BC + 1;	} /* IN   A,(C)       */
OP(ed,79) { OUT(z80, z80->BC, z80->A);	z80->WZ = z80->BC + 1;						} /* OUT  (C),A       */
OP(ed,7a) { ADC16(z80, sp);															} /* ADC  HL,SP       */
OP(ed,7b) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->sp); z80->WZ = z80->ea+1;} /* LD   SP,(w)      */
OP(ed,7c) { NEG(z80);																} /* NEG              */
OP(ed,7d) { RETI(z80);																} /* RETI             */
OP(ed,7e) { z80->im = 2;															} /* im   2           */
OP(ed,7f) { illegal_2(z80);															} /* DB   ED,7F       */

OP(ed,80) { illegal_2(z80);															} /* DB   ED          */
OP(ed,81) { illegal_2(z80);															} /* DB   ED          */
OP(ed,82) { illegal_2(z80);															} /* DB   ED          */
OP(ed,83) { illegal_2(z80);															} /* DB   ED          */
OP(ed,84) { illegal_2(z80);															} /* DB   ED          */
OP(ed,85) { illegal_2(z80);															} /* DB   ED          */
OP(ed,86) { illegal_2(z80);															} /* DB   ED          */
OP(ed,87) { illegal_2(z80);															} /* DB   ED          */

OP(ed,88) { illegal_2(z80);															} /* DB   ED          */
OP(ed,89) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,8f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,90) { illegal_2(z80);															} /* DB   ED          */
OP(ed,91) { illegal_2(z80);															} /* DB   ED          */
OP(ed,92) { illegal_2(z80);															} /* DB   ED          */
OP(ed,93) { illegal_2(z80);															} /* DB   ED          */
OP(ed,94) { illegal_2(z80);															} /* DB   ED          */
OP(ed,95) { illegal_2(z80);															} /* DB   ED          */
OP(ed,96) { illegal_2(z80);															} /* DB   ED          */
OP(ed,97) { illegal_2(z80);															} /* DB   ED          */

OP(ed,98) { illegal_2(z80);															} /* DB   ED          */
OP(ed,99) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9a) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9b) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9c) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9d) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9e) { illegal_2(z80);															} /* DB   ED          */
OP(ed,9f) { illegal_2(z80);															} /* DB   ED          */

OP(ed,a0) { LDI(z80);																} /* LDI              */
OP(ed,a1) { CPI(z80);																} /* CPI              */
OP(ed,a2) { INI(z80);																} /* INI              */
OP(ed,a3) { OUTI(z80);																} /* OUTI             */
OP(ed,a4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,a5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,a6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,a7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,a8) { LDD(z80);																} /* LDD              */
OP(ed,a9) { CPD(z80);																} /* CPD              */
OP(ed,aa) { IND(z80);																} /* IND              */
OP(ed,ab) { OUTD(z80);																} /* OUTD             */
OP(ed,ac) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ad) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ae) { illegal_2(z80);															} /* DB   ED          */
OP(ed,af) { illegal_2(z80);															} /* DB   ED          */

OP(ed,b0) { LDIR(z80);																} /* LDIR             */
OP(ed,b1) { CPIR(z80);																} /* CPIR             */
OP(ed,b2) { INIR(z80);																} /* INIR             */
OP(ed,b3) { OTIR(z80);																} /* OTIR             */
OP(ed,b4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,b5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,b6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,b7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,b8) { LDDR(z80);																} /* LDDR             */
OP(ed,b9) { CPDR(z80);																} /* CPDR             */
OP(ed,ba) { INDR(z80);																} /* INDR             */
OP(ed,bb) { OTDR(z80);																} /* OTDR             */
OP(ed,bc) { illegal_2(z80);															} /* DB   ED          */
OP(ed,bd) { illegal_2(z80);															} /* DB   ED          */
OP(ed,be) { illegal_2(z80);															} /* DB   ED          */
OP(ed,bf) { illegal_2(z80);															} /* DB   ED          */

OP(ed,c0) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c1) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c2) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c3) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,c8) { illegal_2(z80);															} /* DB   ED          */
OP(ed,c9) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ca) { illegal_2(z80);															} /* DB   ED          */
OP(ed,cb) { illegal_2(z80);															} /* DB   ED          */
OP(ed,cc) { illegal_2(z80);															} /* DB   ED          */
OP(ed,cd) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ce) { illegal_2(z80);															} /* DB   ED          */
OP(ed,cf) { illegal_2(z80);															} /* DB   ED          */

OP(ed,d0) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d1) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d2) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d3) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,d8) { illegal_2(z80);															} /* DB   ED          */
OP(ed,d9) { illegal_2(z80);															} /* DB   ED          */
OP(ed,da) { illegal_2(z80);															} /* DB   ED          */
OP(ed,db) { illegal_2(z80);															} /* DB   ED          */
OP(ed,dc) { illegal_2(z80);															} /* DB   ED          */
OP(ed,dd) { illegal_2(z80);															} /* DB   ED          */
OP(ed,de) { illegal_2(z80);															} /* DB   ED          */
OP(ed,df) { illegal_2(z80);															} /* DB   ED          */

OP(ed,e0) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e1) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e2) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e3) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,e8) { illegal_2(z80);															} /* DB   ED          */
OP(ed,e9) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ea) { illegal_2(z80);															} /* DB   ED          */
OP(ed,eb) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ec) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ed) { illegal_2(z80); 														} /* DB   ED          */
OP(ed,ee) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ef) { illegal_2(z80);															} /* DB   ED          */

OP(ed,f0) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f1) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f2) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f3) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f4) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f5) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f6) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f7) { illegal_2(z80);															} /* DB   ED          */

OP(ed,f8) { illegal_2(z80);															} /* DB   ED          */
OP(ed,f9) { illegal_2(z80);															} /* DB   ED          */
OP(ed,fa) { illegal_2(z80);															} /* DB   ED          */
OP(ed,fb) { illegal_2(z80);															} /* DB   ED          */
OP(ed,fc) { illegal_2(z80);															} /* DB   ED          */
OP(ed,fd) { illegal_2(z80);															} /* DB   ED          */
OP(ed,fe) { illegal_2(z80);															} /* DB   ED          */
OP(ed,ff) { illegal_2(z80);															} /* DB   ED          */


/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {																			} /* NOP              */
OP(op,01) { z80->BC = ARG16(z80);													} /* LD   BC,w        */
OP(op,02) { WM(z80,z80->BC,z80->A); z80->WZ_L = (z80->BC + 1) & 0xFF;  z80->WZ_H = z80->A; } /* LD (BC),A */
OP(op,03) { z80->BC++;																} /* INC  BC          */
OP(op,04) { z80->B = INC(z80, z80->B);												} /* INC  B           */
OP(op,05) { z80->B = DEC(z80, z80->B);												} /* DEC  B           */
OP(op,06) { z80->B = ARG(z80);														} /* LD   B,n         */
OP(op,07) { RLCA(z80);																} /* RLCA             */

OP(op,08) { EX_AF(z80);																} /* EX   AF,AF'      */
OP(op,09) { ADD16(z80, hl, bc);														} /* ADD  HL,BC       */
OP(op,0a) { z80->A = RM(z80, z80->BC);	z80->WZ=z80->BC+1;  						} /* LD   A,(BC)      */
OP(op,0b) { z80->BC--;																} /* DEC  BC          */
OP(op,0c) { z80->C = INC(z80, z80->C);												} /* INC  C           */
OP(op,0d) { z80->C = DEC(z80, z80->C);												} /* DEC  C           */
OP(op,0e) { z80->C = ARG(z80);														} /* LD   C,n         */
OP(op,0f) { RRCA(z80);																} /* RRCA             */

OP(op,10) { z80->B--; JR_COND(z80, z80->B, 0x10);									} /* DJNZ o           */
OP(op,11) { z80->DE = ARG16(z80);													} /* LD   DE,w        */
OP(op,12) { WM(z80,z80->DE,z80->A); z80->WZ_L = (z80->DE + 1) & 0xFF;  z80->WZ_H = z80->A; } /* LD (DE),A */
OP(op,13) { z80->DE++;																} /* INC  DE          */
OP(op,14) { z80->D = INC(z80, z80->D);												} /* INC  D           */
OP(op,15) { z80->D = DEC(z80, z80->D);												} /* DEC  D           */
OP(op,16) { z80->D = ARG(z80);														} /* LD   D,n         */
OP(op,17) { RLA(z80);																} /* RLA              */

OP(op,18) { JR(z80);																} /* JR   o           */
OP(op,19) { ADD16(z80, hl, de);														} /* ADD  HL,DE       */
OP(op,1a) { z80->A = RM(z80, z80->DE); z80->WZ=z80->DE+1;							} /* LD   A,(DE)      */
OP(op,1b) { z80->DE--;																} /* DEC  DE          */
OP(op,1c) { z80->E = INC(z80, z80->E);												} /* INC  E           */
OP(op,1d) { z80->E = DEC(z80, z80->E);												} /* DEC  E           */
OP(op,1e) { z80->E = ARG(z80);														} /* LD   E,n         */
OP(op,1f) { RRA(z80);																} /* RRA              */

OP(op,20) { JR_COND(z80, !(z80->F & ZF), 0x20);										} /* JR   NZ,o        */
OP(op,21) { z80->HL = ARG16(z80);													} /* LD   HL,w        */
OP(op,22) { z80->ea = ARG16(z80); WM16(z80, z80->ea, &z80->hl);	z80->WZ = z80->ea+1;} /* LD   (w),HL      */
OP(op,23) { z80->HL++;																} /* INC  HL          */
OP(op,24) { z80->H = INC(z80, z80->H);												} /* INC  H           */
OP(op,25) { z80->H = DEC(z80, z80->H);												} /* DEC  H           */
OP(op,26) { z80->H = ARG(z80);														} /* LD   H,n         */
OP(op,27) { DAA(z80);																} /* DAA              */

OP(op,28) { JR_COND(z80, z80->F & ZF, 0x28);										} /* JR   Z,o         */
OP(op,29) { ADD16(z80, hl, hl);														} /* ADD  HL,HL       */
OP(op,2a) { z80->ea = ARG16(z80); RM16(z80, z80->ea, &z80->hl);	z80->WZ = z80->ea+1;} /* LD   HL,(w)      */
OP(op,2b) { z80->HL--;																} /* DEC  HL          */
OP(op,2c) { z80->L = INC(z80, z80->L);												} /* INC  L           */
OP(op,2d) { z80->L = DEC(z80, z80->L);												} /* DEC  L           */
OP(op,2e) { z80->L = ARG(z80);														} /* LD   L,n         */
OP(op,2f) { z80->A ^= 0xff; z80->F = (z80->F&(SF|ZF|PF|CF))|HF|NF|(z80->A&(YF|XF));	} /* CPL              */

OP(op,30) { JR_COND(z80, !(z80->F & CF), 0x30);										} /* JR   NC,o        */
OP(op,31) { z80->SP = ARG16(z80);													} /* LD   SP,w        */
OP(op,32) { z80->ea=ARG16(z80);WM(z80,z80->ea,z80->A);z80->WZ_L=(z80->ea+1)&0xFF;z80->WZ_H=z80->A; } /* LD   (w),A       */
OP(op,33) { z80->SP++;																} /* INC  SP          */
OP(op,34) { WM(z80, z80->HL, INC(z80, RM(z80, z80->HL)));							} /* INC  (HL)        */
OP(op,35) { WM(z80, z80->HL, DEC(z80, RM(z80, z80->HL)));							} /* DEC  (HL)        */
OP(op,36) { WM(z80, z80->HL, ARG(z80));												} /* LD   (HL),n      */
OP(op,37) { z80->F = (z80->F & (SF|ZF|YF|XF|PF)) | CF | (z80->A & (YF|XF));			} /* SCF              */

OP(op,38) { JR_COND(z80, z80->F & CF, 0x38);										} /* JR   C,o         */
OP(op,39) { ADD16(z80, hl, sp);														} /* ADD  HL,SP       */
OP(op,3a) { z80->ea = ARG16(z80); z80->A = RM(z80, z80->ea); z80->WZ=z80->ea+1;		} /* LD   A,(w)       */
OP(op,3b) { z80->SP--;																} /* DEC  SP          */
OP(op,3c) { z80->A = INC(z80, z80->A);												} /* INC  A           */
OP(op,3d) { z80->A = DEC(z80, z80->A);												} /* DEC  A           */
OP(op,3e) { z80->A = ARG(z80);														} /* LD   A,n         */
OP(op,3f) { z80->F = ((z80->F&(SF|ZF|YF|XF|PF|CF))|((z80->F&CF)<<4)|(z80->A&(YF|XF)))^CF; } /* CCF        */

OP(op,40) {																			} /* LD   B,B         */
OP(op,41) { z80->B = z80->C;														} /* LD   B,C         */
OP(op,42) { z80->B = z80->D;														} /* LD   B,D         */
OP(op,43) { z80->B = z80->E;														} /* LD   B,E         */
OP(op,44) { z80->B = z80->H;														} /* LD   B,H         */
OP(op,45) { z80->B = z80->L;														} /* LD   B,L         */
OP(op,46) { z80->B = RM(z80, z80->HL);												} /* LD   B,(HL)      */
OP(op,47) { z80->B = z80->A;														} /* LD   B,A         */

OP(op,48) { z80->C = z80->B;														} /* LD   C,B         */
OP(op,49) {																			} /* LD   C,C         */
OP(op,4a) { z80->C = z80->D;														} /* LD   C,D         */
OP(op,4b) { z80->C = z80->E;														} /* LD   C,E         */
OP(op,4c) { z80->C = z80->H;														} /* LD   C,H         */
OP(op,4d) { z80->C = z80->L;														} /* LD   C,L         */
OP(op,4e) { z80->C = RM(z80, z80->HL);												} /* LD   C,(HL)      */
OP(op,4f) { z80->C = z80->A;														} /* LD   C,A         */

OP(op,50) { z80->D = z80->B;														} /* LD   D,B         */
OP(op,51) { z80->D = z80->C;														} /* LD   D,C         */
OP(op,52) {																			} /* LD   D,D         */
OP(op,53) { z80->D = z80->E;														} /* LD   D,E         */
OP(op,54) { z80->D = z80->H;														} /* LD   D,H         */
OP(op,55) { z80->D = z80->L;														} /* LD   D,L         */
OP(op,56) { z80->D = RM(z80, z80->HL);												} /* LD   D,(HL)      */
OP(op,57) { z80->D = z80->A;														} /* LD   D,A         */

OP(op,58) { z80->E = z80->B;														} /* LD   E,B         */
OP(op,59) { z80->E = z80->C;														} /* LD   E,C         */
OP(op,5a) { z80->E = z80->D;														} /* LD   E,D         */
OP(op,5b) {																			} /* LD   E,E         */
OP(op,5c) { z80->E = z80->H;														} /* LD   E,H         */
OP(op,5d) { z80->E = z80->L;														} /* LD   E,L         */
OP(op,5e) { z80->E = RM(z80, z80->HL);												} /* LD   E,(HL)      */
OP(op,5f) { z80->E = z80->A;														} /* LD   E,A         */

OP(op,60) { z80->H = z80->B;														} /* LD   H,B         */
OP(op,61) { z80->H = z80->C;														} /* LD   H,C         */
OP(op,62) { z80->H = z80->D;														} /* LD   H,D         */
OP(op,63) { z80->H = z80->E;														} /* LD   H,E         */
OP(op,64) {																			} /* LD   H,H         */
OP(op,65) { z80->H = z80->L;														} /* LD   H,L         */
OP(op,66) { z80->H = RM(z80, z80->HL);												} /* LD   H,(HL)      */
OP(op,67) { z80->H = z80->A;														} /* LD   H,A         */

OP(op,68) { z80->L = z80->B;														} /* LD   L,B         */
OP(op,69) { z80->L = z80->C;														} /* LD   L,C         */
OP(op,6a) { z80->L = z80->D;														} /* LD   L,D         */
OP(op,6b) { z80->L = z80->E;														} /* LD   L,E         */
OP(op,6c) { z80->L = z80->H;														} /* LD   L,H         */
OP(op,6d) {																			} /* LD   L,L         */
OP(op,6e) { z80->L = RM(z80, z80->HL);												} /* LD   L,(HL)      */
OP(op,6f) { z80->L = z80->A;														} /* LD   L,A         */

OP(op,70) { WM(z80, z80->HL, z80->B);												} /* LD   (HL),B      */
OP(op,71) { WM(z80, z80->HL, z80->C);												} /* LD   (HL),C      */
OP(op,72) { WM(z80, z80->HL, z80->D);												} /* LD   (HL),D      */
OP(op,73) { WM(z80, z80->HL, z80->E);												} /* LD   (HL),E      */
OP(op,74) { WM(z80, z80->HL, z80->H);												} /* LD   (HL),H      */
OP(op,75) { WM(z80, z80->HL, z80->L);												} /* LD   (HL),L      */
OP(op,76) { ENTER_HALT(z80);														} /* halt             */
OP(op,77) { WM(z80, z80->HL, z80->A);												} /* LD   (HL),A      */

OP(op,78) { z80->A = z80->B;														} /* LD   A,B         */
OP(op,79) { z80->A = z80->C;														} /* LD   A,C         */
OP(op,7a) { z80->A = z80->D;														} /* LD   A,D         */
OP(op,7b) { z80->A = z80->E;														} /* LD   A,E         */
OP(op,7c) { z80->A = z80->H;														} /* LD   A,H         */
OP(op,7d) { z80->A = z80->L;														} /* LD   A,L         */
OP(op,7e) { z80->A = RM(z80, z80->HL);												} /* LD   A,(HL)      */
OP(op,7f) {																			} /* LD   A,A         */

OP(op,80) { ADD(z80, z80->B);														} /* ADD  A,B         */
OP(op,81) { ADD(z80, z80->C);														} /* ADD  A,C         */
OP(op,82) { ADD(z80, z80->D);														} /* ADD  A,D         */
OP(op,83) { ADD(z80, z80->E);														} /* ADD  A,E         */
OP(op,84) { ADD(z80, z80->H);														} /* ADD  A,H         */
OP(op,85) { ADD(z80, z80->L);														} /* ADD  A,L         */
OP(op,86) { ADD(z80, RM(z80, z80->HL));												} /* ADD  A,(HL)      */
OP(op,87) { ADD(z80, z80->A);														} /* ADD  A,A         */

OP(op,88) { ADC(z80, z80->B);														} /* ADC  A,B         */
OP(op,89) { ADC(z80, z80->C);														} /* ADC  A,C         */
OP(op,8a) { ADC(z80, z80->D);														} /* ADC  A,D         */
OP(op,8b) { ADC(z80, z80->E);														} /* ADC  A,E         */
OP(op,8c) { ADC(z80, z80->H);														} /* ADC  A,H         */
OP(op,8d) { ADC(z80, z80->L);														} /* ADC  A,L         */
OP(op,8e) { ADC(z80, RM(z80, z80->HL));												} /* ADC  A,(HL)      */
OP(op,8f) { ADC(z80, z80->A);														} /* ADC  A,A         */

OP(op,90) { SUB(z80, z80->B);														} /* SUB  B           */
OP(op,91) { SUB(z80, z80->C);														} /* SUB  C           */
OP(op,92) { SUB(z80, z80->D);														} /* SUB  D           */
OP(op,93) { SUB(z80, z80->E);														} /* SUB  E           */
OP(op,94) { SUB(z80, z80->H);														} /* SUB  H           */
OP(op,95) { SUB(z80, z80->L);														} /* SUB  L           */
OP(op,96) { SUB(z80, RM(z80, z80->HL));												} /* SUB  (HL)        */
OP(op,97) { SUB(z80, z80->A);														} /* SUB  A           */

OP(op,98) { SBC(z80, z80->B);														} /* SBC  A,B         */
OP(op,99) { SBC(z80, z80->C);														} /* SBC  A,C         */
OP(op,9a) { SBC(z80, z80->D);														} /* SBC  A,D         */
OP(op,9b) { SBC(z80, z80->E);														} /* SBC  A,E         */
OP(op,9c) { SBC(z80, z80->H);														} /* SBC  A,H         */
OP(op,9d) { SBC(z80, z80->L);														} /* SBC  A,L         */
OP(op,9e) { SBC(z80, RM(z80, z80->HL));												} /* SBC  A,(HL)      */
OP(op,9f) { SBC(z80, z80->A);														} /* SBC  A,A         */

OP(op,a0) { AND(z80, z80->B);														} /* AND  B           */
OP(op,a1) { AND(z80, z80->C);														} /* AND  C           */
OP(op,a2) { AND(z80, z80->D);														} /* AND  D           */
OP(op,a3) { AND(z80, z80->E);														} /* AND  E           */
OP(op,a4) { AND(z80, z80->H);														} /* AND  H           */
OP(op,a5) { AND(z80, z80->L);														} /* AND  L           */
OP(op,a6) { AND(z80, RM(z80, z80->HL));												} /* AND  (HL)        */
OP(op,a7) { AND(z80, z80->A);														} /* AND  A           */

OP(op,a8) { XOR(z80, z80->B);														} /* XOR  B           */
OP(op,a9) { XOR(z80, z80->C);														} /* XOR  C           */
OP(op,aa) { XOR(z80, z80->D);														} /* XOR  D           */
OP(op,ab) { XOR(z80, z80->E);														} /* XOR  E           */
OP(op,ac) { XOR(z80, z80->H);														} /* XOR  H           */
OP(op,ad) { XOR(z80, z80->L);														} /* XOR  L           */
OP(op,ae) { XOR(z80, RM(z80, z80->HL));												} /* XOR  (HL)        */
OP(op,af) { XOR(z80, z80->A);														} /* XOR  A           */

OP(op,b0) { OR(z80, z80->B);														} /* OR   B           */
OP(op,b1) { OR(z80, z80->C);														} /* OR   C           */
OP(op,b2) { OR(z80, z80->D);														} /* OR   D           */
OP(op,b3) { OR(z80, z80->E);														} /* OR   E           */
OP(op,b4) { OR(z80, z80->H);														} /* OR   H           */
OP(op,b5) { OR(z80, z80->L);														} /* OR   L           */
OP(op,b6) { OR(z80, RM(z80, z80->HL));												} /* OR   (HL)        */
OP(op,b7) { OR(z80, z80->A);														} /* OR   A           */

OP(op,b8) { CP(z80, z80->B);														} /* CP   B           */
OP(op,b9) { CP(z80, z80->C);														} /* CP   C           */
OP(op,ba) { CP(z80, z80->D);														} /* CP   D           */
OP(op,bb) { CP(z80, z80->E);														} /* CP   E           */
OP(op,bc) { CP(z80, z80->H);														} /* CP   H           */
OP(op,bd) { CP(z80, z80->L);														} /* CP   L           */
OP(op,be) { CP(z80, RM(z80, z80->HL));												} /* CP   (HL)        */
OP(op,bf) { CP(z80, z80->A);														} /* CP   A           */

OP(op,c0) { RET_COND(z80, !(z80->F & ZF), 0xc0);									} /* RET  NZ          */
OP(op,c1) { POP(z80, bc);															} /* POP  BC          */
OP(op,c2) { JP_COND(z80, !(z80->F & ZF));											} /* JP   NZ,a        */
OP(op,c3) { JP(z80);																} /* JP   a           */
OP(op,c4) { CALL_COND(z80, !(z80->F & ZF), 0xc4);									} /* CALL NZ,a        */
OP(op,c5) { PUSH(z80, bc);															} /* PUSH BC          */
OP(op,c6) { ADD(z80, ARG(z80));														} /* ADD  A,n         */
OP(op,c7) { RST(z80, 0x00);															} /* RST  0           */

OP(op,c8) { RET_COND(z80, z80->F & ZF, 0xc8);										} /* RET  Z           */
OP(op,c9) { POP(z80, pc); z80->WZ=z80->PCD;											} /* RET              */
OP(op,ca) { JP_COND(z80, z80->F & ZF);												} /* JP   Z,a         */
OP(op,cb) { z80->r++; EXEC(z80,cb,ROP(z80));										} /* **** CB xx       */
OP(op,cc) { CALL_COND(z80, z80->F & ZF, 0xcc);										} /* CALL Z,a         */
OP(op,cd) { CALL(z80);																} /* CALL a           */
OP(op,ce) { ADC(z80, ARG(z80));														} /* ADC  A,n         */
OP(op,cf) { RST(z80, 0x08);															} /* RST  1           */

OP(op,d0) { RET_COND(z80, !(z80->F & CF), 0xd0);									} /* RET  NC          */
OP(op,d1) { POP(z80, de);															} /* POP  DE          */
OP(op,d2) { JP_COND(z80, !(z80->F & CF));											} /* JP   NC,a        */
OP(op,d3) { unsigned n = ARG(z80) | (z80->A << 8); OUT(z80, n, z80->A);	z80->WZ_L = ((n & 0xff) + 1) & 0xff;  z80->WZ_H = z80->A;	} /* OUT  (n),A       */
OP(op,d4) { CALL_COND(z80, !(z80->F & CF), 0xd4);									} /* CALL NC,a        */
OP(op,d5) { PUSH(z80, de);															} /* PUSH DE          */
OP(op,d6) { SUB(z80, ARG(z80));														} /* SUB  n           */
OP(op,d7) { RST(z80, 0x10);															} /* RST  2           */

OP(op,d8) { RET_COND(z80, z80->F & CF, 0xd8);										} /* RET  C           */
OP(op,d9) { EXX(z80);																} /* EXX              */
OP(op,da) { JP_COND(z80, z80->F & CF);												} /* JP   C,a         */
OP(op,db) { unsigned n = ARG(z80) | (z80->A << 8); z80->A = IN(z80, n);	z80->WZ = n + 1; } /* IN   A,(n)  */
OP(op,dc) { CALL_COND(z80, z80->F & CF, 0xdc);										} /* CALL C,a         */
OP(op,dd) { z80->r++; EXEC(z80,dd,ROP(z80));										} /* **** DD xx       */
OP(op,de) { SBC(z80, ARG(z80));														} /* SBC  A,n         */
OP(op,df) { RST(z80, 0x18);															} /* RST  3           */

OP(op,e0) { RET_COND(z80, !(z80->F & PF), 0xe0);									} /* RET  PO          */
OP(op,e1) { POP(z80, hl);															} /* POP  HL          */
OP(op,e2) { JP_COND(z80, !(z80->F & PF));											} /* JP   PO,a        */
OP(op,e3) { EXSP(z80, hl);															} /* EX   HL,(SP)     */
OP(op,e4) { CALL_COND(z80, !(z80->F & PF), 0xe4);									} /* CALL PO,a        */
OP(op,e5) { PUSH(z80, hl);															} /* PUSH HL          */
OP(op,e6) { AND(z80, ARG(z80));														} /* AND  n           */
OP(op,e7) { RST(z80, 0x20);															} /* RST  4           */

OP(op,e8) { RET_COND(z80, z80->F & PF, 0xe8);										} /* RET  PE          */
OP(op,e9) { z80->PC = z80->HL;														} /* JP   (HL)        */
OP(op,ea) { JP_COND(z80, z80->F & PF);												} /* JP   PE,a        */
OP(op,eb) { EX_DE_HL(z80);															} /* EX   DE,HL       */
OP(op,ec) { CALL_COND(z80, z80->F & PF, 0xec);										} /* CALL PE,a        */
OP(op,ed) { z80->r++; EXEC(z80,ed,ROP(z80));										} /* **** ED xx       */
OP(op,ee) { XOR(z80, ARG(z80));														} /* XOR  n           */
OP(op,ef) { RST(z80, 0x28);															} /* RST  5           */

OP(op,f0) { RET_COND(z80, !(z80->F & SF), 0xf0);									} /* RET  P           */
OP(op,f1) { POP(z80, af);															} /* POP  AF          */
OP(op,f2) { JP_COND(z80, !(z80->F & SF));											} /* JP   P,a         */
OP(op,f3) { z80->iff1 = z80->iff2 = 0;												} /* DI               */
OP(op,f4) { CALL_COND(z80, !(z80->F & SF), 0xf4);									} /* CALL P,a         */
OP(op,f5) { PUSH(z80, af);															} /* PUSH AF          */
OP(op,f6) { OR(z80, ARG(z80));														} /* OR   n           */
OP(op,f7) { RST(z80, 0x30);															} /* RST  6           */

OP(op,f8) { RET_COND(z80, z80->F & SF, 0xf8);										} /* RET  M           */
OP(op,f9) { z80->SP = z80->HL;														} /* LD   SP,HL       */
OP(op,fa) { JP_COND(z80, z80->F & SF);												} /* JP   M,a         */
OP(op,fb) { EI(z80);																} /* EI               */
OP(op,fc) { CALL_COND(z80, z80->F & SF, 0xfc);										} /* CALL M,a         */
OP(op,fd) { z80->r++; EXEC(z80,fd,ROP(z80));										} /* **** FD xx       */
OP(op,fe) { CP(z80, ARG(z80));														} /* CP   n           */
OP(op,ff) { RST(z80, 0x38);															} /* RST  7           */


static void take_interrupt(z80_state *z80)
{
	int irq_vector;

	/* there isn't a valid previous program counter */
	z80->PRVPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT(z80);

	/* Clear both interrupt flip flops */
	z80->iff1 = z80->iff2 = 0;

	/* Daisy chain mode? If so, call the requesting device */
	if (z80->daisy.present())
		irq_vector = z80->daisy.call_ack_device();

	/* else call back the cpu interface to retrieve the vector */
	else
		irq_vector = (*z80->irq_callback)(z80->device, 0);

	LOG(("Z80 '%s' single int. irq_vector $%02x\n", z80->device->tag(), irq_vector));

	/* Interrupt mode 2. Call [i:databyte] */
	if( z80->im == 2 )
	{
		irq_vector = (irq_vector & 0xff) | (z80->i << 8);
		PUSH(z80, pc);
		RM16(z80, irq_vector, &z80->pc);
		LOG(("Z80 '%s' IM2 [$%04x] = $%04x\n", z80->device->tag(), irq_vector, z80->PCD));
		/* CALL opcode timing + 'interrupt latency' cycles */
		z80->icount -= z80->cc_op[0xcd] + z80->cc_ex[0xff];
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( z80->im == 1 )
	{
		LOG(("Z80 '%s' IM1 $0038\n", z80->device->tag()));
		PUSH(z80, pc);
		z80->PCD = 0x0038;
		/* RST $38 + 'interrupt latency' cycles */
		z80->icount -= z80->cc_op[0xff] + cc_ex[0xff];
	}
	else
	{
		/* Interrupt mode 0. We check for CALL and JP instructions, */
		/* if neither of these were found we assume a 1 byte opcode */
		/* was placed on the databus                                */
		LOG(("Z80 '%s' IM0 $%04x\n", z80->device->tag(), irq_vector));

		/* check for nop */
		if (irq_vector != 0x00)
		{
			switch (irq_vector & 0xff0000)
			{
				case 0xcd0000:	/* call */
					PUSH(z80, pc);
					z80->PCD = irq_vector & 0xffff;
					 /* CALL $xxxx cycles */
					z80->icount -= z80->cc_op[0xcd];
					break;
				case 0xc30000:	/* jump */
					z80->PCD = irq_vector & 0xffff;
					/* JP $xxxx cycles */
					z80->icount -= z80->cc_op[0xc3];
					break;
				default:		/* rst (or other opcodes?) */
					PUSH(z80, pc);
					z80->PCD = irq_vector & 0x0038;
					/* RST $xx cycles */
					z80->icount -= z80->cc_op[0xff];
					break;
			}
		}

		/* 'interrupt latency' cycles */
		z80->icount -= z80->cc_ex[0xff];
	}
	z80->WZ=z80->PCD;
}

static void take_interrupt_nsc800(z80_state *z80)
{
	/* there isn't a valid previous program counter */
	z80->PRVPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT(z80);

	/* Clear both interrupt flip flops */
	z80->iff1 = z80->iff2 = 0;

	if (z80->nsc800_irq_state[NSC800_RSTA])
	{
		PUSH(z80, pc);
		z80->PCD = 0x003c;
	}
	else if (z80->nsc800_irq_state[NSC800_RSTB])
	{
		PUSH(z80, pc);
		z80->PCD = 0x0034;
	}
	else if (z80->nsc800_irq_state[NSC800_RSTC])
	{
		PUSH(z80, pc);
		z80->PCD = 0x002c;
	}

	/* 'interrupt latency' cycles */
	z80->icount -= z80->cc_op[0xff] + cc_ex[0xff];

	z80->WZ=z80->PCD;
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
static CPU_INIT( z80 )
{
	z80_state *z80 = get_safe_token(device);
	int i, p;

	if( !SZHVC_add || !SZHVC_sub )
	{
		int oldval, newval, val;
		UINT8 *padd, *padc, *psub, *psbc;
		/* allocate big flag arrays once */
		SZHVC_add = global_alloc_array(UINT8, 2*256*256);
		SZHVC_sub = global_alloc_array(UINT8, 2*256*256);
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

	device->save_item(NAME(z80->prvpc.w.l));
	device->save_item(NAME(z80->PC));
	device->save_item(NAME(z80->SP));
	device->save_item(NAME(z80->AF));
	device->save_item(NAME(z80->BC));
	device->save_item(NAME(z80->DE));
	device->save_item(NAME(z80->HL));
	device->save_item(NAME(z80->IX));
	device->save_item(NAME(z80->IY));
	device->save_item(NAME(z80->WZ));
	device->save_item(NAME(z80->af2.w.l));
	device->save_item(NAME(z80->bc2.w.l));
	device->save_item(NAME(z80->de2.w.l));
	device->save_item(NAME(z80->hl2.w.l));
	device->save_item(NAME(z80->r));
	device->save_item(NAME(z80->r2));
	device->save_item(NAME(z80->iff1));
	device->save_item(NAME(z80->iff2));
	device->save_item(NAME(z80->halt));
	device->save_item(NAME(z80->im));
	device->save_item(NAME(z80->i));
	device->save_item(NAME(z80->nmi_state));
	device->save_item(NAME(z80->nmi_pending));
	device->save_item(NAME(z80->irq_state));
	device->save_item(NAME(z80->wait_state));
	device->save_item(NAME(z80->busrq_state));
	device->save_item(NAME(z80->after_ei));
	device->save_item(NAME(z80->after_ldair));

	/* Reset registers to their initial values */
	z80->PRVPC = 0;
	z80->PCD = 0;
	z80->SPD = 0;
	z80->AFD = 0;
	z80->BCD = 0;
	z80->DED = 0;
	z80->HLD = 0;
	z80->IXD = 0;
	z80->IYD = 0;
	z80->WZ = 0;
	z80->af2.d = 0;
	z80->bc2.d = 0;
	z80->de2.d = 0;
	z80->hl2.d = 0;
	z80->r = 0;
	z80->r2 = 0;
	z80->iff1 = 0;
	z80->iff2 = 0;
	z80->halt = 0;
	z80->im = 0;
	z80->i = 0;
	z80->nmi_state = 0;
	z80->nmi_pending = 0;
	z80->irq_state = 0;
	z80->wait_state = 0;
	z80->busrq_state = 0;
	z80->after_ei = 0;
	z80->after_ldair = 0;
	z80->ea = 0;

	if (device->static_config() != NULL)
		z80->daisy.init(device, (const z80_daisy_config *)device->static_config());
	z80->irq_callback = irqcallback;
	z80->device = device;
	z80->program = device->space(AS_PROGRAM);
	z80->direct = &z80->program->direct();
	z80->io = device->space(AS_IO);
	z80->IX = z80->IY = 0xffff; /* IX and IY are FFFF after a reset! */
	z80->F = ZF;			/* Zero flag is set */

	/* set up the state table */
	{
		device_state_interface *state;
		device->interface(state);
		state->state_add(Z80_PC,          "PC",        z80->pc.w.l);
		state->state_add(STATE_GENPC,     "GENPC",     z80->pc.w.l).noshow();
		state->state_add(STATE_GENPCBASE, "GENPCBASE", z80->prvpc.w.l).noshow();
		state->state_add(Z80_SP,          "SP",        z80->SP);
		state->state_add(STATE_GENSP,     "GENSP",     z80->SP).noshow();
		state->state_add(STATE_GENFLAGS,  "GENFLAGS",  z80->F).noshow().formatstr("%8s");
		state->state_add(Z80_A,           "A",         z80->A).noshow();
		state->state_add(Z80_B,           "B",         z80->B).noshow();
		state->state_add(Z80_C,           "C",         z80->C).noshow();
		state->state_add(Z80_D,           "D",         z80->D).noshow();
		state->state_add(Z80_E,           "E",         z80->E).noshow();
		state->state_add(Z80_H,           "H",         z80->H).noshow();
		state->state_add(Z80_L,           "L",         z80->L).noshow();
		state->state_add(Z80_AF,          "AF",        z80->AF);
		state->state_add(Z80_BC,          "BC",        z80->BC);
		state->state_add(Z80_DE,          "DE",        z80->DE);
		state->state_add(Z80_HL,          "HL",        z80->HL);
		state->state_add(Z80_IX,          "IX",        z80->IX);
		state->state_add(Z80_IY,          "IY",        z80->IY);
		state->state_add(Z80_AF2,         "AF2",       z80->af2.w.l);
		state->state_add(Z80_BC2,         "BC2",       z80->bc2.w.l);
		state->state_add(Z80_DE2,         "DE2",       z80->de2.w.l);
		state->state_add(Z80_HL2,         "HL2",       z80->hl2.w.l);
		state->state_add(Z80_WZ,          "WZ",        z80->WZ);
		state->state_add(Z80_R,           "R",         z80->rtemp).callimport().callexport();
		state->state_add(Z80_I,           "I",         z80->i);
		state->state_add(Z80_IM,          "IM",        z80->im).mask(0x3);
		state->state_add(Z80_IFF1,        "IFF1",      z80->iff1).mask(0x1);
		state->state_add(Z80_IFF2,        "IFF2",      z80->iff2).mask(0x1);
		state->state_add(Z80_HALT,        "HALT",      z80->halt).mask(0x1);
	}

	/* setup cycle tables */
	z80->cc_op = cc_op;
	z80->cc_cb = cc_cb;
	z80->cc_ed = cc_ed;
	z80->cc_xy = cc_xy;
	z80->cc_xycb = cc_xycb;
	z80->cc_ex = cc_ex;
}

static CPU_INIT( nsc800 )
{
	z80_state *z80 = get_safe_token(device);
	device->save_item(NAME(z80->nsc800_irq_state));
	CPU_INIT_CALL (z80);
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
static CPU_RESET( z80 )
{
	z80_state *z80 = get_safe_token(device);

	z80->PC = 0x0000;
	z80->i = 0;
	z80->r = 0;
	z80->r2 = 0;
	z80->nmi_state = CLEAR_LINE;
	z80->nmi_pending = FALSE;
	z80->irq_state = CLEAR_LINE;
	z80->wait_state = CLEAR_LINE;
	z80->busrq_state = CLEAR_LINE;
	z80->after_ei = FALSE;
	z80->after_ldair = FALSE;
	z80->iff1 = 0;
	z80->iff2 = 0;

	z80->daisy.reset();

	z80->WZ=z80->PCD;
}

 static CPU_RESET( nsc800 )
{
	z80_state *z80 = get_safe_token(device);
	memset(z80->nsc800_irq_state, 0, sizeof(z80->nsc800_irq_state));
	CPU_RESET_CALL(z80);
}

static CPU_EXIT( z80 )
{
	global_free(SZHVC_add);
	SZHVC_add = NULL;
	global_free(SZHVC_sub);
	SZHVC_sub = NULL;
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
static CPU_EXECUTE( z80 )
{
	z80_state *z80 = get_safe_token(device);

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (z80->nmi_pending)
	{
		LOG(("Z80 '%s' take NMI\n", z80->device->tag()));
		z80->PRVPC = -1;			/* there isn't a valid previous program counter */
		LEAVE_HALT(z80);			/* Check if processor was halted */

#if HAS_LDAIR_QUIRK
		/* reset parity flag after LD A,I or LD A,R */
		if (z80->after_ldair) z80->F &= ~PF;
#endif
		z80->after_ldair = FALSE;

		z80->iff1 = 0;
		PUSH(z80, pc);
		z80->PCD = 0x0066;
		z80->WZ=z80->PCD;
		z80->icount -= 11;
		z80->nmi_pending = FALSE;
	}

	do
	{
		/* check for IRQs before each instruction */
		if (z80->irq_state != CLEAR_LINE && z80->iff1 && !z80->after_ei)
		{
#if HAS_LDAIR_QUIRK
			/* reset parity flag after LD A,I or LD A,R */
			if (z80->after_ldair) z80->F &= ~PF;
#endif
			take_interrupt(z80);
		}
		z80->after_ei = FALSE;
		z80->after_ldair = FALSE;

		z80->PRVPC = z80->PCD;
		debugger_instruction_hook(device, z80->PCD);
		z80->r++;
		EXEC_INLINE(z80,op,ROP(z80));
	} while (z80->icount > 0);
}

 static CPU_EXECUTE( nsc800 )
{
	z80_state *z80 = get_safe_token(device);

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (z80->nmi_pending)
	{
		LOG(("Z80 '%s' take NMI\n", z80->device->tag()));
		z80->PRVPC = -1;			/* there isn't a valid previous program counter */
		LEAVE_HALT(z80);			/* Check if processor was halted */

		z80->iff1 = 0;
		PUSH(z80, pc);
		z80->PCD = 0x0066;
		z80->WZ=z80->PCD;
		z80->icount -= 11;
		z80->nmi_pending = FALSE;
	}

	do
	{
		/* check for NSC800 IRQs line RSTA, RSTB, RSTC */
		if ((z80->nsc800_irq_state[NSC800_RSTA] != CLEAR_LINE ||
			z80->nsc800_irq_state[NSC800_RSTB] != CLEAR_LINE ||
			z80->nsc800_irq_state[NSC800_RSTC] != CLEAR_LINE) && z80->iff1 && !z80->after_ei)
				take_interrupt_nsc800(z80);

		/* check for IRQs before each instruction */
		if (z80->irq_state != CLEAR_LINE && z80->iff1 && !z80->after_ei)
			take_interrupt(z80);

		z80->after_ei = FALSE;

		z80->PRVPC = z80->PCD;
		debugger_instruction_hook(device, z80->PCD);
		z80->r++;
		EXEC_INLINE(z80,op,ROP(z80));
	} while (z80->icount > 0);
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(z80_state *z80, int irqline, int state)
{
	switch (irqline)
	{
	case Z80_INPUT_LINE_BUSRQ:
		z80->busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		/* mark an NMI pending on the rising edge */
		if (z80->nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			z80->nmi_pending = TRUE;
		z80->nmi_state = state;
		break;

	case INPUT_LINE_IRQ0:
		/* update the IRQ state via the daisy chain */
		z80->irq_state = state;
		if (z80->daisy.present())
			z80->irq_state = ( z80->daisy.update_irq_state() == ASSERT_LINE ) ? ASSERT_LINE : z80->irq_state;

		/* the main execute loop will take the interrupt */
		break;

	case Z80_INPUT_LINE_WAIT:
		z80->wait_state = state;
		break;
	}
}


static void set_irq_line_nsc800(z80_state *z80, int irqline, int state)
{
	switch (irqline)
	{
	case Z80_INPUT_LINE_BUSRQ:
		z80->busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		/* mark an NMI pending on the rising edge */
		if (z80->nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			z80->nmi_pending = TRUE;
		z80->nmi_state = state;
		break;

	case NSC800_RSTA:
		z80->nsc800_irq_state[NSC800_RSTA] = state;
		break;

	case NSC800_RSTB:
		z80->nsc800_irq_state[NSC800_RSTB] = state;
		break;

	case NSC800_RSTC:
		z80->nsc800_irq_state[NSC800_RSTC] = state;
		break;

	case INPUT_LINE_IRQ0:
		/* update the IRQ state via the daisy chain */
		z80->irq_state = state;
		if (z80->daisy.present())
			z80->irq_state = z80->daisy.update_irq_state();

		/* the main execute loop will take the interrupt */
		break;

	case Z80_INPUT_LINE_WAIT:
		z80->wait_state = state;
		break;
	}
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( z80 )
{
	z80_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case Z80_R:
			cpustate->r = cpustate->rtemp & 0x7f;
			cpustate->r2 = cpustate->rtemp & 0x80;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z80) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STATE( z80 )
{
	z80_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case Z80_R:
			cpustate->rtemp = (cpustate->r & 0x7f) | (cpustate->r2 & 0x80);
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z80) called for unexpected value\n");
			break;
	}
}

static CPU_EXPORT_STRING( z80 )
{
	z80_state *cpustate = get_safe_token(device);

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				cpustate->F & 0x80 ? 'S':'.',
				cpustate->F & 0x40 ? 'Z':'.',
				cpustate->F & 0x20 ? 'Y':'.',
				cpustate->F & 0x10 ? 'H':'.',
				cpustate->F & 0x08 ? 'X':'.',
				cpustate->F & 0x04 ? 'P':'.',
				cpustate->F & 0x02 ? 'N':'.',
				cpustate->F & 0x01 ? 'C':'.');
			break;
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( z80 )
{
	z80_state *z80 = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_BUSRQ:	set_irq_line(z80, Z80_INPUT_LINE_BUSRQ, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:			set_irq_line(z80, INPUT_LINE_NMI, info->i); 		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:			set_irq_line(z80, INPUT_LINE_IRQ0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_WAIT:		set_irq_line(z80, Z80_INPUT_LINE_WAIT, info->i);	break;
	}
}

static CPU_SET_INFO( nsc800 )
{
	z80_state *z80 = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_BUSRQ:	set_irq_line(z80, Z80_INPUT_LINE_BUSRQ, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:			set_irq_line_nsc800(z80, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + NSC800_RSTA:				set_irq_line_nsc800(z80, NSC800_RSTA, info->i); 	break;
		case CPUINFO_INT_INPUT_STATE + NSC800_RSTB:				set_irq_line_nsc800(z80, NSC800_RSTB, info->i); 	break;
		case CPUINFO_INT_INPUT_STATE + NSC800_RSTC:				set_irq_line_nsc800(z80, NSC800_RSTC, info->i); 	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:			set_irq_line_nsc800(z80, INPUT_LINE_IRQ0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_WAIT:		set_irq_line(z80, Z80_INPUT_LINE_WAIT, info->i);	break;
	}
}

void z80_set_cycle_tables(device_t *device, const UINT8 *op, const UINT8 *cb, const UINT8 *ed, const UINT8 *xy, const UINT8 *xycb, const UINT8 *ex)
{
	z80_state *z80 = get_safe_token(device);

	z80->cc_op = (op != NULL) ? op : cc_op;
	z80->cc_cb = (cb != NULL) ? cb : cc_cb;
	z80->cc_ed = (ed != NULL) ? ed : cc_ed;
	z80->cc_xy = (xy != NULL) ? xy : cc_xy;
	z80->cc_xycb = (xycb != NULL) ? xycb : cc_xycb;
	z80->cc_ex = (ex != NULL) ? ex : cc_ex;
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( z80 )
{
	z80_state *z80 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(z80_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 4;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:		info->i = 8;						break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:		info->i = 16;						break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:		info->i = 0;						break;
		case DEVINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;						break;
		case DEVINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 16;						break;
		case DEVINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;						break;

		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_BUSRQ:	info->i = z80->busrq_state;		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:			info->i = z80->nmi_state;		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_IRQ0:			info->i = z80->irq_state;		break;
		case CPUINFO_INT_INPUT_STATE + Z80_INPUT_LINE_WAIT:		info->i = z80->wait_state;		break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(z80);					break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(z80);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(z80);						break;
		case CPUINFO_FCT_EXIT:			info->exit = CPU_EXIT_NAME(z80);						break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(z80);					break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(z80);			break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(z80);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(z80);		break;
		case CPUINFO_FCT_EXPORT_STRING:	info->export_string = CPU_EXPORT_STRING_NAME(z80);		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &z80->icount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Z80");					break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Zilog Z80");			break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "3.9");					break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;
	}
}

CPU_GET_INFO( nsc800 )
{
	z80_state *z80 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		case CPUINFO_INT_INPUT_LINES:					info->i = 7;									break;

		case CPUINFO_INT_INPUT_STATE + NSC800_RSTA:		info->i = z80->nsc800_irq_state[NSC800_RSTA];	break;
		case CPUINFO_INT_INPUT_STATE + NSC800_RSTB:		info->i = z80->nsc800_irq_state[NSC800_RSTB];	break;
		case CPUINFO_INT_INPUT_STATE + NSC800_RSTC:		info->i = z80->nsc800_irq_state[NSC800_RSTC];	break;

		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(nsc800);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(nsc800);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(nsc800);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(nsc800);		break;

		case DEVINFO_STR_NAME:							strcpy(info->s, "NSC800");						break;

		default:										CPU_GET_INFO_CALL(z80); 						break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(Z80, z80);
DEFINE_LEGACY_CPU_DEVICE(NSC800, nsc800);
