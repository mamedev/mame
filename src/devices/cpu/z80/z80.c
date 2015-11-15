// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   z80.c
 *   Portable Z80 emulator V3.9
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
 *    - Changed variable ea and arg16() function to UINT32; this
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
inline void z80_device::halt()
{
	PC--;
	m_halt = 1;
}

/***************************************************************
 * Leave halt state; write 0 to fake port
 ***************************************************************/
inline void z80_device::leave_halt()
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
inline UINT8 z80_device::in(UINT16 port)
{
	return m_io->read_byte(port);
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
inline void z80_device::out(UINT16 port, UINT8 value)
{
	m_io->write_byte(port, value);
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
inline UINT8 z80_device::rm(UINT16 addr)
{
	return m_program->read_byte(addr);
}

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
inline void z80_device::rm16(UINT16 addr, PAIR &r)
{
	r.b.l = rm(addr);
	r.b.h = rm((addr+1));
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
inline void z80_device::wm(UINT16 addr, UINT8 value)
{
	m_program->write_byte(addr, value);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
inline void z80_device::wm16(UINT16 addr, PAIR &r)
{
	wm(addr, r.b.l);
	wm((addr+1), r.b.h);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
inline UINT8 z80_device::rop()
{
	unsigned pc = PCD;
	PC++;
	return m_decrypted_opcodes_direct->read_byte(pc);
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
inline UINT8 z80_device::arg()
{
	unsigned pc = PCD;
	PC++;
	return m_direct->read_byte(pc);
}

inline UINT16 z80_device::arg16()
{
	unsigned pc = PCD;
	PC += 2;
	return m_direct->read_byte(pc) | (m_direct->read_byte((pc+1)&0xffff) << 8);
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
inline void z80_device::eax()
{
	m_ea = (UINT32)(UINT16)(IX + (INT8)arg());
	WZ = m_ea;
}

inline void z80_device::eay()
{
	m_ea = (UINT32)(UINT16)(IY + (INT8)arg());
	WZ = m_ea;
}

/***************************************************************
 * POP
 ***************************************************************/
inline void z80_device::pop(PAIR &r)
{
	rm16(SPD, r);
	SP += 2;
}

/***************************************************************
 * PUSH
 ***************************************************************/
inline void z80_device::push(PAIR &r)
{
	SP -= 2;
	wm16(SPD, r);
}

/***************************************************************
 * JP
 ***************************************************************/
inline void z80_device::jp(void)
{
	PCD = arg16();
	WZ = PCD;
}

/***************************************************************
 * JP_COND
 ***************************************************************/
inline void z80_device::jp_cond(bool cond)
{
	if (cond)
	{
		PCD = arg16();
		WZ = PCD;
	}
	else
	{
		WZ = arg16(); /* implicit do PC += 2 */
	}
}

/***************************************************************
 * JR
 ***************************************************************/
inline void z80_device::jr()
{
	INT8 a = (INT8)arg();    /* arg() also increments PC */
	PC += a;             /* so don't do PC += arg() */
	WZ = PC;
}

/***************************************************************
 * JR_COND
 ***************************************************************/
inline void z80_device::jr_cond(bool cond, UINT8 opcode)
{
	if (cond)
	{
		jr();
		CC(ex, opcode);
	}
	else
		PC++;
}

/***************************************************************
 * CALL
 ***************************************************************/
inline void z80_device::call()
{
	m_ea = arg16();
	WZ = m_ea;
	push(m_pc);
	PCD = m_ea;
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
inline void z80_device::call_cond(bool cond, UINT8 opcode)
{
	if (cond)
	{
		m_ea = arg16();
		WZ = m_ea;
		push(m_pc);
		PCD = m_ea;
		CC(ex, opcode);
	}
	else
	{
		WZ = arg16();  /* implicit call PC+=2;   */
	}
}

/***************************************************************
 * RET_COND
 ***************************************************************/
inline void z80_device::ret_cond(bool cond, UINT8 opcode)
{
	if (cond)
	{
		pop(m_pc);
		WZ = PC;
		CC(ex, opcode);
	}
}

/***************************************************************
 * RETN
 ***************************************************************/
inline void z80_device::retn()
{
	LOG(("Z80 '%s' RETN m_iff1:%d m_iff2:%d\n",
		tag(), m_iff1, m_iff2));
	pop(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
}

/***************************************************************
 * RETI
 ***************************************************************/
inline void z80_device::reti()
{
	pop(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
	m_daisy.call_reti_device();
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
inline void z80_device::ld_r_a()
{
	m_r = A;
	m_r2 = A & 0x80;            /* keep bit 7 of r */
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
inline void z80_device::ld_a_r()
{
	A = (m_r & 0x7f) | m_r2;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = TRUE;
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
inline void z80_device::ld_i_a()
{
	m_i = A;
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
inline void z80_device::ld_a_i()
{
	A = m_i;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = TRUE;
}

/***************************************************************
 * RST
 ***************************************************************/
inline void z80_device::rst(UINT16 addr)
{
	push(m_pc);
	PCD = addr;
	WZ = PC;
}

/***************************************************************
 * INC  r8
 ***************************************************************/
inline UINT8 z80_device::inc(UINT8 value)
{
	UINT8 res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (UINT8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline UINT8 z80_device::dec(UINT8 value)
{
	UINT8 res = value - 1;
	F = (F & CF) | SZHV_dec[res];
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
inline void z80_device::rlca()
{
	A = (A << 1) | (A >> 7);
	F = (F & (SF | ZF | PF)) | (A & (YF | XF | CF));
}

/***************************************************************
 * RRCA
 ***************************************************************/
inline void z80_device::rrca()
{
	F = (F & (SF | ZF | PF)) | (A & CF);
	A = (A >> 1) | (A << 7);
	F |= (A & (YF | XF));
}

/***************************************************************
 * RLA
 ***************************************************************/
inline void z80_device::rla()
{
	UINT8 res = (A << 1) | (F & CF);
	UINT8 c = (A & 0x80) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void z80_device::rra()
{
	UINT8 res = (A >> 1) | (F << 7);
	UINT8 c = (A & 0x01) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
inline void z80_device::rrd()
{
	UINT8 n = rm(HL);
	WZ = HL+1;
	wm(HL, (n >> 4) | (A << 4));
	A = (A & 0xf0) | (n & 0x0f);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * RLD
 ***************************************************************/
inline void z80_device::rld()
{
	UINT8 n = rm(HL);
	WZ = HL+1;
	wm(HL, (n << 4) | (A & 0x0f));
	A = (A & 0xf0) | (n >> 4);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void z80_device::add_a(UINT8 value)
{
	UINT32 ah = AFD & 0xff00;
	UINT32 res = (UINT8)((ah >> 8) + value);
	F = SZHVC_add[ah | res];
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void z80_device::adc_a(UINT8 value)
{
	UINT32 ah = AFD & 0xff00, c = AFD & 1;
	UINT32 res = (UINT8)((ah >> 8) + value + c);
	F = SZHVC_add[(c << 16) | ah | res];
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void z80_device::sub(UINT8 value)
{
	UINT32 ah = AFD & 0xff00;
	UINT32 res = (UINT8)((ah >> 8) - value);
	F = SZHVC_sub[ah | res];
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void z80_device::sbc_a(UINT8 value)
{
	UINT32 ah = AFD & 0xff00, c = AFD & 1;
	UINT32 res = (UINT8)((ah >> 8) - value - c);
	F = SZHVC_sub[(c<<16) | ah | res];
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
inline void z80_device::neg()
{
	UINT8 value = A;
	A = 0;
	sub(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
inline void z80_device::daa()
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
inline void z80_device::and_a(UINT8 value)
{
	A &= value;
	F = SZP[A] | HF;
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void z80_device::or_a(UINT8 value)
{
	A |= value;
	F = SZP[A];
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void z80_device::xor_a(UINT8 value)
{
	A ^= value;
	F = SZP[A];
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void z80_device::cp(UINT8 value)
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
inline void z80_device::ex_af()
{
	PAIR tmp;
	tmp = m_af; m_af = m_af2; m_af2 = tmp;
}

/***************************************************************
 * EX   DE,HL
 ***************************************************************/
inline void z80_device::ex_de_hl()
{
	PAIR tmp;
	tmp = m_de; m_de = m_hl; m_hl = tmp;
}

/***************************************************************
 * EXX
 ***************************************************************/
inline void z80_device::exx()
{
	PAIR tmp;
	tmp = m_bc; m_bc = m_bc2; m_bc2 = tmp;
	tmp = m_de; m_de = m_de2; m_de2 = tmp;
	tmp = m_hl; m_hl = m_hl2; m_hl2 = tmp;
}

/***************************************************************
 * EX   (SP),r16
 ***************************************************************/
inline void z80_device::ex_sp(PAIR &r)
{
	PAIR tmp = { { 0, 0, 0, 0 } };
	rm16(SPD, tmp);
	wm16(SPD, r);
	r = tmp;
	WZ = r.d;
}

/***************************************************************
 * ADD16
 ***************************************************************/
inline void z80_device::add16(PAIR &dr, PAIR &sr)
{
	UINT32 res = dr.d + sr.d;
	WZ = dr.d + 1;
	F = (F & (SF | ZF | VF)) |
		(((dr.d ^ res ^ sr.d) >> 8) & HF) |
		((res >> 16) & CF) | ((res >> 8) & (YF | XF));
	dr.w.l = (UINT16)res;
}

/***************************************************************
 * ADC  HL,r16
 ***************************************************************/
inline void z80_device::adc_hl(PAIR &r)
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
 * SBC  HL,r16
 ***************************************************************/
inline void z80_device::sbc_hl(PAIR &r)
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
inline UINT8 z80_device::rlc(UINT8 value)
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
inline UINT8 z80_device::rrc(UINT8 value)
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
inline UINT8 z80_device::rl(UINT8 value)
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
inline UINT8 z80_device::rr(UINT8 value)
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
inline UINT8 z80_device::sla(UINT8 value)
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
inline UINT8 z80_device::sra(UINT8 value)
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
inline UINT8 z80_device::sll(UINT8 value)
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
inline UINT8 z80_device::srl(UINT8 value)
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
inline void z80_device::bit(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (value & (YF|XF));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void z80_device::bit_hl(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void z80_device::bit_xy(int bit, UINT8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | ((m_ea>>8) & (YF|XF));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
inline UINT8 z80_device::res(int bit, UINT8 value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
inline UINT8 z80_device::set(int bit, UINT8 value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
inline void z80_device::ldi()
{
	UINT8 io = rm(HL);
	wm(DE, io);
	F &= SF | ZF | CF;
	if ((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */
	HL++; DE++; BC--;
	if(BC) F |= VF;
}

/***************************************************************
 * CPI
 ***************************************************************/
inline void z80_device::cpi()
{
	UINT8 val = rm(HL);
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
inline void z80_device::ini()
{
	unsigned t;
	UINT8 io = in(BC);
	WZ = BC + 1;
	B--;
	wm(HL, io);
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
inline void z80_device::outi()
{
	unsigned t;
	UINT8 io = rm(HL);
	B--;
	WZ = BC + 1;
	out(BC, io);
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
inline void z80_device::ldd()
{
	UINT8 io = rm(HL);
	wm(DE, io);
	F &= SF | ZF | CF;
	if ((A + io) & 0x02) F |= YF; /* bit 1 -> flag 5 */
	if ((A + io) & 0x08) F |= XF; /* bit 3 -> flag 3 */
	HL--; DE--; BC--;
	if (BC) F |= VF;
}

/***************************************************************
 * CPD
 ***************************************************************/
inline void z80_device::cpd()
{
	UINT8 val = rm(HL);
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
inline void z80_device::ind()
{
	unsigned t;
	UINT8 io = in(BC);
	WZ = BC - 1;
	B--;
	wm(HL, io);
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
inline void z80_device::outd()
{
	unsigned t;
	UINT8 io = rm(HL);
	B--;
	WZ = BC - 1;
	out(BC, io);
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
inline void z80_device::ldir()
{
	ldi();
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
inline void z80_device::cpir()
{
	cpi();
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
inline void z80_device::inir()
{
	ini();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xb2);
	}
}

/***************************************************************
 * OTIR
 ***************************************************************/
inline void z80_device::otir()
{
	outi();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xb3);
	}
}

/***************************************************************
 * LDDR
 ***************************************************************/
inline void z80_device::lddr()
{
	ldd();
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
inline void z80_device::cpdr()
{
	cpd();
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
inline void z80_device::indr()
{
	ind();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xba);
	}
}

/***************************************************************
 * OTDR
 ***************************************************************/
inline void z80_device::otdr()
{
	outd();
	if (B != 0)
	{
		PC -= 2;
		CC(ex, 0xbb);
	}
}

/***************************************************************
 * EI
 ***************************************************************/
inline void z80_device::ei()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = TRUE;
}

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { B = rlc(B);             } /* RLC  B           */
OP(cb,01) { C = rlc(C);             } /* RLC  C           */
OP(cb,02) { D = rlc(D);             } /* RLC  D           */
OP(cb,03) { E = rlc(E);             } /* RLC  E           */
OP(cb,04) { H = rlc(H);             } /* RLC  H           */
OP(cb,05) { L = rlc(L);             } /* RLC  L           */
OP(cb,06) { wm(HL, rlc(rm(HL)));    } /* RLC  (HL)        */
OP(cb,07) { A = rlc(A);             } /* RLC  A           */

OP(cb,08) { B = rrc(B);             } /* RRC  B           */
OP(cb,09) { C = rrc(C);             } /* RRC  C           */
OP(cb,0a) { D = rrc(D);             } /* RRC  D           */
OP(cb,0b) { E = rrc(E);             } /* RRC  E           */
OP(cb,0c) { H = rrc(H);             } /* RRC  H           */
OP(cb,0d) { L = rrc(L);             } /* RRC  L           */
OP(cb,0e) { wm(HL, rrc(rm(HL)));    } /* RRC  (HL)        */
OP(cb,0f) { A = rrc(A);             } /* RRC  A           */

OP(cb,10) { B = rl(B);              } /* RL   B           */
OP(cb,11) { C = rl(C);              } /* RL   C           */
OP(cb,12) { D = rl(D);              } /* RL   D           */
OP(cb,13) { E = rl(E);              } /* RL   E           */
OP(cb,14) { H = rl(H);              } /* RL   H           */
OP(cb,15) { L = rl(L);              } /* RL   L           */
OP(cb,16) { wm(HL, rl(rm(HL)));     } /* RL   (HL)        */
OP(cb,17) { A = rl(A);              } /* RL   A           */

OP(cb,18) { B = rr(B);              } /* RR   B           */
OP(cb,19) { C = rr(C);              } /* RR   C           */
OP(cb,1a) { D = rr(D);              } /* RR   D           */
OP(cb,1b) { E = rr(E);              } /* RR   E           */
OP(cb,1c) { H = rr(H);              } /* RR   H           */
OP(cb,1d) { L = rr(L);              } /* RR   L           */
OP(cb,1e) { wm(HL, rr(rm(HL)));     } /* RR   (HL)        */
OP(cb,1f) { A = rr(A);              } /* RR   A           */

OP(cb,20) { B = sla(B);             } /* SLA  B           */
OP(cb,21) { C = sla(C);             } /* SLA  C           */
OP(cb,22) { D = sla(D);             } /* SLA  D           */
OP(cb,23) { E = sla(E);             } /* SLA  E           */
OP(cb,24) { H = sla(H);             } /* SLA  H           */
OP(cb,25) { L = sla(L);             } /* SLA  L           */
OP(cb,26) { wm(HL, sla(rm(HL)));    } /* SLA  (HL)        */
OP(cb,27) { A = sla(A);             } /* SLA  A           */

OP(cb,28) { B = sra(B);             } /* SRA  B           */
OP(cb,29) { C = sra(C);             } /* SRA  C           */
OP(cb,2a) { D = sra(D);             } /* SRA  D           */
OP(cb,2b) { E = sra(E);             } /* SRA  E           */
OP(cb,2c) { H = sra(H);             } /* SRA  H           */
OP(cb,2d) { L = sra(L);             } /* SRA  L           */
OP(cb,2e) { wm(HL, sra(rm(HL)));    } /* SRA  (HL)        */
OP(cb,2f) { A = sra(A);             } /* SRA  A           */

OP(cb,30) { B = sll(B);             } /* SLL  B           */
OP(cb,31) { C = sll(C);             } /* SLL  C           */
OP(cb,32) { D = sll(D);             } /* SLL  D           */
OP(cb,33) { E = sll(E);             } /* SLL  E           */
OP(cb,34) { H = sll(H);             } /* SLL  H           */
OP(cb,35) { L = sll(L);             } /* SLL  L           */
OP(cb,36) { wm(HL, sll(rm(HL)));    } /* SLL  (HL)        */
OP(cb,37) { A = sll(A);             } /* SLL  A           */

OP(cb,38) { B = srl(B);             } /* SRL  B           */
OP(cb,39) { C = srl(C);             } /* SRL  C           */
OP(cb,3a) { D = srl(D);             } /* SRL  D           */
OP(cb,3b) { E = srl(E);             } /* SRL  E           */
OP(cb,3c) { H = srl(H);             } /* SRL  H           */
OP(cb,3d) { L = srl(L);             } /* SRL  L           */
OP(cb,3e) { wm(HL, srl(rm(HL)));    } /* SRL  (HL)        */
OP(cb,3f) { A = srl(A);             } /* SRL  A           */

OP(cb,40) { bit(0, B);              } /* BIT  0,B         */
OP(cb,41) { bit(0, C);              } /* BIT  0,C         */
OP(cb,42) { bit(0, D);              } /* BIT  0,D         */
OP(cb,43) { bit(0, E);              } /* BIT  0,E         */
OP(cb,44) { bit(0, H);              } /* BIT  0,H         */
OP(cb,45) { bit(0, L);              } /* BIT  0,L         */
OP(cb,46) { bit_hl(0, rm(HL));      } /* BIT  0,(HL)      */
OP(cb,47) { bit(0, A);              } /* BIT  0,A         */

OP(cb,48) { bit(1, B);              } /* BIT  1,B         */
OP(cb,49) { bit(1, C);              } /* BIT  1,C         */
OP(cb,4a) { bit(1, D);              } /* BIT  1,D         */
OP(cb,4b) { bit(1, E);              } /* BIT  1,E         */
OP(cb,4c) { bit(1, H);              } /* BIT  1,H         */
OP(cb,4d) { bit(1, L);              } /* BIT  1,L         */
OP(cb,4e) { bit_hl(1, rm(HL));      } /* BIT  1,(HL)      */
OP(cb,4f) { bit(1, A);              } /* BIT  1,A         */

OP(cb,50) { bit(2, B);              } /* BIT  2,B         */
OP(cb,51) { bit(2, C);              } /* BIT  2,C         */
OP(cb,52) { bit(2, D);              } /* BIT  2,D         */
OP(cb,53) { bit(2, E);              } /* BIT  2,E         */
OP(cb,54) { bit(2, H);              } /* BIT  2,H         */
OP(cb,55) { bit(2, L);              } /* BIT  2,L         */
OP(cb,56) { bit_hl(2, rm(HL));      } /* BIT  2,(HL)      */
OP(cb,57) { bit(2, A);              } /* BIT  2,A         */

OP(cb,58) { bit(3, B);              } /* BIT  3,B         */
OP(cb,59) { bit(3, C);              } /* BIT  3,C         */
OP(cb,5a) { bit(3, D);              } /* BIT  3,D         */
OP(cb,5b) { bit(3, E);              } /* BIT  3,E         */
OP(cb,5c) { bit(3, H);              } /* BIT  3,H         */
OP(cb,5d) { bit(3, L);              } /* BIT  3,L         */
OP(cb,5e) { bit_hl(3, rm(HL));      } /* BIT  3,(HL)      */
OP(cb,5f) { bit(3, A);              } /* BIT  3,A         */

OP(cb,60) { bit(4, B);              } /* BIT  4,B         */
OP(cb,61) { bit(4, C);              } /* BIT  4,C         */
OP(cb,62) { bit(4, D);              } /* BIT  4,D         */
OP(cb,63) { bit(4, E);              } /* BIT  4,E         */
OP(cb,64) { bit(4, H);              } /* BIT  4,H         */
OP(cb,65) { bit(4, L);              } /* BIT  4,L         */
OP(cb,66) { bit_hl(4, rm(HL));      } /* BIT  4,(HL)      */
OP(cb,67) { bit(4, A);              } /* BIT  4,A         */

OP(cb,68) { bit(5, B);              } /* BIT  5,B         */
OP(cb,69) { bit(5, C);              } /* BIT  5,C         */
OP(cb,6a) { bit(5, D);              } /* BIT  5,D         */
OP(cb,6b) { bit(5, E);              } /* BIT  5,E         */
OP(cb,6c) { bit(5, H);              } /* BIT  5,H         */
OP(cb,6d) { bit(5, L);              } /* BIT  5,L         */
OP(cb,6e) { bit_hl(5, rm(HL));      } /* BIT  5,(HL)      */
OP(cb,6f) { bit(5, A);              } /* BIT  5,A         */

OP(cb,70) { bit(6, B);              } /* BIT  6,B         */
OP(cb,71) { bit(6, C);              } /* BIT  6,C         */
OP(cb,72) { bit(6, D);              } /* BIT  6,D         */
OP(cb,73) { bit(6, E);              } /* BIT  6,E         */
OP(cb,74) { bit(6, H);              } /* BIT  6,H         */
OP(cb,75) { bit(6, L);              } /* BIT  6,L         */
OP(cb,76) { bit_hl(6, rm(HL));      } /* BIT  6,(HL)      */
OP(cb,77) { bit(6, A);              } /* BIT  6,A         */

OP(cb,78) { bit(7, B);              } /* BIT  7,B         */
OP(cb,79) { bit(7, C);              } /* BIT  7,C         */
OP(cb,7a) { bit(7, D);              } /* BIT  7,D         */
OP(cb,7b) { bit(7, E);              } /* BIT  7,E         */
OP(cb,7c) { bit(7, H);              } /* BIT  7,H         */
OP(cb,7d) { bit(7, L);              } /* BIT  7,L         */
OP(cb,7e) { bit_hl(7, rm(HL));      } /* BIT  7,(HL)      */
OP(cb,7f) { bit(7, A);              } /* BIT  7,A         */

OP(cb,80) { B = res(0, B);          } /* RES  0,B         */
OP(cb,81) { C = res(0, C);          } /* RES  0,C         */
OP(cb,82) { D = res(0, D);          } /* RES  0,D         */
OP(cb,83) { E = res(0, E);          } /* RES  0,E         */
OP(cb,84) { H = res(0, H);          } /* RES  0,H         */
OP(cb,85) { L = res(0, L);          } /* RES  0,L         */
OP(cb,86) { wm(HL, res(0, rm(HL))); } /* RES  0,(HL)      */
OP(cb,87) { A = res(0, A);          } /* RES  0,A         */

OP(cb,88) { B = res(1, B);          } /* RES  1,B         */
OP(cb,89) { C = res(1, C);          } /* RES  1,C         */
OP(cb,8a) { D = res(1, D);          } /* RES  1,D         */
OP(cb,8b) { E = res(1, E);          } /* RES  1,E         */
OP(cb,8c) { H = res(1, H);          } /* RES  1,H         */
OP(cb,8d) { L = res(1, L);          } /* RES  1,L         */
OP(cb,8e) { wm(HL, res(1, rm(HL))); } /* RES  1,(HL)      */
OP(cb,8f) { A = res(1, A);          } /* RES  1,A         */

OP(cb,90) { B = res(2, B);          } /* RES  2,B         */
OP(cb,91) { C = res(2, C);          } /* RES  2,C         */
OP(cb,92) { D = res(2, D);          } /* RES  2,D         */
OP(cb,93) { E = res(2, E);          } /* RES  2,E         */
OP(cb,94) { H = res(2, H);          } /* RES  2,H         */
OP(cb,95) { L = res(2, L);          } /* RES  2,L         */
OP(cb,96) { wm(HL, res(2, rm(HL))); } /* RES  2,(HL)      */
OP(cb,97) { A = res(2, A);          } /* RES  2,A         */

OP(cb,98) { B = res(3, B);          } /* RES  3,B         */
OP(cb,99) { C = res(3, C);          } /* RES  3,C         */
OP(cb,9a) { D = res(3, D);          } /* RES  3,D         */
OP(cb,9b) { E = res(3, E);          } /* RES  3,E         */
OP(cb,9c) { H = res(3, H);          } /* RES  3,H         */
OP(cb,9d) { L = res(3, L);          } /* RES  3,L         */
OP(cb,9e) { wm(HL, res(3, rm(HL))); } /* RES  3,(HL)      */
OP(cb,9f) { A = res(3, A);          } /* RES  3,A         */

OP(cb,a0) { B = res(4, B);          } /* RES  4,B         */
OP(cb,a1) { C = res(4, C);          } /* RES  4,C         */
OP(cb,a2) { D = res(4, D);          } /* RES  4,D         */
OP(cb,a3) { E = res(4, E);          } /* RES  4,E         */
OP(cb,a4) { H = res(4, H);          } /* RES  4,H         */
OP(cb,a5) { L = res(4, L);          } /* RES  4,L         */
OP(cb,a6) { wm(HL, res(4, rm(HL))); } /* RES  4,(HL)      */
OP(cb,a7) { A = res(4, A);          } /* RES  4,A         */

OP(cb,a8) { B = res(5, B);          } /* RES  5,B         */
OP(cb,a9) { C = res(5, C);          } /* RES  5,C         */
OP(cb,aa) { D = res(5, D);          } /* RES  5,D         */
OP(cb,ab) { E = res(5, E);          } /* RES  5,E         */
OP(cb,ac) { H = res(5, H);          } /* RES  5,H         */
OP(cb,ad) { L = res(5, L);          } /* RES  5,L         */
OP(cb,ae) { wm(HL, res(5, rm(HL))); } /* RES  5,(HL)      */
OP(cb,af) { A = res(5, A);          } /* RES  5,A         */

OP(cb,b0) { B = res(6, B);          } /* RES  6,B         */
OP(cb,b1) { C = res(6, C);          } /* RES  6,C         */
OP(cb,b2) { D = res(6, D);          } /* RES  6,D         */
OP(cb,b3) { E = res(6, E);          } /* RES  6,E         */
OP(cb,b4) { H = res(6, H);          } /* RES  6,H         */
OP(cb,b5) { L = res(6, L);          } /* RES  6,L         */
OP(cb,b6) { wm(HL, res(6, rm(HL))); } /* RES  6,(HL)      */
OP(cb,b7) { A = res(6, A);          } /* RES  6,A         */

OP(cb,b8) { B = res(7, B);          } /* RES  7,B         */
OP(cb,b9) { C = res(7, C);          } /* RES  7,C         */
OP(cb,ba) { D = res(7, D);          } /* RES  7,D         */
OP(cb,bb) { E = res(7, E);          } /* RES  7,E         */
OP(cb,bc) { H = res(7, H);          } /* RES  7,H         */
OP(cb,bd) { L = res(7, L);          } /* RES  7,L         */
OP(cb,be) { wm(HL, res(7, rm(HL))); } /* RES  7,(HL)      */
OP(cb,bf) { A = res(7, A);          } /* RES  7,A         */

OP(cb,c0) { B = set(0, B);          } /* SET  0,B         */
OP(cb,c1) { C = set(0, C);          } /* SET  0,C         */
OP(cb,c2) { D = set(0, D);          } /* SET  0,D         */
OP(cb,c3) { E = set(0, E);          } /* SET  0,E         */
OP(cb,c4) { H = set(0, H);          } /* SET  0,H         */
OP(cb,c5) { L = set(0, L);          } /* SET  0,L         */
OP(cb,c6) { wm(HL, set(0, rm(HL))); } /* SET  0,(HL)      */
OP(cb,c7) { A = set(0, A);          } /* SET  0,A         */

OP(cb,c8) { B = set(1, B);          } /* SET  1,B         */
OP(cb,c9) { C = set(1, C);          } /* SET  1,C         */
OP(cb,ca) { D = set(1, D);          } /* SET  1,D         */
OP(cb,cb) { E = set(1, E);          } /* SET  1,E         */
OP(cb,cc) { H = set(1, H);          } /* SET  1,H         */
OP(cb,cd) { L = set(1, L);          } /* SET  1,L         */
OP(cb,ce) { wm(HL, set(1, rm(HL))); } /* SET  1,(HL)      */
OP(cb,cf) { A = set(1, A);          } /* SET  1,A         */

OP(cb,d0) { B = set(2, B);          } /* SET  2,B         */
OP(cb,d1) { C = set(2, C);          } /* SET  2,C         */
OP(cb,d2) { D = set(2, D);          } /* SET  2,D         */
OP(cb,d3) { E = set(2, E);          } /* SET  2,E         */
OP(cb,d4) { H = set(2, H);          } /* SET  2,H         */
OP(cb,d5) { L = set(2, L);          } /* SET  2,L         */
OP(cb,d6) { wm(HL, set(2, rm(HL))); } /* SET  2,(HL)      */
OP(cb,d7) { A = set(2, A);          } /* SET  2,A         */

OP(cb,d8) { B = set(3, B);          } /* SET  3,B         */
OP(cb,d9) { C = set(3, C);          } /* SET  3,C         */
OP(cb,da) { D = set(3, D);          } /* SET  3,D         */
OP(cb,db) { E = set(3, E);          } /* SET  3,E         */
OP(cb,dc) { H = set(3, H);          } /* SET  3,H         */
OP(cb,dd) { L = set(3, L);          } /* SET  3,L         */
OP(cb,de) { wm(HL, set(3, rm(HL))); } /* SET  3,(HL)      */
OP(cb,df) { A = set(3, A);          } /* SET  3,A         */

OP(cb,e0) { B = set(4, B);          } /* SET  4,B         */
OP(cb,e1) { C = set(4, C);          } /* SET  4,C         */
OP(cb,e2) { D = set(4, D);          } /* SET  4,D         */
OP(cb,e3) { E = set(4, E);          } /* SET  4,E         */
OP(cb,e4) { H = set(4, H);          } /* SET  4,H         */
OP(cb,e5) { L = set(4, L);          } /* SET  4,L         */
OP(cb,e6) { wm(HL, set(4, rm(HL))); } /* SET  4,(HL)      */
OP(cb,e7) { A = set(4, A);          } /* SET  4,A         */

OP(cb,e8) { B = set(5, B);          } /* SET  5,B         */
OP(cb,e9) { C = set(5, C);          } /* SET  5,C         */
OP(cb,ea) { D = set(5, D);          } /* SET  5,D         */
OP(cb,eb) { E = set(5, E);          } /* SET  5,E         */
OP(cb,ec) { H = set(5, H);          } /* SET  5,H         */
OP(cb,ed) { L = set(5, L);          } /* SET  5,L         */
OP(cb,ee) { wm(HL, set(5, rm(HL))); } /* SET  5,(HL)      */
OP(cb,ef) { A = set(5, A);          } /* SET  5,A         */

OP(cb,f0) { B = set(6, B);          } /* SET  6,B         */
OP(cb,f1) { C = set(6, C);          } /* SET  6,C         */
OP(cb,f2) { D = set(6, D);          } /* SET  6,D         */
OP(cb,f3) { E = set(6, E);          } /* SET  6,E         */
OP(cb,f4) { H = set(6, H);          } /* SET  6,H         */
OP(cb,f5) { L = set(6, L);          } /* SET  6,L         */
OP(cb,f6) { wm(HL, set(6, rm(HL))); } /* SET  6,(HL)      */
OP(cb,f7) { A = set(6, A);          } /* SET  6,A         */

OP(cb,f8) { B = set(7, B);          } /* SET  7,B         */
OP(cb,f9) { C = set(7, C);          } /* SET  7,C         */
OP(cb,fa) { D = set(7, D);          } /* SET  7,D         */
OP(cb,fb) { E = set(7, E);          } /* SET  7,E         */
OP(cb,fc) { H = set(7, H);          } /* SET  7,H         */
OP(cb,fd) { L = set(7, L);          } /* SET  7,L         */
OP(cb,fe) { wm(HL, set(7, rm(HL))); } /* SET  7,(HL)      */
OP(cb,ff) { A = set(7, A);          } /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
OP(xycb,00) { B = rlc(rm(m_ea)); wm(m_ea, B);    } /* RLC  B=(XY+o)    */
OP(xycb,01) { C = rlc(rm(m_ea)); wm(m_ea, C);    } /* RLC  C=(XY+o)    */
OP(xycb,02) { D = rlc(rm(m_ea)); wm(m_ea, D);    } /* RLC  D=(XY+o)    */
OP(xycb,03) { E = rlc(rm(m_ea)); wm(m_ea, E);    } /* RLC  E=(XY+o)    */
OP(xycb,04) { H = rlc(rm(m_ea)); wm(m_ea, H);    } /* RLC  H=(XY+o)    */
OP(xycb,05) { L = rlc(rm(m_ea)); wm(m_ea, L);    } /* RLC  L=(XY+o)    */
OP(xycb,06) { wm(m_ea, rlc(rm(m_ea)));           } /* RLC  (XY+o)      */
OP(xycb,07) { A = rlc(rm(m_ea)); wm(m_ea, A);    } /* RLC  A=(XY+o)    */

OP(xycb,08) { B = rrc(rm(m_ea)); wm(m_ea, B);    } /* RRC  B=(XY+o)    */
OP(xycb,09) { C = rrc(rm(m_ea)); wm(m_ea, C);    } /* RRC  C=(XY+o)    */
OP(xycb,0a) { D = rrc(rm(m_ea)); wm(m_ea, D);    } /* RRC  D=(XY+o)    */
OP(xycb,0b) { E = rrc(rm(m_ea)); wm(m_ea, E);    } /* RRC  E=(XY+o)    */
OP(xycb,0c) { H = rrc(rm(m_ea)); wm(m_ea, H);    } /* RRC  H=(XY+o)    */
OP(xycb,0d) { L = rrc(rm(m_ea)); wm(m_ea, L);    } /* RRC  L=(XY+o)    */
OP(xycb,0e) { wm(m_ea,rrc(rm(m_ea)));            } /* RRC  (XY+o)      */
OP(xycb,0f) { A = rrc(rm(m_ea)); wm(m_ea, A);    } /* RRC  A=(XY+o)    */

OP(xycb,10) { B = rl(rm(m_ea)); wm(m_ea, B);     } /* RL   B=(XY+o)    */
OP(xycb,11) { C = rl(rm(m_ea)); wm(m_ea, C);     } /* RL   C=(XY+o)    */
OP(xycb,12) { D = rl(rm(m_ea)); wm(m_ea, D);     } /* RL   D=(XY+o)    */
OP(xycb,13) { E = rl(rm(m_ea)); wm(m_ea, E);     } /* RL   E=(XY+o)    */
OP(xycb,14) { H = rl(rm(m_ea)); wm(m_ea, H);     } /* RL   H=(XY+o)    */
OP(xycb,15) { L = rl(rm(m_ea)); wm(m_ea, L);     } /* RL   L=(XY+o)    */
OP(xycb,16) { wm(m_ea,rl(rm(m_ea)));             } /* RL   (XY+o)      */
OP(xycb,17) { A = rl(rm(m_ea)); wm(m_ea, A);     } /* RL   A=(XY+o)    */

OP(xycb,18) { B = rr(rm(m_ea)); wm(m_ea, B);     } /* RR   B=(XY+o)    */
OP(xycb,19) { C = rr(rm(m_ea)); wm(m_ea, C);     } /* RR   C=(XY+o)    */
OP(xycb,1a) { D = rr(rm(m_ea)); wm(m_ea, D);     } /* RR   D=(XY+o)    */
OP(xycb,1b) { E = rr(rm(m_ea)); wm(m_ea, E);     } /* RR   E=(XY+o)    */
OP(xycb,1c) { H = rr(rm(m_ea)); wm(m_ea, H);     } /* RR   H=(XY+o)    */
OP(xycb,1d) { L = rr(rm(m_ea)); wm(m_ea, L);     } /* RR   L=(XY+o)    */
OP(xycb,1e) { wm(m_ea, rr(rm(m_ea)));            } /* RR   (XY+o)      */
OP(xycb,1f) { A = rr(rm(m_ea)); wm(m_ea, A);     } /* RR   A=(XY+o)    */

OP(xycb,20) { B = sla(rm(m_ea)); wm(m_ea, B);    } /* SLA  B=(XY+o)    */
OP(xycb,21) { C = sla(rm(m_ea)); wm(m_ea, C);    } /* SLA  C=(XY+o)    */
OP(xycb,22) { D = sla(rm(m_ea)); wm(m_ea, D);    } /* SLA  D=(XY+o)    */
OP(xycb,23) { E = sla(rm(m_ea)); wm(m_ea, E);    } /* SLA  E=(XY+o)    */
OP(xycb,24) { H = sla(rm(m_ea)); wm(m_ea, H);    } /* SLA  H=(XY+o)    */
OP(xycb,25) { L = sla(rm(m_ea)); wm(m_ea, L);    } /* SLA  L=(XY+o)    */
OP(xycb,26) { wm(m_ea, sla(rm(m_ea)));           } /* SLA  (XY+o)      */
OP(xycb,27) { A = sla(rm(m_ea)); wm(m_ea, A);    } /* SLA  A=(XY+o)    */

OP(xycb,28) { B = sra(rm(m_ea)); wm(m_ea, B);    } /* SRA  B=(XY+o)    */
OP(xycb,29) { C = sra(rm(m_ea)); wm(m_ea, C);    } /* SRA  C=(XY+o)    */
OP(xycb,2a) { D = sra(rm(m_ea)); wm(m_ea, D);    } /* SRA  D=(XY+o)    */
OP(xycb,2b) { E = sra(rm(m_ea)); wm(m_ea, E);    } /* SRA  E=(XY+o)    */
OP(xycb,2c) { H = sra(rm(m_ea)); wm(m_ea, H);    } /* SRA  H=(XY+o)    */
OP(xycb,2d) { L = sra(rm(m_ea)); wm(m_ea, L);    } /* SRA  L=(XY+o)    */
OP(xycb,2e) { wm(m_ea, sra(rm(m_ea)));           } /* SRA  (XY+o)      */
OP(xycb,2f) { A = sra(rm(m_ea)); wm(m_ea, A);    } /* SRA  A=(XY+o)    */

OP(xycb,30) { B = sll(rm(m_ea)); wm(m_ea, B);    } /* SLL  B=(XY+o)    */
OP(xycb,31) { C = sll(rm(m_ea)); wm(m_ea, C);    } /* SLL  C=(XY+o)    */
OP(xycb,32) { D = sll(rm(m_ea)); wm(m_ea, D);    } /* SLL  D=(XY+o)    */
OP(xycb,33) { E = sll(rm(m_ea)); wm(m_ea, E);    } /* SLL  E=(XY+o)    */
OP(xycb,34) { H = sll(rm(m_ea)); wm(m_ea, H);    } /* SLL  H=(XY+o)    */
OP(xycb,35) { L = sll(rm(m_ea)); wm(m_ea, L);    } /* SLL  L=(XY+o)    */
OP(xycb,36) { wm(m_ea, sll(rm(m_ea)));           } /* SLL  (XY+o)      */
OP(xycb,37) { A = sll(rm(m_ea)); wm(m_ea, A);    } /* SLL  A=(XY+o)    */

OP(xycb,38) { B = srl(rm(m_ea)); wm(m_ea, B);    } /* SRL  B=(XY+o)    */
OP(xycb,39) { C = srl(rm(m_ea)); wm(m_ea, C);    } /* SRL  C=(XY+o)    */
OP(xycb,3a) { D = srl(rm(m_ea)); wm(m_ea, D);    } /* SRL  D=(XY+o)    */
OP(xycb,3b) { E = srl(rm(m_ea)); wm(m_ea, E);    } /* SRL  E=(XY+o)    */
OP(xycb,3c) { H = srl(rm(m_ea)); wm(m_ea, H);    } /* SRL  H=(XY+o)    */
OP(xycb,3d) { L = srl(rm(m_ea)); wm(m_ea, L);    } /* SRL  L=(XY+o)    */
OP(xycb,3e) { wm(m_ea, srl(rm(m_ea)));           } /* SRL  (XY+o)      */
OP(xycb,3f) { A = srl(rm(m_ea)); wm(m_ea, A);    } /* SRL  A=(XY+o)    */

OP(xycb,40) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,41) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,42) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,43) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,44) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,45) { xycb_46();                         } /* BIT  0,(XY+o)    */
OP(xycb,46) { bit_xy(0, rm(m_ea));               } /* BIT  0,(XY+o)    */
OP(xycb,47) { xycb_46();                         } /* BIT  0,(XY+o)    */

OP(xycb,48) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,49) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4a) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4b) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4c) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4d) { xycb_4e();                         } /* BIT  1,(XY+o)    */
OP(xycb,4e) { bit_xy(1, rm(m_ea));               } /* BIT  1,(XY+o)    */
OP(xycb,4f) { xycb_4e();                         } /* BIT  1,(XY+o)    */

OP(xycb,50) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,51) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,52) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,53) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,54) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,55) { xycb_56();                         } /* BIT  2,(XY+o)    */
OP(xycb,56) { bit_xy(2, rm(m_ea));               } /* BIT  2,(XY+o)    */
OP(xycb,57) { xycb_56();                         } /* BIT  2,(XY+o)    */

OP(xycb,58) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,59) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5a) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5b) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5c) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5d) { xycb_5e();                         } /* BIT  3,(XY+o)    */
OP(xycb,5e) { bit_xy(3, rm(m_ea));               } /* BIT  3,(XY+o)    */
OP(xycb,5f) { xycb_5e();                         } /* BIT  3,(XY+o)    */

OP(xycb,60) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,61) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,62) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,63) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,64) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,65) { xycb_66();                         } /* BIT  4,(XY+o)    */
OP(xycb,66) { bit_xy(4, rm(m_ea));               } /* BIT  4,(XY+o)    */
OP(xycb,67) { xycb_66();                         } /* BIT  4,(XY+o)    */

OP(xycb,68) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,69) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6a) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6b) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6c) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6d) { xycb_6e();                         } /* BIT  5,(XY+o)    */
OP(xycb,6e) { bit_xy(5, rm(m_ea));               } /* BIT  5,(XY+o)    */
OP(xycb,6f) { xycb_6e();                         } /* BIT  5,(XY+o)    */

OP(xycb,70) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,71) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,72) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,73) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,74) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,75) { xycb_76();                         } /* BIT  6,(XY+o)    */
OP(xycb,76) { bit_xy(6, rm(m_ea));               } /* BIT  6,(XY+o)    */
OP(xycb,77) { xycb_76();                         } /* BIT  6,(XY+o)    */

OP(xycb,78) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,79) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7a) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7b) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7c) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7d) { xycb_7e();                         } /* BIT  7,(XY+o)    */
OP(xycb,7e) { bit_xy(7, rm(m_ea));               } /* BIT  7,(XY+o)    */
OP(xycb,7f) { xycb_7e();                         } /* BIT  7,(XY+o)    */

