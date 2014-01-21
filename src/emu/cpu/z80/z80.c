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

#undef BIT

#define VERBOSE             0

/* On an NMOS Z80, if LD A,I or LD A,R is interrupted, P/V flag gets reset,
   even if IFF2 was set before this instruction. This issue was fixed on
   the CMOS Z80, so until knowing (most) Z80 types on hardware, it's disabled */
#define HAS_LDAIR_QUIRK     0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/****************************************************************************/
/* The Z80 registers. halt is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(r&127)|(r2&128)    */
/****************************************************************************/

#define CF      0x01
#define NF      0x02
#define PF      0x04
#define VF      PF
#define XF      0x08
#define HF      0x10
#define YF      0x20
#define ZF      0x40
#define SF      0x80

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

#define PRVPC   m_prvpc.d     /* previous program counter */

#define PCD     m_pc.d
#define PC      m_pc.w.l

#define SPD     m_sp.d
#define SP      m_sp.w.l

#define AFD     m_af.d
#define AF      m_af.w.l
#define A       m_af.b.h
#define F       m_af.b.l

#define BCD     m_bc.d
#define BC      m_bc.w.l
#define B       m_bc.b.h
#define C       m_bc.b.l

#define DED     m_de.d
#define DE      m_de.w.l
#define D       m_de.b.h
#define E       m_de.b.l

#define HLD     m_hl.d
#define HL      m_hl.w.l
#define H       m_hl.b.h
#define L       m_hl.b.l

#define IXD     m_ix.d
#define IX      m_ix.w.l
#define HX      m_ix.b.h
#define LX      m_ix.b.l

#define IYD     m_iy.d
#define IY      m_iy.w.l
#define HY      m_iy.b.h
#define LY      m_iy.b.l

#define WZ      m_wz.w.l
#define WZ_H    m_wz.b.h
#define WZ_L    m_wz.b.l


static bool tables_initialised = false;
static UINT8 SZ[256];       /* zero and sign flags */
static UINT8 SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];      /* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static UINT8 SZHVC_add[2*256*256];
static UINT8 SZHVC_sub[2*256*256];

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
	5,10,10,10,10,11, 7,11, 5,10,10, 0,10,17, 7,11, /* cb -> cc_cb */
	5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 0, 7,11, /* dd -> cc_xy */
	5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 0, 7,11, /* ed -> cc_ed */
	5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 0, 7,11      /* fd -> cc_xy */
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
	5+4,10+4,10+4,10+4,10+4,11+4, 7+4,11+4, 5+4,10+4,10+4, 0  ,10+4,17+4, 7+4,11+4, /* cb -> cc_xycb */
	5+4,10+4,10+4,11+4,10+4,11+4, 7+4,11+4, 5+4, 4+4,10+4,11+4,10+4, 4  , 7+4,11+4, /* dd -> cc_xy again */
	5+4,10+4,10+4,19+4,10+4,11+4, 7+4,11+4, 5+4, 4+4,10+4, 4+4,10+4, 4  , 7+4,11+4, /* ed -> cc_ed */
	5+4,10+4,10+4, 4+4,10+4,11+4, 7+4,11+4, 5+4, 6+4,10+4, 4+4,10+4, 4  , 7+4,11+4      /* fd -> cc_xy again */
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
	5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* DJNZ */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NZ/JR Z */
	5, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, /* JR NC/JR C */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	5, 5, 5, 5, 0, 0, 0, 0, 5, 5, 5, 5, 0, 0, 0, 0, /* LDIR/CPIR/INIR/OTIR LDDR/CPDR/INDR/OTDR */
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2,
	6, 0, 0, 0, 7, 0, 0, 2, 6, 0, 0, 0, 7, 0, 0, 2
};

#define m_cc_dd   m_cc_xy
#define m_cc_fd   m_cc_xy

/***************************************************************
 * define an opcode function
 ***************************************************************/
#define OP(prefix,opcode) inline void z80_device::prefix##_##opcode()

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) do { m_icount -= m_cc_##prefix[opcode]; } while (0)

#define EXEC(prefix,opcode) do { \
	unsigned op = opcode; \
	CC(prefix,op); \
	switch(op) \
	{  \
	case 0x00:prefix##_##00();break; case 0x01:prefix##_##01();break; case 0x02:prefix##_##02();break; case 0x03:prefix##_##03();break; \
	case 0x04:prefix##_##04();break; case 0x05:prefix##_##05();break; case 0x06:prefix##_##06();break; case 0x07:prefix##_##07();break; \
	case 0x08:prefix##_##08();break; case 0x09:prefix##_##09();break; case 0x0a:prefix##_##0a();break; case 0x0b:prefix##_##0b();break; \
	case 0x0c:prefix##_##0c();break; case 0x0d:prefix##_##0d();break; case 0x0e:prefix##_##0e();break; case 0x0f:prefix##_##0f();break; \
	case 0x10:prefix##_##10();break; case 0x11:prefix##_##11();break; case 0x12:prefix##_##12();break; case 0x13:prefix##_##13();break; \
	case 0x14:prefix##_##14();break; case 0x15:prefix##_##15();break; case 0x16:prefix##_##16();break; case 0x17:prefix##_##17();break; \
	case 0x18:prefix##_##18();break; case 0x19:prefix##_##19();break; case 0x1a:prefix##_##1a();break; case 0x1b:prefix##_##1b();break; \
	case 0x1c:prefix##_##1c();break; case 0x1d:prefix##_##1d();break; case 0x1e:prefix##_##1e();break; case 0x1f:prefix##_##1f();break; \
	case 0x20:prefix##_##20();break; case 0x21:prefix##_##21();break; case 0x22:prefix##_##22();break; case 0x23:prefix##_##23();break; \
	case 0x24:prefix##_##24();break; case 0x25:prefix##_##25();break; case 0x26:prefix##_##26();break; case 0x27:prefix##_##27();break; \
	case 0x28:prefix##_##28();break; case 0x29:prefix##_##29();break; case 0x2a:prefix##_##2a();break; case 0x2b:prefix##_##2b();break; \
	case 0x2c:prefix##_##2c();break; case 0x2d:prefix##_##2d();break; case 0x2e:prefix##_##2e();break; case 0x2f:prefix##_##2f();break; \
	case 0x30:prefix##_##30();break; case 0x31:prefix##_##31();break; case 0x32:prefix##_##32();break; case 0x33:prefix##_##33();break; \
	case 0x34:prefix##_##34();break; case 0x35:prefix##_##35();break; case 0x36:prefix##_##36();break; case 0x37:prefix##_##37();break; \
	case 0x38:prefix##_##38();break; case 0x39:prefix##_##39();break; case 0x3a:prefix##_##3a();break; case 0x3b:prefix##_##3b();break; \
	case 0x3c:prefix##_##3c();break; case 0x3d:prefix##_##3d();break; case 0x3e:prefix##_##3e();break; case 0x3f:prefix##_##3f();break; \
	case 0x40:prefix##_##40();break; case 0x41:prefix##_##41();break; case 0x42:prefix##_##42();break; case 0x43:prefix##_##43();break; \
	case 0x44:prefix##_##44();break; case 0x45:prefix##_##45();break; case 0x46:prefix##_##46();break; case 0x47:prefix##_##47();break; \
	case 0x48:prefix##_##48();break; case 0x49:prefix##_##49();break; case 0x4a:prefix##_##4a();break; case 0x4b:prefix##_##4b();break; \
	case 0x4c:prefix##_##4c();break; case 0x4d:prefix##_##4d();break; case 0x4e:prefix##_##4e();break; case 0x4f:prefix##_##4f();break; \
	case 0x50:prefix##_##50();break; case 0x51:prefix##_##51();break; case 0x52:prefix##_##52();break; case 0x53:prefix##_##53();break; \
	case 0x54:prefix##_##54();break; case 0x55:prefix##_##55();break; case 0x56:prefix##_##56();break; case 0x57:prefix##_##57();break; \
	case 0x58:prefix##_##58();break; case 0x59:prefix##_##59();break; case 0x5a:prefix##_##5a();break; case 0x5b:prefix##_##5b();break; \
	case 0x5c:prefix##_##5c();break; case 0x5d:prefix##_##5d();break; case 0x5e:prefix##_##5e();break; case 0x5f:prefix##_##5f();break; \
	case 0x60:prefix##_##60();break; case 0x61:prefix##_##61();break; case 0x62:prefix##_##62();break; case 0x63:prefix##_##63();break; \
	case 0x64:prefix##_##64();break; case 0x65:prefix##_##65();break; case 0x66:prefix##_##66();break; case 0x67:prefix##_##67();break; \
	case 0x68:prefix##_##68();break; case 0x69:prefix##_##69();break; case 0x6a:prefix##_##6a();break; case 0x6b:prefix##_##6b();break; \
	case 0x6c:prefix##_##6c();break; case 0x6d:prefix##_##6d();break; case 0x6e:prefix##_##6e();break; case 0x6f:prefix##_##6f();break; \
	case 0x70:prefix##_##70();break; case 0x71:prefix##_##71();break; case 0x72:prefix##_##72();break; case 0x73:prefix##_##73();break; \
	case 0x74:prefix##_##74();break; case 0x75:prefix##_##75();break; case 0x76:prefix##_##76();break; case 0x77:prefix##_##77();break; \
	case 0x78:prefix##_##78();break; case 0x79:prefix##_##79();break; case 0x7a:prefix##_##7a();break; case 0x7b:prefix##_##7b();break; \
	case 0x7c:prefix##_##7c();break; case 0x7d:prefix##_##7d();break; case 0x7e:prefix##_##7e();break; case 0x7f:prefix##_##7f();break; \
	case 0x80:prefix##_##80();break; case 0x81:prefix##_##81();break; case 0x82:prefix##_##82();break; case 0x83:prefix##_##83();break; \
	case 0x84:prefix##_##84();break; case 0x85:prefix##_##85();break; case 0x86:prefix##_##86();break; case 0x87:prefix##_##87();break; \
	case 0x88:prefix##_##88();break; case 0x89:prefix##_##89();break; case 0x8a:prefix##_##8a();break; case 0x8b:prefix##_##8b();break; \
	case 0x8c:prefix##_##8c();break; case 0x8d:prefix##_##8d();break; case 0x8e:prefix##_##8e();break; case 0x8f:prefix##_##8f();break; \
	case 0x90:prefix##_##90();break; case 0x91:prefix##_##91();break; case 0x92:prefix##_##92();break; case 0x93:prefix##_##93();break; \
	case 0x94:prefix##_##94();break; case 0x95:prefix##_##95();break; case 0x96:prefix##_##96();break; case 0x97:prefix##_##97();break; \
	case 0x98:prefix##_##98();break; case 0x99:prefix##_##99();break; case 0x9a:prefix##_##9a();break; case 0x9b:prefix##_##9b();break; \
	case 0x9c:prefix##_##9c();break; case 0x9d:prefix##_##9d();break; case 0x9e:prefix##_##9e();break; case 0x9f:prefix##_##9f();break; \
	case 0xa0:prefix##_##a0();break; case 0xa1:prefix##_##a1();break; case 0xa2:prefix##_##a2();break; case 0xa3:prefix##_##a3();break; \
	case 0xa4:prefix##_##a4();break; case 0xa5:prefix##_##a5();break; case 0xa6:prefix##_##a6();break; case 0xa7:prefix##_##a7();break; \
	case 0xa8:prefix##_##a8();break; case 0xa9:prefix##_##a9();break; case 0xaa:prefix##_##aa();break; case 0xab:prefix##_##ab();break; \
	case 0xac:prefix##_##ac();break; case 0xad:prefix##_##ad();break; case 0xae:prefix##_##ae();break; case 0xaf:prefix##_##af();break; \
	case 0xb0:prefix##_##b0();break; case 0xb1:prefix##_##b1();break; case 0xb2:prefix##_##b2();break; case 0xb3:prefix##_##b3();break; \
	case 0xb4:prefix##_##b4();break; case 0xb5:prefix##_##b5();break; case 0xb6:prefix##_##b6();break; case 0xb7:prefix##_##b7();break; \
	case 0xb8:prefix##_##b8();break; case 0xb9:prefix##_##b9();break; case 0xba:prefix##_##ba();break; case 0xbb:prefix##_##bb();break; \
	case 0xbc:prefix##_##bc();break; case 0xbd:prefix##_##bd();break; case 0xbe:prefix##_##be();break; case 0xbf:prefix##_##bf();break; \
	case 0xc0:prefix##_##c0();break; case 0xc1:prefix##_##c1();break; case 0xc2:prefix##_##c2();break; case 0xc3:prefix##_##c3();break; \
	case 0xc4:prefix##_##c4();break; case 0xc5:prefix##_##c5();break; case 0xc6:prefix##_##c6();break; case 0xc7:prefix##_##c7();break; \
	case 0xc8:prefix##_##c8();break; case 0xc9:prefix##_##c9();break; case 0xca:prefix##_##ca();break; case 0xcb:prefix##_##cb();break; \
	case 0xcc:prefix##_##cc();break; case 0xcd:prefix##_##cd();break; case 0xce:prefix##_##ce();break; case 0xcf:prefix##_##cf();break; \
	case 0xd0:prefix##_##d0();break; case 0xd1:prefix##_##d1();break; case 0xd2:prefix##_##d2();break; case 0xd3:prefix##_##d3();break; \
	case 0xd4:prefix##_##d4();break; case 0xd5:prefix##_##d5();break; case 0xd6:prefix##_##d6();break; case 0xd7:prefix##_##d7();break; \
	case 0xd8:prefix##_##d8();break; case 0xd9:prefix##_##d9();break; case 0xda:prefix##_##da();break; case 0xdb:prefix##_##db();break; \
	case 0xdc:prefix##_##dc();break; case 0xdd:prefix##_##dd();break; case 0xde:prefix##_##de();break; case 0xdf:prefix##_##df();break; \
	case 0xe0:prefix##_##e0();break; case 0xe1:prefix##_##e1();break; case 0xe2:prefix##_##e2();break; case 0xe3:prefix##_##e3();break; \
	case 0xe4:prefix##_##e4();break; case 0xe5:prefix##_##e5();break; case 0xe6:prefix##_##e6();break; case 0xe7:prefix##_##e7();break; \
	case 0xe8:prefix##_##e8();break; case 0xe9:prefix##_##e9();break; case 0xea:prefix##_##ea();break; case 0xeb:prefix##_##eb();break; \
	case 0xec:prefix##_##ec();break; case 0xed:prefix##_##ed();break; case 0xee:prefix##_##ee();break; case 0xef:prefix##_##ef();break; \
	case 0xf0:prefix##_##f0();break; case 0xf1:prefix##_##f1();break; case 0xf2:prefix##_##f2();break; case 0xf3:prefix##_##f3();break; \
	case 0xf4:prefix##_##f4();break; case 0xf5:prefix##_##f5();break; case 0xf6:prefix##_##f6();break; case 0xf7:prefix##_##f7();break; \
	case 0xf8:prefix##_##f8();break; case 0xf9:prefix##_##f9();break; case 0xfa:prefix##_##fa();break; case 0xfb:prefix##_##fb();break; \
	case 0xfc:prefix##_##fc();break; case 0xfd:prefix##_##fd();break; case 0xfe:prefix##_##fe();break; case 0xff:prefix##_##ff();break; \
	} \
} while (0)

/***************************************************************
 * Enter halt state; write 1 to fake port on first execution
 ***************************************************************/
inline void z80_device::ENTER_HALT()
{
	PC--;
	m_halt = 1;
}

/***************************************************************
 * Leave halt state; write 0 to fake port
 ***************************************************************/
