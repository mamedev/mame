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
 *    Fundation for M cycles emulation. Currently we preserve cc_* tables with total timings.
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
 *    - Changed variable ea and arg16() function to uint32_t; this
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

#define TMP     m_m_shared.w.l
#define TMP_H   m_m_shared.b.h
#define TMP_L   m_m_shared.b.l

static bool tables_initialised = false;
static uint8_t SZ[256];       /* zero and sign flags */
static uint8_t SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static uint8_t SZP[256];      /* zero, sign and parity flags */
static uint8_t SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static uint8_t SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static uint8_t SZHVC_add[2*256*256];
static uint8_t SZHVC_sub[2*256*256];

static const uint8_t cc_op[0x100] = {
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

static const uint8_t cc_ed[0x100] = {
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
static const uint8_t cc_xy[0x100] = {
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

static const uint8_t cc_xycb[0x100] = {
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
static const uint8_t cc_ex[0x100] = {
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
#define OP(prefix,opcode) prefix##_##opcode = ops_flat({{[&]()
//#define OPN ,[&]()
#define EOP }, next_op()});

#define OPR(prefix,opcode) prefix##_##opcode = 

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define CC(prefix,opcode) do { m_icount_executing += m_cc_##prefix == nullptr ? 0 : m_cc_##prefix[opcode]; } while (0)

#define T(icount) do { \
	m_icount -= icount; \
	m_icount_executing -= icount; \
} while (0)

#define CP(prefix, opcode) case 0x##opcode: return &z80_device::prefix##_##opcode;break;

// T Memory Address
#define MTM ((m_cc_op == nullptr ? 4 : m_cc_op[0])-1)

#define EXEC(prefix,opcode) do { \
	unsigned op = opcode; \
	switch(op) \
	{  \
	CP(prefix, 00) CP(prefix, 01) CP(prefix, 02) CP(prefix, 03) CP(prefix, 04) CP(prefix, 05) CP(prefix, 06) CP(prefix, 07) \
	CP(prefix, 08) CP(prefix, 09) CP(prefix, 0a) CP(prefix, 0b) CP(prefix, 0c) CP(prefix, 0d) CP(prefix, 0e) CP(prefix, 0f) \
	CP(prefix, 10) CP(prefix, 11) CP(prefix, 12) CP(prefix, 13) CP(prefix, 14) CP(prefix, 15) CP(prefix, 16) CP(prefix, 17) \
	CP(prefix, 18) CP(prefix, 19) CP(prefix, 1a) CP(prefix, 1b) CP(prefix, 1c) CP(prefix, 1d) CP(prefix, 1e) CP(prefix, 1f) \
	CP(prefix, 20) CP(prefix, 21) CP(prefix, 22) CP(prefix, 23) CP(prefix, 24) CP(prefix, 25) CP(prefix, 26) CP(prefix, 27) \
	CP(prefix, 28) CP(prefix, 29) CP(prefix, 2a) CP(prefix, 2b) CP(prefix, 2c) CP(prefix, 2d) CP(prefix, 2e) CP(prefix, 2f) \
	CP(prefix, 30) CP(prefix, 31) CP(prefix, 32) CP(prefix, 33) CP(prefix, 34) CP(prefix, 35) CP(prefix, 36) CP(prefix, 37) \
	CP(prefix, 38) CP(prefix, 39) CP(prefix, 3a) CP(prefix, 3b) CP(prefix, 3c) CP(prefix, 3d) CP(prefix, 3e) CP(prefix, 3f) \
	CP(prefix, 40) CP(prefix, 41) CP(prefix, 42) CP(prefix, 43) CP(prefix, 44) CP(prefix, 45) CP(prefix, 46) CP(prefix, 47) \
	CP(prefix, 48) CP(prefix, 49) CP(prefix, 4a) CP(prefix, 4b) CP(prefix, 4c) CP(prefix, 4d) CP(prefix, 4e) CP(prefix, 4f) \
	CP(prefix, 50) CP(prefix, 51) CP(prefix, 52) CP(prefix, 53) CP(prefix, 54) CP(prefix, 55) CP(prefix, 56) CP(prefix, 57) \
	CP(prefix, 58) CP(prefix, 59) CP(prefix, 5a) CP(prefix, 5b) CP(prefix, 5c) CP(prefix, 5d) CP(prefix, 5e) CP(prefix, 5f) \
	CP(prefix, 60) CP(prefix, 61) CP(prefix, 62) CP(prefix, 63) CP(prefix, 64) CP(prefix, 65) CP(prefix, 66) CP(prefix, 67) \
	CP(prefix, 68) CP(prefix, 69) CP(prefix, 6a) CP(prefix, 6b) CP(prefix, 6c) CP(prefix, 6d) CP(prefix, 6e) CP(prefix, 6f) \
	CP(prefix, 70) CP(prefix, 71) CP(prefix, 72) CP(prefix, 73) CP(prefix, 74) CP(prefix, 75) CP(prefix, 76) CP(prefix, 77) \
	CP(prefix, 78) CP(prefix, 79) CP(prefix, 7a) CP(prefix, 7b) CP(prefix, 7c) CP(prefix, 7d) CP(prefix, 7e) CP(prefix, 7f) \
	CP(prefix, 80) CP(prefix, 81) CP(prefix, 82) CP(prefix, 83) CP(prefix, 84) CP(prefix, 85) CP(prefix, 86) CP(prefix, 87) \
	CP(prefix, 88) CP(prefix, 89) CP(prefix, 8a) CP(prefix, 8b) CP(prefix, 8c) CP(prefix, 8d) CP(prefix, 8e) CP(prefix, 8f) \
	CP(prefix, 90) CP(prefix, 91) CP(prefix, 92) CP(prefix, 93) CP(prefix, 94) CP(prefix, 95) CP(prefix, 96) CP(prefix, 97) \
	CP(prefix, 98) CP(prefix, 99) CP(prefix, 9a) CP(prefix, 9b) CP(prefix, 9c) CP(prefix, 9d) CP(prefix, 9e) CP(prefix, 9f) \
	CP(prefix, a0) CP(prefix, a1) CP(prefix, a2) CP(prefix, a3) CP(prefix, a4) CP(prefix, a5) CP(prefix, a6) CP(prefix, a7) \
	CP(prefix, a8) CP(prefix, a9) CP(prefix, aa) CP(prefix, ab) CP(prefix, ac) CP(prefix, ad) CP(prefix, ae) CP(prefix, af) \
	CP(prefix, b0) CP(prefix, b1) CP(prefix, b2) CP(prefix, b3) CP(prefix, b4) CP(prefix, b5) CP(prefix, b6) CP(prefix, b7) \
	CP(prefix, b8) CP(prefix, b9) CP(prefix, ba) CP(prefix, bb) CP(prefix, bc) CP(prefix, bd) CP(prefix, be) CP(prefix, bf) \
	CP(prefix, c0) CP(prefix, c1) CP(prefix, c2) CP(prefix, c3) CP(prefix, c4) CP(prefix, c5) CP(prefix, c6) CP(prefix, c7) \
	CP(prefix, c8) CP(prefix, c9) CP(prefix, ca) CP(prefix, cb) CP(prefix, cc) CP(prefix, cd) CP(prefix, ce) CP(prefix, cf) \
	CP(prefix, d0) CP(prefix, d1) CP(prefix, d2) CP(prefix, d3) CP(prefix, d4) CP(prefix, d5) CP(prefix, d6) CP(prefix, d7) \
	CP(prefix, d8) CP(prefix, d9) CP(prefix, da) CP(prefix, db) CP(prefix, dc) CP(prefix, dd) CP(prefix, de) CP(prefix, df) \
	CP(prefix, e0) CP(prefix, e1) CP(prefix, e2) CP(prefix, e3) CP(prefix, e4) CP(prefix, e5) CP(prefix, e6) CP(prefix, e7) \
	CP(prefix, e8) CP(prefix, e9) CP(prefix, ea) CP(prefix, eb) CP(prefix, ec) CP(prefix, ed) CP(prefix, ee) CP(prefix, ef) \
	CP(prefix, f0) CP(prefix, f1) CP(prefix, f2) CP(prefix, f3) CP(prefix, f4) CP(prefix, f5) CP(prefix, f6) CP(prefix, f7) \
	CP(prefix, f8) CP(prefix, f9) CP(prefix, fa) CP(prefix, fb) CP(prefix, fc) CP(prefix, fd) CP(prefix, fe) CP(prefix, ff) \
	} \
} while (0)

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
inline uint8_t z80_device::in(uint16_t port)
{
	u8 res = m_io.read_byte(port);
	T(4);
	return res;
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
inline void z80_device::out(uint16_t port, uint8_t value)
{
	m_io.write_byte(port, value);
	T(4);
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
uint8_t z80_device::rm(uint16_t addr)
{
	u8 res = m_data.read_byte(addr);
	T(MTM);
	return res;
}

uint8_t z80_device::rm_reg(uint16_t addr)
{
	u8 res = rm(addr);
	nomreq_addr(addr, 1);
	return res;
}

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
inline void z80_device::rm16(uint16_t addr, PAIR &r)
{
	r.b.l = rm(addr);
	r.b.h = rm(addr+1);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
void z80_device::wm(uint16_t addr, uint8_t value)
{
	// As we don't count changes between read and write, simply adjust to the end of requested.
	if(m_icount_executing != MTM) T(m_icount_executing - MTM);
	m_data.write_byte(addr, value);
	T(MTM);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
inline void z80_device::wm16(uint16_t addr, PAIR &r)
{
	m_icount_executing -= MTM;
	wm(addr, r.b.l);
	m_icount_executing += MTM;
	wm(addr+1, r.b.h);
}

/***************************************************************
 * Write a word to (SP)
 ***************************************************************/
inline void z80_device::wm16_sp(PAIR &r)
{
	SP--;
	m_icount_executing -= MTM;
	wm(SPD, r.b.h);
	m_icount_executing += MTM;
	SP--;
	wm(SPD, r.b.l);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
uint8_t z80_device::rop()
{
	uint8_t res = m_opcodes.read_byte(PCD);
	T(execute_min_cycles());
	m_refresh_cb((m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f), 0x00, 0xff);
	T(execute_min_cycles());
	PC++;
	m_r++;

	return res;
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 ***************************************************************/
uint8_t z80_device::arg()
{
	u8 res = m_args.read_byte(PCD);
	T(MTM);
	PC++;

	return res;
}

uint16_t z80_device::arg16()
{
	u8 const res = arg();

	return (u16(arg()) << 8) | res;
}

inline z80_device::ops_type z80_device::arg16_n()
{
	return {[&](){ TMP_L = arg(); },[&](){ TMP_H = arg(); }};
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
inline void z80_device::eax()
{
	m_ea = (uint32_t)(uint16_t)(IX + (int8_t)arg());
	WZ = m_ea;
}

inline void z80_device::eay()
{
	m_ea = (uint32_t)(uint16_t)(IY + (int8_t)arg());
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
	nomreq_ir(1);
	wm16_sp(r);
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
		WZ = arg16(); /* implicit do PC += 2 */
}

/***************************************************************
 * JR
 ***************************************************************/
inline void z80_device::jr()
{
	int8_t a = (int8_t)arg(); /* arg() also increments PC */
	nomreq_addr(PCD-1, 5);
	PC += a;                  /* so don't do PC += arg() */
	WZ = PC;
}

/***************************************************************
 * JR_COND
 ***************************************************************/
inline void z80_device::jr_cond(bool cond, uint8_t opcode)
{
	if (cond)
	{
		CC(ex, opcode);
		jr();
	}
	else
	{
		WZ = arg();
		//nomreq_addr(PCD, 3);
		//PC++;
	}
}

/***************************************************************
 * CALL
 ***************************************************************/
inline void z80_device::call()
{
	m_ea = arg16();
	nomreq_addr(PCD-1, 1);
	WZ = m_ea;
	wm16_sp(m_pc);
	PCD = m_ea;
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
inline void z80_device::call_cond(bool cond, uint8_t opcode)
{
	if (cond)
	{
		CC(ex, opcode);
		m_ea = arg16();
		nomreq_addr(PCD-1, 1);
		WZ = m_ea;
		wm16_sp(m_pc);
		PCD = m_ea;
	}
	else
		WZ = arg16(); /* implicit call PC+=2; */
}

/***************************************************************
 * RET_COND
 ***************************************************************/
inline void z80_device::ret_cond(bool cond, uint8_t opcode)
{
	nomreq_ir(1);
	if (cond)
	{
		CC(ex, opcode);
		pop(m_pc);
		WZ = PC;
	}
}

/***************************************************************
 * RETN
 ***************************************************************/
inline void z80_device::retn()
{
	LOG(("Z80 RETN m_iff1:%d m_iff2:%d\n", m_iff1, m_iff2));
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
	daisy_call_reti_device();
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
inline void z80_device::ld_r_a()
{
	nomreq_ir(1);
	m_r = A;
	m_r2 = A & 0x80; /* keep bit 7 of r */
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
inline void z80_device::ld_a_r()
{
	nomreq_ir(1);
	A = (m_r & 0x7f) | m_r2;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = true;
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
inline void z80_device::ld_i_a()
{
	nomreq_ir(1);
	m_i = A;
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
inline void z80_device::ld_a_i()
{
	nomreq_ir(1);
	A = m_i;
	F = (F & CF) | SZ[A] | (m_iff2 << 2);
	m_after_ldair = true;
}

/***************************************************************
 * RST
 ***************************************************************/
inline void z80_device::rst(uint16_t addr)
{
	//nomreq_ir(1);
	push(m_pc);
	PCD = addr;
	WZ = PC;
}

/***************************************************************
 * INC  r8
 ***************************************************************/
inline uint8_t z80_device::inc(uint8_t value)
{
	uint8_t res = value + 1;
	F = (F & CF) | SZHV_inc[res];
	return (uint8_t)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline uint8_t z80_device::dec(uint8_t value)
{
	uint8_t res = value - 1;
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
	uint8_t res = (A << 1) | (F & CF);
	uint8_t c = (A & 0x80) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void z80_device::rra()
{
	uint8_t res = (A >> 1) | (F << 7);
	uint8_t c = (A & 0x01) ? CF : 0;
	F = (F & (SF | ZF | PF)) | c | (res & (YF | XF));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
inline void z80_device::rrd()
{
	uint8_t n = rm(HL);
	WZ = HL+1;
	nomreq_addr(HL, 4);
	wm(HL, (n >> 4) | (A << 4));
	A = (A & 0xf0) | (n & 0x0f);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * RLD
 ***************************************************************/
inline void z80_device::rld()
{
	uint8_t n = rm(HL);
	WZ = HL+1;
	nomreq_addr(HL, 4);
	wm(HL, (n << 4) | (A & 0x0f));
	A = (A & 0xf0) | (n >> 4);
	F = (F & CF) | SZP[A];
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void z80_device::add_a(uint8_t value)
{
	uint32_t ah = AFD & 0xff00;
	uint32_t res = (uint8_t)((ah >> 8) + value);
	F = SZHVC_add[ah | res];
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void z80_device::adc_a(uint8_t value)
{
	uint32_t ah = AFD & 0xff00, c = AFD & 1;
	uint32_t res = (uint8_t)((ah >> 8) + value + c);
	F = SZHVC_add[(c << 16) | ah | res];
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void z80_device::sub(uint8_t value)
{
	uint32_t ah = AFD & 0xff00;
	uint32_t res = (uint8_t)((ah >> 8) - value);
	F = SZHVC_sub[ah | res];
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void z80_device::sbc_a(uint8_t value)
{
	uint32_t ah = AFD & 0xff00, c = AFD & 1;
	uint32_t res = (uint8_t)((ah >> 8) - value - c);
	F = SZHVC_sub[(c<<16) | ah | res];
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
inline void z80_device::neg()
{
	uint8_t value = A;
	A = 0;
	sub(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
inline void z80_device::daa()
{
	uint8_t a = A;
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
inline void z80_device::and_a(uint8_t value)
{
	A &= value;
	F = SZP[A] | HF;
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void z80_device::or_a(uint8_t value)
{
	A |= value;
	F = SZP[A];
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void z80_device::xor_a(uint8_t value)
{
	A ^= value;
	F = SZP[A];
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void z80_device::cp(uint8_t value)
{
	unsigned val = value;
	uint32_t ah = AFD & 0xff00;
	uint32_t res = (uint8_t)((ah >> 8) - val);
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
	pop(tmp);
	nomreq_addr(SPD - 1, 1);
	m_icount_executing -= 2;
	wm16_sp(r);
	m_icount_executing += 2;
	nomreq_addr(SPD, 2);
	r = tmp;
	WZ = r.d;
}

/***************************************************************
 * ADD16
 ***************************************************************/
inline void z80_device::add16(PAIR &dr, PAIR &sr)
{
	nomreq_ir(7);
	uint32_t res = dr.d + sr.d;
	WZ = dr.d + 1;
	F = (F & (SF | ZF | VF)) |
		(((dr.d ^ res ^ sr.d) >> 8) & HF) |
		((res >> 16) & CF) | ((res >> 8) & (YF | XF));
	dr.w.l = (uint16_t)res;
}

/***************************************************************
 * ADC  HL,r16
 ***************************************************************/
inline void z80_device::adc_hl(PAIR &r)
{
	nomreq_ir(7);
	uint32_t res = HLD + r.d + (F & CF);
	WZ = HL + 1;
	F = (((HLD ^ res ^ r.d) >> 8) & HF) |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((r.d ^ HLD ^ 0x8000) & (r.d ^ res) & 0x8000) >> 13);
	HL = (uint16_t)res;
}

/***************************************************************
 * SBC  HL,r16
 ***************************************************************/
inline void z80_device::sbc_hl(PAIR &r)
{
	nomreq_ir(7);
	uint32_t res = HLD - r.d - (F & CF);
	WZ = HL + 1;
	F = (((HLD ^ res ^ r.d) >> 8) & HF) | NF |
		((res >> 16) & CF) |
		((res >> 8) & (SF | YF | XF)) |
		((res & 0xffff) ? 0 : ZF) |
		(((r.d ^ HLD) & (HLD ^ res) &0x8000) >> 13);
	HL = (uint16_t)res;
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
inline uint8_t z80_device::rlc(uint8_t value)
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
inline uint8_t z80_device::rrc(uint8_t value)
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
inline uint8_t z80_device::rl(uint8_t value)
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
inline uint8_t z80_device::rr(uint8_t value)
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
inline uint8_t z80_device::sla(uint8_t value)
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
inline uint8_t z80_device::sra(uint8_t value)
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
inline uint8_t z80_device::sll(uint8_t value)
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
inline uint8_t z80_device::srl(uint8_t value)
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
inline void z80_device::bit(int bit, uint8_t value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (value & (YF|XF));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void z80_device::bit_hl(int bit, uint8_t value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void z80_device::bit_xy(int bit, uint8_t value)
{
	F = (F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | ((m_ea>>8) & (YF|XF));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
inline uint8_t z80_device::res(int bit, uint8_t value)
{
	return value & ~(1<<bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
inline uint8_t z80_device::set(int bit, uint8_t value)
{
	return value | (1<<bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
inline void z80_device::ldi()
{
	uint8_t io = rm(HL);
	m_icount_executing -= 2;
	wm(DE, io);
	m_icount_executing += 2;
	nomreq_addr(DE, 2);
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
	uint8_t val = rm(HL);
	nomreq_addr(HL, 5);
	uint8_t res = A - val;
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
	nomreq_ir(1);
	unsigned t;
	uint8_t io = in(BC);
	WZ = BC + 1;
	B--;
	wm(HL, io);
	HL++;
	F = SZ[B];
	t = (unsigned)((C + 1) & 0xff) + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * OUTI
 ***************************************************************/
inline void z80_device::outi()
{
	nomreq_ir(1);
	unsigned t;
	uint8_t io = rm(HL);
	B--;
	WZ = BC + 1;
	out(BC, io);
	HL++;
	F = SZ[B];
	t = (unsigned)L + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * LDD
 ***************************************************************/
inline void z80_device::ldd()
{
	uint8_t io = rm(HL);
	m_icount_executing -= 2;
	wm(DE, io);
	m_icount_executing += 2;
	nomreq_addr(DE, 2);
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
	uint8_t val = rm(HL);
	nomreq_addr(HL, 5);
	uint8_t res = A - val;
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
	nomreq_ir(1);
	unsigned t;
	uint8_t io = in(BC);
	WZ = BC - 1;
	B--;
	wm(HL, io);
	HL--;
	F = SZ[B];
	t = ((unsigned)(C - 1) & 0xff) + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * OUTD
 ***************************************************************/
inline void z80_device::outd()
{
	nomreq_ir(1);
	unsigned t;
	uint8_t io = rm(HL);
	B--;
	WZ = BC - 1;
	out(BC, io);
	HL--;
	F = SZ[B];
	t = (unsigned)L + (unsigned)io;
	if (io & SF) F |= NF;
	if (t & 0x100) F |= HF | CF;
	F |= SZP[(uint8_t)(t & 0x07) ^ B] & PF;
}

/***************************************************************
 * LDIR
 ***************************************************************/
inline void z80_device::ldir()
{
	ldi();
	if (BC != 0)
	{
		CC(ex, 0xb0);
		nomreq_addr(DE, 5);
		PC -= 2;
		WZ = PC + 1;
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
		CC(ex, 0xb1);
		nomreq_addr(HL, 5);
		PC -= 2;
		WZ = PC + 1;
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
		CC(ex, 0xb2);
		nomreq_addr(HL, 5);
		PC -= 2;
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
		CC(ex, 0xb3);
		nomreq_addr(BC, 5);
		PC -= 2;
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
		CC(ex, 0xb8);
		nomreq_addr(DE, 5);
		PC -= 2;
		WZ = PC + 1;
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
		CC(ex, 0xb9);
		nomreq_addr(HL, 5);
		PC -= 2;
		WZ = PC + 1;
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
		CC(ex, 0xba);
		nomreq_addr(HL, 5);
		PC -= 2;
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
		CC(ex, 0xbb);
		nomreq_addr(BC, 5);
		PC -= 2;
	}
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
			m_opcodes.read_byte((PCD-1)&0xffff), m_opcodes.read_byte(PCD), PCD-1);
}

inline z80_device::ops_type z80_device::illegal_1(z80_device::ops_type ref) {
	//illegal_1();
	return ref;
}

inline void z80_device::illegal_2()
{
	logerror("Z80 ill. opcode $ed $%02x\n",
			m_opcodes.read_byte((PCD-1)&0xffff));
}

void z80_device::init_instructions() {
	//m_tmp = {{[&](){B = rlc(B);}}};

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
OP(cb,00) { B = rlc(B);             } EOP /* RLC  B           */
OP(cb,01) { C = rlc(C);             } EOP /* RLC  C           */
OP(cb,02) { D = rlc(D);             } EOP /* RLC  D           */
OP(cb,03) { E = rlc(E);             } EOP /* RLC  E           */
OP(cb,04) { H = rlc(H);             } EOP /* RLC  H           */
OP(cb,05) { L = rlc(L);             } EOP /* RLC  L           */
OP(cb,06) { wm(HL, rlc(rm_reg(HL)));} EOP /* RLC  (HL)        */
OP(cb,07) { A = rlc(A);             } EOP /* RLC  A           */

OP(cb,08) { B = rrc(B);             } EOP /* RRC  B           */
OP(cb,09) { C = rrc(C);             } EOP /* RRC  C           */
OP(cb,0a) { D = rrc(D);             } EOP /* RRC  D           */
OP(cb,0b) { E = rrc(E);             } EOP /* RRC  E           */
OP(cb,0c) { H = rrc(H);             } EOP /* RRC  H           */
OP(cb,0d) { L = rrc(L);             } EOP /* RRC  L           */
OP(cb,0e) { wm(HL, rrc(rm_reg(HL)));} EOP /* RRC  (HL)        */
OP(cb,0f) { A = rrc(A);             } EOP /* RRC  A           */

OP(cb,10) { B = rl(B);              } EOP /* RL   B           */
OP(cb,11) { C = rl(C);              } EOP /* RL   C           */
OP(cb,12) { D = rl(D);              } EOP /* RL   D           */
OP(cb,13) { E = rl(E);              } EOP /* RL   E           */
OP(cb,14) { H = rl(H);              } EOP /* RL   H           */
OP(cb,15) { L = rl(L);              } EOP /* RL   L           */
OP(cb,16) { wm(HL, rl(rm_reg(HL))); } EOP /* RL   (HL)        */
OP(cb,17) { A = rl(A);              } EOP /* RL   A           */

OP(cb,18) { B = rr(B);              } EOP /* RR   B           */
OP(cb,19) { C = rr(C);              } EOP /* RR   C           */
OP(cb,1a) { D = rr(D);              } EOP /* RR   D           */
OP(cb,1b) { E = rr(E);              } EOP /* RR   E           */
OP(cb,1c) { H = rr(H);              } EOP /* RR   H           */
OP(cb,1d) { L = rr(L);              } EOP /* RR   L           */
OP(cb,1e) { wm(HL, rr(rm_reg(HL))); } EOP /* RR   (HL)        */
OP(cb,1f) { A = rr(A);              } EOP /* RR   A           */

OP(cb,20) { B = sla(B);             } EOP /* SLA  B           */
OP(cb,21) { C = sla(C);             } EOP /* SLA  C           */
OP(cb,22) { D = sla(D);             } EOP /* SLA  D           */
OP(cb,23) { E = sla(E);             } EOP /* SLA  E           */
OP(cb,24) { H = sla(H);             } EOP /* SLA  H           */
OP(cb,25) { L = sla(L);             } EOP /* SLA  L           */
OP(cb,26) { wm(HL, sla(rm_reg(HL)));} EOP /* SLA  (HL)        */
OP(cb,27) { A = sla(A);             } EOP /* SLA  A           */

OP(cb,28) { B = sra(B);             } EOP /* SRA  B           */
OP(cb,29) { C = sra(C);             } EOP /* SRA  C           */
OP(cb,2a) { D = sra(D);             } EOP /* SRA  D           */
OP(cb,2b) { E = sra(E);             } EOP /* SRA  E           */
OP(cb,2c) { H = sra(H);             } EOP /* SRA  H           */
OP(cb,2d) { L = sra(L);             } EOP /* SRA  L           */
OP(cb,2e) { wm(HL, sra(rm_reg(HL)));} EOP /* SRA  (HL)        */
OP(cb,2f) { A = sra(A);             } EOP /* SRA  A           */

OP(cb,30) { B = sll(B);             } EOP /* SLL  B           */
OP(cb,31) { C = sll(C);             } EOP /* SLL  C           */
OP(cb,32) { D = sll(D);             } EOP /* SLL  D           */
OP(cb,33) { E = sll(E);             } EOP /* SLL  E           */
OP(cb,34) { H = sll(H);             } EOP /* SLL  H           */
OP(cb,35) { L = sll(L);             } EOP /* SLL  L           */
OP(cb,36) { wm(HL, sll(rm_reg(HL)));} EOP /* SLL  (HL)        */
OP(cb,37) { A = sll(A);             } EOP /* SLL  A           */

OP(cb,38) { B = srl(B);             } EOP /* SRL  B           */
OP(cb,39) { C = srl(C);             } EOP /* SRL  C           */
OP(cb,3a) { D = srl(D);             } EOP /* SRL  D           */
OP(cb,3b) { E = srl(E);             } EOP /* SRL  E           */
OP(cb,3c) { H = srl(H);             } EOP /* SRL  H           */
OP(cb,3d) { L = srl(L);             } EOP /* SRL  L           */
OP(cb,3e) { wm(HL, srl(rm_reg(HL)));} EOP /* SRL  (HL)        */
OP(cb,3f) { A = srl(A);             } EOP /* SRL  A           */

OP(cb,40) { bit(0, B);              } EOP /* BIT  0,B         */
OP(cb,41) { bit(0, C);              } EOP /* BIT  0,C         */
OP(cb,42) { bit(0, D);              } EOP /* BIT  0,D         */
OP(cb,43) { bit(0, E);              } EOP /* BIT  0,E         */
OP(cb,44) { bit(0, H);              } EOP /* BIT  0,H         */
OP(cb,45) { bit(0, L);              } EOP /* BIT  0,L         */
OP(cb,46) { bit_hl(0, rm_reg(HL));  } EOP /* BIT  0,(HL)      */
OP(cb,47) { bit(0, A);              } EOP /* BIT  0,A         */

OP(cb,48) { bit(1, B);              } EOP /* BIT  1,B         */
OP(cb,49) { bit(1, C);              } EOP /* BIT  1,C         */
OP(cb,4a) { bit(1, D);              } EOP /* BIT  1,D         */
OP(cb,4b) { bit(1, E);              } EOP /* BIT  1,E         */
OP(cb,4c) { bit(1, H);              } EOP /* BIT  1,H         */
OP(cb,4d) { bit(1, L);              } EOP /* BIT  1,L         */
OP(cb,4e) { bit_hl(1, rm_reg(HL));  } EOP /* BIT  1,(HL)      */
OP(cb,4f) { bit(1, A);              } EOP /* BIT  1,A         */

OP(cb,50) { bit(2, B);              } EOP /* BIT  2,B         */
OP(cb,51) { bit(2, C);              } EOP /* BIT  2,C         */
OP(cb,52) { bit(2, D);              } EOP /* BIT  2,D         */
OP(cb,53) { bit(2, E);              } EOP /* BIT  2,E         */
OP(cb,54) { bit(2, H);              } EOP /* BIT  2,H         */
OP(cb,55) { bit(2, L);              } EOP /* BIT  2,L         */
OP(cb,56) { bit_hl(2, rm_reg(HL));  } EOP /* BIT  2,(HL)      */
OP(cb,57) { bit(2, A);              } EOP /* BIT  2,A         */

OP(cb,58) { bit(3, B);              } EOP /* BIT  3,B         */
OP(cb,59) { bit(3, C);              } EOP /* BIT  3,C         */
OP(cb,5a) { bit(3, D);              } EOP /* BIT  3,D         */
OP(cb,5b) { bit(3, E);              } EOP /* BIT  3,E         */
OP(cb,5c) { bit(3, H);              } EOP /* BIT  3,H         */
OP(cb,5d) { bit(3, L);              } EOP /* BIT  3,L         */
OP(cb,5e) { bit_hl(3, rm_reg(HL));  } EOP /* BIT  3,(HL)      */
OP(cb,5f) { bit(3, A);              } EOP /* BIT  3,A         */

OP(cb,60) { bit(4, B);              } EOP /* BIT  4,B         */
OP(cb,61) { bit(4, C);              } EOP /* BIT  4,C         */
OP(cb,62) { bit(4, D);              } EOP /* BIT  4,D         */
OP(cb,63) { bit(4, E);              } EOP /* BIT  4,E         */
OP(cb,64) { bit(4, H);              } EOP /* BIT  4,H         */
OP(cb,65) { bit(4, L);              } EOP /* BIT  4,L         */
OP(cb,66) { bit_hl(4, rm_reg(HL));  } EOP /* BIT  4,(HL)      */
OP(cb,67) { bit(4, A);              } EOP /* BIT  4,A         */

OP(cb,68) { bit(5, B);              } EOP /* BIT  5,B         */
OP(cb,69) { bit(5, C);              } EOP /* BIT  5,C         */
OP(cb,6a) { bit(5, D);              } EOP /* BIT  5,D         */
OP(cb,6b) { bit(5, E);              } EOP /* BIT  5,E         */
OP(cb,6c) { bit(5, H);              } EOP /* BIT  5,H         */
OP(cb,6d) { bit(5, L);              } EOP /* BIT  5,L         */
OP(cb,6e) { bit_hl(5, rm_reg(HL));  } EOP /* BIT  5,(HL)      */
OP(cb,6f) { bit(5, A);              } EOP /* BIT  5,A         */

OP(cb,70) { bit(6, B);              } EOP /* BIT  6,B         */
OP(cb,71) { bit(6, C);              } EOP /* BIT  6,C         */
OP(cb,72) { bit(6, D);              } EOP /* BIT  6,D         */
OP(cb,73) { bit(6, E);              } EOP /* BIT  6,E         */
OP(cb,74) { bit(6, H);              } EOP /* BIT  6,H         */
OP(cb,75) { bit(6, L);              } EOP /* BIT  6,L         */
OP(cb,76) { bit_hl(6, rm_reg(HL));  } EOP /* BIT  6,(HL)      */
OP(cb,77) { bit(6, A);              } EOP /* BIT  6,A         */

OP(cb,78) { bit(7, B);              } EOP /* BIT  7,B         */
OP(cb,79) { bit(7, C);              } EOP /* BIT  7,C         */
OP(cb,7a) { bit(7, D);              } EOP /* BIT  7,D         */
OP(cb,7b) { bit(7, E);              } EOP /* BIT  7,E         */
OP(cb,7c) { bit(7, H);              } EOP /* BIT  7,H         */
OP(cb,7d) { bit(7, L);              } EOP /* BIT  7,L         */
OP(cb,7e) { bit_hl(7, rm_reg(HL));  } EOP /* BIT  7,(HL)      */
OP(cb,7f) { bit(7, A);              } EOP /* BIT  7,A         */

OP(cb,80) { B = res(0, B);          } EOP /* RES  0,B         */
OP(cb,81) { C = res(0, C);          } EOP /* RES  0,C         */
OP(cb,82) { D = res(0, D);          } EOP /* RES  0,D         */
OP(cb,83) { E = res(0, E);          } EOP /* RES  0,E         */
OP(cb,84) { H = res(0, H);          } EOP /* RES  0,H         */
OP(cb,85) { L = res(0, L);          } EOP /* RES  0,L         */
OP(cb,86) { wm(HL, res(0, rm_reg(HL))); } EOP /* RES  0,(HL)      */
OP(cb,87) { A = res(0, A);          } EOP /* RES  0,A         */

OP(cb,88) { B = res(1, B);          } EOP /* RES  1,B         */
OP(cb,89) { C = res(1, C);          } EOP /* RES  1,C         */
OP(cb,8a) { D = res(1, D);          } EOP /* RES  1,D         */
OP(cb,8b) { E = res(1, E);          } EOP /* RES  1,E         */
OP(cb,8c) { H = res(1, H);          } EOP /* RES  1,H         */
OP(cb,8d) { L = res(1, L);          } EOP /* RES  1,L         */
OP(cb,8e) { wm(HL, res(1, rm_reg(HL))); } EOP /* RES  1,(HL)      */
OP(cb,8f) { A = res(1, A);          } EOP /* RES  1,A         */

OP(cb,90) { B = res(2, B);          } EOP /* RES  2,B         */
OP(cb,91) { C = res(2, C);          } EOP /* RES  2,C         */
OP(cb,92) { D = res(2, D);          } EOP /* RES  2,D         */
OP(cb,93) { E = res(2, E);          } EOP /* RES  2,E         */
OP(cb,94) { H = res(2, H);          } EOP /* RES  2,H         */
OP(cb,95) { L = res(2, L);          } EOP /* RES  2,L         */
OP(cb,96) { wm(HL, res(2, rm_reg(HL))); } EOP /* RES  2,(HL)      */
OP(cb,97) { A = res(2, A);          } EOP /* RES  2,A         */

OP(cb,98) { B = res(3, B);          } EOP /* RES  3,B         */
OP(cb,99) { C = res(3, C);          } EOP /* RES  3,C         */
OP(cb,9a) { D = res(3, D);          } EOP /* RES  3,D         */
OP(cb,9b) { E = res(3, E);          } EOP /* RES  3,E         */
OP(cb,9c) { H = res(3, H);          } EOP /* RES  3,H         */
OP(cb,9d) { L = res(3, L);          } EOP /* RES  3,L         */
OP(cb,9e) { wm(HL, res(3, rm_reg(HL))); } EOP /* RES  3,(HL)      */
OP(cb,9f) { A = res(3, A);          } EOP /* RES  3,A         */

OP(cb,a0) { B = res(4, B);          } EOP /* RES  4,B         */
OP(cb,a1) { C = res(4, C);          } EOP /* RES  4,C         */
OP(cb,a2) { D = res(4, D);          } EOP /* RES  4,D         */
OP(cb,a3) { E = res(4, E);          } EOP /* RES  4,E         */
OP(cb,a4) { H = res(4, H);          } EOP /* RES  4,H         */
OP(cb,a5) { L = res(4, L);          } EOP /* RES  4,L         */
OP(cb,a6) {wm(HL, res(4, rm_reg(HL))); } EOP /* RES  4,(HL)      */
OP(cb,a7) { A = res(4, A);          } EOP /* RES  4,A         */

OP(cb,a8) { B = res(5, B);          } EOP /* RES  5,B         */
OP(cb,a9) { C = res(5, C);          } EOP /* RES  5,C         */
OP(cb,aa) { D = res(5, D);          } EOP /* RES  5,D         */
OP(cb,ab) { E = res(5, E);          } EOP /* RES  5,E         */
OP(cb,ac) { H = res(5, H);          } EOP /* RES  5,H         */
OP(cb,ad) { L = res(5, L);          } EOP /* RES  5,L         */
OP(cb,ae) { wm(HL, res(5, rm_reg(HL))); } EOP /* RES  5,(HL)      */
OP(cb,af) { A = res(5, A);          } EOP /* RES  5,A         */

OP(cb,b0) { B = res(6, B);          } EOP /* RES  6,B         */
OP(cb,b1) { C = res(6, C);          } EOP /* RES  6,C         */
OP(cb,b2) { D = res(6, D);          } EOP /* RES  6,D         */
OP(cb,b3) { E = res(6, E);          } EOP /* RES  6,E         */
OP(cb,b4) { H = res(6, H);          } EOP /* RES  6,H         */
OP(cb,b5) { L = res(6, L);          } EOP /* RES  6,L         */
OP(cb,b6) { wm(HL, res(6, rm_reg(HL))); } EOP /* RES  6,(HL)      */
OP(cb,b7) { A = res(6, A);          } EOP /* RES  6,A         */

OP(cb,b8) { B = res(7, B);          } EOP /* RES  7,B         */
OP(cb,b9) { C = res(7, C);          } EOP /* RES  7,C         */
OP(cb,ba) { D = res(7, D);          } EOP /* RES  7,D         */
OP(cb,bb) { E = res(7, E);          } EOP /* RES  7,E         */
OP(cb,bc) { H = res(7, H);          } EOP /* RES  7,H         */
OP(cb,bd) { L = res(7, L);          } EOP /* RES  7,L         */
OP(cb,be) { wm(HL, res(7, rm_reg(HL))); } EOP /* RES  7,(HL)      */
OP(cb,bf) { A = res(7, A);          } EOP /* RES  7,A         */

OP(cb,c0) { B = set(0, B);          } EOP /* SET  0,B         */
OP(cb,c1) { C = set(0, C);          } EOP /* SET  0,C         */
OP(cb,c2) { D = set(0, D);          } EOP /* SET  0,D         */
OP(cb,c3) { E = set(0, E);          } EOP /* SET  0,E         */
OP(cb,c4) { H = set(0, H);          } EOP /* SET  0,H         */
OP(cb,c5) { L = set(0, L);          } EOP /* SET  0,L         */
OP(cb,c6) { wm(HL, set(0, rm_reg(HL))); } EOP /* SET  0,(HL)      */
OP(cb,c7) { A = set(0, A);          } EOP /* SET  0,A         */

OP(cb,c8) { B = set(1, B);          } EOP /* SET  1,B         */
OP(cb,c9) { C = set(1, C);          } EOP /* SET  1,C         */
OP(cb,ca) { D = set(1, D);          } EOP /* SET  1,D         */
OP(cb,cb) { E = set(1, E);          } EOP /* SET  1,E         */
OP(cb,cc) { H = set(1, H);          } EOP /* SET  1,H         */
OP(cb,cd) { L = set(1, L);          } EOP /* SET  1,L         */
OP(cb,ce) { wm(HL, set(1, rm_reg(HL))); } EOP /* SET  1,(HL)      */
OP(cb,cf) { A = set(1, A);          } EOP /* SET  1,A         */

OP(cb,d0) { B = set(2, B);          } EOP /* SET  2,B         */
OP(cb,d1) { C = set(2, C);          } EOP /* SET  2,C         */
OP(cb,d2) { D = set(2, D);          } EOP /* SET  2,D         */
OP(cb,d3) { E = set(2, E);          } EOP /* SET  2,E         */
OP(cb,d4) { H = set(2, H);          } EOP /* SET  2,H         */
OP(cb,d5) { L = set(2, L);          } EOP /* SET  2,L         */
OP(cb,d6) { wm(HL, set(2, rm_reg(HL))); } EOP /* SET  2,(HL)      */
OP(cb,d7) { A = set(2, A);          } EOP /* SET  2,A         */

OP(cb,d8) { B = set(3, B);          } EOP /* SET  3,B         */
OP(cb,d9) { C = set(3, C);          } EOP /* SET  3,C         */
OP(cb,da) { D = set(3, D);          } EOP /* SET  3,D         */
OP(cb,db) { E = set(3, E);          } EOP /* SET  3,E         */
OP(cb,dc) { H = set(3, H);          } EOP /* SET  3,H         */
OP(cb,dd) { L = set(3, L);          } EOP /* SET  3,L         */
OP(cb,de) { wm(HL, set(3, rm_reg(HL))); } EOP /* SET  3,(HL)      */
OP(cb,df) { A = set(3, A);          } EOP /* SET  3,A         */

OP(cb,e0) { B = set(4, B);          } EOP /* SET  4,B         */
OP(cb,e1) { C = set(4, C);          } EOP /* SET  4,C         */
OP(cb,e2) { D = set(4, D);          } EOP /* SET  4,D         */
OP(cb,e3) { E = set(4, E);          } EOP /* SET  4,E         */
OP(cb,e4) { H = set(4, H);          } EOP /* SET  4,H         */
OP(cb,e5) { L = set(4, L);          } EOP /* SET  4,L         */
OP(cb,e6) { wm(HL, set(4, rm_reg(HL))); } EOP /* SET  4,(HL)      */
OP(cb,e7) { A = set(4, A);          } EOP /* SET  4,A         */

OP(cb,e8) { B = set(5, B);          } EOP /* SET  5,B         */
OP(cb,e9) { C = set(5, C);          } EOP /* SET  5,C         */
OP(cb,ea) { D = set(5, D);          } EOP /* SET  5,D         */
OP(cb,eb) { E = set(5, E);          } EOP /* SET  5,E         */
OP(cb,ec) { H = set(5, H);          } EOP /* SET  5,H         */
OP(cb,ed) { L = set(5, L);          } EOP /* SET  5,L         */
OP(cb,ee) { wm(HL, set(5, rm_reg(HL))); } EOP /* SET  5,(HL)      */
OP(cb,ef) { A = set(5, A);          } EOP /* SET  5,A         */

OP(cb,f0) { B = set(6, B);          } EOP /* SET  6,B         */
OP(cb,f1) { C = set(6, C);          } EOP /* SET  6,C         */
OP(cb,f2) { D = set(6, D);          } EOP /* SET  6,D         */
OP(cb,f3) { E = set(6, E);          } EOP /* SET  6,E         */
OP(cb,f4) { H = set(6, H);          } EOP /* SET  6,H         */
OP(cb,f5) { L = set(6, L);          } EOP /* SET  6,L         */
OP(cb,f6) { wm(HL, set(6, rm_reg(HL))); } EOP /* SET  6,(HL)      */
OP(cb,f7) { A = set(6, A);          } EOP /* SET  6,A         */

OP(cb,f8) { B = set(7, B);          } EOP /* SET  7,B         */
OP(cb,f9) { C = set(7, C);          } EOP /* SET  7,C         */
OP(cb,fa) { D = set(7, D);          } EOP /* SET  7,D         */
OP(cb,fb) { E = set(7, E);          } EOP /* SET  7,E         */
OP(cb,fc) { H = set(7, H);          } EOP /* SET  7,H         */
OP(cb,fd) { L = set(7, L);          } EOP /* SET  7,L         */
OP(cb,fe) { wm(HL, set(7, rm_reg(HL))); } EOP /* SET  7,(HL)      */
OP(cb,ff) { A = set(7, A);          } EOP /* SET  7,A         */


/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit EOPrations with (IX+o)
**********************************************************/
OP(xycb,00) { B = rlc(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* RLC  B=(XY+o)    */
OP(xycb,01) { C = rlc(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* RLC  C=(XY+o)    */
OP(xycb,02) { D = rlc(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* RLC  D=(XY+o)    */
OP(xycb,03) { E = rlc(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* RLC  E=(XY+o)    */
OP(xycb,04) { H = rlc(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* RLC  H=(XY+o)    */
OP(xycb,05) { L = rlc(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* RLC  L=(XY+o)    */
OP(xycb,06) { wm(m_ea, rlc(rm_reg(m_ea)));           } EOP /* RLC  (XY+o)      */
OP(xycb,07) { A = rlc(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* RLC  A=(XY+o)    */

OP(xycb,08) { B = rrc(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* RRC  B=(XY+o)    */
OP(xycb,09) { C = rrc(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* RRC  C=(XY+o)    */
OP(xycb,0a) { D = rrc(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* RRC  D=(XY+o)    */
OP(xycb,0b) { E = rrc(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* RRC  E=(XY+o)    */
OP(xycb,0c) { H = rrc(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* RRC  H=(XY+o)    */
OP(xycb,0d) { L = rrc(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* RRC  L=(XY+o)    */
OP(xycb,0e) { wm(m_ea,rrc(rm_reg(m_ea)));            } EOP /* RRC  (XY+o)      */
OP(xycb,0f) { A = rrc(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* RRC  A=(XY+o)    */

OP(xycb,10) { B = rl(rm_reg(m_ea)); wm(m_ea, B);     } EOP /* RL   B=(XY+o)    */
OP(xycb,11) { C = rl(rm_reg(m_ea)); wm(m_ea, C);     } EOP /* RL   C=(XY+o)    */
OP(xycb,12) { D = rl(rm_reg(m_ea)); wm(m_ea, D);     } EOP /* RL   D=(XY+o)    */
OP(xycb,13) { E = rl(rm_reg(m_ea)); wm(m_ea, E);     } EOP /* RL   E=(XY+o)    */
OP(xycb,14) { H = rl(rm_reg(m_ea)); wm(m_ea, H);     } EOP /* RL   H=(XY+o)    */
OP(xycb,15) { L = rl(rm_reg(m_ea)); wm(m_ea, L);     } EOP /* RL   L=(XY+o)    */
OP(xycb,16) { wm(m_ea,rl(rm_reg(m_ea)));             } EOP /* RL   (XY+o)      */
OP(xycb,17) { A = rl(rm_reg(m_ea)); wm(m_ea, A);     } EOP /* RL   A=(XY+o)    */

OP(xycb,18) { B = rr(rm_reg(m_ea)); wm(m_ea, B);     } EOP /* RR   B=(XY+o)    */
OP(xycb,19) { C = rr(rm_reg(m_ea)); wm(m_ea, C);     } EOP /* RR   C=(XY+o)    */
OP(xycb,1a) { D = rr(rm_reg(m_ea)); wm(m_ea, D);     } EOP /* RR   D=(XY+o)    */
OP(xycb,1b) { E = rr(rm_reg(m_ea)); wm(m_ea, E);     } EOP /* RR   E=(XY+o)    */
OP(xycb,1c) { H = rr(rm_reg(m_ea)); wm(m_ea, H);     } EOP /* RR   H=(XY+o)    */
OP(xycb,1d) { L = rr(rm_reg(m_ea)); wm(m_ea, L);     } EOP /* RR   L=(XY+o)    */
OP(xycb,1e) { wm(m_ea, rr(rm_reg(m_ea)));            } EOP /* RR   (XY+o)      */
OP(xycb,1f) { A = rr(rm_reg(m_ea)); wm(m_ea, A);     } EOP /* RR   A=(XY+o)    */

OP(xycb,20) { B = sla(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* SLA  B=(XY+o)    */
OP(xycb,21) { C = sla(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* SLA  C=(XY+o)    */
OP(xycb,22) { D = sla(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* SLA  D=(XY+o)    */
OP(xycb,23) { E = sla(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* SLA  E=(XY+o)    */
OP(xycb,24) { H = sla(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* SLA  H=(XY+o)    */
OP(xycb,25) { L = sla(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* SLA  L=(XY+o)    */
OP(xycb,26) { wm(m_ea, sla(rm_reg(m_ea)));           } EOP /* SLA  (XY+o)      */
OP(xycb,27) { A = sla(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* SLA  A=(XY+o)    */

OP(xycb,28) { B = sra(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* SRA  B=(XY+o)    */
OP(xycb,29) { C = sra(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* SRA  C=(XY+o)    */
OP(xycb,2a) { D = sra(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* SRA  D=(XY+o)    */
OP(xycb,2b) { E = sra(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* SRA  E=(XY+o)    */
OP(xycb,2c) { H = sra(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* SRA  H=(XY+o)    */
OP(xycb,2d) { L = sra(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* SRA  L=(XY+o)    */
OP(xycb,2e) { wm(m_ea, sra(rm_reg(m_ea)));           } EOP /* SRA  (XY+o)      */
OP(xycb,2f) { A = sra(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* SRA  A=(XY+o)    */

OP(xycb,30) { B = sll(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* SLL  B=(XY+o)    */
OP(xycb,31) { C = sll(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* SLL  C=(XY+o)    */
OP(xycb,32) { D = sll(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* SLL  D=(XY+o)    */
OP(xycb,33) { E = sll(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* SLL  E=(XY+o)    */
OP(xycb,34) { H = sll(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* SLL  H=(XY+o)    */
OP(xycb,35) { L = sll(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* SLL  L=(XY+o)    */
OP(xycb,36) { wm(m_ea, sll(rm_reg(m_ea)));           } EOP /* SLL  (XY+o)      */
OP(xycb,37) { A = sll(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* SLL  A=(XY+o)    */

OP(xycb,38) { B = srl(rm_reg(m_ea)); wm(m_ea, B);    } EOP /* SRL  B=(XY+o)    */
OP(xycb,39) { C = srl(rm_reg(m_ea)); wm(m_ea, C);    } EOP /* SRL  C=(XY+o)    */
OP(xycb,3a) { D = srl(rm_reg(m_ea)); wm(m_ea, D);    } EOP /* SRL  D=(XY+o)    */
OP(xycb,3b) { E = srl(rm_reg(m_ea)); wm(m_ea, E);    } EOP /* SRL  E=(XY+o)    */
OP(xycb,3c) { H = srl(rm_reg(m_ea)); wm(m_ea, H);    } EOP /* SRL  H=(XY+o)    */
OP(xycb,3d) { L = srl(rm_reg(m_ea)); wm(m_ea, L);    } EOP /* SRL  L=(XY+o)    */
OP(xycb,3e) { wm(m_ea, srl(rm_reg(m_ea)));           } EOP /* SRL  (XY+o)      */
OP(xycb,3f) { A = srl(rm_reg(m_ea)); wm(m_ea, A);    } EOP /* SRL  A=(XY+o)    */

OPR(xycb,40) xycb_46;                                  /* BIT  0,(XY+o)    */
OPR(xycb,41) xycb_46;                                  /* BIT  0,(XY+o)    */
OPR(xycb,42) xycb_46;                                  /* BIT  0,(XY+o)    */
OPR(xycb,43) xycb_46;                                  /* BIT  0,(XY+o)    */
OPR(xycb,44) xycb_46;                                  /* BIT  0,(XY+o)    */
OPR(xycb,45) xycb_46;                                  /* BIT  0,(XY+o)    */
OP(xycb,46) { bit_xy(0, rm_reg(m_ea));           } EOP /* BIT  0,(XY+o)    */
OPR(xycb,47) xycb_46;                                  /* BIT  0,(XY+o)    */

OPR(xycb,48) xycb_4e;                                  /* BIT  1,(XY+o)    */
OPR(xycb,49) xycb_4e;                                  /* BIT  1,(XY+o)    */
OPR(xycb,4a) xycb_4e;                                  /* BIT  1,(XY+o)    */
OPR(xycb,4b) xycb_4e;                                  /* BIT  1,(XY+o)    */
OPR(xycb,4c) xycb_4e;                                  /* BIT  1,(XY+o)    */
OPR(xycb,4d) xycb_4e;                                  /* BIT  1,(XY+o)    */
OP(xycb,4e) { bit_xy(1, rm_reg(m_ea));           } EOP /* BIT  1,(XY+o)    */
OPR(xycb,4f) xycb_4e;                                  /* BIT  1,(XY+o)    */

OPR(xycb,50) xycb_56;                                  /* BIT  2,(XY+o)    */
OPR(xycb,51) xycb_56;                                  /* BIT  2,(XY+o)    */
OPR(xycb,52) xycb_56;                                  /* BIT  2,(XY+o)    */
OPR(xycb,53) xycb_56;                                  /* BIT  2,(XY+o)    */
OPR(xycb,54) xycb_56;                                  /* BIT  2,(XY+o)    */
OPR(xycb,55) xycb_56;                                  /* BIT  2,(XY+o)    */
OP(xycb,56) { bit_xy(2, rm_reg(m_ea));           } EOP /* BIT  2,(XY+o)    */
OPR(xycb,57) xycb_56;                                  /* BIT  2,(XY+o)    */

OPR(xycb,58) xycb_5e;                                  /* BIT  3,(XY+o)    */
OPR(xycb,59) xycb_5e;                                  /* BIT  3,(XY+o)    */
OPR(xycb,5a) xycb_5e;                                  /* BIT  3,(XY+o)    */
OPR(xycb,5b) xycb_5e;                                  /* BIT  3,(XY+o)    */
OPR(xycb,5c) xycb_5e;                                  /* BIT  3,(XY+o)    */
OPR(xycb,5d) xycb_5e;                                  /* BIT  3,(XY+o)    */
OP(xycb,5e) { bit_xy(3, rm_reg(m_ea));           } EOP /* BIT  3,(XY+o)    */
OPR(xycb,5f) xycb_5e;                                  /* BIT  3,(XY+o)    */

OPR(xycb,60) xycb_66;                                  /* BIT  4,(XY+o)    */
OPR(xycb,61) xycb_66;                                  /* BIT  4,(XY+o)    */
OPR(xycb,62) xycb_66;                                  /* BIT  4,(XY+o)    */
OPR(xycb,63) xycb_66;                                  /* BIT  4,(XY+o)    */
OPR(xycb,64) xycb_66;                                  /* BIT  4,(XY+o)    */
OPR(xycb,65) xycb_66;                                  /* BIT  4,(XY+o)    */
OP(xycb,66) { bit_xy(4, rm_reg(m_ea));           } EOP /* BIT  4,(XY+o)    */
OPR(xycb,67) xycb_66;                                  /* BIT  4,(XY+o)    */

OPR(xycb,68) xycb_6e;                                  /* BIT  5,(XY+o)    */
OPR(xycb,69) xycb_6e;                                  /* BIT  5,(XY+o)    */
OPR(xycb,6a) xycb_6e;                                  /* BIT  5,(XY+o)    */
OPR(xycb,6b) xycb_6e;                                  /* BIT  5,(XY+o)    */
OPR(xycb,6c) xycb_6e;                                  /* BIT  5,(XY+o)    */
OPR(xycb,6d) xycb_6e;                                  /* BIT  5,(XY+o)    */
OP(xycb,6e) { bit_xy(5, rm_reg(m_ea));           } EOP /* BIT  5,(XY+o)    */
OPR(xycb,6f) xycb_6e;                                  /* BIT  5,(XY+o)    */

OPR(xycb,70) xycb_76;                                  /* BIT  6,(XY+o)    */
OPR(xycb,71) xycb_76;                                  /* BIT  6,(XY+o)    */
OPR(xycb,72) xycb_76;                                  /* BIT  6,(XY+o)    */
OPR(xycb,73) xycb_76;                                  /* BIT  6,(XY+o)    */
OPR(xycb,74) xycb_76;                                  /* BIT  6,(XY+o)    */
OPR(xycb,75) xycb_76;                                  /* BIT  6,(XY+o)    */
OP(xycb,76) { bit_xy(6, rm_reg(m_ea));           } EOP /* BIT  6,(XY+o)    */
OPR(xycb,77) xycb_76;                                  /* BIT  6,(XY+o)    */

OPR(xycb,78) xycb_7e;                                  /* BIT  7,(XY+o)    */
OPR(xycb,79) xycb_7e;                                  /* BIT  7,(XY+o)    */
OPR(xycb,7a) xycb_7e;                                  /* BIT  7,(XY+o)    */
OPR(xycb,7b) xycb_7e;                                  /* BIT  7,(XY+o)    */
OPR(xycb,7c) xycb_7e;                                  /* BIT  7,(XY+o)    */
OPR(xycb,7d) xycb_7e;                                  /* BIT  7,(XY+o)    */
OP(xycb,7e) { bit_xy(7, rm_reg(m_ea));           } EOP /* BIT  7,(XY+o)    */
OPR(xycb,7f) xycb_7e;                                  /* BIT  7,(XY+o)    */

OP(xycb,80) { B = res(0, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  0,B=(XY+o)  */
OP(xycb,81) { C = res(0, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  0,C=(XY+o)  */
OP(xycb,82) { D = res(0, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  0,D=(XY+o)  */
OP(xycb,83) { E = res(0, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  0,E=(XY+o)  */
OP(xycb,84) { H = res(0, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  0,H=(XY+o)  */
OP(xycb,85) { L = res(0, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  0,L=(XY+o)  */
OP(xycb,86) { wm(m_ea, res(0, rm_reg(m_ea)));        } EOP /* RES  0,(XY+o)    */
OP(xycb,87) { A = res(0, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  0,A=(XY+o)  */

OP(xycb,88) { B = res(1, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  1,B=(XY+o)  */
OP(xycb,89) { C = res(1, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  1,C=(XY+o)  */
OP(xycb,8a) { D = res(1, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  1,D=(XY+o)  */
OP(xycb,8b) { E = res(1, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  1,E=(XY+o)  */
OP(xycb,8c) { H = res(1, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  1,H=(XY+o)  */
OP(xycb,8d) { L = res(1, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  1,L=(XY+o)  */
OP(xycb,8e) { wm(m_ea, res(1, rm_reg(m_ea)));        } EOP /* RES  1,(XY+o)    */
OP(xycb,8f) { A = res(1, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  1,A=(XY+o)  */

OP(xycb,90) { B = res(2, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  2,B=(XY+o)  */
OP(xycb,91) { C = res(2, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  2,C=(XY+o)  */
OP(xycb,92) { D = res(2, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  2,D=(XY+o)  */
OP(xycb,93) { E = res(2, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  2,E=(XY+o)  */
OP(xycb,94) { H = res(2, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  2,H=(XY+o)  */
OP(xycb,95) { L = res(2, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  2,L=(XY+o)  */
OP(xycb,96) { wm(m_ea, res(2, rm_reg(m_ea)));        } EOP /* RES  2,(XY+o)    */
OP(xycb,97) { A = res(2, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  2,A=(XY+o)  */

OP(xycb,98) { B = res(3, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  3,B=(XY+o)  */
OP(xycb,99) { C = res(3, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  3,C=(XY+o)  */
OP(xycb,9a) { D = res(3, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  3,D=(XY+o)  */
OP(xycb,9b) { E = res(3, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  3,E=(XY+o)  */
OP(xycb,9c) { H = res(3, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  3,H=(XY+o)  */
OP(xycb,9d) { L = res(3, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  3,L=(XY+o)  */
OP(xycb,9e) { wm(m_ea, res(3, rm_reg(m_ea)));        } EOP /* RES  3,(XY+o)    */
OP(xycb,9f) { A = res(3, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  3,A=(XY+o)  */

OP(xycb,a0) { B = res(4, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  4,B=(XY+o)  */
OP(xycb,a1) { C = res(4, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  4,C=(XY+o)  */
OP(xycb,a2) { D = res(4, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  4,D=(XY+o)  */
OP(xycb,a3) { E = res(4, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  4,E=(XY+o)  */
OP(xycb,a4) { H = res(4, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  4,H=(XY+o)  */
OP(xycb,a5) { L = res(4, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  4,L=(XY+o)  */
OP(xycb,a6) { wm(m_ea, res(4, rm_reg(m_ea)));        } EOP /* RES  4,(XY+o)    */
OP(xycb,a7) { A = res(4, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  4,A=(XY+o)  */

OP(xycb,a8) { B = res(5, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  5,B=(XY+o)  */
OP(xycb,a9) { C = res(5, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  5,C=(XY+o)  */
OP(xycb,aa) { D = res(5, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  5,D=(XY+o)  */
OP(xycb,ab) { E = res(5, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  5,E=(XY+o)  */
OP(xycb,ac) { H = res(5, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  5,H=(XY+o)  */
OP(xycb,ad) { L = res(5, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  5,L=(XY+o)  */
OP(xycb,ae) { wm(m_ea, res(5, rm_reg(m_ea)));        } EOP /* RES  5,(XY+o)    */
OP(xycb,af) { A = res(5, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  5,A=(XY+o)  */

OP(xycb,b0) { B = res(6, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  6,B=(XY+o)  */
OP(xycb,b1) { C = res(6, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  6,C=(XY+o)  */
OP(xycb,b2) { D = res(6, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  6,D=(XY+o)  */
OP(xycb,b3) { E = res(6, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  6,E=(XY+o)  */
OP(xycb,b4) { H = res(6, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  6,H=(XY+o)  */
OP(xycb,b5) { L = res(6, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  6,L=(XY+o)  */
OP(xycb,b6) { wm(m_ea, res(6, rm_reg(m_ea)));        } EOP /* RES  6,(XY+o)    */
OP(xycb,b7) { A = res(6, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  6,A=(XY+o)  */

OP(xycb,b8) { B = res(7, rm_reg(m_ea)); wm(m_ea, B); } EOP /* RES  7,B=(XY+o)  */
OP(xycb,b9) { C = res(7, rm_reg(m_ea)); wm(m_ea, C); } EOP /* RES  7,C=(XY+o)  */
OP(xycb,ba) { D = res(7, rm_reg(m_ea)); wm(m_ea, D); } EOP /* RES  7,D=(XY+o)  */
OP(xycb,bb) { E = res(7, rm_reg(m_ea)); wm(m_ea, E); } EOP /* RES  7,E=(XY+o)  */
OP(xycb,bc) { H = res(7, rm_reg(m_ea)); wm(m_ea, H); } EOP /* RES  7,H=(XY+o)  */
OP(xycb,bd) { L = res(7, rm_reg(m_ea)); wm(m_ea, L); } EOP /* RES  7,L=(XY+o)  */
OP(xycb,be) { wm(m_ea, res(7, rm_reg(m_ea)));        } EOP /* RES  7,(XY+o)    */
OP(xycb,bf) { A = res(7, rm_reg(m_ea)); wm(m_ea, A); } EOP /* RES  7,A=(XY+o)  */

OP(xycb,c0) { B = set(0, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  0,B=(XY+o)  */
OP(xycb,c1) { C = set(0, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  0,C=(XY+o)  */
OP(xycb,c2) { D = set(0, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  0,D=(XY+o)  */
OP(xycb,c3) { E = set(0, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  0,E=(XY+o)  */
OP(xycb,c4) { H = set(0, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  0,H=(XY+o)  */
OP(xycb,c5) { L = set(0, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  0,L=(XY+o)  */
OP(xycb,c6) { wm(m_ea, set(0, rm_reg(m_ea)));        } EOP /* SET  0,(XY+o)    */
OP(xycb,c7) { A = set(0, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  0,A=(XY+o)  */

OP(xycb,c8) { B = set(1, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  1,B=(XY+o)  */
OP(xycb,c9) { C = set(1, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  1,C=(XY+o)  */
OP(xycb,ca) { D = set(1, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  1,D=(XY+o)  */
OP(xycb,cb) { E = set(1, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  1,E=(XY+o)  */
OP(xycb,cc) { H = set(1, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  1,H=(XY+o)  */
OP(xycb,cd) { L = set(1, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  1,L=(XY+o)  */
OP(xycb,ce) { wm(m_ea, set(1, rm_reg(m_ea)));        } EOP /* SET  1,(XY+o)    */
OP(xycb,cf) { A = set(1, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  1,A=(XY+o)  */

OP(xycb,d0) { B = set(2, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  2,B=(XY+o)  */
OP(xycb,d1) { C = set(2, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  2,C=(XY+o)  */
OP(xycb,d2) { D = set(2, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  2,D=(XY+o)  */
OP(xycb,d3) { E = set(2, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  2,E=(XY+o)  */
OP(xycb,d4) { H = set(2, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  2,H=(XY+o)  */
OP(xycb,d5) { L = set(2, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  2,L=(XY+o)  */
OP(xycb,d6) { wm(m_ea, set(2, rm_reg(m_ea)));        } EOP /* SET  2,(XY+o)    */
OP(xycb,d7) { A = set(2, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  2,A=(XY+o)  */

OP(xycb,d8) { B = set(3, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  3,B=(XY+o)  */
OP(xycb,d9) { C = set(3, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  3,C=(XY+o)  */
OP(xycb,da) { D = set(3, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  3,D=(XY+o)  */
OP(xycb,db) { E = set(3, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  3,E=(XY+o)  */
OP(xycb,dc) { H = set(3, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  3,H=(XY+o)  */
OP(xycb,dd) { L = set(3, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  3,L=(XY+o)  */
OP(xycb,de) { wm(m_ea, set(3, rm_reg(m_ea)));        } EOP /* SET  3,(XY+o)    */
OP(xycb,df) { A = set(3, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  3,A=(XY+o)  */

OP(xycb,e0) { B = set(4, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  4,B=(XY+o)  */
OP(xycb,e1) { C = set(4, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  4,C=(XY+o)  */
OP(xycb,e2) { D = set(4, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  4,D=(XY+o)  */
OP(xycb,e3) { E = set(4, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  4,E=(XY+o)  */
OP(xycb,e4) { H = set(4, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  4,H=(XY+o)  */
OP(xycb,e5) { L = set(4, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  4,L=(XY+o)  */
OP(xycb,e6) { wm(m_ea, set(4, rm_reg(m_ea)));        } EOP /* SET  4,(XY+o)    */
OP(xycb,e7) { A = set(4, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  4,A=(XY+o)  */

OP(xycb,e8) { B = set(5, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  5,B=(XY+o)  */
OP(xycb,e9) { C = set(5, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  5,C=(XY+o)  */
OP(xycb,ea) { D = set(5, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  5,D=(XY+o)  */
OP(xycb,eb) { E = set(5, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  5,E=(XY+o)  */
OP(xycb,ec) { H = set(5, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  5,H=(XY+o)  */
OP(xycb,ed) { L = set(5, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  5,L=(XY+o)  */
OP(xycb,ee) { wm(m_ea, set(5, rm_reg(m_ea)));        } EOP /* SET  5,(XY+o)    */
OP(xycb,ef) { A = set(5, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  5,A=(XY+o)  */

OP(xycb,f0) { B = set(6, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  6,B=(XY+o)  */
OP(xycb,f1) { C = set(6, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  6,C=(XY+o)  */
OP(xycb,f2) { D = set(6, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  6,D=(XY+o)  */
OP(xycb,f3) { E = set(6, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  6,E=(XY+o)  */
OP(xycb,f4) { H = set(6, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  6,H=(XY+o)  */
OP(xycb,f5) { L = set(6, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  6,L=(XY+o)  */
OP(xycb,f6) { wm(m_ea, set(6, rm_reg(m_ea)));        } EOP /* SET  6,(XY+o)    */
OP(xycb,f7) { A = set(6, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  6,A=(XY+o)  */

OP(xycb,f8) { B = set(7, rm_reg(m_ea)); wm(m_ea, B); } EOP /* SET  7,B=(XY+o)  */
OP(xycb,f9) { C = set(7, rm_reg(m_ea)); wm(m_ea, C); } EOP /* SET  7,C=(XY+o)  */
OP(xycb,fa) { D = set(7, rm_reg(m_ea)); wm(m_ea, D); } EOP /* SET  7,D=(XY+o)  */
OP(xycb,fb) { E = set(7, rm_reg(m_ea)); wm(m_ea, E); } EOP /* SET  7,E=(XY+o)  */
OP(xycb,fc) { H = set(7, rm_reg(m_ea)); wm(m_ea, H); } EOP /* SET  7,H=(XY+o)  */
OP(xycb,fd) { L = set(7, rm_reg(m_ea)); wm(m_ea, L); } EOP /* SET  7,L=(XY+o)  */
OP(xycb,fe) { wm(m_ea, set(7, rm_reg(m_ea)));        } EOP /* SET  7,(XY+o)    */
OP(xycb,ff) { A = set(7, rm_reg(m_ea)); wm(m_ea, A); } EOP /* SET  7,A=(XY+o)  */

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
OPR(dd,00) illegal_1(op_00);                                       /* DB   DD          */
OPR(dd,01) illegal_1(op_01);                                       /* DB   DD          */
OPR(dd,02) illegal_1(op_02);                                       /* DB   DD          */
OPR(dd,03) illegal_1(op_03);                                       /* DB   DD          */
OPR(dd,04) illegal_1(op_04);                                       /* DB   DD          */
OPR(dd,05) illegal_1(op_05);                                       /* DB   DD          */
OPR(dd,06) illegal_1(op_06);                                       /* DB   DD          */
OPR(dd,07) illegal_1(op_07);                                       /* DB   DD          */

OPR(dd,08) illegal_1(op_08);                                       /* DB   DD          */
OP(dd,09) { add16(m_ix, m_bc);                               } EOP /* ADD  IX,BC       */
OPR(dd,0a) illegal_1(op_0a);                                       /* DB   DD          */
OPR(dd,0b) illegal_1(op_0b);                                       /* DB   DD          */
OPR(dd,0c) illegal_1(op_0c);                                       /* DB   DD          */
OPR(dd,0d) illegal_1(op_0d);                                       /* DB   DD          */
OPR(dd,0e) illegal_1(op_0e);                                       /* DB   DD          */
OPR(dd,0f) illegal_1(op_0f);                                       /* DB   DD          */

OPR(dd,10) illegal_1(op_10);                                       /* DB   DD          */
OPR(dd,11) illegal_1(op_11);                                       /* DB   DD          */
OPR(dd,12) illegal_1(op_12);                                       /* DB   DD          */
OPR(dd,13) illegal_1(op_13);                                       /* DB   DD          */
OPR(dd,14) illegal_1(op_14);                                       /* DB   DD          */
OPR(dd,15) illegal_1(op_15);                                       /* DB   DD          */
OPR(dd,16) illegal_1(op_16);                                       /* DB   DD          */
OPR(dd,17) illegal_1(op_17);                                       /* DB   DD          */

OPR(dd,18) illegal_1(op_18);                                       /* DB   DD          */
OP(dd,19) { add16(m_ix, m_de);                               } EOP /* ADD  IX,DE       */
OPR(dd,1a) illegal_1(op_1a);                                       /* DB   DD          */
OPR(dd,1b) illegal_1(op_1b);                                       /* DB   DD          */
OPR(dd,1c) illegal_1(op_1c);                                       /* DB   DD          */
OPR(dd,1d) illegal_1(op_1d);                                       /* DB   DD          */
OPR(dd,1e) illegal_1(op_1e);                                       /* DB   DD          */
OPR(dd,1f) illegal_1(op_1f);                                       /* DB   DD          */

OPR(dd,20) illegal_1(op_20);                                       /* DB   DD          */
OP(dd,21) { IX = arg16();                                    } EOP /* LD   IX,w        */
OP(dd,22) { m_ea = arg16(); wm16(m_ea, m_ix); WZ = m_ea + 1; } EOP /* LD   (w),IX      */
OP(dd,23) { nomreq_ir(2); IX++;                              } EOP /* INC  IX          */
OP(dd,24) { HX = inc(HX);                                    } EOP /* INC  HX          */
OP(dd,25) { HX = dec(HX);                                    } EOP /* DEC  HX          */
OP(dd,26) { HX = arg();                                      } EOP /* LD   HX,n        */
OPR(dd,27) illegal_1(op_27);                                       /* DB   DD          */

OPR(dd,28) illegal_1(op_28);                                       /* DB   DD          */
OP(dd,29) { add16(m_ix, m_ix);                               } EOP /* ADD  IX,IX       */
OP(dd,2a) { m_ea = arg16(); rm16(m_ea, m_ix); WZ = m_ea + 1; } EOP /* LD   IX,(w)      */
OP(dd,2b) { nomreq_ir(2); IX--;                              } EOP /* DEC  IX          */
OP(dd,2c) { LX = inc(LX);                                    } EOP /* INC  LX          */
OP(dd,2d) { LX = dec(LX);                                    } EOP /* DEC  LX          */
OP(dd,2e) { LX = arg();                                      } EOP /* LD   LX,n        */
OPR(dd,2f) illegal_1(op_2f);                                       /* DB   DD          */

OPR(dd,30) illegal_1(op_30);                                       /* DB   DD          */
OPR(dd,31) illegal_1(op_31);                                       /* DB   DD          */
OPR(dd,32) illegal_1(op_32);                                       /* DB   DD          */
OPR(dd,33) illegal_1(op_33);                                       /* DB   DD          */
OP(dd,34) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, inc(rm_reg(m_ea))); } EOP /* INC  (IX+o)      */
OP(dd,35) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, dec(rm_reg(m_ea))); } EOP /* DEC  (IX+o)      */
OP(dd,36) { eax(); u8 a = arg(); nomreq_addr(PCD-1, 2); wm(m_ea, a);               } EOP /* LD   (IX+o),n    */
OPR(dd,37) illegal_1(op_37);                                       /* DB   DD          */

OPR(dd,38) illegal_1(op_38);                                       /* DB   DD          */
OP(dd,39) { add16(m_ix, m_sp);                               } EOP /* ADD  IX,SP       */
OPR(dd,3a) illegal_1(op_3a);                                       /* DB   DD          */
OPR(dd,3b) illegal_1(op_3b);                                       /* DB   DD          */
OPR(dd,3c) illegal_1(op_3c);                                       /* DB   DD          */
OPR(dd,3d) illegal_1(op_3d);                                       /* DB   DD          */
OPR(dd,3e) illegal_1(op_3e);                                       /* DB   DD          */
OPR(dd,3f) illegal_1(op_3f);                                       /* DB   DD          */

OPR(dd,40) illegal_1(op_40);                                       /* DB   DD          */
OPR(dd,41) illegal_1(op_41);                                       /* DB   DD          */
OPR(dd,42) illegal_1(op_42);                                       /* DB   DD          */
OPR(dd,43) illegal_1(op_43);                                       /* DB   DD          */
OP(dd,44) { B = HX;                                          } EOP /* LD   B,HX        */
OP(dd,45) { B = LX;                                          } EOP /* LD   B,LX        */
OP(dd,46) { eax(); nomreq_addr(PCD-1, 5); B = rm(m_ea);      } EOP /* LD   B,(IX+o)    */
OPR(dd,47) illegal_1(op_47);                                       /* DB   DD          */

OPR(dd,48) illegal_1(op_48);                                       /* DB   DD          */
OPR(dd,49) illegal_1(op_49);                                       /* DB   DD          */
OPR(dd,4a) illegal_1(op_4a);                                       /* DB   DD          */
OPR(dd,4b) illegal_1(op_4b);                                       /* DB   DD          */
OP(dd,4c) { C = HX;                                          } EOP /* LD   C,HX        */
OP(dd,4d) { C = LX;                                          } EOP /* LD   C,LX        */
OP(dd,4e) { eax(); nomreq_addr(PCD-1, 5); C = rm(m_ea);      } EOP /* LD   C,(IX+o)    */
OPR(dd,4f) illegal_1(op_4f);                                       /* DB   DD          */

OPR(dd,50) illegal_1(op_50);                                       /* DB   DD          */
OPR(dd,51) illegal_1(op_51);                                       /* DB   DD          */
OPR(dd,52) illegal_1(op_52);                                       /* DB   DD          */
OPR(dd,53) illegal_1(op_53);                                       /* DB   DD          */
OP(dd,54) { D = HX;                                          } EOP /* LD   D,HX        */
OP(dd,55) { D = LX;                                          } EOP /* LD   D,LX        */
OP(dd,56) { eax(); nomreq_addr(PCD-1, 5); D = rm(m_ea);      } EOP /* LD   D,(IX+o)    */
OPR(dd,57) illegal_1(op_57);                                       /* DB   DD          */

OPR(dd,58) illegal_1(op_58);                                       /* DB   DD          */
OPR(dd,59) illegal_1(op_59);                                       /* DB   DD          */
OPR(dd,5a) illegal_1(op_5a);                                       /* DB   DD          */
OPR(dd,5b) illegal_1(op_5b);                                       /* DB   DD          */
OP(dd,5c) { E = HX;                                          } EOP /* LD   E,HX        */
OP(dd,5d) { E = LX;                                          } EOP /* LD   E,LX        */
OP(dd,5e) { eax(); nomreq_addr(PCD-1, 5); E = rm(m_ea);      } EOP /* LD   E,(IX+o)    */
OPR(dd,5f) illegal_1(op_5f);                                       /* DB   DD          */

OP(dd,60) { HX = B;                                          } EOP /* LD   HX,B        */
OP(dd,61) { HX = C;                                          } EOP /* LD   HX,C        */
OP(dd,62) { HX = D;                                          } EOP /* LD   HX,D        */
OP(dd,63) { HX = E;                                          } EOP /* LD   HX,E        */
OP(dd,64) {                                                  } EOP /* LD   HX,HX       */
OP(dd,65) { HX = LX;                                         } EOP /* LD   HX,LX       */
OP(dd,66) { eax(); nomreq_addr(PCD-1, 5); H = rm(m_ea);      } EOP /* LD   H,(IX+o)    */
OP(dd,67) { HX = A;                                          } EOP /* LD   HX,A        */

OP(dd,68) { LX = B;                                          } EOP /* LD   LX,B        */
OP(dd,69) { LX = C;                                          } EOP /* LD   LX,C        */
OP(dd,6a) { LX = D;                                          } EOP /* LD   LX,D        */
OP(dd,6b) { LX = E;                                          } EOP /* LD   LX,E        */
OP(dd,6c) { LX = HX;                                         } EOP /* LD   LX,HX       */
OP(dd,6d) {                                                  } EOP /* LD   LX,LX       */
OP(dd,6e) { eax(); nomreq_addr(PCD-1, 5); L = rm(m_ea);      } EOP /* LD   L,(IX+o)    */
OP(dd,6f) { LX = A;                                          } EOP /* LD   LX,A        */

OP(dd,70) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, B);       } EOP /* LD   (IX+o),B    */
OP(dd,71) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, C);       } EOP /* LD   (IX+o),C    */
OP(dd,72) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, D);       } EOP /* LD   (IX+o),D    */
OP(dd,73) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, E);       } EOP /* LD   (IX+o),E    */
OP(dd,74) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, H);       } EOP /* LD   (IX+o),H    */
OP(dd,75) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, L);       } EOP /* LD   (IX+o),L    */
OPR(dd,76) illegal_1(op_76);                                       /* DB   DD          */
OP(dd,77) { eax(); nomreq_addr(PCD-1, 5); wm(m_ea, A);       } EOP /* LD   (IX+o),A    */

OPR(dd,78) illegal_1(op_78);                                       /* DB   DD          */
OPR(dd,79) illegal_1(op_79);                                       /* DB   DD          */
OPR(dd,7a) illegal_1(op_7a);                                       /* DB   DD          */
OPR(dd,7b) illegal_1(op_7b);                                       /* DB   DD          */
OP(dd,7c) { A = HX;                                          } EOP /* LD   A,HX        */
OP(dd,7d) { A = LX;                                          } EOP /* LD   A,LX        */
OP(dd,7e) { eax(); nomreq_addr(PCD-1, 5); A = rm(m_ea);      } EOP /* LD   A,(IX+o)    */
OPR(dd,7f) illegal_1(op_7f);                                       /* DB   DD          */

OPR(dd,80) illegal_1(op_80);                                       /* DB   DD          */
OPR(dd,81) illegal_1(op_81);                                       /* DB   DD          */
OPR(dd,82) illegal_1(op_82);                                       /* DB   DD          */
OPR(dd,83) illegal_1(op_83);                                       /* DB   DD          */
OP(dd,84) { add_a(HX);                                       } EOP /* ADD  A,HX        */
OP(dd,85) { add_a(LX);                                       } EOP /* ADD  A,LX        */
OP(dd,86) { eax(); nomreq_addr(PCD-1, 5); add_a(rm(m_ea));   } EOP /* ADD  A,(IX+o)    */
OPR(dd,87) illegal_1(op_87);                                       /* DB   DD          */

OPR(dd,88) illegal_1(op_88);                                       /* DB   DD          */
OPR(dd,89) illegal_1(op_89);                                       /* DB   DD          */
OPR(dd,8a) illegal_1(op_8a);                                       /* DB   DD          */
OPR(dd,8b) illegal_1(op_8b);                                       /* DB   DD          */
OP(dd,8c) { adc_a(HX);                                       } EOP /* ADC  A,HX        */
OP(dd,8d) { adc_a(LX);                                       } EOP /* ADC  A,LX        */
OP(dd,8e) { eax(); nomreq_addr(PCD-1, 5); adc_a(rm(m_ea));   } EOP /* ADC  A,(IX+o)    */
OPR(dd,8f) illegal_1(op_8f);                                       /* DB   DD          */

OPR(dd,90) illegal_1(op_90);                                       /* DB   DD          */
OPR(dd,91) illegal_1(op_91);                                       /* DB   DD          */
OPR(dd,92) illegal_1(op_92);                                       /* DB   DD          */
OPR(dd,93) illegal_1(op_93);                                       /* DB   DD          */
OP(dd,94) { sub(HX);                                         } EOP /* SUB  HX          */
OP(dd,95) { sub(LX);                                         } EOP /* SUB  LX          */
OP(dd,96) { eax(); nomreq_addr(PCD-1, 5); sub(rm(m_ea));     } EOP /* SUB  (IX+o)      */
OPR(dd,97) illegal_1(op_97);                                       /* DB   DD          */

OPR(dd,98) illegal_1(op_98);                                       /* DB   DD          */
OPR(dd,99) illegal_1(op_99);                                       /* DB   DD          */
OPR(dd,9a) illegal_1(op_9a);                                       /* DB   DD          */
OPR(dd,9b) illegal_1(op_9b);                                       /* DB   DD          */
OP(dd,9c) { sbc_a(HX);                                       } EOP /* SBC  A,HX        */
OP(dd,9d) { sbc_a(LX);                                       } EOP /* SBC  A,LX        */
OP(dd,9e) { eax(); nomreq_addr(PCD-1, 5); sbc_a(rm(m_ea));   } EOP /* SBC  A,(IX+o)    */
OPR(dd,9f) illegal_1(op_9f);                                       /* DB   DD          */

OPR(dd,a0) illegal_1(op_a0);                                       /* DB   DD          */
OPR(dd,a1) illegal_1(op_a1);                                       /* DB   DD          */
OPR(dd,a2) illegal_1(op_a2);                                       /* DB   DD          */
OPR(dd,a3) illegal_1(op_a3);                                       /* DB   DD          */
OP(dd,a4) { and_a(HX);                                       } EOP /* AND  HX          */
OP(dd,a5) { and_a(LX);                                       } EOP /* AND  LX          */
OP(dd,a6) { eax(); nomreq_addr(PCD-1, 5); and_a(rm(m_ea));   } EOP /* AND  (IX+o)      */
OPR(dd,a7) illegal_1(op_a7);                                       /* DB   DD          */

OPR(dd,a8) illegal_1(op_a8);                                       /* DB   DD          */
OPR(dd,a9) illegal_1(op_a9);                                       /* DB   DD          */
OPR(dd,aa) illegal_1(op_aa);                                       /* DB   DD          */
OPR(dd,ab) illegal_1(op_ab);                                       /* DB   DD          */
OP(dd,ac) { xor_a(HX);                                       } EOP /* XOR  HX          */
OP(dd,ad) { xor_a(LX);                                       } EOP /* XOR  LX          */
OP(dd,ae) { eax(); nomreq_addr(PCD-1, 5); xor_a(rm(m_ea));   } EOP /* XOR  (IX+o)      */
OPR(dd,af) illegal_1(op_af);                                       /* DB   DD          */

OPR(dd,b0) illegal_1(op_b0);                                       /* DB   DD          */
OPR(dd,b1) illegal_1(op_b1);                                       /* DB   DD          */
OPR(dd,b2) illegal_1(op_b2);                                       /* DB   DD          */
OPR(dd,b3) illegal_1(op_b3);                                       /* DB   DD          */
OP(dd,b4) { or_a(HX);                                        } EOP /* OR   HX          */
OP(dd,b5) { or_a(LX);                                        } EOP /* OR   LX          */
OP(dd,b6) { eax(); nomreq_addr(PCD-1, 5); or_a(rm(m_ea));    } EOP /* OR   (IX+o)      */
OPR(dd,b7) illegal_1(op_b7);                                       /* DB   DD          */

OPR(dd,b8) illegal_1(op_b8);                                       /* DB   DD          */
OPR(dd,b9) illegal_1(op_b9);                                       /* DB   DD          */
OPR(dd,ba) illegal_1(op_ba);                                       /* DB   DD          */
OPR(dd,bb) illegal_1(op_bb);                                       /* DB   DD          */
OP(dd,bc) { cp(HX);                                          } EOP /* CP   HX          */
OP(dd,bd) { cp(LX);                                          } EOP /* CP   LX          */
OP(dd,be) { eax(); nomreq_addr(PCD-1, 5); cp(rm(m_ea));      } EOP /* CP   (IX+o)      */
OPR(dd,bf) illegal_1(op_bf);                                       /* DB   DD          */

OPR(dd,c0) illegal_1(op_c0);                                       /* DB   DD          */
OPR(dd,c1) illegal_1(op_c1);                                       /* DB   DD          */
OPR(dd,c2) illegal_1(op_c2);                                       /* DB   DD          */
OPR(dd,c3) illegal_1(op_c3);                                       /* DB   DD          */
OPR(dd,c4) illegal_1(op_c4);                                       /* DB   DD          */
OPR(dd,c5) illegal_1(op_c5);                                       /* DB   DD          */
OPR(dd,c6) illegal_1(op_c6);                                       /* DB   DD          */
OPR(dd,c7) illegal_1(op_c7);                                       /* DB   DD          */

OPR(dd,c8) illegal_1(op_c8);                                       /* DB   DD          */
OPR(dd,c9) illegal_1(op_c9);                                       /* DB   DD          */
OPR(dd,ca) illegal_1(op_ca);                                       /* DB   DD          */
OP(dd,cb) { eax(); m_prefix_next = XY_CB;                    } EOP /* **   DD CB xx    */
OPR(dd,cc) illegal_1(op_cc);                                       /* DB   DD          */
OPR(dd,cd) illegal_1(op_cd);                                       /* DB   DD          */
OPR(dd,ce) illegal_1(op_ce);                                       /* DB   DD          */
OPR(dd,cf) illegal_1(op_cf);                                       /* DB   DD          */

OPR(dd,d0) illegal_1(op_d0);                                       /* DB   DD          */
OPR(dd,d1) illegal_1(op_d1);                                       /* DB   DD          */
OPR(dd,d2) illegal_1(op_d2);                                       /* DB   DD          */
OPR(dd,d3) illegal_1(op_d3);                                       /* DB   DD          */
OPR(dd,d4) illegal_1(op_d4);                                       /* DB   DD          */
OPR(dd,d5) illegal_1(op_d5);                                       /* DB   DD          */
OPR(dd,d6) illegal_1(op_d6);                                       /* DB   DD          */
OPR(dd,d7) illegal_1(op_d7);                                       /* DB   DD          */

OPR(dd,d8) illegal_1(op_d8);                                       /* DB   DD          */
OPR(dd,d9) illegal_1(op_d9);                                       /* DB   DD          */
OPR(dd,da) illegal_1(op_da);                                       /* DB   DD          */
OPR(dd,db) illegal_1(op_db);                                       /* DB   DD          */
OPR(dd,dc) illegal_1(op_dc);                                       /* DB   DD          */
OPR(dd,dd) illegal_1(op_dd);                                       /* DB   DD          */
OPR(dd,de) illegal_1(op_de);                                       /* DB   DD          */
OPR(dd,df) illegal_1(op_df);                                       /* DB   DD          */

OPR(dd,e0) illegal_1(op_e0);                                       /* DB   DD          */
OP(dd,e1) { pop(m_ix);                                       } EOP /* POP  IX          */
OPR(dd,e2) illegal_1(op_e2);                                       /* DB   DD          */
OP(dd,e3) { ex_sp(m_ix);                                     } EOP /* EX   (SP),IX     */
OPR(dd,e4) illegal_1(op_e4);                                       /* DB   DD          */
OP(dd,e5) { push(m_ix);                                      } EOP /* PUSH IX          */
OPR(dd,e6) illegal_1(op_e6);                                       /* DB   DD          */
OPR(dd,e7) illegal_1(op_e7);                                       /* DB   DD          */

OPR(dd,e8) illegal_1(op_e8);                                       /* DB   DD          */
OP(dd,e9) { PC = IX;                                         } EOP /* JP   (IX)        */
OPR(dd,ea) illegal_1(op_ea);                                       /* DB   DD          */
OPR(dd,eb) illegal_1(op_eb);                                       /* DB   DD          */
OPR(dd,ec) illegal_1(op_ec);                                       /* DB   DD          */
OPR(dd,ed) illegal_1(op_ed);                                       /* DB   DD          */
OPR(dd,ee) illegal_1(op_ee);                                       /* DB   DD          */
OPR(dd,ef) illegal_1(op_ef);                                       /* DB   DD          */

OPR(dd,f0) illegal_1(op_f0);                                       /* DB   DD          */
OPR(dd,f1) illegal_1(op_f1);                                       /* DB   DD          */
OPR(dd,f2) illegal_1(op_f2);                                       /* DB   DD          */
OPR(dd,f3) illegal_1(op_f3);                                       /* DB   DD          */
OPR(dd,f4) illegal_1(op_f4);                                       /* DB   DD          */
OPR(dd,f5) illegal_1(op_f5);                                       /* DB   DD          */
OPR(dd,f6) illegal_1(op_f6);                                       /* DB   DD          */
OPR(dd,f7) illegal_1(op_f7);                                       /* DB   DD          */

OPR(dd,f8) illegal_1(op_f8);                                       /* DB   DD          */
OP(dd,f9) { nomreq_ir(2); SP = IX;                           } EOP /* LD   SP,IX       */
OPR(dd,fa) illegal_1(op_fa);                                       /* DB   DD          */
OPR(dd,fb) illegal_1(op_fb);                                       /* DB   DD          */
OPR(dd,fc) illegal_1(op_fc);                                       /* DB   DD          */
OPR(dd,fd) illegal_1(op_fd);                                       /* DB   DD          */
OPR(dd,fe) illegal_1(op_fe);                                       /* DB   DD          */
OPR(dd,ff) illegal_1(op_ff);                                       /* DB   DD          */

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
OPR(fd,00) illegal_1(op_00);                                       /* DB   FD          */
OPR(fd,01) illegal_1(op_01);                                       /* DB   FD          */
OPR(fd,02) illegal_1(op_02);                                       /* DB   FD          */
OPR(fd,03) illegal_1(op_03);                                       /* DB   FD          */
OPR(fd,04) illegal_1(op_04);                                       /* DB   FD          */
OPR(fd,05) illegal_1(op_05);                                       /* DB   FD          */
OPR(fd,06) illegal_1(op_06);                                       /* DB   FD          */
OPR(fd,07) illegal_1(op_07);                                       /* DB   FD          */

OPR(fd,08) illegal_1(op_08);                                       /* DB   FD          */
OP(fd,09) { add16(m_iy, m_bc);                               } EOP /* ADD  IY,BC       */
OPR(fd,0a) illegal_1(op_0a);                                       /* DB   FD          */
OPR(fd,0b) illegal_1(op_0b);                                       /* DB   FD          */
OPR(fd,0c) illegal_1(op_0c);                                       /* DB   FD          */
OPR(fd,0d) illegal_1(op_0d);                                       /* DB   FD          */
OPR(fd,0e) illegal_1(op_0e);                                       /* DB   FD          */
OPR(fd,0f) illegal_1(op_0f);                                       /* DB   FD          */

OPR(fd,10) illegal_1(op_10);                                       /* DB   FD          */
OPR(fd,11) illegal_1(op_11);                                       /* DB   FD          */
OPR(fd,12) illegal_1(op_12);                                       /* DB   FD          */
OPR(fd,13) illegal_1(op_13);                                       /* DB   FD          */
OPR(fd,14) illegal_1(op_14);                                       /* DB   FD          */
OPR(fd,15) illegal_1(op_15);                                       /* DB   FD          */
OPR(fd,16) illegal_1(op_16);                                       /* DB   FD          */
OPR(fd,17) illegal_1(op_17);                                       /* DB   FD          */

OPR(fd,18) illegal_1(op_18);                                       /* DB   FD          */
OP(fd,19) { add16(m_iy, m_de);                               } EOP /* ADD  IY,DE       */
OPR(fd,1a) illegal_1(op_1a);                                       /* DB   FD          */
OPR(fd,1b) illegal_1(op_1b);                                       /* DB   FD          */
OPR(fd,1c) illegal_1(op_1c);                                       /* DB   FD          */
OPR(fd,1d) illegal_1(op_1d);                                       /* DB   FD          */
OPR(fd,1e) illegal_1(op_1e);                                       /* DB   FD          */
OPR(fd,1f) illegal_1(op_1f);                                       /* DB   FD          */

OPR(fd,20) illegal_1(op_20);                                       /* DB   FD          */
OP(fd,21) { IY = arg16();                                    } EOP /* LD   IY,w        */
OP(fd,22) { m_ea = arg16(); wm16(m_ea, m_iy); WZ = m_ea + 1; } EOP /* LD   (w),IY      */
OP(fd,23) { nomreq_ir(2); IY++;                              } EOP /* INC  IY          */
OP(fd,24) { HY = inc(HY);                                    } EOP /* INC  HY          */
OP(fd,25) { HY = dec(HY);                                    } EOP /* DEC  HY          */
OP(fd,26) { HY = arg();                                      } EOP /* LD   HY,n        */
OPR(fd,27) illegal_1(op_27);                                       /* DB   FD          */

OPR(fd,28) illegal_1(op_28);                                       /* DB   FD          */
OP(fd,29) { add16(m_iy, m_iy);                               } EOP /* ADD  IY,IY       */
OP(fd,2a) { m_ea = arg16(); rm16(m_ea, m_iy); WZ = m_ea + 1; } EOP /* LD   IY,(w)      */
OP(fd,2b) { nomreq_ir(2); IY--;                              } EOP /* DEC  IY          */
OP(fd,2c) { LY = inc(LY);                                    } EOP /* INC  LY          */
OP(fd,2d) { LY = dec(LY);                                    } EOP /* DEC  LY          */
OP(fd,2e) { LY = arg();                                      } EOP /* LD   LY,n        */
OPR(fd,2f) illegal_1(op_2f);                                       /* DB   FD          */

OPR(fd,30) illegal_1(op_30);                                       /* DB   FD          */
OPR(fd,31) illegal_1(op_31);                                       /* DB   FD          */
OPR(fd,32) illegal_1(op_32);                                       /* DB   FD          */
OPR(fd,33) illegal_1(op_33);                                       /* DB   FD          */
OP(fd,34) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, inc(rm_reg(m_ea))); } EOP /* INC  (IY+o)      */
OP(fd,35) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, dec(rm_reg(m_ea))); } EOP /* DEC  (IY+o)      */
OP(fd,36) { eay(); u8 a = arg(); nomreq_addr(PCD-1, 2); wm(m_ea, a);   } EOP /* LD   (IY+o),n    */
OPR(fd,37) illegal_1(op_37);                                       /* DB   FD          */

OPR(fd,38) illegal_1(op_38);                                       /* DB   FD          */
OP(fd,39) { add16(m_iy, m_sp);                               } EOP /* ADD  IY,SP       */
OPR(fd,3a) illegal_1(op_3a);                                       /* DB   FD          */
OPR(fd,3b) illegal_1(op_3b);                                       /* DB   FD          */
OPR(fd,3c) illegal_1(op_3c);                                       /* DB   FD          */
OPR(fd,3d) illegal_1(op_3d);                                       /* DB   FD          */
OPR(fd,3e) illegal_1(op_3e);                                       /* DB   FD          */
OPR(fd,3f) illegal_1(op_3f);                                       /* DB   FD          */

OPR(fd,40) illegal_1(op_40);                                       /* DB   FD          */
OPR(fd,41) illegal_1(op_41);                                       /* DB   FD          */
OPR(fd,42) illegal_1(op_42);                                       /* DB   FD          */
OPR(fd,43) illegal_1(op_43);                                       /* DB   FD          */
OP(fd,44) { B = HY;                                          } EOP /* LD   B,HY        */
OP(fd,45) { B = LY;                                          } EOP /* LD   B,LY        */
OP(fd,46) { eay(); nomreq_addr(PCD-1, 5); B = rm(m_ea);      } EOP /* LD   B,(IY+o)    */
OPR(fd,47) illegal_1(op_47);                                       /* DB   FD          */

OPR(fd,48) illegal_1(op_48);                                       /* DB   FD          */
OPR(fd,49) illegal_1(op_49);                                       /* DB   FD          */
OPR(fd,4a) illegal_1(op_4a);                                       /* DB   FD          */
OPR(fd,4b) illegal_1(op_4b);                                       /* DB   FD          */
OP(fd,4c) { C = HY;                                          } EOP /* LD   C,HY        */
OP(fd,4d) { C = LY;                                          } EOP /* LD   C,LY        */
OP(fd,4e) { eay(); nomreq_addr(PCD-1, 5); C = rm(m_ea);      } EOP /* LD   C,(IY+o)    */
OPR(fd,4f) illegal_1(op_4f);                                       /* DB   FD          */

OPR(fd,50) illegal_1(op_50);                                       /* DB   FD          */
OPR(fd,51) illegal_1(op_51);                                       /* DB   FD          */
OPR(fd,52) illegal_1(op_52);                                       /* DB   FD          */
OPR(fd,53) illegal_1(op_53);                                       /* DB   FD          */
OP(fd,54) { D = HY;                                          } EOP /* LD   D,HY        */
OP(fd,55) { D = LY;                                          } EOP /* LD   D,LY        */
OP(fd,56) { eay(); nomreq_addr(PCD-1, 5); D = rm(m_ea);      } EOP /* LD   D,(IY+o)    */
OPR(fd,57) illegal_1(op_57);                                       /* DB   FD          */

OPR(fd,58) illegal_1(op_58);                                       /* DB   FD          */
OPR(fd,59) illegal_1(op_59);                                       /* DB   FD          */
OPR(fd,5a) illegal_1(op_5a);                                       /* DB   FD          */
OPR(fd,5b) illegal_1(op_5b);                                       /* DB   FD          */
OP(fd,5c) { E = HY;                                          } EOP /* LD   E,HY        */
OP(fd,5d) { E = LY;                                          } EOP /* LD   E,LY        */
OP(fd,5e) { eay(); nomreq_addr(PCD-1, 5); E = rm(m_ea);      } EOP /* LD   E,(IY+o)    */
OPR(fd,5f) illegal_1(op_5f);                                       /* DB   FD          */

OP(fd,60) { HY = B;                                          } EOP /* LD   HY,B        */
OP(fd,61) { HY = C;                                          } EOP /* LD   HY,C        */
OP(fd,62) { HY = D;                                          } EOP /* LD   HY,D        */
OP(fd,63) { HY = E;                                          } EOP /* LD   HY,E        */
OP(fd,64) {                                                  } EOP /* LD   HY,HY       */
OP(fd,65) { HY = LY;                                         } EOP /* LD   HY,LY       */
OP(fd,66) { eay(); nomreq_addr(PCD-1, 5); H = rm(m_ea);      } EOP /* LD   H,(IY+o)    */
OP(fd,67) { HY = A;                                          } EOP /* LD   HY,A        */

OP(fd,68) { LY = B;                                          } EOP /* LD   LY,B        */
OP(fd,69) { LY = C;                                          } EOP /* LD   LY,C        */
OP(fd,6a) { LY = D;                                          } EOP /* LD   LY,D        */
OP(fd,6b) { LY = E;                                          } EOP /* LD   LY,E        */
OP(fd,6c) { LY = HY;                                         } EOP /* LD   LY,HY       */
OP(fd,6d) {                                                  } EOP /* LD   LY,LY       */
OP(fd,6e) { eay(); nomreq_addr(PCD-1, 5); L = rm(m_ea);      } EOP /* LD   L,(IY+o)    */
OP(fd,6f) { LY = A;                                          } EOP /* LD   LY,A        */

OP(fd,70) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, B);       } EOP /* LD   (IY+o),B    */
OP(fd,71) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, C);       } EOP /* LD   (IY+o),C    */
OP(fd,72) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, D);       } EOP /* LD   (IY+o),D    */
OP(fd,73) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, E);       } EOP /* LD   (IY+o),E    */
OP(fd,74) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, H);       } EOP /* LD   (IY+o),H    */
OP(fd,75) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, L);       } EOP /* LD   (IY+o),L    */
OPR(fd,76) illegal_1(op_76);                                       /* DB   FD          */
OP(fd,77) { eay(); nomreq_addr(PCD-1, 5); wm(m_ea, A);       } EOP /* LD   (IY+o),A    */

OPR(fd,78) illegal_1(op_78);                                       /* DB   FD          */
OPR(fd,79) illegal_1(op_79);                                       /* DB   FD          */
OPR(fd,7a) illegal_1(op_7a);                                       /* DB   FD          */
OPR(fd,7b) illegal_1(op_7b);                                       /* DB   FD          */
OP(fd,7c) { A = HY;                                          } EOP /* LD   A,HY        */
OP(fd,7d) { A = LY;                                          } EOP /* LD   A,LY        */
OP(fd,7e) { eay(); nomreq_addr(PCD-1, 5); A = rm(m_ea);      } EOP /* LD   A,(IY+o)    */
OPR(fd,7f) illegal_1(op_7f);                                       /* DB   FD          */

OPR(fd,80) illegal_1(op_80);                                       /* DB   FD          */
OPR(fd,81) illegal_1(op_81);                                       /* DB   FD          */
OPR(fd,82) illegal_1(op_82);                                       /* DB   FD          */
OPR(fd,83) illegal_1(op_83);                                       /* DB   FD          */
OP(fd,84) { add_a(HY);                                       } EOP /* ADD  A,HY        */
OP(fd,85) { add_a(LY);                                       } EOP /* ADD  A,LY        */
OP(fd,86) { eay(); nomreq_addr(PCD-1, 5); add_a(rm(m_ea));   } EOP /* ADD  A,(IY+o)    */
OPR(fd,87) illegal_1(op_87);                                       /* DB   FD          */

OPR(fd,88) illegal_1(op_88);                                       /* DB   FD          */
OPR(fd,89) illegal_1(op_89);                                       /* DB   FD          */
OPR(fd,8a) illegal_1(op_8a);                                       /* DB   FD          */
OPR(fd,8b) illegal_1(op_8b);                                       /* DB   FD          */
OP(fd,8c) { adc_a(HY);                                       } EOP /* ADC  A,HY        */
OP(fd,8d) { adc_a(LY);                                       } EOP /* ADC  A,LY        */
OP(fd,8e) { eay(); nomreq_addr(PCD-1, 5); adc_a(rm(m_ea));   } EOP /* ADC  A,(IY+o)    */
OPR(fd,8f) illegal_1(op_8f);                                       /* DB   FD          */

OPR(fd,90) illegal_1(op_90);                                       /* DB   FD          */
OPR(fd,91) illegal_1(op_91);                                       /* DB   FD          */
OPR(fd,92) illegal_1(op_92);                                       /* DB   FD          */
OPR(fd,93) illegal_1(op_93);                                       /* DB   FD          */
OP(fd,94) { sub(HY);                                         } EOP /* SUB  HY          */
OP(fd,95) { sub(LY);                                         } EOP /* SUB  LY          */
OP(fd,96) { eay(); nomreq_addr(PCD-1, 5); sub(rm(m_ea));     } EOP /* SUB  (IY+o)      */
OPR(fd,97) illegal_1(op_97);                                       /* DB   FD          */

OPR(fd,98) illegal_1(op_98);                                       /* DB   FD          */
OPR(fd,99) illegal_1(op_99);                                       /* DB   FD          */
OPR(fd,9a) illegal_1(op_9a);                                       /* DB   FD          */
OPR(fd,9b) illegal_1(op_9b);                                       /* DB   FD          */
OP(fd,9c) { sbc_a(HY);                                       } EOP /* SBC  A,HY        */
OP(fd,9d) { sbc_a(LY);                                       } EOP /* SBC  A,LY        */
OP(fd,9e) { eay(); nomreq_addr(PCD-1, 5); sbc_a(rm(m_ea));   } EOP /* SBC  A,(IY+o)    */
OPR(fd,9f) illegal_1(op_9f);                                       /* DB   FD          */

OPR(fd,a0) illegal_1(op_a0);                                       /* DB   FD          */
OPR(fd,a1) illegal_1(op_a1);                                       /* DB   FD          */
OPR(fd,a2) illegal_1(op_a2);                                       /* DB   FD          */
OPR(fd,a3) illegal_1(op_a3);                                       /* DB   FD          */
OP(fd,a4) { and_a(HY);                                       } EOP /* AND  HY          */
OP(fd,a5) { and_a(LY);                                       } EOP /* AND  LY          */
OP(fd,a6) { eay(); nomreq_addr(PCD-1, 5); and_a(rm(m_ea));   } EOP /* AND  (IY+o)      */
OPR(fd,a7) illegal_1(op_a7);                                       /* DB   FD          */

OPR(fd,a8) illegal_1(op_a8);                                       /* DB   FD          */
OPR(fd,a9) illegal_1(op_a9);                                       /* DB   FD          */
OPR(fd,aa) illegal_1(op_aa);                                       /* DB   FD          */
OPR(fd,ab) illegal_1(op_ab);                                       /* DB   FD          */
OP(fd,ac) { xor_a(HY);                                       } EOP /* XOR  HY          */
OP(fd,ad) { xor_a(LY);                                       } EOP /* XOR  LY          */
OP(fd,ae) { eay(); nomreq_addr(PCD-1, 5); xor_a(rm(m_ea));   } EOP /* XOR  (IY+o)      */
OPR(fd,af) illegal_1(op_af);                                       /* DB   FD          */

OPR(fd,b0) illegal_1(op_b0);                                       /* DB   FD          */
OPR(fd,b1) illegal_1(op_b1);                                       /* DB   FD          */
OPR(fd,b2) illegal_1(op_b2);                                       /* DB   FD          */
OPR(fd,b3) illegal_1(op_b3);                                       /* DB   FD          */
OP(fd,b4) { or_a(HY);                                        } EOP /* OR   HY          */
OP(fd,b5) { or_a(LY);                                        } EOP /* OR   LY          */
OP(fd,b6) { eay(); nomreq_addr(PCD-1, 5); or_a(rm(m_ea));    } EOP /* OR   (IY+o)      */
OPR(fd,b7) illegal_1(op_b7);                                       /* DB   FD          */

OPR(fd,b8) illegal_1(op_b8);                                       /* DB   FD          */
OPR(fd,b9) illegal_1(op_b9);                                       /* DB   FD          */
OPR(fd,ba) illegal_1(op_ba);                                       /* DB   FD          */
OPR(fd,bb) illegal_1(op_bb);                                       /* DB   FD          */
OP(fd,bc) { cp(HY);                                          } EOP /* CP   HY          */
OP(fd,bd) { cp(LY);                                          } EOP /* CP   LY          */
OP(fd,be) { eay(); nomreq_addr(PCD-1, 5); cp(rm(m_ea));      } EOP /* CP   (IY+o)      */
OPR(fd,bf) illegal_1(op_bf);                                       /* DB   FD          */

OPR(fd,c0) illegal_1(op_c0);                                       /* DB   FD          */
OPR(fd,c1) illegal_1(op_c1);                                       /* DB   FD          */
OPR(fd,c2) illegal_1(op_c2);                                       /* DB   FD          */
OPR(fd,c3) illegal_1(op_c3);                                       /* DB   FD          */
OPR(fd,c4) illegal_1(op_c4);                                       /* DB   FD          */
OPR(fd,c5) illegal_1(op_c5);                                       /* DB   FD          */
OPR(fd,c6) illegal_1(op_c6);                                       /* DB   FD          */
OPR(fd,c7) illegal_1(op_c7);                                       /* DB   FD          */

OPR(fd,c8) illegal_1(op_c8);                                       /* DB   FD          */
OPR(fd,c9) illegal_1(op_c9);                                       /* DB   FD          */
OPR(fd,ca) illegal_1(op_ca);                                       /* DB   FD          */
OP(fd,cb) { eay(); m_prefix_next = XY_CB;                    } EOP /* **   FD CB xx    */
OPR(fd,cc) illegal_1(op_cc);                                       /* DB   FD          */
OPR(fd,cd) illegal_1(op_cd);                                       /* DB   FD          */
OPR(fd,ce) illegal_1(op_ce);                                       /* DB   FD          */
OPR(fd,cf) illegal_1(op_cf);                                       /* DB   FD          */

OPR(fd,d0) illegal_1(op_d0);                                       /* DB   FD          */
OPR(fd,d1) illegal_1(op_d1);                                       /* DB   FD          */
OPR(fd,d2) illegal_1(op_d2);                                       /* DB   FD          */
OPR(fd,d3) illegal_1(op_d3);                                       /* DB   FD          */
OPR(fd,d4) illegal_1(op_d4);                                       /* DB   FD          */
OPR(fd,d5) illegal_1(op_d5);                                       /* DB   FD          */
OPR(fd,d6) illegal_1(op_d6);                                       /* DB   FD          */
OPR(fd,d7) illegal_1(op_d7);                                       /* DB   FD          */

OPR(fd,d8) illegal_1(op_d8);                                       /* DB   FD          */
OPR(fd,d9) illegal_1(op_d9);                                       /* DB   FD          */
OPR(fd,da) illegal_1(op_da);                                       /* DB   FD          */
OPR(fd,db) illegal_1(op_db);                                       /* DB   FD          */
OPR(fd,dc) illegal_1(op_dc);                                       /* DB   FD          */
OPR(fd,dd) illegal_1(op_dd);                                       /* DB   FD          */
OPR(fd,de) illegal_1(op_de);                                       /* DB   FD          */
OPR(fd,df) illegal_1(op_df);                                       /* DB   FD          */

OPR(fd,e0) illegal_1(op_e0);                                       /* DB   FD          */
OP(fd,e1) { pop(m_iy);                                       } EOP /* POP  IY          */
OPR(fd,e2) illegal_1(op_e2);                                       /* DB   FD          */
OP(fd,e3) { ex_sp(m_iy);                                     } EOP /* EX   (SP),IY     */
OPR(fd,e4) illegal_1(op_e4);                                       /* DB   FD          */
OP(fd,e5) { push(m_iy);                                      } EOP /* PUSH IY          */
OPR(fd,e6) illegal_1(op_e6);                                       /* DB   FD          */
OPR(fd,e7) illegal_1(op_e7);                                       /* DB   FD          */

OPR(fd,e8) illegal_1(op_e8);                                       /* DB   FD          */
OP(fd,e9) { PC = IY;                                         } EOP /* JP   (IY)        */
OPR(fd,ea) illegal_1(op_ea);                                       /* DB   FD          */
OPR(fd,eb) illegal_1(op_eb);                                       /* DB   FD          */
OPR(fd,ec) illegal_1(op_ec);                                       /* DB   FD          */
OPR(fd,ed) illegal_1(op_ed);                                       /* DB   FD          */
OPR(fd,ee) illegal_1(op_ee);                                       /* DB   FD          */
OPR(fd,ef) illegal_1(op_ef);                                       /* DB   FD          */

OPR(fd,f0) illegal_1(op_f0);                                       /* DB   FD          */
OPR(fd,f1) illegal_1(op_f1);                                       /* DB   FD          */
OPR(fd,f2) illegal_1(op_f2);                                       /* DB   FD          */
OPR(fd,f3) illegal_1(op_f3);                                       /* DB   FD          */
OPR(fd,f4) illegal_1(op_f4);                                       /* DB   FD          */
OPR(fd,f5) illegal_1(op_f5);                                       /* DB   FD          */
OPR(fd,f6) illegal_1(op_f6);                                       /* DB   FD          */
OPR(fd,f7) illegal_1(op_f7);                                       /* DB   FD          */

OPR(fd,f8) illegal_1(op_f8);                                       /* DB   FD          */
OP(fd,f9) { nomreq_ir(2); SP = IY;                           } EOP /* LD   SP,IY       */
OPR(fd,fa) illegal_1(op_fa);                                       /* DB   FD          */
OPR(fd,fb) illegal_1(op_fb);                                       /* DB   FD          */
OPR(fd,fc) illegal_1(op_fc);                                       /* DB   FD          */
OPR(fd,fd) illegal_1(op_fd);                                       /* DB   FD          */
OPR(fd,fe) illegal_1(op_fe);                                       /* DB   FD          */
OPR(fd,ff) illegal_1(op_ff);                                       /* DB   FD          */

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
OP(ed,00) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,01) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,02) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,03) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,04) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,05) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,06) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,07) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,08) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,09) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,0f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,10) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,11) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,12) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,13) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,14) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,15) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,16) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,17) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,18) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,19) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,1f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,20) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,21) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,22) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,23) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,24) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,25) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,26) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,27) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,28) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,29) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,2f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,30) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,31) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,32) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,33) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,34) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,35) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,36) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,37) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,38) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,39) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,3f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,40) { B = in(BC); F = (F & CF) | SZP[B];               } EOP /* IN   B,(C)       */
OP(ed,41) { out(BC, B);                                      } EOP /* OUT  (C),B       */
OP(ed,42) { sbc_hl(m_bc);                                    } EOP /* SBC  HL,BC       */
OP(ed,43) { m_ea = arg16(); wm16(m_ea, m_bc); WZ = m_ea + 1; } EOP /* LD   (w),BC      */
OP(ed,44) { neg();                                           } EOP /* NEG              */
OP(ed,45) { retn();                                          } EOP /* RETN             */
OP(ed,46) { m_im = 0;                                        } EOP /* IM   0           */
OP(ed,47) { ld_i_a();                                        } EOP /* LD   i,A         */

OP(ed,48) { C = in(BC); F = (F & CF) | SZP[C];               } EOP /* IN   C,(C)       */
OP(ed,49) { out(BC, C);                                      } EOP /* OUT  (C),C       */
OP(ed,4a) { adc_hl(m_bc);                                    } EOP /* ADC  HL,BC       */
OP(ed,4b) { m_ea = arg16(); rm16(m_ea, m_bc); WZ = m_ea + 1; } EOP /* LD   BC,(w)      */
OP(ed,4c) { neg();                                           } EOP /* NEG              */
OP(ed,4d) { reti();                                          } EOP /* RETI             */
OP(ed,4e) { m_im = 0;                                        } EOP /* IM   0           */
OP(ed,4f) { ld_r_a();                                        } EOP /* LD   r,A         */

OP(ed,50) { D = in(BC); F = (F & CF) | SZP[D];               } EOP /* IN   D,(C)       */
OP(ed,51) { out(BC, D);                                      } EOP /* OUT  (C),D       */
OP(ed,52) { sbc_hl(m_de);                                    } EOP /* SBC  HL,DE       */
OP(ed,53) { m_ea = arg16(); wm16(m_ea, m_de); WZ = m_ea + 1; } EOP /* LD   (w),DE      */
OP(ed,54) { neg();                                           } EOP /* NEG              */
OP(ed,55) { retn();                                          } EOP /* RETN             */
OP(ed,56) { m_im = 1;                                        } EOP /* IM   1           */
OP(ed,57) { ld_a_i();                                        } EOP /* LD   A,i         */

OP(ed,58) { E = in(BC); F = (F & CF) | SZP[E];               } EOP /* IN   E,(C)       */
OP(ed,59) { out(BC, E);                                      } EOP /* OUT  (C),E       */
OP(ed,5a) { adc_hl(m_de);                                    } EOP /* ADC  HL,DE       */
OP(ed,5b) { m_ea = arg16(); rm16(m_ea, m_de); WZ = m_ea + 1; } EOP /* LD   DE,(w)      */
OP(ed,5c) { neg();                                           } EOP /* NEG              */
OP(ed,5d) { reti();                                          } EOP /* RETI             */
OP(ed,5e) { m_im = 2;                                        } EOP /* IM   2           */
OP(ed,5f) { ld_a_r();                                        } EOP /* LD   A,r         */

OP(ed,60) { H = in(BC); F = (F & CF) | SZP[H];               } EOP /* IN   H,(C)       */
OP(ed,61) { out(BC, H);                                      } EOP /* OUT  (C),H       */
OP(ed,62) { sbc_hl(m_hl);                                    } EOP /* SBC  HL,HL       */
OP(ed,63) { m_ea = arg16(); wm16(m_ea, m_hl); WZ = m_ea + 1; } EOP /* LD   (w),HL      */
OP(ed,64) { neg();                                           } EOP /* NEG              */
OP(ed,65) { retn();                                          } EOP /* RETN             */
OP(ed,66) { m_im = 0;                                        } EOP /* IM   0           */
OP(ed,67) { rrd();                                           } EOP /* RRD  (HL)        */

OP(ed,68) { L = in(BC); F = (F & CF) | SZP[L];               } EOP /* IN   L,(C)       */
OP(ed,69) { out(BC, L);                                      } EOP /* OUT  (C),L       */
OP(ed,6a) { adc_hl(m_hl);                                    } EOP /* ADC  HL,HL       */
OP(ed,6b) { m_ea = arg16(); rm16(m_ea, m_hl); WZ = m_ea + 1; } EOP /* LD   HL,(w)      */
OP(ed,6c) { neg();                                           } EOP /* NEG              */
OP(ed,6d) { reti();                                          } EOP /* RETI             */
OP(ed,6e) { m_im = 0;                                        } EOP /* IM   0           */
OP(ed,6f) { rld();                                           } EOP /* RLD  (HL)        */

OP(ed,70) { uint8_t res = in(BC); F = (F & CF) | SZP[res];   } EOP /* IN   0,(C)       */
OP(ed,71) { out(BC, 0);                                      } EOP /* OUT  (C),0       */
OP(ed,72) { sbc_hl(m_sp);                                    } EOP /* SBC  HL,SP       */
OP(ed,73) { m_ea = arg16(); wm16(m_ea, m_sp); WZ = m_ea + 1; } EOP /* LD   (w),SP      */
OP(ed,74) { neg();                                           } EOP /* NEG              */
OP(ed,75) { retn();                                          } EOP /* RETN             */
OP(ed,76) { m_im = 1;                                        } EOP /* IM   1           */
OP(ed,77) { illegal_2();                                     } EOP /* DB   ED,77       */

OP(ed,78) { A = in(BC); F = (F & CF) | SZP[A]; WZ = BC + 1;  } EOP /* IN   A,(C)       */
OP(ed,79) { out(BC, A);  WZ = BC + 1;                        } EOP /* OUT  (C),A       */
OP(ed,7a) { adc_hl(m_sp);                                    } EOP /* ADC  HL,SP       */
OP(ed,7b) { m_ea = arg16(); rm16(m_ea, m_sp); WZ = m_ea + 1; } EOP /* LD   SP,(w)      */
OP(ed,7c) { neg();                                           } EOP /* NEG              */
OP(ed,7d) { reti();                                          } EOP /* RETI             */
OP(ed,7e) { m_im = 2;                                        } EOP /* IM   2           */
OP(ed,7f) { illegal_2();                                     } EOP /* DB   ED,7F       */

OP(ed,80) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,81) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,82) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,83) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,84) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,85) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,86) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,87) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,88) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,89) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,8f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,90) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,91) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,92) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,93) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,94) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,95) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,96) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,97) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,98) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,99) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9a) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9b) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9c) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9d) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9e) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,9f) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,a0) { ldi();                                           } EOP /* LDI              */
OP(ed,a1) { cpi();                                           } EOP /* CPI              */
OP(ed,a2) { ini();                                           } EOP /* INI              */
OP(ed,a3) { outi();                                          } EOP /* OUTI             */
OP(ed,a4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,a5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,a6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,a7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,a8) { ldd();                                           } EOP /* LDD              */
OP(ed,a9) { cpd();                                           } EOP /* CPD              */
OP(ed,aa) { ind();                                           } EOP /* IND              */
OP(ed,ab) { outd();                                          } EOP /* OUTD             */
OP(ed,ac) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ad) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ae) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,af) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,b0) { ldir();                                          } EOP /* LDIR             */
OP(ed,b1) { cpir();                                          } EOP /* CPIR             */
OP(ed,b2) { inir();                                          } EOP /* INIR             */
OP(ed,b3) { otir();                                          } EOP /* OTIR             */
OP(ed,b4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,b5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,b6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,b7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,b8) { lddr();                                          } EOP /* LDDR             */
OP(ed,b9) { cpdr();                                          } EOP /* CPDR             */
OP(ed,ba) { indr();                                          } EOP /* INDR             */
OP(ed,bb) { otdr();                                          } EOP /* OTDR             */
OP(ed,bc) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,bd) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,be) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,bf) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,c0) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c1) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c2) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c3) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,c8) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,c9) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ca) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,cb) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,cc) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,cd) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ce) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,cf) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,d0) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d1) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d2) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d3) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,d8) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,d9) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,da) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,db) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,dc) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,dd) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,de) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,df) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,e0) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e1) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e2) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e3) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,e8) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,e9) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ea) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,eb) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ec) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ed) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ee) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ef) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,f0) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f1) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f2) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f3) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f4) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f5) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f6) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f7) { illegal_2();                                     } EOP /* DB   ED          */

OP(ed,f8) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,f9) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,fa) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,fb) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,fc) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,fd) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,fe) { illegal_2();                                     } EOP /* DB   ED          */
OP(ed,ff) { illegal_2();                                     } EOP /* DB   ED          */


/**********************************************************
 * main opcodes
 **********************************************************/
OP(op,00) {                                                                       } EOP /* NOP              */
OP(op,01) { BC = arg16();                                                         } EOP /* LD   BC,w        */
OP(op,02) { wm(BC,A); WZ_L = (BC + 1) & 0xFF;  WZ_H = A;                          } EOP /* LD (BC),A        */
OP(op,03) { nomreq_ir(2); BC++;                                                   } EOP /* INC  BC          */
OP(op,04) { B = inc(B);                                                           } EOP /* INC  B           */
OP(op,05) { B = dec(B);                                                           } EOP /* DEC  B           */
OP(op,06) { B = arg();                                                            } EOP /* LD   B,n         */
OP(op,07) { rlca();                                                               } EOP /* RLCA             */

OP(op,08) { ex_af();                                                              } EOP /* EX   AF,AF'      */
OP(op,09) { add16(m_hl, m_bc);                                                    } EOP /* ADD  HL,BC       */
OP(op,0a) { A = rm(BC);  WZ=BC+1;                                                 } EOP /* LD   A,(BC)      */
OP(op,0b) { nomreq_ir(2); BC--;                                                   } EOP /* DEC  BC          */
OP(op,0c) { C = inc(C);                                                           } EOP /* INC  C           */
OP(op,0d) { C = dec(C);                                                           } EOP /* DEC  C           */
OP(op,0e) { C = arg();                                                            } EOP /* LD   C,n         */
OP(op,0f) { rrca();                                                               } EOP /* RRCA             */

OP(op,10) { nomreq_ir(1); B--; jr_cond(B, 0x10);                                  } EOP /* DJNZ o           */
OP(op,11) { }}, arg16_n(), {[&](){ DE = TMP;         /* TODO define templates */  } EOP /* LD   DE,w        */
OP(op,12) { wm(DE,A); WZ_L = (DE + 1) & 0xFF;  WZ_H = A;                          } EOP /* LD (DE),A        */
OP(op,13) { nomreq_ir(2); DE++;                                                   } EOP /* INC  DE          */
OP(op,14) { D = inc(D);                                                           } EOP /* INC  D           */
OP(op,15) { D = dec(D);                                                           } EOP /* DEC  D           */
OP(op,16) { D = arg();                                                            } EOP /* LD   D,n         */
OP(op,17) { rla();                                                                } EOP /* RLA              */

OP(op,18) { jr();                                                                 } EOP /* JR   o           */
OP(op,19) { add16(m_hl, m_de);                                                    } EOP /* ADD  HL,DE       */
OP(op,1a) { A = rm(DE); WZ = DE + 1;                                              } EOP /* LD   A,(DE)      */
OP(op,1b) { nomreq_ir(2); DE--;                                                   } EOP /* DEC  DE          */
OP(op,1c) { E = inc(E);                                                           } EOP /* INC  E           */
OP(op,1d) { E = dec(E);                                                           } EOP /* DEC  E           */
OP(op,1e) { E = arg();                                                            } EOP /* LD   E,n         */
OP(op,1f) { rra();                                                                } EOP /* RRA              */

OP(op,20) { jr_cond(!(F & ZF), 0x20);                                             } EOP /* JR   NZ,o        */
OP(op,21) { HL = arg16();                                                         } EOP /* LD   HL,w        */
OP(op,22) { m_ea = arg16(); wm16(m_ea, m_hl); WZ = m_ea + 1;                      } EOP /* LD   (w),HL      */
OP(op,23) { nomreq_ir(2); HL++;                                                   } EOP /* INC  HL          */
OP(op,24) { H = inc(H);                                                           } EOP /* INC  H           */
OP(op,25) { H = dec(H);                                                           } EOP /* DEC  H           */
OP(op,26) { H = arg();                                                            } EOP /* LD   H,n         */
OP(op,27) { daa();                                                                } EOP /* DAA              */

OP(op,28) { jr_cond(F & ZF, 0x28);                                                } EOP /* JR   Z,o         */
OP(op,29) { add16(m_hl, m_hl);                                                    } EOP /* ADD  HL,HL       */
OP(op,2a) { m_ea = arg16(); rm16(m_ea, m_hl); WZ = m_ea + 1;                      } EOP /* LD   HL,(w)      */
OP(op,2b) { nomreq_ir(2); HL--;                                                   } EOP /* DEC  HL          */
OP(op,2c) { L = inc(L);                                                           } EOP /* INC  L           */
OP(op,2d) { L = dec(L);                                                           } EOP /* DEC  L           */
OP(op,2e) { L = arg();                                                            } EOP /* LD   L,n         */
OP(op,2f) { A ^= 0xff; F = (F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF)); } EOP /* CPL              */

OP(op,30) { jr_cond(!(F & CF), 0x30);                                             } EOP /* JR   NC,o        */
OP(op,31) { SP = arg16();                                                         } EOP /* LD   SP,w        */
OP(op,32) { m_ea = arg16(); wm(m_ea, A); WZ_L = (m_ea + 1) & 0xFF; WZ_H = A;      } EOP /* LD   (w),A       */
OP(op,33) { nomreq_ir(2); SP++;                                                   } EOP /* INC  SP          */
OP(op,34) { wm(HL, inc(rm_reg(HL)));                                              } EOP /* INC  (HL)        */
OP(op,35) { wm(HL, dec(rm_reg(HL)));                                              } EOP /* DEC  (HL)        */
OP(op,36) { wm(HL, arg());                                                        } EOP /* LD   (HL),n      */
OP(op,37) { F = (F & (SF | ZF | YF | XF | PF)) | CF | (A & (YF | XF));            } EOP /* SCF              */

OP(op,38) { jr_cond(F & CF, 0x38);                                                } EOP /* JR   C,o         */
OP(op,39) { add16(m_hl, m_sp);                                                    } EOP /* ADD  HL,SP       */
OP(op,3a) { m_ea = arg16(); A = rm(m_ea); WZ = m_ea + 1;                          } EOP /* LD   A,(w)       */
OP(op,3b) { nomreq_ir(2); SP--;                                                   } EOP /* DEC  SP          */
OP(op,3c) { A = inc(A);                                                           } EOP /* INC  A           */
OP(op,3d) { A = dec(A);                                                           } EOP /* DEC  A           */
OP(op,3e) { A = arg();                                                            } EOP /* LD   A,n         */
OP(op,3f) { F = ((F&(SF|ZF|YF|XF|PF|CF))|((F&CF)<<4)|(A&(YF|XF)))^CF;             } EOP /* CCF              */

OP(op,40) {                                                                       } EOP /* LD   B,B         */
OP(op,41) { B = C;                                                                } EOP /* LD   B,C         */
OP(op,42) { B = D;                                                                } EOP /* LD   B,D         */
OP(op,43) { B = E;                                                                } EOP /* LD   B,E         */
OP(op,44) { B = H;                                                                } EOP /* LD   B,H         */
OP(op,45) { B = L;                                                                } EOP /* LD   B,L         */
OP(op,46) { B = rm(HL);                                                           } EOP /* LD   B,(HL)      */
OP(op,47) { B = A;                                                                } EOP /* LD   B,A         */

OP(op,48) { C = B;                                                                } EOP /* LD   C,B         */
OP(op,49) {                                                                       } EOP /* LD   C,C         */
OP(op,4a) { C = D;                                                                } EOP /* LD   C,D         */
OP(op,4b) { C = E;                                                                } EOP /* LD   C,E         */
OP(op,4c) { C = H;                                                                } EOP /* LD   C,H         */
OP(op,4d) { C = L;                                                                } EOP /* LD   C,L         */
OP(op,4e) { C = rm(HL);                                                           } EOP /* LD   C,(HL)      */
OP(op,4f) { C = A;                                                                } EOP /* LD   C,A         */

OP(op,50) { D = B;                                                                } EOP /* LD   D,B         */
OP(op,51) { D = C;                                                                } EOP /* LD   D,C         */
OP(op,52) {                                                                       } EOP /* LD   D,D         */
OP(op,53) { D = E;                                                                } EOP /* LD   D,E         */
OP(op,54) { D = H;                                                                } EOP /* LD   D,H         */
OP(op,55) { D = L;                                                                } EOP /* LD   D,L         */
OP(op,56) { D = rm(HL);                                                           } EOP /* LD   D,(HL)      */
OP(op,57) { D = A;                                                                } EOP /* LD   D,A         */

OP(op,58) { E = B;                                                                } EOP /* LD   E,B         */
OP(op,59) { E = C;                                                                } EOP /* LD   E,C         */
OP(op,5a) { E = D;                                                                } EOP /* LD   E,D         */
OP(op,5b) {                                                                       } EOP /* LD   E,E         */
OP(op,5c) { E = H;                                                                } EOP /* LD   E,H         */
OP(op,5d) { E = L;                                                                } EOP /* LD   E,L         */
OP(op,5e) { E = rm(HL);                                                           } EOP /* LD   E,(HL)      */
OP(op,5f) { E = A;                                                                } EOP /* LD   E,A         */

OP(op,60) { H = B;                                                                } EOP /* LD   H,B         */
OP(op,61) { H = C;                                                                } EOP /* LD   H,C         */
OP(op,62) { H = D;                                                                } EOP /* LD   H,D         */
OP(op,63) { H = E;                                                                } EOP /* LD   H,E         */
OP(op,64) {                                                                       } EOP /* LD   H,H         */
OP(op,65) { H = L;                                                                } EOP /* LD   H,L         */
OP(op,66) { H = rm(HL);                                                           } EOP /* LD   H,(HL)      */
OP(op,67) { H = A;                                                                } EOP /* LD   H,A         */

OP(op,68) { L = B;                                                                } EOP /* LD   L,B         */
OP(op,69) { L = C;                                                                } EOP /* LD   L,C         */
OP(op,6a) { L = D;                                                                } EOP /* LD   L,D         */
OP(op,6b) { L = E;                                                                } EOP /* LD   L,E         */
OP(op,6c) { L = H;                                                                } EOP /* LD   L,H         */
OP(op,6d) {                                                                       } EOP /* LD   L,L         */
OP(op,6e) { L = rm(HL);                                                           } EOP /* LD   L,(HL)      */
OP(op,6f) { L = A;                                                                } EOP /* LD   L,A         */

OP(op,70) { wm(HL, B);                                                            } EOP /* LD   (HL),B      */
OP(op,71) { wm(HL, C);                                                            } EOP /* LD   (HL),C      */
OP(op,72) { wm(HL, D);                                                            } EOP /* LD   (HL),D      */
OP(op,73) { wm(HL, E);                                                            } EOP /* LD   (HL),E      */
OP(op,74) { wm(HL, H);                                                            } EOP /* LD   (HL),H      */
OP(op,75) { wm(HL, L);                                                            } EOP /* LD   (HL),L      */
OP(op,76) { halt();                                                               } EOP /* HALT             */
OP(op,77) { wm(HL, A);                                                            } EOP /* LD   (HL),A      */

OP(op,78) { A = B;                                                                } EOP /* LD   A,B         */
OP(op,79) { A = C;                                                                } EOP /* LD   A,C         */
OP(op,7a) { A = D;                                                                } EOP /* LD   A,D         */
OP(op,7b) { A = E;                                                                } EOP /* LD   A,E         */
OP(op,7c) { A = H;                                                                } EOP /* LD   A,H         */
OP(op,7d) { A = L;                                                                } EOP /* LD   A,L         */
OP(op,7e) { A = rm(HL);                                                           } EOP /* LD   A,(HL)      */
OP(op,7f) {                                                                       } EOP /* LD   A,A         */

OP(op,80) { add_a(B);                                                             } EOP /* ADD  A,B         */
OP(op,81) { add_a(C);                                                             } EOP /* ADD  A,C         */
OP(op,82) { add_a(D);                                                             } EOP /* ADD  A,D         */
OP(op,83) { add_a(E);                                                             } EOP /* ADD  A,E         */
OP(op,84) { add_a(H);                                                             } EOP /* ADD  A,H         */
OP(op,85) { add_a(L);                                                             } EOP /* ADD  A,L         */
OP(op,86) { add_a(rm(HL));                                                        } EOP /* ADD  A,(HL)      */
OP(op,87) { add_a(A);                                                             } EOP /* ADD  A,A         */

OP(op,88) { adc_a(B);                                                             } EOP /* ADC  A,B         */
OP(op,89) { adc_a(C);                                                             } EOP /* ADC  A,C         */
OP(op,8a) { adc_a(D);                                                             } EOP /* ADC  A,D         */
OP(op,8b) { adc_a(E);                                                             } EOP /* ADC  A,E         */
OP(op,8c) { adc_a(H);                                                             } EOP /* ADC  A,H         */
OP(op,8d) { adc_a(L);                                                             } EOP /* ADC  A,L         */
OP(op,8e) { adc_a(rm(HL));                                                        } EOP /* ADC  A,(HL)      */
OP(op,8f) { adc_a(A);                                                             } EOP /* ADC  A,A         */

OP(op,90) { sub(B);                                                               } EOP /* SUB  B           */
OP(op,91) { sub(C);                                                               } EOP /* SUB  C           */
OP(op,92) { sub(D);                                                               } EOP /* SUB  D           */
OP(op,93) { sub(E);                                                               } EOP /* SUB  E           */
OP(op,94) { sub(H);                                                               } EOP /* SUB  H           */
OP(op,95) { sub(L);                                                               } EOP /* SUB  L           */
OP(op,96) { sub(rm(HL));                                                          } EOP /* SUB  (HL)        */
OP(op,97) { sub(A);                                                               } EOP /* SUB  A           */

OP(op,98) { sbc_a(B);                                                             } EOP /* SBC  A,B         */
OP(op,99) { sbc_a(C);                                                             } EOP /* SBC  A,C         */
OP(op,9a) { sbc_a(D);                                                             } EOP /* SBC  A,D         */
OP(op,9b) { sbc_a(E);                                                             } EOP /* SBC  A,E         */
OP(op,9c) { sbc_a(H);                                                             } EOP /* SBC  A,H         */
OP(op,9d) { sbc_a(L);                                                             } EOP /* SBC  A,L         */
OP(op,9e) { sbc_a(rm(HL));                                                        } EOP /* SBC  A,(HL)      */
OP(op,9f) { sbc_a(A);                                                             } EOP /* SBC  A,A         */

OP(op,a0) { and_a(B);                                                             } EOP /* AND  B           */
OP(op,a1) { and_a(C);                                                             } EOP /* AND  C           */
OP(op,a2) { and_a(D);                                                             } EOP /* AND  D           */
OP(op,a3) { and_a(E);                                                             } EOP /* AND  E           */
OP(op,a4) { and_a(H);                                                             } EOP /* AND  H           */
OP(op,a5) { and_a(L);                                                             } EOP /* AND  L           */
OP(op,a6) { and_a(rm(HL));                                                        } EOP /* AND  (HL)        */
OP(op,a7) { and_a(A);                                                             } EOP /* AND  A           */

OP(op,a8) { xor_a(B);                                                             } EOP /* XOR  B           */
OP(op,a9) { xor_a(C);                                                             } EOP /* XOR  C           */
OP(op,aa) { xor_a(D);                                                             } EOP /* XOR  D           */
OP(op,ab) { xor_a(E);                                                             } EOP /* XOR  E           */
OP(op,ac) { xor_a(H);                                                             } EOP /* XOR  H           */
OP(op,ad) { xor_a(L);                                                             } EOP /* XOR  L           */
OP(op,ae) { xor_a(rm(HL));                                                        } EOP /* XOR  (HL)        */
OP(op,af) { xor_a(A);                                                             } EOP /* XOR  A           */

OP(op,b0) { or_a(B);                                                              } EOP /* OR   B           */
OP(op,b1) { or_a(C);                                                              } EOP /* OR   C           */
OP(op,b2) { or_a(D);                                                              } EOP /* OR   D           */
OP(op,b3) { or_a(E);                                                              } EOP /* OR   E           */
OP(op,b4) { or_a(H);                                                              } EOP /* OR   H           */
OP(op,b5) { or_a(L);                                                              } EOP /* OR   L           */
OP(op,b6) { or_a(rm(HL));                                                         } EOP /* OR   (HL)        */
OP(op,b7) { or_a(A);                                                              } EOP /* OR   A           */

OP(op,b8) { cp(B);                                                                } EOP /* CP   B           */
OP(op,b9) { cp(C);                                                                } EOP /* CP   C           */
OP(op,ba) { cp(D);                                                                } EOP /* CP   D           */
OP(op,bb) { cp(E);                                                                } EOP /* CP   E           */
OP(op,bc) { cp(H);                                                                } EOP /* CP   H           */
OP(op,bd) { cp(L);                                                                } EOP /* CP   L           */
OP(op,be) { cp(rm(HL));                                                           } EOP /* CP   (HL)        */
OP(op,bf) { cp(A);                                                                } EOP /* CP   A           */

OP(op,c0) { ret_cond(!(F & ZF), 0xc0);                                            } EOP /* RET  NZ          */
OP(op,c1) { pop(m_bc);                                                            } EOP /* POP  BC          */
OP(op,c2) { jp_cond(!(F & ZF));                                                   } EOP /* JP   NZ,a        */
OP(op,c3) { jp();                                                                 } EOP /* JP   a           */
OP(op,c4) { call_cond(!(F & ZF), 0xc4);                                           } EOP /* CALL NZ,a        */
OP(op,c5) { push(m_bc);                                                           } EOP /* PUSH BC          */
OP(op,c6) { add_a(arg());                                                         } EOP /* ADD  A,n         */
OP(op,c7) { rst(0x00);                                                            } EOP /* RST  0           */

OP(op,c8) { ret_cond(F & ZF, 0xc8);                                               } EOP /* RET  Z           */
OP(op,c9) { pop(m_pc); WZ = PCD;                                                  } EOP /* RET              */
OP(op,ca) { jp_cond(F & ZF);                                                      } EOP /* JP   Z,a         */
OP(op,cb) { m_prefix_next = CB;                                                   } EOP /* **** CB xx       */
OP(op,cc) { call_cond(F & ZF, 0xcc);                                              } EOP /* CALL Z,a         */
OP(op,cd) { call();                                                               } EOP /* CALL a           */
OP(op,ce) { adc_a(arg());                                                         } EOP /* ADC  A,n         */
OP(op,cf) { rst(0x08);                                                            } EOP /* RST  1           */

OP(op,d0) { ret_cond(!(F & CF), 0xd0);                                            } EOP /* RET  NC          */
OP(op,d1) { pop(m_de);                                                            } EOP /* POP  DE          */
OP(op,d2) { jp_cond(!(F & CF));                                                   } EOP /* JP   NC,a        */
OP(op,d3) { unsigned n = arg() | (A << 8); out(n, A); WZ_L = ((n & 0xff) + 1) & 0xff;  WZ_H = A;   } EOP /* OUT  (n),A       */
OP(op,d4) { call_cond(!(F & CF), 0xd4);                                           } EOP /* CALL NC,a        */
OP(op,d5) { push(m_de);                                                           } EOP /* PUSH DE          */
OP(op,d6) { sub(arg());                                                           } EOP /* SUB  n           */
OP(op,d7) { rst(0x10);                                                            } EOP /* RST  2           */

OP(op,d8) { ret_cond(F & CF, 0xd8);                                               } EOP /* RET  C           */
OP(op,d9) { exx();                                                                } EOP /* EXX              */
OP(op,da) { jp_cond(F & CF);                                                      } EOP /* JP   C,a         */
OP(op,db) { unsigned n = arg() | (A << 8); A = in(n); WZ = n + 1;                 } EOP /* IN   A,(n)       */
OP(op,dc) { call_cond(F & CF, 0xdc);                                              } EOP /* CALL C,a         */
OP(op,dd) { m_prefix_next = DD;                                                   } EOP /* **** DD xx       */
OP(op,de) { sbc_a(arg());                                                         } EOP /* SBC  A,n         */
OP(op,df) { rst(0x18);                                                            } EOP /* RST  3           */

OP(op,e0) { ret_cond(!(F & PF), 0xe0);                                            } EOP /* RET  PO          */
OP(op,e1) { pop(m_hl);                                                            } EOP /* POP  HL          */
OP(op,e2) { jp_cond(!(F & PF));                                                   } EOP /* JP   PO,a        */
OP(op,e3) { ex_sp(m_hl);                                                          } EOP /* EX   HL,(SP)     */
OP(op,e4) { call_cond(!(F & PF), 0xe4);                                           } EOP /* CALL PO,a        */
OP(op,e5) { push(m_hl);                                                           } EOP /* PUSH HL          */
OP(op,e6) { and_a(arg());                                                         } EOP /* AND  n           */
OP(op,e7) { rst(0x20);                                                            } EOP /* RST  4           */

OP(op,e8) { ret_cond(F & PF, 0xe8);                                               } EOP /* RET  PE          */
OP(op,e9) { PC = HL;                                                              } EOP /* JP   (HL)        */
OP(op,ea) { jp_cond(F & PF);                                                      } EOP /* JP   PE,a        */
OP(op,eb) { ex_de_hl();                                                           } EOP /* EX   DE,HL       */
OP(op,ec) { call_cond(F & PF, 0xec);                                              } EOP /* CALL PE,a        */
OP(op,ed) { m_prefix_next = ED;                                                   } EOP /* **** ED xx       */
OP(op,ee) { xor_a(arg());                                                         } EOP /* XOR  n           */
OP(op,ef) { rst(0x28);                                                            } EOP /* RST  5           */

OP(op,f0) { ret_cond(!(F & SF), 0xf0);                                            } EOP /* RET  P           */
OP(op,f1) { pop(m_af);                                                            } EOP /* POP  AF          */
OP(op,f2) { jp_cond(!(F & SF));                                                   } EOP /* JP   P,a         */
OP(op,f3) { m_iff1 = m_iff2 = 0;                                                  } EOP /* DI               */
OP(op,f4) { call_cond(!(F & SF), 0xf4);                                           } EOP /* CALL P,a         */
OP(op,f5) { push(m_af);                                                           } EOP /* PUSH AF          */
OP(op,f6) { or_a(arg());                                                          } EOP /* OR   n           */
OP(op,f7) { rst(0x30);                                                            } EOP /* RST  6           */

OP(op,f8) { ret_cond(F & SF, 0xf8);                                               } EOP /* RET  M           */
OP(op,f9) { nomreq_ir(2); SP = HL;                                                } EOP /* LD   SP,HL       */
OP(op,fa) { jp_cond(F & SF);                                                      } EOP /* JP   M,a         */
OP(op,fb) { ei();                                                                 } EOP /* EI               */
OP(op,fc) { call_cond(F & SF, 0xfc);                                              } EOP /* CALL M,a         */
OP(op,fd) { m_prefix_next = FD;                                                   } EOP /* **** FD xx       */
OP(op,fe) { cp(arg());                                                            } EOP /* CP   n           */
OP(op,ff) { rst(0x38);                                                            } EOP /* RST  7           */

} // end: init_instructions()

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
	int irq_vector = (intf != nullptr) ? intf->z80daisy_irq_ack() : standard_irq_callback_member(*this, 0);
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
					/* RST $xx cycles */
					CC(op, 0xff);
					T(m_icount_executing - MTM * 2);
					wm16_sp(m_pc);
					PCD = irq_vector & 0x0038;
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

void z80_device::nomreq_ir(s8 cycles)
{
	nomreq_addr((m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f), cycles);
}

void z80_device::nomreq_addr(u16 addr, s8 cycles)
{
	for (; cycles; cycles--)
	{
		m_nomreq_cb(addr, 0x00, 0xff);
		T(1);
	}
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
		uint8_t *padd = &SZHVC_add[  0*256];
		uint8_t *padc = &SZHVC_add[256*256];
		uint8_t *psub = &SZHVC_sub[  0*256];
		uint8_t *psbc = &SZHVC_sub[256*256];
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

		init_instructions();
		tables_initialised = true;
	}

	save_item(NAME(m_cycle));
	save_item(NAME(m_prefix));
	save_item(NAME(m_prefix_next));
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
	save_item(NAME(m_m_shared));

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

	m_irqack_cb.resolve_safe();
	m_refresh_cb.resolve_safe();
	m_nomreq_cb.resolve_safe();
	m_halt_cb.resolve_safe();
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
	m_prefix_next = NONE;
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

z80_device::ops_type z80_device::ops_flat(std::vector<ops_type> op_map)
{
	ops_type ops = {};
	for (auto vv = begin(op_map); vv != end(op_map); ++vv)
		for (auto vf = begin(*vv); vf != end(*vv); ++vf)
				ops.push_back(*vf);

	return ops;
}

z80_device::ops_type z80_device::* z80_device::do_exec()
{
	switch (m_prefix)
	{
	case NONE:  EXEC(op,   m_opcode);
	case CB:    EXEC(cb,   m_opcode);
	case DD:    EXEC(dd,   m_opcode);
	case ED:    EXEC(ed,   m_opcode);
	case FD:    EXEC(fd,   m_opcode);
	case XY_CB: EXEC(xycb, m_opcode);
	}
	// error
	return &z80_device::op_00;
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

/****************************************************************************
 * Execute 'cycles' T-states.
 ****************************************************************************/
void z80_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_wait_state)
		{
			m_icount = 0; // stalled
			return;
		}

		ops_type v = this->*do_exec();
		while (m_cycle < v.size() && m_icount > 0)
			v[m_cycle++]();

		if(m_cycle >= v.size())
			m_cycle = 0;
	}

// TODO validation only, cleanup!
if (m_icount < tmp_max_overlap)
{
	tmp_max_overlap = m_icount;
	printf("-> %d\n", -tmp_max_overlap);
}
}

z80_device::ops_type z80_device::next_op()
{
	return {[&](){
		//assert(m_icount_executing == 0); // expected to be valid without custom op_* tables (and in base z80 implementation)
		if (m_icount_executing > 0) T(m_icount_executing); else m_icount_executing = 0;

		// check for interrupts before each instruction
		if (m_prefix_next == NONE) check_interrupts();
	}, [&](){
		if (m_prefix_next == NONE)
		{
			m_after_ei = false;
			m_after_ldair = false;

			PRVPC = PCD;
			debugger_instruction_hook(PCD);
		}

		if (m_prefix_next == XY_CB)
		{
			m_opcode = arg();
			nomreq_addr(PCD-1, 2);
		}
		else
			m_opcode = rop();
	}, [&](){
		// when in HALT state, the fetched opcode is not dispatched (aka a NOP)
		if (m_halt)
		{
			PC--;
			m_opcode = 0;
			m_prefix = NONE;
		}
		else
		{
			m_prefix = m_prefix_next;
			m_prefix_next = NONE;
		}

		calculate_icount();
	}};
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

//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> z80_device::create_disassembler()
{
	return std::make_unique<z80_disassembler>();
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

void z80_device::z80_set_cycle_tables(const uint8_t *op, const uint8_t *cb, const uint8_t *ed, const uint8_t *xy, const uint8_t *xycb, const uint8_t *ex)
{
	m_cc_op = (op != nullptr) ? op : cc_op;
	m_cc_cb = (cb != nullptr) ? cb : cc_cb;
	m_cc_ed = (ed != nullptr) ? ed : cc_ed;
	m_cc_xy = (xy != nullptr) ? xy : cc_xy;
	m_cc_xycb = (xycb != nullptr) ? xycb : cc_xycb;
	m_cc_ex = (ex != nullptr) ? ex : cc_ex;
}


z80_device::z80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	z80_device(mconfig, Z80, tag, owner, clock)
{
}

z80_device::z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
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

nsc800_device::nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: z80_device(mconfig, NSC800, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(NSC800, nsc800_device, "nsc800", "National Semiconductor NSC800")