OP(xycb,80) { B = res(0, rm(m_ea)); wm(m_ea, B); } /* RES  0,B=(XY+o)  */
OP(xycb,81) { C = res(0, rm(m_ea)); wm(m_ea, C); } /* RES  0,C=(XY+o)  */
OP(xycb,82) { D = res(0, rm(m_ea)); wm(m_ea, D); } /* RES  0,D=(XY+o)  */
OP(xycb,83) { E = res(0, rm(m_ea)); wm(m_ea, E); } /* RES  0,E=(XY+o)  */
OP(xycb,84) { H = res(0, rm(m_ea)); wm(m_ea, H); } /* RES  0,H=(XY+o)  */
OP(xycb,85) { L = res(0, rm(m_ea)); wm(m_ea, L); } /* RES  0,L=(XY+o)  */
OP(xycb,86) { wm(m_ea, res(0, rm(m_ea)));        } /* RES  0,(XY+o)    */
OP(xycb,87) { A = res(0, rm(m_ea)); wm(m_ea, A); } /* RES  0,A=(XY+o)  */

OP(xycb,88) { B = res(1, rm(m_ea)); wm(m_ea, B); } /* RES  1,B=(XY+o)  */
OP(xycb,89) { C = res(1, rm(m_ea)); wm(m_ea, C); } /* RES  1,C=(XY+o)  */
OP(xycb,8a) { D = res(1, rm(m_ea)); wm(m_ea, D); } /* RES  1,D=(XY+o)  */
OP(xycb,8b) { E = res(1, rm(m_ea)); wm(m_ea, E); } /* RES  1,E=(XY+o)  */
OP(xycb,8c) { H = res(1, rm(m_ea)); wm(m_ea, H); } /* RES  1,H=(XY+o)  */
OP(xycb,8d) { L = res(1, rm(m_ea)); wm(m_ea, L); } /* RES  1,L=(XY+o)  */
OP(xycb,8e) { wm(m_ea, res(1, rm(m_ea)));        } /* RES  1,(XY+o)    */
OP(xycb,8f) { A = res(1, rm(m_ea)); wm(m_ea, A); } /* RES  1,A=(XY+o)  */