inline void z80_device::LEAVE_HALT()
{
	if( m_halt )
	{
		m_halt = 0;
		PC++;
	}
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
inline UINT8 z80_device::IN(UINT16 port)
{
	return m_io->read_byte(port);
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
inline void z80_device::OUT(UINT16 port, UINT8 value)
{
	m_io->write_byte(port, value);
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
inline UINT8 z80_device::RM(UINT16 addr)
{
	return m_program->read_byte(addr);
}

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
inline void z80_device::RM16(UINT16 addr, PAIR &r)
{
	r.b.l = RM(addr);
	r.b.h = RM((addr+1));
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
inline void z80_device::WM(UINT16 addr, UINT8 value)
{
	m_program->write_byte(addr, value);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
inline void z80_device::WM16(UINT16 addr, PAIR &r)
{
	WM(addr, r.b.l);
	WM((addr+1), r.b.h);
}

/***************************************************************
 * ROP() is identical to RM() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
inline UINT8 z80_device::ROP()
{
	unsigned pc = PCD;
	PC++;
	return m_direct->read_decrypted_byte(pc);
}

/****************************************************************
 * ARG() is identical to ROP() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
inline UINT8 z80_device::ARG()
{
	unsigned pc = PCD;
	PC++;
	return m_direct->read_raw_byte(pc);
}

inline UINT16 z80_device::ARG16()
{
	unsigned pc = PCD;
	PC += 2;
	return m_direct->read_raw_byte(pc) | (m_direct->read_raw_byte((pc+1)&0xffff) << 8);
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
inline void z80_device::EAX()
{
	m_ea = (UINT32)(UINT16)(IX + (INT8)ARG());
	WZ = m_ea;
}

inline void z80_device::EAY()
{
	m_ea = (UINT32)(UINT16)(IY + (INT8)ARG());
	WZ = m_ea;
}

/***************************************************************
 * POP
 ***************************************************************/
inline void z80_device::POP(PAIR &r)
{
	RM16(SPD, r);
	SP += 2;
}

/***************************************************************
 * PUSH
 ***************************************************************/
inline void z80_device::PUSH(PAIR &r)
{
	SP -= 2;
	WM16(SPD, r);
}

/***************************************************************
 * JP
 ***************************************************************/
inline void z80_device::JP(void)
{
	PCD = ARG16();
	WZ = PCD;
}

/***************************************************************
 * JP_COND
 ***************************************************************/
inline void z80_device::JP_COND(bool cond)
{
	if (cond)
	{
		PCD = ARG16();
		WZ = PCD;
	}
	else
	{
		WZ = ARG16(); /* implicit do PC += 2 */
	}
}

/***************************************************************
 * JR
 ***************************************************************/
inline void z80_device::JR()
{
	INT8 arg = (INT8)ARG();    /* ARG() also increments PC */
	PC += arg;             /* so don't do PC += ARG() */
	WZ = PC;
}

/***************************************************************
 * JR_COND
 ***************************************************************/
inline void z80_device::JR_COND(bool cond, UINT8 opcode)
{
	if (cond)
	{
		JR();
		CC(ex, opcode);
	}
	else
		PC++;
}

/***************************************************************
 * CALL
 ***************************************************************/
inline void z80_device::CALL()
{
	m_ea = ARG16();
	WZ = m_ea;
	PUSH(m_pc);
	PCD = m_ea;
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
inline void z80_device::CALL_COND(bool cond, UINT8 opcode)
{
	if (cond)
	{
		m_ea = ARG16();
		WZ = m_ea;
		PUSH(m_pc);
		PCD = m_ea;
		CC(ex, opcode);
	}
	else
	{
		WZ = ARG16();  /* implicit call PC+=2;   */
	}
}

/***************************************************************
 * RET_COND
 ***************************************************************/
inline void z80_device::RET_COND(bool cond, UINT8 opcode)
{
	if (cond)
	{
		POP(m_pc);
		WZ = PC;
		CC(ex, opcode);
	}
}

/***************************************************************
 * RETN
 ***************************************************************/
inline void z80_device::RETN()
{
	LOG(("Z80 '%s' RETN m_iff1:%d m_iff2:%d\n", 
		tag(), m_iff1, m_iff2));
	POP(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
}

/***************************************************************
 * RETI
 ***************************************************************/
inline void z80_device::RETI()
{
	POP(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
	m_daisy.call_reti_device();
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
inline void z80_device::LD_R_A()
{
	m_r = A;
	m_r2 = A & 0x80;            /* keep bit 7 of r */
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
inline void z80_device::LD_A_R()
{
	A = (m_r & 0x7f) | m_r2;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = TRUE;
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
inline void z80_device::LD_I_A()
{
	m_i = A;
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
inline void z80_device::LD_A_I()
{
	A = m_i;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = TRUE;
}

/***************************************************************
 * RST
 ***************************************************************/
inline void z80_device::RST(UINT16 addr)
{
	PUSH(m_pc);
	PCD = addr;
	WZ = PC;
}

/***************************************************************
 * INC  r8
 ***************************************************************/
inline UINT8 z80_device::INC(UINT8 value)
{
	UINT8 res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline UINT8 z80_device::DEC(UINT8 value)
{
	UINT8 res = value - 1;
	F = (F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
inline void z80_device::RLCA()
{
	A = (A << 1) | (A >> 7);
	F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF));
}

/***************************************************************
 * RRCA
 ***************************************************************/
inline void z80_device::RRCA()
{
	F = (F & (SF | ZF | PF)) | (A & CF);
	A = (A >> 1) | (A << 7);
	F |= (A & (YF | XF));
}

/***************************************************************
 * RLA
 ***************************************************************/
inline void z80_device::RLA()
{
	UINT8 res = (A << 1) | (F & CF);
	UINT8 c = (A & 0x80) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void z80_device::RRA()
{
	UINT8 res = (A >> 1) | (F << 7);
	UINT8 c = (A & 0x01) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
inline void z80_device::RRD()
{
	UINT8 n = RM(HL);
	WZ = HL+1;
	WM(HL, (n >> 4) | (A << 4));
	A = (A & 0xf0) | (n & 0x0f);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * RLD
 ***************************************************************/
inline void z80_device::RLD()
{
	UINT8 n = RM(HL);
	WZ = HL+1;
	WM(HL, (n << 4) | (A & 0x0f));
	A = (A & 0xf0) | (n >> 4);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void z80_device::ADD(UINT8 value)
{
	UINT32 ah = AFD & 0xff00;
	UINT32 res = (UINT8)((ah >> 8) + value);
	F = SZHVC_add[ah | res];
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void z80_device::ADC(UINT8 value)
{
	UINT32 ah = AFD & 0xff00, c = AFD & 1;
	UINT32 res = (UINT8)((ah >> 8) + value + c);
	F = SZHVC_add[(c << 16) | ah | res];
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void z80_device::SUB(UINT8 value)
{
	UINT32 ah = AFD & 0xff00;
	UINT32 res = (UINT8)((ah >> 8) - value);
	F = SZHVC_sub[ah | res];
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void z80_device::SBC(UINT8 value)
{
	UINT32 ah = AFD & 0xff00, c = AFD & 1;
	UINT32 res = (UINT8)((ah >> 8) - value - c);
	F = SZHVC_sub[(c<<16) | ah | res];
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
inline void z80_device::NEG()
{
	UINT8 value = A;
	A = 0;
	SUB(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
inline void z80_device::DAA()
{
	UINT8 a = A;
	if (F & NF) {
		if ((F&HF) | ((A&0xf)>9)) a-=6;
		if ((F&CF) | (A>0x99)) a-=0x60;
	}
	else {
		if ((F&HF) | ((A&0xf)>9)) a+=6;
		if ((F&CF) | (A>0x99)) a+=0x60;
	}

	F = (F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a];
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
inline void z80_device::AND(UINT8 value)
{
	A &= value;
	F = SZP[A] | HF;
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void z80_device::OR(UINT8 value)
{
	A |= value;
	F = SZP[A];
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void z80_device::XOR(UINT8 value)
{
	A ^= value;
	F = SZP[A];
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void z80_device::CP(UINT8 value)
{
	unsigned val = value;
	UINT32 ah = AFD & 0xff00;
	UINT32 res = (UINT8)((ah >> 8) - val);
	F = (SZHVC_sub[ah | res] & ~(YF | XF)) |
		(val & (YF | XF));
}

/***************************************************************
 * EX   AF,AF'
 ***************************************************************/
inline void z80_device::EX_AF()
{
	PAIR tmp;
	tmp = m_af; m_af = m_af2; m_af2 = tmp;
}

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
inline void z80_device::EX_DE_HL()
{
	PAIR tmp;
	tmp = m_de; m_de = m_hl; m_hl = tmp;
}

/***************************************************************
 * EXX
 ***************************************************************/
inline void z80_device::EXX()
{
	PAIR tmp;
	tmp = m_bc; m_bc = m_bc2; m_bc2 = tmp;
	tmp = m_de; m_de = m_de2; m_de2 = tmp;
	tmp = m_hl; m_hl = m_hl2; m_hl2 = tmp;
}

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
inline void z80_device::EXSP(PAIR &r)
{
	PAIR tmp = { { 0, 0, 0, 0 } };
	RM16(SPD, tmp);
	WM16(SPD, r);
	r = tmp;
	WZ = r.d;
}

/***************************************************************
 * ADD16
 ***************************************************************/
inline void z80_device::ADD16(PAIR &dr, PAIR &sr)
{
	UINT32 res = dr.d + sr.d;
	WZ = dr.d + 1;
	F = (F & (SF | ZF | VF)) |
		(((dr.d ^ res ^ sr.d) >> 8) & HF) |
		((res >> 16) & CF) | ((res >> 8) & (YF | XF));
	dr.w.l = (UINT16)res;
}

/***************************************************************
 * ADC  r16,r16
 ***************************************************************/
inline void z80_device::ADC16(PAIR &r)
{
	UINT32 res = HLD + r.d + (F & CF);
	WZ = HL + 1;
	F = (((HLD ^ res ^ r.d) >> 8) & HF) |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((r.d ^ HLD ^ 0x8000) & (r.d ^ res) & 0x8000) >> 13);
	HL = (UINT16)res;
}

/***************************************************************
 * SBC  r16,r16
 ***************************************************************/
inline void z80_device::SBC16(PAIR &r)
{
	UINT32 res = HLD - r.d - (F & CF);
	WZ = HL + 1;
	F = (((HLD ^ res ^ r.d) >> 8) & HF) | NF |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((r.d ^ HLD) & (HLD ^ res) &0x8000) >> 13);
	HL = (UINT16)res;
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
inline UINT8 z80_device::RLC(UINT8 value)
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
inline UINT8 z80_device::RRC(UINT8 value)
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
inline UINT8 z80_device::RL(UINT8 value)
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
inline UINT8 z80_device::RR(UINT8 value)
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
inline UINT8 z80_device::SLA(UINT8 value)
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
inline UINT8 z80_device::SRA(UINT8 value)
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
inline UINT8 z80_device::SLL(UINT8 value)
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
inline UINT8 z80_device::SRL(UINT8 value)
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
inline void z80_device::BIT(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (value & (YF|XF));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void z80_device::BIT_HL(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void z80_device::BIT_XY(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | ((m_ea>>8) & (YF|XF));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
inline UINT8 z80_device::RES(int bit, UINT8 value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
inline UINT8 z80_device::SET(int bit, UINT8 value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
inline void z80_device::LDI()
{
	UINT8 io = RM(HL);
	WM(DE, io);
	F &= SF | ZF | CF;
	if ((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */
	HL++; DE++; BC--;
	if(BC) F |= VF;
}

/***************************************************************
 * CPI
 ***************************************************************/
inline void z80_device::CPI()
{
	UINT8 val = RM(HL);
	UINT8 res = A - val;
	WZ++;
	HL++; BC--;
	F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;
	if (F & HF) res -= 1;
	if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
	if (BC) F |= VF;
}

/***************************************************************
 * INI
 ***************************************************************/
inline void z80_device::INI()
{
	unsigned t;
	UINT8 io = IN(BC);
	WZ = BC + 1;
	B--;
	WM(HL, io);
	HL++;
	F = SZ[B];
	t = (unsigned)((C + 1) & 0xff) + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * OUTI
 ***************************************************************/
inline void z80_device::OUTI()
{
	unsigned t;
	UINT8 io = RM(HL);
	B--;
	WZ = BC + 1;
	OUT(BC, io);
	HL++;
	F = SZ[B];
	t = (unsigned)L + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * LDD
 ***************************************************************/
inline void z80_device::LDD()
{
	UINT8 io = RM(HL);
	WM(DE, io);
	F &= SF | ZF | CF;
	if ((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */
	HL--; DE--; BC--;
	if (BC) F |= VF;
}

/***************************************************************
 * CPD
 ***************************************************************/
inline void z80_device::CPD()
{
	UINT8 val = RM(HL);
	UINT8 res = A - val;
	WZ--;
	HL--; BC--;
	F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^val^res)&HF) | NF;
	if (F & HF) res -= 1;
	if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
	if (BC) F |= VF;
}

/***************************************************************
 * IND
 ***************************************************************/
inline void z80_device::IND()
{
	unsigned t;
	UINT8 io = IN(BC);
	WZ = BC - 1;
	B--;
	WM(HL, io);
	HL--;
	F = SZ[B];
	t = ((unsigned)(C - 1) & 0xff) + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * OUTD
 ***************************************************************/
inline void z80_device::OUTD()
{
	unsigned t;
	UINT8 io = RM(HL);
	B--;
	WZ = BC - 1;
	OUT(BC, io);
	HL--;
	F = SZ[B];
	t = (unsigned)L + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(UINT8)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * LDIR
 ***************************************************************/
inline void z80_device::LDIR()
{
	LDI();
	if (BC != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		CC(ex, 0xb0);
	}
}

/***************************************************************
 * CPIR
 ***************************************************************/
inline void z80_device::CPIR()
{
	CPI();
	if (BC != 0 && !(F & ZF))
	{
		PC -= 2;
		WZ = PC + 1;
		CC(ex, 0xb1);
	}
}

/***************************************************************
 * INIR
 ***************************************************************/
inline void z80_device::INIR()
{
	INI();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xb2);
	}
}

/***************************************************************
 * OTIR
 ***************************************************************/
inline void z80_device::OTIR()
{
	OUTI();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xb3);
	}
}

/***************************************************************
 * LDDR
 ***************************************************************/
inline void z80_device::LDDR()
{
	LDD();
	if (BC != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		CC(ex, 0xb8);
	}
}

/***************************************************************
 * CPDR
 ***************************************************************/
inline void z80_device::CPDR()
{
	CPD();
	if (BC != 0 && !(F & ZF))
	{
		PC -= 2;
		WZ = PC + 1;
		CC(ex, 0xb9);
	}
}

/***************************************************************
 * INDR
 ***************************************************************/
inline void z80_device::INDR()
{
	IND();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xba);
	}
}

/***************************************************************
 * OTDR
 ***************************************************************/
inline void z80_device::OTDR()
{
	OUTD();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xbb);
	}
}

/***************************************************************
 * EI
 ***************************************************************/
inline void z80_device::EI()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = TRUE;
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { B = RLC(B);             } /* RLC  B           */
OP(cb,01) { C = RLC(C);             } /* RLC  C           */
OP(cb,02) { D = RLC(D);             } /* RLC  D           */
OP(cb,03) { E = RLC(E);             } /* RLC  E           */
OP(cb,04) { H = RLC(H);             } /* RLC  H           */
OP(cb,05) { L = RLC(L);             } /* RLC  L           */
OP(cb,06) { WM(HL, RLC(RM(HL)));    } /* RLC  (HL)        */
OP(cb,07) { A = RLC(A);             } /* RLC  A           */

OP(cb,08) { B = RRC(B);             } /* RRC  B           */
OP(cb,09) { C = RRC(C);             } /* RRC  C           */
OP(cb,0a) { D = RRC(D);             } /* RRC  D           */
OP(cb,0b) { E = RRC(E);             } /* RRC  E           */
OP(cb,0c) { H = RRC(H);             } /* RRC  H           */
OP(cb,0d) { L = RRC(L);             } /* RRC  L           */
OP(cb,0e) { WM(HL, RRC(RM(HL)));    } /* RRC  (HL)        */
OP(cb,0f) { A = RRC(A);             } /* RRC  A           */

OP(cb,10) { B = RL(B);              } /* RL   B           */
OP(cb,11) { C = RL(C);              } /* RL   C           */
OP(cb,12) { D = RL(D);              } /* RL   D           */
OP(cb,13) { E = RL(E);              } /* RL   E           */
OP(cb,14) { H = RL(H);              } /* RL   H           */
OP(cb,15) { L = RL(L);              } /* RL   L           */
OP(cb,16) { WM(HL, RL(RM(HL)));     } /* RL   (HL)        */
OP(cb,17) { A = RL(A);              } /* RL   A           */

OP(cb,18) { B = RR(B);              } /* RR   B           */
OP(cb,19) { C = RR(C);              } /* RR   C           */
OP(cb,1a) { D = RR(D);              } /* RR   D           */
OP(cb,1b) { E = RR(E);              } /* RR   E           */
OP(cb,1c) { H = RR(H);              } /* RR   H           */
OP(cb,1d) { L = RR(L);              } /* RR   L           */
OP(cb,1e) { WM(HL, RR(RM(HL)));     } /* RR   (HL)        */
OP(cb,1f) { A = RR(A);              } /* RR   A           */

OP(cb,20) { B = SLA(B);             } /* SLA  B           */
OP(cb,21) { C = SLA(C);             } /* SLA  C           */
OP(cb,22) { D = SLA(D);             } /* SLA  D           */
OP(cb,23) { E = SLA(E);             } /* SLA  E           */
OP(cb,24) { H = SLA(H);             } /* SLA  H           */
OP(cb,25) { L = SLA(L);             } /* SLA  L           */
OP(cb,26) { WM(HL, SLA(RM(HL)));    } /* SLA  (HL)        */
OP(cb,27) { A = SLA(A);             } /* SLA  A           */

OP(cb,28) { B = SRA(B);             } /* SRA  B           */
OP(cb,29) { C = SRA(C);             } /* SRA  C           */
OP(cb,2a) { D = SRA(D);             } /* SRA  D           */
OP(cb,2b) { E = SRA(E);             } /* SRA  E           */
OP(cb,2c) { H = SRA(H);             } /* SRA  H           */
OP(cb,2d) { L = SRA(L);             } /* SRA  L           */
OP(cb,2e) { WM(HL, SRA(RM(HL)));    } /* SRA  (HL)        */
OP(cb,2f) { A = SRA(A);             } /* SRA  A           */

OP(cb,30) { B = SLL(B);             } /* SLL  B           */
OP(cb,31) { C = SLL(C);             } /* SLL  C           */
OP(cb,32) { D = SLL(D);             } /* SLL  D           */
OP(cb,33) { E = SLL(E);             } /* SLL  E           */
OP(cb,34) { H = SLL(H);             } /* SLL  H           */
OP(cb,35) { L = SLL(L);             } /* SLL  L           */
OP(cb,36) { WM(HL, SLL(RM(HL)));    } /* SLL  (HL)        */
OP(cb,37) { A = SLL(A);             } /* SLL  A           */

OP(cb,38) { B = SRL(B);             } /* SRL  B           */
OP(cb,39) { C = SRL(C);             } /* SRL  C           */
OP(cb,3a) { D = SRL(D);             } /* SRL  D           */
OP(cb,3b) { E = SRL(E);             } /* SRL  E           */
OP(cb,3c) { H = SRL(H);             } /* SRL  H           */
OP(cb,3d) { L = SRL(L);             } /* SRL  L           */
OP(cb,3e) { WM(HL, SRL(RM(HL)));    } /* SRL  (HL)        */
OP(cb,3f) { A = SRL(A);             } /* SRL  A           */

OP(cb,40) { BIT(0, B);              } /* BIT  0,B         */
OP(cb,41) { BIT(0, C);              } /* BIT  0,C         */
OP(cb,42) { BIT(0, D);              } /* BIT  0,D         */
OP(cb,43) { BIT(0, E);              } /* BIT  0,E         */
OP(cb,44) { BIT(0, H);              } /* BIT  0,H         */
OP(cb,45) { BIT(0, L);              } /* BIT  0,L         */
OP(cb,46) { BIT_HL(0, RM(HL));      } /* BIT  0,(HL)      */
OP(cb,47) { BIT(0, A);              } /* BIT  0,A         */

OP(cb,48) { BIT(1, B);              } /* BIT  1,B         */
OP(cb,49) { BIT(1, C);              } /* BIT  1,C         */
OP(cb,4a) { BIT(1, D);              } /* BIT  1,D         */
OP(cb,4b) { BIT(1, E);              } /* BIT  1,E         */
OP(cb,4c) { BIT(1, H);              } /* BIT  1,H         */
OP(cb,4d) { BIT(1, L);              } /* BIT  1,L         */
OP(cb,4e) { BIT_HL(1, RM(HL));      } /* BIT  1,(HL)      */
OP(cb,4f) { BIT(1, A);              } /* BIT  1,A         */

OP(cb,50) { BIT(2, B);              } /* BIT  2,B         */
OP(cb,51) { BIT(2, C);              } /* BIT  2,C         */
OP(cb,52) { BIT(2, D);              } /* BIT  2,D         */
OP(cb,53) { BIT(2, E);              } /* BIT  2,E         */
OP(cb,54) { BIT(2, H);              } /* BIT  2,H         */
OP(cb,55) { BIT(2, L);              } /* BIT  2,L         */
OP(cb,56) { BIT_HL(2, RM(HL));      } /* BIT  2,(HL)      */
OP(cb,57) { BIT(2, A);              } /* BIT  2,A         */

OP(cb,58) { BIT(3, B);              } /* BIT  3,B         */
OP(cb,59) { BIT(3, C);              } /* BIT  3,C         */
OP(cb,5a) { BIT(3, D);              } /* BIT  3,D         */
OP(cb,5b) { BIT(3, E);              } /* BIT  3,E         */
OP(cb,5c) { BIT(3, H);              } /* BIT  3,H         */
OP(cb,5d) { BIT(3, L);              } /* BIT  3,L         */
OP(cb,5e) { BIT_HL(3, RM(HL));      } /* BIT  3,(HL)      */
OP(cb,5f) { BIT(3, A);              } /* BIT  3,A         */

OP(cb,60) { BIT(4, B);              } /* BIT  4,B         */
OP(cb,61) { BIT(4, C);              } /* BIT  4,C         */
OP(cb,62) { BIT(4, D);              } /* BIT  4,D         */
OP(cb,63) { BIT(4, E);              } /* BIT  4,E         */
OP(cb,64) { BIT(4, H);              } /* BIT  4,H         */
OP(cb,65) { BIT(4, L);              } /* BIT  4,L         */
OP(cb,66) { BIT_HL(4, RM(HL));      } /* BIT  4,(HL)      */
OP(cb,67) { BIT(4, A);              } /* BIT  4,A         */

OP(cb,68) { BIT(5, B);              } /* BIT  5,B         */
OP(cb,69) { BIT(5, C);              } /* BIT  5,C         */
OP(cb,6a) { BIT(5, D);              } /* BIT  5,D         */
OP(cb,6b) { BIT(5, E);              } /* BIT  5,E         */
OP(cb,6c) { BIT(5, H);              } /* BIT  5,H         */
OP(cb,6d) { BIT(5, L);              } /* BIT  5,L         */
OP(cb,6e) { BIT_HL(5, RM(HL));      } /* BIT  5,(HL)      */
OP(cb,6f) { BIT(5, A);              } /* BIT  5,A         */

OP(cb,70) { BIT(6, B);              } /* BIT  6,B         */
OP(cb,71) { BIT(6, C);              } /* BIT  6,C         */
OP(cb,72) { BIT(6, D);              } /* BIT  6,D         */
OP(cb,73) { BIT(6, E);              } /* BIT  6,E         */
OP(cb,74) { BIT(6, H);              } /* BIT  6,H         */
OP(cb,75) { BIT(6, L);              } /* BIT  6,L         */
OP(cb,76) { BIT_HL(6, RM(HL));      } /* BIT  6,(HL)      */
OP(cb,77) { BIT(6, A);              } /* BIT  6,A         */

OP(cb,78) { BIT(7, B);              } /* BIT  7,B         */
OP(cb,79) { BIT(7, C);              } /* BIT  7,C         */
OP(cb,7a) { BIT(7, D);              } /* BIT  7,D         */
OP(cb,7b) { BIT(7, E);              } /* BIT  7,E         */
OP(cb,7c) { BIT(7, H);              } /* BIT  7,H         */
OP(cb,7d) { BIT(7, L);              } /* BIT  7,L         */
OP(cb,7e) { BIT_HL(7, RM(HL));      } /* BIT  7,(HL)      */
OP(cb,7f) { BIT(7, A);              } /* BIT  7,A         */

OP(cb,80) { B = RES(0, B);          } /* RES  0,B         */
OP(cb,81) { C = RES(0, C);          } /* RES  0,C         */
OP(cb,82) { D = RES(0, D);          } /* RES  0,D         */
OP(cb,83) { E = RES(0, E);          } /* RES  0,E         */
OP(cb,84) { H = RES(0, H);          } /* RES  0,H         */
OP(cb,85) { L = RES(0, L);          } /* RES  0,L         */
OP(cb,86) { WM(HL, RES(0, RM(HL))); } /* RES  0,(HL)      */
OP(cb,87) { A = RES(0, A);          } /* RES  0,A         */

OP(cb,88) { B = RES(1, B);          } /* RES  1,B         */
OP(cb,89) { C = RES(1, C);          } /* RES  1,C         */
OP(cb,8a) { D = RES(1, D);          } /* RES  1,D         */
OP(cb,8b) { E = RES(1, E);          } /* RES  1,E         */
OP(cb,8c) { H = RES(1, H);          } /* RES  1,H         */
OP(cb,8d) { L = RES(1, L);          } /* RES  1,L         */
OP(cb,8e) { WM(HL, RES(1, RM(HL))); } /* RES  1,(HL)      */
OP(cb,8f) { A = RES(1, A);          } /* RES  1,A         */

OP(cb,90) { B = RES(2, B);          } /* RES  2,B         */
OP(cb,91) { C = RES(2, C);          } /* RES  2,C         */
OP(cb,92) { D = RES(2, D);          } /* RES  2,D         */
OP(cb,93) { E = RES(2, E);          } /* RES  2,E         */
OP(cb,94) { H = RES(2, H);          } /* RES  2,H         */
OP(cb,95) { L = RES(2, L);          } /* RES  2,L         */
OP(cb,96) { WM(HL, RES(2, RM(HL))); } /* RES  2,(HL)      */
OP(cb,97) { A = RES(2, A);          } /* RES  2,A         */

OP(cb,98) { B = RES(3, B);          } /* RES  3,B         */
OP(cb,99) { C = RES(3, C);          } /* RES  3,C         */
OP(cb,9a) { D = RES(3, D);          } /* RES  3,D         */
OP(cb,9b) { E = RES(3, E);          } /* RES  3,E         */
OP(cb,9c) { H = RES(3, H);          } /* RES  3,H         */
OP(cb,9d) { L = RES(3, L);          } /* RES  3,L         */
OP(cb,9e) { WM(HL, RES(3, RM(HL))); } /* RES  3,(HL)      */
OP(cb,9f) { A = RES(3, A);          } /* RES  3,A         */

OP(cb,a0) { B = RES(4, B);          } /* RES  4,B         */
OP(cb,a1) { C = RES(4, C);          } /* RES  4,C         */
OP(cb,a2) { D = RES(4, D);          } /* RES  4,D         */
OP(cb,a3) { E = RES(4, E);          } /* RES  4,E         */
OP(cb,a4) { H = RES(4, H);          } /* RES  4,H         */
OP(cb,a5) { L = RES(4, L);          } /* RES  4,L         */
OP(cb,a6) { WM(HL, RES(4, RM(HL))); } /* RES  4,(HL)      */
OP(cb,a7) { A = RES(4, A);          } /* RES  4,A         */

OP(cb,a8) { B = RES(5, B);          } /* RES  5,B         */
OP(cb,a9) { C = RES(5, C);          } /* RES  5,C         */
OP(cb,aa) { D = RES(5, D);          } /* RES  5,D         */
OP(cb,ab) { E = RES(5, E);          } /* RES  5,E         */
OP(cb,ac) { H = RES(5, H);          } /* RES  5,H         */
OP(cb,ad) { L = RES(5, L);          } /* RES  5,L         */
OP(cb,ae) { WM(HL, RES(5, RM(HL))); } /* RES  5,(HL)      */
OP(cb,af) { A = RES(5, A);          } /* RES  5,A         */

OP(cb,b0) { B = RES(6, B);          } /* RES  6,B         */
OP(cb,b1) { C = RES(6, C);          } /* RES  6,C         */
OP(cb,b2) { D = RES(6, D);          } /* RES  6,D         */
OP(cb,b3) { E = RES(6, E);          } /* RES  6,E         */
OP(cb,b4) { H = RES(6, H);          } /* RES  6,H         */
OP(cb,b5) { L = RES(6, L);          } /* RES  6,L         */
OP(cb,b6) { WM(HL, RES(6, RM(HL))); } /* RES  6,(HL)      */
OP(cb,b7) { A = RES(6, A);          } /* RES  6,A         */

OP(cb,b8) { B = RES(7, B);          } /* RES  7,B         */
OP(cb,b9) { C = RES(7, C);          } /* RES  7,C         */
OP(cb,ba) { D = RES(7, D);          } /* RES  7,D         */
OP(cb,bb) { E = RES(7, E);          } /* RES  7,E         */
OP(cb,bc) { H = RES(7, H);          } /* RES  7,H         */
OP(cb,bd) { L = RES(7, L);          } /* RES  7,L         */
OP(cb,be) { WM(HL, RES(7, RM(HL))); } /* RES  7,(HL)      */
OP(cb,bf) { A = RES(7, A);          } /* RES  7,A         */

OP(cb,c0) { B = SET(0, B);          } /* SET  0,B         */
OP(cb,c1) { C = SET(0, C);          } /* SET  0,C         */
OP(cb,c2) { D = SET(0, D);          } /* SET  0,D         */
OP(cb,c3) { E = SET(0, E);          } /* SET  0,E         */
OP(cb,c4) { H = SET(0, H);          } /* SET  0,H         */
OP(cb,c5) { L = SET(0, L);          } /* SET  0,L         */
OP(cb,c6) { WM(HL, SET(0, RM(HL))); } /* SET  0,(HL)      */
OP(cb,c7) { A = SET(0, A);          } /* SET  0,A         */

OP(cb,c8) { B = SET(1, B);          } /* SET  1,B         */
OP(cb,c9) { C = SET(1, C);          } /* SET  1,C         */
OP(cb,ca) { D = SET(1, D);          } /* SET  1,D         */
OP(cb,cb) { E = SET(1, E);          } /* SET  1,E         */
OP(cb,cc) { H = SET(1, H);          } /* SET  1,H         */
OP(cb,cd) { L = SET(1, L);          } /* SET  1,L         */
OP(cb,ce) { WM(HL, SET(1, RM(HL))); } /* SET  1,(HL)      */
OP(cb,cf) { A = SET(1, A);          } /* SET  1,A         */

OP(cb,d0) { B = SET(2, B);          } /* SET  2,B         */
OP(cb,d1) { C = SET(2, C);          } /* SET  2,C         */
OP(cb,d2) { D = SET(2, D);          } /* SET  2,D         */
OP(cb,d3) { E = SET(2, E);          } /* SET  2,E         */
OP(cb,d4) { H = SET(2, H);          } /* SET  2,H         */
OP(cb,d5) { L = SET(2, L);          } /* SET  2,L         */
OP(cb,d6) { WM(HL, SET(2, RM(HL))); } /* SET  2,(HL)      */
OP(cb,d7) { A = SET(2, A);          } /* SET  2,A         */

OP(cb,d8) { B = SET(3, B);          } /* SET  3,B         */
OP(cb,d9) { C = SET(3, C);          } /* SET  3,C         */
OP(cb,da) { D = SET(3, D);          } /* SET  3,D         */
OP(cb,db) { E = SET(3, E);          } /* SET  3,E         */
OP(cb,dc) { H = SET(3, H);          } /* SET  3,H         */
OP(cb,dd) { L = SET(3, L);          } /* SET  3,L         */
OP(cb,de) { WM(HL, SET(3, RM(HL))); } /* SET  3,(HL)      */
OP(cb,df) { A = SET(3, A);          } /* SET  3,A         */

OP(cb,e0) { B = SET(4, B);          } /* SET  4,B         */
OP(cb,e1) { C = SET(4, C);          } /* SET  4,C         */
OP(cb,e2) { D = SET(4, D);          } /* SET  4,D         */
OP(cb,e3) { E = SET(4, E);          } /* SET  4,E         */
OP(cb,e4) { H = SET(4, H);          } /* SET  4,H         */
OP(cb,e5) { L = SET(4, L);          } /* SET  4,L         */
OP(cb,e6) { WM(HL, SET(4, RM(HL))); } /* SET  4,(HL)      */
OP(cb,e7) { A = SET(4, A);          } /* SET  4,A         */

OP(cb,e8) { B = SET(5, B);          } /* SET  5,B         */
OP(cb,e9) { C = SET(5, C);          } /* SET  5,C         */
OP(cb,ea) { D = SET(5, D);          } /* SET  5,D         */
OP(cb,eb) { E = SET(5, E);          } /* SET  5,E         */
OP(cb,ec) { H = SET(5, H);          } /* SET  5,H         */
OP(cb,ed) { L = SET(5, L);          } /* SET  5,L         */
OP(cb,ee) { WM(HL, SET(5, RM(HL))); } /* SET  5,(HL)      */
OP(cb,ef) { A = SET(5, A);          } /* SET  5,A         */

OP(cb,f0) { B = SET(6, B);          } /* SET  6,B         */
OP(cb,f1) { C = SET(6, C);          } /* SET  6,C         */
OP(cb,f2) { D = SET(6, D);          } /* SET  6,D         */
OP(cb,f3) { E = SET(6, E);          } /* SET  6,E         */
OP(cb,f4) { H = SET(6, H);          } /* SET  6,H         */
OP(cb,f5) { L = SET(6, L);          } /* SET  6,L         */
OP(cb,f6) { WM(HL, SET(6, RM(HL))); } /* SET  6,(HL)      */
OP(cb,f7) { A = SET(6, A);          } /* SET  6,A         */

OP(cb,f8) { B = SET(7, B);          } /* SET  7,B         */
OP(cb,f9) { C = SET(7, C);          } /* SET  7,C         */
OP(cb,fa) { D = SET(7, D);          } /* SET  7,D         */
OP(cb,fb) { E = SET(7, E);          } /* SET  7,E         */
OP(cb,fc) { H = SET(7, H);          } /* SET  7,H         */
OP(cb,fd) { L = SET(7, L);          } /* SET  7,L         */
OP(cb,fe) { WM(HL, SET(7, RM(HL))); } /* SET  7,(HL)      */
OP(cb,ff) { A = SET(7, A);          } /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { B = RLC(RM(m_ea)); WM(m_ea, B);    } /* RLC  B=(XY+o)    */
OP(xycb,01) { C = RLC(RM(m_ea)); WM(m_ea, C);    } /* RLC  C=(XY+o)    */
OP(xycb,02) { D = RLC(RM(m_ea)); WM(m_ea, D);    } /* RLC  D=(XY+o)    */
OP(xycb,03) { E = RLC(RM(m_ea)); WM(m_ea, E);    } /* RLC  E=(XY+o)    */
OP(xycb,04) { H = RLC(RM(m_ea)); WM(m_ea, H);    } /* RLC  H=(XY+o)    */
OP(xycb,05) { L = RLC(RM(m_ea)); WM(m_ea, L);    } /* RLC  L=(XY+o)    */
OP(xycb,06) { WM(m_ea, RLC(RM(m_ea)));           } /* RLC  (XY+o)      */
OP(xycb,07) { A = RLC(RM(m_ea)); WM(m_ea, A);    } /* RLC  A=(XY+o)    */

OP(xycb,08) { B = RRC(RM(m_ea)); WM(m_ea, B);    } /* RRC  B=(XY+o)    */
OP(xycb,09) { C = RRC(RM(m_ea)); WM(m_ea, C);    } /* RRC  C=(XY+o)    */
OP(xycb,0a) { D = RRC(RM(m_ea)); WM(m_ea, D);    } /* RRC  D=(XY+o)    */
OP(xycb,0b) { E = RRC(RM(m_ea)); WM(m_ea, E);    } /* RRC  E=(XY+o)    */
OP(xycb,0c) { H = RRC(RM(m_ea)); WM(m_ea, H);    } /* RRC  H=(XY+o)    */
OP(xycb,0d) { L = RRC(RM(m_ea)); WM(m_ea, L);    } /* RRC  L=(XY+o)    */
OP(xycb,0e) { WM(m_ea,RRC(RM(m_ea)));            } /* RRC  (XY+o)      */
OP(xycb,0f) { A = RRC(RM(m_ea)); WM(m_ea, A);    } /* RRC  A=(XY+o)    */

OP(xycb,10) { B = RL(RM(m_ea)); WM(m_ea, B);     } /* RL   B=(XY+o)    */
OP(xycb,11) { C = RL(RM(m_ea)); WM(m_ea, C);     } /* RL   C=(XY+o)    */
OP(xycb,12) { D = RL(RM(m_ea)); WM(m_ea, D);     } /* RL   D=(XY+o)    */
OP(xycb,13) { E = RL(RM(m_ea)); WM(m_ea, E);     } /* RL   E=(XY+o)    */
OP(xycb,14) { H = RL(RM(m_ea)); WM(m_ea, H);     } /* RL   H=(XY+o)    */
OP(xycb,15) { L = RL(RM(m_ea)); WM(m_ea, L);     } /* RL   L=(XY+o)    */
OP(xycb,16) { WM(m_ea,RL(RM(m_ea)));             } /* RL   (XY+o)      */
OP(xycb,17) { A = RL(RM(m_ea)); WM(m_ea, A);     } /* RL   A=(XY+o)    */

OP(xycb,18) { B = RR(RM(m_ea)); WM(m_ea, B);     } /* RR   B=(XY+o)    */
OP(xycb,19) { C = RR(RM(m_ea)); WM(m_ea, C);     } /* RR   C=(XY+o)    */
OP(xycb,1a) { D = RR(RM(m_ea)); WM(m_ea, D);     } /* RR   D=(XY+o)    */
OP(xycb,1b) { E = RR(RM(m_ea)); WM(m_ea, E);     } /* RR   E=(XY+o)    */
OP(xycb,1c) { H = RR(RM(m_ea)); WM(m_ea, H);     } /* RR   H=(XY+o)    */
OP(xycb,1d) { L = RR(RM(m_ea)); WM(m_ea, L);     } /* RR   L=(XY+o)    */
OP(xycb,1e) { WM(m_ea, RR(RM(m_ea)));            } /* RR   (XY+o)      */
OP(xycb,1f) { A = RR(RM(m_ea)); WM(m_ea, A);     } /* RR   A=(XY+o)    */

OP(xycb,20) { B = SLA(RM(m_ea)); WM(m_ea, B);    } /* SLA  B=(XY+o)    */
OP(xycb,21) { C = SLA(RM(m_ea)); WM(m_ea, C);    } /* SLA  C=(XY+o)    */
OP(xycb,22) { D = SLA(RM(m_ea)); WM(m_ea, D);    } /* SLA  D=(XY+o)    */
OP(xycb,23) { E = SLA(RM(m_ea)); WM(m_ea, E);    } /* SLA  E=(XY+o)    */
OP(xycb,24) { H = SLA(RM(m_ea)); WM(m_ea, H);    } /* SLA  H=(XY+o)    */
OP(xycb,25) { L = SLA(RM(m_ea)); WM(m_ea, L);    } /* SLA  L=(XY+o)    */
OP(xycb,26) { WM(m_ea, SLA(RM(m_ea)));           } /* SLA  (XY+o)      */
OP(xycb,27) { A = SLA(RM(m_ea)); WM(m_ea, A);    } /* SLA  A=(XY+o)    */

OP(xycb,28) { B = SRA(RM(m_ea)); WM(m_ea, B);    } /* SRA  B=(XY+o)    */
OP(xycb,29) { C = SRA(RM(m_ea)); WM(m_ea, C);    } /* SRA  C=(XY+o)    */
OP(xycb,2a) { D = SRA(RM(m_ea)); WM(m_ea, D);    } /* SRA  D=(XY+o)    */
OP(xycb,2b) { E = SRA(RM(m_ea)); WM(m_ea, E);    } /* SRA  E=(XY+o)    */
OP(xycb,2c) { H = SRA(RM(m_ea)); WM(m_ea, H);    } /* SRA  H=(XY+o)    */
OP(xycb,2d) { L = SRA(RM(m_ea)); WM(m_ea, L);    } /* SRA  L=(XY+o)    */
OP(xycb,2e) { WM(m_ea, SRA(RM(m_ea)));           } /* SRA  (XY+o)      */
OP(xycb,2f) { A = SRA(RM(m_ea)); WM(m_ea, A);    } /* SRA  A=(XY+o)    */

OP(xycb,30) { B = SLL(RM(m_ea)); WM(m_ea, B);    } /* SLL  B=(XY+o)    */
OP(xycb,31) { C = SLL(RM(m_ea)); WM(m_ea, C);    } /* SLL  C=(XY+o)    */
OP(xycb,32) { D = SLL(RM(m_ea)); WM(m_ea, D);    } /* SLL  D=(XY+o)    */
OP(xycb,33) { E = SLL(RM(m_ea)); WM(m_ea, E);    } /* SLL  E=(XY+o)    */
OP(xycb,34) { H = SLL(RM(m_ea)); WM(m_ea, H);    } /* SLL  H=(XY+o)    */
OP(xycb,35) { L = SLL(RM(m_ea)); WM(m_ea, L);    } /* SLL  L=(XY+o)    */
OP(xycb,36) { WM(m_ea, SLL(RM(m_ea)));           } /* SLL  (XY+o)      */
OP(xycb,37) { A = SLL(RM(m_ea)); WM(m_ea, A);    } /* SLL  A=(XY+o)    */

OP(xycb,38) { B = SRL(RM(m_ea)); WM(m_ea, B);    } /* SRL  B=(XY+o)    */
OP(xycb,39) { C = SRL(RM(m_ea)); WM(m_ea, C);    } /* SRL  C=(XY+o)    */
OP(xycb,3a) { D = SRL(RM(m_ea)); WM(m_ea, D);    } /* SRL  D=(XY+o)    */
OP(xycb,3b) { E = SRL(RM(m_ea)); WM(m_ea, E);    } /* SRL  E=(XY+o)    */
OP(xycb,3c) { H = SRL(RM(m_ea)); WM(m_ea, H);    } /* SRL  H=(XY+o)    */
OP(xycb,3d) { L = SRL(RM(m_ea)); WM(m_ea, L);    } /* SRL  L=(XY+o)    */
OP(xycb,3e) { WM(m_ea, SRL(RM(m_ea)));           } /* SRL  (XY+o)      */
OP(xycb,3f) { A = SRL(RM(m_ea)); WM(m_ea, A);    } /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,41) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,42) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,43) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,44) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,45) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,46) { BIT_XY(0, RM(m_ea));               } /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46();                         } /* BIT  0,(XY+o)    */

OP(xycb,48) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,49) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4a) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4b) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4c) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4d) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4e) { BIT_XY(1, RM(m_ea));               } /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e();                         } /* BIT  1,(XY+o)    */

OP(xycb,50) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,51) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,52) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,53) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,54) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,55) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,56) { BIT_XY(2, RM(m_ea));               } /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56();                         } /* BIT  2,(XY+o)    */

OP(xycb,58) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,59) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5a) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5b) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5c) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5d) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5e) { BIT_XY(3, RM(m_ea));               } /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e();                         } /* BIT  3,(XY+o)    */

OP(xycb,60) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,61) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,62) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,63) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,64) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,65) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,66) { BIT_XY(4, RM(m_ea));               } /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66();                         } /* BIT  4,(XY+o)    */

