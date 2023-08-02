// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   z80.cpp
 *   Portable Z80 emulator V3.9
 *
 *   TODO:
 *    - Interrupt mode 0 should be able to execute arbitrary opcodes
 *    - If LD A,I or LD A,R is interrupted, P/V flag gets reset, even if IFF2
 *      was set before this instruction (implemented, but not enabled: we need
 *      document Z80 types first, see below)
 *    - WAIT only stalls between instructions now, it should stall immediately.
 *    - Ideally, the tiny differences between Z80 types should be supported,
 *      currently known differences:
 *       - LD A,I/R P/V flag reset glitch is fixed on CMOS Z80
 *       - OUT (C),0 outputs 0 on NMOS Z80, $FF on CMOS Z80
 *       - SCF/CCF X/Y flags is ((flags | A) & 0x28) on SGS/SHARP/ZiLOG NMOS Z80,
 *         (flags & A & 0x28) on NEC NMOS Z80, other models unknown.
 *         However, recent findings say that SCF/CCF X/Y results depend on whether
 *         or not the previous instruction touched the flag register. And the exact
 *         behaviour on NEC Z80 is still unknown.
 *      This Z80 emulator assumes a ZiLOG NMOS model.
 *
 *   Changes in 0.243:
 *    Foundation for M cycles emulation. Currently we preserve cc_* tables with total timings.
 *    execute_run() behavior (simplified) ...
 *    Before:
 *      + fetch opcode
 *      + call EXEC()
 *        + adjust icount base on cc_* (all T are used after M1 == wrong)
 *          + execute instruction
 *    Now:
 *      + fetch opcode
 *      + call EXEC()
          + execute instruction adjusting icount per each Read (arg(), recursive rop()) and Write
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
 *      might access the old (wrong or even nullptr) banked memory region.
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
 *    - Changed variable ea and arg16() function to u32; this
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
#include "z80.h"
#include "z80dasm.h"

#define VERBOSE             0

/* On an NMOS Z80, if LD A,I or LD A,R is interrupted, P/V flag gets reset,
   even if IFF2 was set before this instruction. This issue was fixed on
   the CMOS Z80, so until knowing (most) Z80 types on hardware, it's disabled */
#define HAS_LDAIR_QUIRK     0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/****************************************************************************/
/* The Z80 registers. halt is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(r&127)|(r2&128)              */
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


#define TADR     m_m_shared_addr.w   // Typically represents values from A0..15 pins. 16bit input in steps.
#define TADR_H   m_m_shared_addr.b.h
#define TADR_L   m_m_shared_addr.b.l
#define TDAT     m_m_shared_data.w   // 16bit input(if use as second parameter) or output in steps.
#define TDAT2    m_m_shared_data2.w
#define TDAT_H   m_m_shared_data.b.h
#define TDAT_L   m_m_shared_data.b.l
#define TDAT8    m_m_shared_data.b.l // Typically represents values from D0..8 pins. 8bit input or output in steps.

static bool tables_initialised = false;
static u8 SZ[256];       /* zero and sign flags */
static u8 SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static u8 SZP[256];      /* zero, sign and parity flags */
static u8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static u8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static u8 SZHVC_add[2*256*256];
static u8 SZHVC_sub[2*256*256];

static const u8 cc_op[0x100] = {
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
	5,10,10,10,10,11, 7,11, 5,10,10, 4,10,17, 7,11, /* cb -> cc_cb */
	5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 4, 7,11, /* dd -> cc_xy */
	5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 4, 7,11, /* ed -> cc_ed */
	5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 4, 7,11  /* fd -> cc_xy */
};

static const u8 cc_cb[0x100] = {
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4,
	4, 4, 4, 4, 4, 4,11, 4, 4, 4, 4, 4, 4, 4,11, 4
};