OP(xycb,90) { B = res(2, rm(m_ea)); wm(m_ea, B); } /* RES  2,B=(XY+o)  */
OP(xycb,91) { C = res(2, rm(m_ea)); wm(m_ea, C); } /* RES  2,C=(XY+o)  */
OP(xycb,92) { D = res(2, rm(m_ea)); wm(m_ea, D); } /* RES  2,D=(XY+o)  */
OP(xycb,93) { E = res(2, rm(m_ea)); wm(m_ea, E); } /* RES  2,E=(XY+o)  */
OP(xycb,94) { H = res(2, rm(m_ea)); wm(m_ea, H); } /* RES  2,H=(XY+o)  */
OP(xycb,95) { L = res(2, rm(m_ea)); wm(m_ea, L); } /* RES  2,L=(XY+o)  */
OP(xycb,96) { wm(m_ea, res(2, rm(m_ea)));        } /* RES  2,(XY+o)    */
OP(xycb,97) { A = res(2, rm(m_ea)); wm(m_ea, A); } /* RES  2,A=(XY+o)  */

OP(xycb,98) { B = res(3, rm(m_ea)); wm(m_ea, B); } /* RES  3,B=(XY+o)  */
OP(xycb,99) { C = res(3, rm(m_ea)); wm(m_ea, C); } /* RES  3,C=(XY+o)  */
OP(xycb,9a) { D = res(3, rm(m_ea)); wm(m_ea, D); } /* RES  3,D=(XY+o)  */
OP(xycb,9b) { E = res(3, rm(m_ea)); wm(m_ea, E); } /* RES  3,E=(XY+o)  */
OP(xycb,9c) { H = res(3, rm(m_ea)); wm(m_ea, H); } /* RES  3,H=(XY+o)  */
OP(xycb,9d) { L = res(3, rm(m_ea)); wm(m_ea, L); } /* RES  3,L=(XY+o)  */
OP(xycb,9e) { wm(m_ea, res(3, rm(m_ea)));        } /* RES  3,(XY+o)    */
OP(xycb,9f) { A = res(3, rm(m_ea)); wm(m_ea, A); } /* RES  3,A=(XY+o)  */