OP(xycb,68) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,69) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6a) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6b) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6c) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6d) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6e) { BIT_XY(5, RM(m_ea));               } /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e();                         } /* BIT  5,(XY+o)    */

OP(xycb,70) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,71) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,72) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,73) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,74) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,75) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,76) { BIT_XY(6, RM(m_ea));               } /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76();                         } /* BIT  6,(XY+o)    */

OP(xycb,78) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,79) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7a) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7b) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7c) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7d) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7e) { BIT_XY(7, RM(m_ea));               } /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e();                         } /* BIT  7,(XY+o)    */

OP(xycb,80) { B = RES(0, RM(m_ea)); WM(m_ea, B); } /* RES  0,B=(XY+o)  */
OP(xycb,81) { C = RES(0, RM(m_ea)); WM(m_ea, C); } /* RES  0,C=(XY+o)  */
OP(xycb,82) { D = RES(0, RM(m_ea)); WM(m_ea, D); } /* RES  0,D=(XY+o)  */
OP(xycb,83) { E = RES(0, RM(m_ea)); WM(m_ea, E); } /* RES  0,E=(XY+o)  */
OP(xycb,84) { H = RES(0, RM(m_ea)); WM(m_ea, H); } /* RES  0,H=(XY+o)  */
OP(xycb,85) { L = RES(0, RM(m_ea)); WM(m_ea, L); } /* RES  0,L=(XY+o)  */
OP(xycb,86) { WM(m_ea, RES(0, RM(m_ea)));        } /* RES  0,(XY+o)    */
OP(xycb,87) { A = RES(0, RM(m_ea)); WM(m_ea, A); } /* RES  0,A=(XY+o)  */

OP(xycb,88) { B = RES(1, RM(m_ea)); WM(m_ea, B); } /* RES  1,B=(XY+o)  */
OP(xycb,89) { C = RES(1, RM(m_ea)); WM(m_ea, C); } /* RES  1,C=(XY+o)  */
OP(xycb,8a) { D = RES(1, RM(m_ea)); WM(m_ea, D); } /* RES  1,D=(XY+o)  */
OP(xycb,8b) { E = RES(1, RM(m_ea)); WM(m_ea, E); } /* RES  1,E=(XY+o)  */
OP(xycb,8c) { H = RES(1, RM(m_ea)); WM(m_ea, H); } /* RES  1,H=(XY+o)  */
OP(xycb,8d) { L = RES(1, RM(m_ea)); WM(m_ea, L); } /* RES  1,L=(XY+o)  */
OP(xycb,8e) { WM(m_ea, RES(1, RM(m_ea)));        } /* RES  1,(XY+o)    */
OP(xycb,8f) { A = RES(1, RM(m_ea)); WM(m_ea, A); } /* RES  1,A=(XY+o)  */

OP(xycb,90) { B = RES(2, RM(m_ea)); WM(m_ea, B); } /* RES  2,B=(XY+o)  */
OP(xycb,91) { C = RES(2, RM(m_ea)); WM(m_ea, C); } /* RES  2,C=(XY+o)  */
OP(xycb,92) { D = RES(2, RM(m_ea)); WM(m_ea, D); } /* RES  2,D=(XY+o)  */
OP(xycb,93) { E = RES(2, RM(m_ea)); WM(m_ea, E); } /* RES  2,E=(XY+o)  */
OP(xycb,94) { H = RES(2, RM(m_ea)); WM(m_ea, H); } /* RES  2,H=(XY+o)  */
OP(xycb,95) { L = RES(2, RM(m_ea)); WM(m_ea, L); } /* RES  2,L=(XY+o)  */
OP(xycb,96) { WM(m_ea, RES(2, RM(m_ea)));        } /* RES  2,(XY+o)    */
OP(xycb,97) { A = RES(2, RM(m_ea)); WM(m_ea, A); } /* RES  2,A=(XY+o)  */

OP(xycb,98) { B = RES(3, RM(m_ea)); WM(m_ea, B); } /* RES  3,B=(XY+o)  */
OP(xycb,99) { C = RES(3, RM(m_ea)); WM(m_ea, C); } /* RES  3,C=(XY+o)  */
OP(xycb,9a) { D = RES(3, RM(m_ea)); WM(m_ea, D); } /* RES  3,D=(XY+o)  */
OP(xycb,9b) { E = RES(3, RM(m_ea)); WM(m_ea, E); } /* RES  3,E=(XY+o)  */
OP(xycb,9c) { H = RES(3, RM(m_ea)); WM(m_ea, H); } /* RES  3,H=(XY+o)  */
OP(xycb,9d) { L = RES(3, RM(m_ea)); WM(m_ea, L); } /* RES  3,L=(XY+o)  */
OP(xycb,9e) { WM(m_ea, RES(3, RM(m_ea)));        } /* RES  3,(XY+o)    */
OP(xycb,9f) { A = RES(3, RM(m_ea)); WM(m_ea, A); } /* RES  3,A=(XY+o)  */

OP(xycb,a0) { B = RES(4, RM(m_ea)); WM(m_ea, B); } /* RES  4,B=(XY+o)  */
OP(xycb,a1) { C = RES(4, RM(m_ea)); WM(m_ea, C); } /* RES  4,C=(XY+o)  */
OP(xycb,a2) { D = RES(4, RM(m_ea)); WM(m_ea, D); } /* RES  4,D=(XY+o)  */
OP(xycb,a3) { E = RES(4, RM(m_ea)); WM(m_ea, E); } /* RES  4,E=(XY+o)  */
OP(xycb,a4) { H = RES(4, RM(m_ea)); WM(m_ea, H); } /* RES  4,H=(XY+o)  */
OP(xycb,a5) { L = RES(4, RM(m_ea)); WM(m_ea, L); } /* RES  4,L=(XY+o)  */
OP(xycb,a6) { WM(m_ea, RES(4, RM(m_ea)));        } /* RES  4,(XY+o)    */
OP(xycb,a7) { A = RES(4, RM(m_ea)); WM(m_ea, A); } /* RES  4,A=(XY+o)  */