static const u8 cc_ed[0x100] = {
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	8, 8,11,16, 4,10, 4, 5, 8, 8,11,16, 4,10, 4, 5,
	8, 8,11,16, 4,10, 4, 5, 8, 8,11,16, 4,10, 4, 5,
	8, 8,11,16, 4,10, 4,14, 8, 8,11,16, 4,10, 4,14,
	8, 8,11,16, 4,10, 4, 4, 8, 8,11,16, 4,10, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	12,12,12,12,4, 4, 4, 4,12,12,12,12, 4, 4, 4, 4,
	12,12,12,12,4, 4, 4, 4,12,12,12,12, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

/* ix/iy: with the exception of (i+offset) opcodes, for total add t-states from main_opcode_table[DD/FD] == 4 */
static const u8 cc_xy[0x100] = {
	 4,10, 7, 6, 4, 4, 7, 4, 4,11, 7, 6, 4, 4, 7, 4,
	 8,10, 7, 6, 4, 4, 7, 4,12,11, 7, 6, 4, 4, 7, 4,
	 7,10,16, 6, 4, 4, 7, 4, 7,11,16, 6, 4, 4, 7, 4,
	 7,10,13, 6,19,19,15, 4, 7,11,13, 6, 4, 4, 7, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	15,15,15,15,15,15, 4,15, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 4, 4, 4, 4, 4, 4,15, 4, 4, 4, 4, 4, 4, 4,15, 4,
	 5,10,10,10,10,11, 7,11, 5,10,10, 7,10,17, 7,11, /* cb -> cc_xycb */
	 5,10,10,11,10,11, 7,11, 5, 4,10,11,10, 4, 7,11, /* dd -> cc_xy again */
	 5,10,10,19,10,11, 7,11, 5, 4,10, 4,10, 4, 7,11, /* ed -> cc_ed */
	 5,10,10, 4,10,11, 7,11, 5, 6,10, 4,10, 4, 7,11  /* fd -> cc_xy again */
};

static const u8 cc_xycb[0x100] = {
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
	12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12
};

/* extra cycles if jr/jp/call taken and 'interrupt latency' on rst 0-7 */
static const u8 cc_ex[0x100] = {
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
 * define an opcode builder helpers
 ***************************************************************/
#define ST_F op_builder(*this).add([&]()
#define ST_M op_builder(*this).add(
#define DOIF )->do_if([&]() -> bool
#define DOELSE )->do_else()->add(
#define EDO )->edo(
#define EST )->get_steps();
#define FN )->add([&]()
#define MN )->add(

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) do { m_icount_executing += m_cc_##prefix == nullptr ? 0 : m_cc_##prefix[opcode]; } while (0)
// Defines cycles used by mread/mwrite/in/out (excluding opcode_fetch)
#define MTM (3*m_cycles_multiplier)
#define T(icount) execute_cycles(icount)

/***************************************************************
 * Enter halt state; write 1 to callback on first execution
 ***************************************************************/
inline void z80_device::halt()
{
	if (!m_halt)
	{
		m_halt = 1;
		m_halt_cb(1);
	}
}

/***************************************************************
 * Leave halt state; write 0 to callback
 ***************************************************************/
inline void z80_device::leave_halt()
{
	if (m_halt)
	{
		m_halt = 0;
		m_halt_cb(0);
	}
}

/***************************************************************
 * Input a byte from given I/O port
 ***************************************************************/
z80_device::ops_type z80_device::in()
{
	return ST_F {
		TDAT8 = m_io.read_byte(TADR);
		T(4 * m_cycles_multiplier);   } EST
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
z80_device::ops_type z80_device::out()
{
	return ST_F {
		m_io.write_byte(TADR, TDAT8);
		T(4 * m_cycles_multiplier);   } EST
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
u8 z80_device::data_read(u16 addr)
{
	const u8 tmp = m_data.read_byte(translate_memory_address(addr));
	T(MTM);
	return tmp;
}

z80_device::ops_type z80_device::rm()
{
	return ST_F {
		TDAT8 = data_read(TADR); } EST
}

z80_device::ops_type z80_device::rm_reg()
{
	return ST_M
		rm()           MN
		nomreq_addr(1) EST
}

/***************************************************************
 * Read a word from given memory location
 *  in: TADR
 * out: TDAT
 ***************************************************************/
z80_device::ops_type z80_device::rm16()
{
	return ST_M
		rm()                       FN {
		TDAT_H = TDAT_L;
		TADR++;                    } MN
		rm()                       FN {
		std::swap(TDAT_H, TDAT_L); } EST
}

inline void z80_device::rm16(uint16_t addr, PAIR &r)
{
	r.b.l = data_read(addr);
	r.b.h = data_read(addr+1);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
void z80_device::data_write(u16 addr, u8 value) {
	m_data.write_byte(translate_memory_address((u32)addr), value);
	T(MTM);
}

z80_device::ops_type z80_device::wm()
{
	return ST_F {
		// As we don't count changes between read and write, simply adjust to the end of requested.
		if (m_icount_executing != MTM) T(m_icount_executing - MTM);
		data_write(TADR, TDAT8);                                    } EST
}


/***************************************************************
 * Write a word to given memory location
 *  in: TADR, TDAT
 ***************************************************************/
z80_device::ops_type z80_device::wm16()
{
	return ST_F {
		m_icount_executing -= MTM; } MN
		wm()                       FN {
		m_icount_executing += MTM;
		TADR++;
		TDAT8=TDAT_H;              } MN
	    wm()                       EST
}

/***************************************************************
 * Write a word to (SP)
 *  in: TDAT
 ***************************************************************/
z80_device::ops_type z80_device::wm16_sp()
{
	return ST_F {
		SP--;                                                       } FN {
		m_icount_executing -= MTM;
		if (m_icount_executing != MTM) T(m_icount_executing - MTM);
		data_write(SPD, TDAT_H);
		m_icount_executing += MTM;                                  } FN {
		SP--;                                                       } FN {
		data_write(SPD, TDAT_L);                                    } EST
}

inline void z80_device::wm16_sp(PAIR &r)
{
	m_icount_executing -= MTM;
	if (m_icount_executing != MTM) T(m_icount_executing - MTM);
	SP--;
	data_write(SPD, r.b.h);
	m_icount_executing += MTM;
	SP--;
	data_write(SPD, r.b.l);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
u8 z80_device::opcode_read()
{
	const u8 tmp = m_opcodes.read_byte(translate_memory_address(PCD));
	T(get_opfetch_cycles() - (2 * m_cycles_multiplier));
	return tmp;
}

z80_device::ops_type z80_device::rop()
{
	return ST_F {
		TDAT8 = opcode_read();                                               } FN {
		m_refresh_cb((m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f), 0x00, 0xff);
		T(2 * m_cycles_multiplier);                                          } FN {
		PC++;
		m_r++;                                                               } EST
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 * out: TDAT8
 ***************************************************************/
u8 z80_device::arg_read()
{
	const u8 tmp = m_args.read_byte(translate_memory_address(PCD));
	T(MTM);
	return tmp;
}

z80_device::ops_type z80_device::arg()
{
	return ST_F {
		TDAT8 = arg_read(); } FN {
		PC++;               } EST
}

z80_device::ops_type z80_device::arg16()
{
	return ST_M
		arg()                      FN {
		TDAT_H = TDAT_L;           } MN
		arg()                      FN {
		std::swap(TDAT_H, TDAT_L); } EST
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
z80_device::ops_type z80_device::eax()
{
	return ST_M
		arg()                              FN {
		m_ea = (u32)(u16)(IX + (s8)TDAT8);
		WZ = m_ea;                         } EST
}

z80_device::ops_type z80_device::eay()
{
	return ST_M
		arg()                              FN {
		m_ea = (u32)(u16)(IY + (s8)TDAT8);
		WZ = m_ea;                         } EST
}

/***************************************************************
 * POP
 ***************************************************************/
z80_device::ops_type z80_device::pop()
{
	return ST_F {
		TDAT_L = data_read(SPD); } FN {
		SP++;                    } FN {
		TDAT_H = data_read(SPD); } FN {
		SP++;                    } EST
}

/***************************************************************
 * PUSH
 *  in: TDAT
 ***************************************************************/
z80_device::ops_type z80_device::push()
{
	return ST_M
		nomreq_ir(1) MN
		wm16_sp()    EST
}

/***************************************************************
 * JP
 ***************************************************************/
z80_device::ops_type z80_device::jp()
{
	return ST_M
		arg16()     FN {
		PCD = TDAT;
		WZ = PC;    } EST
}

/***************************************************************
 * JP_COND
 ***************************************************************/
z80_device::ops_type z80_device::jp_cond()
{
	return ST_M
		DOIF { return TDAT8; } MN
			arg16()		     FN {
			PC=TDAT;
			WZ=PCD;          }
		DOELSE
			/* implicit do PC += 2 */
			arg16()		     FN {
			WZ=TDAT;         }
		EDO EST
}

/***************************************************************
 * JR
 ***************************************************************/
z80_device::ops_type z80_device::jr()
{
	return ST_M
		arg()            FN {
		TADR=PCD-1;      } MN
		nomreq_addr(5)   FN {
		PC += (s8)TDAT8;
		WZ = PC;         } EST
}

/***************************************************************
 * JR_COND
 ***************************************************************/
z80_device::ops_type z80_device::jr_cond(u8 opcode)
{
	return ST_M
		DOIF { return TDAT8; } )->add([&, opcode]() {
			CC(ex, opcode);  } MN
			jr()
		DOELSE
			arg()
		EDO EST
}

/***************************************************************
 * CALL
 ***************************************************************/
z80_device::ops_type z80_device::call()
{
	return ST_M
		arg16()        FN {
		m_ea=TDAT;
		TADR=PCD-1;    } MN
		nomreq_addr(1) FN {
		WZ = m_ea;
		TDAT=PC;       } MN
		wm16_sp()      FN {
		PCD=m_ea;      } EST
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
z80_device::ops_type z80_device::call_cond(u8 opcode)
{
	return ST_M
		DOIF { return TDAT8; } )->add([&, opcode]() {
			CC(ex, opcode);    } MN
			call()
		DOELSE
			arg16()            FN {
			WZ=TDAT;           }
		EDO EST
}

/***************************************************************
 * RET_COND
 ***************************************************************/
z80_device::ops_type z80_device::ret_cond(u8 opcode)
{
	return ST_M
		nomreq_ir(1)
		DOIF { return TDAT8; } )->add([&, opcode]() {
			CC(ex, opcode);    } MN
			pop()			   FN {
			PC=TDAT;
			WZ = PC;           }
		EDO EST
}

/***************************************************************
 * RETN
 ***************************************************************/
z80_device::ops_type z80_device::retn()
{
	return ST_M
		pop()                     FN {
		PC=TDAT;
		LOG(("Z80 RETN m_iff1:%d m_iff2:%d\n", m_iff1, m_iff2));
		WZ = PC;
		m_iff1 = m_iff2;          } EST
}

/***************************************************************
 * RETI
 ***************************************************************/
z80_device::ops_type z80_device::reti()
{
	return ST_M
		pop()                     FN {
		PC=TDAT;
		WZ = PC;
		m_iff1 = m_iff2;
		daisy_call_reti_device(); } EST
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
z80_device::ops_type z80_device::ld_r_a()
{
	return ST_M
		nomreq_ir(1)                           FN {
		m_r = A;
		m_r2 = A & 0x80; /* keep bit 7 of r */ } EST
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
z80_device::ops_type z80_device::ld_a_r()
{
	return ST_M
		nomreq_ir(1)                          FN {
		A = (m_r & 0x7f) | m_r2;
		F = (F & CF) | SZ[A] | (m_iff2 << 2);
		m_after_ldair = true;                 } EST
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
z80_device::ops_type z80_device::ld_i_a()
{
	return ST_M
		nomreq_ir(1) FN {
		m_i = A;     } EST
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
z80_device::ops_type z80_device::ld_a_i()
{
	return ST_M
		nomreq_ir(1)                          FN {
		A = m_i;
		F = (F & CF) | SZ[A] | (m_iff2 << 2);
		m_after_ldair = true;                 } EST
}

/***************************************************************
 * RST
 ***************************************************************/
z80_device::ops_type z80_device::rst(u16 addr)
{
	return ST_F {
		TDAT=PC;  } MN
		push()    )->add([&, addr]() {
		PC=addr;
		WZ=PC;    } EST
}

/***************************************************************
 * INC  r8
 ***************************************************************/
inline void z80_device::inc(u8 &r)
{
	++r;
	F = (F & CF) | SZHV_inc[r];
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline void z80_device::dec(u8 &r)
{
	--r;
	F = (F & CF) | SZHV_dec[r];
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
	u8 res = (A << 1) | (F & CF);
	u8 c = (A & 0x80) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void z80_device::rra()
{
	u8 res = (A >> 1) | (F << 7);
	u8 c = (A & 0x01) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
z80_device::ops_type z80_device::rrd()
{
	return ST_F {
		TADR=HL;                          } MN
		rm()                              FN {
		WZ = HL+1;                        } MN
		nomreq_addr(4)                    FN {
		TDAT_H=TDAT8;
		TDAT8=(TDAT8 >> 4) | (A << 4);    } MN
		wm()                              FN {
		A = (A & 0xf0) | (TDAT_H & 0x0f);
		F = (F & CF) | SZP[A];            } EST
}

/***************************************************************
 * RLD
 ***************************************************************/
z80_device::ops_type z80_device::rld()
{
	return ST_F {
		TADR=HL;                          } MN
		rm()                              FN {
		WZ = HL+1;                        } MN
		nomreq_addr(4)                    FN {
		TDAT_H=TDAT8;
		TDAT8=(TDAT8 << 4) | (A & 0x0f);  } MN
		wm()                              FN {
		A = (A & 0xf0) | (TDAT_H >> 4);
		F = (F & CF) | SZP[A];            } EST
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void z80_device::add_a(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) + value);
	F = SZHVC_add[ah | res];
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void z80_device::adc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) + value + c);
	F = SZHVC_add[(c << 16) | ah | res];
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void z80_device::sub(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - value);
	F = SZHVC_sub[ah | res];
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void z80_device::sbc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) - value - c);
	F = SZHVC_sub[(c<<16) | ah | res];
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
inline void z80_device::neg()
{
	u8 value = A;
	A = 0;
	sub(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
inline void z80_device::daa()
{
	u8 a = A;
	if (F & NF)
	{
		if ((F&HF) | ((A&0xf)>9)) a-=6;
		if ((F&CF) | (A>0x99)) a-=0x60;
	}
	else
	{
		if ((F&HF) | ((A&0xf)>9)) a+=6;
		if ((F&CF) | (A>0x99)) a+=0x60;
	}

	F = (F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a];
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
inline void z80_device::and_a(u8 value)
{
	A &= value;
	F = SZP[A] | HF;
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void z80_device::or_a(u8 value)
{
	A |= value;
	F = SZP[A];
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void z80_device::xor_a(u8 value)
{
	A ^= value;
	F = SZP[A];
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void z80_device::cp(u8 value)
{
	unsigned val = value;
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - val);
	F = (SZHVC_sub[ah | res] & ~(YF | XF)) |
		(val & (YF | XF));
}

/***************************************************************
 * EXX
 ***************************************************************/
inline void z80_device::exx()
{
	std::swap(m_bc, m_bc2);
	std::swap(m_de, m_de2);
	std::swap(m_hl, m_hl2);
}

/***************************************************************
 * EX   (SP),r16
 *  in: TDAT
 ***************************************************************/
z80_device::ops_type z80_device::ex_sp()
{
	return ST_F {
		TDAT2=TDAT;                    } MN
		pop()                          FN {
		TADR=SP-1;                     } MN
		nomreq_addr(1)                 FN {
		std::swap(TDAT, TDAT2);
		m_icount_executing -= 2;       } MN
		wm16_sp()                      FN {
		m_icount_executing += 2;
		TADR=SP;                       } MN
		nomreq_addr(2)                 FN {
		std::swap(TDAT, TDAT2);
		WZ=TDAT;                       } EST
}

/***************************************************************
 * ADD16
 ***************************************************************/
z80_device::ops_type z80_device::add16()
{
	return ST_M
		nomreq_ir(7)                                       FN {
		u32 res = TDAT + TDAT2;
		WZ = TDAT + 1;
		F = (F & (SF | ZF | VF)) |
			(((TDAT ^ res ^ TDAT2) >> 8) & HF) |
			((res >> 16) & CF) | ((res >> 8) & (YF | XF));
		TDAT = (u16)res;                                   } EST
}

/***************************************************************
 * ADC  HL,r16
 ***************************************************************/
z80_device::ops_type z80_device::adc_hl()
{
	return ST_M
		nomreq_ir(7)                                                 FN {
		u32 res = HLD + TDAT + (F & CF);
		WZ = HL + 1;
		F = (((HLD ^ res ^ TDAT) >> 8) & HF) |
			((res >> 16) & CF) |
			((res >> 8) & (SF | YF | XF)) |
			((res & 0xffff) ? 0 : ZF) |
			(((TDAT ^ HLD ^ 0x8000) & (TDAT ^ res) & 0x8000) >> 13);
		HL = (u16)res;                                               } EST
}

/***************************************************************
 * SBC  HL,r16
 ***************************************************************/
z80_device::ops_type z80_device::sbc_hl()
{
	return ST_M
		nomreq_ir(7)                                      FN {
		u32 res = HLD - TDAT - (F & CF);
		WZ = HL + 1;
		F = (((HLD ^ res ^ TDAT) >> 8) & HF) | NF |
			((res >> 16) & CF) |
			((res >> 8) & (SF | YF | XF)) |
			((res & 0xffff) ? 0 : ZF) |
			(((TDAT ^ HLD) & (HLD ^ res) &0x8000) >> 13);
		HL = (u16)res;                                    } EST
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
inline u8 z80_device::rlc(u8 value)
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
inline u8 z80_device::rrc(u8 value)
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
inline u8 z80_device::rl(u8 value)
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
inline u8 z80_device::rr(u8 value)
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
inline u8 z80_device::sla(u8 value)
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
inline u8 z80_device::sra(u8 value)
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
inline u8 z80_device::sll(u8 value)
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
inline u8 z80_device::srl(u8 value)
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
inline void z80_device::bit(int bit, u8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (value & (YF|XF));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void z80_device::bit_hl(int bit, u8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void z80_device::bit_xy(int bit, u8 value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | ((m_ea>>8) & (YF|XF));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
inline u8 z80_device::res(int bit, u8 value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
inline u8 z80_device::set(int bit, u8 value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
z80_device::ops_type z80_device::ldi()
{
	return ST_F {
		TADR=HL;                         } MN
		rm()                             FN {
		m_icount_executing -= 2;
		TADR=DE;                         } MN
		wm()                             FN {
		m_icount_executing += 2;         } MN
		nomreq_addr(2)                   FN {
		F &= SF | ZF | CF;
		if ((A + TDAT8) & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if ((A + TDAT8) & 0x08) F |= XF; /* bit 3 -> flag 3 */
		HL++; DE++; BC--;
		if(BC) F |= VF;                  } EST
}

/***************************************************************
 * CPI
 ***************************************************************/
z80_device::ops_type z80_device::cpi()
{
	return ST_F {
		TADR=HL;                    } MN
		rm()                        MN
		nomreq_addr(5)              FN {
		u8 res = A - TDAT8;
		WZ++;
		HL++; BC--;
		F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^TDAT8^res)&HF) | NF;
		if (F & HF) res -= 1;
		if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
		if (BC) F |= VF;            } EST
}

/***************************************************************
 * INI
 ***************************************************************/
z80_device::ops_type z80_device::ini()
{
	return ST_M
		nomreq_ir(1)                                               FN {
		TADR=BC;                                                   } MN
		in() FN {
		WZ = BC + 1;
		B--;
		TADR=HL;                                                   } MN
		wm()                                                       FN {
		HL++;
		F = SZ[B];
		unsigned t = (unsigned)((C + 1) & 0xff) + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;                         } EST
}

/***************************************************************
 * OUTI
 ***************************************************************/
z80_device::ops_type z80_device::outi()
{
	return ST_M
		nomreq_ir(1)                             FN {
		TADR=HL;                                 } MN
		rm()                                     FN {
		B--;
		WZ = BC + 1;
		TADR=BC;                                 } MN
		out()                                    FN {
		HL++;
		F = SZ[B];
		unsigned t = (unsigned)L + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;       } EST
}

/***************************************************************
 * LDD
 ***************************************************************/
z80_device::ops_type z80_device::ldd()
{
	return ST_F {
		TADR=HL;                      } MN
		rm()                          FN {
		m_icount_executing -= 2;
		TADR=DE;                      } MN
		wm()                          FN {
		m_icount_executing += 2;      } MN
		nomreq_addr(2)                FN {
		F &= SF | ZF | CF;
		if ((A + TDAT8) & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if ((A + TDAT8) & 0x08) F |= XF; /* bit 3 -> flag 3 */
		HL--; DE--; BC--;
		if (BC) F |= VF;              } EST
}

/***************************************************************
 * CPD
 ***************************************************************/
z80_device::ops_type z80_device::cpd()
{
	return ST_F {
		TADR=HL;                                                   } MN
		rm()                                                       FN {
		TADR=HL;                                                   } MN
		nomreq_addr(5)                                             FN {
		u8 res = A - TDAT8;
		WZ--;
		HL--; BC--;
		F = (F & CF) | (SZ[res]&~(YF|XF)) | ((A^TDAT8^res)&HF) | NF;
		if (F & HF) res -= 1;
		if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
		if (BC) F |= VF;                                           } EST
}

/***************************************************************
 * IND
 ***************************************************************/
z80_device::ops_type z80_device::ind()
{
	return ST_M
		nomreq_ir(1)                                        FN {
		TADR=BC;                                            } MN
		in()                                                FN {
		WZ = BC - 1;
		B--;
		TADR=HL;                                            } MN
		wm()                                                FN {
		HL--;
		F = SZ[B];
		unsigned t = ((unsigned)(C - 1) & 0xff) + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;                  } EST
}

/***************************************************************
 * OUTD
 ***************************************************************/
z80_device::ops_type z80_device::outd()
{
	return ST_M
		nomreq_ir(1)                                FN {
		TADR=HL;                                    } MN
		rm()                                        FN {
		B--;
		WZ = BC - 1;
		TADR=BC;                                    } MN
		out()                                       FN {
		HL--;
		F = SZ[B];
		unsigned t = (unsigned)L + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;          } EST
}

/***************************************************************
 * LDIR
 ***************************************************************/
z80_device::ops_type z80_device::ldir()
{
	return ST_M
		ldi()
		DOIF { return BC != 0; } FN {
			CC(ex, 0xb0);
			TADR=DE;           } MN
			nomreq_addr(5)     FN {
			PC -= 2;
			WZ = PC + 1;       }
		EDO EST
}

/***************************************************************
 * CPIR
 ***************************************************************/
z80_device::ops_type z80_device::cpir()
{
	return ST_M
		cpi()
		DOIF { return BC != 0 && !(F & ZF); } FN {
			CC(ex, 0xb1);
			TADR=HL;                        } MN
			nomreq_addr(5)                  FN {
			PC -= 2;
			WZ = PC + 1;                    }
		EDO EST
}

/***************************************************************
 * INIR
 ***************************************************************/
z80_device::ops_type z80_device::inir()
{
	return ST_M
		ini()
		DOIF { return B != 0; } FN {
			CC(ex, 0xb2);
			TADR=HL;          } MN
			nomreq_addr(5)    FN {
			PC -= 2;          }
		EDO EST
}

/***************************************************************
 * OTIR
 ***************************************************************/
z80_device::ops_type z80_device::otir()
{
	return ST_M
		outi()
		DOIF { return B != 0; } FN {
			CC(ex, 0xb3);
			TADR=BC;          } MN
			nomreq_addr(5)    FN {
			PC -= 2;          }
		EDO EST
}

/***************************************************************
 * LDDR
 ***************************************************************/
z80_device::ops_type z80_device::lddr()
{
	return ST_M
		ldd()
		DOIF { return BC != 0; } FN {
			CC(ex, 0xb8);
			TADR=DE;             } MN
			nomreq_addr(5)       FN {
			PC -= 2;
			WZ = PC + 1;         }
		EDO EST
}

/***************************************************************
 * CPDR
 ***************************************************************/
z80_device::ops_type z80_device::cpdr()
{
	return ST_M
		cpd()
		DOIF { return BC != 0 && !(F & ZF); } FN {
			CC(ex, 0xb9);
			TADR=HL;                        } MN
			nomreq_addr(5)                  FN {
			PC -= 2;
			WZ = PC + 1;                    }
		EDO EST
}

/***************************************************************
 * INDR
 ***************************************************************/
z80_device::ops_type z80_device::indr()
{
	return ST_M
		ind()
		DOIF { return B != 0; } FN {
			CC(ex, 0xba);
			TADR=HL;          } MN
			nomreq_addr(5)    FN {
			PC -= 2;          }
		EDO EST
}

/***************************************************************
 * OTDR
 ***************************************************************/
z80_device::ops_type z80_device::otdr()
{
	return ST_M
		outd()
		DOIF { return B != 0; } FN {
			CC(ex, 0xbb);
			TADR=BC;          } MN
			nomreq_addr(5)    FN {
			PC -= 2;          }
		EDO EST
}

/***************************************************************
 * EI
 ***************************************************************/
inline void z80_device::ei()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = true;
}

inline void z80_device::illegal_1() {
	logerror("Z80 ill. opcode $%02x $%02x ($%04x)\n",
			m_opcodes.read_byte(translate_memory_address((PCD-1)&0xffff)), m_opcodes.read_byte(translate_memory_address(PCD)), PCD-1);
}

inline void z80_device::illegal_2()
{
	logerror("Z80 ill. opcode $ed $%02x\n",
			m_opcodes.read_byte(translate_memory_address((PCD-1)&0xffff)));
}

void z80_device::init_op_steps() {

#define OP(prefix,opcode) m_op_steps[prefix][0x##opcode] = op_builder(*this).add([&]()
#define OP_M(prefix,opcode) m_op_steps[prefix][0x##opcode] = op_builder(*this).add(
#define JP(op) )->jump(0x##op);
#define OP_J(prefix,opcode,p_to,op_to) m_op_steps[prefix][0x##opcode] = op_builder(*this).jump(p_to, 0x##op_to);
#define JP_P(p_to) )->add([&]() { m_cycle=~0; m_prefix=p_to; m_opcode=TDAT8; calculate_icount(); })->get_steps();
#define EOP )->build();

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
/* RLC  B          */ OP(CB,00) { B = rlc(B);             } EOP
/* RLC  C          */ OP(CB,01) { C = rlc(C);             } EOP
/* RLC  D          */ OP(CB,02) { D = rlc(D);             } EOP
/* RLC  E          */ OP(CB,03) { E = rlc(E);             } EOP
/* RLC  H          */ OP(CB,04) { H = rlc(H);             } EOP
/* RLC  L          */ OP(CB,05) { L = rlc(L);             } EOP
/* RLC  (HL)       */ OP(CB,06) { TADR=HL; } MN rm_reg() FN { TDAT8=rlc(TDAT8); } MN wm() EOP
/* RLC  A          */ OP(CB,07) { A = rlc(A);             } EOP

/* RRC  B          */ OP(CB,08) { B = rrc(B);             } EOP
/* RRC  C          */ OP(CB,09) { C = rrc(C);             } EOP
/* RRC  D          */ OP(CB,0a) { D = rrc(D);             } EOP
/* RRC  E          */ OP(CB,0b) { E = rrc(E);             } EOP
/* RRC  H          */ OP(CB,0c) { H = rrc(H);             } EOP
/* RRC  L          */ OP(CB,0d) { L = rrc(L);             } EOP
/* RRC  (HL)       */ OP(CB,0e) { TADR=HL; } MN rm_reg() FN { TDAT8=rrc(TDAT8); } MN wm() EOP
/* RRC  A          */ OP(CB,0f) { A = rrc(A);             } EOP

/* RL   B          */ OP(CB,10) { B = rl(B);              } EOP
/* RL   C          */ OP(CB,11) { C = rl(C);              } EOP
/* RL   D          */ OP(CB,12) { D = rl(D);              } EOP
/* RL   E          */ OP(CB,13) { E = rl(E);              } EOP
/* RL   H          */ OP(CB,14) { H = rl(H);              } EOP
/* RL   L          */ OP(CB,15) { L = rl(L);              } EOP
/* RL   (HL)       */ OP(CB,16) { TADR=HL; } MN rm_reg() FN { TDAT8=rl(TDAT8); } MN wm() EOP
/* RL   A          */ OP(CB,17) { A = rl(A);              } EOP

/* RR   B          */ OP(CB,18) { B = rr(B);              } EOP
/* RR   C          */ OP(CB,19) { C = rr(C);              } EOP
/* RR   D          */ OP(CB,1a) { D = rr(D);              } EOP
/* RR   E          */ OP(CB,1b) { E = rr(E);              } EOP
/* RR   H          */ OP(CB,1c) { H = rr(H);              } EOP
/* RR   L          */ OP(CB,1d) { L = rr(L);              } EOP
/* RR   (HL)       */ OP(CB,1e) { TADR=HL; } MN rm_reg() FN { TDAT8=rr(TDAT8); } MN wm() EOP
/* RR   A          */ OP(CB,1f) { A = rr(A);              } EOP

/* SLA  B          */ OP(CB,20) { B = sla(B);             } EOP
/* SLA  C          */ OP(CB,21) { C = sla(C);             } EOP
/* SLA  D          */ OP(CB,22) { D = sla(D);             } EOP
/* SLA  E          */ OP(CB,23) { E = sla(E);             } EOP
/* SLA  H          */ OP(CB,24) { H = sla(H);             } EOP
/* SLA  L          */ OP(CB,25) { L = sla(L);             } EOP
/* SLA  (HL)       */ OP(CB,26) { TADR=HL; } MN rm_reg() FN { TDAT8=sla(TDAT8); } MN wm() EOP
/* SLA  A          */ OP(CB,27) { A = sla(A);             } EOP

/* SRA  B          */ OP(CB,28) { B = sra(B);             } EOP
/* SRA  C          */ OP(CB,29) { C = sra(C);             } EOP
/* SRA  D          */ OP(CB,2a) { D = sra(D);             } EOP
/* SRA  E          */ OP(CB,2b) { E = sra(E);             } EOP
/* SRA  H          */ OP(CB,2c) { H = sra(H);             } EOP
/* SRA  L          */ OP(CB,2d) { L = sra(L);             } EOP
/* SRA  (HL)       */ OP(CB,2e) { TADR=HL; } MN rm_reg() FN { TDAT8=sra(TDAT8); } MN wm() EOP
/* SRA  A          */ OP(CB,2f) { A = sra(A);             } EOP

/* SLL  B          */ OP(CB,30) { B = sll(B);             } EOP
/* SLL  C          */ OP(CB,31) { C = sll(C);             } EOP
/* SLL  D          */ OP(CB,32) { D = sll(D);             } EOP
/* SLL  E          */ OP(CB,33) { E = sll(E);             } EOP
/* SLL  H          */ OP(CB,34) { H = sll(H);             } EOP
/* SLL  L          */ OP(CB,35) { L = sll(L);             } EOP
/* SLL  (HL)       */ OP(CB,36) { TADR=HL; } MN rm_reg() FN { TDAT8=sll(TDAT8); } MN wm() EOP
/* SLL  A          */ OP(CB,37) { A = sll(A);             } EOP

/* SRL  B          */ OP(CB,38) { B = srl(B);             } EOP
/* SRL  C          */ OP(CB,39) { C = srl(C);             } EOP
/* SRL  D          */ OP(CB,3a) { D = srl(D);             } EOP
/* SRL  E          */ OP(CB,3b) { E = srl(E);             } EOP
/* SRL  H          */ OP(CB,3c) { H = srl(H);             } EOP
/* SRL  L          */ OP(CB,3d) { L = srl(L);             } EOP
/* SRL  (HL)       */ OP(CB,3e) { TADR=HL; } MN rm_reg() FN { TDAT8=srl(TDAT8); } MN wm() EOP
/* SRL  A          */ OP(CB,3f) { A = srl(A);             } EOP

/* BIT  0,B        */ OP(CB,40) { bit(0, B);              } EOP
/* BIT  0,C        */ OP(CB,41) { bit(0, C);              } EOP
/* BIT  0,D        */ OP(CB,42) { bit(0, D);              } EOP
/* BIT  0,E        */ OP(CB,43) { bit(0, E);              } EOP
/* BIT  0,H        */ OP(CB,44) { bit(0, H);              } EOP
/* BIT  0,L        */ OP(CB,45) { bit(0, L);              } EOP
/* BIT  0,(HL)     */ OP(CB,46) { TADR=HL; } MN rm_reg() FN { bit_hl(0, TDAT8); } EOP
/* BIT  0,A        */ OP(CB,47) { bit(0, A);              } EOP

/* BIT  1,B        */ OP(CB,48) { bit(1, B);              } EOP
/* BIT  1,C        */ OP(CB,49) { bit(1, C);              } EOP
/* BIT  1,D        */ OP(CB,4a) { bit(1, D);              } EOP
/* BIT  1,E        */ OP(CB,4b) { bit(1, E);              } EOP
/* BIT  1,H        */ OP(CB,4c) { bit(1, H);              } EOP
/* BIT  1,L        */ OP(CB,4d) { bit(1, L);              } EOP
/* BIT  1,(HL)     */ OP(CB,4e) { TADR=HL; } MN rm_reg() FN { bit_hl(1, TDAT8); } EOP
/* BIT  1,A        */ OP(CB,4f) { bit(1, A);              } EOP

/* BIT  2,B        */ OP(CB,50) { bit(2, B);              } EOP
/* BIT  2,C        */ OP(CB,51) { bit(2, C);              } EOP
/* BIT  2,D        */ OP(CB,52) { bit(2, D);              } EOP
/* BIT  2,E        */ OP(CB,53) { bit(2, E);              } EOP
/* BIT  2,H        */ OP(CB,54) { bit(2, H);              } EOP
/* BIT  2,L        */ OP(CB,55) { bit(2, L);              } EOP
/* BIT  2,(HL)     */ OP(CB,56) { TADR=HL; } MN rm_reg() FN { bit_hl(2, TDAT8); } EOP
/* BIT  2,A        */ OP(CB,57) { bit(2, A);              } EOP

/* BIT  3,B        */ OP(CB,58) { bit(3, B);              } EOP
/* BIT  3,C        */ OP(CB,59) { bit(3, C);              } EOP
/* BIT  3,D        */ OP(CB,5a) { bit(3, D);              } EOP
/* BIT  3,E        */ OP(CB,5b) { bit(3, E);              } EOP
/* BIT  3,H        */ OP(CB,5c) { bit(3, H);              } EOP
/* BIT  3,L        */ OP(CB,5d) { bit(3, L);              } EOP
/* BIT  3,(HL)     */ OP(CB,5e) { TADR=HL; } MN rm_reg() FN { bit_hl(3, TDAT8); } EOP
/* BIT  3,A        */ OP(CB,5f) { bit(3, A);              } EOP

/* BIT  4,B        */ OP(CB,60) { bit(4, B);              } EOP
/* BIT  4,C        */ OP(CB,61) { bit(4, C);              } EOP
/* BIT  4,D        */ OP(CB,62) { bit(4, D);              } EOP
/* BIT  4,E        */ OP(CB,63) { bit(4, E);              } EOP
/* BIT  4,H        */ OP(CB,64) { bit(4, H);              } EOP
/* BIT  4,L        */ OP(CB,65) { bit(4, L);              } EOP
/* BIT  4,(HL)     */ OP(CB,66) { TADR=HL; } MN rm_reg() FN { bit_hl(4, TDAT8); } EOP
/* BIT  4,A        */ OP(CB,67) { bit(4, A);              } EOP

/* BIT  5,B        */ OP(CB,68) { bit(5, B);              } EOP
/* BIT  5,C        */ OP(CB,69) { bit(5, C);              } EOP
/* BIT  5,D        */ OP(CB,6a) { bit(5, D);              } EOP
/* BIT  5,E        */ OP(CB,6b) { bit(5, E);              } EOP
/* BIT  5,H        */ OP(CB,6c) { bit(5, H);              } EOP
/* BIT  5,L        */ OP(CB,6d) { bit(5, L);              } EOP
/* BIT  5,(HL)     */ OP(CB,6e) { TADR=HL; } MN rm_reg() FN { bit_hl(5, TDAT8); } EOP
/* BIT  5,A        */ OP(CB,6f) { bit(5, A);              } EOP

/* BIT  6,B        */ OP(CB,70) { bit(6, B);              } EOP
/* BIT  6,C        */ OP(CB,71) { bit(6, C);              } EOP
/* BIT  6,D        */ OP(CB,72) { bit(6, D);              } EOP
/* BIT  6,E        */ OP(CB,73) { bit(6, E);              } EOP
/* BIT  6,H        */ OP(CB,74) { bit(6, H);              } EOP
/* BIT  6,L        */ OP(CB,75) { bit(6, L);              } EOP
/* BIT  6,(HL)     */ OP(CB,76) { TADR=HL; } MN rm_reg() FN { bit_hl(6, TDAT8); } EOP
/* BIT  6,A        */ OP(CB,77) { bit(6, A);              } EOP

/* BIT  7,B        */ OP(CB,78) { bit(7, B);              } EOP
/* BIT  7,C        */ OP(CB,79) { bit(7, C);              } EOP
/* BIT  7,D        */ OP(CB,7a) { bit(7, D);              } EOP
/* BIT  7,E        */ OP(CB,7b) { bit(7, E);              } EOP
/* BIT  7,H        */ OP(CB,7c) { bit(7, H);              } EOP
/* BIT  7,L        */ OP(CB,7d) { bit(7, L);              } EOP
/* BIT  7,(HL)     */ OP(CB,7e) { TADR=HL; } MN rm_reg() FN { bit_hl(7, TDAT8); } EOP
/* BIT  7,A        */ OP(CB,7f) { bit(7, A);              } EOP

/* RES  0,B        */ OP(CB,80) { B = res(0, B);          } EOP
/* RES  0,C        */ OP(CB,81) { C = res(0, C);          } EOP
/* RES  0,D        */ OP(CB,82) { D = res(0, D);          } EOP
/* RES  0,E        */ OP(CB,83) { E = res(0, E);          } EOP
/* RES  0,H        */ OP(CB,84) { H = res(0, H);          } EOP
/* RES  0,L        */ OP(CB,85) { L = res(0, L);          } EOP
/* RES  0,(HL)     */ OP(CB,86) { TADR=HL; } MN rm_reg() FN { TDAT8=res(0, TDAT8); } MN wm() EOP
/* RES  0,A        */ OP(CB,87) { A = res(0, A);          } EOP

/* RES  1,B        */ OP(CB,88) { B = res(1, B);          } EOP
/* RES  1,C        */ OP(CB,89) { C = res(1, C);          } EOP
/* RES  1,D        */ OP(CB,8a) { D = res(1, D);          } EOP
/* RES  1,E        */ OP(CB,8b) { E = res(1, E);          } EOP
/* RES  1,H        */ OP(CB,8c) { H = res(1, H);          } EOP
/* RES  1,L        */ OP(CB,8d) { L = res(1, L);          } EOP
/* RES  1,(HL)     */ OP(CB,8e) { TADR=HL; } MN rm_reg() FN { TDAT8=res(1, TDAT8); } MN wm() EOP
/* RES  1,A        */ OP(CB,8f) { A = res(1, A);          } EOP

/* RES  2,B        */ OP(CB,90) { B = res(2, B);          } EOP
/* RES  2,C        */ OP(CB,91) { C = res(2, C);          } EOP
/* RES  2,D        */ OP(CB,92) { D = res(2, D);          } EOP
/* RES  2,E        */ OP(CB,93) { E = res(2, E);          } EOP
/* RES  2,H        */ OP(CB,94) { H = res(2, H);          } EOP
/* RES  2,L        */ OP(CB,95) { L = res(2, L);          } EOP
/* RES  2,(HL)     */ OP(CB,96) { TADR=HL; } MN rm_reg() FN { TDAT8=res(2, TDAT8); } MN wm() EOP
/* RES  2,A        */ OP(CB,97) { A = res(2, A);          } EOP

/* RES  3,B        */ OP(CB,98) { B = res(3, B);          } EOP
/* RES  3,C        */ OP(CB,99) { C = res(3, C);          } EOP
/* RES  3,D        */ OP(CB,9a) { D = res(3, D);          } EOP
/* RES  3,E        */ OP(CB,9b) { E = res(3, E);          } EOP
/* RES  3,H        */ OP(CB,9c) { H = res(3, H);          } EOP
/* RES  3,L        */ OP(CB,9d) { L = res(3, L);          } EOP
/* RES  3,(HL)     */ OP(CB,9e) { TADR=HL; } MN rm_reg() FN { TDAT8=res(3, TDAT8); } MN wm() EOP
/* RES  3,A        */ OP(CB,9f) { A = res(3, A);          } EOP

/* RES  4,B        */ OP(CB,a0) { B = res(4, B);          } EOP
/* RES  4,C        */ OP(CB,a1) { C = res(4, C);          } EOP
/* RES  4,D        */ OP(CB,a2) { D = res(4, D);          } EOP
/* RES  4,E        */ OP(CB,a3) { E = res(4, E);          } EOP
/* RES  4,H        */ OP(CB,a4) { H = res(4, H);          } EOP
/* RES  4,L        */ OP(CB,a5) { L = res(4, L);          } EOP
/* RES  4,(HL)     */ OP(CB,a6) { TADR=HL; } MN rm_reg() FN { TDAT8=res(4, TDAT8); } MN wm() EOP
/* RES  4,A        */ OP(CB,a7) { A = res(4, A);          } EOP

/* RES  5,B        */ OP(CB,a8) { B = res(5, B);          } EOP
/* RES  5,C        */ OP(CB,a9) { C = res(5, C);          } EOP
/* RES  5,D        */ OP(CB,aa) { D = res(5, D);          } EOP
/* RES  5,E        */ OP(CB,ab) { E = res(5, E);          } EOP
/* RES  5,H        */ OP(CB,ac) { H = res(5, H);          } EOP
/* RES  5,L        */ OP(CB,ad) { L = res(5, L);          } EOP
/* RES  5,(HL)     */ OP(CB,ae) { TADR=HL; } MN rm_reg() FN { TDAT8=res(5, TDAT8); } MN wm() EOP
/* RES  5,A        */ OP(CB,af) { A = res(5, A);          } EOP

/* RES  6,B        */ OP(CB,b0) { B = res(6, B);          } EOP
/* RES  6,C        */ OP(CB,b1) { C = res(6, C);          } EOP
/* RES  6,D        */ OP(CB,b2) { D = res(6, D);          } EOP
/* RES  6,E        */ OP(CB,b3) { E = res(6, E);          } EOP
/* RES  6,H        */ OP(CB,b4) { H = res(6, H);          } EOP
/* RES  6,L        */ OP(CB,b5) { L = res(6, L);          } EOP
/* RES  6,(HL)     */ OP(CB,b6) { TADR=HL; } MN rm_reg() FN { TDAT8=res(6, TDAT8); } MN wm() EOP
/* RES  6,A        */ OP(CB,b7) { A = res(6, A);          } EOP

/* RES  7,B        */ OP(CB,b8) { B = res(7, B);          } EOP
/* RES  7,C        */ OP(CB,b9) { C = res(7, C);          } EOP
/* RES  7,D        */ OP(CB,ba) { D = res(7, D);          } EOP
/* RES  7,E        */ OP(CB,bb) { E = res(7, E);          } EOP
/* RES  7,H        */ OP(CB,bc) { H = res(7, H);          } EOP
/* RES  7,L        */ OP(CB,bd) { L = res(7, L);          } EOP
/* RES  7,(HL)     */ OP(CB,be) { TADR=HL; } MN rm_reg() FN { TDAT8=res(7, TDAT8); } MN wm() EOP
/* RES  7,A        */ OP(CB,bf) { A = res(7, A);          } EOP

/* SET  0,B        */ OP(CB,c0) { B = set(0, B);          } EOP
/* SET  0,C        */ OP(CB,c1) { C = set(0, C);          } EOP
/* SET  0,D        */ OP(CB,c2) { D = set(0, D);          } EOP
/* SET  0,E        */ OP(CB,c3) { E = set(0, E);          } EOP
/* SET  0,H        */ OP(CB,c4) { H = set(0, H);          } EOP
/* SET  0,L        */ OP(CB,c5) { L = set(0, L);          } EOP
/* SET  0,(HL)     */ OP(CB,c6) { TADR=HL; } MN rm_reg() FN { TDAT8=set(0, TDAT8); } MN wm() EOP
/* SET  0,A        */ OP(CB,c7) { A = set(0, A);          } EOP

/* SET  1,B        */ OP(CB,c8) { B = set(1, B);          } EOP
/* SET  1,C        */ OP(CB,c9) { C = set(1, C);          } EOP
/* SET  1,D        */ OP(CB,ca) { D = set(1, D);          } EOP
/* SET  1,E        */ OP(CB,cb) { E = set(1, E);          } EOP
/* SET  1,H        */ OP(CB,cc) { H = set(1, H);          } EOP
/* SET  1,L        */ OP(CB,cd) { L = set(1, L);          } EOP
/* SET  1,(HL)     */ OP(CB,ce) { TADR=HL; } MN rm_reg() FN { TDAT8=set(1, TDAT8); } MN wm() EOP
/* SET  1,A        */ OP(CB,cf) { A = set(1, A);          } EOP

/* SET  2,B        */ OP(CB,d0) { B = set(2, B);          } EOP
/* SET  2,C        */ OP(CB,d1) { C = set(2, C);          } EOP
/* SET  2,D        */ OP(CB,d2) { D = set(2, D);          } EOP
/* SET  2,E        */ OP(CB,d3) { E = set(2, E);          } EOP
/* SET  2,H        */ OP(CB,d4) { H = set(2, H);          } EOP
/* SET  2,L        */ OP(CB,d5) { L = set(2, L);          } EOP
/* SET  2,(HL)     */ OP(CB,d6) { TADR=HL; } MN rm_reg() FN { TDAT8=set(2, TDAT8); } MN wm() EOP
/* SET  2,A        */ OP(CB,d7) { A = set(2, A);          } EOP

/* SET  3,B        */ OP(CB,d8) { B = set(3, B);          } EOP
/* SET  3,C        */ OP(CB,d9) { C = set(3, C);          } EOP
/* SET  3,D        */ OP(CB,da) { D = set(3, D);          } EOP
/* SET  3,E        */ OP(CB,db) { E = set(3, E);          } EOP
/* SET  3,H        */ OP(CB,dc) { H = set(3, H);          } EOP
/* SET  3,L        */ OP(CB,dd) { L = set(3, L);          } EOP
/* SET  3,(HL)     */ OP(CB,de) { TADR=HL; } MN rm_reg() FN { TDAT8=set(3, TDAT8); } MN wm() EOP
/* SET  3,A        */ OP(CB,df) { A = set(3, A);          } EOP

/* SET  4,B        */ OP(CB,e0) { B = set(4, B);          } EOP
/* SET  4,C        */ OP(CB,e1) { C = set(4, C);          } EOP
/* SET  4,D        */ OP(CB,e2) { D = set(4, D);          } EOP
/* SET  4,E        */ OP(CB,e3) { E = set(4, E);          } EOP
/* SET  4,H        */ OP(CB,e4) { H = set(4, H);          } EOP
/* SET  4,L        */ OP(CB,e5) { L = set(4, L);          } EOP
/* SET  4,(HL)     */ OP(CB,e6) { TADR=HL; } MN rm_reg() FN { TDAT8=set(4, TDAT8); } MN wm() EOP
/* SET  4,A        */ OP(CB,e7) { A = set(4, A);          } EOP

/* SET  5,B        */ OP(CB,e8) { B = set(5, B);          } EOP
/* SET  5,C        */ OP(CB,e9) { C = set(5, C);          } EOP
/* SET  5,D        */ OP(CB,ea) { D = set(5, D);          } EOP
/* SET  5,E        */ OP(CB,eb) { E = set(5, E);          } EOP
/* SET  5,H        */ OP(CB,ec) { H = set(5, H);          } EOP
/* SET  5,L        */ OP(CB,ed) { L = set(5, L);          } EOP
/* SET  5,(HL)     */ OP(CB,ee) { TADR=HL; } MN rm_reg() FN { TDAT8=set(5, TDAT8); } MN wm() EOP
/* SET  5,A        */ OP(CB,ef) { A = set(5, A);          } EOP

/* SET  6,B        */ OP(CB,f0) { B = set(6, B);          } EOP
/* SET  6,C        */ OP(CB,f1) { C = set(6, C);          } EOP
/* SET  6,D        */ OP(CB,f2) { D = set(6, D);          } EOP
/* SET  6,E        */ OP(CB,f3) { E = set(6, E);          } EOP
/* SET  6,H        */ OP(CB,f4) { H = set(6, H);          } EOP
/* SET  6,L        */ OP(CB,f5) { L = set(6, L);          } EOP
/* SET  6,(HL)     */ OP(CB,f6) { TADR=HL; } MN rm_reg() FN { TDAT8=set(6, TDAT8); } MN wm() EOP
/* SET  6,A        */ OP(CB,f7) { A = set(6, A);          } EOP

/* SET  7,B        */ OP(CB,f8) { B = set(7, B);          } EOP
/* SET  7,C        */ OP(CB,f9) { C = set(7, C);          } EOP
/* SET  7,D        */ OP(CB,fa) { D = set(7, D);          } EOP
/* SET  7,E        */ OP(CB,fb) { E = set(7, E);          } EOP
/* SET  7,H        */ OP(CB,fc) { H = set(7, H);          } EOP
/* SET  7,L        */ OP(CB,fd) { L = set(7, L);          } EOP
/* SET  7,(HL)     */ OP(CB,fe) { TADR=HL; } MN rm_reg() FN { TDAT8=set(7, TDAT8); } MN wm() EOP
/* SET  7,A        */ OP(CB,ff) { A = set(7, A);          } EOP

/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
/* RLC  B=(XY+o)   */ OP(XY_CB,00) { TADR=m_ea; } MN rm_reg() FN { B=rlc(TDAT8); TDAT8=B; } MN wm() EOP
/* RLC  C=(XY+o)   */ OP(XY_CB,01) { TADR=m_ea; } MN rm_reg() FN { C=rlc(TDAT8); TDAT8=C; } MN wm() EOP
/* RLC  D=(XY+o)   */ OP(XY_CB,02) { TADR=m_ea; } MN rm_reg() FN { D=rlc(TDAT8); TDAT8=D; } MN wm() EOP
/* RLC  E=(XY+o)   */ OP(XY_CB,03) { TADR=m_ea; } MN rm_reg() FN { E=rlc(TDAT8); TDAT8=E; } MN wm() EOP
/* RLC  H=(XY+o)   */ OP(XY_CB,04) { TADR=m_ea; } MN rm_reg() FN { H=rlc(TDAT8); TDAT8=H; } MN wm() EOP
/* RLC  L=(XY+o)   */ OP(XY_CB,05) { TADR=m_ea; } MN rm_reg() FN { L=rlc(TDAT8); TDAT8=L; } MN wm() EOP
/* RLC  (XY+o)     */ OP(XY_CB,06) { TADR=m_ea; } MN rm_reg() FN { TDAT8=rlc(TDAT8); } MN wm()      EOP
/* RLC  A=(XY+o)   */ OP(XY_CB,07) { TADR=m_ea; } MN rm_reg() FN { A=rlc(TDAT8); TDAT8=A; } MN wm() EOP

/* RRC  B=(XY+o)   */ OP(XY_CB,08) { TADR=m_ea; } MN rm_reg() FN { B=rrc(TDAT8); TDAT8=B; } MN wm() EOP
/* RRC  C=(XY+o)   */ OP(XY_CB,09) { TADR=m_ea; } MN rm_reg() FN { C=rrc(TDAT8); TDAT8=C; } MN wm() EOP
/* RRC  D=(XY+o)   */ OP(XY_CB,0a) { TADR=m_ea; } MN rm_reg() FN { D=rrc(TDAT8); TDAT8=D; } MN wm() EOP
/* RRC  E=(XY+o)   */ OP(XY_CB,0b) { TADR=m_ea; } MN rm_reg() FN { E=rrc(TDAT8); TDAT8=E; } MN wm() EOP
/* RRC  H=(XY+o)   */ OP(XY_CB,0c) { TADR=m_ea; } MN rm_reg() FN { H=rrc(TDAT8); TDAT8=H; } MN wm() EOP
/* RRC  L=(XY+o)   */ OP(XY_CB,0d) { TADR=m_ea; } MN rm_reg() FN { L=rrc(TDAT8); TDAT8=L; } MN wm() EOP
/* RRC  (XY+o)     */ OP(XY_CB,0e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=rrc(TDAT8); } MN wm()      EOP
/* RRC  A=(XY+o)   */ OP(XY_CB,0f) { TADR=m_ea; } MN rm_reg() FN { A=rrc(TDAT8); TDAT8=A; } MN wm() EOP

/* RL   B=(XY+o)   */ OP(XY_CB,10) { TADR=m_ea; } MN rm_reg() FN { B=rl(TDAT8); TDAT8=B; } MN wm() EOP
/* RL   C=(XY+o)   */ OP(XY_CB,11) { TADR=m_ea; } MN rm_reg() FN { C=rl(TDAT8); TDAT8=C; } MN wm() EOP
/* RL   D=(XY+o)   */ OP(XY_CB,12) { TADR=m_ea; } MN rm_reg() FN { D=rl(TDAT8); TDAT8=D; } MN wm() EOP
/* RL   E=(XY+o)   */ OP(XY_CB,13) { TADR=m_ea; } MN rm_reg() FN { E=rl(TDAT8); TDAT8=E; } MN wm() EOP
/* RL   H=(XY+o)   */ OP(XY_CB,14) { TADR=m_ea; } MN rm_reg() FN { H=rl(TDAT8); TDAT8=H; } MN wm() EOP
/* RL   L=(XY+o)   */ OP(XY_CB,15) { TADR=m_ea; } MN rm_reg() FN { L=rl(TDAT8); TDAT8=L; } MN wm() EOP
/* RL   (XY+o)     */ OP(XY_CB,16) { TADR=m_ea; } MN rm_reg() FN { TDAT8=rl(TDAT8); } MN wm()      EOP
/* RL   A=(XY+o)   */ OP(XY_CB,17) { TADR=m_ea; } MN rm_reg() FN { A=rl(TDAT8); TDAT8=A; } MN wm() EOP

/* RR   B=(XY+o)   */ OP(XY_CB,18) { TADR=m_ea; } MN rm_reg() FN { B=rr(TDAT8); TDAT8=B; } MN wm() EOP
/* RR   C=(XY+o)   */ OP(XY_CB,19) { TADR=m_ea; } MN rm_reg() FN { C=rr(TDAT8); TDAT8=C; } MN wm() EOP
/* RR   D=(XY+o)   */ OP(XY_CB,1a) { TADR=m_ea; } MN rm_reg() FN { D=rr(TDAT8); TDAT8=D; } MN wm() EOP
/* RR   E=(XY+o)   */ OP(XY_CB,1b) { TADR=m_ea; } MN rm_reg() FN { E=rr(TDAT8); TDAT8=E; } MN wm() EOP
/* RR   H=(XY+o)   */ OP(XY_CB,1c) { TADR=m_ea; } MN rm_reg() FN { H=rr(TDAT8); TDAT8=H; } MN wm() EOP
/* RR   L=(XY+o)   */ OP(XY_CB,1d) { TADR=m_ea; } MN rm_reg() FN { L=rr(TDAT8); TDAT8=L; } MN wm() EOP
/* RR   (XY+o)     */ OP(XY_CB,1e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=rr(TDAT8); } MN wm()      EOP
/* RR   A=(XY+o)   */ OP(XY_CB,1f) { TADR=m_ea; } MN rm_reg() FN { A=rr(TDAT8); TDAT8=A; } MN wm() EOP

/* SLA  B=(XY+o)   */ OP(XY_CB,20) { TADR=m_ea; } MN rm_reg() FN { B=sla(TDAT8); TDAT8=B; } MN wm() EOP
/* SLA  C=(XY+o)   */ OP(XY_CB,21) { TADR=m_ea; } MN rm_reg() FN { C=sla(TDAT8); TDAT8=C; } MN wm() EOP
/* SLA  D=(XY+o)   */ OP(XY_CB,22) { TADR=m_ea; } MN rm_reg() FN { D=sla(TDAT8); TDAT8=D; } MN wm() EOP
/* SLA  E=(XY+o)   */ OP(XY_CB,23) { TADR=m_ea; } MN rm_reg() FN { E=sla(TDAT8); TDAT8=E; } MN wm() EOP
/* SLA  H=(XY+o)   */ OP(XY_CB,24) { TADR=m_ea; } MN rm_reg() FN { H=sla(TDAT8); TDAT8=H; } MN wm() EOP
/* SLA  L=(XY+o)   */ OP(XY_CB,25) { TADR=m_ea; } MN rm_reg() FN { L=sla(TDAT8); TDAT8=L; } MN wm() EOP
/* SLA  (XY+o)     */ OP(XY_CB,26) { TADR=m_ea; } MN rm_reg() FN { TDAT8=sla(TDAT8); } MN wm()      EOP
/* SLA  A=(XY+o)   */ OP(XY_CB,27) { TADR=m_ea; } MN rm_reg() FN { A=sla(TDAT8); TDAT8=A; } MN wm() EOP

/* SRA  B=(XY+o)   */ OP(XY_CB,28) { TADR=m_ea; } MN rm_reg() FN { B=sra(TDAT8); TDAT8=B; } MN wm() EOP
/* SRA  C=(XY+o)   */ OP(XY_CB,29) { TADR=m_ea; } MN rm_reg() FN { C=sra(TDAT8); TDAT8=C; } MN wm() EOP
/* SRA  D=(XY+o)   */ OP(XY_CB,2a) { TADR=m_ea; } MN rm_reg() FN { D=sra(TDAT8); TDAT8=D; } MN wm() EOP
/* SRA  E=(XY+o)   */ OP(XY_CB,2b) { TADR=m_ea; } MN rm_reg() FN { E=sra(TDAT8); TDAT8=E; } MN wm() EOP
/* SRA  H=(XY+o)   */ OP(XY_CB,2c) { TADR=m_ea; } MN rm_reg() FN { H=sra(TDAT8); TDAT8=H; } MN wm() EOP
/* SRA  L=(XY+o)   */ OP(XY_CB,2d) { TADR=m_ea; } MN rm_reg() FN { L=sra(TDAT8); TDAT8=L; } MN wm() EOP
/* SRA  (XY+o)     */ OP(XY_CB,2e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=sra(TDAT8); } MN wm()      EOP
/* SRA  A=(XY+o)   */ OP(XY_CB,2f) { TADR=m_ea; } MN rm_reg() FN { A=sra(TDAT8); TDAT8=A; } MN wm() EOP

/* SLL  B=(XY+o)   */ OP(XY_CB,30) { TADR=m_ea; } MN rm_reg() FN { B=sll(TDAT8); TDAT8=B; } MN wm() EOP
/* SLL  C=(XY+o)   */ OP(XY_CB,31) { TADR=m_ea; } MN rm_reg() FN { C=sll(TDAT8); TDAT8=C; } MN wm() EOP
/* SLL  D=(XY+o)   */ OP(XY_CB,32) { TADR=m_ea; } MN rm_reg() FN { D=sll(TDAT8); TDAT8=D; } MN wm() EOP
/* SLL  E=(XY+o)   */ OP(XY_CB,33) { TADR=m_ea; } MN rm_reg() FN { E=sll(TDAT8); TDAT8=E; } MN wm() EOP
/* SLL  H=(XY+o)   */ OP(XY_CB,34) { TADR=m_ea; } MN rm_reg() FN { H=sll(TDAT8); TDAT8=H; } MN wm() EOP
/* SLL  L=(XY+o)   */ OP(XY_CB,35) { TADR=m_ea; } MN rm_reg() FN { L=sll(TDAT8); TDAT8=L; } MN wm() EOP
/* SLL  (XY+o)     */ OP(XY_CB,36) { TADR=m_ea; } MN rm_reg() FN { TDAT8=sll(TDAT8); } MN wm()      EOP
/* SLL  A=(XY+o)   */ OP(XY_CB,37) { TADR=m_ea; } MN rm_reg() FN { A=sll(TDAT8); TDAT8=A; } MN wm() EOP

/* SRL  B=(XY+o)   */ OP(XY_CB,38) { TADR=m_ea; } MN rm_reg() FN { B=srl(TDAT8); TDAT8=B; } MN wm() EOP
/* SRL  C=(XY+o)   */ OP(XY_CB,39) { TADR=m_ea; } MN rm_reg() FN { C=srl(TDAT8); TDAT8=C; } MN wm() EOP
/* SRL  D=(XY+o)   */ OP(XY_CB,3a) { TADR=m_ea; } MN rm_reg() FN { D=srl(TDAT8); TDAT8=D; } MN wm() EOP
/* SRL  E=(XY+o)   */ OP(XY_CB,3b) { TADR=m_ea; } MN rm_reg() FN { E=srl(TDAT8); TDAT8=E; } MN wm() EOP
/* SRL  H=(XY+o)   */ OP(XY_CB,3c) { TADR=m_ea; } MN rm_reg() FN { H=srl(TDAT8); TDAT8=H; } MN wm() EOP
/* SRL  L=(XY+o)   */ OP(XY_CB,3d) { TADR=m_ea; } MN rm_reg() FN { L=srl(TDAT8); TDAT8=L; } MN wm() EOP
/* SRL  (XY+o)     */ OP(XY_CB,3e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=srl(TDAT8); } MN wm()      EOP
/* SRL  A=(XY+o)   */ OP(XY_CB,3f) { TADR=m_ea; } MN rm_reg() FN { A=srl(TDAT8); TDAT8=A; } MN wm() EOP

/* BIT  0,(XY+o)   */ OP_J(XY_CB,40, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP_J(XY_CB,41, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP_J(XY_CB,42, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP_J(XY_CB,43, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP_J(XY_CB,44, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP_J(XY_CB,45, XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,46) { TADR=m_ea; } MN rm_reg() FN { bit_xy(0, TDAT8); } EOP
/* BIT  0,(XY+o)   */ OP_J(XY_CB,47, XY_CB, 46)

/* BIT  1,(XY+o)   */ OP_J(XY_CB,48, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP_J(XY_CB,49, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP_J(XY_CB,4a, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP_J(XY_CB,4b, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP_J(XY_CB,4c, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP_J(XY_CB,4d, XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4e) { TADR=m_ea; } MN rm_reg() FN { bit_xy(1, TDAT8); } EOP
/* BIT  1,(XY+o)   */ OP_J(XY_CB,4f, XY_CB, 4e)

/* BIT  2,(XY+o)   */ OP_J(XY_CB,50, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP_J(XY_CB,51, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP_J(XY_CB,52, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP_J(XY_CB,53, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP_J(XY_CB,54, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP_J(XY_CB,55, XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,56) { TADR=m_ea; } MN rm_reg() FN { bit_xy(2, TDAT8); } EOP
/* BIT  2,(XY+o)   */ OP_J(XY_CB,57, XY_CB, 56)

/* BIT  3,(XY+o)   */ OP_J(XY_CB,58, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP_J(XY_CB,59, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP_J(XY_CB,5a, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP_J(XY_CB,5b, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP_J(XY_CB,5c, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP_J(XY_CB,5d, XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5e) { TADR=m_ea; } MN rm_reg() FN { bit_xy(3, TDAT8); } EOP
/* BIT  3,(XY+o)   */ OP_J(XY_CB,5f, XY_CB, 5e)

/* BIT  4,(XY+o)   */ OP_J(XY_CB,60, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP_J(XY_CB,61, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP_J(XY_CB,62, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP_J(XY_CB,63, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP_J(XY_CB,64, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP_J(XY_CB,65, XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,66) { TADR=m_ea; } MN rm_reg() FN { bit_xy(4, TDAT8); } EOP
/* BIT  4,(XY+o)   */ OP_J(XY_CB,67, XY_CB, 66)

/* BIT  5,(XY+o)   */ OP_J(XY_CB,68, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP_J(XY_CB,69, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP_J(XY_CB,6a, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP_J(XY_CB,6b, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP_J(XY_CB,6c, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP_J(XY_CB,6d, XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6e) { TADR=m_ea; } MN rm_reg() FN { bit_xy(5, TDAT8); } EOP
/* BIT  5,(XY+o)   */ OP_J(XY_CB,6f, XY_CB, 6e)

/* BIT  6,(XY+o)   */ OP_J(XY_CB,70, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP_J(XY_CB,71, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP_J(XY_CB,72, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP_J(XY_CB,73, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP_J(XY_CB,74, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP_J(XY_CB,75, XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,76) { TADR=m_ea; } MN rm_reg() FN { bit_xy(6, TDAT8); } EOP
/* BIT  6,(XY+o)   */ OP_J(XY_CB,77, XY_CB, 76)

/* BIT  7,(XY+o)   */ OP_J(XY_CB,78, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP_J(XY_CB,79, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP_J(XY_CB,7a, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP_J(XY_CB,7b, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP_J(XY_CB,7c, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP_J(XY_CB,7d, XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7e) { TADR=m_ea; } MN rm_reg() FN { bit_xy(7, TDAT8); } EOP
/* BIT  7,(XY+o)   */ OP_J(XY_CB,7f, XY_CB, 7e)

/* RES  0,B=(XY+o) */ OP(XY_CB,80) { TADR=m_ea; } MN rm_reg() FN { B=res(0, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  0,C=(XY+o) */ OP(XY_CB,81) { TADR=m_ea; } MN rm_reg() FN { C=res(0, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  0,D=(XY+o) */ OP(XY_CB,82) { TADR=m_ea; } MN rm_reg() FN { D=res(0, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  0,E=(XY+o) */ OP(XY_CB,83) { TADR=m_ea; } MN rm_reg() FN { E=res(0, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  0,H=(XY+o) */ OP(XY_CB,84) { TADR=m_ea; } MN rm_reg() FN { H=res(0, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  0,L=(XY+o) */ OP(XY_CB,85) { TADR=m_ea; } MN rm_reg() FN { L=res(0, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  0,(XY+o)   */ OP(XY_CB,86) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(0, TDAT8); } MN wm() EOP
/* RES  0,A=(XY+o) */ OP(XY_CB,87) { TADR=m_ea; } MN rm_reg() FN { A=res(0, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  1,B=(XY+o) */ OP(XY_CB,88) { TADR=m_ea; } MN rm_reg() FN { B=res(1, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  1,C=(XY+o) */ OP(XY_CB,89) { TADR=m_ea; } MN rm_reg() FN { C=res(1, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  1,D=(XY+o) */ OP(XY_CB,8a) { TADR=m_ea; } MN rm_reg() FN { D=res(1, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  1,E=(XY+o) */ OP(XY_CB,8b) { TADR=m_ea; } MN rm_reg() FN { E=res(1, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  1,H=(XY+o) */ OP(XY_CB,8c) { TADR=m_ea; } MN rm_reg() FN { H=res(1, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  1,L=(XY+o) */ OP(XY_CB,8d) { TADR=m_ea; } MN rm_reg() FN { L=res(1, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  1,(XY+o)   */ OP(XY_CB,8e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(1, TDAT8); } MN wm() EOP
/* RES  1,A=(XY+o) */ OP(XY_CB,8f) { TADR=m_ea; } MN rm_reg() FN { A=res(1, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  2,B=(XY+o) */ OP(XY_CB,90) { TADR=m_ea; } MN rm_reg() FN { B=res(2, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  2,C=(XY+o) */ OP(XY_CB,91) { TADR=m_ea; } MN rm_reg() FN { C=res(2, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  2,D=(XY+o) */ OP(XY_CB,92) { TADR=m_ea; } MN rm_reg() FN { D=res(2, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  2,E=(XY+o) */ OP(XY_CB,93) { TADR=m_ea; } MN rm_reg() FN { E=res(2, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  2,H=(XY+o) */ OP(XY_CB,94) { TADR=m_ea; } MN rm_reg() FN { H=res(2, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  2,L=(XY+o) */ OP(XY_CB,95) { TADR=m_ea; } MN rm_reg() FN { L=res(2, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  2,(XY+o)   */ OP(XY_CB,96) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(2, TDAT8); } MN wm() EOP
/* RES  2,A=(XY+o) */ OP(XY_CB,97) { TADR=m_ea; } MN rm_reg() FN { A=res(2, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  3,B=(XY+o) */ OP(XY_CB,98) { TADR=m_ea; } MN rm_reg() FN { B=res(3, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  3,C=(XY+o) */ OP(XY_CB,99) { TADR=m_ea; } MN rm_reg() FN { C=res(3, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  3,D=(XY+o) */ OP(XY_CB,9a) { TADR=m_ea; } MN rm_reg() FN { D=res(3, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  3,E=(XY+o) */ OP(XY_CB,9b) { TADR=m_ea; } MN rm_reg() FN { E=res(3, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  3,H=(XY+o) */ OP(XY_CB,9c) { TADR=m_ea; } MN rm_reg() FN { H=res(3, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  3,L=(XY+o) */ OP(XY_CB,9d) { TADR=m_ea; } MN rm_reg() FN { L=res(3, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  3,(XY+o)   */ OP(XY_CB,9e) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(3, TDAT8); } MN wm() EOP
/* RES  3,A=(XY+o) */ OP(XY_CB,9f) { TADR=m_ea; } MN rm_reg() FN { A=res(3, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  4,B=(XY+o) */ OP(XY_CB,a0) { TADR=m_ea; } MN rm_reg() FN { B=res(4, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  4,C=(XY+o) */ OP(XY_CB,a1) { TADR=m_ea; } MN rm_reg() FN { C=res(4, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  4,D=(XY+o) */ OP(XY_CB,a2) { TADR=m_ea; } MN rm_reg() FN { D=res(4, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  4,E=(XY+o) */ OP(XY_CB,a3) { TADR=m_ea; } MN rm_reg() FN { E=res(4, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  4,H=(XY+o) */ OP(XY_CB,a4) { TADR=m_ea; } MN rm_reg() FN { H=res(4, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  4,L=(XY+o) */ OP(XY_CB,a5) { TADR=m_ea; } MN rm_reg() FN { L=res(4, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  4,(XY+o)   */ OP(XY_CB,a6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(4, TDAT8); } MN wm() EOP
/* RES  4,A=(XY+o) */ OP(XY_CB,a7) { TADR=m_ea; } MN rm_reg() FN { A=res(4, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  5,B=(XY+o) */ OP(XY_CB,a8) { TADR=m_ea; } MN rm_reg() FN { B=res(5, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  5,C=(XY+o) */ OP(XY_CB,a9) { TADR=m_ea; } MN rm_reg() FN { C=res(5, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  5,D=(XY+o) */ OP(XY_CB,aa) { TADR=m_ea; } MN rm_reg() FN { D=res(5, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  5,E=(XY+o) */ OP(XY_CB,ab) { TADR=m_ea; } MN rm_reg() FN { E=res(5, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  5,H=(XY+o) */ OP(XY_CB,ac) { TADR=m_ea; } MN rm_reg() FN { H=res(5, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  5,L=(XY+o) */ OP(XY_CB,ad) { TADR=m_ea; } MN rm_reg() FN { L=res(5, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  5,(XY+o)   */ OP(XY_CB,ae) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(5, TDAT8); } MN wm() EOP
/* RES  5,A=(XY+o) */ OP(XY_CB,af) { TADR=m_ea; } MN rm_reg() FN { A=res(5, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  6,B=(XY+o) */ OP(XY_CB,b0) { TADR=m_ea; } MN rm_reg() FN { B=res(6, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  6,C=(XY+o) */ OP(XY_CB,b1) { TADR=m_ea; } MN rm_reg() FN { C=res(6, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  6,D=(XY+o) */ OP(XY_CB,b2) { TADR=m_ea; } MN rm_reg() FN { D=res(6, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  6,E=(XY+o) */ OP(XY_CB,b3) { TADR=m_ea; } MN rm_reg() FN { E=res(6, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  6,H=(XY+o) */ OP(XY_CB,b4) { TADR=m_ea; } MN rm_reg() FN { H=res(6, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  6,L=(XY+o) */ OP(XY_CB,b5) { TADR=m_ea; } MN rm_reg() FN { L=res(6, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  6,(XY+o)   */ OP(XY_CB,b6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(6, TDAT8); } MN wm() EOP
/* RES  6,A=(XY+o) */ OP(XY_CB,b7) { TADR=m_ea; } MN rm_reg() FN { A=res(6, TDAT8); TDAT8=A; } MN wm() EOP

/* RES  7,B=(XY+o) */ OP(XY_CB,b8) { TADR=m_ea; } MN rm_reg() FN { B=res(7, TDAT8); TDAT8=B; } MN wm() EOP
/* RES  7,C=(XY+o) */ OP(XY_CB,b9) { TADR=m_ea; } MN rm_reg() FN { C=res(7, TDAT8); TDAT8=C; } MN wm() EOP
/* RES  7,D=(XY+o) */ OP(XY_CB,ba) { TADR=m_ea; } MN rm_reg() FN { D=res(7, TDAT8); TDAT8=D; } MN wm() EOP
/* RES  7,E=(XY+o) */ OP(XY_CB,bb) { TADR=m_ea; } MN rm_reg() FN { E=res(7, TDAT8); TDAT8=E; } MN wm() EOP
/* RES  7,H=(XY+o) */ OP(XY_CB,bc) { TADR=m_ea; } MN rm_reg() FN { H=res(7, TDAT8); TDAT8=H; } MN wm() EOP
/* RES  7,L=(XY+o) */ OP(XY_CB,bd) { TADR=m_ea; } MN rm_reg() FN { L=res(7, TDAT8); TDAT8=L; } MN wm() EOP
/* RES  7,(XY+o)   */ OP(XY_CB,be) { TADR=m_ea; } MN rm_reg() FN { TDAT8=res(7, TDAT8); } MN wm() EOP
/* RES  7,A=(XY+o) */ OP(XY_CB,bf) { TADR=m_ea; } MN rm_reg() FN { A=res(7, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  0,B=(XY+o) */ OP(XY_CB,c0) { TADR=m_ea; } MN rm_reg() FN { B=set(0, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  0,C=(XY+o) */ OP(XY_CB,c1) { TADR=m_ea; } MN rm_reg() FN { C=set(0, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  0,D=(XY+o) */ OP(XY_CB,c2) { TADR=m_ea; } MN rm_reg() FN { D=set(0, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  0,E=(XY+o) */ OP(XY_CB,c3) { TADR=m_ea; } MN rm_reg() FN { E=set(0, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  0,H=(XY+o) */ OP(XY_CB,c4) { TADR=m_ea; } MN rm_reg() FN { H=set(0, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  0,L=(XY+o) */ OP(XY_CB,c5) { TADR=m_ea; } MN rm_reg() FN { L=set(0, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  0,(XY+o)   */ OP(XY_CB,c6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(0, TDAT8); } MN wm() EOP
/* SET  0,A=(XY+o) */ OP(XY_CB,c7) { TADR=m_ea; } MN rm_reg() FN { A=set(0, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  1,B=(XY+o) */ OP(XY_CB,c8) { TADR=m_ea; } MN rm_reg() FN { B=set(1, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  1,C=(XY+o) */ OP(XY_CB,c9) { TADR=m_ea; } MN rm_reg() FN { C=set(1, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  1,D=(XY+o) */ OP(XY_CB,ca) { TADR=m_ea; } MN rm_reg() FN { D=set(1, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  1,E=(XY+o) */ OP(XY_CB,cb) { TADR=m_ea; } MN rm_reg() FN { E=set(1, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  1,H=(XY+o) */ OP(XY_CB,cc) { TADR=m_ea; } MN rm_reg() FN { H=set(1, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  1,L=(XY+o) */ OP(XY_CB,cd) { TADR=m_ea; } MN rm_reg() FN { L=set(1, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  1,(XY+o)   */ OP(XY_CB,ce) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(1, TDAT8); } MN wm() EOP
/* SET  1,A=(XY+o) */ OP(XY_CB,cf) { TADR=m_ea; } MN rm_reg() FN { A=set(1, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  2,B=(XY+o) */ OP(XY_CB,d0) { TADR=m_ea; } MN rm_reg() FN { B=set(2, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  2,C=(XY+o) */ OP(XY_CB,d1) { TADR=m_ea; } MN rm_reg() FN { C=set(2, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  2,D=(XY+o) */ OP(XY_CB,d2) { TADR=m_ea; } MN rm_reg() FN { D=set(2, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  2,E=(XY+o) */ OP(XY_CB,d3) { TADR=m_ea; } MN rm_reg() FN { E=set(2, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  2,H=(XY+o) */ OP(XY_CB,d4) { TADR=m_ea; } MN rm_reg() FN { H=set(2, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  2,L=(XY+o) */ OP(XY_CB,d5) { TADR=m_ea; } MN rm_reg() FN { L=set(2, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  2,(XY+o)   */ OP(XY_CB,d6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(2, TDAT8); } MN wm() EOP
/* SET  2,A=(XY+o) */ OP(XY_CB,d7) { TADR=m_ea; } MN rm_reg() FN { A=set(2, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  3,B=(XY+o) */ OP(XY_CB,d8) { TADR=m_ea; } MN rm_reg() FN { B=set(3, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  3,C=(XY+o) */ OP(XY_CB,d9) { TADR=m_ea; } MN rm_reg() FN { C=set(3, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  3,D=(XY+o) */ OP(XY_CB,da) { TADR=m_ea; } MN rm_reg() FN { D=set(3, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  3,E=(XY+o) */ OP(XY_CB,db) { TADR=m_ea; } MN rm_reg() FN { E=set(3, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  3,H=(XY+o) */ OP(XY_CB,dc) { TADR=m_ea; } MN rm_reg() FN { H=set(3, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  3,L=(XY+o) */ OP(XY_CB,dd) { TADR=m_ea; } MN rm_reg() FN { L=set(3, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  3,(XY+o)   */ OP(XY_CB,de) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(3, TDAT8); } MN wm() EOP
/* SET  3,A=(XY+o) */ OP(XY_CB,df) { TADR=m_ea; } MN rm_reg() FN { A=set(3, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  4,B=(XY+o) */ OP(XY_CB,e0) { TADR=m_ea; } MN rm_reg() FN { B=set(4, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  4,C=(XY+o) */ OP(XY_CB,e1) { TADR=m_ea; } MN rm_reg() FN { C=set(4, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  4,D=(XY+o) */ OP(XY_CB,e2) { TADR=m_ea; } MN rm_reg() FN { D=set(4, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  4,E=(XY+o) */ OP(XY_CB,e3) { TADR=m_ea; } MN rm_reg() FN { E=set(4, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  4,H=(XY+o) */ OP(XY_CB,e4) { TADR=m_ea; } MN rm_reg() FN { H=set(4, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  4,L=(XY+o) */ OP(XY_CB,e5) { TADR=m_ea; } MN rm_reg() FN { L=set(4, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  4,(XY+o)   */ OP(XY_CB,e6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(4, TDAT8); } MN wm() EOP
/* SET  4,A=(XY+o) */ OP(XY_CB,e7) { TADR=m_ea; } MN rm_reg() FN { A=set(4, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  5,B=(XY+o) */ OP(XY_CB,e8) { TADR=m_ea; } MN rm_reg() FN { B=set(5, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  5,C=(XY+o) */ OP(XY_CB,e9) { TADR=m_ea; } MN rm_reg() FN { C=set(5, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  5,D=(XY+o) */ OP(XY_CB,ea) { TADR=m_ea; } MN rm_reg() FN { D=set(5, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  5,E=(XY+o) */ OP(XY_CB,eb) { TADR=m_ea; } MN rm_reg() FN { E=set(5, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  5,H=(XY+o) */ OP(XY_CB,ec) { TADR=m_ea; } MN rm_reg() FN { H=set(5, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  5,L=(XY+o) */ OP(XY_CB,ed) { TADR=m_ea; } MN rm_reg() FN { L=set(5, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  5,(XY+o)   */ OP(XY_CB,ee) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(5, TDAT8); } MN wm() EOP
/* SET  5,A=(XY+o) */ OP(XY_CB,ef) { TADR=m_ea; } MN rm_reg() FN { A=set(5, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  6,B=(XY+o) */ OP(XY_CB,f0) { TADR=m_ea; } MN rm_reg() FN { B=set(6, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  6,C=(XY+o) */ OP(XY_CB,f1) { TADR=m_ea; } MN rm_reg() FN { C=set(6, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  6,D=(XY+o) */ OP(XY_CB,f2) { TADR=m_ea; } MN rm_reg() FN { D=set(6, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  6,E=(XY+o) */ OP(XY_CB,f3) { TADR=m_ea; } MN rm_reg() FN { E=set(6, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  6,H=(XY+o) */ OP(XY_CB,f4) { TADR=m_ea; } MN rm_reg() FN { H=set(6, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  6,L=(XY+o) */ OP(XY_CB,f5) { TADR=m_ea; } MN rm_reg() FN { L=set(6, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  6,(XY+o)   */ OP(XY_CB,f6) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(6, TDAT8); } MN wm() EOP
/* SET  6,A=(XY+o) */ OP(XY_CB,f7) { TADR=m_ea; } MN rm_reg() FN { A=set(6, TDAT8); TDAT8=A; } MN wm() EOP

/* SET  7,B=(XY+o) */ OP(XY_CB,f8) { TADR=m_ea; } MN rm_reg() FN { B=set(7, TDAT8); TDAT8=B; } MN wm() EOP
/* SET  7,C=(XY+o) */ OP(XY_CB,f9) { TADR=m_ea; } MN rm_reg() FN { C=set(7, TDAT8); TDAT8=C; } MN wm() EOP
/* SET  7,D=(XY+o) */ OP(XY_CB,fa) { TADR=m_ea; } MN rm_reg() FN { D=set(7, TDAT8); TDAT8=D; } MN wm() EOP
/* SET  7,E=(XY+o) */ OP(XY_CB,fb) { TADR=m_ea; } MN rm_reg() FN { E=set(7, TDAT8); TDAT8=E; } MN wm() EOP
/* SET  7,H=(XY+o) */ OP(XY_CB,fc) { TADR=m_ea; } MN rm_reg() FN { H=set(7, TDAT8); TDAT8=H; } MN wm() EOP
/* SET  7,L=(XY+o) */ OP(XY_CB,fd) { TADR=m_ea; } MN rm_reg() FN { L=set(7, TDAT8); TDAT8=L; } MN wm() EOP
/* SET  7,(XY+o)   */ OP(XY_CB,fe) { TADR=m_ea; } MN rm_reg() FN { TDAT8=set(7, TDAT8); } MN wm() EOP
/* SET  7,A=(XY+o) */ OP(XY_CB,ff) { TADR=m_ea; } MN rm_reg() FN { A=set(7, TDAT8); TDAT8=A; } MN wm() EOP

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
/* DB   DD         */ OP(DD,00) { illegal_1(); } JP(00)
/* DB   DD         */ OP(DD,01) { illegal_1(); } JP(01)
/* DB   DD         */ OP(DD,02) { illegal_1(); } JP(02)
/* DB   DD         */ OP(DD,03) { illegal_1(); } JP(03)
/* DB   DD         */ OP(DD,04) { illegal_1(); } JP(04)
/* DB   DD         */ OP(DD,05) { illegal_1(); } JP(05)
/* DB   DD         */ OP(DD,06) { illegal_1(); } JP(06)
/* DB   DD         */ OP(DD,07) { illegal_1(); } JP(07)

/* DB   DD         */ OP(DD,08) { illegal_1(); } JP(08)
/* ADD  IX,BC      */ OP(DD,09) { TDAT=IX; TDAT2=BC; } MN add16() FN { IX=TDAT;    } EOP
/* DB   DD         */ OP(DD,0a) { illegal_1(); } JP(0a)
/* DB   DD         */ OP(DD,0b) { illegal_1(); } JP(0b)
/* DB   DD         */ OP(DD,0c) { illegal_1(); } JP(0c)
/* DB   DD         */ OP(DD,0d) { illegal_1(); } JP(0d)
/* DB   DD         */ OP(DD,0e) { illegal_1(); } JP(0e)
/* DB   DD         */ OP(DD,0f) { illegal_1(); } JP(0f)

/* DB   DD         */ OP(DD,10) { illegal_1(); } JP(10)
/* DB   DD         */ OP(DD,11) { illegal_1(); } JP(11)
/* DB   DD         */ OP(DD,12) { illegal_1(); } JP(12)
/* DB   DD         */ OP(DD,13) { illegal_1(); } JP(13)
/* DB   DD         */ OP(DD,14) { illegal_1(); } JP(14)
/* DB   DD         */ OP(DD,15) { illegal_1(); } JP(15)
/* DB   DD         */ OP(DD,16) { illegal_1(); } JP(16)
/* DB   DD         */ OP(DD,17) { illegal_1(); } JP(17)

/* DB   DD         */ OP(DD,18) { illegal_1(); } JP(18)
/* ADD  IX,DE      */ OP(DD,19) { TDAT=IX; TDAT2=DE; } MN add16() FN { IX=TDAT;    } EOP
/* DB   DD         */ OP(DD,1a) { illegal_1(); } JP(1a)
/* DB   DD         */ OP(DD,1b) { illegal_1(); } JP(1b)
/* DB   DD         */ OP(DD,1c) { illegal_1(); } JP(1c)
/* DB   DD         */ OP(DD,1d) { illegal_1(); } JP(1d)
/* DB   DD         */ OP(DD,1e) { illegal_1(); } JP(1e)
/* DB   DD         */ OP(DD,1f) { illegal_1(); } JP(1f)

/* DB   DD         */ OP(DD,20) { illegal_1(); } JP(20)
/* LD   IX,w       */ OP_M(DD,21) arg16() FN { IX = TDAT;                          } EOP
/* LD   (w),IX     */ OP_M(DD,22) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=IX; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* INC  IX         */ OP_M(DD,23) nomreq_ir(2) FN { IX++;                          } EOP
/* INC  HX         */ OP(DD,24) { inc(HX);                                         } EOP
/* DEC  HX         */ OP(DD,25) { dec(HX);                                         } EOP
/* LD   HX,n       */ OP_M(DD,26) arg() FN { HX = TDAT8;                           } EOP
/* DB   DD         */ OP(DD,27) { illegal_1(); } JP(27)

/* DB   DD         */ OP(DD,28) { illegal_1(); } JP(28)
/* ADD  IX,IX      */ OP(DD,29) { TDAT=IX; TDAT2=IX; } MN add16() FN { IX=TDAT;    } EOP
/* LD   IX,(w)     */ OP_M(DD,2a) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { IX=TDAT; WZ = m_ea + 1; } EOP
/* DEC  IX         */ OP_M(DD,2b) nomreq_ir(2) FN { IX--;                          } EOP
/* INC  LX         */ OP(DD,2c) { inc(LX);                                         } EOP
/* DEC  LX         */ OP(DD,2d) { dec(LX);                                         } EOP
/* LD   LX,n       */ OP_M(DD,2e) arg() FN { LX = TDAT8;                           } EOP
/* DB   DD         */ OP(DD,2f) { illegal_1(); } JP(2f)

/* DB   DD         */ OP(DD,30) { illegal_1(); } JP(30)
/* DB   DD         */ OP(DD,31) { illegal_1(); } JP(31)
/* DB   DD         */ OP(DD,32) { illegal_1(); } JP(32)
/* DB   DD         */ OP(DD,33) { illegal_1(); } JP(33)
/* INC  (IX+o)     */ OP_M(DD,34) eax() FN { TADR=PCD-1; } MN nomreq_addr(5) FN { TADR=m_ea; } MN rm_reg() FN { inc(TDAT8); } MN wm() EOP
/* DEC  (IX+o)     */ OP_M(DD,35) eax() FN { TADR=PCD-1; } MN nomreq_addr(5) FN { TADR=m_ea; } MN rm_reg() FN { dec(TDAT8); } MN wm() EOP
/* LD   (IX+o),n   */ OP_M(DD,36) eax() MN arg() FN { TADR=PCD-1; } MN nomreq_addr(2) FN { TADR=m_ea; } MN wm() EOP
/* DB   DD         */ OP(DD,37) { illegal_1(); } JP(37)

/* DB   DD         */ OP(DD,38) { illegal_1(); } JP(38)
/* ADD  IX,SP      */ OP(DD,39) { TDAT=IX; TDAT2=SP; } MN add16() FN { IX=TDAT;    } EOP
/* DB   DD         */ OP(DD,3a) { illegal_1(); } JP(3a)
/* DB   DD         */ OP(DD,3b) { illegal_1(); } JP(3b)
/* DB   DD         */ OP(DD,3c) { illegal_1(); } JP(3c)
/* DB   DD         */ OP(DD,3d) { illegal_1(); } JP(3d)
/* DB   DD         */ OP(DD,3e) { illegal_1(); } JP(3e)
/* DB   DD         */ OP(DD,3f) { illegal_1(); } JP(3f)

/* DB   DD         */ OP(DD,40) { illegal_1(); } JP(40)
/* DB   DD         */ OP(DD,41) { illegal_1(); } JP(41)
/* DB   DD         */ OP(DD,42) { illegal_1(); } JP(42)
/* DB   DD         */ OP(DD,43) { illegal_1(); } JP(43)
/* LD   B,HX       */ OP(DD,44) { B = HX;                                          } EOP
/* LD   B,LX       */ OP(DD,45) { B = LX;                                          } EOP
/* LD   B,(IX+o)   */ OP_M(DD,46) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { B = TDAT8; } EOP
/* DB   DD         */ OP(DD,47) { illegal_1(); } JP(47)

/* DB   DD         */ OP(DD,48) { illegal_1(); } JP(48)
/* DB   DD         */ OP(DD,49) { illegal_1(); } JP(49)
/* DB   DD         */ OP(DD,4a) { illegal_1(); } JP(4a)
/* DB   DD         */ OP(DD,4b) { illegal_1(); } JP(4b)
/* LD   C,HX       */ OP(DD,4c) { C = HX;                                          } EOP
/* LD   C,LX       */ OP(DD,4d) { C = LX;                                          } EOP
/* LD   C,(IX+o)   */ OP_M(DD,4e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { C = TDAT8; } EOP
/* DB   DD         */ OP(DD,4f) { illegal_1(); } JP(4f)

/* DB   DD         */ OP(DD,50) { illegal_1(); } JP(50)
/* DB   DD         */ OP(DD,51) { illegal_1(); } JP(51)
/* DB   DD         */ OP(DD,52) { illegal_1(); } JP(52)
/* DB   DD         */ OP(DD,53) { illegal_1(); } JP(53)
/* LD   D,HX       */ OP(DD,54) { D = HX;                                          } EOP
/* LD   D,LX       */ OP(DD,55) { D = LX;                                          } EOP
/* LD   D,(IX+o)   */ OP_M(DD,56) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { D = TDAT8; } EOP
/* DB   DD         */ OP(DD,57) { illegal_1(); } JP(57)

/* DB   DD         */ OP(DD,58) { illegal_1(); } JP(58)
/* DB   DD         */ OP(DD,59) { illegal_1(); } JP(59)
/* DB   DD         */ OP(DD,5a) { illegal_1(); } JP(5a)
/* DB   DD         */ OP(DD,5b) { illegal_1(); } JP(5b)
/* LD   E,HX       */ OP(DD,5c) { E = HX;                                          } EOP
/* LD   E,LX       */ OP(DD,5d) { E = LX;                                          } EOP
/* LD   E,(IX+o)   */ OP_M(DD,5e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { E = TDAT8; } EOP
/* DB   DD         */ OP(DD,5f) { illegal_1(); } JP(5f)

/* LD   HX,B       */ OP(DD,60) { HX = B;                                          } EOP
/* LD   HX,C       */ OP(DD,61) { HX = C;                                          } EOP
/* LD   HX,D       */ OP(DD,62) { HX = D;                                          } EOP
/* LD   HX,E       */ OP(DD,63) { HX = E;                                          } EOP
/* LD   HX,HX      */ OP_M(DD,64)                                                    EOP
/* LD   HX,LX      */ OP(DD,65) { HX = LX;                                         } EOP
/* LD   H,(IX+o)   */ OP_M(DD,66) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { H = TDAT8; } EOP
/* LD   HX,A       */ OP(DD,67) { HX = A;                                          } EOP

/* LD   LX,B       */ OP(DD,68) { LX = B;                                          } EOP
/* LD   LX,C       */ OP(DD,69) { LX = C;                                          } EOP
/* LD   LX,D       */ OP(DD,6a) { LX = D;                                          } EOP
/* LD   LX,E       */ OP(DD,6b) { LX = E;                                          } EOP
/* LD   LX,HX      */ OP(DD,6c) { LX = HX;                                         } EOP
/* LD   LX,LX      */ OP_M(DD,6d)                                                    EOP
/* LD   L,(IX+o)   */ OP_M(DD,6e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { L = TDAT8; } EOP
/* LD   LX,A       */ OP(DD,6f) { LX = A;                                          } EOP

/* LD   (IX+o),B   */ OP_M(DD,70) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = B; } MN wm() EOP
/* LD   (IX+o),C   */ OP_M(DD,71) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = C; } MN wm() EOP
/* LD   (IX+o),D   */ OP_M(DD,72) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = D; } MN wm() EOP
/* LD   (IX+o),E   */ OP_M(DD,73) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = E; } MN wm() EOP
/* LD   (IX+o),H   */ OP_M(DD,74) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = H; } MN wm() EOP
/* LD   (IX+o),L   */ OP_M(DD,75) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = L; } MN wm() EOP
/* DB   DD         */ OP(DD,76) { illegal_1(); } JP(76)
/* LD   (IX+o),A   */ OP_M(DD,77) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = A; } MN wm() EOP

/* DB   DD         */ OP(DD,78) { illegal_1(); } JP(78)
/* DB   DD         */ OP(DD,79) { illegal_1(); } JP(79)
/* DB   DD         */ OP(DD,7a) { illegal_1(); } JP(7a)
/* DB   DD         */ OP(DD,7b) { illegal_1(); } JP(7b)
/* LD   A,HX       */ OP(DD,7c) { A = HX;                                          } EOP
/* LD   A,LX       */ OP(DD,7d) { A = LX;                                          } EOP
/* LD   A,(IX+o)   */ OP_M(DD,7e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { A = TDAT8; } EOP
/* DB   DD         */ OP(DD,7f) { illegal_1(); } JP(7f)

/* DB   DD         */ OP(DD,80) { illegal_1(); } JP(80)
/* DB   DD         */ OP(DD,81) { illegal_1(); } JP(81)
/* DB   DD         */ OP(DD,82) { illegal_1(); } JP(82)
/* DB   DD         */ OP(DD,83) { illegal_1(); } JP(83)
/* ADD  A,HX       */ OP(DD,84) { add_a(HX);                                       } EOP
/* ADD  A,LX       */ OP(DD,85) { add_a(LX);                                       } EOP
/* ADD  A,(IX+o)   */ OP_M(DD,86) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { add_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,87) { illegal_1(); } JP(87)

/* DB   DD         */ OP(DD,88) { illegal_1(); } JP(88)
/* DB   DD         */ OP(DD,89) { illegal_1(); } JP(89)
/* DB   DD         */ OP(DD,8a) { illegal_1(); } JP(8a)
/* DB   DD         */ OP(DD,8b) { illegal_1(); } JP(8b)
/* ADC  A,HX       */ OP(DD,8c) { adc_a(HX);                                       } EOP
/* ADC  A,LX       */ OP(DD,8d) { adc_a(LX);                                       } EOP
/* ADC  A,(IX+o)   */ OP_M(DD,8e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { adc_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,8f) { illegal_1(); } JP(8f)

/* DB   DD         */ OP(DD,90) { illegal_1(); } JP(90)
/* DB   DD         */ OP(DD,91) { illegal_1(); } JP(91)
/* DB   DD         */ OP(DD,92) { illegal_1(); } JP(92)
/* DB   DD         */ OP(DD,93) { illegal_1(); } JP(93)
/* SUB  HX         */ OP(DD,94) { sub(HX);                                         } EOP
/* SUB  LX         */ OP(DD,95) { sub(LX);                                         } EOP
/* SUB  (IX+o)     */ OP_M(DD,96) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { sub(TDAT8); } EOP
/* DB   DD         */ OP(DD,97) { illegal_1(); } JP(97)

/* DB   DD         */ OP(DD,98) { illegal_1(); } JP(98)
/* DB   DD         */ OP(DD,99) { illegal_1(); } JP(99)
/* DB   DD         */ OP(DD,9a) { illegal_1(); } JP(9a)
/* DB   DD         */ OP(DD,9b) { illegal_1(); } JP(9b)
/* SBC  A,HX       */ OP(DD,9c) { sbc_a(HX);                                       } EOP
/* SBC  A,LX       */ OP(DD,9d) { sbc_a(LX);                                       } EOP
/* SBC  A,(IX+o)   */ OP_M(DD,9e) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { sbc_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,9f) { illegal_1(); } JP(9f)

/* DB   DD         */ OP(DD,a0) { illegal_1(); } JP(a0)
/* DB   DD         */ OP(DD,a1) { illegal_1(); } JP(a1)
/* DB   DD         */ OP(DD,a2) { illegal_1(); } JP(a2)
/* DB   DD         */ OP(DD,a3) { illegal_1(); } JP(a3)
/* AND  HX         */ OP(DD,a4) { and_a(HX);                                       } EOP
/* AND  LX         */ OP(DD,a5) { and_a(LX);                                       } EOP
/* AND  (IX+o)     */ OP_M(DD,a6) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { and_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,a7) { illegal_1(); } JP(a7)

/* DB   DD         */ OP(DD,a8) { illegal_1(); } JP(a8)
/* DB   DD         */ OP(DD,a9) { illegal_1(); } JP(a9)
/* DB   DD         */ OP(DD,aa) { illegal_1(); } JP(aa)
/* DB   DD         */ OP(DD,ab) { illegal_1(); } JP(ab)
/* XOR  HX         */ OP(DD,ac) { xor_a(HX);                                       } EOP
/* XOR  LX         */ OP(DD,ad) { xor_a(LX);                                       } EOP
/* XOR  (IX+o)     */ OP_M(DD,ae) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { xor_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,af) { illegal_1(); } JP(af)

/* DB   DD         */ OP(DD,b0) { illegal_1(); } JP(b0)
/* DB   DD         */ OP(DD,b1) { illegal_1(); } JP(b1)
/* DB   DD         */ OP(DD,b2) { illegal_1(); } JP(b2)
/* DB   DD         */ OP(DD,b3) { illegal_1(); } JP(b3)
/* OR   HX         */ OP(DD,b4) { or_a(HX);                                        } EOP
/* OR   LX         */ OP(DD,b5) { or_a(LX);                                        } EOP
/* OR   (IX+o)     */ OP_M(DD,b6) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { or_a(TDAT8); } EOP
/* DB   DD         */ OP(DD,b7) { illegal_1(); } JP(b7)

/* DB   DD         */ OP(DD,b8) { illegal_1(); } JP(b8)
/* DB   DD         */ OP(DD,b9) { illegal_1(); } JP(b9)
/* DB   DD         */ OP(DD,ba) { illegal_1(); } JP(ba)
/* DB   DD         */ OP(DD,bb) { illegal_1(); } JP(bb)
/* CP   HX         */ OP(DD,bc) { cp(HX);                                          } EOP
/* CP   LX         */ OP(DD,bd) { cp(LX);                                          } EOP
/* CP   (IX+o)     */ OP_M(DD,be) eax() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { cp(TDAT8); } EOP
/* DB   DD         */ OP(DD,bf) { illegal_1(); } JP(bf)

/* DB   DD         */ OP(DD,c0) { illegal_1(); } JP(c0)
/* DB   DD         */ OP(DD,c1) { illegal_1(); } JP(c1)
/* DB   DD         */ OP(DD,c2) { illegal_1(); } JP(c2)
/* DB   DD         */ OP(DD,c3) { illegal_1(); } JP(c3)
/* DB   DD         */ OP(DD,c4) { illegal_1(); } JP(c4)
/* DB   DD         */ OP(DD,c5) { illegal_1(); } JP(c5)
/* DB   DD         */ OP(DD,c6) { illegal_1(); } JP(c6)
/* DB   DD         */ OP(DD,c7) { illegal_1(); } JP(c7)

/* DB   DD         */ OP(DD,c8) { illegal_1(); } JP(c8)
/* DB   DD         */ OP(DD,c9) { illegal_1(); } JP(c9)
/* DB   DD         */ OP(DD,ca) { illegal_1(); } JP(ca)
/* **   DD CB xx   */ OP_M(DD,cb) eax() MN arg() FN { TADR=PCD-1; } MN nomreq_addr(2) JP_P(XY_CB)
/* DB   DD         */ OP(DD,cc) { illegal_1(); } JP(cc)
/* DB   DD         */ OP(DD,cd) { illegal_1(); } JP(cd)
/* DB   DD         */ OP(DD,ce) { illegal_1(); } JP(ce)
/* DB   DD         */ OP(DD,cf) { illegal_1(); } JP(cf)

/* DB   DD         */ OP(DD,d0) { illegal_1(); } JP(d0)
/* DB   DD         */ OP(DD,d1) { illegal_1(); } JP(d1)
/* DB   DD         */ OP(DD,d2) { illegal_1(); } JP(d2)
/* DB   DD         */ OP(DD,d3) { illegal_1(); } JP(d3)
/* DB   DD         */ OP(DD,d4) { illegal_1(); } JP(d4)
/* DB   DD         */ OP(DD,d5) { illegal_1(); } JP(d5)
/* DB   DD         */ OP(DD,d6) { illegal_1(); } JP(d6)
/* DB   DD         */ OP(DD,d7) { illegal_1(); } JP(d7)

/* DB   DD         */ OP(DD,d8) { illegal_1(); } JP(d8)
/* DB   DD         */ OP(DD,d9) { illegal_1(); } JP(d9)
/* DB   DD         */ OP(DD,da) { illegal_1(); } JP(da)
/* DB   DD         */ OP(DD,db) { illegal_1(); } JP(db)
/* DB   DD         */ OP(DD,dc) { illegal_1(); } JP(dc)
/* DB   DD         */ OP(DD,dd) { illegal_1(); } JP(dd)
/* DB   DD         */ OP(DD,de) { illegal_1(); } JP(de)
/* DB   DD         */ OP(DD,df) { illegal_1(); } JP(df)

/* DB   DD         */ OP(DD,e0) { illegal_1(); } JP(e0)
/* POP  IX         */ OP_M(DD,e1) pop() FN { IX=TDAT;                              } EOP
/* DB   DD         */ OP(DD,e2) { illegal_1(); } JP(e2)
/* EX   (SP),IX    */ OP(DD,e3) { TDAT=IX; } MN ex_sp() FN { IX=TDAT; }              EOP
/* DB   DD         */ OP(DD,e4) { illegal_1(); } JP(e4)
/* PUSH IX         */ OP(DD,e5) { TDAT=IX; } MN push()                               EOP
/* DB   DD         */ OP(DD,e6) { illegal_1(); } JP(e6)
/* DB   DD         */ OP(DD,e7) { illegal_1(); } JP(e7)

/* DB   DD         */ OP(DD,e8) { illegal_1(); } JP(e8)
/* JP   (IX)       */ OP(DD,e9) { PC = IX;                                         } EOP
/* DB   DD         */ OP(DD,ea) { illegal_1(); } JP(ea)
/* DB   DD         */ OP(DD,eb) { illegal_1(); } JP(eb)
/* DB   DD         */ OP(DD,ec) { illegal_1(); } JP(ec)
/* DB   DD         */ OP(DD,ed) { illegal_1(); } JP(ed)
/* DB   DD         */ OP(DD,ee) { illegal_1(); } JP(ee)
/* DB   DD         */ OP(DD,ef) { illegal_1(); } JP(ef)

/* DB   DD         */ OP(DD,f0) { illegal_1(); } JP(f0)
/* DB   DD         */ OP(DD,f1) { illegal_1(); } JP(f1)
/* DB   DD         */ OP(DD,f2) { illegal_1(); } JP(f2)
/* DB   DD         */ OP(DD,f3) { illegal_1(); } JP(f3)
/* DB   DD         */ OP(DD,f4) { illegal_1(); } JP(f4)
/* DB   DD         */ OP(DD,f5) { illegal_1(); } JP(f5)
/* DB   DD         */ OP(DD,f6) { illegal_1(); } JP(f6)
/* DB   DD         */ OP(DD,f7) { illegal_1(); } JP(f7)

/* DB   DD         */ OP(DD,f8) { illegal_1(); } JP(f8)
/* LD   SP,IX      */ OP_M(DD,f9) nomreq_ir(2) FN { SP = IX;                       } EOP
/* DB   DD         */ OP(DD,fa) { illegal_1(); } JP(fa)
/* DB   DD         */ OP(DD,fb) { illegal_1(); } JP(fb)
/* DB   DD         */ OP(DD,fc) { illegal_1(); } JP(fc)
/* DB   DD         */ OP(DD,fd) { illegal_1(); } JP(fd)
/* DB   DD         */ OP(DD,fe) { illegal_1(); } JP(fe)
/* DB   DD         */ OP(DD,ff) { illegal_1(); } JP(ff)

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
/* DB   FD         */ OP(FD,00) { illegal_1(); } JP(00)
/* DB   FD         */ OP(FD,01) { illegal_1(); } JP(01)
/* DB   FD         */ OP(FD,02) { illegal_1(); } JP(02)
/* DB   FD         */ OP(FD,03) { illegal_1(); } JP(03)
/* DB   FD         */ OP(FD,04) { illegal_1(); } JP(04)
/* DB   FD         */ OP(FD,05) { illegal_1(); } JP(05)
/* DB   FD         */ OP(FD,06) { illegal_1(); } JP(06)
/* DB   FD         */ OP(FD,07) { illegal_1(); } JP(07)

/* DB   FD         */ OP(FD,08) { illegal_1(); } JP(08)
/* ADD  IY,BC      */ OP(FD,09) { TDAT=IY; TDAT2=BC; } MN add16() FN { IY=TDAT;    } EOP
/* DB   FD         */ OP(FD,0a) { illegal_1(); } JP(0a)
/* DB   FD         */ OP(FD,0b) { illegal_1(); } JP(0b)
/* DB   FD         */ OP(FD,0c) { illegal_1(); } JP(0c)
/* DB   FD         */ OP(FD,0d) { illegal_1(); } JP(0d)
/* DB   FD         */ OP(FD,0e) { illegal_1(); } JP(0e)
/* DB   FD         */ OP(FD,0f) { illegal_1(); } JP(0f)

/* DB   FD         */ OP(FD,10) { illegal_1(); } JP(10)
/* DB   FD         */ OP(FD,11) { illegal_1(); } JP(11)
/* DB   FD         */ OP(FD,12) { illegal_1(); } JP(12)
/* DB   FD         */ OP(FD,13) { illegal_1(); } JP(13)
/* DB   FD         */ OP(FD,14) { illegal_1(); } JP(14)
/* DB   FD         */ OP(FD,15) { illegal_1(); } JP(15)
/* DB   FD         */ OP(FD,16) { illegal_1(); } JP(16)
/* DB   FD         */ OP(FD,17) { illegal_1(); } JP(17)

/* DB   FD         */ OP(FD,18) { illegal_1(); } JP(18)
/* ADD  IY,DE      */ OP(FD,19) { TDAT=IY; TDAT2=DE; } MN add16() FN { IY=TDAT;    } EOP
/* DB   FD         */ OP(FD,1a) { illegal_1(); } JP(1a)
/* DB   FD         */ OP(FD,1b) { illegal_1(); } JP(1b)
/* DB   FD         */ OP(FD,1c) { illegal_1(); } JP(1c)
/* DB   FD         */ OP(FD,1d) { illegal_1(); } JP(1d)
/* DB   FD         */ OP(FD,1e) { illegal_1(); } JP(1e)
/* DB   FD         */ OP(FD,1f) { illegal_1(); } JP(1f)

/* DB   FD         */ OP(FD,20) { illegal_1(); } JP(20)
/* LD   IY,w       */ OP_M(FD,21) arg16() FN { IY = TDAT;                          } EOP
/* LD   (w),IY     */ OP_M(FD,22) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=IY; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* INC  IY         */ OP_M(FD,23) nomreq_ir(2) FN { IY++;                          } EOP
/* INC  HY         */ OP(FD,24) { inc(HY);                                         } EOP
/* DEC  HY         */ OP(FD,25) { dec(HY);                                         } EOP
/* LD   HY,n       */ OP_M(FD,26) arg() FN { HY = TDAT8;                           } EOP
/* DB   FD         */ OP(FD,27) { illegal_1(); } JP(27)

/* DB   FD         */ OP(FD,28) { illegal_1(); } JP(28)
/* ADD  IY,IY      */ OP(FD,29) { TDAT=IY; TDAT2=IY; } MN add16() FN { IY=TDAT;    } EOP
/* LD   IY,(w)     */ OP_M(FD,2a) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { IY=TDAT; WZ = m_ea + 1; } EOP
/* DEC  IY         */ OP_M(FD,2b) nomreq_ir(2) FN { IY--;                          } EOP
/* INC  LY         */ OP(FD,2c) { inc(LY);                                         } EOP
/* DEC  LY         */ OP(FD,2d) { dec(LY);                                         } EOP
/* LD   LY,n       */ OP_M(FD,2e) arg() FN { LY = TDAT8;                           } EOP
/* DB   FD         */ OP(FD,2f) { illegal_1(); } JP(2f)

/* DB   FD         */ OP(FD,30) { illegal_1(); } JP(30)
/* DB   FD         */ OP(FD,31) { illegal_1(); } JP(31)
/* DB   FD         */ OP(FD,32) { illegal_1(); } JP(32)
/* DB   FD         */ OP(FD,33) { illegal_1(); } JP(33)
/* INC  (IY+o)     */ OP_M(FD,34) eay() FN { TADR=PCD-1; } MN nomreq_addr(5) FN { TADR=m_ea; } MN rm_reg() FN { inc(TDAT8); } MN wm() EOP
/* DEC  (IY+o)     */ OP_M(FD,35) eay() FN { TADR=PCD-1; } MN nomreq_addr(5) FN { TADR=m_ea; } MN rm_reg() FN { dec(TDAT8); } MN wm() EOP
/* LD   (IY+o),n   */ OP_M(FD,36) eay() MN arg() FN { TADR=PCD-1; } MN nomreq_addr(2) FN { TADR=m_ea; } MN wm() EOP
/* DB   FD         */ OP(FD,37) { illegal_1(); } JP(37)

/* DB   FD         */ OP(FD,38) { illegal_1(); } JP(38)
/* ADD  IY,SP      */ OP(FD,39) { TDAT=IY; TDAT2=SP; } MN add16() FN { IY=TDAT;    } EOP
/* DB   FD         */ OP(FD,3a) { illegal_1(); } JP(3a)
/* DB   FD         */ OP(FD,3b) { illegal_1(); } JP(3b)
/* DB   FD         */ OP(FD,3c) { illegal_1(); } JP(3c)
/* DB   FD         */ OP(FD,3d) { illegal_1(); } JP(3d)
/* DB   FD         */ OP(FD,3e) { illegal_1(); } JP(3e)
/* DB   FD         */ OP(FD,3f) { illegal_1(); } JP(3f)

/* DB   FD         */ OP(FD,40) { illegal_1(); } JP(40)
/* DB   FD         */ OP(FD,41) { illegal_1(); } JP(41)
/* DB   FD         */ OP(FD,42) { illegal_1(); } JP(42)
/* DB   FD         */ OP(FD,43) { illegal_1(); } JP(43)
/* LD   B,HY       */ OP(FD,44) { B = HY;                                          } EOP
/* LD   B,LY       */ OP(FD,45) { B = LY;                                          } EOP
/* LD   B,(IY+o)   */ OP_M(FD,46) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { B = TDAT8; } EOP
/* DB   FD         */ OP(FD,47) { illegal_1(); } JP(47)

/* DB   FD         */ OP(FD,48) { illegal_1(); } JP(48)
/* DB   FD         */ OP(FD,49) { illegal_1(); } JP(49)
/* DB   FD         */ OP(FD,4a) { illegal_1(); } JP(4a)
/* DB   FD         */ OP(FD,4b) { illegal_1(); } JP(4b)
/* LD   C,HY       */ OP(FD,4c) { C = HY;                                          } EOP
/* LD   C,LY       */ OP(FD,4d) { C = LY;                                          } EOP
/* LD   C,(IY+o)   */ OP_M(FD,4e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { C = TDAT8; } EOP
/* DB   FD         */ OP(FD,4f) { illegal_1(); } JP(4f)

/* DB   FD         */ OP(FD,50) { illegal_1(); } JP(50)
/* DB   FD         */ OP(FD,51) { illegal_1(); } JP(51)
/* DB   FD         */ OP(FD,52) { illegal_1(); } JP(52)
/* DB   FD         */ OP(FD,53) { illegal_1(); } JP(53)
/* LD   D,HY       */ OP(FD,54) { D = HY;                                          } EOP
/* LD   D,LY       */ OP(FD,55) { D = LY;                                          } EOP
/* LD   D,(IY+o)   */ OP_M(FD,56) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { D = TDAT8; } EOP
/* DB   FD         */ OP(FD,57) { illegal_1(); } JP(57)

/* DB   FD         */ OP(FD,58) { illegal_1(); } JP(58)
/* DB   FD         */ OP(FD,59) { illegal_1(); } JP(59)
/* DB   FD         */ OP(FD,5a) { illegal_1(); } JP(5a)
/* DB   FD         */ OP(FD,5b) { illegal_1(); } JP(5b)
/* LD   E,HY       */ OP(FD,5c) { E = HY;                                          } EOP
/* LD   E,LY       */ OP(FD,5d) { E = LY;                                          } EOP
/* LD   E,(IY+o)   */ OP_M(FD,5e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { E = TDAT8; } EOP
/* DB   FD         */ OP(FD,5f) { illegal_1(); } JP(5f)

/* LD   HY,B       */ OP(FD,60) { HY = B;                                          } EOP
/* LD   HY,C       */ OP(FD,61) { HY = C;                                          } EOP
/* LD   HY,D       */ OP(FD,62) { HY = D;                                          } EOP
/* LD   HY,E       */ OP(FD,63) { HY = E;                                          } EOP
/* LD   HY,HY      */ OP_M(FD,64)                                                    EOP
/* LD   HY,LY      */ OP(FD,65) { HY = LY;                                         } EOP
/* LD   H,(IY+o)   */ OP_M(FD,66) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { H = TDAT8; } EOP
/* LD   HY,A       */ OP(FD,67) { HY = A;                                          } EOP

/* LD   LY,B       */ OP(FD,68) { LY = B;                                          } EOP
/* LD   LY,C       */ OP(FD,69) { LY = C;                                          } EOP
/* LD   LY,D       */ OP(FD,6a) { LY = D;                                          } EOP
/* LD   LY,E       */ OP(FD,6b) { LY = E;                                          } EOP
/* LD   LY,HY      */ OP(FD,6c) { LY = HY;                                         } EOP
/* LD   LY,LY      */ OP_M(FD,6d)                                                    EOP
/* LD   L,(IY+o)   */ OP_M(FD,6e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { L = TDAT8; } EOP
/* LD   LY,A       */ OP(FD,6f) { LY = A;                                          } EOP

/* LD   (IY+o),B   */ OP_M(FD,70) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = B; } MN wm() EOP
/* LD   (IY+o),C   */ OP_M(FD,71) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = C; } MN wm() EOP
/* LD   (IY+o),D   */ OP_M(FD,72) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = D; } MN wm() EOP
/* LD   (IY+o),E   */ OP_M(FD,73) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = E; } MN wm() EOP
/* LD   (IY+o),H   */ OP_M(FD,74) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = H; } MN wm() EOP
/* LD   (IY+o),L   */ OP_M(FD,75) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = L; } MN wm() EOP
/* DB   FD         */ OP(FD,76) { illegal_1(); } JP(76)
/* LD   (IY+o),A   */ OP_M(FD,77) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; TDAT8 = A; } MN wm() EOP

/* DB   FD         */ OP(FD,78) { illegal_1(); } JP(78)
/* DB   FD         */ OP(FD,79) { illegal_1(); } JP(79)
/* DB   FD         */ OP(FD,7a) { illegal_1(); } JP(7a)
/* DB   FD         */ OP(FD,7b) { illegal_1(); } JP(7b)
/* LD   A,HY       */ OP(FD,7c) { A = HY;                                          } EOP
/* LD   A,LY       */ OP(FD,7d) { A = LY;                                          } EOP
/* LD   A,(IY+o)   */ OP_M(FD,7e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { A = TDAT8; } EOP
/* DB   FD         */ OP(FD,7f) { illegal_1(); } JP(7f)

/* DB   FD         */ OP(FD,80) { illegal_1(); } JP(80)
/* DB   FD         */ OP(FD,81) { illegal_1(); } JP(81)
/* DB   FD         */ OP(FD,82) { illegal_1(); } JP(82)
/* DB   FD         */ OP(FD,83) { illegal_1(); } JP(83)
/* ADD  A,HY       */ OP(FD,84) { add_a(HY);                                       } EOP
/* ADD  A,LY       */ OP(FD,85) { add_a(LY);                                       } EOP
/* ADD  A,(IY+o)   */ OP_M(FD,86) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { add_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,87) { illegal_1(); } JP(87)

/* DB   FD         */ OP(FD,88) { illegal_1(); } JP(88)
/* DB   FD         */ OP(FD,89) { illegal_1(); } JP(89)
/* DB   FD         */ OP(FD,8a) { illegal_1(); } JP(8a)
/* DB   FD         */ OP(FD,8b) { illegal_1(); } JP(8b)
/* ADC  A,HY       */ OP(FD,8c) { adc_a(HY);                                       } EOP
/* ADC  A,LY       */ OP(FD,8d) { adc_a(LY);                                       } EOP
/* ADC  A,(IY+o)   */ OP_M(FD,8e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { adc_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,8f) { illegal_1(); } JP(8f)

/* DB   FD         */ OP(FD,90) { illegal_1(); } JP(90)
/* DB   FD         */ OP(FD,91) { illegal_1(); } JP(91)
/* DB   FD         */ OP(FD,92) { illegal_1(); } JP(92)
/* DB   FD         */ OP(FD,93) { illegal_1(); } JP(93)
/* SUB  HY         */ OP(FD,94) { sub(HY);                                         } EOP
/* SUB  LY         */ OP(FD,95) { sub(LY);                                         } EOP
/* SUB  (IY+o)     */ OP_M(FD,96) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { sub(TDAT8); } EOP
/* DB   FD         */ OP(FD,97) { illegal_1(); } JP(97)

/* DB   FD         */ OP(FD,98) { illegal_1(); } JP(98)
/* DB   FD         */ OP(FD,99) { illegal_1(); } JP(99)
/* DB   FD         */ OP(FD,9a) { illegal_1(); } JP(9a)
/* DB   FD         */ OP(FD,9b) { illegal_1(); } JP(9b)
/* SBC  A,HY       */ OP(FD,9c) { sbc_a(HY);                                       } EOP
/* SBC  A,LY       */ OP(FD,9d) { sbc_a(LY);                                       } EOP
/* SBC  A,(IY+o)   */ OP_M(FD,9e) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { sbc_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,9f) { illegal_1(); } JP(9f)

/* DB   FD         */ OP(FD,a0) { illegal_1(); } JP(a0)
/* DB   FD         */ OP(FD,a1) { illegal_1(); } JP(a1)
/* DB   FD         */ OP(FD,a2) { illegal_1(); } JP(a2)
/* DB   FD         */ OP(FD,a3) { illegal_1(); } JP(a3)
/* AND  HY         */ OP(FD,a4) { and_a(HY);                                       } EOP
/* AND  LY         */ OP(FD,a5) { and_a(LY);                                       } EOP
/* AND  (IY+o)     */ OP_M(FD,a6) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { and_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,a7) { illegal_1(); } JP(a7)

/* DB   FD         */ OP(FD,a8) { illegal_1(); } JP(a8)
/* DB   FD         */ OP(FD,a9) { illegal_1(); } JP(a9)
/* DB   FD         */ OP(FD,aa) { illegal_1(); } JP(aa)
/* DB   FD         */ OP(FD,ab) { illegal_1(); } JP(ab)
/* XOR  HY         */ OP(FD,ac) { xor_a(HY);                                       } EOP
/* XOR  LY         */ OP(FD,ad) { xor_a(LY);                                       } EOP
/* XOR  (IY+o)     */ OP_M(FD,ae) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { xor_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,af) { illegal_1(); } JP(af)

/* DB   FD         */ OP(FD,b0) { illegal_1(); } JP(b0)
/* DB   FD         */ OP(FD,b1) { illegal_1(); } JP(b1)
/* DB   FD         */ OP(FD,b2) { illegal_1(); } JP(b2)
/* DB   FD         */ OP(FD,b3) { illegal_1(); } JP(b3)
/* OR   HY         */ OP(FD,b4) { or_a(HY);                                        } EOP
/* OR   LY         */ OP(FD,b5) { or_a(LY);                                        } EOP
/* OR   (IY+o)     */ OP_M(FD,b6) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { or_a(TDAT8); } EOP
/* DB   FD         */ OP(FD,b7) { illegal_1(); } JP(b7)

/* DB   FD         */ OP(FD,b8) { illegal_1(); } JP(b8)
/* DB   FD         */ OP(FD,b9) { illegal_1(); } JP(b9)
/* DB   FD         */ OP(FD,ba) { illegal_1(); } JP(ba)
/* DB   FD         */ OP(FD,bb) { illegal_1(); } JP(bb)
/* CP   HY         */ OP(FD,bc) { cp(HY);                                          } EOP
/* CP   LY         */ OP(FD,bd) { cp(LY);                                          } EOP
/* CP   (IY+o)     */ OP_M(FD,be) eay() FN { TADR = PCD-1; } MN nomreq_addr(5) FN { TADR = m_ea; } MN rm() FN { cp(TDAT8); } EOP
/* DB   FD         */ OP(FD,bf) { illegal_1(); } JP(bf)

/* DB   FD         */ OP(FD,c0) { illegal_1(); } JP(c0)
/* DB   FD         */ OP(FD,c1) { illegal_1(); } JP(c1)
/* DB   FD         */ OP(FD,c2) { illegal_1(); } JP(c2)
/* DB   FD         */ OP(FD,c3) { illegal_1(); } JP(c3)
/* DB   FD         */ OP(FD,c4) { illegal_1(); } JP(c4)
/* DB   FD         */ OP(FD,c5) { illegal_1(); } JP(c5)
/* DB   FD         */ OP(FD,c6) { illegal_1(); } JP(c6)
/* DB   FD         */ OP(FD,c7) { illegal_1(); } JP(c7)

/* DB   FD         */ OP(FD,c8) { illegal_1(); } JP(c8)
/* DB   FD         */ OP(FD,c9) { illegal_1(); } JP(c9)
/* DB   FD         */ OP(FD,ca) { illegal_1(); } JP(ca)
/* **   FD CB xx   */ OP_M(FD,cb) eay() MN arg() FN { TADR=PCD-1; } MN nomreq_addr(2) JP_P(XY_CB)
/* DB   FD         */ OP(FD,cc) { illegal_1(); } JP(cc)
/* DB   FD         */ OP(FD,cd) { illegal_1(); } JP(cd)
/* DB   FD         */ OP(FD,ce) { illegal_1(); } JP(ce)
/* DB   FD         */ OP(FD,cf) { illegal_1(); } JP(cf)

/* DB   FD         */ OP(FD,d0) { illegal_1(); } JP(d0)
/* DB   FD         */ OP(FD,d1) { illegal_1(); } JP(d1)
/* DB   FD         */ OP(FD,d2) { illegal_1(); } JP(d2)
/* DB   FD         */ OP(FD,d3) { illegal_1(); } JP(d3)
/* DB   FD         */ OP(FD,d4) { illegal_1(); } JP(d4)
/* DB   FD         */ OP(FD,d5) { illegal_1(); } JP(d5)
/* DB   FD         */ OP(FD,d6) { illegal_1(); } JP(d6)
/* DB   FD         */ OP(FD,d7) { illegal_1(); } JP(d7)

/* DB   FD         */ OP(FD,d8) { illegal_1(); } JP(d8)
/* DB   FD         */ OP(FD,d9) { illegal_1(); } JP(d9)
/* DB   FD         */ OP(FD,da) { illegal_1(); } JP(da)
/* DB   FD         */ OP(FD,db) { illegal_1(); } JP(db)
/* DB   FD         */ OP(FD,dc) { illegal_1(); } JP(dc)
/* DB   FD         */ OP(FD,dd) { illegal_1(); } JP(dd)
/* DB   FD         */ OP(FD,de) { illegal_1(); } JP(de)
/* DB   FD         */ OP(FD,df) { illegal_1(); } JP(df)

/* DB   FD         */ OP(FD,e0) { illegal_1(); } JP(e0)
/* POP  IY         */ OP_M(FD,e1) pop() FN { IY=TDAT;                              } EOP
/* DB   FD         */ OP(FD,e2) { illegal_1(); } JP(e2)
/* EX   (SP),IY    */ OP(FD,e3) { TDAT=IY; } MN ex_sp() FN { IY=TDAT; }              EOP
/* DB   FD         */ OP(FD,e4) { illegal_1(); } JP(e4)
/* PUSH IY         */ OP(FD,e5) { TDAT=IY; } MN push()                               EOP
/* DB   FD         */ OP(FD,e6) { illegal_1(); } JP(e6)
/* DB   FD         */ OP(FD,e7) { illegal_1(); } JP(e7)

/* DB   FD         */ OP(FD,e8) { illegal_1(); } JP(e8)
/* JP   (IY)       */ OP(FD,e9) { PC = IY;                                         } EOP
/* DB   FD         */ OP(FD,ea) { illegal_1(); } JP(ea)
/* DB   FD         */ OP(FD,eb) { illegal_1(); } JP(eb)
/* DB   FD         */ OP(FD,ec) { illegal_1(); } JP(ec)
/* DB   FD         */ OP(FD,ed) { illegal_1(); } JP(ed)
/* DB   FD         */ OP(FD,ee) { illegal_1(); } JP(ee)
/* DB   FD         */ OP(FD,ef) { illegal_1(); } JP(ef)

/* DB   FD         */ OP(FD,f0) { illegal_1(); } JP(f0)
/* DB   FD         */ OP(FD,f1) { illegal_1(); } JP(f1)
/* DB   FD         */ OP(FD,f2) { illegal_1(); } JP(f2)
/* DB   FD         */ OP(FD,f3) { illegal_1(); } JP(f3)
/* DB   FD         */ OP(FD,f4) { illegal_1(); } JP(f4)
/* DB   FD         */ OP(FD,f5) { illegal_1(); } JP(f5)
/* DB   FD         */ OP(FD,f6) { illegal_1(); } JP(f6)
/* DB   FD         */ OP(FD,f7) { illegal_1(); } JP(f7)

/* DB   FD         */ OP(FD,f8) { illegal_1(); } JP(f8)
/* LD   SP,IY      */ OP_M(FD,f9) nomreq_ir(2) FN { SP = IY;                       } EOP
/* DB   FD         */ OP(FD,fa) { illegal_1(); } JP(fa)
/* DB   FD         */ OP(FD,fb) { illegal_1(); } JP(fb)
/* DB   FD         */ OP(FD,fc) { illegal_1(); } JP(fc)
/* DB   FD         */ OP(FD,fd) { illegal_1(); } JP(fd)
/* DB   FD         */ OP(FD,fe) { illegal_1(); } JP(fe)
/* DB   FD         */ OP(FD,ff) { illegal_1(); } JP(ff)

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
/* DB   ED         */ OP(ED,00) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,01) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,02) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,03) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,04) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,05) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,06) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,07) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,08) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,09) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,0f) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,10) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,11) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,12) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,13) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,14) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,15) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,16) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,17) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,18) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,19) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,1f) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,20) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,21) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,22) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,23) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,24) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,25) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,26) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,27) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,28) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,29) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,2f) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,30) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,31) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,32) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,33) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,34) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,35) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,36) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,37) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,38) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,39) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,3f) { illegal_2();                                     } EOP

/* IN   B,(C)      */ OP(ED,40) { TADR=BC; } MN in() FN { B = TDAT8; F = (F & CF) | SZP[B]; WZ = TADR + 1; } EOP
/* OUT  (C),B      */ OP(ED,41) { TADR=BC; TDAT8=B; } MN out() FN { WZ = TADR + 1; } EOP
/* SBC  HL,BC      */ OP(ED,42) { TDAT=BC; } MN sbc_hl()                             EOP
/* LD   (w),BC     */ OP_M(ED,43) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=BC; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,44) { neg();                                           } EOP
/* RETN            */ OP_M(ED,45) retn()                                             EOP
/* IM   0          */ OP(ED,46) { m_im = 0;                                        } EOP
/* LD   i,A        */ OP_M(ED,47) ld_i_a()                                           EOP

/* IN   C,(C)      */ OP(ED,48) { TADR=BC; } MN in() FN { C = TDAT8; F = (F & CF) | SZP[C]; WZ = TADR + 1;} EOP
/* OUT  (C),C      */ OP(ED,49) { TADR=BC; TDAT8=C; } MN out() FN { WZ = TADR + 1; } EOP
/* ADC  HL,BC      */ OP(ED,4a) { TDAT=BC; } MN adc_hl()                             EOP
/* LD   BC,(w)     */ OP_M(ED,4b) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { BC=TDAT; WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,4c) { neg();                                           } EOP
/* RETI            */ OP_M(ED,4d) reti()                                             EOP
/* IM   0          */ OP(ED,4e) { m_im = 0;                                        } EOP
/* LD   r,A        */ OP_M(ED,4f) ld_r_a()                                           EOP

/* IN   D,(C)      */ OP(ED,50) { TADR=BC; } MN in() FN { D = TDAT8; F = (F & CF) | SZP[D]; WZ = TADR + 1; } EOP
/* OUT  (C),D      */ OP(ED,51) { TADR=BC; TDAT8=D; } MN out() FN { WZ = TADR + 1; } EOP
/* SBC  HL,DE      */ OP(ED,52) { TDAT=DE; } MN sbc_hl()                             EOP
/* LD   (w),DE     */ OP_M(ED,53) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=DE; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,54) { neg();                                           } EOP
/* RETN            */ OP_M(ED,55) retn()                                             EOP
/* IM   1          */ OP(ED,56) { m_im = 1;                                        } EOP
/* LD   A,i        */ OP_M(ED,57) ld_a_i()                                           EOP

/* IN   E,(C)      */ OP(ED,58) { TADR=BC; } MN in() FN { E = TDAT8; F = (F & CF) | SZP[E]; WZ = TADR + 1; } EOP
/* OUT  (C),E      */ OP(ED,59) { TADR=BC; TDAT8=E; } MN out() FN { WZ = TADR + 1; } EOP
/* ADC  HL,DE      */ OP(ED,5a) { TDAT=DE; } MN adc_hl()                             EOP
/* LD   DE,(w)     */ OP_M(ED,5b) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { DE=TDAT; WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,5c) { neg();                                           } EOP
/* RETI            */ OP_M(ED,5d) reti()                                             EOP
/* IM   2          */ OP(ED,5e) { m_im = 2;                                        } EOP
/* LD   A,r        */ OP_M(ED,5f) ld_a_r()                                           EOP

/* IN   H,(C)      */ OP(ED,60) { TADR=BC; } MN in() FN { H = TDAT8; F = (F & CF) | SZP[H]; WZ = TADR + 1; } EOP
/* OUT  (C),H      */ OP(ED,61) { TADR=BC; TDAT8=H; } MN out() FN { WZ = TADR + 1; } EOP
/* SBC  HL,HL      */ OP(ED,62) { TDAT=HL; } MN sbc_hl()                             EOP
/* LD   (w),HL     */ OP_M(ED,63) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=HL; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,64) { neg();                                           } EOP
/* RETN            */ OP_M(ED,65) retn()                                             EOP
/* IM   0          */ OP(ED,66) { m_im = 0;                                        } EOP
/* RRD  (HL)       */ OP_M(ED,67) rrd()                                              EOP

/* IN   L,(C)      */ OP(ED,68) { TADR=BC; } MN in() FN { L = TDAT8; F = (F & CF) | SZP[L]; WZ = TADR + 1; } EOP
/* OUT  (C),L      */ OP(ED,69) { TADR=BC; TDAT8=L; } MN out() FN { WZ = TADR + 1; } EOP
/* ADC  HL,HL      */ OP(ED,6a) { TDAT=HL; } MN adc_hl()                             EOP
/* LD   HL,(w)     */ OP_M(ED,6b) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { HL=TDAT; WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,6c) { neg();                                           } EOP
/* RETI            */ OP_M(ED,6d) reti()                                             EOP
/* IM   0          */ OP(ED,6e) { m_im = 0;                                        } EOP
/* RLD  (HL)       */ OP_M(ED,6f) rld()                                              EOP

/* IN   0,(C)      */ OP(ED,70) { TADR=BC; } MN in() FN { F = (F & CF) | SZP[TDAT8]; WZ = TADR + 1; } EOP
/* OUT  (C),0      */ OP(ED,71) { TADR=BC; TDAT8=0; } MN out() FN { WZ = TADR + 1; } EOP
/* SBC  HL,SP      */ OP(ED,72) { TDAT=SP; } MN sbc_hl()                             EOP
/* LD   (w),SP     */ OP_M(ED,73) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT=SP; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,74) { neg();                                           } EOP
/* RETN            */ OP_M(ED,75) retn()                                             EOP
/* IM   1          */ OP(ED,76) { m_im = 1;                                        } EOP
/* DB   ED,77      */ OP(ED,77) { illegal_2();                                     } EOP

/* IN   A,(C)      */ OP(ED,78) { TADR=BC; } MN in() FN { A = TDAT8; F = (F & CF) | SZP[A]; WZ = TADR + 1; } EOP
/* OUT  (C),A      */ OP(ED,79) { TADR=BC; TDAT8=A; } MN out() FN { WZ = TADR + 1; } EOP
/* ADC  HL,SP      */ OP(ED,7a) { TDAT=SP; } MN adc_hl()                             EOP
/* LD   SP,(w)     */ OP_M(ED,7b) arg16() FN { m_ea=TDAT; TADR=m_ea; } MN rm16() FN { SP=TDAT; WZ = m_ea + 1; } EOP
/* NEG             */ OP(ED,7c) { neg();                                           } EOP
/* RETI            */ OP_M(ED,7d) reti()                                             EOP
/* IM   2          */ OP(ED,7e) { m_im = 2;                                        } EOP
/* DB   ED,7F      */ OP(ED,7f) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,80) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,81) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,82) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,83) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,84) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,85) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,86) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,87) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,88) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,89) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,8f) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,90) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,91) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,92) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,93) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,94) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,95) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,96) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,97) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,98) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,99) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9a) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9b) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9c) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9d) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9e) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,9f) { illegal_2();                                     } EOP

/* LDI             */ OP_M(ED,a0) ldi()                                              EOP
/* CPI             */ OP_M(ED,a1) cpi()                                              EOP
/* INI             */ OP_M(ED,a2) ini()                                              EOP
/* OUTI            */ OP_M(ED,a3) outi()                                             EOP
/* DB   ED         */ OP(ED,a4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,a5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,a6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,a7) { illegal_2();                                     } EOP

/* LDD             */ OP_M(ED,a8) ldd()                                              EOP
/* CPD             */ OP_M(ED,a9) cpd()                                              EOP
/* IND             */ OP_M(ED,aa) ind()                                              EOP
/* OUTD            */ OP_M(ED,ab) outd()                                             EOP
/* DB   ED         */ OP(ED,ac) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ad) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ae) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,af) { illegal_2();                                     } EOP

/* LDIR            */ OP_M(ED,b0) ldir()                                             EOP
/* CPIR            */ OP_M(ED,b1) cpir()                                             EOP
/* INIR            */ OP_M(ED,b2) inir()                                             EOP
/* OTIR            */ OP_M(ED,b3) otir()                                             EOP
/* DB   ED         */ OP(ED,b4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,b5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,b6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,b7) { illegal_2();                                     } EOP

/* LDDR            */ OP_M(ED,b8) lddr()                                             EOP
/* CPDR            */ OP_M(ED,b9) cpdr()                                             EOP
/* INDR            */ OP_M(ED,ba) indr()                                             EOP
/* OTDR            */ OP_M(ED,bb) otdr()                                             EOP
/* DB   ED         */ OP(ED,bc) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,bd) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,be) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,bf) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,c0) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c1) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c2) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c3) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c7) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,c8) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,c9) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ca) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,cb) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,cc) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,cd) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ce) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,cf) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,d0) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d1) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d2) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d3) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d7) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,d8) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,d9) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,da) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,db) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,dc) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,dd) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,de) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,df) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,e0) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e1) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e2) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e3) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e7) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,e8) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,e9) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ea) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,eb) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ec) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ed) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ee) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ef) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,f0) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f1) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f2) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f3) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f4) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f5) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f6) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f7) { illegal_2();                                     } EOP

/* DB   ED         */ OP(ED,f8) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,f9) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,fa) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,fb) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,fc) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,fd) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,fe) { illegal_2();                                     } EOP
/* DB   ED         */ OP(ED,ff) { illegal_2();                                     } EOP

/**********************************************************
 * main opcodes
 **********************************************************/
/* NOP             */ OP_M(NONE,00)                                                                         EOP
/* LD   BC,w       */ OP_M(NONE,01) arg16() FN { BC = TDAT;                                               } EOP
/* LD (BC),A       */ OP(NONE,02) { TADR=BC; TDAT8=A; } MN wm() FN { WZ_L = (BC + 1) & 0xFF;  WZ_H = A;   } EOP
/* INC  BC         */ OP_M(NONE,03) nomreq_ir(2) FN { BC++;                                               } EOP
/* INC  B          */ OP(NONE,04) { inc(B);                                                               } EOP
/* DEC  B          */ OP(NONE,05) { dec(B);                                                               } EOP
/* LD   B,n        */ OP_M(NONE,06) arg() FN { B = TDAT8;                                                 } EOP
/* RLCA            */ OP(NONE,07) { rlca();                                                               } EOP

/* EX   AF,AF'     */ OP(NONE,08) { std::swap(m_af, m_af2);                                               } EOP
/* ADD  HL,BC      */ OP(NONE,09) { TDAT=HL; TDAT2=BC; } MN add16() FN { HL=TDAT;                         } EOP
/* LD   A,(BC)     */ OP(NONE,0a) { TADR=BC; } MN rm() FN { A=TDAT8;  WZ=BC+1;                            } EOP
/* DEC  BC         */ OP_M(NONE,0b) nomreq_ir(2) FN { BC--;                                               } EOP
/* INC  C          */ OP(NONE,0c) { inc(C);                                                               } EOP
/* DEC  C          */ OP(NONE,0d) { dec(C);                                                               } EOP
/* LD   C,n        */ OP_M(NONE,0e) arg() FN { C = TDAT8;                                                 } EOP
/* RRCA            */ OP(NONE,0f) { rrca();                                                               } EOP

/* DJNZ o          */ OP_M(NONE,10) nomreq_ir(1) FN { TDAT8=--B; } MN jr_cond(0x10)                         EOP
/* LD   DE,w       */ OP_M(NONE,11) arg16() FN { DE = TDAT;                                               } EOP
/* LD (DE),A       */ OP(NONE,12) { TADR=DE; TDAT8=A; } MN wm() FN { WZ_L = (DE + 1) & 0xFF;  WZ_H = A;   } EOP
/* INC  DE         */ OP_M(NONE,13) nomreq_ir(2) FN { DE++;                                               } EOP
/* INC  D          */ OP(NONE,14) { inc(D);                                                               } EOP
/* DEC  D          */ OP(NONE,15) { dec(D);                                                               } EOP
/* LD   D,n        */ OP_M(NONE,16) arg() FN { D=TDAT8;                                                   } EOP
/* RLA             */ OP(NONE,17) { rla();                                                                } EOP

/* JR   o          */ OP_M(NONE,18) jr()                                                                    EOP
/* ADD  HL,DE      */ OP(NONE,19) { TDAT=HL; TDAT2=DE; } MN add16() FN { HL=TDAT;                         } EOP
/* LD   A,(DE)     */ OP(NONE,1a) { TADR=DE; } MN rm() FN { A=TDAT8; WZ = DE + 1;                         } EOP
/* DEC  DE         */ OP_M(NONE,1b) nomreq_ir(2) FN { DE--;                                               } EOP
/* INC  E          */ OP(NONE,1c) { inc(E);                                                               } EOP
/* DEC  E          */ OP(NONE,1d) { dec(E);                                                               } EOP
/* LD   E,n        */ OP_M(NONE,1e) arg() FN { E=TDAT8;                                                   } EOP
/* RRA             */ OP(NONE,1f) { rra();                                                                } EOP

/* JR   NZ,o       */ OP(NONE,20) { TDAT8=!(F & ZF); } MN jr_cond(0x20)                                     EOP
/* LD   HL,w       */ OP_M(NONE,21) arg16() FN { HL = TDAT;                                               } EOP
/* LD   (w),HL     */ OP_M(NONE,22) arg16() FN { m_ea=TDAT; TADR=TDAT; TDAT=HL; } MN wm16() FN { WZ = m_ea + 1; } EOP
/* INC  HL         */ OP_M(NONE,23) nomreq_ir(2) FN { HL++;                                               } EOP
/* INC  H          */ OP(NONE,24) { inc(H);                                                               } EOP
/* DEC  H          */ OP(NONE,25) { dec(H);                                                               } EOP
/* LD   H,n        */ OP_M(NONE,26) arg() FN { H=TDAT8;                                                   } EOP
/* DAA             */ OP(NONE,27) { daa();                                                                } EOP

/* JR   Z,o        */ OP(NONE,28) { TDAT8=F & ZF; } MN jr_cond(0x28)                                        EOP
/* ADD  HL,HL      */ OP(NONE,29) { TDAT=HL; TDAT2=HL; } MN add16() FN { HL=TDAT;                         } EOP
/* LD   HL,(w)     */ OP_M(NONE,2a) arg16() FN { m_ea=TDAT; TADR=TDAT; } MN rm16() FN { HL=TDAT; WZ = m_ea + 1; } EOP
/* DEC  HL         */ OP_M(NONE,2b) nomreq_ir(2) FN { HL--;                                               } EOP
/* INC  L          */ OP(NONE,2c) { inc(L);                                                               } EOP
/* DEC  L          */ OP(NONE,2d) { dec(L);                                                               } EOP
/* LD   L,n        */ OP_M(NONE,2e) arg() FN { L=TDAT8;                                                   } EOP
/* CPL             */ OP(NONE,2f) { A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); } EOP

/* JR   NC,o       */ OP(NONE,30) { TDAT8=!(F & CF); } MN jr_cond(0x30)                                     EOP
/* LD   SP,w       */ OP_M(NONE,31) arg16() FN { SP = TDAT;                                               } EOP
/* LD   (w),A      */ OP_M(NONE,32) arg16() FN { m_ea=TDAT; TADR=m_ea; TDAT8=A; } MN wm() FN { WZ_L = (m_ea + 1) & 0xFF; WZ_H = A; } EOP
/* INC  SP         */ OP_M(NONE,33) nomreq_ir(2) FN { SP++;                                               } EOP
/* INC  (HL)       */ OP(NONE,34) { TADR=HL; } MN rm_reg() FN { inc(TDAT8); } MN wm()                       EOP
/* DEC  (HL)       */ OP(NONE,35) { TADR=HL; } MN rm_reg() FN { dec(TDAT8); } MN wm()                       EOP
/* LD   (HL),n     */ OP_M(NONE,36) arg() FN { TADR=HL; } MN wm()                                           EOP
/* SCF             */ OP(NONE,37) { F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF));            } EOP

/* JR   C,o        */ OP(NONE,38) { TDAT8=F & CF; } MN jr_cond(0x38)                                        EOP
/* ADD  HL,SP      */ OP(NONE,39) { TDAT=HL; TDAT2=SP; } MN add16() FN { HL=TDAT;                         } EOP
/* LD   A,(w)      */ OP_M(NONE,3a) arg16() FN { m_ea=TDAT; TADR=TDAT; } MN rm() FN { A=TDAT8; WZ = m_ea + 1; } EOP
/* DEC  SP         */ OP_M(NONE,3b) nomreq_ir(2) FN { SP--;                                               } EOP
/* INC  A          */ OP(NONE,3c) { inc(A);                                                               } EOP
/* DEC  A          */ OP(NONE,3d) { dec(A);                                                               } EOP
/* LD   A,n        */ OP_M(NONE,3e) arg() FN { A = TDAT8;                                                 } EOP
/* CCF             */ OP(NONE,3f) { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;             } EOP

/* LD   B,B        */ OP_M(NONE,40)                                                                         EOP
/* LD   B,C        */ OP(NONE,41) { B = C;                                                                } EOP
/* LD   B,D        */ OP(NONE,42) { B = D;                                                                } EOP
/* LD   B,E        */ OP(NONE,43) { B = E;                                                                } EOP
/* LD   B,H        */ OP(NONE,44) { B = H;                                                                } EOP
/* LD   B,L        */ OP(NONE,45) { B = L;                                                                } EOP
/* LD   B,(HL)     */ OP(NONE,46) { TADR=HL; } MN rm() FN { B = TDAT8;                                    } EOP
/* LD   B,A        */ OP(NONE,47) { B = A;                                                                } EOP

/* LD   C,B        */ OP(NONE,48) { C = B;                                                                } EOP
/* LD   C,C        */ OP_M(NONE,49)                                                                         EOP
/* LD   C,D        */ OP(NONE,4a) { C = D;                                                                } EOP
/* LD   C,E        */ OP(NONE,4b) { C = E;                                                                } EOP
/* LD   C,H        */ OP(NONE,4c) { C = H;                                                                } EOP
/* LD   C,L        */ OP(NONE,4d) { C = L;                                                                } EOP
/* LD   C,(HL)     */ OP(NONE,4e) { TADR=HL; } MN rm() FN { C = TDAT8;                                    } EOP
/* LD   C,A        */ OP(NONE,4f) { C = A;                                                                } EOP

/* LD   D,B        */ OP(NONE,50) { D = B;                                                                } EOP
/* LD   D,C        */ OP(NONE,51) { D = C;                                                                } EOP
/* LD   D,D        */ OP_M(NONE,52)                                                                         EOP
/* LD   D,E        */ OP(NONE,53) { D = E;                                                                } EOP
/* LD   D,H        */ OP(NONE,54) { D = H;                                                                } EOP
/* LD   D,L        */ OP(NONE,55) { D = L;                                                                } EOP
/* LD   D,(HL)     */ OP(NONE,56) { TADR=HL; } MN rm() FN { D = TDAT8;                                    } EOP
/* LD   D,A        */ OP(NONE,57) { D = A;                                                                } EOP

/* LD   E,B        */ OP(NONE,58) { E = B;                                                                } EOP
/* LD   E,C        */ OP(NONE,59) { E = C;                                                                } EOP
/* LD   E,D        */ OP(NONE,5a) { E = D;                                                                } EOP
/* LD   E,E        */ OP_M(NONE,5b)                                                                         EOP
/* LD   E,H        */ OP(NONE,5c) { E = H;                                                                } EOP
/* LD   E,L        */ OP(NONE,5d) { E = L;                                                                } EOP
/* LD   E,(HL)     */ OP(NONE,5e) { TADR=HL; } MN rm() FN { E = TDAT8;                                    } EOP
/* LD   E,A        */ OP(NONE,5f) { E = A;                                                                } EOP

/* LD   H,B        */ OP(NONE,60) { H = B;                                                                } EOP
/* LD   H,C        */ OP(NONE,61) { H = C;                                                                } EOP
/* LD   H,D        */ OP(NONE,62) { H = D;                                                                } EOP
/* LD   H,E        */ OP(NONE,63) { H = E;                                                                } EOP
/* LD   H,H        */ OP_M(NONE,64)                                                                         EOP
/* LD   H,L        */ OP(NONE,65) { H = L;                                                                } EOP
/* LD   H,(HL)     */ OP(NONE,66) { TADR=HL; } MN rm() FN { H = TDAT8;                                    } EOP
/* LD   H,A        */ OP(NONE,67) { H = A;                                                                } EOP

/* LD   L,B        */ OP(NONE,68) { L = B;                                                                } EOP
/* LD   L,C        */ OP(NONE,69) { L = C;                                                                } EOP
/* LD   L,D        */ OP(NONE,6a) { L = D;                                                                } EOP
/* LD   L,E        */ OP(NONE,6b) { L = E;                                                                } EOP
/* LD   L,H        */ OP(NONE,6c) { L = H;                                                                } EOP
/* LD   L,L        */ OP_M(NONE,6d)                                                                         EOP
/* LD   L,(HL)     */ OP(NONE,6e) { TADR=HL; } MN rm() FN { L = TDAT8;                                    } EOP
/* LD   L,A        */ OP(NONE,6f) { L = A;                                                                } EOP

/* LD   (HL),B     */ OP(NONE,70) { TADR=HL; TDAT=B; } MN wm()                                              EOP
/* LD   (HL),C     */ OP(NONE,71) { TADR=HL; TDAT=C; } MN wm()                                              EOP
/* LD   (HL),D     */ OP(NONE,72) { TADR=HL; TDAT=D; } MN wm()                                              EOP
/* LD   (HL),E     */ OP(NONE,73) { TADR=HL; TDAT=E; } MN wm()                                              EOP
/* LD   (HL),H     */ OP(NONE,74) { TADR=HL; TDAT=H; } MN wm()                                              EOP
/* LD   (HL),L     */ OP(NONE,75) { TADR=HL; TDAT=L; } MN wm()                                              EOP
/* HALT            */ OP(NONE,76) { halt();                                                               } EOP
/* LD   (HL),A     */ OP(NONE,77) { TADR=HL; TDAT=A; } MN wm()                                              EOP

/* LD   A,B        */ OP(NONE,78) { A = B;                                                                } EOP
/* LD   A,C        */ OP(NONE,79) { A = C;                                                                } EOP
/* LD   A,D        */ OP(NONE,7a) { A = D;                                                                } EOP
/* LD   A,E        */ OP(NONE,7b) { A = E;                                                                } EOP
/* LD   A,H        */ OP(NONE,7c) { A = H;                                                                } EOP
/* LD   A,L        */ OP(NONE,7d) { A = L;                                                                } EOP
/* LD   A,(HL)     */ OP(NONE,7e) { TADR=HL; } MN rm() FN { A = TDAT8;                                    } EOP
/* LD   A,A        */ OP_M(NONE,7f)                                                                         EOP

/* ADD  A,B        */ OP(NONE,80) { add_a(B);                                                             } EOP
/* ADD  A,C        */ OP(NONE,81) { add_a(C);                                                             } EOP
/* ADD  A,D        */ OP(NONE,82) { add_a(D);                                                             } EOP
/* ADD  A,E        */ OP(NONE,83) { add_a(E);                                                             } EOP
/* ADD  A,H        */ OP(NONE,84) { add_a(H);                                                             } EOP
/* ADD  A,L        */ OP(NONE,85) { add_a(L);                                                             } EOP
/* ADD  A,(HL)     */ OP(NONE,86) { TADR=HL; } MN rm() FN { add_a(TDAT8);                                 } EOP
/* ADD  A,A        */ OP(NONE,87) { add_a(A);                                                             } EOP

/* ADC  A,B        */ OP(NONE,88) { adc_a(B);                                                             } EOP
/* ADC  A,C        */ OP(NONE,89) { adc_a(C);                                                             } EOP
/* ADC  A,D        */ OP(NONE,8a) { adc_a(D);                                                             } EOP
/* ADC  A,E        */ OP(NONE,8b) { adc_a(E);                                                             } EOP
/* ADC  A,H        */ OP(NONE,8c) { adc_a(H);                                                             } EOP
/* ADC  A,L        */ OP(NONE,8d) { adc_a(L);                                                             } EOP
/* ADC  A,(HL)     */ OP(NONE,8e) { TADR=HL; } MN rm() FN { adc_a(TDAT8);                                 } EOP
/* ADC  A,A        */ OP(NONE,8f) { adc_a(A);                                                             } EOP

/* SUB  B          */ OP(NONE,90) { sub(B);                                                               } EOP
/* SUB  C          */ OP(NONE,91) { sub(C);                                                               } EOP
/* SUB  D          */ OP(NONE,92) { sub(D);                                                               } EOP
/* SUB  E          */ OP(NONE,93) { sub(E);                                                               } EOP
/* SUB  H          */ OP(NONE,94) { sub(H);                                                               } EOP
/* SUB  L          */ OP(NONE,95) { sub(L);                                                               } EOP
/* SUB  (HL)       */ OP(NONE,96) { TADR=HL; } MN rm() FN { sub(TDAT8);                                   } EOP
/* SUB  A          */ OP(NONE,97) { sub(A);                                                               } EOP

/* SBC  A,B        */ OP(NONE,98) { sbc_a(B);                                                             } EOP
/* SBC  A,C        */ OP(NONE,99) { sbc_a(C);                                                             } EOP
/* SBC  A,D        */ OP(NONE,9a) { sbc_a(D);                                                             } EOP
/* SBC  A,E        */ OP(NONE,9b) { sbc_a(E);                                                             } EOP
/* SBC  A,H        */ OP(NONE,9c) { sbc_a(H);                                                             } EOP
/* SBC  A,L        */ OP(NONE,9d) { sbc_a(L);                                                             } EOP
/* SBC  A,(HL)     */ OP(NONE,9e) { TADR=HL; } MN rm() FN { sbc_a(TDAT8);                                 } EOP
/* SBC  A,A        */ OP(NONE,9f) { sbc_a(A);                                                             } EOP

/* AND  B          */ OP(NONE,a0) { and_a(B);                                                             } EOP
/* AND  C          */ OP(NONE,a1) { and_a(C);                                                             } EOP
/* AND  D          */ OP(NONE,a2) { and_a(D);                                                             } EOP
/* AND  E          */ OP(NONE,a3) { and_a(E);                                                             } EOP
/* AND  H          */ OP(NONE,a4) { and_a(H);                                                             } EOP
/* AND  L          */ OP(NONE,a5) { and_a(L);                                                             } EOP
/* AND  (HL)       */ OP(NONE,a6) { TADR=HL; } MN rm() FN { and_a(TDAT8);                                 } EOP
/* AND  A          */ OP(NONE,a7) { and_a(A);                                                             } EOP

/* XOR  B          */ OP(NONE,a8) { xor_a(B);                                                             } EOP
/* XOR  C          */ OP(NONE,a9) { xor_a(C);                                                             } EOP
/* XOR  D          */ OP(NONE,aa) { xor_a(D);                                                             } EOP
/* XOR  E          */ OP(NONE,ab) { xor_a(E);                                                             } EOP
/* XOR  H          */ OP(NONE,ac) { xor_a(H);                                                             } EOP
/* XOR  L          */ OP(NONE,ad) { xor_a(L);                                                             } EOP
/* XOR  (HL)       */ OP(NONE,ae) { TADR=HL; } MN rm() FN { xor_a(TDAT8);                                 } EOP
/* XOR  A          */ OP(NONE,af) { xor_a(A);                                                             } EOP

/* OR   B          */ OP(NONE,b0) { or_a(B);                                                              } EOP
/* OR   C          */ OP(NONE,b1) { or_a(C);                                                              } EOP
/* OR   D          */ OP(NONE,b2) { or_a(D);                                                              } EOP
/* OR   E          */ OP(NONE,b3) { or_a(E);                                                              } EOP
/* OR   H          */ OP(NONE,b4) { or_a(H);                                                              } EOP
/* OR   L          */ OP(NONE,b5) { or_a(L);                                                              } EOP
/* OR   (HL)       */ OP(NONE,b6) { TADR=HL; } MN rm() FN { or_a(TDAT8);                                  } EOP
/* OR   A          */ OP(NONE,b7) { or_a(A);                                                              } EOP

/* CP   B          */ OP(NONE,b8) { cp(B);                                                                } EOP
/* CP   C          */ OP(NONE,b9) { cp(C);                                                                } EOP
/* CP   D          */ OP(NONE,ba) { cp(D);                                                                } EOP
/* CP   E          */ OP(NONE,bb) { cp(E);                                                                } EOP
/* CP   H          */ OP(NONE,bc) { cp(H);                                                                } EOP
/* CP   L          */ OP(NONE,bd) { cp(L);                                                                } EOP
/* CP   (HL)       */ OP(NONE,be) { TADR=HL; } MN rm() FN { cp(TDAT8);                                    } EOP
/* CP   A          */ OP(NONE,bf) { cp(A);                                                                } EOP

/* RET  NZ         */ OP(NONE,c0) { TDAT8=!(F & ZF); } MN ret_cond(0xc0)                                    EOP
/* POP  BC         */ OP_M(NONE,c1) pop() FN { BC=TDAT;                                                   } EOP
/* JP   NZ,a       */ OP(NONE,c2) { TDAT8=!(F & ZF); } MN jp_cond()                                         EOP
/* JP   a          */ OP_M(NONE,c3) jp()                                                                    EOP
/* CALL NZ,a       */ OP(NONE,c4) { TDAT8=!(F & ZF); } MN call_cond(0xc4)                                   EOP
/* PUSH BC         */ OP(NONE,c5) { TDAT=BC; } MN push()                                                    EOP
/* ADD  A,n        */ OP_M(NONE,c6) arg() FN { add_a(TDAT8);                                              } EOP
/* RST  0          */ OP_M(NONE,c7) rst(0x00)                                                               EOP

/* RET  Z          */ OP(NONE,c8) { TDAT8=(F & ZF); } MN ret_cond(0xc8)                                     EOP
/* RET             */ OP_M(NONE,c9) pop() FN { PC=TDAT; WZ = PCD;                                         } EOP
/* JP   Z,a        */ OP(NONE,ca) { TDAT8=F & ZF; } MN jp_cond()                                            EOP
/* **** CB xx      */ OP_M(NONE,cb) rop() JP_P(CB)
/* CALL Z,a        */ OP(NONE,cc) { TDAT8=F & ZF; } MN call_cond(0xcc)                                      EOP
/* CALL a          */ OP_M(NONE,cd) call()                                                                  EOP
/* ADC  A,n        */ OP_M(NONE,ce) arg() FN { adc_a(TDAT8);                                              } EOP
/* RST  1          */ OP_M(NONE,cf) rst(0x08)                                                               EOP

/* RET  NC         */ OP(NONE,d0) { TDAT8=!(F & CF); } MN ret_cond(0xd0)                                    EOP
/* POP  DE         */ OP_M(NONE,d1) pop() FN { DE=TDAT;                                                   } EOP
/* JP   NC,a       */ OP(NONE,d2) { TDAT8=!(F & CF); } MN jp_cond()                                         EOP
/* OUT  (n),A      */ OP_M(NONE,d3) arg() FN { TADR = TDAT8 | (A << 8); TDAT=A; } MN out() FN { WZ_L = ((TADR & 0xff) + 1) & 0xff;  WZ_H = A; } EOP
/* CALL NC,a       */ OP(NONE,d4) { TDAT8=!(F & CF); } MN call_cond(0xd4)                                   EOP
/* PUSH DE         */ OP(NONE,d5) { TDAT=DE; } MN push()                                                    EOP
/* SUB  n          */ OP_M(NONE,d6) arg() FN { sub(TDAT8);                                                } EOP
/* RST  2          */ OP_M(NONE,d7) rst(0x10)                                                               EOP

/* RET  C          */ OP(NONE,d8) { TDAT8=(F & CF); } MN ret_cond(0xd8)                                     EOP
/* EXX             */ OP(NONE,d9) { exx();                                                                } EOP
/* JP   C,a        */ OP(NONE,da) { TDAT8=F & CF; } MN jp_cond()                                            EOP
/* IN   A,(n)      */ OP_M(NONE,db) arg() FN { TADR = TDAT8 | (A << 8); } MN in() FN { A = TDAT8; WZ = TADR + 1; } EOP
/* CALL C,a        */ OP(NONE,dc) { TDAT8=F & CF; } MN call_cond(0xdc)                                      EOP
/* **** DD xx      */ OP_M(NONE,dd) rop() JP_P(DD)
/* SBC  A,n        */ OP_M(NONE,de) arg() FN { sbc_a(TDAT8);                                              } EOP
/* RST  3          */ OP_M(NONE,df) rst(0x18)                                                               EOP

/* RET  PO         */ OP(NONE,e0) { TDAT8=!(F & PF); } MN ret_cond(0xe0)                                    EOP
/* POP  HL         */ OP_M(NONE,e1) pop() FN { HL=TDAT;                                                   } EOP
/* JP   PO,a       */ OP(NONE,e2) { TDAT8=!(F & PF); } MN jp_cond()                                         EOP
/* EX   HL,(SP)    */ OP(NONE,e3) { TDAT=HL; } MN ex_sp() FN { HL=TDAT; }                                   EOP
/* CALL PO,a       */ OP(NONE,e4) { TDAT8=!(F & PF); } MN call_cond(0xe4)                                   EOP
/* PUSH HL         */ OP(NONE,e5) { TDAT=HL; } MN push()                                                    EOP
/* AND  n          */ OP_M(NONE,e6) arg() FN { and_a(TDAT8);                                              } EOP
/* RST  4          */ OP_M(NONE,e7) rst(0x20)                                                               EOP

/* RET  PE         */ OP(NONE,e8) { TDAT8=(F & PF); } MN ret_cond(0xe8)                                     EOP
/* JP   (HL)       */ OP(NONE,e9) { PC = HL;                                                              } EOP
/* JP   PE,a       */ OP(NONE,ea) { TDAT8=F & PF; } MN jp_cond()                                            EOP
/* EX   DE,HL      */ OP(NONE,eb) { std::swap(DE, HL);                                                    } EOP
/* CALL PE,a       */ OP(NONE,ec) { TDAT8=F & PF; } MN call_cond(0xec)                                      EOP
/* **** ED xx      */ OP_M(NONE,ed) rop() JP_P(ED)
/* XOR  n          */ OP_M(NONE,ee) arg() FN { xor_a(TDAT8);                                              } EOP
/* RST  5          */ OP_M(NONE,ef) rst(0x28)                                                               EOP

/* RET  P          */ OP(NONE,f0) { TDAT8=!(F & SF); } MN ret_cond(0xf0)                                    EOP
/* POP  AF         */ OP_M(NONE,f1) pop() FN { AF=TDAT;                                                   } EOP
/* JP   P,a        */ OP(NONE,f2) { TDAT8=!(F & SF); } MN jp_cond()                                         EOP
/* DI              */ OP(NONE,f3) { m_iff1 = m_iff2 = 0;                                                  } EOP
/* CALL P,a        */ OP(NONE,f4) { TDAT8=!(F & SF); } MN call_cond(0xf4)                                   EOP
/* PUSH AF         */ OP(NONE,f5) { TDAT=AF; } MN push()                                                    EOP
/* OR   n          */ OP_M(NONE,f6) arg() FN { or_a(TDAT8);                                               } EOP
/* RST  6          */ OP_M(NONE,f7) rst(0x30)                                                               EOP

/* RET  M          */ OP(NONE,f8) { TDAT8=(F & SF); } MN ret_cond(0xf8)                                     EOP
/* LD   SP,HL      */ OP_M(NONE,f9) nomreq_ir(2) FN { SP = HL;                                            } EOP
/* JP   M,a        */ OP(NONE,fa) { TDAT8=F & SF; } MN jp_cond()                                            EOP
/* EI              */ OP(NONE,fb) { ei();                                                                 } EOP
/* CALL M,a        */ OP(NONE,fc) { TDAT8=F & SF; } MN call_cond(0xfc)                                      EOP
/* **** FD xx      */ OP_M(NONE,fd) rop() JP_P(FD)
/* CP   n          */ OP_M(NONE,fe) arg() FN { cp(TDAT8);                                                 } EOP
/* RST  7          */ OP_M(NONE,ff) rst(0x38)                                                               EOP

} // end: init_op_steps()

void z80_device::take_nmi()
{
	/* Check if processor was halted */
	leave_halt();

#if HAS_LDAIR_QUIRK
	/* reset parity flag after LD A,I or LD A,R */
	if (m_after_ldair) F &= ~PF;
#endif

	m_iff1 = 0;
	m_r++;

	m_icount_executing = 11;
	T(m_icount_executing - MTM * 2);
	wm16_sp(m_pc);
	PCD = 0x0066;
	WZ=PCD;
	m_nmi_pending = false;
}

void z80_device::take_interrupt()
{
	// check if processor was halted
	leave_halt();

	// clear both interrupt flip flops
	m_iff1 = m_iff2 = 0;

	// say hi
	// Not precise in all cases. z80 must finish current instruction (NOP) to reach this state - in such case frame timings are shifter from cb event if calulated based on it.
	m_irqack_cb(true);
	m_r++;

	// fetch the IRQ vector
	device_z80daisy_interface *intf = daisy_get_irq_device();
	int irq_vector = (intf != nullptr) ? intf->z80daisy_irq_ack() : standard_irq_callback(0, m_pc.w.l);
	LOG(("Z80 single int. irq_vector $%02x\n", irq_vector));

	/* 'interrupt latency' cycles */
	m_icount_executing = 0;
	CC(ex, 0xff); // 2
	T(m_icount_executing);

	/* Interrupt mode 2. Call [i:databyte] */
	if( m_im == 2 )
	{
		// Zilog's datasheet claims that "the least-significant bit must be a zero."
		// However, experiments have confirmed that IM 2 vectors do not have to be
		// even, and all 8 bits will be used; even $FF is handled normally.
		/* CALL opcode timing */
		CC(op, 0xcd); // 17+2=19
		T(m_icount_executing - MTM * 4);
		m_icount_executing -= MTM * 2; // save for rm16
		wm16_sp(m_pc);
		m_icount_executing += MTM * 2;
		irq_vector = (irq_vector & 0xff) | (m_i << 8);
		rm16(irq_vector, m_pc);
		LOG(("Z80 IM2 [$%04x] = $%04x\n", irq_vector, PCD));
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( m_im == 1 )
	{
		LOG(("Z80 '%s' IM1 $0038\n", tag()));
		/* RST $38 */
		CC(op, 0xff); // 11+2=13
		T(m_icount_executing - MTM * 2);
		wm16_sp(m_pc);
		PCD = 0x0038;
	}
	else
	{
		/* Interrupt mode 0. We check for CALL and JP instructions, */
		/* if neither of these were found we assume a 1 byte opcode */
		/* was placed on the databus                                */
		LOG(("Z80 IM0 $%04x\n", irq_vector));

		/* check for nop */
		if (irq_vector != 0x00)
		{
			switch (irq_vector & 0xff0000)
			{
				case 0xcd0000:  /* call */
					/* CALL $xxxx cycles */
					CC(op, 0xcd);
					T(m_icount_executing - MTM * 2);
					wm16_sp(m_pc);
					PCD = irq_vector & 0xffff;
					break;
				case 0xc30000:  /* jump */
					/* JP $xxxx cycles */
					CC(op, 0xc3);
					T(m_icount_executing);
					PCD = irq_vector & 0xffff;
					break;
				default:        /* rst (or other opcodes?) */
					if (irq_vector == 0xfb)
					{
						// EI
						CC(op, 0xfb);
						T(m_icount_executing);
						ei();
					}
					else if ((irq_vector & 0xc7) == 0xc7)
					{
						/* RST $xx cycles */
						CC(op, 0xff);
						T(m_icount_executing - MTM * 2);
						wm16_sp(m_pc);
						PCD = irq_vector & 0x0038;
					}
					else
						logerror("take_interrupt: unexpected opcode in im0 mode: 0x%02x\n", irq_vector);
					break;
			}
		}
	}
	WZ=PCD;

#if HAS_LDAIR_QUIRK
	/* reset parity flag after LD A,I or LD A,R */
	if (m_after_ldair) F &= ~PF;
#endif
}

z80_device::ops_type z80_device::nomreq_ir(s8 cycles)
{
	return ST_F {
		TADR = (m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f); } MN
		nomreq_addr(cycles)                               EST
}

z80_device::ops_type z80_device::nomreq_addr(s8 cycles)
{
	auto steps = ST_F {
		m_nomreq_cb(TADR, 0x00, 0xff); } FN {
		T(1*m_cycles_multiplier);      } EST

	while(--cycles) {
		steps.push_back(steps[0]);
		steps.push_back(steps[1]);
	}

	return steps;
}

void nsc800_device::take_interrupt_nsc800()
{
	/* Check if processor was halted */
	leave_halt();

	/* Clear both interrupt flip flops */
	m_iff1 = m_iff2 = 0;

	/* 'interrupt latency' cycles */
	m_icount_executing = 0;
	CC(op, 0xff);
	CC(ex, 0xff); //2

	T(m_icount_executing - MTM * 2);
	if (m_nsc800_irq_state[NSC800_RSTA])
	{
		wm16_sp(m_pc);
		PCD = 0x003c;
	}
	else if (m_nsc800_irq_state[NSC800_RSTB])
	{
		wm16_sp(m_pc);
		PCD = 0x0034;
	}
	else if (m_nsc800_irq_state[NSC800_RSTC])
	{
		wm16_sp(m_pc);
		PCD = 0x002c;
	}
	T(m_icount_executing);

	WZ=PCD;

#if HAS_LDAIR_QUIRK
	/* reset parity flag after LD A,I or LD A,R */
	if (m_after_ldair) F &= ~PF;
#endif
}

/****************************************************************************
 * Processor initialization
 ****************************************************************************/
ALLOW_SAVE_TYPE(z80_device::op_prefix);
void z80_device::device_start()
{
	if( !tables_initialised )
	{
		u8 *padd = &SZHVC_add[  0*256];
		u8 *padc = &SZHVC_add[256*256];
		u8 *psub = &SZHVC_sub[  0*256];
		u8 *psbc = &SZHVC_sub[256*256];
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
	init_op_steps();

	save_item(NAME(m_icount_executing));
	save_item(NAME(m_cycle));
	save_item(NAME(m_prefix));
	save_item(NAME(m_opcode));

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
	save_item(NAME(m_m_shared_addr.w));
	save_item(NAME(m_m_shared_data.w));
	save_item(NAME(m_m_shared_data2.w));

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

	space(AS_PROGRAM).cache(m_args);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_opcodes);
	space(AS_PROGRAM).specific(m_data);
	space(AS_IO).specific(m_io);

	IX = IY = 0xffff; /* IX and IY are FFFF after a reset! */
	F = ZF;           /* Zero flag is set */

	/* set up the state table */
	state_add(STATE_GENPC,     "PC",        m_pc.w.l).callimport();
	state_add(STATE_GENPCBASE, "CURPC",     m_prvpc.w.l).callimport().noshow();
	state_add(Z80_SP,          "SP",        SP);
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
	set_icountptr(m_icount);

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
	leave_halt();

	m_icount_executing = 0;
	m_cycle = 0;
	m_prefix = NONE;
	m_opcode = 0;

	PC = 0x0000;
	m_i = 0;
	m_r = 0;
	m_r2 = 0;
	m_nmi_pending = false;
	m_after_ei = false;
	m_after_ldair = false;
	m_iff1 = 0;
	m_iff2 = 0;

	WZ=PCD;
}

void nsc800_device::device_reset()
{
	z80_device::device_reset();
	memset(m_nsc800_irq_state, 0, sizeof(m_nsc800_irq_state));
}

void z80_device::calculate_icount()
{
	switch (m_prefix)
	{
		case NONE:  CC(op,   m_opcode); break;
		case CB:    CC(cb,   m_opcode); break;
		case DD:    CC(dd,   m_opcode); break;
		case ED:    CC(ed,   m_opcode); break;
		case FD:    CC(fd,   m_opcode); break;
		case XY_CB: CC(xycb, m_opcode); break;
		default:    assert(false);
	}
}

inline void z80_device::execute_cycles(u8 icount)
{
	//assert(icount >= 0);
	m_icount -= icount;
	m_icount_executing -= icount;
}

/****************************************************************************
 * Execute 'cycles' T-states.
 ****************************************************************************/
void z80_device::execute_run()
{
	while (m_icount > 0)
	{
		ops_type &v = m_op_steps[m_prefix][m_opcode];
		while (m_cycle < v.size() && m_icount > 0)
		{
			if (m_wait_state)
			{
				m_icount = 0; // stalled
				return;
			}
			const int icount = m_icount;
			const int executing = m_icount_executing;
			v[m_cycle++]();
			if ((m_icount < 0) && access_to_be_redone())
			{
				m_icount = icount;
				m_icount_executing = executing;
				m_cycle--;
				return;
			}
		}

		if(m_cycle >= v.size())
			m_cycle = 0;
	}
}

z80_device::ops_type z80_device::next_op()
{
	return ST_F {
		//assert(m_icount_executing == 0); // expected to be valid without custom op_* tables (and in base z80 implementation)
		if (m_icount_executing > 0) T(m_icount_executing); else m_icount_executing = 0;
		// check for interrupts before each instruction
		check_interrupts();
	//} FN {
		m_after_ei = false;
		m_after_ldair = false;

		PRVPC = PCD;
		debugger_instruction_hook(PCD);
	} MN
		rop()
	FN {
		m_prefix = NONE;
		m_opcode = TDAT8;
		// when in HALT state, the fetched opcode is not dispatched (aka a NOP)
		if (m_halt)
		{
			PC--;
			m_opcode = 0;
		}

		calculate_icount();
	} EST
}

void z80_device::check_interrupts()
{
	if (m_nmi_pending)
		take_nmi();
	else if (m_irq_state != CLEAR_LINE && m_iff1 && !m_after_ei)
		take_interrupt();
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
			m_nmi_pending = true;
		m_nmi_state = state;
		break;

	case INPUT_LINE_IRQ0:
		/* update the IRQ state via the daisy chain */
		m_irq_state = state;
		if (daisy_chain_present())
			m_irq_state = (daisy_update_irq_state() == ASSERT_LINE ) ? ASSERT_LINE : m_irq_state;

		/* the main execute loop will take the interrupt */
		break;

	case Z80_INPUT_LINE_WAIT:
		m_wait_state = state;
		break;

	default:
		break;
	}
}

void nsc800_device::check_interrupts()
{
	if (m_nmi_pending)
		take_nmi();
	else if ((m_nsc800_irq_state[NSC800_RSTA] != CLEAR_LINE || m_nsc800_irq_state[NSC800_RSTB] != CLEAR_LINE || m_nsc800_irq_state[NSC800_RSTC] != CLEAR_LINE) && m_iff1 && !m_after_ei)
		take_interrupt_nsc800();
	else if (m_irq_state != CLEAR_LINE && m_iff1 && !m_after_ei)
		take_interrupt();
}

void nsc800_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case NSC800_RSTA:
		m_nsc800_irq_state[NSC800_RSTA] = state;
		break;

	case NSC800_RSTB:
		m_nsc800_irq_state[NSC800_RSTB] = state;
		break;

	case NSC800_RSTC:
		m_nsc800_irq_state[NSC800_RSTC] = state;
		break;

	default:
		z80_device::execute_set_input(inputnum, state);
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
		case STATE_GENPC:
			m_prvpc = m_pc;
			break;

		case STATE_GENPCBASE:
			m_pc = m_prvpc;
			break;

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

void z80_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
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

/**************************************************************************
 * disassemble - call the disassembly helper function
 **************************************************************************/
std::unique_ptr<util::disasm_interface> z80_device::create_disassembler()
{
	return std::make_unique<z80_disassembler>();
}

void z80_device::z80_set_cycle_tables(const u8 *op, const u8 *cb, const u8 *ed, const u8 *xy, const u8 *xycb, const u8 *ex)
{
	m_cc_op = (op != nullptr) ? op : cc_op;
	m_cc_cb = (cb != nullptr) ? cb : cc_cb;
	m_cc_ed = (ed != nullptr) ? ed : cc_ed;
	m_cc_xy = (xy != nullptr) ? xy : cc_xy;
	m_cc_xycb = (xycb != nullptr) ? xycb : cc_xycb;
	m_cc_ex = (ex != nullptr) ? ex : cc_ex;
}

z80_device::z80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	z80_device(mconfig, Z80, tag, owner, clock)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	cpu_device(mconfig, type, tag, owner, clock),
	z80_daisy_chain_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 16, 0),
	m_opcodes_config("opcodes", ENDIANNESS_LITTLE, 8, 16, 0),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0),
	m_irqack_cb(*this),
	m_refresh_cb(*this),
	m_nomreq_cb(*this),
	m_halt_cb(*this)
{
}

device_memory_interface::space_config_vector z80_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_opcodes_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_IO,      &m_io_config)
		};
}

DEFINE_DEVICE_TYPE(Z80, z80_device, "z80", "Zilog Z80")

nsc800_device::nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, NSC800, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(NSC800, nsc800_device, "nsc800", "National Semiconductor NSC800")