OP(xycb,a0) { B = res(4, rm(m_ea)); wm(m_ea, B); } /* RES  4,B=(XY+o)  */
OP(xycb,a1) { C = res(4, rm(m_ea)); wm(m_ea, C); } /* RES  4,C=(XY+o)  */
OP(xycb,a2) { D = res(4, rm(m_ea)); wm(m_ea, D); } /* RES  4,D=(XY+o)  */
OP(xycb,a3) { E = res(4, rm(m_ea)); wm(m_ea, E); } /* RES  4,E=(XY+o)  */
OP(xycb,a4) { H = res(4, rm(m_ea)); wm(m_ea, H); } /* RES  4,H=(XY+o)  */
OP(xycb,a5) { L = res(4, rm(m_ea)); wm(m_ea, L); } /* RES  4,L=(XY+o)  */
OP(xycb,a6) { wm(m_ea, res(4, rm(m_ea)));        } /* RES  4,(XY+o)    */
OP(xycb,a7) { A = res(4, rm(m_ea)); wm(m_ea, A); } /* RES  4,A=(XY+o)  */

OP(xycb,a8) { B = res(5, rm(m_ea)); wm(m_ea, B); } /* RES  5,B=(XY+o)  */
OP(xycb,a9) { C = res(5, rm(m_ea)); wm(m_ea, C); } /* RES  5,C=(XY+o)  */
OP(xycb,aa) { D = res(5, rm(m_ea)); wm(m_ea, D); } /* RES  5,D=(XY+o)  */
OP(xycb,ab) { E = res(5, rm(m_ea)); wm(m_ea, E); } /* RES  5,E=(XY+o)  */
OP(xycb,ac) { H = res(5, rm(m_ea)); wm(m_ea, H); } /* RES  5,H=(XY+o)  */
OP(xycb,ad) { L = res(5, rm(m_ea)); wm(m_ea, L); } /* RES  5,L=(XY+o)  */
OP(xycb,ae) { wm(m_ea, res(5, rm(m_ea)));        } /* RES  5,(XY+o)    */
OP(xycb,af) { A = res(5, rm(m_ea)); wm(m_ea, A); } /* RES  5,A=(XY+o)  */

OP(xycb,b0) { B = res(6, rm(m_ea)); wm(m_ea, B); } /* RES  6,B=(XY+o)  */
OP(xycb,b1) { C = res(6, rm(m_ea)); wm(m_ea, C); } /* RES  6,C=(XY+o)  */
OP(xycb,b2) { D = res(6, rm(m_ea)); wm(m_ea, D); } /* RES  6,D=(XY+o)  */
OP(xycb,b3) { E = res(6, rm(m_ea)); wm(m_ea, E); } /* RES  6,E=(XY+o)  */
OP(xycb,b4) { H = res(6, rm(m_ea)); wm(m_ea, H); } /* RES  6,H=(XY+o)  */
OP(xycb,b5) { L = res(6, rm(m_ea)); wm(m_ea, L); } /* RES  6,L=(XY+o)  */
OP(xycb,b6) { wm(m_ea, res(6, rm(m_ea)));        } /* RES  6,(XY+o)    */
OP(xycb,b7) { A = res(6, rm(m_ea)); wm(m_ea, A); } /* RES  6,A=(XY+o)  */

OP(xycb,b8) { B = res(7, rm(m_ea)); wm(m_ea, B); } /* RES  7,B=(XY+o)  */
OP(xycb,b9) { C = res(7, rm(m_ea)); wm(m_ea, C); } /* RES  7,C=(XY+o)  */
OP(xycb,ba) { D = res(7, rm(m_ea)); wm(m_ea, D); } /* RES  7,D=(XY+o)  */
OP(xycb,bb) { E = res(7, rm(m_ea)); wm(m_ea, E); } /* RES  7,E=(XY+o)  */
OP(xycb,bc) { H = res(7, rm(m_ea)); wm(m_ea, H); } /* RES  7,H=(XY+o)  */
OP(xycb,bd) { L = res(7, rm(m_ea)); wm(m_ea, L); } /* RES  7,L=(XY+o)  */
OP(xycb,be) { wm(m_ea, res(7, rm(m_ea)));        } /* RES  7,(XY+o)    */
OP(xycb,bf) { A = res(7, rm(m_ea)); wm(m_ea, A); } /* RES  7,A=(XY+o)  */

OP(xycb,c0) { B = set(0, rm(m_ea)); wm(m_ea, B); } /* SET  0,B=(XY+o)  */
OP(xycb,c1) { C = set(0, rm(m_ea)); wm(m_ea, C); } /* SET  0,C=(XY+o)  */
OP(xycb,c2) { D = set(0, rm(m_ea)); wm(m_ea, D); } /* SET  0,D=(XY+o)  */
OP(xycb,c3) { E = set(0, rm(m_ea)); wm(m_ea, E); } /* SET  0,E=(XY+o)  */
OP(xycb,c4) { H = set(0, rm(m_ea)); wm(m_ea, H); } /* SET  0,H=(XY+o)  */
OP(xycb,c5) { L = set(0, rm(m_ea)); wm(m_ea, L); } /* SET  0,L=(XY+o)  */
OP(xycb,c6) { wm(m_ea, set(0, rm(m_ea)));        } /* SET  0,(XY+o)    */
OP(xycb,c7) { A = set(0, rm(m_ea)); wm(m_ea, A); } /* SET  0,A=(XY+o)  */

OP(xycb,c8) { B = set(1, rm(m_ea)); wm(m_ea, B); } /* SET  1,B=(XY+o)  */
OP(xycb,c9) { C = set(1, rm(m_ea)); wm(m_ea, C); } /* SET  1,C=(XY+o)  */
OP(xycb,ca) { D = set(1, rm(m_ea)); wm(m_ea, D); } /* SET  1,D=(XY+o)  */
OP(xycb,cb) { E = set(1, rm(m_ea)); wm(m_ea, E); } /* SET  1,E=(XY+o)  */
OP(xycb,cc) { H = set(1, rm(m_ea)); wm(m_ea, H); } /* SET  1,H=(XY+o)  */
OP(xycb,cd) { L = set(1, rm(m_ea)); wm(m_ea, L); } /* SET  1,L=(XY+o)  */
OP(xycb,ce) { wm(m_ea, set(1, rm(m_ea)));        } /* SET  1,(XY+o)    */
OP(xycb,cf) { A = set(1, rm(m_ea)); wm(m_ea, A); } /* SET  1,A=(XY+o)  */

OP(xycb,d0) { B = set(2, rm(m_ea)); wm(m_ea, B); } /* SET  2,B=(XY+o)  */
OP(xycb,d1) { C = set(2, rm(m_ea)); wm(m_ea, C); } /* SET  2,C=(XY+o)  */
OP(xycb,d2) { D = set(2, rm(m_ea)); wm(m_ea, D); } /* SET  2,D=(XY+o)  */
OP(xycb,d3) { E = set(2, rm(m_ea)); wm(m_ea, E); } /* SET  2,E=(XY+o)  */
OP(xycb,d4) { H = set(2, rm(m_ea)); wm(m_ea, H); } /* SET  2,H=(XY+o)  */
OP(xycb,d5) { L = set(2, rm(m_ea)); wm(m_ea, L); } /* SET  2,L=(XY+o)  */
OP(xycb,d6) { wm(m_ea, set(2, rm(m_ea)));        } /* SET  2,(XY+o)    */
OP(xycb,d7) { A = set(2, rm(m_ea)); wm(m_ea, A); } /* SET  2,A=(XY+o)  */

OP(xycb,d8) { B = set(3, rm(m_ea)); wm(m_ea, B); } /* SET  3,B=(XY+o)  */
OP(xycb,d9) { C = set(3, rm(m_ea)); wm(m_ea, C); } /* SET  3,C=(XY+o)  */
OP(xycb,da) { D = set(3, rm(m_ea)); wm(m_ea, D); } /* SET  3,D=(XY+o)  */
OP(xycb,db) { E = set(3, rm(m_ea)); wm(m_ea, E); } /* SET  3,E=(XY+o)  */
OP(xycb,dc) { H = set(3, rm(m_ea)); wm(m_ea, H); } /* SET  3,H=(XY+o)  */
OP(xycb,dd) { L = set(3, rm(m_ea)); wm(m_ea, L); } /* SET  3,L=(XY+o)  */
OP(xycb,de) { wm(m_ea, set(3, rm(m_ea)));        } /* SET  3,(XY+o)    */
OP(xycb,df) { A = set(3, rm(m_ea)); wm(m_ea, A); } /* SET  3,A=(XY+o)  */

OP(xycb,e0) { B = set(4, rm(m_ea)); wm(m_ea, B); } /* SET  4,B=(XY+o)  */
OP(xycb,e1) { C = set(4, rm(m_ea)); wm(m_ea, C); } /* SET  4,C=(XY+o)  */
OP(xycb,e2) { D = set(4, rm(m_ea)); wm(m_ea, D); } /* SET  4,D=(XY+o)  */
OP(xycb,e3) { E = set(4, rm(m_ea)); wm(m_ea, E); } /* SET  4,E=(XY+o)  */
OP(xycb,e4) { H = set(4, rm(m_ea)); wm(m_ea, H); } /* SET  4,H=(XY+o)  */
OP(xycb,e5) { L = set(4, rm(m_ea)); wm(m_ea, L); } /* SET  4,L=(XY+o)  */
OP(xycb,e6) { wm(m_ea, set(4, rm(m_ea)));        } /* SET  4,(XY+o)    */
OP(xycb,e7) { A = set(4, rm(m_ea)); wm(m_ea, A); } /* SET  4,A=(XY+o)  */

OP(xycb,e8) { B = set(5, rm(m_ea)); wm(m_ea, B); } /* SET  5,B=(XY+o)  */
OP(xycb,e9) { C = set(5, rm(m_ea)); wm(m_ea, C); } /* SET  5,C=(XY+o)  */
OP(xycb,ea) { D = set(5, rm(m_ea)); wm(m_ea, D); } /* SET  5,D=(XY+o)  */
OP(xycb,eb) { E = set(5, rm(m_ea)); wm(m_ea, E); } /* SET  5,E=(XY+o)  */
OP(xycb,ec) { H = set(5, rm(m_ea)); wm(m_ea, H); } /* SET  5,H=(XY+o)  */
OP(xycb,ed) { L = set(5, rm(m_ea)); wm(m_ea, L); } /* SET  5,L=(XY+o)  */
OP(xycb,ee) { wm(m_ea, set(5, rm(m_ea)));        } /* SET  5,(XY+o)    */
OP(xycb,ef) { A = set(5, rm(m_ea)); wm(m_ea, A); } /* SET  5,A=(XY+o)  */

OP(xycb,f0) { B = set(6, rm(m_ea)); wm(m_ea, B); } /* SET  6,B=(XY+o)  */
OP(xycb,f1) { C = set(6, rm(m_ea)); wm(m_ea, C); } /* SET  6,C=(XY+o)  */
OP(xycb,f2) { D = set(6, rm(m_ea)); wm(m_ea, D); } /* SET  6,D=(XY+o)  */
OP(xycb,f3) { E = set(6, rm(m_ea)); wm(m_ea, E); } /* SET  6,E=(XY+o)  */
OP(xycb,f4) { H = set(6, rm(m_ea)); wm(m_ea, H); } /* SET  6,H=(XY+o)  */
OP(xycb,f5) { L = set(6, rm(m_ea)); wm(m_ea, L); } /* SET  6,L=(XY+o)  */
OP(xycb,f6) { wm(m_ea, set(6, rm(m_ea)));        } /* SET  6,(XY+o)    */
OP(xycb,f7) { A = set(6, rm(m_ea)); wm(m_ea, A); } /* SET  6,A=(XY+o)  */