OP(xycb,a8) { B = RES(5, RM(m_ea)); WM(m_ea, B); } /* RES  5,B=(XY+o)  */
OP(xycb,a9) { C = RES(5, RM(m_ea)); WM(m_ea, C); } /* RES  5,C=(XY+o)  */
OP(xycb,aa) { D = RES(5, RM(m_ea)); WM(m_ea, D); } /* RES  5,D=(XY+o)  */
OP(xycb,ab) { E = RES(5, RM(m_ea)); WM(m_ea, E); } /* RES  5,E=(XY+o)  */
OP(xycb,ac) { H = RES(5, RM(m_ea)); WM(m_ea, H); } /* RES  5,H=(XY+o)  */
OP(xycb,ad) { L = RES(5, RM(m_ea)); WM(m_ea, L); } /* RES  5,L=(XY+o)  */
OP(xycb,ae) { WM(m_ea, RES(5, RM(m_ea)));        } /* RES  5,(XY+o)    */
OP(xycb,af) { A = RES(5, RM(m_ea)); WM(m_ea, A); } /* RES  5,A=(XY+o)  */

OP(xycb,b0) { B = RES(6, RM(m_ea)); WM(m_ea, B); } /* RES  6,B=(XY+o)  */
OP(xycb,b1) { C = RES(6, RM(m_ea)); WM(m_ea, C); } /* RES  6,C=(XY+o)  */
OP(xycb,b2) { D = RES(6, RM(m_ea)); WM(m_ea, D); } /* RES  6,D=(XY+o)  */
OP(xycb,b3) { E = RES(6, RM(m_ea)); WM(m_ea, E); } /* RES  6,E=(XY+o)  */
OP(xycb,b4) { H = RES(6, RM(m_ea)); WM(m_ea, H); } /* RES  6,H=(XY+o)  */
OP(xycb,b5) { L = RES(6, RM(m_ea)); WM(m_ea, L); } /* RES  6,L=(XY+o)  */
OP(xycb,b6) { WM(m_ea, RES(6, RM(m_ea)));        } /* RES  6,(XY+o)    */
OP(xycb,b7) { A = RES(6, RM(m_ea)); WM(m_ea, A); } /* RES  6,A=(XY+o)  */

OP(xycb,b8) { B = RES(7, RM(m_ea)); WM(m_ea, B); } /* RES  7,B=(XY+o)  */
OP(xycb,b9) { C = RES(7, RM(m_ea)); WM(m_ea, C); } /* RES  7,C=(XY+o)  */
OP(xycb,ba) { D = RES(7, RM(m_ea)); WM(m_ea, D); } /* RES  7,D=(XY+o)  */
OP(xycb,bb) { E = RES(7, RM(m_ea)); WM(m_ea, E); } /* RES  7,E=(XY+o)  */
OP(xycb,bc) { H = RES(7, RM(m_ea)); WM(m_ea, H); } /* RES  7,H=(XY+o)  */
OP(xycb,bd) { L = RES(7, RM(m_ea)); WM(m_ea, L); } /* RES  7,L=(XY+o)  */
OP(xycb,be) { WM(m_ea, RES(7, RM(m_ea)));        } /* RES  7,(XY+o)    */
OP(xycb,bf) { A = RES(7, RM(m_ea)); WM(m_ea, A); } /* RES  7,A=(XY+o)  */

OP(xycb,c0) { B = SET(0, RM(m_ea)); WM(m_ea, B); } /* SET  0,B=(XY+o)  */
OP(xycb,c1) { C = SET(0, RM(m_ea)); WM(m_ea, C); } /* SET  0,C=(XY+o)  */
OP(xycb,c2) { D = SET(0, RM(m_ea)); WM(m_ea, D); } /* SET  0,D=(XY+o)  */
OP(xycb,c3) { E = SET(0, RM(m_ea)); WM(m_ea, E); } /* SET  0,E=(XY+o)  */
OP(xycb,c4) { H = SET(0, RM(m_ea)); WM(m_ea, H); } /* SET  0,H=(XY+o)  */
OP(xycb,c5) { L = SET(0, RM(m_ea)); WM(m_ea, L); } /* SET  0,L=(XY+o)  */
OP(xycb,c6) { WM(m_ea, SET(0, RM(m_ea)));        } /* SET  0,(XY+o)    */
OP(xycb,c7) { A = SET(0, RM(m_ea)); WM(m_ea, A); } /* SET  0,A=(XY+o)  */

OP(xycb,c8) { B = SET(1, RM(m_ea)); WM(m_ea, B); } /* SET  1,B=(XY+o)  */
OP(xycb,c9) { C = SET(1, RM(m_ea)); WM(m_ea, C); } /* SET  1,C=(XY+o)  */
OP(xycb,ca) { D = SET(1, RM(m_ea)); WM(m_ea, D); } /* SET  1,D=(XY+o)  */
OP(xycb,cb) { E = SET(1, RM(m_ea)); WM(m_ea, E); } /* SET  1,E=(XY+o)  */
OP(xycb,cc) { H = SET(1, RM(m_ea)); WM(m_ea, H); } /* SET  1,H=(XY+o)  */
OP(xycb,cd) { L = SET(1, RM(m_ea)); WM(m_ea, L); } /* SET  1,L=(XY+o)  */
OP(xycb,ce) { WM(m_ea, SET(1, RM(m_ea)));        } /* SET  1,(XY+o)    */
OP(xycb,cf) { A = SET(1, RM(m_ea)); WM(m_ea, A); } /* SET  1,A=(XY+o)  */

OP(xycb,d0) { B = SET(2, RM(m_ea)); WM(m_ea, B); } /* SET  2,B=(XY+o)  */
OP(xycb,d1) { C = SET(2, RM(m_ea)); WM(m_ea, C); } /* SET  2,C=(XY+o)  */
OP(xycb,d2) { D = SET(2, RM(m_ea)); WM(m_ea, D); } /* SET  2,D=(XY+o)  */
OP(xycb,d3) { E = SET(2, RM(m_ea)); WM(m_ea, E); } /* SET  2,E=(XY+o)  */
OP(xycb,d4) { H = SET(2, RM(m_ea)); WM(m_ea, H); } /* SET  2,H=(XY+o)  */
OP(xycb,d5) { L = SET(2, RM(m_ea)); WM(m_ea, L); } /* SET  2,L=(XY+o)  */
OP(xycb,d6) { WM(m_ea, SET(2, RM(m_ea)));        } /* SET  2,(XY+o)    */
OP(xycb,d7) { A = SET(2, RM(m_ea)); WM(m_ea, A); } /* SET  2,A=(XY+o)  */

OP(xycb,d8) { B = SET(3, RM(m_ea)); WM(m_ea, B); } /* SET  3,B=(XY+o)  */
OP(xycb,d9) { C = SET(3, RM(m_ea)); WM(m_ea, C); } /* SET  3,C=(XY+o)  */
OP(xycb,da) { D = SET(3, RM(m_ea)); WM(m_ea, D); } /* SET  3,D=(XY+o)  */
OP(xycb,db) { E = SET(3, RM(m_ea)); WM(m_ea, E); } /* SET  3,E=(XY+o)  */
OP(xycb,dc) { H = SET(3, RM(m_ea)); WM(m_ea, H); } /* SET  3,H=(XY+o)  */
OP(xycb,dd) { L = SET(3, RM(m_ea)); WM(m_ea, L); } /* SET  3,L=(XY+o)  */
OP(xycb,de) { WM(m_ea, SET(3, RM(m_ea)));        } /* SET  3,(XY+o)    */
OP(xycb,df) { A = SET(3, RM(m_ea)); WM(m_ea, A); } /* SET  3,A=(XY+o)  */

OP(xycb,e0) { B = SET(4, RM(m_ea)); WM(m_ea, B); } /* SET  4,B=(XY+o)  */
OP(xycb,e1) { C = SET(4, RM(m_ea)); WM(m_ea, C); } /* SET  4,C=(XY+o)  */
OP(xycb,e2) { D = SET(4, RM(m_ea)); WM(m_ea, D); } /* SET  4,D=(XY+o)  */
OP(xycb,e3) { E = SET(4, RM(m_ea)); WM(m_ea, E); } /* SET  4,E=(XY+o)  */
OP(xycb,e4) { H = SET(4, RM(m_ea)); WM(m_ea, H); } /* SET  4,H=(XY+o)  */
OP(xycb,e5) { L = SET(4, RM(m_ea)); WM(m_ea, L); } /* SET  4,L=(XY+o)  */
OP(xycb,e6) { WM(m_ea, SET(4, RM(m_ea)));        } /* SET  4,(XY+o)    */
OP(xycb,e7) { A = SET(4, RM(m_ea)); WM(m_ea, A); } /* SET  4,A=(XY+o)  */

OP(xycb,e8) { B = SET(5, RM(m_ea)); WM(m_ea, B); } /* SET  5,B=(XY+o)  */
OP(xycb,e9) { C = SET(5, RM(m_ea)); WM(m_ea, C); } /* SET  5,C=(XY+o)  */
OP(xycb,ea) { D = SET(5, RM(m_ea)); WM(m_ea, D); } /* SET  5,D=(XY+o)  */
OP(xycb,eb) { E = SET(5, RM(m_ea)); WM(m_ea, E); } /* SET  5,E=(XY+o)  */
OP(xycb,ec) { H = SET(5, RM(m_ea)); WM(m_ea, H); } /* SET  5,H=(XY+o)  */
OP(xycb,ed) { L = SET(5, RM(m_ea)); WM(m_ea, L); } /* SET  5,L=(XY+o)  */
OP(xycb,ee) { WM(m_ea, SET(5, RM(m_ea)));        } /* SET  5,(XY+o)    */
OP(xycb,ef) { A = SET(5, RM(m_ea)); WM(m_ea, A); } /* SET  5,A=(XY+o)  */

OP(xycb,f0) { B = SET(6, RM(m_ea)); WM(m_ea, B); } /* SET  6,B=(XY+o)  */
OP(xycb,f1) { C = SET(6, RM(m_ea)); WM(m_ea, C); } /* SET  6,C=(XY+o)  */
OP(xycb,f2) { D = SET(6, RM(m_ea)); WM(m_ea, D); } /* SET  6,D=(XY+o)  */
OP(xycb,f3) { E = SET(6, RM(m_ea)); WM(m_ea, E); } /* SET  6,E=(XY+o)  */
OP(xycb,f4) { H = SET(6, RM(m_ea)); WM(m_ea, H); } /* SET  6,H=(XY+o)  */
OP(xycb,f5) { L = SET(6, RM(m_ea)); WM(m_ea, L); } /* SET  6,L=(XY+o)  */
OP(xycb,f6) { WM(m_ea, SET(6, RM(m_ea)));        } /* SET  6,(XY+o)    */
OP(xycb,f7) { A = SET(6, RM(m_ea)); WM(m_ea, A); } /* SET  6,A=(XY+o)  */

OP(xycb,f8) { B = SET(7, RM(m_ea)); WM(m_ea, B); } /* SET  7,B=(XY+o)  */
OP(xycb,f9) { C = SET(7, RM(m_ea)); WM(m_ea, C); } /* SET  7,C=(XY+o)  */
OP(xycb,fa) { D = SET(7, RM(m_ea)); WM(m_ea, D); } /* SET  7,D=(XY+o)  */
OP(xycb,fb) { E = SET(7, RM(m_ea)); WM(m_ea, E); } /* SET  7,E=(XY+o)  */
OP(xycb,fc) { H = SET(7, RM(m_ea)); WM(m_ea, H); } /* SET  7,H=(XY+o)  */
OP(xycb,fd) { L = SET(7, RM(m_ea)); WM(m_ea, L); } /* SET  7,L=(XY+o)  */
OP(xycb,fe) { WM(m_ea, SET(7, RM(m_ea)));        } /* SET  7,(XY+o)    */
OP(xycb,ff) { A = SET(7, RM(m_ea)); WM(m_ea, A); } /* SET  7,A=(XY+o)  */

OP(illegal,1) {
	logerror("Z80 '%s' ill. opcode $%02x $%02x\n",
			tag(), m_direct->read_decrypted_byte((PCD-1)&0xffff), m_direct->read_decrypted_byte(PCD));
}

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OP(dd,00) { illegal_1(); op_00();                            } /* DB   DD          */
OP(dd,01) { illegal_1(); op_01();                            } /* DB   DD          */
OP(dd,02) { illegal_1(); op_02();                            } /* DB   DD          */
OP(dd,03) { illegal_1(); op_03();                            } /* DB   DD          */
OP(dd,04) { illegal_1(); op_04();                            } /* DB   DD          */
OP(dd,05) { illegal_1(); op_05();                            } /* DB   DD          */
OP(dd,06) { illegal_1(); op_06();                            } /* DB   DD          */
OP(dd,07) { illegal_1(); op_07();                            } /* DB   DD          */

OP(dd,08) { illegal_1(); op_08();                            } /* DB   DD          */
OP(dd,09) { ADD16(m_ix, m_bc);                               } /* ADD  IX,BC       */
OP(dd,0a) { illegal_1(); op_0a();                            } /* DB   DD          */
OP(dd,0b) { illegal_1(); op_0b();                            } /* DB   DD          */
OP(dd,0c) { illegal_1(); op_0c();                            } /* DB   DD          */
OP(dd,0d) { illegal_1(); op_0d();                            } /* DB   DD          */
OP(dd,0e) { illegal_1(); op_0e();                            } /* DB   DD          */
OP(dd,0f) { illegal_1(); op_0f();                            } /* DB   DD          */

OP(dd,10) { illegal_1(); op_10();                            } /* DB   DD          */
OP(dd,11) { illegal_1(); op_11();                            } /* DB   DD          */
OP(dd,12) { illegal_1(); op_12();                            } /* DB   DD          */
OP(dd,13) { illegal_1(); op_13();                            } /* DB   DD          */
OP(dd,14) { illegal_1(); op_14();                            } /* DB   DD          */
OP(dd,15) { illegal_1(); op_15();                            } /* DB   DD          */
OP(dd,16) { illegal_1(); op_16();                            } /* DB   DD          */
OP(dd,17) { illegal_1(); op_17();                            } /* DB   DD          */

OP(dd,18) { illegal_1(); op_18();                            } /* DB   DD          */
OP(dd,19) { ADD16(m_ix, m_de);                               } /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(); op_1a();                            } /* DB   DD          */
OP(dd,1b) { illegal_1(); op_1b();                            } /* DB   DD          */
OP(dd,1c) { illegal_1(); op_1c();                            } /* DB   DD          */
OP(dd,1d) { illegal_1(); op_1d();                            } /* DB   DD          */
OP(dd,1e) { illegal_1(); op_1e();                            } /* DB   DD          */
OP(dd,1f) { illegal_1(); op_1f();                            } /* DB   DD          */

OP(dd,20) { illegal_1(); op_20();                            } /* DB   DD          */
OP(dd,21) { IX = ARG16();                                    } /* LD   IX,w        */
OP(dd,22) { m_ea = ARG16(); WM16(m_ea, m_ix); WZ = m_ea + 1; } /* LD   (w),IX      */
OP(dd,23) { IX++;                                            } /* INC  IX          */
OP(dd,24) { HX = INC(HX);                                    } /* INC  HX          */
OP(dd,25) { HX = DEC(HX);                                    } /* DEC  HX          */
OP(dd,26) { HX = ARG();                                      } /* LD   HX,n        */
OP(dd,27) { illegal_1(); op_27();                            } /* DB   DD          */

OP(dd,28) { illegal_1(); op_28();                            } /* DB   DD          */
OP(dd,29) { ADD16(m_ix, m_ix);                               } /* ADD  IX,IX       */
OP(dd,2a) { m_ea = ARG16(); RM16(m_ea, m_ix); WZ = m_ea + 1; } /* LD   IX,(w)      */
OP(dd,2b) { IX--;                                            } /* DEC  IX          */
OP(dd,2c) { LX = INC(LX);                                    } /* INC  LX          */
OP(dd,2d) { LX = DEC(LX);                                    } /* DEC  LX          */
OP(dd,2e) { LX = ARG();                                      } /* LD   LX,n        */
OP(dd,2f) { illegal_1(); op_2f();                            } /* DB   DD          */

OP(dd,30) { illegal_1(); op_30();                            } /* DB   DD          */
OP(dd,31) { illegal_1(); op_31();                            } /* DB   DD          */
OP(dd,32) { illegal_1(); op_32();                            } /* DB   DD          */
OP(dd,33) { illegal_1(); op_33();                            } /* DB   DD          */
OP(dd,34) { EAX(); WM(m_ea, INC(RM(m_ea)));                  } /* INC  (IX+o)      */
OP(dd,35) { EAX(); WM(m_ea, DEC(RM(m_ea)));                  } /* DEC  (IX+o)      */
OP(dd,36) { EAX(); WM(m_ea, ARG());                          } /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(); op_37();                            } /* DB   DD          */

OP(dd,38) { illegal_1(); op_38();                            } /* DB   DD          */
OP(dd,39) { ADD16(m_ix, m_sp);                               } /* ADD  IX,SP       */
OP(dd,3a) { illegal_1(); op_3a();                            } /* DB   DD          */
OP(dd,3b) { illegal_1(); op_3b();                            } /* DB   DD          */
OP(dd,3c) { illegal_1(); op_3c();                            } /* DB   DD          */
OP(dd,3d) { illegal_1(); op_3d();                            } /* DB   DD          */
OP(dd,3e) { illegal_1(); op_3e();                            } /* DB   DD          */
OP(dd,3f) { illegal_1(); op_3f();                            } /* DB   DD          */

OP(dd,40) { illegal_1(); op_40();                            } /* DB   DD          */
OP(dd,41) { illegal_1(); op_41();                            } /* DB   DD          */
OP(dd,42) { illegal_1(); op_42();                            } /* DB   DD          */
OP(dd,43) { illegal_1(); op_43();                            } /* DB   DD          */
OP(dd,44) { B = HX;                                          } /* LD   B,HX        */
OP(dd,45) { B = LX;                                          } /* LD   B,LX        */
OP(dd,46) { EAX(); B = RM(m_ea);                             } /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(); op_47();                            } /* DB   DD          */

OP(dd,48) { illegal_1(); op_48();                            } /* DB   DD          */
OP(dd,49) { illegal_1(); op_49();                            } /* DB   DD          */
OP(dd,4a) { illegal_1(); op_4a();                            } /* DB   DD          */
OP(dd,4b) { illegal_1(); op_4b();                            } /* DB   DD          */
OP(dd,4c) { C = HX;                                          } /* LD   C,HX        */
OP(dd,4d) { C = LX;                                          } /* LD   C,LX        */
OP(dd,4e) { EAX(); C = RM(m_ea);                             } /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(); op_4f();                            } /* DB   DD          */

OP(dd,50) { illegal_1(); op_50();                            } /* DB   DD          */
OP(dd,51) { illegal_1(); op_51();                            } /* DB   DD          */
OP(dd,52) { illegal_1(); op_52();                            } /* DB   DD          */
OP(dd,53) { illegal_1(); op_53();                            } /* DB   DD          */
OP(dd,54) { D = HX;                                          } /* LD   D,HX        */
OP(dd,55) { D = LX;                                          } /* LD   D,LX        */
OP(dd,56) { EAX(); D = RM(m_ea);                             } /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(); op_57();                            } /* DB   DD          */

OP(dd,58) { illegal_1(); op_58();                            } /* DB   DD          */
OP(dd,59) { illegal_1(); op_59();                            } /* DB   DD          */
OP(dd,5a) { illegal_1(); op_5a();                            } /* DB   DD          */
OP(dd,5b) { illegal_1(); op_5b();                            } /* DB   DD          */
OP(dd,5c) { E = HX;                                          } /* LD   E,HX        */
OP(dd,5d) { E = LX;                                          } /* LD   E,LX        */
OP(dd,5e) { EAX(); E = RM(m_ea);                             } /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(); op_5f();                            } /* DB   DD          */

OP(dd,60) { HX = B;                                          } /* LD   HX,B        */
OP(dd,61) { HX = C;                                          } /* LD   HX,C        */
OP(dd,62) { HX = D;                                          } /* LD   HX,D        */
OP(dd,63) { HX = E;                                          } /* LD   HX,E        */
OP(dd,64) {                                                  } /* LD   HX,HX       */
OP(dd,65) { HX = LX;                                         } /* LD   HX,LX       */
OP(dd,66) { EAX(); H = RM(m_ea);                             } /* LD   H,(IX+o)    */
OP(dd,67) { HX = A;                                          } /* LD   HX,A        */

OP(dd,68) { LX = B;                                          } /* LD   LX,B        */
OP(dd,69) { LX = C;                                          } /* LD   LX,C        */
OP(dd,6a) { LX = D;                                          } /* LD   LX,D        */
OP(dd,6b) { LX = E;                                          } /* LD   LX,E        */
OP(dd,6c) { LX = HX;                                         } /* LD   LX,HX       */
OP(dd,6d) {                                                  } /* LD   LX,LX       */
OP(dd,6e) { EAX(); L = RM(m_ea);                             } /* LD   L,(IX+o)    */
OP(dd,6f) { LX = A;                                          } /* LD   LX,A        */

OP(dd,70) { EAX(); WM(m_ea, B);                              } /* LD   (IX+o),B    */
OP(dd,71) { EAX(); WM(m_ea, C);                              } /* LD   (IX+o),C    */
OP(dd,72) { EAX(); WM(m_ea, D);                              } /* LD   (IX+o),D    */
OP(dd,73) { EAX(); WM(m_ea, E);                              } /* LD   (IX+o),E    */
OP(dd,74) { EAX(); WM(m_ea, H);                              } /* LD   (IX+o),H    */
OP(dd,75) { EAX(); WM(m_ea, L);                              } /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(); op_76();                            } /* DB   DD          */
OP(dd,77) { EAX(); WM(m_ea, A);                              } /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(); op_78();                            } /* DB   DD          */
OP(dd,79) { illegal_1(); op_79();                            } /* DB   DD          */
OP(dd,7a) { illegal_1(); op_7a();                            } /* DB   DD          */
OP(dd,7b) { illegal_1(); op_7b();                            } /* DB   DD          */
OP(dd,7c) { A = HX;                                          } /* LD   A,HX        */
OP(dd,7d) { A = LX;                                          } /* LD   A,LX        */
OP(dd,7e) { EAX(); A = RM(m_ea);                             } /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(); op_7f();                            } /* DB   DD          */

