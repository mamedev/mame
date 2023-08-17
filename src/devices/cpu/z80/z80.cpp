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
 *         (flags & A & 0x28).
 *         However, recent findings say that SCF/CCF X/Y results depend on whether
 *         or not the previous instruction touched the flag register.
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
 *        + execute instruction adjusting icount per each Read (arg(), recursive rop()) and Write
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

#define LOG_UNDOC (1U << 1)
#define LOG_INT   (1U << 2)
#define LOG_TIME  (1U << 3)

#define VERBOSE ( LOG_UNDOC /*| LOG_INT*/ )
#include "logmacro.h"

#define LOGUNDOC(...) LOGMASKED(LOG_UNDOC, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)


/* On an NMOS Z80, if LD A,I or LD A,R is interrupted, P/V flag gets reset,
   even if IFF2 was set before this instruction. This issue was fixed on
   the CMOS Z80, so until knowing (most) Z80 types on hardware, it's disabled */
#define HAS_LDAIR_QUIRK  0


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
#define Q       m_q

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


/***************************************************************
 * define an opcode builder helpers
 ***************************************************************/
#define DEF(name) z80_device::ops_type z80_device::name { return op_builder(*this).foo([]() {
#define CALL })->call([&]() -> ops_type { return
#define THEN })->add([&]() {
#define IF(cond) })->do_if([&]() -> bool { return cond;
#define ELSE })->do_else()->foo([]() {
#define ENDIF })->edo()->foo([]() {
#define ENDDEF })->get_steps(); }

/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
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
DEF( in() )
	THEN
		TDAT8 = m_io.read_interruptible(TADR);
		T(m_iorq_cycles);
ENDDEF

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
DEF( out() )
	THEN
		m_io.write_interruptible(TADR, TDAT8);
		T(m_iorq_cycles);
ENDDEF

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
inline u8 z80_device::data_read(u16 addr)
{
	return m_data.read_interruptible(translate_memory_address(addr));
}

DEF( rm() )
	THEN
		TDAT8 = data_read(TADR);
		T(m_memrq_cycles);
ENDDEF

DEF( rm_reg() )
	CALL rm();
	CALL nomreq_addr(1);
ENDDEF

/***************************************************************
 * Read a word from given memory location
 *  in: TADR
 * out: TDAT
 ***************************************************************/
DEF( rm16() )
	CALL rm();
	THEN
		TDAT_H = TDAT_L;
		TADR++;
	CALL rm();
	THEN
		std::swap(TDAT_H, TDAT_L);
ENDDEF