OP(xycb,f8) { B = set(7, rm(m_ea)); wm(m_ea, B); } /* SET  7,B=(XY+o)  */
OP(xycb,f9) { C = set(7, rm(m_ea)); wm(m_ea, C); } /* SET  7,C=(XY+o)  */
OP(xycb,fa) { D = set(7, rm(m_ea)); wm(m_ea, D); } /* SET  7,D=(XY+o)  */
OP(xycb,fb) { E = set(7, rm(m_ea)); wm(m_ea, E); } /* SET  7,E=(XY+o)  */
OP(xycb,fc) { H = set(7, rm(m_ea)); wm(m_ea, H); } /* SET  7,H=(XY+o)  */
OP(xycb,fd) { L = set(7, rm(m_ea)); wm(m_ea, L); } /* SET  7,L=(XY+o)  */
OP(xycb,fe) { wm(m_ea, set(7, rm(m_ea)));        } /* SET  7,(XY+o)    */
OP(xycb,ff) { A = set(7, rm(m_ea)); wm(m_ea, A); } /* SET  7,A=(XY+o)  */

OP(illegal,1) {
	logerror("Z80 '%s' ill. opcode $%02x $%02x\n",
			tag(), m_decrypted_opcodes_direct->read_byte((PCD-1)&0xffff), m_decrypted_opcodes_direct->read_byte(PCD));
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
OP(dd,09) { add16(m_ix, m_bc);                               } /* ADD  IX,BC       */
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
OP(dd,19) { add16(m_ix, m_de);                               } /* ADD  IX,DE       */
OP(dd,1a) { illegal_1(); op_1a();                            } /* DB   DD          */
OP(dd,1b) { illegal_1(); op_1b();                            } /* DB   DD          */
OP(dd,1c) { illegal_1(); op_1c();                            } /* DB   DD          */
OP(dd,1d) { illegal_1(); op_1d();                            } /* DB   DD          */
OP(dd,1e) { illegal_1(); op_1e();                            } /* DB   DD          */
OP(dd,1f) { illegal_1(); op_1f();                            } /* DB   DD          */

OP(dd,20) { illegal_1(); op_20();                            } /* DB   DD          */
OP(dd,21) { IX = arg16();                                    } /* LD   IX,w        */
OP(dd,22) { m_ea = arg16(); wm16(m_ea, m_ix); WZ = m_ea + 1; } /* LD   (w),IX      */
OP(dd,23) { IX++;                                            } /* INC  IX          */
OP(dd,24) { HX = inc(HX);                                    } /* INC  HX          */
OP(dd,25) { HX = dec(HX);                                    } /* DEC  HX          */
OP(dd,26) { HX = arg();                                      } /* LD   HX,n        */
OP(dd,27) { illegal_1(); op_27();                            } /* DB   DD          */

OP(dd,28) { illegal_1(); op_28();                            } /* DB   DD          */
OP(dd,29) { add16(m_ix, m_ix);                               } /* ADD  IX,IX       */
OP(dd,2a) { m_ea = arg16(); rm16(m_ea, m_ix); WZ = m_ea + 1; } /* LD   IX,(w)      */
OP(dd,2b) { IX--;                                            } /* DEC  IX          */
OP(dd,2c) { LX = inc(LX);                                    } /* INC  LX          */
OP(dd,2d) { LX = dec(LX);                                    } /* DEC  LX          */
OP(dd,2e) { LX = arg();                                      } /* LD   LX,n        */
OP(dd,2f) { illegal_1(); op_2f();                            } /* DB   DD          */

OP(dd,30) { illegal_1(); op_30();                            } /* DB   DD          */
OP(dd,31) { illegal_1(); op_31();                            } /* DB   DD          */
OP(dd,32) { illegal_1(); op_32();                            } /* DB   DD          */
OP(dd,33) { illegal_1(); op_33();                            } /* DB   DD          */
OP(dd,34) { eax(); wm(m_ea, inc(rm(m_ea)));                  } /* INC  (IX+o)      */
OP(dd,35) { eax(); wm(m_ea, dec(rm(m_ea)));                  } /* DEC  (IX+o)      */
OP(dd,36) { eax(); wm(m_ea, arg());                          } /* LD   (IX+o),n    */
OP(dd,37) { illegal_1(); op_37();                            } /* DB   DD          */

OP(dd,38) { illegal_1(); op_38();                            } /* DB   DD          */
OP(dd,39) { add16(m_ix, m_sp);                               } /* ADD  IX,SP       */
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
OP(dd,46) { eax(); B = rm(m_ea);                             } /* LD   B,(IX+o)    */
OP(dd,47) { illegal_1(); op_47();                            } /* DB   DD          */

OP(dd,48) { illegal_1(); op_48();                            } /* DB   DD          */
OP(dd,49) { illegal_1(); op_49();                            } /* DB   DD          */
OP(dd,4a) { illegal_1(); op_4a();                            } /* DB   DD          */
OP(dd,4b) { illegal_1(); op_4b();                            } /* DB   DD          */
OP(dd,4c) { C = HX;                                          } /* LD   C,HX        */
OP(dd,4d) { C = LX;                                          } /* LD   C,LX        */
OP(dd,4e) { eax(); C = rm(m_ea);                             } /* LD   C,(IX+o)    */
OP(dd,4f) { illegal_1(); op_4f();                            } /* DB   DD          */

OP(dd,50) { illegal_1(); op_50();                            } /* DB   DD          */
OP(dd,51) { illegal_1(); op_51();                            } /* DB   DD          */
OP(dd,52) { illegal_1(); op_52();                            } /* DB   DD          */
OP(dd,53) { illegal_1(); op_53();                            } /* DB   DD          */
OP(dd,54) { D = HX;                                          } /* LD   D,HX        */
OP(dd,55) { D = LX;                                          } /* LD   D,LX        */
OP(dd,56) { eax(); D = rm(m_ea);                             } /* LD   D,(IX+o)    */
OP(dd,57) { illegal_1(); op_57();                            } /* DB   DD          */

OP(dd,58) { illegal_1(); op_58();                            } /* DB   DD          */
OP(dd,59) { illegal_1(); op_59();                            } /* DB   DD          */
OP(dd,5a) { illegal_1(); op_5a();                            } /* DB   DD          */
OP(dd,5b) { illegal_1(); op_5b();                            } /* DB   DD          */
OP(dd,5c) { E = HX;                                          } /* LD   E,HX        */
OP(dd,5d) { E = LX;                                          } /* LD   E,LX        */
OP(dd,5e) { eax(); E = rm(m_ea);                             } /* LD   E,(IX+o)    */
OP(dd,5f) { illegal_1(); op_5f();                            } /* DB   DD          */

OP(dd,60) { HX = B;                                          } /* LD   HX,B        */
OP(dd,61) { HX = C;                                          } /* LD   HX,C        */
OP(dd,62) { HX = D;                                          } /* LD   HX,D        */
OP(dd,63) { HX = E;                                          } /* LD   HX,E        */
OP(dd,64) {                                                  } /* LD   HX,HX       */
OP(dd,65) { HX = LX;                                         } /* LD   HX,LX       */
OP(dd,66) { eax(); H = rm(m_ea);                             } /* LD   H,(IX+o)    */
OP(dd,67) { HX = A;                                          } /* LD   HX,A        */

OP(dd,68) { LX = B;                                          } /* LD   LX,B        */
OP(dd,69) { LX = C;                                          } /* LD   LX,C        */
OP(dd,6a) { LX = D;                                          } /* LD   LX,D        */
OP(dd,6b) { LX = E;                                          } /* LD   LX,E        */
OP(dd,6c) { LX = HX;                                         } /* LD   LX,HX       */
OP(dd,6d) {                                                  } /* LD   LX,LX       */
OP(dd,6e) { eax(); L = rm(m_ea);                             } /* LD   L,(IX+o)    */
OP(dd,6f) { LX = A;                                          } /* LD   LX,A        */

OP(dd,70) { eax(); wm(m_ea, B);                              } /* LD   (IX+o),B    */
OP(dd,71) { eax(); wm(m_ea, C);                              } /* LD   (IX+o),C    */
OP(dd,72) { eax(); wm(m_ea, D);                              } /* LD   (IX+o),D    */
OP(dd,73) { eax(); wm(m_ea, E);                              } /* LD   (IX+o),E    */
OP(dd,74) { eax(); wm(m_ea, H);                              } /* LD   (IX+o),H    */
OP(dd,75) { eax(); wm(m_ea, L);                              } /* LD   (IX+o),L    */
OP(dd,76) { illegal_1(); op_76();                            } /* DB   DD          */
OP(dd,77) { eax(); wm(m_ea, A);                              } /* LD   (IX+o),A    */

OP(dd,78) { illegal_1(); op_78();                            } /* DB   DD          */
OP(dd,79) { illegal_1(); op_79();                            } /* DB   DD          */
OP(dd,7a) { illegal_1(); op_7a();                            } /* DB   DD          */
OP(dd,7b) { illegal_1(); op_7b();                            } /* DB   DD          */
OP(dd,7c) { A = HX;                                          } /* LD   A,HX        */
OP(dd,7d) { A = LX;                                          } /* LD   A,LX        */
OP(dd,7e) { eax(); A = rm(m_ea);                             } /* LD   A,(IX+o)    */
OP(dd,7f) { illegal_1(); op_7f();                            } /* DB   DD          */

OP(dd,80) { illegal_1(); op_80();                            } /* DB   DD          */
OP(dd,81) { illegal_1(); op_81();                            } /* DB   DD          */
OP(dd,82) { illegal_1(); op_82();                            } /* DB   DD          */
OP(dd,83) { illegal_1(); op_83();                            } /* DB   DD          */
OP(dd,84) { add_a(HX);                                       } /* ADD  A,HX        */
OP(dd,85) { add_a(LX);                                       } /* ADD  A,LX        */
OP(dd,86) { eax(); add_a(rm(m_ea));                          } /* ADD  A,(IX+o)    */
OP(dd,87) { illegal_1(); op_87();                            } /* DB   DD          */

OP(dd,88) { illegal_1(); op_88();                            } /* DB   DD          */
OP(dd,89) { illegal_1(); op_89();                            } /* DB   DD          */
OP(dd,8a) { illegal_1(); op_8a();                            } /* DB   DD          */
OP(dd,8b) { illegal_1(); op_8b();                            } /* DB   DD          */
OP(dd,8c) { adc_a(HX);                                       } /* ADC  A,HX        */
OP(dd,8d) { adc_a(LX);                                       } /* ADC  A,LX        */
OP(dd,8e) { eax(); adc_a(rm(m_ea));                          } /* ADC  A,(IX+o)    */
OP(dd,8f) { illegal_1(); op_8f();                            } /* DB   DD          */

OP(dd,90) { illegal_1(); op_90();                            } /* DB   DD          */
OP(dd,91) { illegal_1(); op_91();                            } /* DB   DD          */
OP(dd,92) { illegal_1(); op_92();                            } /* DB   DD          */
OP(dd,93) { illegal_1(); op_93();                            } /* DB   DD          */
OP(dd,94) { sub(HX);                                         } /* SUB  HX          */
OP(dd,95) { sub(LX);                                         } /* SUB  LX          */
OP(dd,96) { eax(); sub(rm(m_ea));                            } /* SUB  (IX+o)      */
OP(dd,97) { illegal_1(); op_97();                            } /* DB   DD          */

OP(dd,98) { illegal_1(); op_98();                            } /* DB   DD          */
OP(dd,99) { illegal_1(); op_99();                            } /* DB   DD          */
OP(dd,9a) { illegal_1(); op_9a();                            } /* DB   DD          */
OP(dd,9b) { illegal_1(); op_9b();                            } /* DB   DD          */
OP(dd,9c) { sbc_a(HX);                                       } /* SBC  A,HX        */
OP(dd,9d) { sbc_a(LX);                                       } /* SBC  A,LX        */
OP(dd,9e) { eax(); sbc_a(rm(m_ea));                          } /* SBC  A,(IX+o)    */
OP(dd,9f) { illegal_1(); op_9f();                            } /* DB   DD          */

OP(dd,a0) { illegal_1(); op_a0();                            } /* DB   DD          */
OP(dd,a1) { illegal_1(); op_a1();                            } /* DB   DD          */
OP(dd,a2) { illegal_1(); op_a2();                            } /* DB   DD          */
OP(dd,a3) { illegal_1(); op_a3();                            } /* DB   DD          */
OP(dd,a4) { and_a(HX);                                       } /* AND  HX          */
OP(dd,a5) { and_a(LX);                                       } /* AND  LX          */
OP(dd,a6) { eax(); and_a(rm(m_ea));                          } /* AND  (IX+o)      */
OP(dd,a7) { illegal_1(); op_a7();                            } /* DB   DD          */

OP(dd,a8) { illegal_1(); op_a8();                            } /* DB   DD          */
OP(dd,a9) { illegal_1(); op_a9();                            } /* DB   DD          */
OP(dd,aa) { illegal_1(); op_aa();                            } /* DB   DD          */
OP(dd,ab) { illegal_1(); op_ab();                            } /* DB   DD          */
OP(dd,ac) { xor_a(HX);                                       } /* XOR  HX          */
OP(dd,ad) { xor_a(LX);                                       } /* XOR  LX          */
OP(dd,ae) { eax(); xor_a(rm(m_ea));                          } /* XOR  (IX+o)      */
OP(dd,af) { illegal_1(); op_af();                            } /* DB   DD          */

OP(dd,b0) { illegal_1(); op_b0();                            } /* DB   DD          */
OP(dd,b1) { illegal_1(); op_b1();                            } /* DB   DD          */
OP(dd,b2) { illegal_1(); op_b2();                            } /* DB   DD          */
OP(dd,b3) { illegal_1(); op_b3();                            } /* DB   DD          */
OP(dd,b4) { or_a(HX);                                        } /* OR   HX          */
OP(dd,b5) { or_a(LX);                                        } /* OR   LX          */
OP(dd,b6) { eax(); or_a(rm(m_ea));                           } /* OR   (IX+o)      */
OP(dd,b7) { illegal_1(); op_b7();                            } /* DB   DD          */

OP(dd,b8) { illegal_1(); op_b8();                            } /* DB   DD          */
OP(dd,b9) { illegal_1(); op_b9();                            } /* DB   DD          */
OP(dd,ba) { illegal_1(); op_ba();                            } /* DB   DD          */
OP(dd,bb) { illegal_1(); op_bb();                            } /* DB   DD          */
OP(dd,bc) { cp(HX);                                          } /* CP   HX          */
OP(dd,bd) { cp(LX);                                          } /* CP   LX          */
OP(dd,be) { eax(); cp(rm(m_ea));                             } /* CP   (IX+o)      */
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
OP(dd,cb) { eax(); EXEC(xycb,arg());                         } /* **   DD CB xx    */
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
OP(dd,e1) { pop(m_ix);                                       } /* POP  IX          */
OP(dd,e2) { illegal_1(); op_e2();                            } /* DB   DD          */
OP(dd,e3) { ex_sp(m_ix);                                     } /* EX   (SP),IX     */
OP(dd,e4) { illegal_1(); op_e4();                            } /* DB   DD          */
OP(dd,e5) { push(m_ix);                                      } /* PUSH IX          */
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
OP(fd,09) { add16(m_iy, m_bc);                               } /* ADD  IY,BC       */
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
OP(fd,19) { add16(m_iy, m_de);                               } /* ADD  IY,DE       */
OP(fd,1a) { illegal_1(); op_1a();                            } /* DB   FD          */
OP(fd,1b) { illegal_1(); op_1b();                            } /* DB   FD          */
OP(fd,1c) { illegal_1(); op_1c();                            } /* DB   FD          */
OP(fd,1d) { illegal_1(); op_1d();                            } /* DB   FD          */
OP(fd,1e) { illegal_1(); op_1e();                            } /* DB   FD          */
OP(fd,1f) { illegal_1(); op_1f();                            } /* DB   FD          */

OP(fd,20) { illegal_1(); op_20();                            } /* DB   FD          */
OP(fd,21) { IY = arg16();                                    } /* LD   IY,w        */
OP(fd,22) { m_ea = arg16(); wm16(m_ea, m_iy); WZ = m_ea + 1; } /* LD   (w),IY      */
OP(fd,23) { IY++;                                            } /* INC  IY          */
OP(fd,24) { HY = inc(HY);                                    } /* INC  HY          */
OP(fd,25) { HY = dec(HY);                                    } /* DEC  HY          */
OP(fd,26) { HY = arg();                                      } /* LD   HY,n        */
OP(fd,27) { illegal_1(); op_27();                            } /* DB   FD          */

OP(fd,28) { illegal_1(); op_28();                            } /* DB   FD          */
OP(fd,29) { add16(m_iy, m_iy);                               } /* ADD  IY,IY       */
OP(fd,2a) { m_ea = arg16(); rm16(m_ea, m_iy); WZ = m_ea + 1; } /* LD   IY,(w)      */
OP(fd,2b) { IY--;                                            } /* DEC  IY          */
OP(fd,2c) { LY = inc(LY);                                    } /* INC  LY          */
OP(fd,2d) { LY = dec(LY);                                    } /* DEC  LY          */
OP(fd,2e) { LY = arg();                                      } /* LD   LY,n        */
OP(fd,2f) { illegal_1(); op_2f();                            } /* DB   FD          */

OP(fd,30) { illegal_1(); op_30();                            } /* DB   FD          */
OP(fd,31) { illegal_1(); op_31();                            } /* DB   FD          */
OP(fd,32) { illegal_1(); op_32();                            } /* DB   FD          */
OP(fd,33) { illegal_1(); op_33();                            } /* DB   FD          */
OP(fd,34) { eay(); wm(m_ea, inc(rm(m_ea)));                  } /* INC  (IY+o)      */
OP(fd,35) { eay(); wm(m_ea, dec(rm(m_ea)));                  } /* DEC  (IY+o)      */
OP(fd,36) { eay(); wm(m_ea, arg());                          } /* LD   (IY+o),n    */
OP(fd,37) { illegal_1(); op_37();                            } /* DB   FD          */

OP(fd,38) { illegal_1(); op_38();                            } /* DB   FD          */
OP(fd,39) { add16(m_iy, m_sp);                               } /* ADD  IY,SP       */
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
OP(fd,46) { eay(); B = rm(m_ea);                             } /* LD   B,(IY+o)    */
OP(fd,47) { illegal_1(); op_47();                            } /* DB   FD          */

OP(fd,48) { illegal_1(); op_48();                            } /* DB   FD          */
OP(fd,49) { illegal_1(); op_49();                            } /* DB   FD          */
OP(fd,4a) { illegal_1(); op_4a();                            } /* DB   FD          */
OP(fd,4b) { illegal_1(); op_4b();                            } /* DB   FD          */
OP(fd,4c) { C = HY;                                          } /* LD   C,HY        */
OP(fd,4d) { C = LY;                                          } /* LD   C,LY        */
OP(fd,4e) { eay(); C = rm(m_ea);                             } /* LD   C,(IY+o)    */
OP(fd,4f) { illegal_1(); op_4f();                            } /* DB   FD          */

OP(fd,50) { illegal_1(); op_50();                            } /* DB   FD          */
OP(fd,51) { illegal_1(); op_51();                            } /* DB   FD          */
OP(fd,52) { illegal_1(); op_52();                            } /* DB   FD          */
OP(fd,53) { illegal_1(); op_53();                            } /* DB   FD          */
OP(fd,54) { D = HY;                                          } /* LD   D,HY        */
OP(fd,55) { D = LY;                                          } /* LD   D,LY        */
OP(fd,56) { eay(); D = rm(m_ea);                             } /* LD   D,(IY+o)    */
OP(fd,57) { illegal_1(); op_57();                            } /* DB   FD          */

OP(fd,58) { illegal_1(); op_58();                            } /* DB   FD          */
OP(fd,59) { illegal_1(); op_59();                            } /* DB   FD          */
OP(fd,5a) { illegal_1(); op_5a();                            } /* DB   FD          */
OP(fd,5b) { illegal_1(); op_5b();                            } /* DB   FD          */
OP(fd,5c) { E = HY;                                          } /* LD   E,HY        */
OP(fd,5d) { E = LY;                                          } /* LD   E,LY        */
OP(fd,5e) { eay(); E = rm(m_ea);                             } /* LD   E,(IY+o)    */
OP(fd,5f) { illegal_1(); op_5f();                            } /* DB   FD          */

OP(fd,60) { HY = B;                                          } /* LD   HY,B        */
OP(fd,61) { HY = C;                                          } /* LD   HY,C        */
OP(fd,62) { HY = D;                                          } /* LD   HY,D        */
OP(fd,63) { HY = E;                                          } /* LD   HY,E        */
OP(fd,64) {                                                  } /* LD   HY,HY       */
OP(fd,65) { HY = LY;                                         } /* LD   HY,LY       */
OP(fd,66) { eay(); H = rm(m_ea);                             } /* LD   H,(IY+o)    */
OP(fd,67) { HY = A;                                          } /* LD   HY,A        */

OP(fd,68) { LY = B;                                          } /* LD   LY,B        */
OP(fd,69) { LY = C;                                          } /* LD   LY,C        */
OP(fd,6a) { LY = D;                                          } /* LD   LY,D        */
OP(fd,6b) { LY = E;                                          } /* LD   LY,E        */
OP(fd,6c) { LY = HY;                                         } /* LD   LY,HY       */
OP(fd,6d) {                                                  } /* LD   LY,LY       */
OP(fd,6e) { eay(); L = rm(m_ea);                             } /* LD   L,(IY+o)    */
OP(fd,6f) { LY = A;                                          } /* LD   LY,A        */

OP(fd,70) { eay(); wm(m_ea, B);                              } /* LD   (IY+o),B    */
OP(fd,71) { eay(); wm(m_ea, C);                              } /* LD   (IY+o),C    */
OP(fd,72) { eay(); wm(m_ea, D);                              } /* LD   (IY+o),D    */
OP(fd,73) { eay(); wm(m_ea, E);                              } /* LD   (IY+o),E    */
OP(fd,74) { eay(); wm(m_ea, H);                              } /* LD   (IY+o),H    */
OP(fd,75) { eay(); wm(m_ea, L);                              } /* LD   (IY+o),L    */
OP(fd,76) { illegal_1(); op_76();                            } /* DB   FD          */
OP(fd,77) { eay(); wm(m_ea, A);                              } /* LD   (IY+o),A    */

OP(fd,78) { illegal_1(); op_78();                            } /* DB   FD          */
OP(fd,79) { illegal_1(); op_79();                            } /* DB   FD          */
OP(fd,7a) { illegal_1(); op_7a();                            } /* DB   FD          */
OP(fd,7b) { illegal_1(); op_7b();                            } /* DB   FD          */
OP(fd,7c) { A = HY;                                          } /* LD   A,HY        */
OP(fd,7d) { A = LY;                                          } /* LD   A,LY        */
OP(fd,7e) { eay(); A = rm(m_ea);                             } /* LD   A,(IY+o)    */
OP(fd,7f) { illegal_1(); op_7f();                            } /* DB   FD          */

OP(fd,80) { illegal_1(); op_80();                            } /* DB   FD          */
OP(fd,81) { illegal_1(); op_81();                            } /* DB   FD          */
OP(fd,82) { illegal_1(); op_82();                            } /* DB   FD          */
OP(fd,83) { illegal_1(); op_83();                            } /* DB   FD          */
OP(fd,84) { add_a(HY);                                       } /* ADD  A,HY        */
OP(fd,85) { add_a(LY);                                       } /* ADD  A,LY        */
OP(fd,86) { eay(); add_a(rm(m_ea));                          } /* ADD  A,(IY+o)    */
OP(fd,87) { illegal_1(); op_87();                            } /* DB   FD          */

OP(fd,88) { illegal_1(); op_88();                            } /* DB   FD          */
OP(fd,89) { illegal_1(); op_89();                            } /* DB   FD          */
OP(fd,8a) { illegal_1(); op_8a();                            } /* DB   FD          */
OP(fd,8b) { illegal_1(); op_8b();                            } /* DB   FD          */
OP(fd,8c) { adc_a(HY);                                       } /* ADC  A,HY        */
OP(fd,8d) { adc_a(LY);                                       } /* ADC  A,LY        */
OP(fd,8e) { eay(); adc_a(rm(m_ea));                          } /* ADC  A,(IY+o)    */
OP(fd,8f) { illegal_1(); op_8f();                            } /* DB   FD          */

OP(fd,90) { illegal_1(); op_90();                            } /* DB   FD          */
OP(fd,91) { illegal_1(); op_91();                            } /* DB   FD          */
OP(fd,92) { illegal_1(); op_92();                            } /* DB   FD          */
OP(fd,93) { illegal_1(); op_93();                            } /* DB   FD          */
OP(fd,94) { sub(HY);                                         } /* SUB  HY          */
OP(fd,95) { sub(LY);                                         } /* SUB  LY          */
OP(fd,96) { eay(); sub(rm(m_ea));                            } /* SUB  (IY+o)      */
OP(fd,97) { illegal_1(); op_97();                            } /* DB   FD          */

OP(fd,98) { illegal_1(); op_98();                            } /* DB   FD          */
OP(fd,99) { illegal_1(); op_99();                            } /* DB   FD          */
OP(fd,9a) { illegal_1(); op_9a();                            } /* DB   FD          */
OP(fd,9b) { illegal_1(); op_9b();                            } /* DB   FD          */
OP(fd,9c) { sbc_a(HY);                                       } /* SBC  A,HY        */
OP(fd,9d) { sbc_a(LY);                                       } /* SBC  A,LY        */
OP(fd,9e) { eay(); sbc_a(rm(m_ea));                          } /* SBC  A,(IY+o)    */
OP(fd,9f) { illegal_1(); op_9f();                            } /* DB   FD          */

OP(fd,a0) { illegal_1(); op_a0();                            } /* DB   FD          */
OP(fd,a1) { illegal_1(); op_a1();                            } /* DB   FD          */
OP(fd,a2) { illegal_1(); op_a2();                            } /* DB   FD          */
OP(fd,a3) { illegal_1(); op_a3();                            } /* DB   FD          */
OP(fd,a4) { and_a(HY);                                       } /* AND  HY          */
OP(fd,a5) { and_a(LY);                                       } /* AND  LY          */
OP(fd,a6) { eay(); and_a(rm(m_ea));                          } /* AND  (IY+o)      */
OP(fd,a7) { illegal_1(); op_a7();                            } /* DB   FD          */

OP(fd,a8) { illegal_1(); op_a8();                            } /* DB   FD          */
OP(fd,a9) { illegal_1(); op_a9();                            } /* DB   FD          */
OP(fd,aa) { illegal_1(); op_aa();                            } /* DB   FD          */
OP(fd,ab) { illegal_1(); op_ab();                            } /* DB   FD          */
OP(fd,ac) { xor_a(HY);                                       } /* XOR  HY          */
OP(fd,ad) { xor_a(LY);                                       } /* XOR  LY          */
OP(fd,ae) { eay(); xor_a(rm(m_ea));                          } /* XOR  (IY+o)      */
OP(fd,af) { illegal_1(); op_af();                            } /* DB   FD          */

OP(fd,b0) { illegal_1(); op_b0();                            } /* DB   FD          */
OP(fd,b1) { illegal_1(); op_b1();                            } /* DB   FD          */
OP(fd,b2) { illegal_1(); op_b2();                            } /* DB   FD          */
OP(fd,b3) { illegal_1(); op_b3();                            } /* DB   FD          */
OP(fd,b4) { or_a(HY);                                        } /* OR   HY          */
OP(fd,b5) { or_a(LY);                                        } /* OR   LY          */
OP(fd,b6) { eay(); or_a(rm(m_ea));                           } /* OR   (IY+o)      */
OP(fd,b7) { illegal_1(); op_b7();                            } /* DB   FD          */

OP(fd,b8) { illegal_1(); op_b8();                            } /* DB   FD          */
OP(fd,b9) { illegal_1(); op_b9();                            } /* DB   FD          */
OP(fd,ba) { illegal_1(); op_ba();                            } /* DB   FD          */
OP(fd,bb) { illegal_1(); op_bb();                            } /* DB   FD          */
OP(fd,bc) { cp(HY);                                          } /* CP   HY          */
OP(fd,bd) { cp(LY);                                          } /* CP   LY          */
OP(fd,be) { eay(); cp(rm(m_ea));                             } /* CP   (IY+o)      */
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
OP(fd,cb) { eay(); EXEC(xycb,arg());                         } /* **   FD CB xx    */
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
OP(fd,e1) { pop(m_iy);                                       } /* POP  IY          */
OP(fd,e2) { illegal_1(); op_e2();                            } /* DB   FD          */
OP(fd,e3) { ex_sp(m_iy);                                     } /* EX   (SP),IY     */
OP(fd,e4) { illegal_1(); op_e4();                            } /* DB   FD          */
OP(fd,e5) { push(m_iy);                                      } /* PUSH IY          */
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
			tag(), m_decrypted_opcodes_direct->read_byte((PCD-1)&0xffff));
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