OP(dd,80) { illegal_1(); op_80();                            } /* DB   DD          */
OP(dd,81) { illegal_1(); op_81();                            } /* DB   DD          */
OP(dd,82) { illegal_1(); op_82();                            } /* DB   DD          */
OP(dd,83) { illegal_1(); op_83();                            } /* DB   DD          */
OP(dd,84) { ADD(HX);                                         } /* ADD  A,HX        */
OP(dd,85) { ADD(LX);                                         } /* ADD  A,LX        */
OP(dd,86) { EAX(); ADD(RM(m_ea));                            } /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(); op_87();                            } /* DB   DD          */

OP(dd,88) { illegal_1(); op_88();                            } /* DB   DD          */
OP(dd,89) { illegal_1(); op_89();                            } /* DB   DD          */
OP(dd,8a) { illegal_1(); op_8a();                            } /* DB   DD          */
OP(dd,8b) { illegal_1(); op_8b();                            } /* DB   DD          */
OP(dd,8c) { ADC(HX);                                         } /* ADC  A,HX        */
OP(dd,8d) { ADC(LX);                                         } /* ADC  A,LX        */
OP(dd,8e) { EAX(); ADC(RM(m_ea));                            } /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(); op_8f();                            } /* DB   DD          */

OP(dd,90) { illegal_1(); op_90();                            } /* DB   DD          */
OP(dd,91) { illegal_1(); op_91();                            } /* DB   DD          */
OP(dd,92) { illegal_1(); op_92();                            } /* DB   DD          */
OP(dd,93) { illegal_1(); op_93();                            } /* DB   DD          */
OP(dd,94) { SUB(HX);                                         } /* SUB  HX          */
OP(dd,95) { SUB(LX);                                         } /* SUB  LX          */
OP(dd,96) { EAX(); SUB(RM(m_ea));                            } /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(); op_97();                            } /* DB   DD          */

OP(dd,98) { illegal_1(); op_98();                            } /* DB   DD          */
OP(dd,99) { illegal_1(); op_99();                            } /* DB   DD          */
OP(dd,9a) { illegal_1(); op_9a();                            } /* DB   DD          */
OP(dd,9b) { illegal_1(); op_9b();                            } /* DB   DD          */
OP(dd,9c) { SBC(HX);                                         } /* SBC  A,HX        */
OP(dd,9d) { SBC(LX);                                         } /* SBC  A,LX        */
OP(dd,9e) { EAX(); SBC(RM(m_ea));                            } /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(); op_9f();                            } /* DB   DD          */

OP(dd,a0) { illegal_1(); op_a0();                            } /* DB   DD          */
OP(dd,a1) { illegal_1(); op_a1();                            } /* DB   DD          */
OP(dd,a2) { illegal_1(); op_a2();                            } /* DB   DD          */
OP(dd,a3) { illegal_1(); op_a3();                            } /* DB   DD          */
OP(dd,a4) { AND(HX);                                         } /* AND  HX          */
OP(dd,a5) { AND(LX);                                         } /* AND  LX          */
OP(dd,a6) { EAX(); AND(RM(m_ea));                            } /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(); op_a7();                            } /* DB   DD          */

OP(dd,a8) { illegal_1(); op_a8();                            } /* DB   DD          */
OP(dd,a9) { illegal_1(); op_a9();                            } /* DB   DD          */
OP(dd,aa) { illegal_1(); op_aa();                            } /* DB   DD          */
OP(dd,ab) { illegal_1(); op_ab();                            } /* DB   DD          */
OP(dd,ac) { XOR(HX);                                         } /* XOR  HX          */
OP(dd,ad) { XOR(LX);                                         } /* XOR  LX          */
OP(dd,ae) { EAX(); XOR(RM(m_ea));                            } /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(); op_af();                            } /* DB   DD          */

OP(dd,b0) { illegal_1(); op_b0();                            } /* DB   DD          */
OP(dd,b1) { illegal_1(); op_b1();                            } /* DB   DD          */
OP(dd,b2) { illegal_1(); op_b2();                            } /* DB   DD          */
OP(dd,b3) { illegal_1(); op_b3();                            } /* DB   DD          */
OP(dd,b4) { OR(HX);                                          } /* OR   HX          */
OP(dd,b5) { OR(LX);                                          } /* OR   LX          */
OP(dd,b6) { EAX(); OR(RM(m_ea));                             } /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(); op_b7();                            } /* DB   DD          */

OP(dd,b8) { illegal_1(); op_b8();                            } /* DB   DD          */
OP(dd,b9) { illegal_1(); op_b9();                            } /* DB   DD          */
OP(dd,ba) { illegal_1(); op_ba();                            } /* DB   DD          */
OP(dd,bb) { illegal_1(); op_bb();                            } /* DB   DD          */
OP(dd,bc) { CP(HX);                                          } /* CP   HX          */
OP(dd,bd) { CP(LX);                                          } /* CP   LX          */
OP(dd,be) { EAX(); CP(RM(m_ea));                             } /* CP   (IX+o)      */
OP(dd,bf) { illegal_1(); op_bf();                            } /* DB   DD          */

OP(dd,c0) { illegal_1(); op_c0();                            } /* DB   DD          */
OP(dd,c1) { illegal_1(); op_c1();                            } /* DB   DD          */
OP(dd,c2) { illegal_1(); op_c2();                            } /* DB   DD          */
OP(dd,c3) { illegal_1(); op_c3();                            } /* DB   DD          */
OP(dd,c4) { illegal_1(); op_c4();                            } /* DB   DD          */
OP(dd,c5) { illegal_1(); op_c5();                            } /* DB   DD          */
OP(dd,c6) { illegal_1(); op_c6();                            } /* DB   DD          */
OP(dd,c7) { illegal_1(); op_c7();                            } /* DB   DD          */

OP(dd,c8) { illegal_1(); op_c8();                            } /* DB   DD          */
OP(dd,c9) { illegal_1(); op_c9();                            } /* DB   DD          */
OP(dd,ca) { illegal_1(); op_ca();                            } /* DB   DD          */
OP(dd,cb) { EAX(); EXEC(xycb,ARG());                         } /* **   DD CB xx    */
OP(dd,cc) { illegal_1(); op_cc();                            } /* DB   DD          */
OP(dd,cd) { illegal_1(); op_cd();                            } /* DB   DD          */
OP(dd,ce) { illegal_1(); op_ce();                            } /* DB   DD          */
OP(dd,cf) { illegal_1(); op_cf();                            } /* DB   DD          */

OP(dd,d0) { illegal_1(); op_d0();                            } /* DB   DD          */
OP(dd,d1) { illegal_1(); op_d1();                            } /* DB   DD          */
OP(dd,d2) { illegal_1(); op_d2();                            } /* DB   DD          */
OP(dd,d3) { illegal_1(); op_d3();                            } /* DB   DD          */
OP(dd,d4) { illegal_1(); op_d4();                            } /* DB   DD          */
OP(dd,d5) { illegal_1(); op_d5();                            } /* DB   DD          */
OP(dd,d6) { illegal_1(); op_d6();                            } /* DB   DD          */
OP(dd,d7) { illegal_1(); op_d7();                            } /* DB   DD          */

OP(dd,d8) { illegal_1(); op_d8();                            } /* DB   DD          */
OP(dd,d9) { illegal_1(); op_d9();                            } /* DB   DD          */
OP(dd,da) { illegal_1(); op_da();                            } /* DB   DD          */
OP(dd,db) { illegal_1(); op_db();                            } /* DB   DD          */
OP(dd,dc) { illegal_1(); op_dc();                            } /* DB   DD          */
OP(dd,dd) { illegal_1(); op_dd();                            } /* DB   DD          */
OP(dd,de) { illegal_1(); op_de();                            } /* DB   DD          */
OP(dd,df) { illegal_1(); op_df();                            } /* DB   DD          */

OP(dd,e0) { illegal_1(); op_e0();                            } /* DB   DD          */
OP(dd,e1) { POP(m_ix);                                       } /* POP  IX          */
OP(dd,e2) { illegal_1(); op_e2();                            } /* DB   DD          */
OP(dd,e3) { EXSP(m_ix);                                      } /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(); op_e4();                            } /* DB   DD          */
OP(dd,e5) { PUSH(m_ix);                                      } /* PUSH IX          */
OP(dd,e6) { illegal_1(); op_e6();                            } /* DB   DD          */
OP(dd,e7) { illegal_1(); op_e7();                            } /* DB   DD          */

OP(dd,e8) { illegal_1(); op_e8();                            } /* DB   DD          */
OP(dd,e9) { PC = IX;                                         } /* JP   (IX)        */
OP(dd,ea) { illegal_1(); op_ea();                            } /* DB   DD          */
OP(dd,eb) { illegal_1(); op_eb();                            } /* DB   DD          */
OP(dd,ec) { illegal_1(); op_ec();                            } /* DB   DD          */
OP(dd,ed) { illegal_1(); op_ed();                            } /* DB   DD          */
OP(dd,ee) { illegal_1(); op_ee();                            } /* DB   DD          */
OP(dd,ef) { illegal_1(); op_ef();                            } /* DB   DD          */

OP(dd,f0) { illegal_1(); op_f0();                            } /* DB   DD          */
OP(dd,f1) { illegal_1(); op_f1();                            } /* DB   DD          */
OP(dd,f2) { illegal_1(); op_f2();                            } /* DB   DD          */
OP(dd,f3) { illegal_1(); op_f3();                            } /* DB   DD          */
OP(dd,f4) { illegal_1(); op_f4();                            } /* DB   DD          */
OP(dd,f5) { illegal_1(); op_f5();                            } /* DB   DD          */
OP(dd,f6) { illegal_1(); op_f6();                            } /* DB   DD          */
OP(dd,f7) { illegal_1(); op_f7();                            } /* DB   DD          */

OP(dd,f8) { illegal_1(); op_f8();                            } /* DB   DD          */
OP(dd,f9) { SP = IX;                                         } /* LD   SP,IX       */
OP(dd,fa) { illegal_1(); op_fa();                            } /* DB   DD          */
OP(dd,fb) { illegal_1(); op_fb();                            } /* DB   DD          */
OP(dd,fc) { illegal_1(); op_fc();                            } /* DB   DD          */
OP(dd,fd) { illegal_1(); op_fd();                            } /* DB   DD          */
OP(dd,fe) { illegal_1(); op_fe();                            } /* DB   DD          */
OP(dd,ff) { illegal_1(); op_ff();                            } /* DB   DD          */

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OP(fd,00) { illegal_1(); op_00();                            } /* DB   FD          */
OP(fd,01) { illegal_1(); op_01();                            } /* DB   FD          */
OP(fd,02) { illegal_1(); op_02();                            } /* DB   FD          */
OP(fd,03) { illegal_1(); op_03();                            } /* DB   FD          */
OP(fd,04) { illegal_1(); op_04();                            } /* DB   FD          */
OP(fd,05) { illegal_1(); op_05();                            } /* DB   FD          */
OP(fd,06) { illegal_1(); op_06();                            } /* DB   FD          */
OP(fd,07) { illegal_1(); op_07();                            } /* DB   FD          */

OP(fd,08) { illegal_1(); op_08();                            } /* DB   FD          */
OP(fd,09) { ADD16(m_iy, m_bc);                               } /* ADD  IY,BC       */
OP(fd,0a) { illegal_1(); op_0a();                            } /* DB   FD          */
OP(fd,0b) { illegal_1(); op_0b();                            } /* DB   FD          */
OP(fd,0c) { illegal_1(); op_0c();                            } /* DB   FD          */
OP(fd,0d) { illegal_1(); op_0d();                            } /* DB   FD          */
OP(fd,0e) { illegal_1(); op_0e();                            } /* DB   FD          */
OP(fd,0f) { illegal_1(); op_0f();                            } /* DB   FD          */

OP(fd,10) { illegal_1(); op_10();                            } /* DB   FD          */
OP(fd,11) { illegal_1(); op_11();                            } /* DB   FD          */
OP(fd,12) { illegal_1(); op_12();                            } /* DB   FD          */
OP(fd,13) { illegal_1(); op_13();                            } /* DB   FD          */
OP(fd,14) { illegal_1(); op_14();                            } /* DB   FD          */
OP(fd,15) { illegal_1(); op_15();                            } /* DB   FD          */
OP(fd,16) { illegal_1(); op_16();                            } /* DB   FD          */
OP(fd,17) { illegal_1(); op_17();                            } /* DB   FD          */

OP(fd,18) { illegal_1(); op_18();                            } /* DB   FD          */
OP(fd,19) { ADD16(m_iy, m_de);                               } /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(); op_1a();                            } /* DB   FD          */
OP(fd,1b) { illegal_1(); op_1b();                            } /* DB   FD          */
OP(fd,1c) { illegal_1(); op_1c();                            } /* DB   FD          */
OP(fd,1d) { illegal_1(); op_1d();                            } /* DB   FD          */
OP(fd,1e) { illegal_1(); op_1e();                            } /* DB   FD          */
OP(fd,1f) { illegal_1(); op_1f();                            } /* DB   FD          */

OP(fd,20) { illegal_1(); op_20();                            } /* DB   FD          */
OP(fd,21) { IY = ARG16();                                    } /* LD   IY,w        */
OP(fd,22) { m_ea = ARG16(); WM16(m_ea, m_iy); WZ = m_ea + 1; } /* LD   (w),IY      */
OP(fd,23) { IY++;                                            } /* INC  IY          */
OP(fd,24) { HY = INC(HY);                                    } /* INC  HY          */
OP(fd,25) { HY = DEC(HY);                                    } /* DEC  HY          */
OP(fd,26) { HY = ARG();                                      } /* LD   HY,n        */
OP(fd,27) { illegal_1(); op_27();                            } /* DB   FD          */

OP(fd,28) { illegal_1(); op_28();                            } /* DB   FD          */
OP(fd,29) { ADD16(m_iy, m_iy);                               } /* ADD  IY,IY       */
OP(fd,2a) { m_ea = ARG16(); RM16(m_ea, m_iy); WZ = m_ea + 1; } /* LD   IY,(w)      */
OP(fd,2b) { IY--;                                            } /* DEC  IY          */
OP(fd,2c) { LY = INC(LY);                                    } /* INC  LY          */
OP(fd,2d) { LY = DEC(LY);                                    } /* DEC  LY          */
OP(fd,2e) { LY = ARG();                                      } /* LD   LY,n        */
OP(fd,2f) { illegal_1(); op_2f();                            } /* DB   FD          */

OP(fd,30) { illegal_1(); op_30();                            } /* DB   FD          */
OP(fd,31) { illegal_1(); op_31();                            } /* DB   FD          */
OP(fd,32) { illegal_1(); op_32();                            } /* DB   FD          */
OP(fd,33) { illegal_1(); op_33();                            } /* DB   FD          */
OP(fd,34) { EAY(); WM(m_ea, INC(RM(m_ea)));                  } /* INC  (IY+o)      */
OP(fd,35) { EAY(); WM(m_ea, DEC(RM(m_ea)));                  } /* DEC  (IY+o)      */
OP(fd,36) { EAY(); WM(m_ea, ARG());                          } /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(); op_37();                            } /* DB   FD          */

OP(fd,38) { illegal_1(); op_38();                            } /* DB   FD          */
OP(fd,39) { ADD16(m_iy, m_sp);                               } /* ADD  IY,SP       */
OP(fd,3a) { illegal_1(); op_3a();                            } /* DB   FD          */
OP(fd,3b) { illegal_1(); op_3b();                            } /* DB   FD          */
OP(fd,3c) { illegal_1(); op_3c();                            } /* DB   FD          */
OP(fd,3d) { illegal_1(); op_3d();                            } /* DB   FD          */
OP(fd,3e) { illegal_1(); op_3e();                            } /* DB   FD          */
OP(fd,3f) { illegal_1(); op_3f();                            } /* DB   FD          */

OP(fd,40) { illegal_1(); op_40();                            } /* DB   FD          */
OP(fd,41) { illegal_1(); op_41();                            } /* DB   FD          */
OP(fd,42) { illegal_1(); op_42();                            } /* DB   FD          */
OP(fd,43) { illegal_1(); op_43();                            } /* DB   FD          */
OP(fd,44) { B = HY;                                          } /* LD   B,HY        */
OP(fd,45) { B = LY;                                          } /* LD   B,LY        */
OP(fd,46) { EAY(); B = RM(m_ea);                             } /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(); op_47();                            } /* DB   FD          */

OP(fd,48) { illegal_1(); op_48();                            } /* DB   FD          */
OP(fd,49) { illegal_1(); op_49();                            } /* DB   FD          */
OP(fd,4a) { illegal_1(); op_4a();                            } /* DB   FD          */
OP(fd,4b) { illegal_1(); op_4b();                            } /* DB   FD          */
OP(fd,4c) { C = HY;                                          } /* LD   C,HY        */
OP(fd,4d) { C = LY;                                          } /* LD   C,LY        */
OP(fd,4e) { EAY(); C = RM(m_ea);                             } /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(); op_4f();                            } /* DB   FD          */

OP(fd,50) { illegal_1(); op_50();                            } /* DB   FD          */
OP(fd,51) { illegal_1(); op_51();                            } /* DB   FD          */
OP(fd,52) { illegal_1(); op_52();                            } /* DB   FD          */
OP(fd,53) { illegal_1(); op_53();                            } /* DB   FD          */
OP(fd,54) { D = HY;                                          } /* LD   D,HY        */
OP(fd,55) { D = LY;                                          } /* LD   D,LY        */
OP(fd,56) { EAY(); D = RM(m_ea);                             } /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(); op_57();                            } /* DB   FD          */

OP(fd,58) { illegal_1(); op_58();                            } /* DB   FD          */
OP(fd,59) { illegal_1(); op_59();                            } /* DB   FD          */
OP(fd,5a) { illegal_1(); op_5a();                            } /* DB   FD          */
OP(fd,5b) { illegal_1(); op_5b();                            } /* DB   FD          */
OP(fd,5c) { E = HY;                                          } /* LD   E,HY        */
OP(fd,5d) { E = LY;                                          } /* LD   E,LY        */
OP(fd,5e) { EAY(); E = RM(m_ea);                             } /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(); op_5f();                            } /* DB   FD          */

OP(fd,60) { HY = B;                                          } /* LD   HY,B        */
OP(fd,61) { HY = C;                                          } /* LD   HY,C        */
OP(fd,62) { HY = D;                                          } /* LD   HY,D        */
OP(fd,63) { HY = E;                                          } /* LD   HY,E        */
OP(fd,64) {                                                  } /* LD   HY,HY       */
OP(fd,65) { HY = LY;                                         } /* LD   HY,LY       */
OP(fd,66) { EAY(); H = RM(m_ea);                             } /* LD   H,(IY+o)    */
OP(fd,67) { HY = A;                                          } /* LD   HY,A        */

OP(fd,68) { LY = B;                                          } /* LD   LY,B        */
OP(fd,69) { LY = C;                                          } /* LD   LY,C        */
OP(fd,6a) { LY = D;                                          } /* LD   LY,D        */
OP(fd,6b) { LY = E;                                          } /* LD   LY,E        */
OP(fd,6c) { LY = HY;                                         } /* LD   LY,HY       */
OP(fd,6d) {                                                  } /* LD   LY,LY       */
OP(fd,6e) { EAY(); L = RM(m_ea);                             } /* LD   L,(IY+o)    */
OP(fd,6f) { LY = A;                                          } /* LD   LY,A        */

OP(fd,70) { EAY(); WM(m_ea, B);                              } /* LD   (IY+o),B    */
OP(fd,71) { EAY(); WM(m_ea, C);                              } /* LD   (IY+o),C    */
OP(fd,72) { EAY(); WM(m_ea, D);                              } /* LD   (IY+o),D    */
OP(fd,73) { EAY(); WM(m_ea, E);                              } /* LD   (IY+o),E    */
OP(fd,74) { EAY(); WM(m_ea, H);                              } /* LD   (IY+o),H    */
OP(fd,75) { EAY(); WM(m_ea, L);                              } /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(); op_76();                            } /* DB   FD          */
OP(fd,77) { EAY(); WM(m_ea, A);                              } /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(); op_78();                            } /* DB   FD          */
OP(fd,79) { illegal_1(); op_79();                            } /* DB   FD          */
OP(fd,7a) { illegal_1(); op_7a();                            } /* DB   FD          */
OP(fd,7b) { illegal_1(); op_7b();                            } /* DB   FD          */
OP(fd,7c) { A = HY;                                          } /* LD   A,HY        */
OP(fd,7d) { A = LY;                                          } /* LD   A,LY        */
OP(fd,7e) { EAY(); A = RM(m_ea);                             } /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(); op_7f();                            } /* DB   FD          */

OP(fd,80) { illegal_1(); op_80();                            } /* DB   FD          */
OP(fd,81) { illegal_1(); op_81();                            } /* DB   FD          */
OP(fd,82) { illegal_1(); op_82();                            } /* DB   FD          */
OP(fd,83) { illegal_1(); op_83();                            } /* DB   FD          */
OP(fd,84) { ADD(HY);                                         } /* ADD  A,HY        */
OP(fd,85) { ADD(LY);                                         } /* ADD  A,LY        */
OP(fd,86) { EAY(); ADD(RM(m_ea));                            } /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(); op_87();                            } /* DB   FD          */

OP(fd,88) { illegal_1(); op_88();                            } /* DB   FD          */
OP(fd,89) { illegal_1(); op_89();                            } /* DB   FD          */
OP(fd,8a) { illegal_1(); op_8a();                            } /* DB   FD          */
OP(fd,8b) { illegal_1(); op_8b();                            } /* DB   FD          */
OP(fd,8c) { ADC(HY);                                         } /* ADC  A,HY        */
OP(fd,8d) { ADC(LY);                                         } /* ADC  A,LY        */
OP(fd,8e) { EAY(); ADC(RM(m_ea));                            } /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(); op_8f();                            } /* DB   FD          */