inline void z80_device::rm16(uint16_t addr, PAIR &r)
{
	r.b.l = data_read(addr);
	T(m_memrq_cycles);
	r.b.h = data_read(addr+1);
	T(m_memrq_cycles);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
inline void z80_device::data_write(u16 addr, u8 value) {
	m_data.write_interruptible(translate_memory_address((u32)addr), value);
}

DEF( wm() )
	THEN
		data_write(TADR, TDAT8);
		T(m_memrq_cycles);
ENDDEF

/***************************************************************
 * Write a word to given memory location
 *  in: TADR, TDAT
 ***************************************************************/
DEF( wm16() )
	CALL wm();
	THEN
		TADR++;
		TDAT8=TDAT_H;
	CALL wm();
ENDDEF

/***************************************************************
 * Write a word to (SP)
 *  in: TDAT
 ***************************************************************/
DEF( wm16_sp() )
	THEN
		SP--;
	THEN
		data_write(SPD, TDAT_H);
		T(m_memrq_cycles);
	THEN
		SP--;
	THEN
		data_write(SPD, TDAT_L);
		T(m_memrq_cycles);
ENDDEF

inline void z80_device::wm16_sp(PAIR &r)
{
	SP--;
	data_write(SPD, r.b.h);
	T(m_memrq_cycles);
	SP--;
	data_write(SPD, r.b.l);
	T(m_memrq_cycles);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
inline u8 z80_device::opcode_read()
{
	return m_opcodes.read_byte(translate_memory_address(PCD));
}

DEF( rop() )
	THEN
		TDAT8 = opcode_read();
		T(m_m1_cycles - 2);
	THEN
		m_refresh_cb((m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f), 0x00, 0xff);
		T(2);
	THEN
		PC++;
		m_r++;
		Q = m_qtemp;
		m_qtemp = YF | XF;
ENDDEF

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 * out: TDAT8
 ***************************************************************/
inline u8 z80_device::arg_read()
{
	return m_args.read_byte(translate_memory_address(PCD));
}

DEF( arg() )
	THEN
		TDAT8 = arg_read();
		T(m_memrq_cycles);
	THEN
		PC++;
ENDDEF

DEF( arg16() )
	CALL arg();
	THEN
		TDAT_H = TDAT_L;
	CALL arg();
	THEN
		std::swap(TDAT_H, TDAT_L);
ENDDEF

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
DEF( eax() )
	CALL arg();
	THEN
		m_ea = (u32)(u16)(IX + (s8)TDAT8);
		WZ = m_ea;
ENDDEF

DEF( eay() )
	CALL arg();
	THEN
		m_ea = (u32)(u16)(IY + (s8)TDAT8);
		WZ = m_ea;
ENDDEF

/***************************************************************
 * POP
 ***************************************************************/
DEF( pop() )
	THEN
		TDAT_L = data_read(SPD);
		T(m_memrq_cycles);
	THEN
		SP++;
	THEN
		TDAT_H = data_read(SPD); 
		T(m_memrq_cycles);
	THEN
		SP++;
ENDDEF

/***************************************************************
 * PUSH
 *  in: TDAT
 ***************************************************************/
DEF( push() )
	CALL nomreq_ir(1);
	CALL wm16_sp();
ENDDEF

/***************************************************************
 * JP
 ***************************************************************/
DEF( jp() )
	CALL arg16();
	THEN
		PCD = TDAT;
		WZ = PC;
ENDDEF

/***************************************************************
 * JP_COND
 ***************************************************************/
DEF( jp_cond() )
	IF ( TDAT8 )
		CALL arg16();
		THEN
			PC=TDAT;
			WZ=PCD;
	ELSE
		/* implicit do PC += 2 */
		CALL arg16();
		THEN
			WZ=TDAT;
	ENDIF
ENDDEF

/***************************************************************
 * JR
 ***************************************************************/
DEF( jr() )
	CALL arg();
	THEN
		TADR=PCD-1;
	CALL nomreq_addr(5);
	THEN
		PC += (s8)TDAT8;
		WZ = PC;
ENDDEF

/***************************************************************
 * JR_COND
 ***************************************************************/
DEF( jr_cond(u8 opcode) )
	IF ( TDAT8 )
		CALL jr();
	ELSE
		CALL arg();
	ENDIF
ENDDEF

/***************************************************************
 * CALL
 ***************************************************************/
DEF( call() )
	CALL arg16();
	THEN
		m_ea=TDAT;
		TADR=PCD-1;
	CALL nomreq_addr(1);
	THEN
		WZ = m_ea;
		TDAT=PC;
	CALL wm16_sp();
	THEN
		PCD=m_ea;
ENDDEF

/***************************************************************
 * CALL_COND
 ***************************************************************/
DEF( call_cond(u8 opcode) )
	IF ( TDAT8 )
		CALL call();
	ELSE
		CALL arg16();
		THEN
			WZ=TDAT;
	ENDIF
ENDDEF

/***************************************************************
 * RET_COND
 ***************************************************************/
DEF( ret_cond(u8 opcode) )
	CALL nomreq_ir(1);
	IF ( TDAT8 )
		CALL pop();
		THEN
			PC=TDAT;
			WZ = PC;
	ENDIF
ENDDEF

/***************************************************************
 * RETN
 ***************************************************************/
DEF( retn() )
	CALL pop();
	THEN
		PC=TDAT;
		LOGINT("RETN m_iff1:%d m_iff2:%d\n", m_iff1, m_iff2);
		WZ = PC;
		m_iff1 = m_iff2;
ENDDEF

/***************************************************************
 * RETI
 ***************************************************************/
DEF( reti() )
	CALL pop();
	THEN
		PC=TDAT;
		WZ = PC;
		m_iff1 = m_iff2;
		daisy_call_reti_device();
ENDDEF

/***************************************************************
 * LD   R,A
 ***************************************************************/
DEF( ld_r_a() )
	CALL nomreq_ir(1);
	THEN
		m_r = A;
		m_r2 = A & 0x80; /* keep bit 7 of r */
ENDDEF

/***************************************************************
 * LD   A,R
 ***************************************************************/
DEF( ld_a_r() )
	CALL nomreq_ir(1);
	THEN
		A = (m_r & 0x7f) | m_r2;
		set_f((F & CF) | SZ[A] | (m_iff2 << 2));
		m_after_ldair = true;
ENDDEF

/***************************************************************
 * LD   I,A
 ***************************************************************/
DEF( ld_i_a() )
	CALL nomreq_ir(1);
	THEN
		m_i = A;
ENDDEF

/***************************************************************
 * LD   A,I
 ***************************************************************/
DEF( ld_a_i() )
	CALL nomreq_ir(1);
	THEN
		A = m_i;
		set_f((F & CF) | SZ[A] | (m_iff2 << 2));
		m_after_ldair = true;
ENDDEF

/***************************************************************
 * RST
 ***************************************************************/
DEF( rst(u16 addr) )
	THEN
		TDAT=PC;
	CALL push();
	})->add([&, addr]() { // single occurrence. does it worth to have macro?
		PC=addr;
		WZ=PC;
ENDDEF

/***************************************************************
 * INC  r8
 ***************************************************************/
inline void z80_device::inc(u8 &r)
{
	++r;
	set_f((F & CF) | SZHV_inc[r]);
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline void z80_device::dec(u8 &r)
{
	--r;
	set_f((F & CF) | SZHV_dec[r]);
}

/***************************************************************
 * RLCA
 ***************************************************************/
inline void z80_device::rlca()
{
	A = (A << 1) | (A >> 7);
	set_f((F & (SF | ZF | PF)) | (A & (YF | XF | CF)));
}

/***************************************************************
 * RRCA
 ***************************************************************/
inline void z80_device::rrca()
{
	set_f((F & (SF | ZF | PF)) | (A & CF));
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
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void z80_device::rra()
{
	u8 res = (A >> 1) | (F << 7);
	u8 c = (A & 0x01) ? CF : 0;
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
DEF( rrd() )
	THEN
		TADR=HL;
	CALL rm();
	THEN
		WZ = HL+1;
	CALL nomreq_addr(4);
	THEN
		TDAT_H=TDAT8;
		TDAT8=(TDAT8 >> 4) | (A << 4);
	CALL wm();
	THEN
		A = (A & 0xf0) | (TDAT_H & 0x0f);
		set_f((F & CF) | SZP[A]);
ENDDEF

/***************************************************************
 * RLD
 ***************************************************************/
DEF( rld() )
	THEN
		TADR=HL;
	CALL rm();
	THEN
		WZ = HL+1;
	CALL nomreq_addr(4);
	THEN
		TDAT_H=TDAT8;
		TDAT8=(TDAT8 << 4) | (A & 0x0f);
	CALL wm();
	THEN
		A = (A & 0xf0) | (TDAT_H >> 4);
		set_f((F & CF) | SZP[A]);
ENDDEF

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void z80_device::add_a(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) + value);
	set_f(SZHVC_add[ah | res]);
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void z80_device::adc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) + value + c);
	set_f(SZHVC_add[(c << 16) | ah | res]);
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void z80_device::sub(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - value);
	set_f(SZHVC_sub[ah | res]);
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void z80_device::sbc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) - value - c);
	set_f(SZHVC_sub[(c<<16) | ah | res]);
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

	set_f((F&(CF|NF)) | (A>0x99) | ((A^a)&HF) | SZP[a]);
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
inline void z80_device::and_a(u8 value)
{
	A &= value;
	set_f(SZP[A] | HF);
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void z80_device::or_a(u8 value)
{
	A |= value;
	set_f(SZP[A]);
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void z80_device::xor_a(u8 value)
{
	A ^= value;
	set_f(SZP[A]);
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void z80_device::cp(u8 value)
{
	unsigned val = value;
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - val);
	set_f((SZHVC_sub[ah | res] & ~(YF | XF)) | (val & (YF | XF)));
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
DEF( ex_sp() )
	THEN
		TDAT2=TDAT;
	CALL pop();
	THEN
		TADR=SP-1;
	CALL nomreq_addr(1);
	THEN
		std::swap(TDAT, TDAT2);
	CALL wm16_sp();
	THEN
		TADR=SP;
	CALL nomreq_addr(2);
	THEN
		std::swap(TDAT, TDAT2);
		WZ=TDAT;
ENDDEF

/***************************************************************
 * ADD16
 ***************************************************************/
DEF( add16() )
	CALL nomreq_ir(7);
	THEN
		u32 res = TDAT + TDAT2;
		WZ = TDAT + 1;
		set_f((F & (SF | ZF | VF)) |
			(((TDAT ^ res ^ TDAT2) >> 8) & HF) |
			((res >> 16) & CF) | ((res >> 8) & (YF | XF)));
		TDAT = (u16)res;
ENDDEF

/***************************************************************
 * ADC  HL,r16
 ***************************************************************/
DEF( adc_hl() )
	CALL nomreq_ir(7);
	THEN
		u32 res = HLD + TDAT + (F & CF);
		WZ = HL + 1;
		set_f((((HLD ^ res ^ TDAT) >> 8) & HF) |
			((res >> 16) & CF) |
			((res >> 8) & (SF | YF | XF)) |
			((res & 0xffff) ? 0 : ZF) |
			(((TDAT ^ HLD ^ 0x8000) & (TDAT ^ res) & 0x8000) >> 13));
		HL = (u16)res;
ENDDEF

/***************************************************************
 * SBC  HL,r16
 ***************************************************************/
DEF( sbc_hl() )
	CALL nomreq_ir(7);
	THEN
		u32 res = HLD - TDAT - (F & CF);
		WZ = HL + 1;
		set_f((((HLD ^ res ^ TDAT) >> 8) & HF) | NF |
			((res >> 16) & CF) |
			((res >> 8) & (SF | YF | XF)) |
			((res & 0xffff) ? 0 : ZF) |
			(((TDAT ^ HLD) & (HLD ^ res) &0x8000) >> 13));
		HL = (u16)res;
ENDDEF

/***************************************************************
 * RLC  r8
 ***************************************************************/
inline u8 z80_device::rlc(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = ((res << 1) | (res >> 7)) & 0xff;
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
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
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * BIT  bit,r8
 ***************************************************************/
inline void z80_device::bit(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (value & (YF|XF)));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void z80_device::bit_hl(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | (WZ_H & (YF|XF)));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void z80_device::bit_xy(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1<<bit)] & ~(YF|XF)) | ((m_ea>>8) & (YF|XF)));
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
DEF( ldi() )
	THEN
		TADR=HL;
	CALL rm();
	THEN
		TADR=DE;
	CALL wm();
	CALL nomreq_addr(2);
	THEN
		set_f(F & (SF | ZF | CF));
		if ((A + TDAT8) & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if ((A + TDAT8) & 0x08) F |= XF; /* bit 3 -> flag 3 */
		HL++; DE++; BC--;
		if(BC) F |= VF;
ENDDEF

/***************************************************************
 * CPI
 ***************************************************************/
DEF( cpi() )
	THEN
		TADR=HL;
	CALL rm();
	CALL nomreq_addr(5);
	THEN
		u8 res = A - TDAT8;
		WZ++;
		HL++; BC--;
		set_f((F & CF) | (SZ[res]&~(YF|XF)) | ((A^TDAT8^res)&HF) | NF);
		if (F & HF) res -= 1;
		if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
		if (BC) F |= VF;
ENDDEF

/***************************************************************
 * INI
 ***************************************************************/
DEF( ini() )
	CALL nomreq_ir(1);
	THEN
		TADR=BC;
	CALL in();
	THEN
		WZ = BC + 1;
		B--;
		TADR=HL;
	CALL wm();
	THEN
		HL++;
		set_f(SZ[B]);
		unsigned t = (unsigned)((C + 1) & 0xff) + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;
ENDDEF

/***************************************************************
 * OUTI
 ***************************************************************/
DEF( outi() )
	CALL nomreq_ir(1);
	THEN
		TADR=HL;
	CALL rm();
	THEN
		B--;
		WZ = BC + 1;
		TADR=BC;
	CALL out();
	THEN
		HL++;
		set_f(SZ[B]);
		unsigned t = (unsigned)L + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;
ENDDEF

/***************************************************************
 * LDD
 ***************************************************************/
DEF( ldd() )
	THEN
		TADR=HL;
	CALL rm();
	THEN
		TADR=DE;
	CALL wm();
	CALL nomreq_addr(2);
	THEN
		F &= SF | ZF | CF;
		if ((A + TDAT8) & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if ((A + TDAT8) & 0x08) F |= XF; /* bit 3 -> flag 3 */
		HL--; DE--; BC--;
		if (BC) F |= VF;
ENDDEF

/***************************************************************
 * CPD
 ***************************************************************/
DEF( cpd() )
	THEN
		TADR=HL;
	CALL rm();
	THEN
		TADR=HL;
	CALL nomreq_addr(5);
	THEN
		u8 res = A - TDAT8;
		WZ--;
		HL--; BC--;
		set_f((F & CF) | (SZ[res]&~(YF|XF)) | ((A^TDAT8^res)&HF) | NF);
		if (F & HF) res -= 1;
		if (res & 0x02) F |= YF; /* bit 1 -> flag 5 */
		if (res & 0x08) F |= XF; /* bit 3 -> flag 3 */
		if (BC) F |= VF;
ENDDEF

/***************************************************************
 * IND
 ***************************************************************/
DEF( ind() )
	CALL nomreq_ir(1);
	THEN
		TADR=BC;
	CALL in();
	THEN
		WZ = BC - 1;
		B--;
		TADR=HL;
	CALL wm();
	THEN
		HL--;
		set_f(SZ[B]);
		unsigned t = ((unsigned)(C - 1) & 0xff) + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;
ENDDEF

/***************************************************************
 * OUTD
 ***************************************************************/
DEF( outd() )
	CALL nomreq_ir(1);
	THEN
		TADR=HL;
	CALL rm();
	THEN
		B--;
		WZ = BC - 1;
		TADR=BC;
	CALL out();
	THEN
		HL--;
		set_f(SZ[B]);
		unsigned t = (unsigned)L + (unsigned)TDAT8;
		if (TDAT8 & SF) F |= NF;
		if (t & 0x100) F |= HF | CF;
		F |= SZP[(u8)(t & 0x07) ^ B] & PF;
ENDDEF

/***************************************************************
 * LDIR
 ***************************************************************/
DEF( ldir() )
	CALL ldi();
	IF ( BC != 0 )
		THEN
			TADR=DE;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
			WZ = PC + 1;
	ENDIF
ENDDEF

/***************************************************************
 * CPIR
 ***************************************************************/
DEF( cpir() )
	CALL cpi();
	IF ( BC != 0 && !(F & ZF) )
		THEN
			TADR=HL;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
			WZ = PC + 1;
	ENDIF
ENDDEF

/***************************************************************
 * INIR
 ***************************************************************/
DEF( inir() )
	CALL ini();
	IF ( B != 0 )
		THEN
			TADR=HL;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
	ENDIF
ENDDEF

/***************************************************************
 * OTIR
 ***************************************************************/
DEF( otir() )
	CALL outi();
	IF ( B != 0 )
		THEN
			TADR=BC;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
	ENDIF
ENDDEF

/***************************************************************
 * LDDR
 ***************************************************************/
DEF( lddr() )
	CALL ldd();
	IF ( BC != 0 )
		THEN
			TADR=DE;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
			WZ = PC + 1;
	ENDIF
ENDDEF

/***************************************************************
 * CPDR
 ***************************************************************/
DEF( cpdr() )
	CALL cpd();
	IF ( BC != 0 && !(F & ZF) )
		THEN
			TADR=HL;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
			WZ = PC + 1;
	ENDIF
ENDDEF

/***************************************************************
 * INDR
 ***************************************************************/
DEF( indr() )
	CALL ind();
	IF ( B != 0 )
		THEN
			TADR=HL;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
	ENDIF
ENDDEF

/***************************************************************
 * OTDR
 ***************************************************************/
DEF( otdr() )
	CALL outd();
	IF ( B != 0 )
		THEN
			TADR=BC;
		CALL nomreq_addr(5);
		THEN
			PC -= 2;
	ENDIF
ENDDEF

/***************************************************************
 * EI
 ***************************************************************/
inline void z80_device::ei()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = true;
}

inline void z80_device::set_f(u8 f)
{
	m_qtemp = 0;
	F = f;
}

inline void z80_device::illegal_1() {
	LOGUNDOC("ill. opcode $%02x $%02x ($%04x)\n",
			m_opcodes.read_byte(translate_memory_address((PCD-1)&0xffff)), m_opcodes.read_byte(translate_memory_address(PCD)), PCD-1);
}

inline void z80_device::illegal_2()
{
	LOGUNDOC("ill. opcode $ed $%02x\n",
			m_opcodes.read_byte(translate_memory_address((PCD-1)&0xffff)));
}

void z80_device::init_op_steps() {

#define OP(prefix,opcode) m_op_steps[prefix][0x##opcode] = op_builder(*this).foo([]() {
#define THENJP(p_to) })->add([&]() { m_cycle=~0; m_prefix=p_to; m_opcode=TDAT8; })->get_steps();
#define JP(op) })->jump(0x##op);
#define JPP(p_to,op_to) })->jump(p_to, 0x##op_to);
#define ENDOP })->build();

/**********************************************************
 * opcodes with CB prefix
 * rotate, shift and bit operations
 **********************************************************/
/* RLC  B          */ OP(CB,00) THEN B = rlc(B);                                               ENDOP
/* RLC  C          */ OP(CB,01) THEN C = rlc(C);                                               ENDOP
/* RLC  D          */ OP(CB,02) THEN D = rlc(D);                                               ENDOP
/* RLC  E          */ OP(CB,03) THEN E = rlc(E);                                               ENDOP
/* RLC  H          */ OP(CB,04) THEN H = rlc(H);                                               ENDOP
/* RLC  L          */ OP(CB,05) THEN L = rlc(L);                                               ENDOP
/* RLC  (HL)       */ OP(CB,06) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=rlc(TDAT8); CALL wm(); ENDOP
/* RLC  A          */ OP(CB,07) THEN A = rlc(A);                                               ENDOP

/* RRC  B          */ OP(CB,08) THEN B = rrc(B);                                               ENDOP
/* RRC  C          */ OP(CB,09) THEN C = rrc(C);                                               ENDOP
/* RRC  D          */ OP(CB,0a) THEN D = rrc(D);                                               ENDOP
/* RRC  E          */ OP(CB,0b) THEN E = rrc(E);                                               ENDOP
/* RRC  H          */ OP(CB,0c) THEN H = rrc(H);                                               ENDOP
/* RRC  L          */ OP(CB,0d) THEN L = rrc(L);                                               ENDOP
/* RRC  (HL)       */ OP(CB,0e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=rrc(TDAT8); CALL wm(); ENDOP
/* RRC  A          */ OP(CB,0f) THEN A = rrc(A);                                               ENDOP

/* RL   B          */ OP(CB,10) THEN B = rl(B);                                                ENDOP
/* RL   C          */ OP(CB,11) THEN C = rl(C);                                                ENDOP
/* RL   D          */ OP(CB,12) THEN D = rl(D);                                                ENDOP
/* RL   E          */ OP(CB,13) THEN E = rl(E);                                                ENDOP
/* RL   H          */ OP(CB,14) THEN H = rl(H);                                                ENDOP
/* RL   L          */ OP(CB,15) THEN L = rl(L);                                                ENDOP
/* RL   (HL)       */ OP(CB,16) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=rl(TDAT8); CALL wm();  ENDOP
/* RL   A          */ OP(CB,17) THEN A = rl(A);                                                ENDOP

/* RR   B          */ OP(CB,18) THEN B = rr(B);                                                ENDOP
/* RR   C          */ OP(CB,19) THEN C = rr(C);                                                ENDOP
/* RR   D          */ OP(CB,1a) THEN D = rr(D);                                                ENDOP
/* RR   E          */ OP(CB,1b) THEN E = rr(E);                                                ENDOP
/* RR   H          */ OP(CB,1c) THEN H = rr(H);                                                ENDOP
/* RR   L          */ OP(CB,1d) THEN L = rr(L);                                                ENDOP
/* RR   (HL)       */ OP(CB,1e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=rr(TDAT8); CALL wm();  ENDOP
/* RR   A          */ OP(CB,1f) THEN A = rr(A);                                                ENDOP

/* SLA  B          */ OP(CB,20) THEN B = sla(B);                                               ENDOP
/* SLA  C          */ OP(CB,21) THEN C = sla(C);                                               ENDOP
/* SLA  D          */ OP(CB,22) THEN D = sla(D);                                               ENDOP
/* SLA  E          */ OP(CB,23) THEN E = sla(E);                                               ENDOP
/* SLA  H          */ OP(CB,24) THEN H = sla(H);                                               ENDOP
/* SLA  L          */ OP(CB,25) THEN L = sla(L);                                               ENDOP
/* SLA  (HL)       */ OP(CB,26) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=sla(TDAT8); CALL wm(); ENDOP
/* SLA  A          */ OP(CB,27) THEN A = sla(A);                                               ENDOP

/* SRA  B          */ OP(CB,28) THEN B = sra(B);                                               ENDOP
/* SRA  C          */ OP(CB,29) THEN C = sra(C);                                               ENDOP
/* SRA  D          */ OP(CB,2a) THEN D = sra(D);                                               ENDOP
/* SRA  E          */ OP(CB,2b) THEN E = sra(E);                                               ENDOP
/* SRA  H          */ OP(CB,2c) THEN H = sra(H);                                               ENDOP
/* SRA  L          */ OP(CB,2d) THEN L = sra(L);                                               ENDOP
/* SRA  (HL)       */ OP(CB,2e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=sra(TDAT8); CALL wm(); ENDOP
/* SRA  A          */ OP(CB,2f) THEN A = sra(A);                                               ENDOP

/* SLL  B          */ OP(CB,30) THEN B = sll(B);                                               ENDOP
/* SLL  C          */ OP(CB,31) THEN C = sll(C);                                               ENDOP
/* SLL  D          */ OP(CB,32) THEN D = sll(D);                                               ENDOP
/* SLL  E          */ OP(CB,33) THEN E = sll(E);                                               ENDOP
/* SLL  H          */ OP(CB,34) THEN H = sll(H);                                               ENDOP
/* SLL  L          */ OP(CB,35) THEN L = sll(L);                                               ENDOP
/* SLL  (HL)       */ OP(CB,36) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=sll(TDAT8); CALL wm(); ENDOP
/* SLL  A          */ OP(CB,37) THEN A = sll(A);                                               ENDOP

/* SRL  B          */ OP(CB,38) THEN B = srl(B);                                               ENDOP
/* SRL  C          */ OP(CB,39) THEN C = srl(C);                                               ENDOP
/* SRL  D          */ OP(CB,3a) THEN D = srl(D);                                               ENDOP
/* SRL  E          */ OP(CB,3b) THEN E = srl(E);                                               ENDOP
/* SRL  H          */ OP(CB,3c) THEN H = srl(H);                                               ENDOP
/* SRL  L          */ OP(CB,3d) THEN L = srl(L);                                               ENDOP
/* SRL  (HL)       */ OP(CB,3e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=srl(TDAT8); CALL wm(); ENDOP
/* SRL  A          */ OP(CB,3f) THEN A = srl(A);                                               ENDOP

/* BIT  0,B        */ OP(CB,40) THEN bit(0, B);                                                ENDOP
/* BIT  0,C        */ OP(CB,41) THEN bit(0, C);                                                ENDOP
/* BIT  0,D        */ OP(CB,42) THEN bit(0, D);                                                ENDOP
/* BIT  0,E        */ OP(CB,43) THEN bit(0, E);                                                ENDOP
/* BIT  0,H        */ OP(CB,44) THEN bit(0, H);                                                ENDOP
/* BIT  0,L        */ OP(CB,45) THEN bit(0, L);                                                ENDOP
/* BIT  0,(HL)     */ OP(CB,46) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(0, TDAT8);            ENDOP
/* BIT  0,A        */ OP(CB,47) THEN bit(0, A);                                                ENDOP

/* BIT  1,B        */ OP(CB,48) THEN bit(1, B);                                                ENDOP
/* BIT  1,C        */ OP(CB,49) THEN bit(1, C);                                                ENDOP
/* BIT  1,D        */ OP(CB,4a) THEN bit(1, D);                                                ENDOP
/* BIT  1,E        */ OP(CB,4b) THEN bit(1, E);                                                ENDOP
/* BIT  1,H        */ OP(CB,4c) THEN bit(1, H);                                                ENDOP
/* BIT  1,L        */ OP(CB,4d) THEN bit(1, L);                                                ENDOP
/* BIT  1,(HL)     */ OP(CB,4e) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(1, TDAT8);            ENDOP
/* BIT  1,A        */ OP(CB,4f) THEN bit(1, A);                                                ENDOP

/* BIT  2,B        */ OP(CB,50) THEN bit(2, B);                                                ENDOP
/* BIT  2,C        */ OP(CB,51) THEN bit(2, C);                                                ENDOP
/* BIT  2,D        */ OP(CB,52) THEN bit(2, D);                                                ENDOP
/* BIT  2,E        */ OP(CB,53) THEN bit(2, E);                                                ENDOP
/* BIT  2,H        */ OP(CB,54) THEN bit(2, H);                                                ENDOP
/* BIT  2,L        */ OP(CB,55) THEN bit(2, L);                                                ENDOP
/* BIT  2,(HL)     */ OP(CB,56) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(2, TDAT8);            ENDOP
/* BIT  2,A        */ OP(CB,57) THEN bit(2, A);                                                ENDOP

/* BIT  3,B        */ OP(CB,58) THEN bit(3, B);                                                ENDOP
/* BIT  3,C        */ OP(CB,59) THEN bit(3, C);                                                ENDOP
/* BIT  3,D        */ OP(CB,5a) THEN bit(3, D);                                                ENDOP
/* BIT  3,E        */ OP(CB,5b) THEN bit(3, E);                                                ENDOP
/* BIT  3,H        */ OP(CB,5c) THEN bit(3, H);                                                ENDOP
/* BIT  3,L        */ OP(CB,5d) THEN bit(3, L);                                                ENDOP
/* BIT  3,(HL)     */ OP(CB,5e) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(3, TDAT8);            ENDOP
/* BIT  3,A        */ OP(CB,5f) THEN bit(3, A);                                                ENDOP

/* BIT  4,B        */ OP(CB,60) THEN bit(4, B);                                                ENDOP
/* BIT  4,C        */ OP(CB,61) THEN bit(4, C);                                                ENDOP
/* BIT  4,D        */ OP(CB,62) THEN bit(4, D);                                                ENDOP
/* BIT  4,E        */ OP(CB,63) THEN bit(4, E);                                                ENDOP
/* BIT  4,H        */ OP(CB,64) THEN bit(4, H);                                                ENDOP
/* BIT  4,L        */ OP(CB,65) THEN bit(4, L);                                                ENDOP
/* BIT  4,(HL)     */ OP(CB,66) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(4, TDAT8);            ENDOP
/* BIT  4,A        */ OP(CB,67) THEN bit(4, A);                                                ENDOP

/* BIT  5,B        */ OP(CB,68) THEN bit(5, B);                                                ENDOP
/* BIT  5,C        */ OP(CB,69) THEN bit(5, C);                                                ENDOP
/* BIT  5,D        */ OP(CB,6a) THEN bit(5, D);                                                ENDOP
/* BIT  5,E        */ OP(CB,6b) THEN bit(5, E);                                                ENDOP
/* BIT  5,H        */ OP(CB,6c) THEN bit(5, H);                                                ENDOP
/* BIT  5,L        */ OP(CB,6d) THEN bit(5, L);                                                ENDOP
/* BIT  5,(HL)     */ OP(CB,6e) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(5, TDAT8);            ENDOP
/* BIT  5,A        */ OP(CB,6f) THEN bit(5, A);                                                ENDOP

/* BIT  6,B        */ OP(CB,70) THEN bit(6, B);                                                ENDOP
/* BIT  6,C        */ OP(CB,71) THEN bit(6, C);                                                ENDOP
/* BIT  6,D        */ OP(CB,72) THEN bit(6, D);                                                ENDOP
/* BIT  6,E        */ OP(CB,73) THEN bit(6, E);                                                ENDOP
/* BIT  6,H        */ OP(CB,74) THEN bit(6, H);                                                ENDOP
/* BIT  6,L        */ OP(CB,75) THEN bit(6, L);                                                ENDOP
/* BIT  6,(HL)     */ OP(CB,76) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(6, TDAT8);            ENDOP
/* BIT  6,A        */ OP(CB,77) THEN bit(6, A);                                                ENDOP

/* BIT  7,B        */ OP(CB,78) THEN bit(7, B);                                                ENDOP
/* BIT  7,C        */ OP(CB,79) THEN bit(7, C);                                                ENDOP
/* BIT  7,D        */ OP(CB,7a) THEN bit(7, D);                                                ENDOP
/* BIT  7,E        */ OP(CB,7b) THEN bit(7, E);                                                ENDOP
/* BIT  7,H        */ OP(CB,7c) THEN bit(7, H);                                                ENDOP
/* BIT  7,L        */ OP(CB,7d) THEN bit(7, L);                                                ENDOP
/* BIT  7,(HL)     */ OP(CB,7e) THEN TADR=HL; CALL rm_reg(); THEN bit_hl(7, TDAT8);            ENDOP
/* BIT  7,A        */ OP(CB,7f) THEN bit(7, A);                                                ENDOP

/* RES  0,B        */ OP(CB,80) THEN B = res(0, B);                                               ENDOP
/* RES  0,C        */ OP(CB,81) THEN C = res(0, C);                                               ENDOP
/* RES  0,D        */ OP(CB,82) THEN D = res(0, D);                                               ENDOP
/* RES  0,E        */ OP(CB,83) THEN E = res(0, E);                                               ENDOP
/* RES  0,H        */ OP(CB,84) THEN H = res(0, H);                                               ENDOP
/* RES  0,L        */ OP(CB,85) THEN L = res(0, L);                                               ENDOP
/* RES  0,(HL)     */ OP(CB,86) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(0, TDAT8); CALL wm(); ENDOP
/* RES  0,A        */ OP(CB,87) THEN A = res(0, A);                                               ENDOP

/* RES  1,B        */ OP(CB,88) THEN B = res(1, B);                                               ENDOP
/* RES  1,C        */ OP(CB,89) THEN C = res(1, C);                                               ENDOP
/* RES  1,D        */ OP(CB,8a) THEN D = res(1, D);                                               ENDOP
/* RES  1,E        */ OP(CB,8b) THEN E = res(1, E);                                               ENDOP
/* RES  1,H        */ OP(CB,8c) THEN H = res(1, H);                                               ENDOP
/* RES  1,L        */ OP(CB,8d) THEN L = res(1, L);                                               ENDOP
/* RES  1,(HL)     */ OP(CB,8e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(1, TDAT8); CALL wm(); ENDOP
/* RES  1,A        */ OP(CB,8f) THEN A = res(1, A);                                               ENDOP

/* RES  2,B        */ OP(CB,90) THEN B = res(2, B);                                               ENDOP
/* RES  2,C        */ OP(CB,91) THEN C = res(2, C);                                               ENDOP
/* RES  2,D        */ OP(CB,92) THEN D = res(2, D);                                               ENDOP
/* RES  2,E        */ OP(CB,93) THEN E = res(2, E);                                               ENDOP
/* RES  2,H        */ OP(CB,94) THEN H = res(2, H);                                               ENDOP
/* RES  2,L        */ OP(CB,95) THEN L = res(2, L);                                               ENDOP
/* RES  2,(HL)     */ OP(CB,96) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(2, TDAT8); CALL wm(); ENDOP
/* RES  2,A        */ OP(CB,97) THEN A = res(2, A);                                               ENDOP

/* RES  3,B        */ OP(CB,98) THEN B = res(3, B);                                               ENDOP
/* RES  3,C        */ OP(CB,99) THEN C = res(3, C);                                               ENDOP
/* RES  3,D        */ OP(CB,9a) THEN D = res(3, D);                                               ENDOP
/* RES  3,E        */ OP(CB,9b) THEN E = res(3, E);                                               ENDOP
/* RES  3,H        */ OP(CB,9c) THEN H = res(3, H);                                               ENDOP
/* RES  3,L        */ OP(CB,9d) THEN L = res(3, L);                                               ENDOP
/* RES  3,(HL)     */ OP(CB,9e) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(3, TDAT8); CALL wm(); ENDOP
/* RES  3,A        */ OP(CB,9f) THEN A = res(3, A);                                               ENDOP

/* RES  4,B        */ OP(CB,a0) THEN B = res(4, B);                                               ENDOP
/* RES  4,C        */ OP(CB,a1) THEN C = res(4, C);                                               ENDOP
/* RES  4,D        */ OP(CB,a2) THEN D = res(4, D);                                               ENDOP
/* RES  4,E        */ OP(CB,a3) THEN E = res(4, E);                                               ENDOP
/* RES  4,H        */ OP(CB,a4) THEN H = res(4, H);                                               ENDOP
/* RES  4,L        */ OP(CB,a5) THEN L = res(4, L);                                               ENDOP
/* RES  4,(HL)     */ OP(CB,a6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(4, TDAT8); CALL wm(); ENDOP
/* RES  4,A        */ OP(CB,a7) THEN A = res(4, A);                                               ENDOP

/* RES  5,B        */ OP(CB,a8) THEN B = res(5, B);                                               ENDOP
/* RES  5,C        */ OP(CB,a9) THEN C = res(5, C);                                               ENDOP
/* RES  5,D        */ OP(CB,aa) THEN D = res(5, D);                                               ENDOP
/* RES  5,E        */ OP(CB,ab) THEN E = res(5, E);                                               ENDOP
/* RES  5,H        */ OP(CB,ac) THEN H = res(5, H);                                               ENDOP
/* RES  5,L        */ OP(CB,ad) THEN L = res(5, L);                                               ENDOP
/* RES  5,(HL)     */ OP(CB,ae) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(5, TDAT8); CALL wm(); ENDOP
/* RES  5,A        */ OP(CB,af) THEN A = res(5, A);                                               ENDOP

/* RES  6,B        */ OP(CB,b0) THEN B = res(6, B);                                               ENDOP
/* RES  6,C        */ OP(CB,b1) THEN C = res(6, C);                                               ENDOP
/* RES  6,D        */ OP(CB,b2) THEN D = res(6, D);                                               ENDOP
/* RES  6,E        */ OP(CB,b3) THEN E = res(6, E);                                               ENDOP
/* RES  6,H        */ OP(CB,b4) THEN H = res(6, H);                                               ENDOP
/* RES  6,L        */ OP(CB,b5) THEN L = res(6, L);                                               ENDOP
/* RES  6,(HL)     */ OP(CB,b6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(6, TDAT8); CALL wm(); ENDOP
/* RES  6,A        */ OP(CB,b7) THEN A = res(6, A);                                               ENDOP

/* RES  7,B        */ OP(CB,b8) THEN B = res(7, B);                                               ENDOP
/* RES  7,C        */ OP(CB,b9) THEN C = res(7, C);                                               ENDOP
/* RES  7,D        */ OP(CB,ba) THEN D = res(7, D);                                               ENDOP
/* RES  7,E        */ OP(CB,bb) THEN E = res(7, E);                                               ENDOP
/* RES  7,H        */ OP(CB,bc) THEN H = res(7, H);                                               ENDOP
/* RES  7,L        */ OP(CB,bd) THEN L = res(7, L);                                               ENDOP
/* RES  7,(HL)     */ OP(CB,be) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=res(7, TDAT8); CALL wm(); ENDOP
/* RES  7,A        */ OP(CB,bf) THEN A = res(7, A);                                               ENDOP

/* SET  0,B        */ OP(CB,c0) THEN B = set(0, B);                                               ENDOP
/* SET  0,C        */ OP(CB,c1) THEN C = set(0, C);                                               ENDOP
/* SET  0,D        */ OP(CB,c2) THEN D = set(0, D);                                               ENDOP
/* SET  0,E        */ OP(CB,c3) THEN E = set(0, E);                                               ENDOP
/* SET  0,H        */ OP(CB,c4) THEN H = set(0, H);                                               ENDOP
/* SET  0,L        */ OP(CB,c5) THEN L = set(0, L);                                               ENDOP
/* SET  0,(HL)     */ OP(CB,c6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(0, TDAT8); CALL wm(); ENDOP
/* SET  0,A        */ OP(CB,c7) THEN A = set(0, A);                                               ENDOP

/* SET  1,B        */ OP(CB,c8) THEN B = set(1, B);                                               ENDOP
/* SET  1,C        */ OP(CB,c9) THEN C = set(1, C);                                               ENDOP
/* SET  1,D        */ OP(CB,ca) THEN D = set(1, D);                                               ENDOP
/* SET  1,E        */ OP(CB,cb) THEN E = set(1, E);                                               ENDOP
/* SET  1,H        */ OP(CB,cc) THEN H = set(1, H);                                               ENDOP
/* SET  1,L        */ OP(CB,cd) THEN L = set(1, L);                                               ENDOP
/* SET  1,(HL)     */ OP(CB,ce) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(1, TDAT8); CALL wm(); ENDOP
/* SET  1,A        */ OP(CB,cf) THEN A = set(1, A);                                               ENDOP

/* SET  2,B        */ OP(CB,d0) THEN B = set(2, B);                                               ENDOP
/* SET  2,C        */ OP(CB,d1) THEN C = set(2, C);                                               ENDOP
/* SET  2,D        */ OP(CB,d2) THEN D = set(2, D);                                               ENDOP
/* SET  2,E        */ OP(CB,d3) THEN E = set(2, E);                                               ENDOP
/* SET  2,H        */ OP(CB,d4) THEN H = set(2, H);                                               ENDOP
/* SET  2,L        */ OP(CB,d5) THEN L = set(2, L);                                               ENDOP
/* SET  2,(HL)     */ OP(CB,d6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(2, TDAT8); CALL wm(); ENDOP
/* SET  2,A        */ OP(CB,d7) THEN A = set(2, A);                                               ENDOP

/* SET  3,B        */ OP(CB,d8) THEN B = set(3, B);                                               ENDOP
/* SET  3,C        */ OP(CB,d9) THEN C = set(3, C);                                               ENDOP
/* SET  3,D        */ OP(CB,da) THEN D = set(3, D);                                               ENDOP
/* SET  3,E        */ OP(CB,db) THEN E = set(3, E);                                               ENDOP
/* SET  3,H        */ OP(CB,dc) THEN H = set(3, H);                                               ENDOP
/* SET  3,L        */ OP(CB,dd) THEN L = set(3, L);                                               ENDOP
/* SET  3,(HL)     */ OP(CB,de) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(3, TDAT8); CALL wm(); ENDOP
/* SET  3,A        */ OP(CB,df) THEN A = set(3, A);                                               ENDOP

/* SET  4,B        */ OP(CB,e0) THEN B = set(4, B);                                               ENDOP
/* SET  4,C        */ OP(CB,e1) THEN C = set(4, C);                                               ENDOP
/* SET  4,D        */ OP(CB,e2) THEN D = set(4, D);                                               ENDOP
/* SET  4,E        */ OP(CB,e3) THEN E = set(4, E);                                               ENDOP
/* SET  4,H        */ OP(CB,e4) THEN H = set(4, H);                                               ENDOP
/* SET  4,L        */ OP(CB,e5) THEN L = set(4, L);                                               ENDOP
/* SET  4,(HL)     */ OP(CB,e6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(4, TDAT8); CALL wm(); ENDOP
/* SET  4,A        */ OP(CB,e7) THEN A = set(4, A);                                               ENDOP

/* SET  5,B        */ OP(CB,e8) THEN B = set(5, B);                                               ENDOP
/* SET  5,C        */ OP(CB,e9) THEN C = set(5, C);                                               ENDOP
/* SET  5,D        */ OP(CB,ea) THEN D = set(5, D);                                               ENDOP
/* SET  5,E        */ OP(CB,eb) THEN E = set(5, E);                                               ENDOP
/* SET  5,H        */ OP(CB,ec) THEN H = set(5, H);                                               ENDOP
/* SET  5,L        */ OP(CB,ed) THEN L = set(5, L);                                               ENDOP
/* SET  5,(HL)     */ OP(CB,ee) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(5, TDAT8); CALL wm(); ENDOP
/* SET  5,A        */ OP(CB,ef) THEN A = set(5, A);                                               ENDOP

/* SET  6,B        */ OP(CB,f0) THEN B = set(6, B);                                               ENDOP
/* SET  6,C        */ OP(CB,f1) THEN C = set(6, C);                                               ENDOP
/* SET  6,D        */ OP(CB,f2) THEN D = set(6, D);                                               ENDOP
/* SET  6,E        */ OP(CB,f3) THEN E = set(6, E);                                               ENDOP
/* SET  6,H        */ OP(CB,f4) THEN H = set(6, H);                                               ENDOP
/* SET  6,L        */ OP(CB,f5) THEN L = set(6, L);                                               ENDOP
/* SET  6,(HL)     */ OP(CB,f6) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(6, TDAT8); CALL wm(); ENDOP
/* SET  6,A        */ OP(CB,f7) THEN A = set(6, A);                                               ENDOP

/* SET  7,B        */ OP(CB,f8) THEN B = set(7, B);                                               ENDOP
/* SET  7,C        */ OP(CB,f9) THEN C = set(7, C);                                               ENDOP
/* SET  7,D        */ OP(CB,fa) THEN D = set(7, D);                                               ENDOP
/* SET  7,E        */ OP(CB,fb) THEN E = set(7, E);                                               ENDOP
/* SET  7,H        */ OP(CB,fc) THEN H = set(7, H);                                               ENDOP
/* SET  7,L        */ OP(CB,fd) THEN L = set(7, L);                                               ENDOP
/* SET  7,(HL)     */ OP(CB,fe) THEN TADR=HL; CALL rm_reg(); THEN TDAT8=set(7, TDAT8); CALL wm(); ENDOP
/* SET  7,A        */ OP(CB,ff) THEN A = set(7, A);                                               ENDOP

/**********************************************************
* opcodes with DD/FD CB prefix
* rotate, shift and bit operations with (IX+o)
**********************************************************/
/* RLC  B=(XY+o)   */ OP(XY_CB,00) THEN TADR=m_ea; CALL rm_reg(); THEN B=rlc(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RLC  C=(XY+o)   */ OP(XY_CB,01) THEN TADR=m_ea; CALL rm_reg(); THEN C=rlc(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RLC  D=(XY+o)   */ OP(XY_CB,02) THEN TADR=m_ea; CALL rm_reg(); THEN D=rlc(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RLC  E=(XY+o)   */ OP(XY_CB,03) THEN TADR=m_ea; CALL rm_reg(); THEN E=rlc(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RLC  H=(XY+o)   */ OP(XY_CB,04) THEN TADR=m_ea; CALL rm_reg(); THEN H=rlc(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RLC  L=(XY+o)   */ OP(XY_CB,05) THEN TADR=m_ea; CALL rm_reg(); THEN L=rlc(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RLC  (XY+o)     */ OP(XY_CB,06) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=rlc(TDAT8); CALL wm();      ENDOP
/* RLC  A=(XY+o)   */ OP(XY_CB,07) THEN TADR=m_ea; CALL rm_reg(); THEN A=rlc(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RRC  B=(XY+o)   */ OP(XY_CB,08) THEN TADR=m_ea; CALL rm_reg(); THEN B=rrc(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RRC  C=(XY+o)   */ OP(XY_CB,09) THEN TADR=m_ea; CALL rm_reg(); THEN C=rrc(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RRC  D=(XY+o)   */ OP(XY_CB,0a) THEN TADR=m_ea; CALL rm_reg(); THEN D=rrc(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RRC  E=(XY+o)   */ OP(XY_CB,0b) THEN TADR=m_ea; CALL rm_reg(); THEN E=rrc(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RRC  H=(XY+o)   */ OP(XY_CB,0c) THEN TADR=m_ea; CALL rm_reg(); THEN H=rrc(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RRC  L=(XY+o)   */ OP(XY_CB,0d) THEN TADR=m_ea; CALL rm_reg(); THEN L=rrc(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RRC  (XY+o)     */ OP(XY_CB,0e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=rrc(TDAT8); CALL wm();      ENDOP
/* RRC  A=(XY+o)   */ OP(XY_CB,0f) THEN TADR=m_ea; CALL rm_reg(); THEN A=rrc(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RL   B=(XY+o)   */ OP(XY_CB,10) THEN TADR=m_ea; CALL rm_reg(); THEN B=rl(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RL   C=(XY+o)   */ OP(XY_CB,11) THEN TADR=m_ea; CALL rm_reg(); THEN C=rl(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RL   D=(XY+o)   */ OP(XY_CB,12) THEN TADR=m_ea; CALL rm_reg(); THEN D=rl(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RL   E=(XY+o)   */ OP(XY_CB,13) THEN TADR=m_ea; CALL rm_reg(); THEN E=rl(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RL   H=(XY+o)   */ OP(XY_CB,14) THEN TADR=m_ea; CALL rm_reg(); THEN H=rl(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RL   L=(XY+o)   */ OP(XY_CB,15) THEN TADR=m_ea; CALL rm_reg(); THEN L=rl(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RL   (XY+o)     */ OP(XY_CB,16) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=rl(TDAT8); CALL wm();      ENDOP
/* RL   A=(XY+o)   */ OP(XY_CB,17) THEN TADR=m_ea; CALL rm_reg(); THEN A=rl(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RR   B=(XY+o)   */ OP(XY_CB,18) THEN TADR=m_ea; CALL rm_reg(); THEN B=rr(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RR   C=(XY+o)   */ OP(XY_CB,19) THEN TADR=m_ea; CALL rm_reg(); THEN C=rr(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RR   D=(XY+o)   */ OP(XY_CB,1a) THEN TADR=m_ea; CALL rm_reg(); THEN D=rr(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RR   E=(XY+o)   */ OP(XY_CB,1b) THEN TADR=m_ea; CALL rm_reg(); THEN E=rr(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RR   H=(XY+o)   */ OP(XY_CB,1c) THEN TADR=m_ea; CALL rm_reg(); THEN H=rr(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RR   L=(XY+o)   */ OP(XY_CB,1d) THEN TADR=m_ea; CALL rm_reg(); THEN L=rr(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RR   (XY+o)     */ OP(XY_CB,1e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=rr(TDAT8); CALL wm();      ENDOP
/* RR   A=(XY+o)   */ OP(XY_CB,1f) THEN TADR=m_ea; CALL rm_reg(); THEN A=rr(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SLA  B=(XY+o)   */ OP(XY_CB,20) THEN TADR=m_ea; CALL rm_reg(); THEN B=sla(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SLA  C=(XY+o)   */ OP(XY_CB,21) THEN TADR=m_ea; CALL rm_reg(); THEN C=sla(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SLA  D=(XY+o)   */ OP(XY_CB,22) THEN TADR=m_ea; CALL rm_reg(); THEN D=sla(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SLA  E=(XY+o)   */ OP(XY_CB,23) THEN TADR=m_ea; CALL rm_reg(); THEN E=sla(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SLA  H=(XY+o)   */ OP(XY_CB,24) THEN TADR=m_ea; CALL rm_reg(); THEN H=sla(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SLA  L=(XY+o)   */ OP(XY_CB,25) THEN TADR=m_ea; CALL rm_reg(); THEN L=sla(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SLA  (XY+o)     */ OP(XY_CB,26) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=sla(TDAT8); CALL wm();      ENDOP
/* SLA  A=(XY+o)   */ OP(XY_CB,27) THEN TADR=m_ea; CALL rm_reg(); THEN A=sla(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SRA  B=(XY+o)   */ OP(XY_CB,28) THEN TADR=m_ea; CALL rm_reg(); THEN B=sra(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SRA  C=(XY+o)   */ OP(XY_CB,29) THEN TADR=m_ea; CALL rm_reg(); THEN C=sra(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SRA  D=(XY+o)   */ OP(XY_CB,2a) THEN TADR=m_ea; CALL rm_reg(); THEN D=sra(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SRA  E=(XY+o)   */ OP(XY_CB,2b) THEN TADR=m_ea; CALL rm_reg(); THEN E=sra(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SRA  H=(XY+o)   */ OP(XY_CB,2c) THEN TADR=m_ea; CALL rm_reg(); THEN H=sra(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SRA  L=(XY+o)   */ OP(XY_CB,2d) THEN TADR=m_ea; CALL rm_reg(); THEN L=sra(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SRA  (XY+o)     */ OP(XY_CB,2e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=sra(TDAT8); CALL wm();      ENDOP
/* SRA  A=(XY+o)   */ OP(XY_CB,2f) THEN TADR=m_ea; CALL rm_reg(); THEN A=sra(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SLL  B=(XY+o)   */ OP(XY_CB,30) THEN TADR=m_ea; CALL rm_reg(); THEN B=sll(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SLL  C=(XY+o)   */ OP(XY_CB,31) THEN TADR=m_ea; CALL rm_reg(); THEN C=sll(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SLL  D=(XY+o)   */ OP(XY_CB,32) THEN TADR=m_ea; CALL rm_reg(); THEN D=sll(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SLL  E=(XY+o)   */ OP(XY_CB,33) THEN TADR=m_ea; CALL rm_reg(); THEN E=sll(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SLL  H=(XY+o)   */ OP(XY_CB,34) THEN TADR=m_ea; CALL rm_reg(); THEN H=sll(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SLL  L=(XY+o)   */ OP(XY_CB,35) THEN TADR=m_ea; CALL rm_reg(); THEN L=sll(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SLL  (XY+o)     */ OP(XY_CB,36) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=sll(TDAT8); CALL wm();      ENDOP
/* SLL  A=(XY+o)   */ OP(XY_CB,37) THEN TADR=m_ea; CALL rm_reg(); THEN A=sll(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SRL  B=(XY+o)   */ OP(XY_CB,38) THEN TADR=m_ea; CALL rm_reg(); THEN B=srl(TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SRL  C=(XY+o)   */ OP(XY_CB,39) THEN TADR=m_ea; CALL rm_reg(); THEN C=srl(TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SRL  D=(XY+o)   */ OP(XY_CB,3a) THEN TADR=m_ea; CALL rm_reg(); THEN D=srl(TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SRL  E=(XY+o)   */ OP(XY_CB,3b) THEN TADR=m_ea; CALL rm_reg(); THEN E=srl(TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SRL  H=(XY+o)   */ OP(XY_CB,3c) THEN TADR=m_ea; CALL rm_reg(); THEN H=srl(TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SRL  L=(XY+o)   */ OP(XY_CB,3d) THEN TADR=m_ea; CALL rm_reg(); THEN L=srl(TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SRL  (XY+o)     */ OP(XY_CB,3e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=srl(TDAT8); CALL wm();      ENDOP
/* SRL  A=(XY+o)   */ OP(XY_CB,3f) THEN TADR=m_ea; CALL rm_reg(); THEN A=srl(TDAT8); TDAT8=A; CALL wm(); ENDOP

/* BIT  0,(XY+o)   */ OP(XY_CB,40) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,41) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,42) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,43) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,44) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,45) JPP(XY_CB, 46)
/* BIT  0,(XY+o)   */ OP(XY_CB,46) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(0, TDAT8); ENDOP
/* BIT  0,(XY+o)   */ OP(XY_CB,47) JPP(XY_CB, 46)

/* BIT  1,(XY+o)   */ OP(XY_CB,48) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,49) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4a) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4b) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4c) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4d) JPP(XY_CB, 4e)
/* BIT  1,(XY+o)   */ OP(XY_CB,4e) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(1, TDAT8); ENDOP
/* BIT  1,(XY+o)   */ OP(XY_CB,4f) JPP(XY_CB, 4e)

/* BIT  2,(XY+o)   */ OP(XY_CB,50) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,51) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,52) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,53) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,54) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,55) JPP(XY_CB, 56)
/* BIT  2,(XY+o)   */ OP(XY_CB,56) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(2, TDAT8); ENDOP
/* BIT  2,(XY+o)   */ OP(XY_CB,57) JPP(XY_CB, 56)

/* BIT  3,(XY+o)   */ OP(XY_CB,58) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,59) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5a) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5b) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5c) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5d) JPP(XY_CB, 5e)
/* BIT  3,(XY+o)   */ OP(XY_CB,5e) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(3, TDAT8); ENDOP
/* BIT  3,(XY+o)   */ OP(XY_CB,5f) JPP(XY_CB, 5e)

/* BIT  4,(XY+o)   */ OP(XY_CB,60) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,61) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,62) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,63) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,64) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,65) JPP(XY_CB, 66)
/* BIT  4,(XY+o)   */ OP(XY_CB,66) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(4, TDAT8); ENDOP
/* BIT  4,(XY+o)   */ OP(XY_CB,67) JPP(XY_CB, 66)

/* BIT  5,(XY+o)   */ OP(XY_CB,68) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,69) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6a) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6b) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6c) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6d) JPP(XY_CB, 6e)
/* BIT  5,(XY+o)   */ OP(XY_CB,6e) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(5, TDAT8); ENDOP
/* BIT  5,(XY+o)   */ OP(XY_CB,6f) JPP(XY_CB, 6e)

/* BIT  6,(XY+o)   */ OP(XY_CB,70) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,71) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,72) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,73) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,74) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,75) JPP(XY_CB, 76)
/* BIT  6,(XY+o)   */ OP(XY_CB,76) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(6, TDAT8); ENDOP
/* BIT  6,(XY+o)   */ OP(XY_CB,77) JPP(XY_CB, 76)

/* BIT  7,(XY+o)   */ OP(XY_CB,78) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,79) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7a) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7b) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7c) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7d) JPP(XY_CB, 7e)
/* BIT  7,(XY+o)   */ OP(XY_CB,7e) THEN TADR=m_ea; CALL rm_reg(); THEN bit_xy(7, TDAT8); ENDOP
/* BIT  7,(XY+o)   */ OP(XY_CB,7f) JPP(XY_CB, 7e)

/* RES  0,B=(XY+o) */ OP(XY_CB,80) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(0, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  0,C=(XY+o) */ OP(XY_CB,81) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(0, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  0,D=(XY+o) */ OP(XY_CB,82) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(0, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  0,E=(XY+o) */ OP(XY_CB,83) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(0, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  0,H=(XY+o) */ OP(XY_CB,84) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(0, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  0,L=(XY+o) */ OP(XY_CB,85) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(0, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  0,(XY+o)   */ OP(XY_CB,86) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(0, TDAT8); CALL wm();      ENDOP
/* RES  0,A=(XY+o) */ OP(XY_CB,87) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(0, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  1,B=(XY+o) */ OP(XY_CB,88) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(1, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  1,C=(XY+o) */ OP(XY_CB,89) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(1, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  1,D=(XY+o) */ OP(XY_CB,8a) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(1, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  1,E=(XY+o) */ OP(XY_CB,8b) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(1, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  1,H=(XY+o) */ OP(XY_CB,8c) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(1, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  1,L=(XY+o) */ OP(XY_CB,8d) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(1, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  1,(XY+o)   */ OP(XY_CB,8e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(1, TDAT8); CALL wm();      ENDOP
/* RES  1,A=(XY+o) */ OP(XY_CB,8f) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(1, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  2,B=(XY+o) */ OP(XY_CB,90) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(2, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  2,C=(XY+o) */ OP(XY_CB,91) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(2, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  2,D=(XY+o) */ OP(XY_CB,92) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(2, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  2,E=(XY+o) */ OP(XY_CB,93) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(2, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  2,H=(XY+o) */ OP(XY_CB,94) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(2, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  2,L=(XY+o) */ OP(XY_CB,95) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(2, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  2,(XY+o)   */ OP(XY_CB,96) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(2, TDAT8); CALL wm();      ENDOP
/* RES  2,A=(XY+o) */ OP(XY_CB,97) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(2, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  3,B=(XY+o) */ OP(XY_CB,98) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(3, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  3,C=(XY+o) */ OP(XY_CB,99) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(3, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  3,D=(XY+o) */ OP(XY_CB,9a) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(3, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  3,E=(XY+o) */ OP(XY_CB,9b) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(3, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  3,H=(XY+o) */ OP(XY_CB,9c) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(3, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  3,L=(XY+o) */ OP(XY_CB,9d) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(3, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  3,(XY+o)   */ OP(XY_CB,9e) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(3, TDAT8); CALL wm();      ENDOP
/* RES  3,A=(XY+o) */ OP(XY_CB,9f) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(3, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  4,B=(XY+o) */ OP(XY_CB,a0) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(4, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  4,C=(XY+o) */ OP(XY_CB,a1) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(4, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  4,D=(XY+o) */ OP(XY_CB,a2) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(4, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  4,E=(XY+o) */ OP(XY_CB,a3) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(4, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  4,H=(XY+o) */ OP(XY_CB,a4) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(4, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  4,L=(XY+o) */ OP(XY_CB,a5) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(4, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  4,(XY+o)   */ OP(XY_CB,a6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(4, TDAT8); CALL wm();      ENDOP
/* RES  4,A=(XY+o) */ OP(XY_CB,a7) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(4, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  5,B=(XY+o) */ OP(XY_CB,a8) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(5, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  5,C=(XY+o) */ OP(XY_CB,a9) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(5, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  5,D=(XY+o) */ OP(XY_CB,aa) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(5, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  5,E=(XY+o) */ OP(XY_CB,ab) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(5, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  5,H=(XY+o) */ OP(XY_CB,ac) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(5, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  5,L=(XY+o) */ OP(XY_CB,ad) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(5, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  5,(XY+o)   */ OP(XY_CB,ae) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(5, TDAT8); CALL wm();      ENDOP
/* RES  5,A=(XY+o) */ OP(XY_CB,af) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(5, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  6,B=(XY+o) */ OP(XY_CB,b0) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(6, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  6,C=(XY+o) */ OP(XY_CB,b1) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(6, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  6,D=(XY+o) */ OP(XY_CB,b2) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(6, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  6,E=(XY+o) */ OP(XY_CB,b3) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(6, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  6,H=(XY+o) */ OP(XY_CB,b4) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(6, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  6,L=(XY+o) */ OP(XY_CB,b5) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(6, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  6,(XY+o)   */ OP(XY_CB,b6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(6, TDAT8); CALL wm();      ENDOP
/* RES  6,A=(XY+o) */ OP(XY_CB,b7) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(6, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* RES  7,B=(XY+o) */ OP(XY_CB,b8) THEN TADR=m_ea; CALL rm_reg(); THEN B=res(7, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* RES  7,C=(XY+o) */ OP(XY_CB,b9) THEN TADR=m_ea; CALL rm_reg(); THEN C=res(7, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* RES  7,D=(XY+o) */ OP(XY_CB,ba) THEN TADR=m_ea; CALL rm_reg(); THEN D=res(7, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* RES  7,E=(XY+o) */ OP(XY_CB,bb) THEN TADR=m_ea; CALL rm_reg(); THEN E=res(7, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* RES  7,H=(XY+o) */ OP(XY_CB,bc) THEN TADR=m_ea; CALL rm_reg(); THEN H=res(7, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* RES  7,L=(XY+o) */ OP(XY_CB,bd) THEN TADR=m_ea; CALL rm_reg(); THEN L=res(7, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* RES  7,(XY+o)   */ OP(XY_CB,be) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=res(7, TDAT8); CALL wm();      ENDOP
/* RES  7,A=(XY+o) */ OP(XY_CB,bf) THEN TADR=m_ea; CALL rm_reg(); THEN A=res(7, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  0,B=(XY+o) */ OP(XY_CB,c0) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(0, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  0,C=(XY+o) */ OP(XY_CB,c1) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(0, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  0,D=(XY+o) */ OP(XY_CB,c2) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(0, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  0,E=(XY+o) */ OP(XY_CB,c3) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(0, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  0,H=(XY+o) */ OP(XY_CB,c4) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(0, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  0,L=(XY+o) */ OP(XY_CB,c5) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(0, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  0,(XY+o)   */ OP(XY_CB,c6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(0, TDAT8); CALL wm();      ENDOP
/* SET  0,A=(XY+o) */ OP(XY_CB,c7) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(0, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  1,B=(XY+o) */ OP(XY_CB,c8) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(1, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  1,C=(XY+o) */ OP(XY_CB,c9) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(1, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  1,D=(XY+o) */ OP(XY_CB,ca) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(1, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  1,E=(XY+o) */ OP(XY_CB,cb) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(1, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  1,H=(XY+o) */ OP(XY_CB,cc) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(1, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  1,L=(XY+o) */ OP(XY_CB,cd) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(1, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  1,(XY+o)   */ OP(XY_CB,ce) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(1, TDAT8); CALL wm();      ENDOP
/* SET  1,A=(XY+o) */ OP(XY_CB,cf) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(1, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  2,B=(XY+o) */ OP(XY_CB,d0) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(2, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  2,C=(XY+o) */ OP(XY_CB,d1) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(2, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  2,D=(XY+o) */ OP(XY_CB,d2) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(2, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  2,E=(XY+o) */ OP(XY_CB,d3) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(2, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  2,H=(XY+o) */ OP(XY_CB,d4) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(2, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  2,L=(XY+o) */ OP(XY_CB,d5) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(2, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  2,(XY+o)   */ OP(XY_CB,d6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(2, TDAT8); CALL wm();      ENDOP
/* SET  2,A=(XY+o) */ OP(XY_CB,d7) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(2, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  3,B=(XY+o) */ OP(XY_CB,d8) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(3, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  3,C=(XY+o) */ OP(XY_CB,d9) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(3, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  3,D=(XY+o) */ OP(XY_CB,da) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(3, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  3,E=(XY+o) */ OP(XY_CB,db) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(3, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  3,H=(XY+o) */ OP(XY_CB,dc) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(3, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  3,L=(XY+o) */ OP(XY_CB,dd) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(3, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  3,(XY+o)   */ OP(XY_CB,de) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(3, TDAT8); CALL wm();      ENDOP
/* SET  3,A=(XY+o) */ OP(XY_CB,df) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(3, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  4,B=(XY+o) */ OP(XY_CB,e0) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(4, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  4,C=(XY+o) */ OP(XY_CB,e1) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(4, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  4,D=(XY+o) */ OP(XY_CB,e2) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(4, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  4,E=(XY+o) */ OP(XY_CB,e3) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(4, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  4,H=(XY+o) */ OP(XY_CB,e4) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(4, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  4,L=(XY+o) */ OP(XY_CB,e5) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(4, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  4,(XY+o)   */ OP(XY_CB,e6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(4, TDAT8); CALL wm();      ENDOP
/* SET  4,A=(XY+o) */ OP(XY_CB,e7) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(4, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  5,B=(XY+o) */ OP(XY_CB,e8) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(5, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  5,C=(XY+o) */ OP(XY_CB,e9) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(5, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  5,D=(XY+o) */ OP(XY_CB,ea) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(5, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  5,E=(XY+o) */ OP(XY_CB,eb) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(5, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  5,H=(XY+o) */ OP(XY_CB,ec) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(5, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  5,L=(XY+o) */ OP(XY_CB,ed) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(5, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  5,(XY+o)   */ OP(XY_CB,ee) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(5, TDAT8); CALL wm();      ENDOP
/* SET  5,A=(XY+o) */ OP(XY_CB,ef) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(5, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  6,B=(XY+o) */ OP(XY_CB,f0) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(6, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  6,C=(XY+o) */ OP(XY_CB,f1) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(6, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  6,D=(XY+o) */ OP(XY_CB,f2) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(6, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  6,E=(XY+o) */ OP(XY_CB,f3) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(6, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  6,H=(XY+o) */ OP(XY_CB,f4) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(6, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  6,L=(XY+o) */ OP(XY_CB,f5) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(6, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  6,(XY+o)   */ OP(XY_CB,f6) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(6, TDAT8); CALL wm();      ENDOP
/* SET  6,A=(XY+o) */ OP(XY_CB,f7) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(6, TDAT8); TDAT8=A; CALL wm(); ENDOP

/* SET  7,B=(XY+o) */ OP(XY_CB,f8) THEN TADR=m_ea; CALL rm_reg(); THEN B=set(7, TDAT8); TDAT8=B; CALL wm(); ENDOP
/* SET  7,C=(XY+o) */ OP(XY_CB,f9) THEN TADR=m_ea; CALL rm_reg(); THEN C=set(7, TDAT8); TDAT8=C; CALL wm(); ENDOP
/* SET  7,D=(XY+o) */ OP(XY_CB,fa) THEN TADR=m_ea; CALL rm_reg(); THEN D=set(7, TDAT8); TDAT8=D; CALL wm(); ENDOP
/* SET  7,E=(XY+o) */ OP(XY_CB,fb) THEN TADR=m_ea; CALL rm_reg(); THEN E=set(7, TDAT8); TDAT8=E; CALL wm(); ENDOP
/* SET  7,H=(XY+o) */ OP(XY_CB,fc) THEN TADR=m_ea; CALL rm_reg(); THEN H=set(7, TDAT8); TDAT8=H; CALL wm(); ENDOP
/* SET  7,L=(XY+o) */ OP(XY_CB,fd) THEN TADR=m_ea; CALL rm_reg(); THEN L=set(7, TDAT8); TDAT8=L; CALL wm(); ENDOP
/* SET  7,(XY+o)   */ OP(XY_CB,fe) THEN TADR=m_ea; CALL rm_reg(); THEN TDAT8=set(7, TDAT8); CALL wm();      ENDOP
/* SET  7,A=(XY+o) */ OP(XY_CB,ff) THEN TADR=m_ea; CALL rm_reg(); THEN A=set(7, TDAT8); TDAT8=A; CALL wm(); ENDOP

/**********************************************************
 * IX register related opcodes (DD prefix)
 **********************************************************/
/* DB   DD         */ OP(DD,00) THEN illegal_1(); JP(00)
/* DB   DD         */ OP(DD,01) THEN illegal_1(); JP(01)
/* DB   DD         */ OP(DD,02) THEN illegal_1(); JP(02)
/* DB   DD         */ OP(DD,03) THEN illegal_1(); JP(03)
/* DB   DD         */ OP(DD,04) THEN illegal_1(); JP(04)
/* DB   DD         */ OP(DD,05) THEN illegal_1(); JP(05)
/* DB   DD         */ OP(DD,06) THEN illegal_1(); JP(06)
/* DB   DD         */ OP(DD,07) THEN illegal_1(); JP(07)

/* DB   DD         */ OP(DD,08) THEN illegal_1(); JP(08)
/* ADD  IX,BC      */ OP(DD,09) THEN TDAT=IX; TDAT2=BC; CALL add16(); THEN IX=TDAT;    ENDOP
/* DB   DD         */ OP(DD,0a) THEN illegal_1(); JP(0a)
/* DB   DD         */ OP(DD,0b) THEN illegal_1(); JP(0b)
/* DB   DD         */ OP(DD,0c) THEN illegal_1(); JP(0c)
/* DB   DD         */ OP(DD,0d) THEN illegal_1(); JP(0d)
/* DB   DD         */ OP(DD,0e) THEN illegal_1(); JP(0e)
/* DB   DD         */ OP(DD,0f) THEN illegal_1(); JP(0f)

/* DB   DD         */ OP(DD,10) THEN illegal_1(); JP(10)
/* DB   DD         */ OP(DD,11) THEN illegal_1(); JP(11)
/* DB   DD         */ OP(DD,12) THEN illegal_1(); JP(12)
/* DB   DD         */ OP(DD,13) THEN illegal_1(); JP(13)
/* DB   DD         */ OP(DD,14) THEN illegal_1(); JP(14)
/* DB   DD         */ OP(DD,15) THEN illegal_1(); JP(15)
/* DB   DD         */ OP(DD,16) THEN illegal_1(); JP(16)
/* DB   DD         */ OP(DD,17) THEN illegal_1(); JP(17)

/* DB   DD         */ OP(DD,18) THEN illegal_1(); JP(18)
/* ADD  IX,DE      */ OP(DD,19) THEN TDAT=IX; TDAT2=DE; CALL add16(); THEN IX=TDAT;    ENDOP
/* DB   DD         */ OP(DD,1a) THEN illegal_1(); JP(1a)
/* DB   DD         */ OP(DD,1b) THEN illegal_1(); JP(1b)
/* DB   DD         */ OP(DD,1c) THEN illegal_1(); JP(1c)
/* DB   DD         */ OP(DD,1d) THEN illegal_1(); JP(1d)
/* DB   DD         */ OP(DD,1e) THEN illegal_1(); JP(1e)
/* DB   DD         */ OP(DD,1f) THEN illegal_1(); JP(1f)

/* DB   DD         */ OP(DD,20) THEN illegal_1(); JP(20)
/* LD   IX,w       */ OP(DD,21) CALL arg16(); THEN IX = TDAT;                          ENDOP
/* LD   (w),IX     */ OP(DD,22) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=IX; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* INC  IX         */ OP(DD,23) CALL nomreq_ir(2); THEN IX++;                          ENDOP
/* INC  HX         */ OP(DD,24) THEN inc(HX);                                          ENDOP
/* DEC  HX         */ OP(DD,25) THEN dec(HX);                                          ENDOP
/* LD   HX,n       */ OP(DD,26) CALL arg(); THEN HX = TDAT8;                           ENDOP
/* DB   DD         */ OP(DD,27) THEN illegal_1(); JP(27)

/* DB   DD         */ OP(DD,28) THEN illegal_1(); JP(28)
/* ADD  IX,IX      */ OP(DD,29) THEN TDAT=IX; TDAT2=IX; CALL add16(); THEN IX=TDAT;    ENDOP
/* LD   IX,(w)     */ OP(DD,2a) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN IX=TDAT; WZ = m_ea + 1; ENDOP
/* DEC  IX         */ OP(DD,2b) CALL nomreq_ir(2); THEN IX--;                          ENDOP
/* INC  LX         */ OP(DD,2c) THEN inc(LX);                                          ENDOP
/* DEC  LX         */ OP(DD,2d) THEN dec(LX);                                          ENDOP
/* LD   LX,n       */ OP(DD,2e) CALL arg(); THEN LX = TDAT8;                           ENDOP
/* DB   DD         */ OP(DD,2f) THEN illegal_1(); JP(2f)

/* DB   DD         */ OP(DD,30) THEN illegal_1(); JP(30)
/* DB   DD         */ OP(DD,31) THEN illegal_1(); JP(31)
/* DB   DD         */ OP(DD,32) THEN illegal_1(); JP(32)
/* DB   DD         */ OP(DD,33) THEN illegal_1(); JP(33)
/* INC  (IX+o)     */ OP(DD,34) CALL eax(); THEN TADR=PCD-1; CALL nomreq_addr(5); THEN TADR=m_ea; CALL rm_reg(); THEN inc(TDAT8); CALL wm(); ENDOP
/* DEC  (IX+o)     */ OP(DD,35) CALL eax(); THEN TADR=PCD-1; CALL nomreq_addr(5); THEN TADR=m_ea; CALL rm_reg(); THEN dec(TDAT8); CALL wm(); ENDOP
/* LD   (IX+o),n   */ OP(DD,36) CALL eax(); CALL arg(); THEN TADR=PCD-1; CALL nomreq_addr(2); THEN TADR=m_ea; CALL wm(); ENDOP
/* DB   DD         */ OP(DD,37) THEN illegal_1(); JP(37)

/* DB   DD         */ OP(DD,38) THEN illegal_1(); JP(38)
/* ADD  IX,SP      */ OP(DD,39) THEN TDAT=IX; TDAT2=SP; CALL add16(); THEN IX=TDAT;   ENDOP
/* DB   DD         */ OP(DD,3a) THEN illegal_1(); JP(3a)
/* DB   DD         */ OP(DD,3b) THEN illegal_1(); JP(3b)
/* DB   DD         */ OP(DD,3c) THEN illegal_1(); JP(3c)
/* DB   DD         */ OP(DD,3d) THEN illegal_1(); JP(3d)
/* DB   DD         */ OP(DD,3e) THEN illegal_1(); JP(3e)
/* DB   DD         */ OP(DD,3f) THEN illegal_1(); JP(3f)

/* DB   DD         */ OP(DD,40) THEN illegal_1(); JP(40)
/* DB   DD         */ OP(DD,41) THEN illegal_1(); JP(41)
/* DB   DD         */ OP(DD,42) THEN illegal_1(); JP(42)
/* DB   DD         */ OP(DD,43) THEN illegal_1(); JP(43)
/* LD   B,HX       */ OP(DD,44) THEN B = HX;                                          ENDOP
/* LD   B,LX       */ OP(DD,45) THEN B = LX;                                          ENDOP
/* LD   B,(IX+o)   */ OP(DD,46) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN B = TDAT8; ENDOP
/* DB   DD         */ OP(DD,47) THEN illegal_1(); JP(47)

/* DB   DD         */ OP(DD,48) THEN illegal_1(); JP(48)
/* DB   DD         */ OP(DD,49) THEN illegal_1(); JP(49)
/* DB   DD         */ OP(DD,4a) THEN illegal_1(); JP(4a)
/* DB   DD         */ OP(DD,4b) THEN illegal_1(); JP(4b)
/* LD   C,HX       */ OP(DD,4c) THEN C = HX;                                          ENDOP
/* LD   C,LX       */ OP(DD,4d) THEN C = LX;                                          ENDOP
/* LD   C,(IX+o)   */ OP(DD,4e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN C = TDAT8; ENDOP
/* DB   DD         */ OP(DD,4f) THEN illegal_1(); JP(4f)

/* DB   DD         */ OP(DD,50) THEN illegal_1(); JP(50)
/* DB   DD         */ OP(DD,51) THEN illegal_1(); JP(51)
/* DB   DD         */ OP(DD,52) THEN illegal_1(); JP(52)
/* DB   DD         */ OP(DD,53) THEN illegal_1(); JP(53)
/* LD   D,HX       */ OP(DD,54) THEN D = HX;                                          ENDOP
/* LD   D,LX       */ OP(DD,55) THEN D = LX;                                          ENDOP
/* LD   D,(IX+o)   */ OP(DD,56) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN D = TDAT8; ENDOP
/* DB   DD         */ OP(DD,57) THEN illegal_1(); JP(57)

/* DB   DD         */ OP(DD,58) THEN illegal_1(); JP(58)
/* DB   DD         */ OP(DD,59) THEN illegal_1(); JP(59)
/* DB   DD         */ OP(DD,5a) THEN illegal_1(); JP(5a)
/* DB   DD         */ OP(DD,5b) THEN illegal_1(); JP(5b)
/* LD   E,HX       */ OP(DD,5c) THEN E = HX;                                          ENDOP
/* LD   E,LX       */ OP(DD,5d) THEN E = LX;                                          ENDOP
/* LD   E,(IX+o)   */ OP(DD,5e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN E = TDAT8; ENDOP
/* DB   DD         */ OP(DD,5f) THEN illegal_1(); JP(5f)

/* LD   HX,B       */ OP(DD,60) THEN HX = B;                                          ENDOP
/* LD   HX,C       */ OP(DD,61) THEN HX = C;                                          ENDOP
/* LD   HX,D       */ OP(DD,62) THEN HX = D;                                          ENDOP
/* LD   HX,E       */ OP(DD,63) THEN HX = E;                                          ENDOP
/* LD   HX,HX      */ OP(DD,64)                                                   ENDOP
/* LD   HX,LX      */ OP(DD,65) THEN HX = LX;                                         ENDOP
/* LD   H,(IX+o)   */ OP(DD,66) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN H = TDAT8; ENDOP
/* LD   HX,A       */ OP(DD,67) THEN HX = A;                                          ENDOP

/* LD   LX,B       */ OP(DD,68) THEN LX = B;                                          ENDOP
/* LD   LX,C       */ OP(DD,69) THEN LX = C;                                          ENDOP
/* LD   LX,D       */ OP(DD,6a) THEN LX = D;                                          ENDOP
/* LD   LX,E       */ OP(DD,6b) THEN LX = E;                                          ENDOP
/* LD   LX,HX      */ OP(DD,6c) THEN LX = HX;                                         ENDOP
/* LD   LX,LX      */ OP(DD,6d)                                                   ENDOP
/* LD   L,(IX+o)   */ OP(DD,6e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN L = TDAT8; ENDOP
/* LD   LX,A       */ OP(DD,6f) THEN LX = A;                                          ENDOP

/* LD   (IX+o),B   */ OP(DD,70) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = B; CALL wm(); ENDOP
/* LD   (IX+o),C   */ OP(DD,71) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = C; CALL wm(); ENDOP
/* LD   (IX+o),D   */ OP(DD,72) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = D; CALL wm(); ENDOP
/* LD   (IX+o),E   */ OP(DD,73) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = E; CALL wm(); ENDOP
/* LD   (IX+o),H   */ OP(DD,74) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = H; CALL wm(); ENDOP
/* LD   (IX+o),L   */ OP(DD,75) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = L; CALL wm(); ENDOP
/* DB   DD         */ OP(DD,76) THEN illegal_1(); JP(76)
/* LD   (IX+o),A   */ OP(DD,77) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = A; CALL wm(); ENDOP

/* DB   DD         */ OP(DD,78) THEN illegal_1(); JP(78)
/* DB   DD         */ OP(DD,79) THEN illegal_1(); JP(79)
/* DB   DD         */ OP(DD,7a) THEN illegal_1(); JP(7a)
/* DB   DD         */ OP(DD,7b) THEN illegal_1(); JP(7b)
/* LD   A,HX       */ OP(DD,7c) THEN A = HX;                                          ENDOP
/* LD   A,LX       */ OP(DD,7d) THEN A = LX;                                          ENDOP
/* LD   A,(IX+o)   */ OP(DD,7e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN A = TDAT8; ENDOP
/* DB   DD         */ OP(DD,7f) THEN illegal_1(); JP(7f)

/* DB   DD         */ OP(DD,80) THEN illegal_1(); JP(80)
/* DB   DD         */ OP(DD,81) THEN illegal_1(); JP(81)
/* DB   DD         */ OP(DD,82) THEN illegal_1(); JP(82)
/* DB   DD         */ OP(DD,83) THEN illegal_1(); JP(83)
/* ADD  A,HX       */ OP(DD,84) THEN add_a(HX);                                       ENDOP
/* ADD  A,LX       */ OP(DD,85) THEN add_a(LX);                                       ENDOP
/* ADD  A,(IX+o)   */ OP(DD,86) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN add_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,87) THEN illegal_1(); JP(87)

/* DB   DD         */ OP(DD,88) THEN illegal_1(); JP(88)
/* DB   DD         */ OP(DD,89) THEN illegal_1(); JP(89)
/* DB   DD         */ OP(DD,8a) THEN illegal_1(); JP(8a)
/* DB   DD         */ OP(DD,8b) THEN illegal_1(); JP(8b)
/* ADC  A,HX       */ OP(DD,8c) THEN adc_a(HX);                                       ENDOP
/* ADC  A,LX       */ OP(DD,8d) THEN adc_a(LX);                                       ENDOP
/* ADC  A,(IX+o)   */ OP(DD,8e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN adc_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,8f) THEN illegal_1(); JP(8f)

/* DB   DD         */ OP(DD,90) THEN illegal_1(); JP(90)
/* DB   DD         */ OP(DD,91) THEN illegal_1(); JP(91)
/* DB   DD         */ OP(DD,92) THEN illegal_1(); JP(92)
/* DB   DD         */ OP(DD,93) THEN illegal_1(); JP(93)
/* SUB  HX         */ OP(DD,94) THEN sub(HX);                                         ENDOP
/* SUB  LX         */ OP(DD,95) THEN sub(LX);                                         ENDOP
/* SUB  (IX+o)     */ OP(DD,96) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN sub(TDAT8); ENDOP
/* DB   DD         */ OP(DD,97) THEN illegal_1(); JP(97)

/* DB   DD         */ OP(DD,98) THEN illegal_1(); JP(98)
/* DB   DD         */ OP(DD,99) THEN illegal_1(); JP(99)
/* DB   DD         */ OP(DD,9a) THEN illegal_1(); JP(9a)
/* DB   DD         */ OP(DD,9b) THEN illegal_1(); JP(9b)
/* SBC  A,HX       */ OP(DD,9c) THEN sbc_a(HX);                                       ENDOP
/* SBC  A,LX       */ OP(DD,9d) THEN sbc_a(LX);                                       ENDOP
/* SBC  A,(IX+o)   */ OP(DD,9e) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN sbc_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,9f) THEN illegal_1(); JP(9f)

/* DB   DD         */ OP(DD,a0) THEN illegal_1(); JP(a0)
/* DB   DD         */ OP(DD,a1) THEN illegal_1(); JP(a1)
/* DB   DD         */ OP(DD,a2) THEN illegal_1(); JP(a2)
/* DB   DD         */ OP(DD,a3) THEN illegal_1(); JP(a3)
/* AND  HX         */ OP(DD,a4) THEN and_a(HX);                                       ENDOP
/* AND  LX         */ OP(DD,a5) THEN and_a(LX);                                       ENDOP
/* AND  (IX+o)     */ OP(DD,a6) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN and_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,a7) THEN illegal_1(); JP(a7)

/* DB   DD         */ OP(DD,a8) THEN illegal_1(); JP(a8)
/* DB   DD         */ OP(DD,a9) THEN illegal_1(); JP(a9)
/* DB   DD         */ OP(DD,aa) THEN illegal_1(); JP(aa)
/* DB   DD         */ OP(DD,ab) THEN illegal_1(); JP(ab)
/* XOR  HX         */ OP(DD,ac) THEN xor_a(HX);                                       ENDOP
/* XOR  LX         */ OP(DD,ad) THEN xor_a(LX);                                       ENDOP
/* XOR  (IX+o)     */ OP(DD,ae) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN xor_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,af) THEN illegal_1(); JP(af)

/* DB   DD         */ OP(DD,b0) THEN illegal_1(); JP(b0)
/* DB   DD         */ OP(DD,b1) THEN illegal_1(); JP(b1)
/* DB   DD         */ OP(DD,b2) THEN illegal_1(); JP(b2)
/* DB   DD         */ OP(DD,b3) THEN illegal_1(); JP(b3)
/* OR   HX         */ OP(DD,b4) THEN or_a(HX);                                        ENDOP
/* OR   LX         */ OP(DD,b5) THEN or_a(LX);                                        ENDOP
/* OR   (IX+o)     */ OP(DD,b6) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN or_a(TDAT8); ENDOP
/* DB   DD         */ OP(DD,b7) THEN illegal_1(); JP(b7)

/* DB   DD         */ OP(DD,b8) THEN illegal_1(); JP(b8)
/* DB   DD         */ OP(DD,b9) THEN illegal_1(); JP(b9)
/* DB   DD         */ OP(DD,ba) THEN illegal_1(); JP(ba)
/* DB   DD         */ OP(DD,bb) THEN illegal_1(); JP(bb)
/* CP   HX         */ OP(DD,bc) THEN cp(HX);                                          ENDOP
/* CP   LX         */ OP(DD,bd) THEN cp(LX);                                          ENDOP
/* CP   (IX+o)     */ OP(DD,be) CALL eax(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN cp(TDAT8); ENDOP
/* DB   DD         */ OP(DD,bf) THEN illegal_1(); JP(bf)

/* DB   DD         */ OP(DD,c0) THEN illegal_1(); JP(c0)
/* DB   DD         */ OP(DD,c1) THEN illegal_1(); JP(c1)
/* DB   DD         */ OP(DD,c2) THEN illegal_1(); JP(c2)
/* DB   DD         */ OP(DD,c3) THEN illegal_1(); JP(c3)
/* DB   DD         */ OP(DD,c4) THEN illegal_1(); JP(c4)
/* DB   DD         */ OP(DD,c5) THEN illegal_1(); JP(c5)
/* DB   DD         */ OP(DD,c6) THEN illegal_1(); JP(c6)
/* DB   DD         */ OP(DD,c7) THEN illegal_1(); JP(c7)

/* DB   DD         */ OP(DD,c8) THEN illegal_1(); JP(c8)
/* DB   DD         */ OP(DD,c9) THEN illegal_1(); JP(c9)
/* DB   DD         */ OP(DD,ca) THEN illegal_1(); JP(ca)
/* **   DD CB xx   */ OP(DD,cb) CALL eax(); CALL arg(); THEN TADR=PCD-1; CALL nomreq_addr(2); THENJP(XY_CB)
/* DB   DD         */ OP(DD,cc) THEN illegal_1(); JP(cc)
/* DB   DD         */ OP(DD,cd) THEN illegal_1(); JP(cd)
/* DB   DD         */ OP(DD,ce) THEN illegal_1(); JP(ce)
/* DB   DD         */ OP(DD,cf) THEN illegal_1(); JP(cf)

/* DB   DD         */ OP(DD,d0) THEN illegal_1(); JP(d0)
/* DB   DD         */ OP(DD,d1) THEN illegal_1(); JP(d1)
/* DB   DD         */ OP(DD,d2) THEN illegal_1(); JP(d2)
/* DB   DD         */ OP(DD,d3) THEN illegal_1(); JP(d3)
/* DB   DD         */ OP(DD,d4) THEN illegal_1(); JP(d4)
/* DB   DD         */ OP(DD,d5) THEN illegal_1(); JP(d5)
/* DB   DD         */ OP(DD,d6) THEN illegal_1(); JP(d6)
/* DB   DD         */ OP(DD,d7) THEN illegal_1(); JP(d7)

/* DB   DD         */ OP(DD,d8) THEN illegal_1(); JP(d8)
/* DB   DD         */ OP(DD,d9) THEN illegal_1(); JP(d9)
/* DB   DD         */ OP(DD,da) THEN illegal_1(); JP(da)
/* DB   DD         */ OP(DD,db) THEN illegal_1(); JP(db)
/* DB   DD         */ OP(DD,dc) THEN illegal_1(); JP(dc)
/* DB   DD         */ OP(DD,dd) THEN illegal_1(); JP(dd)
/* DB   DD         */ OP(DD,de) THEN illegal_1(); JP(de)
/* DB   DD         */ OP(DD,df) THEN illegal_1(); JP(df)

/* DB   DD         */ OP(DD,e0) THEN illegal_1(); JP(e0)
/* POP  IX         */ OP(DD,e1) CALL pop(); THEN IX=TDAT;                              ENDOP
/* DB   DD         */ OP(DD,e2) THEN illegal_1(); JP(e2)
/* EX   (SP),IX    */ OP(DD,e3) THEN TDAT=IX; CALL ex_sp(); THEN IX=TDAT;              ENDOP
/* DB   DD         */ OP(DD,e4) THEN illegal_1(); JP(e4)
/* PUSH IX         */ OP(DD,e5) THEN TDAT=IX; CALL push();                             ENDOP
/* DB   DD         */ OP(DD,e6) THEN illegal_1(); JP(e6)
/* DB   DD         */ OP(DD,e7) THEN illegal_1(); JP(e7)

/* DB   DD         */ OP(DD,e8) THEN illegal_1(); JP(e8)
/* JP   (IX)       */ OP(DD,e9) THEN PC = IX;                                          ENDOP
/* DB   DD         */ OP(DD,ea) THEN illegal_1(); JP(ea)
/* DB   DD         */ OP(DD,eb) THEN illegal_1(); JP(eb)
/* DB   DD         */ OP(DD,ec) THEN illegal_1(); JP(ec)
/* DB   DD         */ OP(DD,ed) THEN illegal_1(); JP(ed)
/* DB   DD         */ OP(DD,ee) THEN illegal_1(); JP(ee)
/* DB   DD         */ OP(DD,ef) THEN illegal_1(); JP(ef)

/* DB   DD         */ OP(DD,f0) THEN illegal_1(); JP(f0)
/* DB   DD         */ OP(DD,f1) THEN illegal_1(); JP(f1)
/* DB   DD         */ OP(DD,f2) THEN illegal_1(); JP(f2)
/* DB   DD         */ OP(DD,f3) THEN illegal_1(); JP(f3)
/* DB   DD         */ OP(DD,f4) THEN illegal_1(); JP(f4)
/* DB   DD         */ OP(DD,f5) THEN illegal_1(); JP(f5)
/* DB   DD         */ OP(DD,f6) THEN illegal_1(); JP(f6)
/* DB   DD         */ OP(DD,f7) THEN illegal_1(); JP(f7)

/* DB   DD         */ OP(DD,f8) THEN illegal_1(); JP(f8)
/* LD   SP,IX      */ OP(DD,f9) CALL nomreq_ir(2); THEN SP = IX;                       ENDOP
/* DB   DD         */ OP(DD,fa) THEN illegal_1(); JP(fa)
/* DB   DD         */ OP(DD,fb) THEN illegal_1(); JP(fb)
/* DB   DD         */ OP(DD,fc) THEN illegal_1(); JP(fc)
/* DB   DD         */ OP(DD,fd) THEN illegal_1(); JP(fd)
/* DB   DD         */ OP(DD,fe) THEN illegal_1(); JP(fe)
/* DB   DD         */ OP(DD,ff) THEN illegal_1(); JP(ff)

/**********************************************************
 * IY register related opcodes (FD prefix)
 **********************************************************/
/* DB   FD         */ OP(FD,00) THEN illegal_1(); JP(00)
/* DB   FD         */ OP(FD,01) THEN illegal_1(); JP(01)
/* DB   FD         */ OP(FD,02) THEN illegal_1(); JP(02)
/* DB   FD         */ OP(FD,03) THEN illegal_1(); JP(03)
/* DB   FD         */ OP(FD,04) THEN illegal_1(); JP(04)
/* DB   FD         */ OP(FD,05) THEN illegal_1(); JP(05)
/* DB   FD         */ OP(FD,06) THEN illegal_1(); JP(06)
/* DB   FD         */ OP(FD,07) THEN illegal_1(); JP(07)

/* DB   FD         */ OP(FD,08) THEN illegal_1(); JP(08)
/* ADD  IY,BC      */ OP(FD,09) THEN TDAT=IY; TDAT2=BC; CALL add16(); THEN IY=TDAT;    ENDOP
/* DB   FD         */ OP(FD,0a) THEN illegal_1(); JP(0a)
/* DB   FD         */ OP(FD,0b) THEN illegal_1(); JP(0b)
/* DB   FD         */ OP(FD,0c) THEN illegal_1(); JP(0c)
/* DB   FD         */ OP(FD,0d) THEN illegal_1(); JP(0d)
/* DB   FD         */ OP(FD,0e) THEN illegal_1(); JP(0e)
/* DB   FD         */ OP(FD,0f) THEN illegal_1(); JP(0f)

/* DB   FD         */ OP(FD,10) THEN illegal_1(); JP(10)
/* DB   FD         */ OP(FD,11) THEN illegal_1(); JP(11)
/* DB   FD         */ OP(FD,12) THEN illegal_1(); JP(12)
/* DB   FD         */ OP(FD,13) THEN illegal_1(); JP(13)
/* DB   FD         */ OP(FD,14) THEN illegal_1(); JP(14)
/* DB   FD         */ OP(FD,15) THEN illegal_1(); JP(15)
/* DB   FD         */ OP(FD,16) THEN illegal_1(); JP(16)
/* DB   FD         */ OP(FD,17) THEN illegal_1(); JP(17)

/* DB   FD         */ OP(FD,18) THEN illegal_1(); JP(18)
/* ADD  IY,DE      */ OP(FD,19) THEN TDAT=IY; TDAT2=DE; CALL add16(); THEN IY=TDAT;    ENDOP
/* DB   FD         */ OP(FD,1a) THEN illegal_1(); JP(1a)
/* DB   FD         */ OP(FD,1b) THEN illegal_1(); JP(1b)
/* DB   FD         */ OP(FD,1c) THEN illegal_1(); JP(1c)
/* DB   FD         */ OP(FD,1d) THEN illegal_1(); JP(1d)
/* DB   FD         */ OP(FD,1e) THEN illegal_1(); JP(1e)
/* DB   FD         */ OP(FD,1f) THEN illegal_1(); JP(1f)

/* DB   FD         */ OP(FD,20) THEN illegal_1(); JP(20)
/* LD   IY,w       */ OP(FD,21) CALL arg16(); THEN IY = TDAT;                          ENDOP
/* LD   (w),IY     */ OP(FD,22) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=IY; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* INC  IY         */ OP(FD,23) CALL nomreq_ir(2); THEN IY++;                          ENDOP
/* INC  HY         */ OP(FD,24) THEN inc(HY);                                          ENDOP
/* DEC  HY         */ OP(FD,25) THEN dec(HY);                                          ENDOP
/* LD   HY,n       */ OP(FD,26) CALL arg(); THEN HY = TDAT8;                           ENDOP
/* DB   FD         */ OP(FD,27) THEN illegal_1(); JP(27)

/* DB   FD         */ OP(FD,28) THEN illegal_1(); JP(28)
/* ADD  IY,IY      */ OP(FD,29) THEN TDAT=IY; TDAT2=IY; CALL add16(); THEN IY=TDAT;    ENDOP
/* LD   IY,(w)     */ OP(FD,2a) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN IY=TDAT; WZ = m_ea + 1; ENDOP
/* DEC  IY         */ OP(FD,2b) CALL nomreq_ir(2); THEN IY--;                          ENDOP
/* INC  LY         */ OP(FD,2c) THEN inc(LY);                                          ENDOP
/* DEC  LY         */ OP(FD,2d) THEN dec(LY);                                          ENDOP
/* LD   LY,n       */ OP(FD,2e) CALL arg(); THEN LY = TDAT8;                           ENDOP
/* DB   FD         */ OP(FD,2f) THEN illegal_1(); JP(2f)

/* DB   FD         */ OP(FD,30) THEN illegal_1(); JP(30)
/* DB   FD         */ OP(FD,31) THEN illegal_1(); JP(31)
/* DB   FD         */ OP(FD,32) THEN illegal_1(); JP(32)
/* DB   FD         */ OP(FD,33) THEN illegal_1(); JP(33)
/* INC  (IY+o)     */ OP(FD,34) CALL eay(); THEN TADR=PCD-1; CALL nomreq_addr(5); THEN TADR=m_ea; CALL rm_reg(); THEN inc(TDAT8); CALL wm(); ENDOP
/* DEC  (IY+o)     */ OP(FD,35) CALL eay(); THEN TADR=PCD-1; CALL nomreq_addr(5); THEN TADR=m_ea; CALL rm_reg(); THEN dec(TDAT8); CALL wm(); ENDOP
/* LD   (IY+o),n   */ OP(FD,36) CALL eay(); CALL arg(); THEN TADR=PCD-1; CALL nomreq_addr(2); THEN TADR=m_ea; CALL wm(); ENDOP
/* DB   FD         */ OP(FD,37) THEN illegal_1(); JP(37)

/* DB   FD         */ OP(FD,38) THEN illegal_1(); JP(38)
/* ADD  IY,SP      */ OP(FD,39) THEN TDAT=IY; TDAT2=SP; CALL add16(); THEN IY=TDAT;    ENDOP
/* DB   FD         */ OP(FD,3a) THEN illegal_1(); JP(3a)
/* DB   FD         */ OP(FD,3b) THEN illegal_1(); JP(3b)
/* DB   FD         */ OP(FD,3c) THEN illegal_1(); JP(3c)
/* DB   FD         */ OP(FD,3d) THEN illegal_1(); JP(3d)
/* DB   FD         */ OP(FD,3e) THEN illegal_1(); JP(3e)
/* DB   FD         */ OP(FD,3f) THEN illegal_1(); JP(3f)

/* DB   FD         */ OP(FD,40) THEN illegal_1(); JP(40)
/* DB   FD         */ OP(FD,41) THEN illegal_1(); JP(41)
/* DB   FD         */ OP(FD,42) THEN illegal_1(); JP(42)
/* DB   FD         */ OP(FD,43) THEN illegal_1(); JP(43)
/* LD   B,HY       */ OP(FD,44) THEN B = HY;                                           ENDOP
/* LD   B,LY       */ OP(FD,45) THEN B = LY;                                           ENDOP
/* LD   B,(IY+o)   */ OP(FD,46) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN B = TDAT8; ENDOP
/* DB   FD         */ OP(FD,47) THEN illegal_1(); JP(47)

/* DB   FD         */ OP(FD,48) THEN illegal_1(); JP(48)
/* DB   FD         */ OP(FD,49) THEN illegal_1(); JP(49)
/* DB   FD         */ OP(FD,4a) THEN illegal_1(); JP(4a)
/* DB   FD         */ OP(FD,4b) THEN illegal_1(); JP(4b)
/* LD   C,HY       */ OP(FD,4c) THEN C = HY;                                           ENDOP
/* LD   C,LY       */ OP(FD,4d) THEN C = LY;                                           ENDOP
/* LD   C,(IY+o)   */ OP(FD,4e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN C = TDAT8; ENDOP
/* DB   FD         */ OP(FD,4f) THEN illegal_1(); JP(4f)

/* DB   FD         */ OP(FD,50) THEN illegal_1(); JP(50)
/* DB   FD         */ OP(FD,51) THEN illegal_1(); JP(51)
/* DB   FD         */ OP(FD,52) THEN illegal_1(); JP(52)
/* DB   FD         */ OP(FD,53) THEN illegal_1(); JP(53)
/* LD   D,HY       */ OP(FD,54) THEN D = HY;                                           ENDOP
/* LD   D,LY       */ OP(FD,55) THEN D = LY;                                           ENDOP
/* LD   D,(IY+o)   */ OP(FD,56) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN D = TDAT8; ENDOP
/* DB   FD         */ OP(FD,57) THEN illegal_1(); JP(57)

/* DB   FD         */ OP(FD,58) THEN illegal_1(); JP(58)
/* DB   FD         */ OP(FD,59) THEN illegal_1(); JP(59)
/* DB   FD         */ OP(FD,5a) THEN illegal_1(); JP(5a)
/* DB   FD         */ OP(FD,5b) THEN illegal_1(); JP(5b)
/* LD   E,HY       */ OP(FD,5c) THEN E = HY;                                           ENDOP
/* LD   E,LY       */ OP(FD,5d) THEN E = LY;                                           ENDOP
/* LD   E,(IY+o)   */ OP(FD,5e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN E = TDAT8; ENDOP
/* DB   FD         */ OP(FD,5f) THEN illegal_1(); JP(5f)

/* LD   HY,B       */ OP(FD,60) THEN HY = B;                                           ENDOP
/* LD   HY,C       */ OP(FD,61) THEN HY = C;                                           ENDOP
/* LD   HY,D       */ OP(FD,62) THEN HY = D;                                           ENDOP
/* LD   HY,E       */ OP(FD,63) THEN HY = E;                                           ENDOP
/* LD   HY,HY      */ OP(FD,64)                                                        ENDOP
/* LD   HY,LY      */ OP(FD,65) THEN HY = LY;                                          ENDOP
/* LD   H,(IY+o)   */ OP(FD,66) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN H = TDAT8; ENDOP
/* LD   HY,A       */ OP(FD,67) THEN HY = A;                                           ENDOP

/* LD   LY,B       */ OP(FD,68) THEN LY = B;                                           ENDOP
/* LD   LY,C       */ OP(FD,69) THEN LY = C;                                           ENDOP
/* LD   LY,D       */ OP(FD,6a) THEN LY = D;                                           ENDOP
/* LD   LY,E       */ OP(FD,6b) THEN LY = E;                                           ENDOP
/* LD   LY,HY      */ OP(FD,6c) THEN LY = HY;                                          ENDOP
/* LD   LY,LY      */ OP(FD,6d)                                                        ENDOP
/* LD   L,(IY+o)   */ OP(FD,6e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN L = TDAT8; ENDOP
/* LD   LY,A       */ OP(FD,6f) THEN LY = A;                                           ENDOP

/* LD   (IY+o),B   */ OP(FD,70) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = B; CALL wm(); ENDOP
/* LD   (IY+o),C   */ OP(FD,71) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = C; CALL wm(); ENDOP
/* LD   (IY+o),D   */ OP(FD,72) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = D; CALL wm(); ENDOP
/* LD   (IY+o),E   */ OP(FD,73) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = E; CALL wm(); ENDOP
/* LD   (IY+o),H   */ OP(FD,74) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = H; CALL wm(); ENDOP
/* LD   (IY+o),L   */ OP(FD,75) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = L; CALL wm(); ENDOP
/* DB   FD         */ OP(FD,76) THEN illegal_1(); JP(76)
/* LD   (IY+o),A   */ OP(FD,77) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; TDAT8 = A; CALL wm(); ENDOP

/* DB   FD         */ OP(FD,78) THEN illegal_1(); JP(78)
/* DB   FD         */ OP(FD,79) THEN illegal_1(); JP(79)
/* DB   FD         */ OP(FD,7a) THEN illegal_1(); JP(7a)
/* DB   FD         */ OP(FD,7b) THEN illegal_1(); JP(7b)
/* LD   A,HY       */ OP(FD,7c) THEN A = HY;                                          ENDOP
/* LD   A,LY       */ OP(FD,7d) THEN A = LY;                                          ENDOP
/* LD   A,(IY+o)   */ OP(FD,7e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN A = TDAT8; ENDOP
/* DB   FD         */ OP(FD,7f) THEN illegal_1(); JP(7f)

/* DB   FD         */ OP(FD,80) THEN illegal_1(); JP(80)
/* DB   FD         */ OP(FD,81) THEN illegal_1(); JP(81)
/* DB   FD         */ OP(FD,82) THEN illegal_1(); JP(82)
/* DB   FD         */ OP(FD,83) THEN illegal_1(); JP(83)
/* ADD  A,HY       */ OP(FD,84) THEN add_a(HY);                                       ENDOP
/* ADD  A,LY       */ OP(FD,85) THEN add_a(LY);                                       ENDOP
/* ADD  A,(IY+o)   */ OP(FD,86) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN add_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,87) THEN illegal_1(); JP(87)

/* DB   FD         */ OP(FD,88) THEN illegal_1(); JP(88)
/* DB   FD         */ OP(FD,89) THEN illegal_1(); JP(89)
/* DB   FD         */ OP(FD,8a) THEN illegal_1(); JP(8a)
/* DB   FD         */ OP(FD,8b) THEN illegal_1(); JP(8b)
/* ADC  A,HY       */ OP(FD,8c) THEN adc_a(HY);                                       ENDOP
/* ADC  A,LY       */ OP(FD,8d) THEN adc_a(LY);                                       ENDOP
/* ADC  A,(IY+o)   */ OP(FD,8e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN adc_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,8f) THEN illegal_1(); JP(8f)

/* DB   FD         */ OP(FD,90) THEN illegal_1(); JP(90)
/* DB   FD         */ OP(FD,91) THEN illegal_1(); JP(91)
/* DB   FD         */ OP(FD,92) THEN illegal_1(); JP(92)
/* DB   FD         */ OP(FD,93) THEN illegal_1(); JP(93)
/* SUB  HY         */ OP(FD,94) THEN sub(HY);                                         ENDOP
/* SUB  LY         */ OP(FD,95) THEN sub(LY);                                         ENDOP
/* SUB  (IY+o)     */ OP(FD,96) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN sub(TDAT8); ENDOP
/* DB   FD         */ OP(FD,97) THEN illegal_1(); JP(97)

/* DB   FD         */ OP(FD,98) THEN illegal_1(); JP(98)
/* DB   FD         */ OP(FD,99) THEN illegal_1(); JP(99)
/* DB   FD         */ OP(FD,9a) THEN illegal_1(); JP(9a)
/* DB   FD         */ OP(FD,9b) THEN illegal_1(); JP(9b)
/* SBC  A,HY       */ OP(FD,9c) THEN sbc_a(HY);                                       ENDOP
/* SBC  A,LY       */ OP(FD,9d) THEN sbc_a(LY);                                       ENDOP
/* SBC  A,(IY+o)   */ OP(FD,9e) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN sbc_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,9f) THEN illegal_1(); JP(9f)

/* DB   FD         */ OP(FD,a0) THEN illegal_1(); JP(a0)
/* DB   FD         */ OP(FD,a1) THEN illegal_1(); JP(a1)
/* DB   FD         */ OP(FD,a2) THEN illegal_1(); JP(a2)
/* DB   FD         */ OP(FD,a3) THEN illegal_1(); JP(a3)
/* AND  HY         */ OP(FD,a4) THEN and_a(HY);                                       ENDOP
/* AND  LY         */ OP(FD,a5) THEN and_a(LY);                                       ENDOP
/* AND  (IY+o)     */ OP(FD,a6) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN and_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,a7) THEN illegal_1(); JP(a7)

/* DB   FD         */ OP(FD,a8) THEN illegal_1(); JP(a8)
/* DB   FD         */ OP(FD,a9) THEN illegal_1(); JP(a9)
/* DB   FD         */ OP(FD,aa) THEN illegal_1(); JP(aa)
/* DB   FD         */ OP(FD,ab) THEN illegal_1(); JP(ab)
/* XOR  HY         */ OP(FD,ac) THEN xor_a(HY);                                       ENDOP
/* XOR  LY         */ OP(FD,ad) THEN xor_a(LY);                                       ENDOP
/* XOR  (IY+o)     */ OP(FD,ae) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN xor_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,af) THEN illegal_1(); JP(af)

/* DB   FD         */ OP(FD,b0) THEN illegal_1(); JP(b0)
/* DB   FD         */ OP(FD,b1) THEN illegal_1(); JP(b1)
/* DB   FD         */ OP(FD,b2) THEN illegal_1(); JP(b2)
/* DB   FD         */ OP(FD,b3) THEN illegal_1(); JP(b3)
/* OR   HY         */ OP(FD,b4) THEN or_a(HY);                                        ENDOP
/* OR   LY         */ OP(FD,b5) THEN or_a(LY);                                        ENDOP
/* OR   (IY+o)     */ OP(FD,b6) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN or_a(TDAT8); ENDOP
/* DB   FD         */ OP(FD,b7) THEN illegal_1(); JP(b7)

/* DB   FD         */ OP(FD,b8) THEN illegal_1(); JP(b8)
/* DB   FD         */ OP(FD,b9) THEN illegal_1(); JP(b9)
/* DB   FD         */ OP(FD,ba) THEN illegal_1(); JP(ba)
/* DB   FD         */ OP(FD,bb) THEN illegal_1(); JP(bb)
/* CP   HY         */ OP(FD,bc) THEN cp(HY);                                          ENDOP
/* CP   LY         */ OP(FD,bd) THEN cp(LY);                                          ENDOP
/* CP   (IY+o)     */ OP(FD,be) CALL eay(); THEN TADR = PCD-1; CALL nomreq_addr(5); THEN TADR = m_ea; CALL rm(); THEN cp(TDAT8); ENDOP
/* DB   FD         */ OP(FD,bf) THEN illegal_1(); JP(bf)

/* DB   FD         */ OP(FD,c0) THEN illegal_1(); JP(c0)
/* DB   FD         */ OP(FD,c1) THEN illegal_1(); JP(c1)
/* DB   FD         */ OP(FD,c2) THEN illegal_1(); JP(c2)
/* DB   FD         */ OP(FD,c3) THEN illegal_1(); JP(c3)
/* DB   FD         */ OP(FD,c4) THEN illegal_1(); JP(c4)
/* DB   FD         */ OP(FD,c5) THEN illegal_1(); JP(c5)
/* DB   FD         */ OP(FD,c6) THEN illegal_1(); JP(c6)
/* DB   FD         */ OP(FD,c7) THEN illegal_1(); JP(c7)

/* DB   FD         */ OP(FD,c8) THEN illegal_1(); JP(c8)
/* DB   FD         */ OP(FD,c9) THEN illegal_1(); JP(c9)
/* DB   FD         */ OP(FD,ca) THEN illegal_1(); JP(ca)
/* **   FD CB xx   */ OP(FD,cb) CALL eay(); CALL arg(); THEN TADR=PCD-1; CALL nomreq_addr(2); THENJP(XY_CB)
/* DB   FD         */ OP(FD,cc) THEN illegal_1(); JP(cc)
/* DB   FD         */ OP(FD,cd) THEN illegal_1(); JP(cd)
/* DB   FD         */ OP(FD,ce) THEN illegal_1(); JP(ce)
/* DB   FD         */ OP(FD,cf) THEN illegal_1(); JP(cf)

/* DB   FD         */ OP(FD,d0) THEN illegal_1(); JP(d0)
/* DB   FD         */ OP(FD,d1) THEN illegal_1(); JP(d1)
/* DB   FD         */ OP(FD,d2) THEN illegal_1(); JP(d2)
/* DB   FD         */ OP(FD,d3) THEN illegal_1(); JP(d3)
/* DB   FD         */ OP(FD,d4) THEN illegal_1(); JP(d4)
/* DB   FD         */ OP(FD,d5) THEN illegal_1(); JP(d5)
/* DB   FD         */ OP(FD,d6) THEN illegal_1(); JP(d6)
/* DB   FD         */ OP(FD,d7) THEN illegal_1(); JP(d7)

/* DB   FD         */ OP(FD,d8) THEN illegal_1(); JP(d8)
/* DB   FD         */ OP(FD,d9) THEN illegal_1(); JP(d9)
/* DB   FD         */ OP(FD,da) THEN illegal_1(); JP(da)
/* DB   FD         */ OP(FD,db) THEN illegal_1(); JP(db)
/* DB   FD         */ OP(FD,dc) THEN illegal_1(); JP(dc)
/* DB   FD         */ OP(FD,dd) THEN illegal_1(); JP(dd)
/* DB   FD         */ OP(FD,de) THEN illegal_1(); JP(de)
/* DB   FD         */ OP(FD,df) THEN illegal_1(); JP(df)

/* DB   FD         */ OP(FD,e0) THEN illegal_1(); JP(e0)
/* POP  IY         */ OP(FD,e1) CALL pop(); THEN IY=TDAT;                 ENDOP
/* DB   FD         */ OP(FD,e2) THEN illegal_1(); JP(e2)
/* EX   (SP),IY    */ OP(FD,e3) THEN TDAT=IY; CALL ex_sp(); THEN IY=TDAT; ENDOP
/* DB   FD         */ OP(FD,e4) THEN illegal_1(); JP(e4)
/* PUSH IY         */ OP(FD,e5) THEN TDAT=IY; CALL push();                ENDOP
/* DB   FD         */ OP(FD,e6) THEN illegal_1(); JP(e6)
/* DB   FD         */ OP(FD,e7) THEN illegal_1(); JP(e7)

/* DB   FD         */ OP(FD,e8) THEN illegal_1(); JP(e8)
/* JP   (IY)       */ OP(FD,e9) THEN PC = IY;                             ENDOP
/* DB   FD         */ OP(FD,ea) THEN illegal_1(); JP(ea)
/* DB   FD         */ OP(FD,eb) THEN illegal_1(); JP(eb)
/* DB   FD         */ OP(FD,ec) THEN illegal_1(); JP(ec)
/* DB   FD         */ OP(FD,ed) THEN illegal_1(); JP(ed)
/* DB   FD         */ OP(FD,ee) THEN illegal_1(); JP(ee)
/* DB   FD         */ OP(FD,ef) THEN illegal_1(); JP(ef)

/* DB   FD         */ OP(FD,f0) THEN illegal_1(); JP(f0)
/* DB   FD         */ OP(FD,f1) THEN illegal_1(); JP(f1)
/* DB   FD         */ OP(FD,f2) THEN illegal_1(); JP(f2)
/* DB   FD         */ OP(FD,f3) THEN illegal_1(); JP(f3)
/* DB   FD         */ OP(FD,f4) THEN illegal_1(); JP(f4)
/* DB   FD         */ OP(FD,f5) THEN illegal_1(); JP(f5)
/* DB   FD         */ OP(FD,f6) THEN illegal_1(); JP(f6)
/* DB   FD         */ OP(FD,f7) THEN illegal_1(); JP(f7)

/* DB   FD         */ OP(FD,f8) THEN illegal_1(); JP(f8)
/* LD   SP,IY      */ OP(FD,f9) CALL nomreq_ir(2); THEN SP = IY;                       ENDOP
/* DB   FD         */ OP(FD,fa) THEN illegal_1(); JP(fa)
/* DB   FD         */ OP(FD,fb) THEN illegal_1(); JP(fb)
/* DB   FD         */ OP(FD,fc) THEN illegal_1(); JP(fc)
/* DB   FD         */ OP(FD,fd) THEN illegal_1(); JP(fd)
/* DB   FD         */ OP(FD,fe) THEN illegal_1(); JP(fe)
/* DB   FD         */ OP(FD,ff) THEN illegal_1(); JP(ff)

/**********************************************************
 * special opcodes (ED prefix)
 **********************************************************/
/* DB   ED         */ OP(ED,00) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,01) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,02) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,03) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,04) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,05) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,06) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,07) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,08) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,09) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,0f) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,10) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,11) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,12) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,13) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,14) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,15) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,16) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,17) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,18) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,19) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,1f) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,20) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,21) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,22) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,23) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,24) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,25) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,26) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,27) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,28) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,29) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,2f) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,30) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,31) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,32) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,33) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,34) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,35) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,36) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,37) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,38) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,39) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,3f) THEN illegal_2();                                               ENDOP

/* IN   B,(C)      */ OP(ED,40) THEN TADR=BC; CALL in(); THEN B = TDAT8; set_f((F & CF) | SZP[B]); WZ = TADR + 1; ENDOP
/* OUT  (C),B      */ OP(ED,41) THEN TADR=BC; TDAT8=B; CALL out(); THEN WZ = TADR + 1; ENDOP
/* SBC  HL,BC      */ OP(ED,42) THEN TDAT=BC; CALL sbc_hl();                             ENDOP
/* LD   (w),BC     */ OP(ED,43) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=BC; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,44) THEN neg();                                           ENDOP
/* RETN            */ OP(ED,45) CALL retn();                                             ENDOP
/* IM   0          */ OP(ED,46) THEN m_im = 0;                                        ENDOP
/* LD   i,A        */ OP(ED,47) CALL ld_i_a();                                           ENDOP

/* IN   C,(C)      */ OP(ED,48) THEN TADR=BC; CALL in(); THEN C = TDAT8; set_f((F & CF) | SZP[C]); WZ = TADR + 1;ENDOP
/* OUT  (C),C      */ OP(ED,49) THEN TADR=BC; TDAT8=C; CALL out(); THEN WZ = TADR + 1; ENDOP
/* ADC  HL,BC      */ OP(ED,4a) THEN TDAT=BC; CALL adc_hl();                             ENDOP
/* LD   BC,(w)     */ OP(ED,4b) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN BC=TDAT; WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,4c) THEN neg();                                           ENDOP
/* RETI            */ OP(ED,4d) CALL reti();                                             ENDOP
/* IM   0          */ OP(ED,4e) THEN m_im = 0;                                        ENDOP
/* LD   r,A        */ OP(ED,4f) CALL ld_r_a();                                           ENDOP

/* IN   D,(C)      */ OP(ED,50) THEN TADR=BC; CALL in(); THEN D = TDAT8; set_f((F & CF) | SZP[D]); WZ = TADR + 1; ENDOP
/* OUT  (C),D      */ OP(ED,51) THEN TADR=BC; TDAT8=D; CALL out(); THEN WZ = TADR + 1; ENDOP
/* SBC  HL,DE      */ OP(ED,52) THEN TDAT=DE; CALL sbc_hl();                             ENDOP
/* LD   (w),DE     */ OP(ED,53) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=DE; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,54) THEN neg();                                           ENDOP
/* RETN            */ OP(ED,55) CALL retn();                                             ENDOP
/* IM   1          */ OP(ED,56) THEN m_im = 1;                                        ENDOP
/* LD   A,i        */ OP(ED,57) CALL ld_a_i();                                           ENDOP

/* IN   E,(C)      */ OP(ED,58) THEN TADR=BC; CALL in(); THEN E = TDAT8; set_f((F & CF) | SZP[E]); WZ = TADR + 1; ENDOP
/* OUT  (C),E      */ OP(ED,59) THEN TADR=BC; TDAT8=E; CALL out(); THEN WZ = TADR + 1; ENDOP
/* ADC  HL,DE      */ OP(ED,5a) THEN TDAT=DE; CALL adc_hl();                             ENDOP
/* LD   DE,(w)     */ OP(ED,5b) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN DE=TDAT; WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,5c) THEN neg();                                           ENDOP
/* RETI            */ OP(ED,5d) CALL reti();                                             ENDOP
/* IM   2          */ OP(ED,5e) THEN m_im = 2;                                        ENDOP
/* LD   A,r        */ OP(ED,5f) CALL ld_a_r();                                           ENDOP

/* IN   H,(C)      */ OP(ED,60) THEN TADR=BC; CALL in(); THEN H = TDAT8; set_f((F & CF) | SZP[H]); WZ = TADR + 1; ENDOP
/* OUT  (C),H      */ OP(ED,61) THEN TADR=BC; TDAT8=H; CALL out(); THEN WZ = TADR + 1; ENDOP
/* SBC  HL,HL      */ OP(ED,62) THEN TDAT=HL; CALL sbc_hl();                             ENDOP
/* LD   (w),HL     */ OP(ED,63) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=HL; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,64) THEN neg();                                           ENDOP
/* RETN            */ OP(ED,65) CALL retn();                                             ENDOP
/* IM   0          */ OP(ED,66) THEN m_im = 0;                                        ENDOP
/* RRD  (HL)       */ OP(ED,67) CALL rrd();                                              ENDOP

/* IN   L,(C)      */ OP(ED,68) THEN TADR=BC; CALL in(); THEN L = TDAT8; set_f((F & CF) | SZP[L]); WZ = TADR + 1; ENDOP
/* OUT  (C),L      */ OP(ED,69) THEN TADR=BC; TDAT8=L; CALL out(); THEN WZ = TADR + 1; ENDOP
/* ADC  HL,HL      */ OP(ED,6a) THEN TDAT=HL; CALL adc_hl();                             ENDOP
/* LD   HL,(w)     */ OP(ED,6b) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN HL=TDAT; WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,6c) THEN neg();                                           ENDOP
/* RETI            */ OP(ED,6d) CALL reti();                                             ENDOP
/* IM   0          */ OP(ED,6e) THEN m_im = 0;                                        ENDOP
/* RLD  (HL)       */ OP(ED,6f) CALL rld();                                              ENDOP

/* IN   0,(C)      */ OP(ED,70) THEN TADR=BC; CALL in(); THEN set_f((F & CF) | SZP[TDAT8]); WZ = TADR + 1; ENDOP
/* OUT  (C),0      */ OP(ED,71) THEN TADR=BC; TDAT8=0; CALL out(); THEN WZ = TADR + 1; ENDOP
/* SBC  HL,SP      */ OP(ED,72) THEN TDAT=SP; CALL sbc_hl();                             ENDOP
/* LD   (w),SP     */ OP(ED,73) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT=SP; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,74) THEN neg();                                           ENDOP
/* RETN            */ OP(ED,75) CALL retn();                                             ENDOP
/* IM   1          */ OP(ED,76) THEN m_im = 1;                                        ENDOP
/* DB   ED,77      */ OP(ED,77) THEN illegal_2();                                               ENDOP

/* IN   A,(C)      */ OP(ED,78) THEN TADR=BC; CALL in(); THEN A = TDAT8; set_f((F & CF) | SZP[A]); WZ = TADR + 1; ENDOP
/* OUT  (C),A      */ OP(ED,79) THEN TADR=BC; TDAT8=A; CALL out(); THEN WZ = TADR + 1; ENDOP
/* ADC  HL,SP      */ OP(ED,7a) THEN TDAT=SP; CALL adc_hl();                             ENDOP
/* LD   SP,(w)     */ OP(ED,7b) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; CALL rm16(); THEN SP=TDAT; WZ = m_ea + 1; ENDOP
/* NEG             */ OP(ED,7c) THEN neg();                                           ENDOP
/* RETI            */ OP(ED,7d) CALL reti();                                             ENDOP
/* IM   2          */ OP(ED,7e) THEN m_im = 2;                                        ENDOP
/* DB   ED,7F      */ OP(ED,7f) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,80) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,81) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,82) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,83) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,84) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,85) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,86) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,87) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,88) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,89) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,8f) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,90) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,91) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,92) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,93) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,94) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,95) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,96) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,97) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,98) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,99) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9a) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9b) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9c) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9d) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9e) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,9f) THEN illegal_2();                                               ENDOP

/* LDI             */ OP(ED,a0) CALL ldi();                                              ENDOP
/* CPI             */ OP(ED,a1) CALL cpi();                                              ENDOP
/* INI             */ OP(ED,a2) CALL ini();                                              ENDOP
/* OUTI            */ OP(ED,a3) CALL outi();                                             ENDOP
/* DB   ED         */ OP(ED,a4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,a5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,a6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,a7) THEN illegal_2();                                               ENDOP

/* LDD             */ OP(ED,a8) CALL ldd();                                              ENDOP
/* CPD             */ OP(ED,a9) CALL cpd();                                              ENDOP
/* IND             */ OP(ED,aa) CALL ind();                                              ENDOP
/* OUTD            */ OP(ED,ab) CALL outd();                                             ENDOP
/* DB   ED         */ OP(ED,ac) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ad) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ae) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,af) THEN illegal_2();                                               ENDOP

/* LDIR            */ OP(ED,b0) CALL ldir();                                             ENDOP
/* CPIR            */ OP(ED,b1) CALL cpir();                                             ENDOP
/* INIR            */ OP(ED,b2) CALL inir();                                             ENDOP
/* OTIR            */ OP(ED,b3) CALL otir();                                             ENDOP
/* DB   ED         */ OP(ED,b4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,b5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,b6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,b7) THEN illegal_2();                                               ENDOP

/* LDDR            */ OP(ED,b8) CALL lddr();                                             ENDOP
/* CPDR            */ OP(ED,b9) CALL cpdr();                                             ENDOP
/* INDR            */ OP(ED,ba) CALL indr();                                             ENDOP
/* OTDR            */ OP(ED,bb) CALL otdr();                                             ENDOP
/* DB   ED         */ OP(ED,bc) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,bd) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,be) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,bf) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,c0) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c1) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c2) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c3) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c7) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,c8) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,c9) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ca) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,cb) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,cc) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,cd) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ce) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,cf) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,d0) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d1) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d2) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d3) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d7) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,d8) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,d9) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,da) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,db) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,dc) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,dd) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,de) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,df) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,e0) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e1) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e2) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e3) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e7) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,e8) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,e9) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ea) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,eb) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ec) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ed) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ee) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ef) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,f0) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f1) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f2) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f3) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f4) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f5) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f6) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f7) THEN illegal_2();                                               ENDOP

/* DB   ED         */ OP(ED,f8) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,f9) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,fa) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,fb) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,fc) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,fd) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,fe) THEN illegal_2();                                               ENDOP
/* DB   ED         */ OP(ED,ff) THEN illegal_2();                                               ENDOP

/**********************************************************
 * main opcodes
 **********************************************************/
/* NOP             */ OP(NONE,00)                                                                        ENDOP
/* LD   BC,w       */ OP(NONE,01) CALL arg16(); THEN BC = TDAT;                                               ENDOP
/* LD (BC),A       */ OP(NONE,02) THEN TADR=BC; TDAT8=A; CALL wm(); THEN WZ_L = (BC + 1) & 0xFF;  WZ_H = A;   ENDOP
/* INC  BC         */ OP(NONE,03) CALL nomreq_ir(2); THEN BC++;                                               ENDOP
/* INC  B          */ OP(NONE,04) THEN inc(B);                                                               ENDOP
/* DEC  B          */ OP(NONE,05) THEN dec(B);                                                               ENDOP
/* LD   B,n        */ OP(NONE,06) CALL arg(); THEN B = TDAT8;                                                 ENDOP
/* RLCA            */ OP(NONE,07) THEN rlca();                                                               ENDOP

/* EX   AF,AF'     */ OP(NONE,08) THEN std::swap(m_af, m_af2);
;                                   ENDOP
/* ADD  HL,BC      */ OP(NONE,09) THEN TDAT=HL; TDAT2=BC; CALL add16(); THEN HL=TDAT;                         ENDOP
/* LD   A,(BC)     */ OP(NONE,0a) THEN TADR=BC; CALL rm(); THEN A=TDAT8;  WZ=BC+1;                            ENDOP
/* DEC  BC         */ OP(NONE,0b) CALL nomreq_ir(2); THEN BC--;                                               ENDOP
/* INC  C          */ OP(NONE,0c) THEN inc(C);                                                               ENDOP
/* DEC  C          */ OP(NONE,0d) THEN dec(C);                                                               ENDOP
/* LD   C,n        */ OP(NONE,0e) CALL arg(); THEN C = TDAT8;                                                 ENDOP
/* RRCA            */ OP(NONE,0f) THEN rrca();                                                               ENDOP

/* DJNZ o          */ OP(NONE,10) CALL nomreq_ir(1); THEN TDAT8=--B; CALL jr_cond(0x10);                         ENDOP
/* LD   DE,w       */ OP(NONE,11) CALL arg16(); THEN DE = TDAT;                                               ENDOP
/* LD (DE),A       */ OP(NONE,12) THEN TADR=DE; TDAT8=A; CALL wm(); THEN WZ_L = (DE + 1) & 0xFF;  WZ_H = A;   ENDOP
/* INC  DE         */ OP(NONE,13) CALL nomreq_ir(2); THEN DE++;                                               ENDOP
/* INC  D          */ OP(NONE,14) THEN inc(D);                                                               ENDOP
/* DEC  D          */ OP(NONE,15) THEN dec(D);                                                               ENDOP
/* LD   D,n        */ OP(NONE,16) CALL arg(); THEN D=TDAT8;                                                   ENDOP
/* RLA             */ OP(NONE,17) THEN rla();                                                                ENDOP

/* JR   o          */ OP(NONE,18) CALL jr();                                                                    ENDOP
/* ADD  HL,DE      */ OP(NONE,19) THEN TDAT=HL; TDAT2=DE; CALL add16(); THEN HL=TDAT;                         ENDOP
/* LD   A,(DE)     */ OP(NONE,1a) THEN TADR=DE; CALL rm(); THEN A=TDAT8; WZ = DE + 1;                         ENDOP
/* DEC  DE         */ OP(NONE,1b) CALL nomreq_ir(2); THEN DE--;                                               ENDOP
/* INC  E          */ OP(NONE,1c) THEN inc(E);                                                               ENDOP
/* DEC  E          */ OP(NONE,1d) THEN dec(E);                                                               ENDOP
/* LD   E,n        */ OP(NONE,1e) CALL arg(); THEN E=TDAT8;                                                   ENDOP
/* RRA             */ OP(NONE,1f) THEN rra();                                                                ENDOP

/* JR   NZ,o       */ OP(NONE,20) THEN TDAT8=!(F & ZF); CALL jr_cond(0x20);                                               ENDOP
/* LD   HL,w       */ OP(NONE,21) CALL arg16(); THEN HL = TDAT;                                               ENDOP
/* LD   (w),HL     */ OP(NONE,22) CALL arg16(); THEN m_ea=TDAT; TADR=TDAT; TDAT=HL; CALL wm16(); THEN WZ = m_ea + 1; ENDOP
/* INC  HL         */ OP(NONE,23) CALL nomreq_ir(2); THEN HL++;                                               ENDOP
/* INC  H          */ OP(NONE,24) THEN inc(H);                                                               ENDOP
/* DEC  H          */ OP(NONE,25) THEN dec(H);                                                               ENDOP
/* LD   H,n        */ OP(NONE,26) CALL arg(); THEN H=TDAT8;                                                   ENDOP
/* DAA             */ OP(NONE,27) THEN daa();                                                                ENDOP

/* JR   Z,o        */ OP(NONE,28) THEN TDAT8=F & ZF; CALL jr_cond(0x28);                                        ENDOP
/* ADD  HL,HL      */ OP(NONE,29) THEN TDAT=HL; TDAT2=HL; CALL add16(); THEN HL=TDAT;                         ENDOP
/* LD   HL,(w)     */ OP(NONE,2a) CALL arg16(); THEN m_ea=TDAT; TADR=TDAT; CALL rm16(); THEN HL=TDAT; WZ = m_ea + 1; ENDOP
/* DEC  HL         */ OP(NONE,2b) CALL nomreq_ir(2); THEN HL--;                                               ENDOP
/* INC  L          */ OP(NONE,2c) THEN inc(L);                                                               ENDOP
/* DEC  L          */ OP(NONE,2d) THEN dec(L);                                                               ENDOP
/* LD   L,n        */ OP(NONE,2e) CALL arg(); THEN L=TDAT8;                                                   ENDOP
/* CPL             */ OP(NONE,2f) THEN A ^= 0xff; set_f((F & (SF | ZF | PF | CF)) | HF | NF | (A & (YF | XF))); ENDOP

/* JR   NC,o       */ OP(NONE,30) THEN TDAT8=!(F & CF); CALL jr_cond(0x30);                                               ENDOP
/* LD   SP,w       */ OP(NONE,31) CALL arg16(); THEN SP = TDAT;                                               ENDOP
/* LD   (w),A      */ OP(NONE,32) CALL arg16(); THEN m_ea=TDAT; TADR=m_ea; TDAT8=A; CALL wm(); THEN WZ_L = (m_ea + 1) & 0xFF; WZ_H = A; ENDOP
/* INC  SP         */ OP(NONE,33) CALL nomreq_ir(2); THEN SP++;                                               ENDOP
/* INC  (HL)       */ OP(NONE,34) THEN TADR=HL; CALL rm_reg(); THEN inc(TDAT8); CALL wm();                       ENDOP
/* DEC  (HL)       */ OP(NONE,35) THEN TADR=HL; CALL rm_reg(); THEN dec(TDAT8); CALL wm();                       ENDOP
/* LD   (HL),n     */ OP(NONE,36) CALL arg(); THEN TADR=HL; CALL wm();                                           ENDOP
/* SCF             */ OP(NONE,37) THEN set_f((F & (SF | ZF | PF)) | CF | (((F & Q) | A) & (YF | XF)));            ENDOP

/* JR   C,o        */ OP(NONE,38) THEN TDAT8=F & CF; CALL jr_cond(0x38);                                        ENDOP
/* ADD  HL,SP      */ OP(NONE,39) THEN TDAT=HL; TDAT2=SP; CALL add16(); THEN HL=TDAT;                         ENDOP
/* LD   A,(w)      */ OP(NONE,3a) CALL arg16(); THEN m_ea=TDAT; TADR=TDAT; CALL rm(); THEN A=TDAT8; WZ = m_ea + 1; ENDOP
/* DEC  SP         */ OP(NONE,3b) CALL nomreq_ir(2); THEN SP--;                                               ENDOP
/* INC  A          */ OP(NONE,3c) THEN inc(A);                                                               ENDOP
/* DEC  A          */ OP(NONE,3d) THEN dec(A);                                                               ENDOP
/* LD   A,n        */ OP(NONE,3e) CALL arg(); THEN A = TDAT8;                                                 ENDOP
/* CCF             */ OP(NONE,3f) THEN set_f(((F & (SF | ZF | PF | CF)) ^ CF) | ((F & CF) << 4) | (((F & Q) | A) & (YF | XF)));         ENDOP

/* LD   B,B        */ OP(NONE,40)                                                                        ENDOP
/* LD   B,C        */ OP(NONE,41) THEN B = C;                                                                ENDOP
/* LD   B,D        */ OP(NONE,42) THEN B = D;                                                                ENDOP
/* LD   B,E        */ OP(NONE,43) THEN B = E;                                                                ENDOP
/* LD   B,H        */ OP(NONE,44) THEN B = H;                                                                ENDOP
/* LD   B,L        */ OP(NONE,45) THEN B = L;                                                                ENDOP
/* LD   B,(HL)     */ OP(NONE,46) THEN TADR=HL; CALL rm(); THEN B = TDAT8;                                    ENDOP
/* LD   B,A        */ OP(NONE,47) THEN B = A;                                                                ENDOP

/* LD   C,B        */ OP(NONE,48) THEN C = B;                                                                ENDOP
/* LD   C,C        */ OP(NONE,49)                                                                        ENDOP
/* LD   C,D        */ OP(NONE,4a) THEN C = D;                                                                ENDOP
/* LD   C,E        */ OP(NONE,4b) THEN C = E;                                                                ENDOP
/* LD   C,H        */ OP(NONE,4c) THEN C = H;                                                                ENDOP
/* LD   C,L        */ OP(NONE,4d) THEN C = L;                                                                ENDOP
/* LD   C,(HL)     */ OP(NONE,4e) THEN TADR=HL; CALL rm(); THEN C = TDAT8;                                    ENDOP
/* LD   C,A        */ OP(NONE,4f) THEN C = A;                                                                ENDOP

/* LD   D,B        */ OP(NONE,50) THEN D = B;                                                                ENDOP
/* LD   D,C        */ OP(NONE,51) THEN D = C;                                                                ENDOP
/* LD   D,D        */ OP(NONE,52)                                                                        ENDOP
/* LD   D,E        */ OP(NONE,53) THEN D = E;                                                                ENDOP
/* LD   D,H        */ OP(NONE,54) THEN D = H;                                                                ENDOP
/* LD   D,L        */ OP(NONE,55) THEN D = L;                                                                ENDOP
/* LD   D,(HL)     */ OP(NONE,56) THEN TADR=HL; CALL rm(); THEN D = TDAT8;                                    ENDOP
/* LD   D,A        */ OP(NONE,57) THEN D = A;                                                                ENDOP

/* LD   E,B        */ OP(NONE,58) THEN E = B;                                                                ENDOP
/* LD   E,C        */ OP(NONE,59) THEN E = C;                                                                ENDOP
/* LD   E,D        */ OP(NONE,5a) THEN E = D;                                                                ENDOP
/* LD   E,E        */ OP(NONE,5b)                                                                        ENDOP
/* LD   E,H        */ OP(NONE,5c) THEN E = H;                                                                ENDOP
/* LD   E,L        */ OP(NONE,5d) THEN E = L;                                                                ENDOP
/* LD   E,(HL)     */ OP(NONE,5e) THEN TADR=HL; CALL rm(); THEN E = TDAT8;                                    ENDOP
/* LD   E,A        */ OP(NONE,5f) THEN E = A;                                                                ENDOP

/* LD   H,B        */ OP(NONE,60) THEN H = B;                                                                ENDOP
/* LD   H,C        */ OP(NONE,61) THEN H = C;                                                                ENDOP
/* LD   H,D        */ OP(NONE,62) THEN H = D;                                                                ENDOP
/* LD   H,E        */ OP(NONE,63) THEN H = E;                                                                ENDOP
/* LD   H,H        */ OP(NONE,64)                                                                        ENDOP
/* LD   H,L        */ OP(NONE,65) THEN H = L;                                                                ENDOP
/* LD   H,(HL)     */ OP(NONE,66) THEN TADR=HL; CALL rm(); THEN H = TDAT8;                                    ENDOP
/* LD   H,A        */ OP(NONE,67) THEN H = A;                                                                ENDOP

/* LD   L,B        */ OP(NONE,68) THEN L = B;                                                                ENDOP
/* LD   L,C        */ OP(NONE,69) THEN L = C;                                                                ENDOP
/* LD   L,D        */ OP(NONE,6a) THEN L = D;                                                                ENDOP
/* LD   L,E        */ OP(NONE,6b) THEN L = E;                                                                ENDOP
/* LD   L,H        */ OP(NONE,6c) THEN L = H;                                                                ENDOP
/* LD   L,L        */ OP(NONE,6d)                                                                        ENDOP
/* LD   L,(HL)     */ OP(NONE,6e) THEN TADR=HL; CALL rm(); THEN L = TDAT8;                                    ENDOP
/* LD   L,A        */ OP(NONE,6f) THEN L = A;                                                                ENDOP

/* LD   (HL),B     */ OP(NONE,70) THEN TADR=HL; TDAT=B; CALL wm();                                              ENDOP
/* LD   (HL),C     */ OP(NONE,71) THEN TADR=HL; TDAT=C; CALL wm();                                              ENDOP
/* LD   (HL),D     */ OP(NONE,72) THEN TADR=HL; TDAT=D; CALL wm();                                              ENDOP
/* LD   (HL),E     */ OP(NONE,73) THEN TADR=HL; TDAT=E; CALL wm();                                              ENDOP
/* LD   (HL),H     */ OP(NONE,74) THEN TADR=HL; TDAT=H; CALL wm();                                              ENDOP
/* LD   (HL),L     */ OP(NONE,75) THEN TADR=HL; TDAT=L; CALL wm();                                              ENDOP
/* HALT            */ OP(NONE,76) THEN halt();                                                               ENDOP
/* LD   (HL),A     */ OP(NONE,77) THEN TADR=HL; TDAT=A; CALL wm();                                              ENDOP

/* LD   A,B        */ OP(NONE,78) THEN A = B;                                                                ENDOP
/* LD   A,C        */ OP(NONE,79) THEN A = C;                                                                ENDOP
/* LD   A,D        */ OP(NONE,7a) THEN A = D;                                                                ENDOP
/* LD   A,E        */ OP(NONE,7b) THEN A = E;                                                                ENDOP
/* LD   A,H        */ OP(NONE,7c) THEN A = H;                                                                ENDOP
/* LD   A,L        */ OP(NONE,7d) THEN A = L;                                                                ENDOP
/* LD   A,(HL)     */ OP(NONE,7e) THEN TADR=HL; CALL rm(); THEN A = TDAT8;                                    ENDOP
/* LD   A,A        */ OP(NONE,7f)                                                                        ENDOP

/* ADD  A,B        */ OP(NONE,80) THEN add_a(B);                                                             ENDOP
/* ADD  A,C        */ OP(NONE,81) THEN add_a(C);                                                             ENDOP
/* ADD  A,D        */ OP(NONE,82) THEN add_a(D);                                                             ENDOP
/* ADD  A,E        */ OP(NONE,83) THEN add_a(E);                                                             ENDOP
/* ADD  A,H        */ OP(NONE,84) THEN add_a(H);                                                             ENDOP
/* ADD  A,L        */ OP(NONE,85) THEN add_a(L);                                                             ENDOP
/* ADD  A,(HL)     */ OP(NONE,86) THEN TADR=HL; CALL rm(); THEN add_a(TDAT8);                                 ENDOP
/* ADD  A,A        */ OP(NONE,87) THEN add_a(A);                                                             ENDOP

/* ADC  A,B        */ OP(NONE,88) THEN adc_a(B);                                                             ENDOP
/* ADC  A,C        */ OP(NONE,89) THEN adc_a(C);                                                             ENDOP
/* ADC  A,D        */ OP(NONE,8a) THEN adc_a(D);                                                             ENDOP
/* ADC  A,E        */ OP(NONE,8b) THEN adc_a(E);                                                             ENDOP
/* ADC  A,H        */ OP(NONE,8c) THEN adc_a(H);                                                             ENDOP
/* ADC  A,L        */ OP(NONE,8d) THEN adc_a(L);                                                             ENDOP
/* ADC  A,(HL)     */ OP(NONE,8e) THEN TADR=HL; CALL rm(); THEN adc_a(TDAT8);                                 ENDOP
/* ADC  A,A        */ OP(NONE,8f) THEN adc_a(A);                                                             ENDOP

/* SUB  B          */ OP(NONE,90) THEN sub(B);                                                               ENDOP
/* SUB  C          */ OP(NONE,91) THEN sub(C);                                                               ENDOP
/* SUB  D          */ OP(NONE,92) THEN sub(D);                                                               ENDOP
/* SUB  E          */ OP(NONE,93) THEN sub(E);                                                               ENDOP
/* SUB  H          */ OP(NONE,94) THEN sub(H);                                                               ENDOP
/* SUB  L          */ OP(NONE,95) THEN sub(L);                                                               ENDOP
/* SUB  (HL)       */ OP(NONE,96) THEN TADR=HL; CALL rm(); THEN sub(TDAT8);                                   ENDOP
/* SUB  A          */ OP(NONE,97) THEN sub(A);                                                               ENDOP

/* SBC  A,B        */ OP(NONE,98) THEN sbc_a(B);                                                             ENDOP
/* SBC  A,C        */ OP(NONE,99) THEN sbc_a(C);                                                             ENDOP
/* SBC  A,D        */ OP(NONE,9a) THEN sbc_a(D);                                                             ENDOP
/* SBC  A,E        */ OP(NONE,9b) THEN sbc_a(E);                                                             ENDOP
/* SBC  A,H        */ OP(NONE,9c) THEN sbc_a(H);                                                             ENDOP
/* SBC  A,L        */ OP(NONE,9d) THEN sbc_a(L);                                                             ENDOP
/* SBC  A,(HL)     */ OP(NONE,9e) THEN TADR=HL; CALL rm(); THEN sbc_a(TDAT8);                                 ENDOP
/* SBC  A,A        */ OP(NONE,9f) THEN sbc_a(A);                                                             ENDOP

/* AND  B          */ OP(NONE,a0) THEN and_a(B);                                                             ENDOP
/* AND  C          */ OP(NONE,a1) THEN and_a(C);                                                             ENDOP
/* AND  D          */ OP(NONE,a2) THEN and_a(D);                                                             ENDOP
/* AND  E          */ OP(NONE,a3) THEN and_a(E);                                                             ENDOP
/* AND  H          */ OP(NONE,a4) THEN and_a(H);                                                             ENDOP
/* AND  L          */ OP(NONE,a5) THEN and_a(L);                                                             ENDOP
/* AND  (HL)       */ OP(NONE,a6) THEN TADR=HL; CALL rm(); THEN and_a(TDAT8);                                 ENDOP
/* AND  A          */ OP(NONE,a7) THEN and_a(A);                                                             ENDOP

/* XOR  B          */ OP(NONE,a8) THEN xor_a(B);                                                             ENDOP
/* XOR  C          */ OP(NONE,a9) THEN xor_a(C);                                                             ENDOP
/* XOR  D          */ OP(NONE,aa) THEN xor_a(D);                                                             ENDOP
/* XOR  E          */ OP(NONE,ab) THEN xor_a(E);                                                             ENDOP
/* XOR  H          */ OP(NONE,ac) THEN xor_a(H);                                                             ENDOP
/* XOR  L          */ OP(NONE,ad) THEN xor_a(L);                                                             ENDOP
/* XOR  (HL)       */ OP(NONE,ae) THEN TADR=HL; CALL rm(); THEN xor_a(TDAT8);                                 ENDOP
/* XOR  A          */ OP(NONE,af) THEN xor_a(A);                                                             ENDOP

/* OR   B          */ OP(NONE,b0) THEN or_a(B);                                                              ENDOP
/* OR   C          */ OP(NONE,b1) THEN or_a(C);                                                              ENDOP
/* OR   D          */ OP(NONE,b2) THEN or_a(D);                                                              ENDOP
/* OR   E          */ OP(NONE,b3) THEN or_a(E);                                                              ENDOP
/* OR   H          */ OP(NONE,b4) THEN or_a(H);                                                              ENDOP
/* OR   L          */ OP(NONE,b5) THEN or_a(L);                                                              ENDOP
/* OR   (HL)       */ OP(NONE,b6) THEN TADR=HL; CALL rm(); THEN or_a(TDAT8);                                  ENDOP
/* OR   A          */ OP(NONE,b7) THEN or_a(A);                                                              ENDOP

/* CP   B          */ OP(NONE,b8) THEN cp(B);                                                                ENDOP
/* CP   C          */ OP(NONE,b9) THEN cp(C);                                                                ENDOP
/* CP   D          */ OP(NONE,ba) THEN cp(D);                                                                ENDOP
/* CP   E          */ OP(NONE,bb) THEN cp(E);                                                                ENDOP
/* CP   H          */ OP(NONE,bc) THEN cp(H);                                                                ENDOP
/* CP   L          */ OP(NONE,bd) THEN cp(L);                                                                ENDOP
/* CP   (HL)       */ OP(NONE,be) THEN TADR=HL; CALL rm(); THEN cp(TDAT8);                                    ENDOP
/* CP   A          */ OP(NONE,bf) THEN cp(A);                                                                ENDOP

/* RET  NZ         */ OP(NONE,c0) THEN TDAT8=!(F & ZF); CALL ret_cond(0xc0);                                    ENDOP
/* POP  BC         */ OP(NONE,c1) CALL pop(); THEN BC=TDAT;                                                   ENDOP
/* JP   NZ,a       */ OP(NONE,c2) THEN TDAT8=!(F & ZF); CALL jp_cond();                                         ENDOP
/* JP   a          */ OP(NONE,c3) CALL jp();                                                                    ENDOP
/* CALL NZ,a       */ OP(NONE,c4) THEN TDAT8=!(F & ZF); CALL call_cond(0xc4);                                   ENDOP
/* PUSH BC         */ OP(NONE,c5) THEN TDAT=BC; CALL push();                                                    ENDOP
/* ADD  A,n        */ OP(NONE,c6) CALL arg(); THEN add_a(TDAT8);                                              ENDOP
/* RST  0          */ OP(NONE,c7) CALL rst(0x00);                                                               ENDOP

/* RET  Z          */ OP(NONE,c8) THEN TDAT8=(F & ZF); CALL ret_cond(0xc8);                                               ENDOP
/* RET             */ OP(NONE,c9) CALL pop(); THEN PC=TDAT; WZ = PCD;                                         ENDOP
/* JP   Z,a        */ OP(NONE,ca) THEN TDAT8=F & ZF; CALL jp_cond();                                            ENDOP
/* **** CB xx      */ OP(NONE,cb) CALL rop(); THENJP(CB)
/* CALL Z,a        */ OP(NONE,cc) THEN TDAT8=F & ZF; CALL call_cond(0xcc);                                      ENDOP
/* CALL a          */ OP(NONE,cd) CALL call();                                                                  ENDOP
/* ADC  A,n        */ OP(NONE,ce) CALL arg(); THEN adc_a(TDAT8);                                              ENDOP
/* RST  1          */ OP(NONE,cf) CALL rst(0x08);                                                               ENDOP

/* RET  NC         */ OP(NONE,d0) THEN TDAT8=!(F & CF); CALL ret_cond(0xd0);                                    ENDOP
/* POP  DE         */ OP(NONE,d1) CALL pop(); THEN DE=TDAT;                                                   ENDOP
/* JP   NC,a       */ OP(NONE,d2) THEN TDAT8=!(F & CF); CALL jp_cond();                                         ENDOP
/* OUT  (n),A      */ OP(NONE,d3) CALL arg(); THEN TADR = TDAT8 | (A << 8); TDAT=A; CALL out(); THEN WZ_L = ((TADR & 0xff) + 1) & 0xff;  WZ_H = A; ENDOP
/* CALL NC,a       */ OP(NONE,d4) THEN TDAT8=!(F & CF); CALL call_cond(0xd4);                                   ENDOP
/* PUSH DE         */ OP(NONE,d5) THEN TDAT=DE; CALL push();                                                    ENDOP
/* SUB  n          */ OP(NONE,d6) CALL arg(); THEN sub(TDAT8);                                                ENDOP
/* RST  2          */ OP(NONE,d7) CALL rst(0x10);                                                               ENDOP

/* RET  C          */ OP(NONE,d8) THEN TDAT8=(F & CF); CALL ret_cond(0xd8);                                               ENDOP
/* EXX             */ OP(NONE,d9) THEN exx();                                                                ENDOP
/* JP   C,a        */ OP(NONE,da) THEN TDAT8=F & CF; CALL jp_cond();                                            ENDOP
/* IN   A,(n)      */ OP(NONE,db) CALL arg(); THEN TADR = TDAT8 | (A << 8); CALL in(); THEN A = TDAT8; WZ = TADR + 1; ENDOP
/* CALL C,a        */ OP(NONE,dc) THEN TDAT8=F & CF; CALL call_cond(0xdc);                                      ENDOP
/* **** DD xx      */ OP(NONE,dd) CALL rop(); THENJP(DD)
/* SBC  A,n        */ OP(NONE,de) CALL arg(); THEN sbc_a(TDAT8);                                              ENDOP
/* RST  3          */ OP(NONE,df) CALL rst(0x18);                                                               ENDOP

/* RET  PO         */ OP(NONE,e0) THEN TDAT8=!(F & PF); CALL ret_cond(0xe0);                                    ENDOP
/* POP  HL         */ OP(NONE,e1) CALL pop(); THEN HL=TDAT;                                                   ENDOP
/* JP   PO,a       */ OP(NONE,e2) THEN TDAT8=!(F & PF); CALL jp_cond();                                         ENDOP
/* EX   HL,(SP)    */ OP(NONE,e3) THEN TDAT=HL; CALL ex_sp(); THEN HL=TDAT;                                    ENDOP
/* CALL PO,a       */ OP(NONE,e4) THEN TDAT8=!(F & PF); CALL call_cond(0xe4);                                   ENDOP
/* PUSH HL         */ OP(NONE,e5) THEN TDAT=HL; CALL push();                                                    ENDOP
/* AND  n          */ OP(NONE,e6) CALL arg(); THEN and_a(TDAT8);                                              ENDOP
/* RST  4          */ OP(NONE,e7) CALL rst(0x20);                                                               ENDOP

/* RET  PE         */ OP(NONE,e8) THEN TDAT8=(F & PF); CALL ret_cond(0xe8);                                               ENDOP
/* JP   (HL)       */ OP(NONE,e9) THEN PC = HL;                                                              ENDOP
/* JP   PE,a       */ OP(NONE,ea) THEN TDAT8=F & PF; CALL jp_cond();                                            ENDOP
/* EX   DE,HL      */ OP(NONE,eb) THEN std::swap(DE, HL);                                                    ENDOP
/* CALL PE,a       */ OP(NONE,ec) THEN TDAT8=F & PF; CALL call_cond(0xec);                                      ENDOP
/* **** ED xx      */ OP(NONE,ed) CALL rop(); THENJP(ED)
/* XOR  n          */ OP(NONE,ee) CALL arg(); THEN xor_a(TDAT8);                                              ENDOP
/* RST  5          */ OP(NONE,ef) CALL rst(0x28);                                                               ENDOP

/* RET  P          */ OP(NONE,f0) THEN TDAT8=!(F & SF); CALL ret_cond(0xf0);                                    ENDOP
/* POP  AF         */ OP(NONE,f1) CALL pop(); THEN AF=TDAT;                                                   ENDOP
/* JP   P,a        */ OP(NONE,f2) THEN TDAT8=!(F & SF); CALL jp_cond();                                         ENDOP
/* DI              */ OP(NONE,f3) THEN m_iff1 = m_iff2 = 0;                                                  ENDOP
/* CALL P,a        */ OP(NONE,f4) THEN TDAT8=!(F & SF); CALL call_cond(0xf4);                                   ENDOP
/* PUSH AF         */ OP(NONE,f5) THEN TDAT=AF; CALL push();                                                    ENDOP
/* OR   n          */ OP(NONE,f6) CALL arg(); THEN or_a(TDAT8);                                               ENDOP
/* RST  6          */ OP(NONE,f7) CALL rst(0x30);                                                               ENDOP

/* RET  M          */ OP(NONE,f8) THEN TDAT8=(F & SF); CALL ret_cond(0xf8);                                               ENDOP
/* LD   SP,HL      */ OP(NONE,f9) CALL nomreq_ir(2); THEN SP = HL;                                            ENDOP
/* JP   M,a        */ OP(NONE,fa) THEN TDAT8=F & SF; CALL jp_cond();                                            ENDOP
/* EI              */ OP(NONE,fb) THEN ei();                                                                 ENDOP
/* CALL M,a        */ OP(NONE,fc) THEN TDAT8=F & SF; CALL call_cond(0xfc);                                      ENDOP
/* **** FD xx      */ OP(NONE,fd) CALL rop(); THENJP(FD)
/* CP   n          */ OP(NONE,fe) CALL arg(); THEN cp(TDAT8);                                                 ENDOP
/* RST  7          */ OP(NONE,ff) CALL rst(0x38);                                                               ENDOP

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

	T(5);
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
	LOGINT("single INT irq_vector $%02x\n", irq_vector);

	/* 'interrupt latency' cycles */
	T(2);

	/* Interrupt mode 2. Call [i:databyte] */
	if( m_im == 2 )
	{
		// Zilog's datasheet claims that "the least-significant bit must be a zero."
		// However, experiments have confirmed that IM 2 vectors do not have to be
		// even, and all 8 bits will be used; even $FF is handled normally.
		/* CALL opcode timing */
		T(5);
		wm16_sp(m_pc);
		irq_vector = (irq_vector & 0xff) | (m_i << 8);
		rm16(irq_vector, m_pc);
		LOGINT("IM2 [$%04x] = $%04x\n", irq_vector, PCD);
	}
	else
	/* Interrupt mode 1. RST 38h */
	if( m_im == 1 )
	{
		LOGINT("'%s' IM1 $0038\n", tag());
		/* RST $38 */
		T(5);
		wm16_sp(m_pc);
		PCD = 0x0038;
	}
	else
	{
		/* Interrupt mode 0. We check for CALL and JP instructions, */
		/* if neither of these were found we assume a 1 byte opcode */
		/* was placed on the databus                                */
		LOGINT("IM0 $%04x\n", irq_vector);

		/* check for nop */
		if (irq_vector != 0x00)
		{
			switch (irq_vector & 0xff0000)
			{
				case 0xcd0000:  /* call */
					/* CALL $xxxx cycles */
					T(11);
					wm16_sp(m_pc);
					PCD = irq_vector & 0xffff;
					break;
				case 0xc30000:  /* jump */
					/* JP $xxxx cycles */
					T(10);
					PCD = irq_vector & 0xffff;
					break;
				default:        /* rst (or other opcodes?) */
					if (irq_vector == 0xfb)
					{
						// EI
						T(4);
						ei();
					}
					else if ((irq_vector & 0xc7) == 0xc7)
					{
						/* RST $xx cycles */
						T(5);
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

DEF( nomreq_ir(s8 cycles) )
	THEN
		TADR = (m_i << 8) | (m_r2 & 0x80) | (m_r & 0x7f);
	CALL nomreq_addr(cycles);
ENDDEF

z80_device::ops_type z80_device::nomreq_addr(s8 cycles)
{
    ops_type steps;
	while(cycles--) {
		steps.push_back([&]() { m_nomreq_cb(TADR, 0x00, 0xff); });
		steps.push_back([&]() { T(1); });
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
	T(7);
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
	else
	{
		T(2 * m_memrq_cycles);
	}

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
	save_item(NAME(m_q));
	save_item(NAME(m_qtemp));
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

	IX = IY = 0xffff; // IX and IY are FFFF after a reset!
	set_f(ZF);        // Zero flag is set

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

inline void z80_device::execute_cycles(u8 icount)
{
	m_icount -= icount;
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
			v[m_cycle++]();
			if ((m_icount < 0) && access_to_be_redone())
			{
				m_icount = icount;
				m_cycle--;
				return;
			}
		}

		if(m_cycle >= v.size())
			m_cycle = 0;
	}
}

DEF( next_op() )
	THEN
		// check for interrupts before each instruction
		check_interrupts();
	//THEN
		m_after_ei = false;
		m_after_ldair = false;

		PRVPC = PCD;
		debugger_instruction_hook(PCD);
	CALL rop();
	THEN
		m_prefix = NONE;
		m_opcode = TDAT8;
		// when in HALT state, the fetched opcode is not dispatched (aka a NOP)
		if (m_halt)
		{
			PC--;
			m_opcode = 0;
		}
ENDDEF

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