OP(ed,40) { B = in(BC); F = (F & CF) | SZP[B];               } /* IN   B,(C)       */
OP(ed,41) { out(BC, B);                                      } /* OUT  (C),B       */
OP(ed,42) { sbc_hl(m_bc);                                    } /* SBC  HL,BC       */
OP(ed,43) { m_ea = arg16(); wm16(m_ea, m_bc); WZ = m_ea + 1; } /* LD   (w),BC      */
OP(ed,44) { neg();                                           } /* NEG              */
OP(ed,45) { retn();                                          } /* RETN             */
OP(ed,46) { m_im = 0;                                        } /* IM   0           */
OP(ed,47) { ld_i_a();                                        } /* LD   i,A         */

OP(ed,48) { C = in(BC); F = (F & CF) | SZP[C];               } /* IN   C,(C)       */
OP(ed,49) { out(BC, C);                                      } /* OUT  (C),C       */
OP(ed,4a) { adc_hl(m_bc);                                    } /* ADC  HL,BC       */
OP(ed,4b) { m_ea = arg16(); rm16(m_ea, m_bc); WZ = m_ea + 1; } /* LD   BC,(w)      */
OP(ed,4c) { neg();                                           } /* NEG              */
OP(ed,4d) { reti();                                          } /* RETI             */
OP(ed,4e) { m_im = 0;                                        } /* IM   0           */
OP(ed,4f) { ld_r_a();                                        } /* LD   r,A         */