OP(fd,90) { illegal_1(); op_90();                            } /* DB   FD          */
OP(fd,91) { illegal_1(); op_91();                            } /* DB   FD          */
OP(fd,92) { illegal_1(); op_92();                            } /* DB   FD          */
OP(fd,93) { illegal_1(); op_93();                            } /* DB   FD          */
OP(fd,94) { SUB(HY);                                         } /* SUB  HY          */
OP(fd,95) { SUB(LY);                                         } /* SUB  LY          */
OP(fd,96) { EAY(); SUB(RM(m_ea));                            } /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(); op_97();                            } /* DB   FD          */

OP(fd,98) { illegal_1(); op_98();                            } /* DB   FD          */
OP(fd,99) { illegal_1(); op_99();                            } /* DB   FD          */
OP(fd,9a) { illegal_1(); op_9a();                            } /* DB   FD          */
OP(fd,9b) { illegal_1(); op_9b();                            } /* DB   FD          */
OP(fd,9c) { SBC(HY);                                         } /* SBC  A,HY        */
OP(fd,9d) { SBC(LY);                                         } /* SBC  A,LY        */
OP(fd,9e) { EAY(); SBC(RM(m_ea));                            } /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(); op_9f();                            } /* DB   FD          */

OP(fd,a0) { illegal_1(); op_a0();                            } /* DB   FD          */
OP(fd,a1) { illegal_1(); op_a1();                            } /* DB   FD          */
OP(fd,a2) { illegal_1(); op_a2();                            } /* DB   FD          */
OP(fd,a3) { illegal_1(); op_a3();                            } /* DB   FD          */
OP(fd,a4) { AND(HY);                                         } /* AND  HY          */
OP(fd,a5) { AND(LY);                                         } /* AND  LY          */
OP(fd,a6) { EAY(); AND(RM(m_ea));                            } /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(); op_a7();                            } /* DB   FD          */

OP(fd,a8) { illegal_1(); op_a8();                            } /* DB   FD          */
OP(fd,a9) { illegal_1(); op_a9();                            } /* DB   FD          */
OP(fd,aa) { illegal_1(); op_aa();                            } /* DB   FD          */
OP(fd,ab) { illegal_1(); op_ab();                            } /* DB   FD          */
OP(fd,ac) { XOR(HY);                                         } /* XOR  HY          */
OP(fd,ad) { XOR(LY);                                         } /* XOR  LY          */
OP(fd,ae) { EAY(); XOR(RM(m_ea));                            } /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(); op_af();                            } /* DB   FD          */

OP(fd,b0) { illegal_1(); op_b0();                            } /* DB   FD          */
OP(fd,b1) { illegal_1(); op_b1();                            } /* DB   FD          */
OP(fd,b2) { illegal_1(); op_b2();                            } /* DB   FD          */
OP(fd,b3) { illegal_1(); op_b3();                            } /* DB   FD          */
OP(fd,b4) { OR(HY);                                          } /* OR   HY          */
OP(fd,b5) { OR(LY);                                          } /* OR   LY          */
OP(fd,b6) { EAY(); OR(RM(m_ea));                             } /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(); op_b7();                            } /* DB   FD          */

OP(fd,b8) { illegal_1(); op_b8();                            } /* DB   FD          */
OP(fd,b9) { illegal_1(); op_b9();                            } /* DB   FD          */
OP(fd,ba) { illegal_1(); op_ba();                            } /* DB   FD          */
OP(fd,bb) { illegal_1(); op_bb();                            } /* DB   FD          */
OP(fd,bc) { CP(HY);                                          } /* CP   HY          */
OP(fd,bd) { CP(LY);                                          } /* CP   LY          */
OP(fd,be) { EAY(); CP(RM(m_ea));                             } /* CP   (IY+o)      */
OP(fd,bf) { illegal_1(); op_bf();                            } /* DB   FD          */

OP(fd,c0) { illegal_1(); op_c0();                            } /* DB   FD          */
OP(fd,c1) { illegal_1(); op_c1();                            } /* DB   FD          */
OP(fd,c2) { illegal_1(); op_c2();                            } /* DB   FD          */
OP(fd,c3) { illegal_1(); op_c3();                            } /* DB   FD          */
OP(fd,c4) { illegal_1(); op_c4();                            } /* DB   FD          */
OP(fd,c5) { illegal_1(); op_c5();                            } /* DB   FD          */
OP(fd,c6) { illegal_1(); op_c6();                            } /* DB   FD          */
OP(fd,c7) { illegal_1(); op_c7();                            } /* DB   FD          */

OP(fd,c8) { illegal_1(); op_c8();                            } /* DB   FD          */
OP(fd,c9) { illegal_1(); op_c9();                            } /* DB   FD          */
OP(fd,ca) { illegal_1(); op_ca();                            } /* DB   FD          */
OP(fd,cb) { EAY(); EXEC(xycb,ARG());                         } /* **   FD CB xx    */
OP(fd,cc) { illegal_1(); op_cc();                            } /* DB   FD          */
OP(fd,cd) { illegal_1(); op_cd();                            } /* DB   FD          */
OP(fd,ce) { illegal_1(); op_ce();                            } /* DB   FD          */
OP(fd,cf) { illegal_1(); op_cf();                            } /* DB   FD          */

OP(fd,d0) { illegal_1(); op_d0();                            } /* DB   FD          */
OP(fd,d1) { illegal_1(); op_d1();                            } /* DB   FD          */
OP(fd,d2) { illegal_1(); op_d2();                            } /* DB   FD          */
OP(fd,d3) { illegal_1(); op_d3();                            } /* DB   FD          */
OP(fd,d4) { illegal_1(); op_d4();                            } /* DB   FD          */
OP(fd,d5) { illegal_1(); op_d5();                            } /* DB   FD          */
OP(fd,d6) { illegal_1(); op_d6();                            } /* DB   FD          */
OP(fd,d7) { illegal_1(); op_d7();                            } /* DB   FD          */

OP(fd,d8) { illegal_1(); op_d8();                            } /* DB   FD          */
OP(fd,d9) { illegal_1(); op_d9();                            } /* DB   FD          */
OP(fd,da) { illegal_1(); op_da();                            } /* DB   FD          */
OP(fd,db) { illegal_1(); op_db();                            } /* DB   FD          */
OP(fd,dc) { illegal_1(); op_dc();                            } /* DB   FD          */
OP(fd,dd) { illegal_1(); op_dd();                            } /* DB   FD          */
OP(fd,de) { illegal_1(); op_de();                            } /* DB   FD          */
OP(fd,df) { illegal_1(); op_df();                            } /* DB   FD          */

OP(fd,e0) { illegal_1(); op_e0();                            } /* DB   FD          */
OP(fd,e1) { POP(m_iy);                                       } /* POP  IY          */
OP(fd,e2) { illegal_1(); op_e2();                            } /* DB   FD          */
OP(fd,e3) { EXSP(m_iy);                                      } /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(); op_e4();                            } /* DB   FD          */
OP(fd,e5) { PUSH(m_iy);                                      } /* PUSH IY          */
OP(fd,e6) { illegal_1(); op_e6();                            } /* DB   FD          */
OP(fd,e7) { illegal_1(); op_e7();                            } /* DB   FD          */

OP(fd,e8) { illegal_1(); op_e8();                            } /* DB   FD          */
OP(fd,e9) { PC = IY;                                         } /* JP   (IY)        */
OP(fd,ea) { illegal_1(); op_ea();                            } /* DB   FD          */
OP(fd,eb) { illegal_1(); op_eb();                            } /* DB   FD          */
OP(fd,ec) { illegal_1(); op_ec();                            } /* DB   FD          */
OP(fd,ed) { illegal_1(); op_ed();                            } /* DB   FD          */
OP(fd,ee) { illegal_1(); op_ee();                            } /* DB   FD          */
OP(fd,ef) { illegal_1(); op_ef();                            } /* DB   FD          */

OP(fd,f0) { illegal_1(); op_f0();                            } /* DB   FD          */
OP(fd,f1) { illegal_1(); op_f1();                            } /* DB   FD          */
OP(fd,f2) { illegal_1(); op_f2();                            } /* DB   FD          */
OP(fd,f3) { illegal_1(); op_f3();                            } /* DB   FD          */
OP(fd,f4) { illegal_1(); op_f4();                            } /* DB   FD          */
OP(fd,f5) { illegal_1(); op_f5();                            } /* DB   FD          */
OP(fd,f6) { illegal_1(); op_f6();                            } /* DB   FD          */
OP(fd,f7) { illegal_1(); op_f7();                            } /* DB   FD          */

OP(fd,f8) { illegal_1(); op_f8();                            } /* DB   FD          */
OP(fd,f9) { SP = IY;                                         } /* LD   SP,IY       */
OP(fd,fa) { illegal_1(); op_fa();                            } /* DB   FD          */
OP(fd,fb) { illegal_1(); op_fb();                            } /* DB   FD          */
OP(fd,fc) { illegal_1(); op_fc();                            } /* DB   FD          */
OP(fd,fd) { illegal_1(); op_fd();                            } /* DB   FD          */
OP(fd,fe) { illegal_1(); op_fe();                            } /* DB   FD          */
OP(fd,ff) { illegal_1(); op_ff();                            } /* DB   FD          */

OP(illegal,2)
{
	logerror("Z80 '%s' ill. opcode $ed $%02x\n",
			tag(), m_direct->read_decrypted_byte((PCD-1)&0xffff));
}

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP(ed,00) { illegal_2();                                     } /* DB   ED          */
OP(ed,01) { illegal_2();                                     } /* DB   ED          */
OP(ed,02) { illegal_2();                                     } /* DB   ED          */
OP(ed,03) { illegal_2();                                     } /* DB   ED          */
OP(ed,04) { illegal_2();                                     } /* DB   ED          */
OP(ed,05) { illegal_2();                                     } /* DB   ED          */
OP(ed,06) { illegal_2();                                     } /* DB   ED          */
OP(ed,07) { illegal_2();                                     } /* DB   ED          */

OP(ed,08) { illegal_2();                                     } /* DB   ED          */
OP(ed,09) { illegal_2();                                     } /* DB   ED          */
OP(ed,0a) { illegal_2();                                     } /* DB   ED          */
OP(ed,0b) { illegal_2();                                     } /* DB   ED          */
OP(ed,0c) { illegal_2();                                     } /* DB   ED          */
OP(ed,0d) { illegal_2();                                     } /* DB   ED          */
OP(ed,0e) { illegal_2();                                     } /* DB   ED          */
OP(ed,0f) { illegal_2();                                     } /* DB   ED          */

OP(ed,10) { illegal_2();                                     } /* DB   ED          */
OP(ed,11) { illegal_2();                                     } /* DB   ED          */
OP(ed,12) { illegal_2();                                     } /* DB   ED          */
OP(ed,13) { illegal_2();                                     } /* DB   ED          */
OP(ed,14) { illegal_2();                                     } /* DB   ED          */
OP(ed,15) { illegal_2();                                     } /* DB   ED          */
OP(ed,16) { illegal_2();                                     } /* DB   ED          */
OP(ed,17) { illegal_2();                                     } /* DB   ED          */

OP(ed,18) { illegal_2();                                     } /* DB   ED          */
OP(ed,19) { illegal_2();                                     } /* DB   ED          */
OP(ed,1a) { illegal_2();                                     } /* DB   ED          */
OP(ed,1b) { illegal_2();                                     } /* DB   ED          */
OP(ed,1c) { illegal_2();                                     } /* DB   ED          */
OP(ed,1d) { illegal_2();                                     } /* DB   ED          */
OP(ed,1e) { illegal_2();                                     } /* DB   ED          */
OP(ed,1f) { illegal_2();                                     } /* DB   ED          */

OP(ed,20) { illegal_2();                                     } /* DB   ED          */
OP(ed,21) { illegal_2();                                     } /* DB   ED          */
OP(ed,22) { illegal_2();                                     } /* DB   ED          */
OP(ed,23) { illegal_2();                                     } /* DB   ED          */
OP(ed,24) { illegal_2();                                     } /* DB   ED          */
OP(ed,25) { illegal_2();                                     } /* DB   ED          */
OP(ed,26) { illegal_2();                                     } /* DB   ED          */
OP(ed,27) { illegal_2();                                     } /* DB   ED          */

OP(ed,28) { illegal_2();                                     } /* DB   ED          */
OP(ed,29) { illegal_2();                                     } /* DB   ED          */
OP(ed,2a) { illegal_2();                                     } /* DB   ED          */
OP(ed,2b) { illegal_2();                                     } /* DB   ED          */
OP(ed,2c) { illegal_2();                                     } /* DB   ED          */
OP(ed,2d) { illegal_2();                                     } /* DB   ED          */
OP(ed,2e) { illegal_2();                                     } /* DB   ED          */
OP(ed,2f) { illegal_2();                                     } /* DB   ED          */

OP(ed,30) { illegal_2();                                     } /* DB   ED          */
OP(ed,31) { illegal_2();                                     } /* DB   ED          */
OP(ed,32) { illegal_2();                                     } /* DB   ED          */
OP(ed,33) { illegal_2();                                     } /* DB   ED          */
OP(ed,34) { illegal_2();                                     } /* DB   ED          */
OP(ed,35) { illegal_2();                                     } /* DB   ED          */
OP(ed,36) { illegal_2();                                     } /* DB   ED          */
OP(ed,37) { illegal_2();                                     } /* DB   ED          */

OP(ed,38) { illegal_2();                                     } /* DB   ED          */
OP(ed,39) { illegal_2();                                     } /* DB   ED          */
OP(ed,3a) { illegal_2();                                     } /* DB   ED          */
OP(ed,3b) { illegal_2();                                     } /* DB   ED          */
OP(ed,3c) { illegal_2();                                     } /* DB   ED          */
OP(ed,3d) { illegal_2();                                     } /* DB   ED          */
OP(ed,3e) { illegal_2();                                     } /* DB   ED          */
OP(ed,3f) { illegal_2();                                     } /* DB   ED          */

OP(ed,40) { B = IN(BC); F = (F & CF) | SZP[B];               } /* IN   B,(C)       */
OP(ed,41) { OUT(BC, B);                                      } /* OUT  (C),B       */
OP(ed,42) { SBC16(m_bc);                                     } /* SBC  HL,BC       */
OP(ed,43) { m_ea = ARG16(); WM16(m_ea, m_bc); WZ = m_ea + 1; } /* LD   (w),BC      */
OP(ed,44) { NEG();                                           } /* NEG              */
OP(ed,45) { RETN();                                          } /* RETN             */
OP(ed,46) { m_im = 0;                                        } /* IM   0           */
OP(ed,47) { LD_I_A();                                        } /* LD   i,A         */

OP(ed,48) { C = IN(BC); F = (F & CF) | SZP[C];               } /* IN   C,(C)       */
OP(ed,49) { OUT(BC, C);                                      } /* OUT  (C),C       */
OP(ed,4a) { ADC16(m_bc);                                     } /* ADC  HL,BC       */
OP(ed,4b) { m_ea = ARG16(); RM16(m_ea, m_bc); WZ = m_ea + 1; } /* LD   BC,(w)      */
OP(ed,4c) { NEG();                                           } /* NEG              */
OP(ed,4d) { RETI();                                          } /* RETI             */
OP(ed,4e) { m_im = 0;                                        } /* IM   0           */
OP(ed,4f) { LD_R_A();                                        } /* LD   r,A         */

OP(ed,50) { D = IN(BC); F = (F & CF) | SZP[D];               } /* IN   D,(C)       */
OP(ed,51) { OUT(BC, D);                                      } /* OUT  (C),D       */
OP(ed,52) { SBC16(m_de);                                     } /* SBC  HL,DE       */
OP(ed,53) { m_ea = ARG16(); WM16(m_ea, m_de); WZ = m_ea + 1; } /* LD   (w),DE      */
OP(ed,54) { NEG();                                           } /* NEG              */
OP(ed,55) { RETN();                                          } /* RETN             */
OP(ed,56) { m_im = 1;                                        } /* IM   1           */
OP(ed,57) { LD_A_I();                                        } /* LD   A,i         */
 
OP(ed,58) { E = IN(BC); F = (F & CF) | SZP[E];               } /* IN   E,(C)       */
OP(ed,59) { OUT(BC, E);                                      } /* OUT  (C),E       */
OP(ed,5a) { ADC16(m_de);                                     } /* ADC  HL,DE       */
OP(ed,5b) { m_ea = ARG16(); RM16(m_ea, m_de); WZ = m_ea + 1; } /* LD   DE,(w)      */
OP(ed,5c) { NEG();                                           } /* NEG              */
OP(ed,5d) { RETI();                                          } /* RETI             */
OP(ed,5e) { m_im = 2;                                        } /* IM   2           */
OP(ed,5f) { LD_A_R();                                        } /* LD   A,r         */

OP(ed,60) { H = IN(BC); F = (F & CF) | SZP[H];               } /* IN   H,(C)       */
OP(ed,61) { OUT(BC, H);                                      } /* OUT  (C),H       */
OP(ed,62) { SBC16(m_hl);                                     } /* SBC  HL,HL       */
OP(ed,63) { m_ea = ARG16(); WM16(m_ea, m_hl); WZ = m_ea + 1; } /* LD   (w),HL      */
OP(ed,64) { NEG();                                           } /* NEG              */
OP(ed,65) { RETN();                                          } /* RETN             */
OP(ed,66) { m_im = 0;                                        } /* IM   0           */
OP(ed,67) { RRD();                                           } /* RRD  (HL)        */

OP(ed,68) { L = IN(BC); F = (F & CF) | SZP[L];               } /* IN   L,(C)       */
OP(ed,69) { OUT(BC, L);                                      } /* OUT  (C),L       */
OP(ed,6a) { ADC16(m_hl);                                     } /* ADC  HL,HL       */
OP(ed,6b) { m_ea = ARG16(); RM16(m_ea, m_hl); WZ = m_ea + 1; } /* LD   HL,(w)      */
OP(ed,6c) { NEG();                                           } /* NEG              */
OP(ed,6d) { RETI();                                          } /* RETI             */
OP(ed,6e) { m_im = 0;                                        } /* IM   0           */
OP(ed,6f) { RLD();                                           } /* RLD  (HL)        */

OP(ed,70) { UINT8 res = IN(BC); F = (F & CF) | SZP[res];     } /* IN   0,(C)       */
OP(ed,71) { OUT(BC, 0);                                      } /* OUT  (C),0       */
OP(ed,72) { SBC16(m_sp);                                     } /* SBC  HL,SP       */
OP(ed,73) { m_ea = ARG16(); WM16(m_ea, m_sp); WZ = m_ea + 1; } /* LD   (w),SP      */
OP(ed,74) { NEG();                                           } /* NEG              */
OP(ed,75) { RETN();                                          } /* RETN             */
OP(ed,76) { m_im = 1;                                        } /* IM   1           */
OP(ed,77) { illegal_2();                                     } /* DB   ED,77       */

OP(ed,78) { A = IN(BC); F = (F & CF) | SZP[A]; WZ = BC + 1;  } /* IN   A,(C)       */
OP(ed,79) { OUT(BC, A);  WZ = BC + 1;                        } /* OUT  (C),A       */
OP(ed,7a) { ADC16(m_sp);                                     } /* ADC  HL,SP       */
OP(ed,7b) { m_ea = ARG16(); RM16(m_ea, m_sp); WZ = m_ea + 1; } /* LD   SP,(w)      */
OP(ed,7c) { NEG();                                           } /* NEG              */
OP(ed,7d) { RETI();                                          } /* RETI             */
OP(ed,7e) { m_im = 2;                                        } /* IM   2           */
OP(ed,7f) { illegal_2();                                     } /* DB   ED,7F       */

OP(ed,80) { illegal_2();                                     } /* DB   ED          */
OP(ed,81) { illegal_2();                                     } /* DB   ED          */
OP(ed,82) { illegal_2();                                     } /* DB   ED          */
OP(ed,83) { illegal_2();                                     } /* DB   ED          */
OP(ed,84) { illegal_2();                                     } /* DB   ED          */
OP(ed,85) { illegal_2();                                     } /* DB   ED          */
OP(ed,86) { illegal_2();                                     } /* DB   ED          */
OP(ed,87) { illegal_2();                                     } /* DB   ED          */

OP(ed,88) { illegal_2();                                     } /* DB   ED          */
OP(ed,89) { illegal_2();                                     } /* DB   ED          */
OP(ed,8a) { illegal_2();                                     } /* DB   ED          */
OP(ed,8b) { illegal_2();                                     } /* DB   ED          */
OP(ed,8c) { illegal_2();                                     } /* DB   ED          */
OP(ed,8d) { illegal_2();                                     } /* DB   ED          */
OP(ed,8e) { illegal_2();                                     } /* DB   ED          */
OP(ed,8f) { illegal_2();                                     } /* DB   ED          */

OP(ed,90) { illegal_2();                                     } /* DB   ED          */
OP(ed,91) { illegal_2();                                     } /* DB   ED          */
OP(ed,92) { illegal_2();                                     } /* DB   ED          */
OP(ed,93) { illegal_2();                                     } /* DB   ED          */
OP(ed,94) { illegal_2();                                     } /* DB   ED          */
OP(ed,95) { illegal_2();                                     } /* DB   ED          */
OP(ed,96) { illegal_2();                                     } /* DB   ED          */
OP(ed,97) { illegal_2();                                     } /* DB   ED          */

OP(ed,98) { illegal_2();                                     } /* DB   ED          */
OP(ed,99) { illegal_2();                                     } /* DB   ED          */
OP(ed,9a) { illegal_2();                                     } /* DB   ED          */
OP(ed,9b) { illegal_2();                                     } /* DB   ED          */
OP(ed,9c) { illegal_2();                                     } /* DB   ED          */
OP(ed,9d) { illegal_2();                                     } /* DB   ED          */
OP(ed,9e) { illegal_2();                                     } /* DB   ED          */
OP(ed,9f) { illegal_2();                                     } /* DB   ED          */

OP(ed,a0) { LDI();                                           } /* LDI              */
OP(ed,a1) { CPI();                                           } /* CPI              */
OP(ed,a2) { INI();                                           } /* INI              */
OP(ed,a3) { OUTI();                                          } /* OUTI             */
OP(ed,a4) { illegal_2();                                     } /* DB   ED          */
OP(ed,a5) { illegal_2();                                     } /* DB   ED          */
OP(ed,a6) { illegal_2();                                     } /* DB   ED          */
OP(ed,a7) { illegal_2();                                     } /* DB   ED          */

OP(ed,a8) { LDD();                                           } /* LDD              */
OP(ed,a9) { CPD();                                           } /* CPD              */
OP(ed,aa) { IND();                                           } /* IND              */
OP(ed,ab) { OUTD();                                          } /* OUTD             */
OP(ed,ac) { illegal_2();                                     } /* DB   ED          */
OP(ed,ad) { illegal_2();                                     } /* DB   ED          */
OP(ed,ae) { illegal_2();                                     } /* DB   ED          */
OP(ed,af) { illegal_2();                                     } /* DB   ED          */

OP(ed,b0) { LDIR();                                          } /* LDIR             */
OP(ed,b1) { CPIR();                                          } /* CPIR             */
OP(ed,b2) { INIR();                                          } /* INIR             */
OP(ed,b3) { OTIR();                                          } /* OTIR             */
OP(ed,b4) { illegal_2();                                     } /* DB   ED          */
OP(ed,b5) { illegal_2();                                     } /* DB   ED          */
OP(ed,b6) { illegal_2();                                     } /* DB   ED          */
OP(ed,b7) { illegal_2();                                     } /* DB   ED          */

OP(ed,b8) { LDDR();                                          } /* LDDR             */
OP(ed,b9) { CPDR();                                          } /* CPDR             */
OP(ed,ba) { INDR();                                          } /* INDR             */
OP(ed,bb) { OTDR();                                          } /* OTDR             */
OP(ed,bc) { illegal_2();                                     } /* DB   ED          */
OP(ed,bd) { illegal_2();                                     } /* DB   ED          */
OP(ed,be) { illegal_2();                                     } /* DB   ED          */
OP(ed,bf) { illegal_2();                                     } /* DB   ED          */

OP(ed,c0) { illegal_2();                                     } /* DB   ED          */
OP(ed,c1) { illegal_2();                                     } /* DB   ED          */
OP(ed,c2) { illegal_2();                                     } /* DB   ED          */
OP(ed,c3) { illegal_2();                                     } /* DB   ED          */
OP(ed,c4) { illegal_2();                                     } /* DB   ED          */
OP(ed,c5) { illegal_2();                                     } /* DB   ED          */
OP(ed,c6) { illegal_2();                                     } /* DB   ED          */
OP(ed,c7) { illegal_2();                                     } /* DB   ED          */

OP(ed,c8) { illegal_2();                                     } /* DB   ED          */
OP(ed,c9) { illegal_2();                                     } /* DB   ED          */
OP(ed,ca) { illegal_2();                                     } /* DB   ED          */
OP(ed,cb) { illegal_2();                                     } /* DB   ED          */
OP(ed,cc) { illegal_2();                                     } /* DB   ED          */
OP(ed,cd) { illegal_2();                                     } /* DB   ED          */
OP(ed,ce) { illegal_2();                                     } /* DB   ED          */
OP(ed,cf) { illegal_2();                                     } /* DB   ED          */

OP(ed,d0) { illegal_2();                                     } /* DB   ED          */
OP(ed,d1) { illegal_2();                                     } /* DB   ED          */
OP(ed,d2) { illegal_2();                                     } /* DB   ED          */
OP(ed,d3) { illegal_2();                                     } /* DB   ED          */
OP(ed,d4) { illegal_2();                                     } /* DB   ED          */
OP(ed,d5) { illegal_2();                                     } /* DB   ED          */
OP(ed,d6) { illegal_2();                                     } /* DB   ED          */
OP(ed,d7) { illegal_2();                                     } /* DB   ED          */

OP(ed,d8) { illegal_2();                                     } /* DB   ED          */
OP(ed,d9) { illegal_2();                                     } /* DB   ED          */
OP(ed,da) { illegal_2();                                     } /* DB   ED          */
OP(ed,db) { illegal_2();                                     } /* DB   ED          */
OP(ed,dc) { illegal_2();                                     } /* DB   ED          */
OP(ed,dd) { illegal_2();                                     } /* DB   ED          */
OP(ed,de) { illegal_2();                                     } /* DB   ED          */
OP(ed,df) { illegal_2();                                     } /* DB   ED          */

OP(ed,e0) { illegal_2();                                     } /* DB   ED          */
OP(ed,e1) { illegal_2();                                     } /* DB   ED          */
OP(ed,e2) { illegal_2();                                     } /* DB   ED          */
OP(ed,e3) { illegal_2();                                     } /* DB   ED          */
OP(ed,e4) { illegal_2();                                     } /* DB   ED          */
OP(ed,e5) { illegal_2();                                     } /* DB   ED          */
OP(ed,e6) { illegal_2();                                     } /* DB   ED          */
OP(ed,e7) { illegal_2();                                     } /* DB   ED          */

OP(ed,e8) { illegal_2();                                     } /* DB   ED          */
OP(ed,e9) { illegal_2();                                     } /* DB   ED          */
OP(ed,ea) { illegal_2();                                     } /* DB   ED          */
OP(ed,eb) { illegal_2();                                     } /* DB   ED          */
OP(ed,ec) { illegal_2();                                     } /* DB   ED          */
OP(ed,ed) { illegal_2();                                     } /* DB   ED          */
OP(ed,ee) { illegal_2();                                     } /* DB   ED          */
OP(ed,ef) { illegal_2();                                     } /* DB   ED          */

OP(ed,f0) { illegal_2();                                     } /* DB   ED          */
OP(ed,f1) { illegal_2();                                     } /* DB   ED          */
OP(ed,f2) { illegal_2();                                     } /* DB   ED          */
OP(ed,f3) { illegal_2();                                     } /* DB   ED          */
OP(ed,f4) { illegal_2();                                     } /* DB   ED          */
OP(ed,f5) { illegal_2();                                     } /* DB   ED          */
OP(ed,f6) { illegal_2();                                     } /* DB   ED          */
OP(ed,f7) { illegal_2();                                     } /* DB   ED          */

OP(ed,f8) { illegal_2();                                     } /* DB   ED          */
OP(ed,f9) { illegal_2();                                     } /* DB   ED          */
OP(ed,fa) { illegal_2();                                     } /* DB   ED          */
OP(ed,fb) { illegal_2();                                     } /* DB   ED          */
OP(ed,fc) { illegal_2();                                     } /* DB   ED          */
OP(ed,fd) { illegal_2();                                     } /* DB   ED          */
OP(ed,fe) { illegal_2();                                     } /* DB   ED          */
OP(ed,ff) { illegal_2();                                     } /* DB   ED          */


/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {                                                                       } /* NOP              */
OP(op,01) { BC = ARG16();                                                         } /* LD   BC,w        */
OP(op,02) { WM(BC,A); WZ_L = (BC + 1) & 0xFF;  WZ_H = A;                          } /* LD (BC),A */
OP(op,03) { BC++;                                                                 } /* INC  BC          */
OP(op,04) { B = INC(B);                                                           } /* INC  B           */
OP(op,05) { B = DEC(B);                                                           } /* DEC  B           */
OP(op,06) { B = ARG();                                                            } /* LD   B,n         */
OP(op,07) { RLCA();                                                               } /* RLCA             */

OP(op,08) { EX_AF();                                                              } /* EX   AF,AF'      */
OP(op,09) { ADD16(m_hl, m_bc);                                                    } /* ADD  HL,BC       */
OP(op,0a) { A = RM(BC);  WZ=BC+1;                                                 } /* LD   A,(BC)      */
OP(op,0b) { BC--;                                                                 } /* DEC  BC          */
OP(op,0c) { C = INC(C);                                                           } /* INC  C           */
OP(op,0d) { C = DEC(C);                                                           } /* DEC  C           */
OP(op,0e) { C = ARG();                                                            } /* LD   C,n         */
OP(op,0f) { RRCA();                                                               } /* RRCA             */

OP(op,10) { B--; JR_COND(B, 0x10);                                                } /* DJNZ o           */
OP(op,11) { DE = ARG16();                                                         } /* LD   DE,w        */
OP(op,12) { WM(DE,A); WZ_L = (DE + 1) & 0xFF;  WZ_H = A;                          } /* LD (DE),A */
OP(op,13) { DE++;                                                                 } /* INC  DE          */
OP(op,14) { D = INC(D);                                                           } /* INC  D           */
OP(op,15) { D = DEC(D);                                                           } /* DEC  D           */
OP(op,16) { D = ARG();                                                            } /* LD   D,n         */
OP(op,17) { RLA();                                                                } /* RLA              */

OP(op,18) { JR();                                                                 } /* JR   o           */
OP(op,19) { ADD16(m_hl, m_de);                                                    } /* ADD  HL,DE       */
OP(op,1a) { A = RM(DE); WZ = DE + 1;                                              } /* LD   A,(DE)      */
OP(op,1b) { DE--;                                                                 } /* DEC  DE          */
OP(op,1c) { E = INC(E);                                                           } /* INC  E           */
OP(op,1d) { E = DEC(E);                                                           } /* DEC  E           */
OP(op,1e) { E = ARG();                                                            } /* LD   E,n         */
OP(op,1f) { RRA();                                                                } /* RRA              */

OP(op,20) { JR_COND(!(F & ZF), 0x20);                                             } /* JR   NZ,o        */
OP(op,21) { HL = ARG16();                                                         } /* LD   HL,w        */
OP(op,22) { m_ea = ARG16(); WM16(m_ea, m_hl); WZ = m_ea + 1;                      } /* LD   (w),HL      */
OP(op,23) { HL++;                                                                 } /* INC  HL          */
OP(op,24) { H = INC(H);                                                           } /* INC  H           */
OP(op,25) { H = DEC(H);                                                           } /* DEC  H           */
OP(op,26) { H = ARG();                                                            } /* LD   H,n         */
OP(op,27) { DAA();                                                                } /* DAA              */

OP(op,28) { JR_COND(F & ZF, 0x28);                                                } /* JR   Z,o         */
OP(op,29) { ADD16(m_hl, m_hl);                                                    } /* ADD  HL,HL       */
OP(op,2a) { m_ea = ARG16(); RM16(m_ea, m_hl); WZ = m_ea+1;                        } /* LD   HL,(w)      */
OP(op,2b) { HL--;                                                                 } /* DEC  HL          */
OP(op,2c) { L = INC(L);                                                           } /* INC  L           */
OP(op,2d) { L = DEC(L);                                                           } /* DEC  L           */
OP(op,2e) { L = ARG();                                                            } /* LD   L,n         */
OP(op,2f) { A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); } /* CPL              */

OP(op,30) { JR_COND(!(F & CF), 0x30);                                             } /* JR   NC,o        */
OP(op,31) { SP = ARG16();                                                         } /* LD   SP,w        */
OP(op,32) { m_ea = ARG16(); WM(m_ea, A); WZ_L = (m_ea + 1) & 0xFF; WZ_H = A;      } /* LD   (w),A       */
OP(op,33) { SP++;                                                                 } /* INC  SP          */
OP(op,34) { WM(HL, INC(RM(HL)));                                                  } /* INC  (HL)        */
OP(op,35) { WM(HL, DEC(RM(HL)));                                                  } /* DEC  (HL)        */
OP(op,36) { WM(HL, ARG());                                                        } /* LD   (HL),n      */
OP(op,37) { F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF));            } /* SCF              */

OP(op,38) { JR_COND(F & CF, 0x38);                                                } /* JR   C,o         */
OP(op,39) { ADD16(m_hl, m_sp);                                                    } /* ADD  HL,SP       */
OP(op,3a) { m_ea = ARG16(); A = RM(m_ea); WZ = m_ea + 1;                          } /* LD   A,(w)       */
OP(op,3b) { SP--;                                                                 } /* DEC  SP          */
OP(op,3c) { A = INC(A);                                                           } /* INC  A           */
OP(op,3d) { A = DEC(A);                                                           } /* DEC  A           */
OP(op,3e) { A = ARG();                                                            } /* LD   A,n         */
OP(op,3f) { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;             } /* CCF        */

OP(op,40) {                                                                       } /* LD   B,B         */
OP(op,41) { B = C;                                                                } /* LD   B,C         */
OP(op,42) { B = D;                                                                } /* LD   B,D         */
OP(op,43) { B = E;                                                                } /* LD   B,E         */
OP(op,44) { B = H;                                                                } /* LD   B,H         */
OP(op,45) { B = L;                                                                } /* LD   B,L         */
OP(op,46) { B = RM(HL);                                                           } /* LD   B,(HL)      */
OP(op,47) { B = A;                                                                } /* LD   B,A         */

OP(op,48) { C = B;                                                                } /* LD   C,B         */
OP(op,49) {                                                                       } /* LD   C,C         */
OP(op,4a) { C = D;                                                                } /* LD   C,D         */
OP(op,4b) { C = E;                                                                } /* LD   C,E         */
OP(op,4c) { C = H;                                                                } /* LD   C,H         */
OP(op,4d) { C = L;                                                                } /* LD   C,L         */
OP(op,4e) { C = RM(HL);                                                           } /* LD   C,(HL)      */
OP(op,4f) { C = A;                                                                } /* LD   C,A         */

OP(op,50) { D = B;                                                                } /* LD   D,B         */
OP(op,51) { D = C;                                                                } /* LD   D,C         */
OP(op,52) {                                                                       } /* LD   D,D         */
OP(op,53) { D = E;                                                                } /* LD   D,E         */
OP(op,54) { D = H;                                                                } /* LD   D,H         */
OP(op,55) { D = L;                                                                } /* LD   D,L         */
OP(op,56) { D = RM(HL);                                                           } /* LD   D,(HL)      */
OP(op,57) { D = A;                                                                } /* LD   D,A         */

OP(op,58) { E = B;                                                                } /* LD   E,B         */
OP(op,59) { E = C;                                                                } /* LD   E,C         */
OP(op,5a) { E = D;                                                                } /* LD   E,D         */
OP(op,5b) {                                                                       } /* LD   E,E         */
OP(op,5c) { E = H;                                                                } /* LD   E,H         */
OP(op,5d) { E = L;                                                                } /* LD   E,L         */
OP(op,5e) { E = RM(HL);                                                           } /* LD   E,(HL)      */
OP(op,5f) { E = A;                                                                } /* LD   E,A         */

OP(op,60) { H = B;                                                                } /* LD   H,B         */
OP(op,61) { H = C;                                                                } /* LD   H,C         */
OP(op,62) { H = D;                                                                } /* LD   H,D         */
OP(op,63) { H = E;                                                                } /* LD   H,E         */
OP(op,64) {                                                                       } /* LD   H,H         */
OP(op,65) { H = L;                                                                } /* LD   H,L         */
OP(op,66) { H = RM(HL);                                                           } /* LD   H,(HL)      */
OP(op,67) { H = A;                                                                } /* LD   H,A         */

OP(op,68) { L = B;                                                                } /* LD   L,B         */
OP(op,69) { L = C;                                                                } /* LD   L,C         */
OP(op,6a) { L = D;                                                                } /* LD   L,D         */
OP(op,6b) { L = E;                                                                } /* LD   L,E         */
OP(op,6c) { L = H;                                                                } /* LD   L,H         */
OP(op,6d) {                                                                       } /* LD   L,L         */
OP(op,6e) { L = RM(HL);                                                           } /* LD   L,(HL)      */
OP(op,6f) { L = A;                                                                } /* LD   L,A         */

OP(op,70) { WM(HL, B);                                                            } /* LD   (HL),B      */
OP(op,71) { WM(HL, C);                                                            } /* LD   (HL),C      */
OP(op,72) { WM(HL, D);                                                            } /* LD   (HL),D      */
OP(op,73) { WM(HL, E);                                                            } /* LD   (HL),E      */
OP(op,74) { WM(HL, H);                                                            } /* LD   (HL),H      */
OP(op,75) { WM(HL, L);                                                            } /* LD   (HL),L      */
OP(op,76) { ENTER_HALT();                                                         } /* halt             */
OP(op,77) { WM(HL, A);                                                            } /* LD   (HL),A      */

OP(op,78) { A = B;                                                                } /* LD   A,B         */
OP(op,79) { A = C;                                                                } /* LD   A,C         */
OP(op,7a) { A = D;                                                                } /* LD   A,D         */
OP(op,7b) { A = E;                                                                } /* LD   A,E         */
OP(op,7c) { A = H;                                                                } /* LD   A,H         */
OP(op,7d) { A = L;                                                                } /* LD   A,L         */
OP(op,7e) { A = RM(HL);                                                           } /* LD   A,(HL)      */
OP(op,7f) {                                                                       } /* LD   A,A         */

OP(op,80) { ADD(B);                                                               } /* ADD  A,B         */
OP(op,81) { ADD(C);                                                               } /* ADD  A,C         */
OP(op,82) { ADD(D);                                                               } /* ADD  A,D         */
OP(op,83) { ADD(E);                                                               } /* ADD  A,E         */
OP(op,84) { ADD(H);                                                               } /* ADD  A,H         */
OP(op,85) { ADD(L);                                                               } /* ADD  A,L         */
OP(op,86) { ADD(RM(HL));                                                          } /* ADD  A,(HL)      */
OP(op,87) { ADD(A);                                                               } /* ADD  A,A         */

OP(op,88) { ADC(B);                                                               } /* ADC  A,B         */
OP(op,89) { ADC(C);                                                               } /* ADC  A,C         */
OP(op,8a) { ADC(D);                                                               } /* ADC  A,D         */
OP(op,8b) { ADC(E);                                                               } /* ADC  A,E         */
OP(op,8c) { ADC(H);                                                               } /* ADC  A,H         */
OP(op,8d) { ADC(L);                                                               } /* ADC  A,L         */
OP(op,8e) { ADC(RM(HL));                                                          } /* ADC  A,(HL)      */
OP(op,8f) { ADC(A);                                                               } /* ADC  A,A         */

OP(op,90) { SUB(B);                                                               } /* SUB  B           */
OP(op,91) { SUB(C);                                                               } /* SUB  C           */
OP(op,92) { SUB(D);                                                               } /* SUB  D           */
OP(op,93) { SUB(E);                                                               } /* SUB  E           */
OP(op,94) { SUB(H);                                                               } /* SUB  H           */
OP(op,95) { SUB(L);                                                               } /* SUB  L           */
OP(op,96) { SUB(RM(HL));                                                          } /* SUB  (HL)        */
OP(op,97) { SUB(A);                                                               } /* SUB  A           */

OP(op,98) { SBC(B);                                                               } /* SBC  A,B         */
OP(op,99) { SBC(C);                                                               } /* SBC  A,C         */
OP(op,9a) { SBC(D);                                                               } /* SBC  A,D         */
OP(op,9b) { SBC(E);                                                               } /* SBC  A,E         */
OP(op,9c) { SBC(H);                                                               } /* SBC  A,H         */
OP(op,9d) { SBC(L);                                                               } /* SBC  A,L         */
OP(op,9e) { SBC(RM(HL));                                                          } /* SBC  A,(HL)      */
OP(op,9f) { SBC(A);                                                               } /* SBC  A,A         */

OP(op,a0) { AND(B);                                                               } /* AND  B           */
OP(op,a1) { AND(C);                                                               } /* AND  C           */
OP(op,a2) { AND(D);                                                               } /* AND  D           */
OP(op,a3) { AND(E);                                                               } /* AND  E           */
OP(op,a4) { AND(H);                                                               } /* AND  H           */
OP(op,a5) { AND(L);                                                               } /* AND  L           */
OP(op,a6) { AND(RM(HL));                                                          } /* AND  (HL)        */
OP(op,a7) { AND(A);                                                               } /* AND  A           */

OP(op,a8) { XOR(B);                                                               } /* XOR  B           */
OP(op,a9) { XOR(C);                                                               } /* XOR  C           */
OP(op,aa) { XOR(D);                                                               } /* XOR  D           */
OP(op,ab) { XOR(E);                                                               } /* XOR  E           */
OP(op,ac) { XOR(H);                                                               } /* XOR  H           */
OP(op,ad) { XOR(L);                                                               } /* XOR  L           */
OP(op,ae) { XOR(RM(HL));                                                          } /* XOR  (HL)        */
OP(op,af) { XOR(A);                                                               } /* XOR  A           */

OP(op,b0) { OR(B);                                                                } /* OR   B           */
OP(op,b1) { OR(C);                                                                } /* OR   C           */
OP(op,b2) { OR(D);                                                                } /* OR   D           */
OP(op,b3) { OR(E);                                                                } /* OR   E           */
OP(op,b4) { OR(H);                                                                } /* OR   H           */
OP(op,b5) { OR(L);                                                                } /* OR   L           */
OP(op,b6) { OR(RM(HL));                                                           } /* OR   (HL)        */
OP(op,b7) { OR(A);                                                                } /* OR   A           */

OP(op,b8) { CP(B);                                                                } /* CP   B           */
OP(op,b9) { CP(C);                                                                } /* CP   C           */
OP(op,ba) { CP(D);                                                                } /* CP   D           */
OP(op,bb) { CP(E);                                                                } /* CP   E           */
OP(op,bc) { CP(H);                                                                } /* CP   H           */
OP(op,bd) { CP(L);                                                                } /* CP   L           */
OP(op,be) { CP(RM(HL));                                                           } /* CP   (HL)        */
OP(op,bf) { CP(A);                                                                } /* CP   A           */

OP(op,c0) { RET_COND(!(F & ZF), 0xc0);                                            } /* RET  NZ          */
OP(op,c1) { POP(m_bc);                                                            } /* POP  BC          */
OP(op,c2) { JP_COND(!(F & ZF));                                                   } /* JP   NZ,a        */
OP(op,c3) { JP();                                                                 } /* JP   a           */
OP(op,c4) { CALL_COND(!(F & ZF), 0xc4);                                           } /* CALL NZ,a        */
OP(op,c5) { PUSH(m_bc);                                                           } /* PUSH BC          */
OP(op,c6) { ADD(ARG());                                                           } /* ADD  A,n         */
OP(op,c7) { RST(0x00);                                                            } /* RST  0           */