OP(ed,50) { D = in(BC); F = (F & CF) | SZP[D];               } /* IN   D,(C)       */
OP(ed,51) { out(BC, D);                                      } /* OUT  (C),D       */
OP(ed,52) { sbc_hl(m_de);                                    } /* SBC  HL,DE       */
OP(ed,53) { m_ea = arg16(); wm16(m_ea, m_de); WZ = m_ea + 1; } /* LD   (w),DE      */
OP(ed,54) { neg();                                           } /* NEG              */
OP(ed,55) { retn();                                          } /* RETN             */
OP(ed,56) { m_im = 1;                                        } /* IM   1           */
OP(ed,57) { ld_a_i();                                        } /* LD   A,i         */

OP(ed,58) { E = in(BC); F = (F & CF) | SZP[E];               } /* IN   E,(C)       */
OP(ed,59) { out(BC, E);                                      } /* OUT  (C),E       */
OP(ed,5a) { adc_hl(m_de);                                    } /* ADC  HL,DE       */
OP(ed,5b) { m_ea = arg16(); rm16(m_ea, m_de); WZ = m_ea + 1; } /* LD   DE,(w)      */
OP(ed,5c) { neg();                                           } /* NEG              */
OP(ed,5d) { reti();                                          } /* RETI             */
OP(ed,5e) { m_im = 2;                                        } /* IM   2           */
OP(ed,5f) { ld_a_r();                                        } /* LD   A,r         */

OP(ed,60) { H = in(BC); F = (F & CF) | SZP[H];               } /* IN   H,(C)       */
OP(ed,61) { out(BC, H);                                      } /* OUT  (C),H       */
OP(ed,62) { sbc_hl(m_hl);                                    } /* SBC  HL,HL       */
OP(ed,63) { m_ea = arg16(); wm16(m_ea, m_hl); WZ = m_ea + 1; } /* LD   (w),HL      */
OP(ed,64) { neg();                                           } /* NEG              */
OP(ed,65) { retn();                                          } /* RETN             */
OP(ed,66) { m_im = 0;                                        } /* IM   0           */
OP(ed,67) { rrd();                                           } /* RRD  (HL)        */

OP(ed,68) { L = in(BC); F = (F & CF) | SZP[L];               } /* IN   L,(C)       */
OP(ed,69) { out(BC, L);                                      } /* OUT  (C),L       */
OP(ed,6a) { adc_hl(m_hl);                                    } /* ADC  HL,HL       */
OP(ed,6b) { m_ea = arg16(); rm16(m_ea, m_hl); WZ = m_ea + 1; } /* LD   HL,(w)      */
OP(ed,6c) { neg();                                           } /* NEG              */
OP(ed,6d) { reti();                                          } /* RETI             */
OP(ed,6e) { m_im = 0;                                        } /* IM   0           */
OP(ed,6f) { rld();                                           } /* RLD  (HL)        */

OP(ed,70) { UINT8 res = in(BC); F = (F & CF) | SZP[res];     } /* IN   0,(C)       */
OP(ed,71) { out(BC, 0);                                      } /* OUT  (C),0       */
OP(ed,72) { sbc_hl(m_sp);                                    } /* SBC  HL,SP       */
OP(ed,73) { m_ea = arg16(); wm16(m_ea, m_sp); WZ = m_ea + 1; } /* LD   (w),SP      */
OP(ed,74) { neg();                                           } /* NEG              */
OP(ed,75) { retn();                                          } /* RETN             */
OP(ed,76) { m_im = 1;                                        } /* IM   1           */
OP(ed,77) { illegal_2();                                     } /* DB   ED,77       */

OP(ed,78) { A = in(BC); F = (F & CF) | SZP[A]; WZ = BC + 1;  } /* IN   A,(C)       */
OP(ed,79) { out(BC, A);  WZ = BC + 1;                        } /* OUT  (C),A       */
OP(ed,7a) { adc_hl(m_sp);                                    } /* ADC  HL,SP       */
OP(ed,7b) { m_ea = arg16(); rm16(m_ea, m_sp); WZ = m_ea + 1; } /* LD   SP,(w)      */
OP(ed,7c) { neg();                                           } /* NEG              */
OP(ed,7d) { reti();                                          } /* RETI             */
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

OP(ed,a0) { ldi();                                           } /* LDI              */
OP(ed,a1) { cpi();                                           } /* CPI              */
OP(ed,a2) { ini();                                           } /* INI              */
OP(ed,a3) { outi();                                          } /* OUTI             */
OP(ed,a4) { illegal_2();                                     } /* DB   ED          */
OP(ed,a5) { illegal_2();                                     } /* DB   ED          */
OP(ed,a6) { illegal_2();                                     } /* DB   ED          */
OP(ed,a7) { illegal_2();                                     } /* DB   ED          */

OP(ed,a8) { ldd();                                           } /* LDD              */
OP(ed,a9) { cpd();                                           } /* CPD              */
OP(ed,aa) { ind();                                           } /* IND              */
OP(ed,ab) { outd();                                          } /* OUTD             */
OP(ed,ac) { illegal_2();                                     } /* DB   ED          */
OP(ed,ad) { illegal_2();                                     } /* DB   ED          */
OP(ed,ae) { illegal_2();                                     } /* DB   ED          */
OP(ed,af) { illegal_2();                                     } /* DB   ED          */

OP(ed,b0) { ldir();                                          } /* LDIR             */
OP(ed,b1) { cpir();                                          } /* CPIR             */
OP(ed,b2) { inir();                                          } /* INIR             */
OP(ed,b3) { otir();                                          } /* OTIR             */
OP(ed,b4) { illegal_2();                                     } /* DB   ED          */
OP(ed,b5) { illegal_2();                                     } /* DB   ED          */
OP(ed,b6) { illegal_2();                                     } /* DB   ED          */
OP(ed,b7) { illegal_2();                                     } /* DB   ED          */

OP(ed,b8) { lddr();                                          } /* LDDR             */
OP(ed,b9) { cpdr();                                          } /* CPDR             */
OP(ed,ba) { indr();                                          } /* INDR             */
OP(ed,bb) { otdr();                                          } /* OTDR             */
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
OP(op,01) { BC = arg16();                                                         } /* LD   BC,w        */
OP(op,02) { wm(BC,A); WZ_L = (BC + 1) & 0xFF;  WZ_H = A;                          } /* LD (BC),A */
OP(op,03) { BC++;                                                                 } /* INC  BC          */
OP(op,04) { B = inc(B);                                                           } /* INC  B           */
OP(op,05) { B = dec(B);                                                           } /* DEC  B           */
OP(op,06) { B = arg();                                                            } /* LD   B,n         */
OP(op,07) { rlca();                                                               } /* RLCA             */

OP(op,08) { ex_af();                                                              } /* EX   AF,AF'      */
OP(op,09) { add16(m_hl, m_bc);                                                    } /* ADD  HL,BC       */
OP(op,0a) { A = rm(BC);  WZ=BC+1;                                                 } /* LD   A,(BC)      */
OP(op,0b) { BC--;                                                                 } /* DEC  BC          */
OP(op,0c) { C = inc(C);                                                           } /* INC  C           */
OP(op,0d) { C = dec(C);                                                           } /* DEC  C           */
OP(op,0e) { C = arg();                                                            } /* LD   C,n         */
OP(op,0f) { rrca();                                                               } /* RRCA             */

OP(op,10) { B--; jr_cond(B, 0x10);                                                } /* DJNZ o           */
OP(op,11) { DE = arg16();                                                         } /* LD   DE,w        */
OP(op,12) { wm(DE,A); WZ_L = (DE + 1) & 0xFF;  WZ_H = A;                          } /* LD (DE),A */
OP(op,13) { DE++;                                                                 } /* INC  DE          */
OP(op,14) { D = inc(D);                                                           } /* INC  D           */
OP(op,15) { D = dec(D);                                                           } /* DEC  D           */
OP(op,16) { D = arg();                                                            } /* LD   D,n         */
OP(op,17) { rla();                                                                } /* RLA              */

OP(op,18) { jr();                                                                 } /* JR   o           */
OP(op,19) { add16(m_hl, m_de);                                                    } /* ADD  HL,DE       */
OP(op,1a) { A = rm(DE); WZ = DE + 1;                                              } /* LD   A,(DE)      */
OP(op,1b) { DE--;                                                                 } /* DEC  DE          */
OP(op,1c) { E = inc(E);                                                           } /* INC  E           */
OP(op,1d) { E = dec(E);                                                           } /* DEC  E           */
OP(op,1e) { E = arg();                                                            } /* LD   E,n         */
OP(op,1f) { rra();                                                                } /* RRA              */

OP(op,20) { jr_cond(!(F & ZF), 0x20);                                             } /* JR   NZ,o        */
OP(op,21) { HL = arg16();                                                         } /* LD   HL,w        */
OP(op,22) { m_ea = arg16(); wm16(m_ea, m_hl); WZ = m_ea + 1;                      } /* LD   (w),HL      */
OP(op,23) { HL++;                                                                 } /* INC  HL          */
OP(op,24) { H = inc(H);                                                           } /* INC  H           */
OP(op,25) { H = dec(H);                                                           } /* DEC  H           */
OP(op,26) { H = arg();                                                            } /* LD   H,n         */
OP(op,27) { daa();                                                                } /* DAA              */

OP(op,28) { jr_cond(F & ZF, 0x28);                                                } /* JR   Z,o         */
OP(op,29) { add16(m_hl, m_hl);                                                    } /* ADD  HL,HL       */
OP(op,2a) { m_ea = arg16(); rm16(m_ea, m_hl); WZ = m_ea+1;                        } /* LD   HL,(w)      */
OP(op,2b) { HL--;                                                                 } /* DEC  HL          */
OP(op,2c) { L = inc(L);                                                           } /* INC  L           */
OP(op,2d) { L = dec(L);                                                           } /* DEC  L           */
OP(op,2e) { L = arg();                                                            } /* LD   L,n         */
OP(op,2f) { A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); } /* CPL              */

OP(op,30) { jr_cond(!(F & CF), 0x30);                                             } /* JR   NC,o        */
OP(op,31) { SP = arg16();                                                         } /* LD   SP,w        */
OP(op,32) { m_ea = arg16(); wm(m_ea, A); WZ_L = (m_ea + 1) & 0xFF; WZ_H = A;      } /* LD   (w),A       */
OP(op,33) { SP++;                                                                 } /* INC  SP          */
OP(op,34) { wm(HL, inc(rm(HL)));                                                  } /* INC  (HL)        */
OP(op,35) { wm(HL, dec(rm(HL)));                                                  } /* DEC  (HL)        */
OP(op,36) { wm(HL, arg());                                                        } /* LD   (HL),n      */
OP(op,37) { F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF));            } /* SCF              */

OP(op,38) { jr_cond(F & CF, 0x38);                                                } /* JR   C,o         */
OP(op,39) { add16(m_hl, m_sp);                                                    } /* ADD  HL,SP       */
OP(op,3a) { m_ea = arg16(); A = rm(m_ea); WZ = m_ea + 1;                          } /* LD   A,(w)       */
OP(op,3b) { SP--;                                                                 } /* DEC  SP          */
OP(op,3c) { A = inc(A);                                                           } /* INC  A           */
OP(op,3d) { A = dec(A);                                                           } /* DEC  A           */
OP(op,3e) { A = arg();                                                            } /* LD   A,n         */
OP(op,3f) { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;             } /* CCF        */

OP(op,40) {                                                                       } /* LD   B,B         */
OP(op,41) { B = C;                                                                } /* LD   B,C         */
OP(op,42) { B = D;                                                                } /* LD   B,D         */
OP(op,43) { B = E;                                                                } /* LD   B,E         */
OP(op,44) { B = H;                                                                } /* LD   B,H         */
OP(op,45) { B = L;                                                                } /* LD   B,L         */
OP(op,46) { B = rm(HL);                                                           } /* LD   B,(HL)      */
OP(op,47) { B = A;                                                                } /* LD   B,A         */

OP(op,48) { C = B;                                                                } /* LD   C,B         */
OP(op,49) {                                                                       } /* LD   C,C         */
OP(op,4a) { C = D;                                                                } /* LD   C,D         */
OP(op,4b) { C = E;                                                                } /* LD   C,E         */
OP(op,4c) { C = H;                                                                } /* LD   C,H         */
OP(op,4d) { C = L;                                                                } /* LD   C,L         */
OP(op,4e) { C = rm(HL);                                                           } /* LD   C,(HL)      */
OP(op,4f) { C = A;                                                                } /* LD   C,A         */

OP(op,50) { D = B;                                                                } /* LD   D,B         */
OP(op,51) { D = C;                                                                } /* LD   D,C         */
OP(op,52) {                                                                       } /* LD   D,D         */
OP(op,53) { D = E;                                                                } /* LD   D,E         */
OP(op,54) { D = H;                                                                } /* LD   D,H         */
OP(op,55) { D = L;                                                                } /* LD   D,L         */
OP(op,56) { D = rm(HL);                                                           } /* LD   D,(HL)      */
OP(op,57) { D = A;                                                                } /* LD   D,A         */

OP(op,58) { E = B;                                                                } /* LD   E,B         */
OP(op,59) { E = C;                                                                } /* LD   E,C         */
OP(op,5a) { E = D;                                                                } /* LD   E,D         */
OP(op,5b) {                                                                       } /* LD   E,E         */
OP(op,5c) { E = H;                                                                } /* LD   E,H         */
OP(op,5d) { E = L;                                                                } /* LD   E,L         */
OP(op,5e) { E = rm(HL);                                                           } /* LD   E,(HL)      */
OP(op,5f) { E = A;                                                                } /* LD   E,A         */

OP(op,60) { H = B;                                                                } /* LD   H,B         */
OP(op,61) { H = C;                                                                } /* LD   H,C         */
OP(op,62) { H = D;                                                                } /* LD   H,D         */
OP(op,63) { H = E;                                                                } /* LD   H,E         */
OP(op,64) {                                                                       } /* LD   H,H         */
OP(op,65) { H = L;                                                                } /* LD   H,L         */
OP(op,66) { H = rm(HL);                                                           } /* LD   H,(HL)      */
OP(op,67) { H = A;                                                                } /* LD   H,A         */

OP(op,68) { L = B;                                                                } /* LD   L,B         */
OP(op,69) { L = C;                                                                } /* LD   L,C         */
OP(op,6a) { L = D;                                                                } /* LD   L,D         */
OP(op,6b) { L = E;                                                                } /* LD   L,E         */
OP(op,6c) { L = H;                                                                } /* LD   L,H         */
OP(op,6d) {                                                                       } /* LD   L,L         */
OP(op,6e) { L = rm(HL);                                                           } /* LD   L,(HL)      */
OP(op,6f) { L = A;                                                                } /* LD   L,A         */