OP(op,c8) { RET_COND(F & ZF, 0xc8);                                               } /* RET  Z           */
OP(op,c9) { POP(m_pc); WZ = PCD;                                                  } /* RET              */
OP(op,ca) { JP_COND(F & ZF);                                                      } /* JP   Z,a         */
OP(op,cb) { m_r++; EXEC(cb,ROP());                                                } /* **** CB xx       */
OP(op,cc) { CALL_COND(F & ZF, 0xcc);                                              } /* CALL Z,a         */
OP(op,cd) { CALL();                                                               } /* CALL a           */
OP(op,ce) { ADC(ARG());                                                           } /* ADC  A,n         */
OP(op,cf) { RST(0x08);                                                            } /* RST  1           */

OP(op,d0) { RET_COND(!(F & CF), 0xd0);                                            } /* RET  NC          */
OP(op,d1) { POP(m_de);                                                            } /* POP  DE          */
OP(op,d2) { JP_COND(!(F & CF));                                                   } /* JP   NC,a        */
OP(op,d3) { unsigned n = ARG() | (A << 8); OUT(n, A); WZ_L = ((n & 0xff) + 1) & 0xff;  WZ_H = A;   } /* OUT  (n),A       */
OP(op,d4) { CALL_COND(!(F & CF), 0xd4);                                           } /* CALL NC,a        */
OP(op,d5) { PUSH(m_de);                                                           } /* PUSH DE          */
OP(op,d6) { SUB(ARG());                                                           } /* SUB  n           */
OP(op,d7) { RST(0x10);                                                            } /* RST  2           */

OP(op,d8) { RET_COND(F & CF, 0xd8);                                               } /* RET  C           */
OP(op,d9) { EXX();                                                                } /* EXX              */
OP(op,da) { JP_COND(F & CF);                                                      } /* JP   C,a         */
OP(op,db) { unsigned n = ARG() | (A << 8); A = IN(n); WZ = n + 1;                 } /* IN   A,(n)  */
OP(op,dc) { CALL_COND(F & CF, 0xdc);                                              } /* CALL C,a         */
OP(op,dd) { m_r++; EXEC(dd,ROP());                                                } /* **** DD xx       */
OP(op,de) { SBC(ARG());                                                           } /* SBC  A,n         */
OP(op,df) { RST(0x18);                                                            } /* RST  3           */

OP(op,e0) { RET_COND(!(F & PF), 0xe0);                                            } /* RET  PO          */
OP(op,e1) { POP(m_hl);                                                            } /* POP  HL          */
OP(op,e2) { JP_COND(!(F & PF));                                                   } /* JP   PO,a        */
OP(op,e3) { EXSP(m_hl);                                                           } /* EX   HL,(SP)     */
OP(op,e4) { CALL_COND(!(F & PF), 0xe4);                                           } /* CALL PO,a        */
OP(op,e5) { PUSH(m_hl);                                                           } /* PUSH HL          */
OP(op,e6) { AND(ARG());                                                           } /* AND  n           */
OP(op,e7) { RST(0x20);                                                            } /* RST  4           */

OP(op,e8) { RET_COND(F & PF, 0xe8);                                               } /* RET  PE          */
OP(op,e9) { PC = HL;                                                              } /* JP   (HL)        */
OP(op,ea) { JP_COND(F & PF);                                                      } /* JP   PE,a        */
OP(op,eb) { EX_DE_HL();                                                           } /* EX   DE,HL       */
OP(op,ec) { CALL_COND(F & PF, 0xec);                                              } /* CALL PE,a        */
OP(op,ed) { m_r++; EXEC(ed,ROP());                                                } /* **** ED xx       */
OP(op,ee) { XOR(ARG());                                                           } /* XOR  n           */
OP(op,ef) { RST(0x28);                                                            } /* RST  5           */

OP(op,f0) { RET_COND(!(F & SF), 0xf0);                                            } /* RET  P           */
OP(op,f1) { POP(m_af);                                                            } /* POP  AF          */
OP(op,f2) { JP_COND(!(F & SF));                                                   } /* JP   P,a         */
OP(op,f3) { m_iff1 = m_iff2 = 0;                                                  } /* DI               */
OP(op,f4) { CALL_COND(!(F & SF), 0xf4);                                           } /* CALL P,a         */
OP(op,f5) { PUSH(m_af);                                                           } /* PUSH AF          */
OP(op,f6) { OR(ARG());                                                            } /* OR   n           */
OP(op,f7) { RST(0x30);                                                            } /* RST  6           */

OP(op,f8) { RET_COND(F & SF, 0xf8);                                               } /* RET  M           */
OP(op,f9) { SP = HL;                                                              } /* LD   SP,HL       */
OP(op,fa) { JP_COND(F & SF);                                                      } /* JP   M,a         */
OP(op,fb) { EI();                                                                 } /* EI               */
OP(op,fc) { CALL_COND(F & SF, 0xfc);                                              } /* CALL M,a         */
OP(op,fd) { m_r++; EXEC(fd,ROP());                                                } /* **** FD xx       */
OP(op,fe) { CP(ARG());                                                            } /* CP   n           */
OP(op,ff) { RST(0x38);                                                            } /* RST  7           */


void z80_device::take_interrupt()
{
	int irq_vector;

	/* there isn't a valid previous program counter */
	PRVPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT();

	/* Clear both interrupt flip flops */
	m_iff1 = m_iff2 = 0;

	/* Daisy chain mode? If so, call the requesting device */
	if (m_daisy.present())
		irq_vector = m_daisy.call_ack_device();

	/* else call back the cpu interface to retrieve the vector */
	else
		irq_vector = (*m_irq_callback)(this, 0);

	LOG(("Z80 '%s' single int. irq_vector $%02x\n", tag(), irq_vector));

	/* Interrupt mode 2. Call [i:databyte] */
	if( m_im == 2 )
	{
		irq_vector = (irq_vector & 0xff) | (m_i << 8);
		PUSH(m_pc);
		RM16(irq_vector, m_pc);
		LOG(("Z80 '%s' IM2 [$%04x] = $%04x\n", tag(), irq_vector, PCD));
		/* CALL opcode timing + 'interrupt latency' cycles */
		m_icount -= m_cc_op[0xcd] + m_cc_ex[0xff];
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( m_im == 1 )
	{
		LOG(("Z80 '%s' IM1 $0038\n", tag()));
		PUSH(m_pc);
		PCD = 0x0038;
		/* RST $38 + 'interrupt latency' cycles */
		m_icount -= m_cc_op[0xff] + cc_ex[0xff];
	}
	else
	{
		/* Interrupt mode 0. We check for CALL and JP instructions, */
		/* if neither of these were found we assume a 1 byte opcode */
		/* was placed on the databus                                */
		LOG(("Z80 '%s' IM0 $%04x\n", tag(), irq_vector));

		/* check for nop */
		if (irq_vector != 0x00)
		{
			switch (irq_vector & 0xff0000)
			{
				case 0xcd0000:  /* call */
					PUSH(m_pc);
					PCD = irq_vector & 0xffff;
						/* CALL $xxxx cycles */
					m_icount -= m_cc_op[0xcd];
					break;
				case 0xc30000:  /* jump */
					PCD = irq_vector & 0xffff;
					/* JP $xxxx cycles */
					m_icount -= m_cc_op[0xc3];
					break;
				default:        /* rst (or other opcodes?) */
					PUSH(m_pc);
					PCD = irq_vector & 0x0038;
					/* RST $xx cycles */
					m_icount -= m_cc_op[0xff];
					break;
			}
		}

		/* 'interrupt latency' cycles */
		m_icount -= m_cc_ex[0xff];
	}
	WZ=PCD;
}

void nsc800_device::take_interrupt_nsc800()
{
	/* there isn't a valid previous program counter */
	PRVPC = -1;

	/* Check if processor was halted */
	LEAVE_HALT();

	/* Clear both interrupt flip flops */
	m_iff1 = m_iff2 = 0;

	if (m_nsc800_irq_state[NSC800_RSTA])
	{
		PUSH(m_pc);
		PCD = 0x003c;
	}
	else if (m_nsc800_irq_state[NSC800_RSTB])
	{
		PUSH(m_pc);
		PCD = 0x0034;
	}
	else if (m_nsc800_irq_state[NSC800_RSTC])
	{
		PUSH(m_pc);
		PCD = 0x002c;
	}

	/* 'interrupt latency' cycles */
	m_icount -= m_cc_op[0xff] + cc_ex[0xff];

	WZ=PCD;
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
void z80_device::device_start()
{
	if( !tables_initialised )
	{
		UINT8 *padd = &SZHVC_add[  0*256];
		UINT8 *padc = &SZHVC_add[256*256];
		UINT8 *psub = &SZHVC_sub[  0*256];
		UINT8 *psbc = &SZHVC_sub[256*256];
		for (int oldval = 0; oldval < 256; oldval++)
		{
			for (int newval = 0; newval < 256; newval++)
			{
				/* add or adc w/o carry set */
				int val = newval - oldval;
				*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padd |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
				if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
				if( newval < oldval ) *padd |= CF;
				if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
				padd++;

				/* adc with carry set */
				val = newval - oldval - 1;
				*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
				*padc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
				if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
				if( newval <= oldval ) *padc |= CF;
				if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
				padc++;

				/* cp, sub or sbc w/o carry set */
				val = oldval - newval;
				*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psub |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
				if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
				if( newval > oldval ) *psub |= CF;
				if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
				psub++;

				/* sbc with carry set */
				val = oldval - newval - 1;
				*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
				*psbc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
				if( (newval & 0x0f) >= (oldval & 0x0f) ) *psbc |= HF;
				if( newval >= oldval ) *psbc |= CF;
				if( (val^oldval) & (oldval^newval) & 0x80 ) *psbc |= VF;
				psbc++;
			}
		}

		for (int i = 0; i < 256; i++)
		{
			int p = 0;
			if( i&0x01 ) ++p;
			if( i&0x02 ) ++p;
			if( i&0x04 ) ++p;
			if( i&0x08 ) ++p;
			if( i&0x10 ) ++p;
			if( i&0x20 ) ++p;
			if( i&0x40 ) ++p;
			if( i&0x80 ) ++p;
			SZ[i] = i ? i & SF : ZF;
			SZ[i] |= (i & (YF | XF));       /* undocumented flag bits 5+3 */
			SZ_BIT[i] = i ? i & SF : ZF | PF;
			SZ_BIT[i] |= (i & (YF | XF));   /* undocumented flag bits 5+3 */
			SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
			SZHV_inc[i] = SZ[i];
			if( i == 0x80 ) SZHV_inc[i] |= VF;
			if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
			SZHV_dec[i] = SZ[i] | NF;
			if( i == 0x7f ) SZHV_dec[i] |= VF;
			if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
		}

		tables_initialised = true;
	}

	save_item(NAME(m_prvpc.w.l));
	save_item(NAME(PC));
	save_item(NAME(SP));
	save_item(NAME(AF));
	save_item(NAME(BC));
	save_item(NAME(DE));
	save_item(NAME(HL));
	save_item(NAME(IX));
	save_item(NAME(IY));
	save_item(NAME(WZ));
	save_item(NAME(m_af2.w.l));
	save_item(NAME(m_bc2.w.l));
	save_item(NAME(m_de2.w.l));
	save_item(NAME(m_hl2.w.l));
	save_item(NAME(m_r));
	save_item(NAME(m_r2));
	save_item(NAME(m_iff1));
	save_item(NAME(m_iff2));
	save_item(NAME(m_halt));
	save_item(NAME(m_im));
	save_item(NAME(m_i));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_wait_state));
	save_item(NAME(m_busrq_state));
	save_item(NAME(m_after_ei));
	save_item(NAME(m_after_ldair));

	/* Reset registers to their initial values */
	PRVPC = 0;
	PCD = 0;
	SPD = 0;
	AFD = 0;
	BCD = 0;
	DED = 0;
	HLD = 0;
	IXD = 0;
	IYD = 0;
	WZ = 0;
	m_af2.d = 0;
	m_bc2.d = 0;
	m_de2.d = 0;
	m_hl2.d = 0;
	m_r = 0;
	m_r2 = 0;
	m_iff1 = 0;
	m_iff2 = 0;
	m_halt = 0;
	m_im = 0;
	m_i = 0;
	m_nmi_state = 0;
	m_nmi_pending = 0;
	m_irq_state = 0;
	m_wait_state = 0;
	m_busrq_state = 0;
	m_after_ei = 0;
	m_after_ldair = 0;
	m_ea = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	if (static_config() != NULL)
		m_daisy.init(this, (const z80_daisy_config *)static_config());
	m_irq_callback = static_standard_irq_callback;

	IX = IY = 0xffff; /* IX and IY are FFFF after a reset! */
	F = ZF;            /* Zero flag is set */

	/* set up the state table */
	state_add(Z80_PC,          "PC",        m_pc.w.l);
	state_add(STATE_GENPC,     "GENPC",     m_pc.w.l).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_prvpc.w.l).noshow();
	state_add(Z80_SP,          "SP",        SP);
	state_add(STATE_GENSP,     "GENSP",     SP).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  F).noshow().formatstr("%8s");
	state_add(Z80_A,           "A",         A).noshow();
	state_add(Z80_B,           "B",         B).noshow();
	state_add(Z80_C,           "C",         C).noshow();
	state_add(Z80_D,           "D",         D).noshow();
	state_add(Z80_E,           "E",         E).noshow();
	state_add(Z80_H,           "H",         H).noshow();
	state_add(Z80_L,           "L",         L).noshow();
	state_add(Z80_AF,          "AF",        AF);
	state_add(Z80_BC,          "BC",        BC);
	state_add(Z80_DE,          "DE",        DE);
	state_add(Z80_HL,          "HL",        HL);
	state_add(Z80_IX,          "IX",        IX);
	state_add(Z80_IY,          "IY",        IY);
	state_add(Z80_AF2,         "AF2",       m_af2.w.l);
	state_add(Z80_BC2,         "BC2",       m_bc2.w.l);
	state_add(Z80_DE2,         "DE2",       m_de2.w.l);
	state_add(Z80_HL2,         "HL2",       m_hl2.w.l);
	state_add(Z80_WZ,          "WZ",        WZ);
	state_add(Z80_R,           "R",         m_rtemp).callimport().callexport();
	state_add(Z80_I,           "I",         m_i);
	state_add(Z80_IM,          "IM",        m_im).mask(0x3);
	state_add(Z80_IFF1,        "IFF1",      m_iff1).mask(0x1);
	state_add(Z80_IFF2,        "IFF2",      m_iff2).mask(0x1);
	state_add(Z80_HALT,        "HALT",      m_halt).mask(0x1);

	// set our instruction counter
	m_icountptr = &m_icount;

	/* setup cycle tables */
	m_cc_op = cc_op;
	m_cc_cb = cc_cb;
	m_cc_ed = cc_ed;
	m_cc_xy = cc_xy;
	m_cc_xycb = cc_xycb;
	m_cc_ex = cc_ex;
}

void nsc800_device::device_start()
{
	z80_device::device_start();
	save_item(NAME(m_nsc800_irq_state));
}

/****************************************************************************
 * Do a reset
 ****************************************************************************/
void z80_device::device_reset()
{
	PC = 0x0000;
	m_i = 0;
	m_r = 0;
	m_r2 = 0;
	m_nmi_pending = FALSE;
	m_after_ei = FALSE;
	m_after_ldair = FALSE;
	m_iff1 = 0;
	m_iff2 = 0;

	m_daisy.reset();

	WZ=PCD;
}

void nsc800_device::device_reset()
{
	z80_device::device_reset();
	memset(m_nsc800_irq_state, 0, sizeof(m_nsc800_irq_state));
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
void z80_device::execute_run()
{
	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (m_nmi_pending)
	{
		LOG(("Z80 '%s' take NMI\n", tag()));
		PRVPC = -1;            /* there isn't a valid previous program counter */
		LEAVE_HALT();            /* Check if processor was halted */

#if HAS_LDAIR_QUIRK
		/* reset parity flag after LD A,I or LD A,R */
		if (m_after_ldair) F &= ~PF;
#endif
		m_after_ldair = FALSE;

		m_iff1 = 0;
		PUSH(m_pc);
		PCD = 0x0066;
		WZ=PCD;
		m_icount -= 11;
		m_nmi_pending = FALSE;
	}

	do
	{
		/* check for IRQs before each instruction */
		if (m_irq_state != CLEAR_LINE && m_iff1 && !m_after_ei)
		{
#if HAS_LDAIR_QUIRK
			/* reset parity flag after LD A,I or LD A,R */
			if (m_after_ldair) F &= ~PF;
#endif
			take_interrupt();
		}
		m_after_ei = FALSE;
		m_after_ldair = FALSE;

		PRVPC = PCD;
		debugger_instruction_hook(this, PCD);
		m_r++;
		EXEC(op,ROP());
	} while (m_icount > 0);
}

void nsc800_device::execute_run()
{
	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (m_nmi_pending)
	{
		LOG(("Z80 '%s' take NMI\n", tag()));
		PRVPC = -1;            /* there isn't a valid previous program counter */
		LEAVE_HALT();            /* Check if processor was halted */

		m_iff1 = 0;
		PUSH(m_pc);
		PCD = 0x0066;
		WZ=PCD;
		m_icount -= 11;
		m_nmi_pending = FALSE;
	}

	do
	{
		/* check for NSC800 IRQs line RSTA, RSTB, RSTC */
		if ((m_nsc800_irq_state[NSC800_RSTA] != CLEAR_LINE || m_nsc800_irq_state[NSC800_RSTB] != CLEAR_LINE || m_nsc800_irq_state[NSC800_RSTC] != CLEAR_LINE) && m_iff1 && !m_after_ei)
			take_interrupt_nsc800();

		/* check for IRQs before each instruction */
		if (m_irq_state != CLEAR_LINE && m_iff1 && !m_after_ei)
			take_interrupt();

		m_after_ei = FALSE;

		PRVPC = PCD;
		debugger_instruction_hook(this, PCD);
		m_r++;
		EXEC(op,ROP());
	} while (m_icount > 0);
}

void z80_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case Z80_INPUT_LINE_BUSRQ:
		m_busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		/* mark an NMI pending on the rising edge */
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			m_nmi_pending = TRUE;
		m_nmi_state = state;
		break;

	case INPUT_LINE_IRQ0:
		/* update the IRQ state via the daisy chain */
		m_irq_state = state;
		if (m_daisy.present())
			m_irq_state = ( m_daisy.update_irq_state() == ASSERT_LINE ) ? ASSERT_LINE : m_irq_state;

		/* the main execute loop will take the interrupt */
		break;

	case Z80_INPUT_LINE_WAIT:
		m_wait_state = state;
		break;
	}
}

void nsc800_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case Z80_INPUT_LINE_BUSRQ:
		m_busrq_state = state;
		break;

	case INPUT_LINE_NMI:
		/* mark an NMI pending on the rising edge */
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			m_nmi_pending = TRUE;
		m_nmi_state = state;
		break;

	case NSC800_RSTA:
		m_nsc800_irq_state[NSC800_RSTA] = state;
		break;

	case NSC800_RSTB:
		m_nsc800_irq_state[NSC800_RSTB] = state;
		break;

	case NSC800_RSTC:
		m_nsc800_irq_state[NSC800_RSTC] = state;
		break;

	case INPUT_LINE_IRQ0:
		/* update the IRQ state via the daisy chain */
		m_irq_state = state;
		if (m_daisy.present())
			m_irq_state = m_daisy.update_irq_state();

		/* the main execute loop will take the interrupt */
		break;

	case Z80_INPUT_LINE_WAIT:
		m_wait_state = state;
		break;
	}
}



/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void z80_device::state_import( const device_state_entry &entry )
{
	switch (entry.index())
	{
		case Z80_R:
			m_r = m_rtemp & 0x7f;
			m_r2 = m_rtemp & 0x80;
			break;

		default:
			fatalerror("CPU_IMPORT_STATE() called for unexpected value\n");
			break;
	}
}


void z80_device::state_export( const device_state_entry &entry )
{
	switch (entry.index())
	{
		case Z80_R:
			m_rtemp = (m_r & 0x7f) | (m_r2 & 0x80);
			break;

		default:
			fatalerror("CPU_EXPORT_STATE() called for unexpected value\n");
			break;
	}
}

void z80_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				F & 0x80 ? 'S':'.',
				F & 0x40 ? 'Z':'.',
				F & 0x20 ? 'Y':'.',
				F & 0x10 ? 'H':'.',
				F & 0x08 ? 'X':'.',
				F & 0x04 ? 'P':'.',
				F & 0x02 ? 'N':'.',
				F & 0x01 ? 'C':'.');
			break;
	}
}

//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t z80_device::disasm_disassemble( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options )
{
	extern CPU_DISASSEMBLE( z80 );
	return CPU_DISASSEMBLE_NAME(z80)(this, buffer, pc, oprom, opram, options);
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

void z80_device::z80_set_cycle_tables(const UINT8 *op, const UINT8 *cb, const UINT8 *ed, const UINT8 *xy, const UINT8 *xycb, const UINT8 *ex)
{
	m_cc_op = (op != NULL) ? op : cc_op;
	m_cc_cb = (cb != NULL) ? cb : cc_cb;
	m_cc_ed = (ed != NULL) ? ed : cc_ed;
	m_cc_xy = (xy != NULL) ? xy : cc_xy;
	m_cc_xycb = (xycb != NULL) ? xycb : cc_xycb;
	m_cc_ex = (ex != NULL) ? ex : cc_ex;
}


z80_device::z80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	cpu_device(mconfig, Z80, "Z80", tag, owner, clock, "z80", __FILE__),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
{
}

const device_type Z80 = &device_creator<z80_device>;

nsc800_device::nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: z80_device(mconfig, NSC800, "NSC800", tag, owner, clock, "nsc800", __FILE__)
{
}

const device_type NSC800 = &device_creator<nsc800_device>;

WRITE_LINE_MEMBER( z80_device::irq_line )
{
	set_input_line( INPUT_LINE_IRQ0, state );
}