OP(op,70) { wm(HL, B);                                                            } /* LD   (HL),B      */
OP(op,71) { wm(HL, C);                                                            } /* LD   (HL),C      */
OP(op,72) { wm(HL, D);                                                            } /* LD   (HL),D      */
OP(op,73) { wm(HL, E);                                                            } /* LD   (HL),E      */
OP(op,74) { wm(HL, H);                                                            } /* LD   (HL),H      */
OP(op,75) { wm(HL, L);                                                            } /* LD   (HL),L      */
OP(op,76) { halt();                                                               } /* halt             */
OP(op,77) { wm(HL, A);                                                            } /* LD   (HL),A      */

OP(op,78) { A = B;                                                                } /* LD   A,B         */
OP(op,79) { A = C;                                                                } /* LD   A,C         */
OP(op,7a) { A = D;                                                                } /* LD   A,D         */
OP(op,7b) { A = E;                                                                } /* LD   A,E         */
OP(op,7c) { A = H;                                                                } /* LD   A,H         */
OP(op,7d) { A = L;                                                                } /* LD   A,L         */
OP(op,7e) { A = rm(HL);                                                           } /* LD   A,(HL)      */
OP(op,7f) {                                                                       } /* LD   A,A         */

OP(op,80) { add_a(B);                                                             } /* ADD  A,B         */
OP(op,81) { add_a(C);                                                             } /* ADD  A,C         */
OP(op,82) { add_a(D);                                                             } /* ADD  A,D         */
OP(op,83) { add_a(E);                                                             } /* ADD  A,E         */
OP(op,84) { add_a(H);                                                             } /* ADD  A,H         */
OP(op,85) { add_a(L);                                                             } /* ADD  A,L         */
OP(op,86) { add_a(rm(HL));                                                        } /* ADD  A,(HL)      */
OP(op,87) { add_a(A);                                                             } /* ADD  A,A         */

OP(op,88) { adc_a(B);                                                             } /* ADC  A,B         */
OP(op,89) { adc_a(C);                                                             } /* ADC  A,C         */
OP(op,8a) { adc_a(D);                                                             } /* ADC  A,D         */
OP(op,8b) { adc_a(E);                                                             } /* ADC  A,E         */
OP(op,8c) { adc_a(H);                                                             } /* ADC  A,H         */
OP(op,8d) { adc_a(L);                                                             } /* ADC  A,L         */
OP(op,8e) { adc_a(rm(HL));                                                        } /* ADC  A,(HL)      */
OP(op,8f) { adc_a(A);                                                             } /* ADC  A,A         */

OP(op,90) { sub(B);                                                               } /* SUB  B           */
OP(op,91) { sub(C);                                                               } /* SUB  C           */
OP(op,92) { sub(D);                                                               } /* SUB  D           */
OP(op,93) { sub(E);                                                               } /* SUB  E           */
OP(op,94) { sub(H);                                                               } /* SUB  H           */
OP(op,95) { sub(L);                                                               } /* SUB  L           */
OP(op,96) { sub(rm(HL));                                                          } /* SUB  (HL)        */
OP(op,97) { sub(A);                                                               } /* SUB  A           */

OP(op,98) { sbc_a(B);                                                             } /* SBC  A,B         */
OP(op,99) { sbc_a(C);                                                             } /* SBC  A,C         */
OP(op,9a) { sbc_a(D);                                                             } /* SBC  A,D         */
OP(op,9b) { sbc_a(E);                                                             } /* SBC  A,E         */
OP(op,9c) { sbc_a(H);                                                             } /* SBC  A,H         */
OP(op,9d) { sbc_a(L);                                                             } /* SBC  A,L         */
OP(op,9e) { sbc_a(rm(HL));                                                        } /* SBC  A,(HL)      */
OP(op,9f) { sbc_a(A);                                                             } /* SBC  A,A         */

OP(op,a0) { and_a(B);                                                             } /* AND  B           */
OP(op,a1) { and_a(C);                                                             } /* AND  C           */
OP(op,a2) { and_a(D);                                                             } /* AND  D           */
OP(op,a3) { and_a(E);                                                             } /* AND  E           */
OP(op,a4) { and_a(H);                                                             } /* AND  H           */
OP(op,a5) { and_a(L);                                                             } /* AND  L           */
OP(op,a6) { and_a(rm(HL));                                                        } /* AND  (HL)        */
OP(op,a7) { and_a(A);                                                             } /* AND  A           */

OP(op,a8) { xor_a(B);                                                             } /* XOR  B           */
OP(op,a9) { xor_a(C);                                                             } /* XOR  C           */
OP(op,aa) { xor_a(D);                                                             } /* XOR  D           */
OP(op,ab) { xor_a(E);                                                             } /* XOR  E           */
OP(op,ac) { xor_a(H);                                                             } /* XOR  H           */
OP(op,ad) { xor_a(L);                                                             } /* XOR  L           */
OP(op,ae) { xor_a(rm(HL));                                                        } /* XOR  (HL)        */
OP(op,af) { xor_a(A);                                                             } /* XOR  A           */

OP(op,b0) { or_a(B);                                                              } /* OR   B           */
OP(op,b1) { or_a(C);                                                              } /* OR   C           */
OP(op,b2) { or_a(D);                                                              } /* OR   D           */
OP(op,b3) { or_a(E);                                                              } /* OR   E           */
OP(op,b4) { or_a(H);                                                              } /* OR   H           */
OP(op,b5) { or_a(L);                                                              } /* OR   L           */
OP(op,b6) { or_a(rm(HL));                                                         } /* OR   (HL)        */
OP(op,b7) { or_a(A);                                                              } /* OR   A           */

OP(op,b8) { cp(B);                                                                } /* CP   B           */
OP(op,b9) { cp(C);                                                                } /* CP   C           */
OP(op,ba) { cp(D);                                                                } /* CP   D           */
OP(op,bb) { cp(E);                                                                } /* CP   E           */
OP(op,bc) { cp(H);                                                                } /* CP   H           */
OP(op,bd) { cp(L);                                                                } /* CP   L           */
OP(op,be) { cp(rm(HL));                                                           } /* CP   (HL)        */
OP(op,bf) { cp(A);                                                                } /* CP   A           */

OP(op,c0) { ret_cond(!(F & ZF), 0xc0);                                            } /* RET  NZ          */
OP(op,c1) { pop(m_bc);                                                            } /* POP  BC          */
OP(op,c2) { jp_cond(!(F & ZF));                                                   } /* JP   NZ,a        */
OP(op,c3) { jp();                                                                 } /* JP   a           */
OP(op,c4) { call_cond(!(F & ZF), 0xc4);                                           } /* CALL NZ,a        */
OP(op,c5) { push(m_bc);                                                           } /* PUSH BC          */
OP(op,c6) { add_a(arg());                                                         } /* ADD  A,n         */
OP(op,c7) { rst(0x00);                                                            } /* RST  0           */

OP(op,c8) { ret_cond(F & ZF, 0xc8);                                               } /* RET  Z           */
OP(op,c9) { pop(m_pc); WZ = PCD;                                                  } /* RET              */
OP(op,ca) { jp_cond(F & ZF);                                                      } /* JP   Z,a         */
OP(op,cb) { m_r++; EXEC(cb,rop());                                                } /* **** CB xx       */
OP(op,cc) { call_cond(F & ZF, 0xcc);                                              } /* CALL Z,a         */
OP(op,cd) { call();                                                               } /* CALL a           */
OP(op,ce) { adc_a(arg());                                                         } /* ADC  A,n         */
OP(op,cf) { rst(0x08);                                                            } /* RST  1           */

OP(op,d0) { ret_cond(!(F & CF), 0xd0);                                            } /* RET  NC          */
OP(op,d1) { pop(m_de);                                                            } /* POP  DE          */
OP(op,d2) { jp_cond(!(F & CF));                                                   } /* JP   NC,a        */
OP(op,d3) { unsigned n = arg() | (A << 8); out(n, A); WZ_L = ((n & 0xff) + 1) & 0xff;  WZ_H = A;   } /* OUT  (n),A       */
OP(op,d4) { call_cond(!(F & CF), 0xd4);                                           } /* CALL NC,a        */
OP(op,d5) { push(m_de);                                                           } /* PUSH DE          */
OP(op,d6) { sub(arg());                                                           } /* SUB  n           */
OP(op,d7) { rst(0x10);                                                            } /* RST  2           */

OP(op,d8) { ret_cond(F & CF, 0xd8);                                               } /* RET  C           */
OP(op,d9) { exx();                                                                } /* EXX              */
OP(op,da) { jp_cond(F & CF);                                                      } /* JP   C,a         */
OP(op,db) { unsigned n = arg() | (A << 8); A = in(n); WZ = n + 1;                 } /* IN   A,(n)  */
OP(op,dc) { call_cond(F & CF, 0xdc);                                              } /* CALL C,a         */
OP(op,dd) { m_r++; EXEC(dd,rop());                                                } /* **** DD xx       */
OP(op,de) { sbc_a(arg());                                                         } /* SBC  A,n         */
OP(op,df) { rst(0x18);                                                            } /* RST  3           */

OP(op,e0) { ret_cond(!(F & PF), 0xe0);                                            } /* RET  PO          */
OP(op,e1) { pop(m_hl);                                                            } /* POP  HL          */
OP(op,e2) { jp_cond(!(F & PF));                                                   } /* JP   PO,a        */
OP(op,e3) { ex_sp(m_hl);                                                          } /* EX   HL,(SP)     */
OP(op,e4) { call_cond(!(F & PF), 0xe4);                                           } /* CALL PO,a        */
OP(op,e5) { push(m_hl);                                                           } /* PUSH HL          */
OP(op,e6) { and_a(arg());                                                         } /* AND  n           */
OP(op,e7) { rst(0x20);                                                            } /* RST  4           */

OP(op,e8) { ret_cond(F & PF, 0xe8);                                               } /* RET  PE          */
OP(op,e9) { PC = HL;                                                              } /* JP   (HL)        */
OP(op,ea) { jp_cond(F & PF);                                                      } /* JP   PE,a        */
OP(op,eb) { ex_de_hl();                                                           } /* EX   DE,HL       */
OP(op,ec) { call_cond(F & PF, 0xec);                                              } /* CALL PE,a        */
OP(op,ed) { m_r++; EXEC(ed,rop());                                                } /* **** ED xx       */
OP(op,ee) { xor_a(arg());                                                         } /* XOR  n           */
OP(op,ef) { rst(0x28);                                                            } /* RST  5           */

OP(op,f0) { ret_cond(!(F & SF), 0xf0);                                            } /* RET  P           */
OP(op,f1) { pop(m_af);                                                            } /* POP  AF          */
OP(op,f2) { jp_cond(!(F & SF));                                                   } /* JP   P,a         */
OP(op,f3) { m_iff1 = m_iff2 = 0;                                                  } /* DI               */
OP(op,f4) { call_cond(!(F & SF), 0xf4);                                           } /* CALL P,a         */
OP(op,f5) { push(m_af);                                                           } /* PUSH AF          */
OP(op,f6) { or_a(arg());                                                          } /* OR   n           */
OP(op,f7) { rst(0x30);                                                            } /* RST  6           */

OP(op,f8) { ret_cond(F & SF, 0xf8);                                               } /* RET  M           */
OP(op,f9) { SP = HL;                                                              } /* LD   SP,HL       */
OP(op,fa) { jp_cond(F & SF);                                                      } /* JP   M,a         */
OP(op,fb) { ei();                                                                 } /* EI               */
OP(op,fc) { call_cond(F & SF, 0xfc);                                              } /* CALL M,a         */
OP(op,fd) { m_r++; EXEC(fd,rop());                                                } /* **** FD xx       */
OP(op,fe) { cp(arg());                                                            } /* CP   n           */
OP(op,ff) { rst(0x38);                                                            } /* RST  7           */


void z80_device::take_interrupt()
{
	int irq_vector;

	/* there isn't a valid previous program counter */
	PRVPC = -1;

	/* Check if processor was halted */
	leave_halt();

	/* Clear both interrupt flip flops */
	m_iff1 = m_iff2 = 0;

	/* Daisy chain mode? If so, call the requesting device */
	if (m_daisy.present())
		irq_vector = m_daisy.call_ack_device();

	/* else call back the cpu interface to retrieve the vector */
	else
		irq_vector = m_irq_callback(*this, 0);

	LOG(("Z80 '%s' single int. irq_vector $%02x\n", tag(), irq_vector));

	/* Interrupt mode 2. Call [i:databyte] */
	if( m_im == 2 )
	{
		irq_vector = (irq_vector & 0xff) | (m_i << 8);
		push(m_pc);
		rm16(irq_vector, m_pc);
		LOG(("Z80 '%s' IM2 [$%04x] = $%04x\n", tag(), irq_vector, PCD));
		/* CALL opcode timing + 'interrupt latency' cycles */
		m_icount -= m_cc_op[0xcd] + m_cc_ex[0xff];
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( m_im == 1 )
	{
		LOG(("Z80 '%s' IM1 $0038\n", tag()));
		push(m_pc);
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
					push(m_pc);
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
					push(m_pc);
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
	leave_halt();

	/* Clear both interrupt flip flops */
	m_iff1 = m_iff2 = 0;

	if (m_nsc800_irq_state[NSC800_RSTA])
	{
		push(m_pc);
		PCD = 0x003c;
	}
	else if (m_nsc800_irq_state[NSC800_RSTB])
	{
		push(m_pc);
		PCD = 0x0034;
	}
	else if (m_nsc800_irq_state[NSC800_RSTC])
	{
		push(m_pc);
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
	m_decrypted_opcodes = has_space(AS_DECRYPTED_OPCODES) ? &space(AS_DECRYPTED_OPCODES) : m_program;
	m_direct = &m_program->direct();
	m_decrypted_opcodes_direct = &m_decrypted_opcodes->direct();
	m_io = &space(AS_IO);

	if (static_config() != NULL)
		m_daisy.init(this, (const z80_daisy_config *)static_config());
	m_irq_callback = device_irq_acknowledge_delegate(FUNC(z80_device::standard_irq_callback_member), this);

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
		leave_halt();            /* Check if processor was halted */

#if HAS_LDAIR_QUIRK
		/* reset parity flag after LD A,I or LD A,R */
		if (m_after_ldair) F &= ~PF;
#endif
		m_after_ldair = FALSE;

		m_iff1 = 0;
		push(m_pc);
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
		EXEC(op,rop());
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
		leave_halt();            /* Check if processor was halted */

		m_iff1 = 0;
		push(m_pc);
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
		EXEC(op,rop());
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
	}
}

void z80_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
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
	m_decrypted_opcodes_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_decrypted_opcodes_config("decrypted_opcodes", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
{
}

const address_space_config *z80_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_IO:                return &m_io_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &m_decrypted_opcodes_config : NULL;
	default:                   return NULL;
	}
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
