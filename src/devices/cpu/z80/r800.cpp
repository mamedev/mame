// license:BSD-3-Clause
// copyright-holders:AJR,Juergen Buchmueller,Wilbert Pol
/***************************************************************************

    ASCII R800 CPU

TODO:
- Internal configuration registers.
- External 24 bits address bus accessible through 9 memory mappers.
- DMA channels.
- Interrupt levels.
- Bits 3 and 5 of the flag register behave differently from the z80.
- Page break wait states.
- Refresh delays.

***************************************************************************/

#include "emu.h"
#include "r800.h"
#include "r800dasm.h"

#define LOG_UNDOC (1U << 1)
#define LOG_INT   (1U << 2)
#define LOG_TIME  (1U << 3)

#define VERBOSE ( LOG_UNDOC /*| LOG_INT*/ )
#include "logmacro.h"

#define LOGUNDOC(...) LOGMASKED(LOG_UNDOC, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(R800, r800_device, "r800", "ASCII R800")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

#define HAS_LDAIR_QUIRK  0

//-------------------------------------------------
//  r800_device - constructor
//-------------------------------------------------

r800_device::r800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, R800, tag, owner, clock)
{
    z80_set_m1_cycles(1);
    z80_set_memrq_cycles(1);
    z80_set_iorq_cycles(1);
}

std::unique_ptr<util::disasm_interface> r800_device::create_disassembler()
{
	return std::make_unique<r800_disassembler>();
}

void r800_device::device_validity_check(validity_checker &valid) const
{
	cpu_device::device_validity_check(valid);
}



// Mostly copy/pasted code from z80.cpp follows

/****************************************************************************
 * The Z80 registers. halt is set to 1 when the CPU is halted, the refresh
 * register is calculated as follows: refresh = (r & 127) | (r2 & 128)
 ****************************************************************************/
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

#define PRVPC   m_prvpc.d     // previous program counter

#define PCD     m_pc.d
#define PC      m_pc.w.l

#define SPD     m_sp.d
#define SP      m_sp.w.l

#define AFD     m_af.d
#define AF      m_af.w.l
#define A       m_af.b.h
#define F       m_af.b.l
#define Q       m_q
#define QT      m_qtemp
#define I       m_i
#define R       m_r
#define R2      m_r2

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

#define TADR     m_shared_addr.w   // Typically represents values from A0..15 pins. 16bit input in steps.
#define TADR_H   m_shared_addr.b.h
#define TADR_L   m_shared_addr.b.l
#define TDAT     m_shared_data.w   // 16bit input(if use as second parameter) or output in steps.
#define TDAT2    m_shared_data2.w
#define TDAT_H   m_shared_data.b.h
#define TDAT_L   m_shared_data.b.l
#define TDAT8    m_shared_data.b.l // Typically represents values from D0..8 pins. 8bit input or output in steps.


/***************************************************************
 * adjust cycle count by n T-states
 ***************************************************************/
#define T(icount) execute_cycles(icount)

/***************************************************************
 * Enter halt state; write 1 to callback on first execution
 ***************************************************************/
inline void r800_device::halt()
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
inline void r800_device::leave_halt()
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
inline u8 r800_device::in(u16 port)
{
	u8 res = m_io.read_byte(port);
	T(m_iorq_cycles);
	return res;
}

/***************************************************************
 * Output a byte to given I/O port
 ***************************************************************/
inline void r800_device::out(u16 port, u8 value)
{
	m_io.write_byte(port, value);
	T(m_iorq_cycles);
}

/***************************************************************
 * Read a byte from given memory location
 ***************************************************************/
inline u8 r800_device::data_read(u16 addr)
{
	return m_data.read_byte(translate_memory_address(addr));
}

u8 r800_device::rm(u16 addr)
{
	u8 res = data_read(addr);
	T(m_memrq_cycles);
	return res;
}

//u8 r800_device::rm_reg(u16 addr)
//{
//	u8 res = rm(addr);
//	nomreq_addr(addr, 1);
//	return res;
//}

/***************************************************************
 * Read a word from given memory location
 ***************************************************************/
inline void r800_device::rm16(u16 addr, PAIR &r)
{
	r.b.l = data_read(addr);
	T(m_memrq_cycles);
	r.b.h = data_read(addr + 1);
	T(m_memrq_cycles);
}

/***************************************************************
 * Write a byte to given memory location
 ***************************************************************/
inline void r800_device::data_write(u16 addr, u8 value)
{
	m_data.write_byte(translate_memory_address((u32)addr), value);
}

inline void r800_device::wm(u16 addr, u8 value)
{
	data_write(addr, value);
	T(m_memrq_cycles);
}

/***************************************************************
 * Write a word to given memory location
 ***************************************************************/
inline void r800_device::wm16(u16 addr, PAIR &r)
{
	wm(addr, r.b.l);
	wm(addr + 1, r.b.h);
}

/***************************************************************
 * Write a word to (SP)
 *  in: TDAT
 ***************************************************************/
inline void r800_device::wm16_sp(PAIR &r)
{
	SP--;
	wm(SPD, r.b.h);
	SP--;
	wm(SPD, r.b.l);
}

/***************************************************************
 * rop() is identical to rm() except it is used for
 * reading opcodes. In case of system with memory mapped I/O,
 * this function can be used to greatly speed up emulation
 ***************************************************************/
inline u8 r800_device::opcode_read()
{
	return m_opcodes.read_byte(translate_memory_address(PCD));
}

u8 r800_device::rop()
{
	u8 res = opcode_read();
	T(m_m1_cycles);
	PC++;
	m_r++;
	Q = m_qtemp;
	m_qtemp = YF | XF;

	return res;
}

/****************************************************************
 * arg() is identical to rop() except it is used
 * for reading opcode arguments. This difference can be used to
 * support systems that use different encoding mechanisms for
 * opcodes and opcode arguments
 * out: TDAT8
 ***************************************************************/
inline u8 r800_device::arg_read()
{
	return m_args.read_byte(translate_memory_address(PCD));
}

u8 r800_device::arg()
{
	u8 res = arg_read();
	T(m_memrq_cycles);
	PC++;

	return res;
}

u16 r800_device::arg16()
{
	u8 const res = arg();

	return (u16(arg()) << 8) | res;
}

/***************************************************************
 * Calculate the effective address EA of an opcode using
 * IX+offset resp. IY+offset addressing.
 ***************************************************************/
inline void r800_device::eax()
{
	m_ea = (u32)(u16)(IX + (s8)arg());
	WZ = m_ea;
}

inline void r800_device::eay()
{
	m_ea = (u32)(u16)(IY + (s8)arg());
	WZ = m_ea;
}

/***************************************************************
 * POP
 ***************************************************************/
inline void r800_device::pop(PAIR &r)
{
	rm16(SPD, r);
	SP += 2;
}

/***************************************************************
 * PUSH
 ***************************************************************/
//inline void r800_device::push(PAIR &r)
//{
//	nomreq_ir(1);
//	wm16_sp(r);
//}

/***************************************************************
 * JP
 ***************************************************************/
inline void r800_device::jp(void)
{
	PCD = arg16();
	WZ = PCD;
}

/***************************************************************
 * JP_COND
 ***************************************************************/
inline void r800_device::jp_cond(bool cond)
{
	if (cond)
	{
		PCD = arg16();
		WZ = PCD;
	}
	else
		WZ = arg16(); // implicit do PC += 2
}

/***************************************************************
 * JR
 ***************************************************************/
inline void r800_device::jr()
{
	s8 a = (s8)arg(); // arg() also increments PC
	PC += a; // so don't do PC += arg()
	WZ = PC;
    T(1);
}

/***************************************************************
 * JR_COND
 ***************************************************************/
inline void r800_device::jr_cond(bool cond, u8 opcode)
{
	if (cond)
	{
		jr();
	}
	else
	{
		arg();
	}
}

/***************************************************************
 * CALL
 ***************************************************************/
inline void r800_device::call()
{
	m_ea = arg16();
	WZ = m_ea;
	wm16_sp(m_pc);
	PCD = m_ea;
}

/***************************************************************
 * CALL_COND
 ***************************************************************/
inline void r800_device::call_cond(bool cond, u8 opcode)
{
	if (cond)
	{
		m_ea = arg16();
		WZ = m_ea;
		wm16_sp(m_pc);
		PCD = m_ea;
	}
	else
		WZ = arg16(); // implicit call PC+=2;
}

/***************************************************************
 * RET_COND
 ***************************************************************/
inline void r800_device::ret_cond(bool cond, u8 opcode)
{
	if (cond)
	{
		pop(m_pc);
		WZ = PC;
	}
}

/***************************************************************
 * RETN
 ***************************************************************/
inline void r800_device::retn()
{
	LOGINT("RETN m_iff1:%d m_iff2:%d\n", m_iff1, m_iff2);
	pop(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
}

/***************************************************************
 * RETI
 ***************************************************************/
inline void r800_device::reti()
{
	pop(m_pc);
	WZ = PC;
	m_iff1 = m_iff2;
	daisy_call_reti_device();
}

/***************************************************************
 * LD   R,A
 ***************************************************************/
inline void r800_device::ld_r_a()
{
	m_r = A;
	m_r2 = A & 0x80; // keep bit 7 of r
}

/***************************************************************
 * LD   A,R
 ***************************************************************/
inline void r800_device::ld_a_r()
{
	A = (m_r & 0x7f) | m_r2;
	set_f((F & CF) | SZ[A] | (m_iff2 << 2));
	m_after_ldair = true;
}

/***************************************************************
 * LD   I,A
 ***************************************************************/
inline void r800_device::ld_i_a()
{
	m_i = A;
}

/***************************************************************
 * LD   A,I
 ***************************************************************/
inline void r800_device::ld_a_i()
{
	A = m_i;
	set_f((F & CF) | SZ[A] | (m_iff2 << 2));
	m_after_ldair = true;
}

/***************************************************************
 * RST
 ***************************************************************/
inline void r800_device::rst(u16 addr)
{
	push(m_pc);
	PCD = addr;
	WZ = PC;
}

/***************************************************************
 * INC  r8
 ***************************************************************/
inline u8 r800_device::inc(u8 value)
{
	u8 res = value + 1;
	set_f((F & CF) | SZHV_inc[res]);
	return (u8)res;
}

/***************************************************************
 * DEC  r8
 ***************************************************************/
inline u8 r800_device::dec(u8 value)
{
	u8 res = value - 1;
	set_f((F & CF) | SZHV_dec[res]);
	return res;
}

/***************************************************************
 * RLCA
 ***************************************************************/
inline void r800_device::rlca()
{
	A = (A << 1) | (A >> 7);
	set_f((F & (SF | ZF | PF)) | (A & (YF | XF | CF)));
}

/***************************************************************
 * RRCA
 ***************************************************************/
inline void r800_device::rrca()
{
	set_f((F & (SF | ZF | PF)) | (A & CF));
	A = (A >> 1) | (A << 7);
	F |= (A & (YF | XF));
}

/***************************************************************
 * RLA
 ***************************************************************/
inline void r800_device::rla()
{
	u8 res = (A << 1) | (F & CF);
	u8 c = (A & 0x80) ? CF : 0;
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * RRA
 ***************************************************************/
inline void r800_device::rra()
{
	u8 res = (A >> 1) | (F << 7);
	u8 c = (A & 0x01) ? CF : 0;
	set_f((F & (SF | ZF | PF)) | c | (res & (YF | XF)));
	A = res;
}

/***************************************************************
 * RRD
 ***************************************************************/
inline void r800_device::rrd()
{
	u8 n = rm(HL);
	WZ = HL + 1;
//	nomreq_addr(HL, 4);
	wm(HL, (n >> 4) | (A << 4));
	A = (A & 0xf0) | (n & 0x0f);
	set_f((F & CF) | SZP[A]);
}

/***************************************************************
 * RLD
 ***************************************************************/
inline void r800_device::rld()
{
	u8 n = rm(HL);
	WZ = HL + 1;
//	nomreq_addr(HL, 4);
	wm(HL, (n << 4) | (A & 0x0f));
	A = (A & 0xf0) | (n >> 4);
	set_f((F & CF) | SZP[A]);
}

/***************************************************************
 * ADD  A,n
 ***************************************************************/
inline void r800_device::add_a(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) + value);
	set_f(SZHVC_add[ah | res]);
	A = res;
}

/***************************************************************
 * ADC  A,n
 ***************************************************************/
inline void r800_device::adc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) + value + c);
	set_f(SZHVC_add[(c << 16) | ah | res]);
	A = res;
}

/***************************************************************
 * SUB  n
 ***************************************************************/
inline void r800_device::sub(u8 value)
{
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - value);
	set_f(SZHVC_sub[ah | res]);
	A = res;
}

/***************************************************************
 * SBC  A,n
 ***************************************************************/
inline void r800_device::sbc_a(u8 value)
{
	u32 ah = AFD & 0xff00, c = AFD & 1;
	u32 res = (u8)((ah >> 8) - value - c);
	set_f(SZHVC_sub[(c << 16) | ah | res]);
	A = res;
}

/***************************************************************
 * NEG
 ***************************************************************/
inline void r800_device::neg()
{
	u8 value = A;
	A = 0;
	sub(value);
}

/***************************************************************
 * DAA
 ***************************************************************/
inline void r800_device::daa()
{
	u8 a = A;
	if (F & NF)
	{
		if ((F & HF) | ((A & 0xf) > 9))
			a -= 6;
		if ((F & CF) | (A > 0x99))
			a -= 0x60;
	}
	else
	{
		if ((F & HF) | ((A & 0xf) > 9))
			a += 6;
		if ((F & CF) | (A > 0x99))
			a += 0x60;
	}

	set_f((F & (CF | NF)) | (A > 0x99) | ((A ^ a) & HF) | SZP[a]);
	A = a;
}

/***************************************************************
 * AND  n
 ***************************************************************/
inline void r800_device::and_a(u8 value)
{
	A &= value;
	set_f(SZP[A] | HF);
}

/***************************************************************
 * OR   n
 ***************************************************************/
inline void r800_device::or_a(u8 value)
{
	A |= value;
	set_f(SZP[A]);
}

/***************************************************************
 * XOR  n
 ***************************************************************/
inline void r800_device::xor_a(u8 value)
{
	A ^= value;
	set_f(SZP[A]);
}

/***************************************************************
 * CP   n
 ***************************************************************/
inline void r800_device::cp(u8 value)
{
	unsigned val = value;
	u32 ah = AFD & 0xff00;
	u32 res = (u8)((ah >> 8) - val);
	set_f((SZHVC_sub[ah | res] & ~(YF | XF)) | (val & (YF | XF)));
}

/***************************************************************
 * EXX
 ***************************************************************/
inline void r800_device::exx()
{
	using std::swap;
	swap(m_bc, m_bc2);
	swap(m_de, m_de2);
	swap(m_hl, m_hl2);
}

/***************************************************************
 * EX   (SP),r16
 *  in: TDAT
 ***************************************************************/
inline void r800_device::ex_sp(PAIR &r)
{
	PAIR tmp = {{0, 0, 0, 0}};
	pop(tmp);
//	nomreq_addr(SPD - 1, 1);
	wm16_sp(r);
//	nomreq_addr(SPD, 2);
	r = tmp;
	WZ = r.d;
}

/***************************************************************
 * ADD16
 ***************************************************************/
inline void r800_device::add16(PAIR &dr, PAIR &sr)
{
	u32 res = dr.d + sr.d;
	WZ = dr.d + 1;
	set_f((F & (SF | ZF | VF)) |
		  (((dr.d ^ res ^ sr.d) >> 8) & HF) |
		  ((res >> 16) & CF) | ((res >> 8) & (YF | XF)));
	dr.w.l = (u16)res;
}

/***************************************************************
 * ADC  HL,r16
 ***************************************************************/
inline void r800_device::adc_hl(PAIR &r)
{
	u32 res = HLD + r.d + (F & CF);
	WZ = HL + 1;
	set_f((((HLD ^ res ^ r.d) >> 8) & HF) |
		  ((res >> 16) & CF) |
		  ((res >> 8) & (SF | YF | XF)) |
		  ((res & 0xffff) ? 0 : ZF) |
		  (((r.d ^ HLD ^ 0x8000) & (r.d ^ res) & 0x8000) >> 13));
	HL = (u16)res;
}

/***************************************************************
 * SBC  HL,r16
 ***************************************************************/
inline void r800_device::sbc_hl(PAIR &r)
{
	u32 res = HLD - r.d - (F & CF);
	WZ = HL + 1;
	set_f((((HLD ^ res ^ r.d) >> 8) & HF) | NF |
		  ((res >> 16) & CF) |
		  ((res >> 8) & (SF | YF | XF)) |
		  ((res & 0xffff) ? 0 : ZF) |
		  (((r.d ^ HLD) & (HLD ^ res) & 0x8000) >> 13));
	HL = (u16)res;
}

/***************************************************************
 * RLC  r8
 ***************************************************************/
inline u8 r800_device::rlc(u8 value)
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
inline u8 r800_device::rrc(u8 value)
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
inline u8 r800_device::rl(u8 value)
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
inline u8 r800_device::rr(u8 value)
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
inline u8 r800_device::sla(u8 value)
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
inline u8 r800_device::sra(u8 value)
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
inline u8 r800_device::sll(u8 value)
{
	unsigned res = value;
	unsigned c = (res & 0x80) ? CF : 0;
	res = (res << 1) & 0xff;
	set_f(SZP[res] | c);
	return res;
}

/***************************************************************
 * SRL  r8
 ***************************************************************/
inline u8 r800_device::srl(u8 value)
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
inline void r800_device::bit(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | (value & (YF | XF)));
}

/***************************************************************
 * BIT  bit,(HL)
 ***************************************************************/
inline void r800_device::bit_hl(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | (WZ_H & (YF | XF)));
}

/***************************************************************
 * BIT  bit,(IX/Y+o)
 ***************************************************************/
inline void r800_device::bit_xy(int bit, u8 value)
{
	set_f((F & CF) | HF | (SZ_BIT[value & (1 << bit)] & ~(YF | XF)) | ((m_ea >> 8) & (YF | XF)));
}

/***************************************************************
 * RES  bit,r8
 ***************************************************************/
inline u8 r800_device::res(int bit, u8 value)
{
	return value & ~(1 << bit);
}

/***************************************************************
 * SET  bit,r8
 ***************************************************************/
inline u8 r800_device::set(int bit, u8 value)
{
	return value | (1 << bit);
}

/***************************************************************
 * LDI
 ***************************************************************/
inline void r800_device::ldi()
{
	u8 io = rm(HL);
	wm(DE, io);
//	nomreq_addr(DE, 1);
	set_f(F & (SF | ZF | CF));
	if ((A + io) & 0x02)
		F |= YF; // bit 1 -> flag 5
	if ((A + io) & 0x08)
		F |= XF; // bit 3 -> flag 3
	HL++;
	DE++;
	BC--;
	if (BC)
		F |= VF;
}

/***************************************************************
 * CPI
 ***************************************************************/
inline void r800_device::cpi()
{
	u8 val = rm(HL);
//	nomreq_addr(HL, 1);
	u8 res = A - val;
	WZ++;
	HL++;
	BC--;
	set_f((F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF);
	if (F & HF)
		res -= 1;
	if (res & 0x02)
		F |= YF; // bit 1 -> flag 5
	if (res & 0x08)
		F |= XF; // bit 3 -> flag 3
	if (BC)
		F |= VF;
}

/***************************************************************
 * INI
 ***************************************************************/
inline u8 r800_device::ini(bool take_extra_cycle)
{
//    if (take_extra_cycle)
//	    nomreq_ir(1);
	const u8 io = in(BC);
	WZ = BC + 1;
	B--;
	wm(HL, io);
	HL++;
	set_f(SZ[B]);
	unsigned t = (unsigned)((C + 1) & 0xff) + (unsigned)io;
	if (io & SF)
		F |= NF;
	if (t & 0x100)
		F |= HF | CF;
	F |= SZP[(u8)(t & 0x07) ^ B] & PF;

	return io;
}

/***************************************************************
 * OUTI
 ***************************************************************/
inline u8 r800_device::outi(bool take_extra_cycle)
{
//    if (take_extra_cycle)
//	    nomreq_ir(1);
	const u8 io = rm(HL);
	B--;
	WZ = BC + 1;
	out(BC, io);
	HL++;
	set_f(SZ[B]);
	unsigned t = (unsigned)L + (unsigned)io;
	if (io & SF)
		F |= NF;
	if (t & 0x100)
		F |= HF | CF;
	F |= SZP[(u8)(t & 0x07) ^ B] & PF;

	return io;
}

/***************************************************************
 * LDD
 ***************************************************************/
inline void r800_device::ldd()
{
	const u8 io = rm(HL);
	wm(DE, io);
//	nomreq_addr(DE, 1);
	set_f(F & (SF | ZF | CF));
	if ((A + io) & 0x02)
		F |= YF; // bit 1 -> flag 5
	if ((A + io) & 0x08)
		F |= XF; // bit 3 -> flag 3
	HL--;
	DE--;
	BC--;
	if (BC)
		F |= VF;
}

/***************************************************************
 * CPD
 ***************************************************************/
inline void r800_device::cpd()
{
	u8 val = rm(HL);
//	nomreq_addr(HL, 1);
	u8 res = A - val;
	WZ--;
	HL--;
	BC--;
	set_f((F & CF) | (SZ[res] & ~(YF | XF)) | ((A ^ val ^ res) & HF) | NF);
	if (F & HF)
		res -= 1;
	if (res & 0x02)
		F |= YF; // bit 1 -> flag 5
	if (res & 0x08)
		F |= XF; // bit 3 -> flag 3
	if (BC)
		F |= VF;
}

/***************************************************************
 * IND
 ***************************************************************/
inline u8 r800_device::ind(bool take_extra_cycle)
{
//    if (take_extra_cycle)
//	    nomreq_ir(1);
	const u8 io = in(BC);
	WZ = BC - 1;
	B--;
	wm(HL, io);
	HL--;
	set_f(SZ[B]);
	unsigned t = ((unsigned)(C - 1) & 0xff) + (unsigned)io;
	if (io & SF)
		F |= NF;
	if (t & 0x100)
		F |= HF | CF;
	F |= SZP[(u8)(t & 0x07) ^ B] & PF;

	return io;
}

/***************************************************************
 * OUTD
 ***************************************************************/
inline u8 r800_device::outd(bool take_extra_cycle)
{
//    if (take_extra_cycle)
//	    nomreq_ir(1);
	const u8 io = rm(HL);
	B--;
	WZ = BC - 1;
	out(BC, io);
	HL--;
	set_f(SZ[B]);
	unsigned t = (unsigned)L + (unsigned)io;
	if (io & SF)
		F |= NF;
	if (t & 0x100)
		F |= HF | CF;
	F |= SZP[(u8)(t & 0x07) ^ B] & PF;

	return io;
}

/***************************************************************
 * LDIR
 ***************************************************************/
inline void r800_device::ldir()
{
	ldi();
	if (BC != 0)
	{
//		nomreq_addr(DE, 1);
		PC -= 2;
		WZ = PC + 1;
		F &= ~(YF | XF);
		F |= (PC >> 8) & (YF | XF);
	}
}

/***************************************************************
 * CPIR
 ***************************************************************/
inline void r800_device::cpir()
{
	cpi();
//	nomreq_addr(HL, 1);
	if (BC != 0 && !(F & ZF))
	{
		PC -= 2;
		WZ = PC + 1;
		F &= ~(YF | XF);
		F |= (PC >> 8) & (YF | XF);
	}
}

/***************************************************************
 * INIR
 ***************************************************************/
inline void r800_device::inir()
{
	const u8 data = ini(B != 1);
	if (B != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		block_io_interrupted_flags(data);
	}
}

/***************************************************************
 * OTIR
 ***************************************************************/
inline void r800_device::otir()
{
	const u8 data = outi(B != 1);
	if (B != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		block_io_interrupted_flags(data);
	}
}

/***************************************************************
 * LDDR
 ***************************************************************/
inline void r800_device::lddr()
{
	ldd();
	if (BC != 0)
	{
//		nomreq_addr(DE, 1);
		PC -= 2;
		WZ = PC + 1;
		F &= ~(YF | XF);
		F |= (PC >> 8) & (YF | XF);
	}
}

/***************************************************************
 * CPDR
 ***************************************************************/
inline void r800_device::cpdr()
{
	cpd();
//	nomreq_addr(HL, 1);
	if (BC != 0 && !(F & ZF))
	{
		PC -= 2;
		WZ = PC + 1;
		F &= ~(YF | XF);
		F |= (PC >> 8) & (YF | XF);
	}
}

/***************************************************************
 * INDR
 ***************************************************************/
inline void r800_device::indr()
{
	const u8 data = ind(B != 1);
	if (B != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		block_io_interrupted_flags(data);
	}
}

/***************************************************************
 * OTDR
 ***************************************************************/
inline void r800_device::otdr()
{
	const u8 data = outd(B != 1);
	if (B != 0)
	{
		PC -= 2;
		WZ = PC + 1;
		block_io_interrupted_flags(data);
	}
}

/***************************************************************
 * EI
 ***************************************************************/
inline void r800_device::ei()
{
	m_iff1 = m_iff2 = 1;
	m_after_ei = true;
}

inline void r800_device::mulub(u8 value)
{
    u16 res = A * value;
    HL = res;
	unsigned c = (res & 0xff00) ? CF : 0;
    unsigned z = (res) ? 0 : ZF;
	set_f((F & (HF|NF)) | z | c);
}

inline void r800_device::muluw(u16 value)
{
    u32 res = HL * value;
    DE = res >> 16;
    HL = res & 0xffff;
	unsigned c = (res & 0xffff0000) ? CF : 0;
    unsigned z = (res) ? 0 : ZF;
	set_f((F & (HF|NF)) | z | c);
}

inline void r800_device::set_f(u8 f)
{
	m_qtemp = 0;
	F = f;
}

inline void r800_device::block_io_interrupted_flags()
{
	// TODO Remove me
}

inline void r800_device::block_io_interrupted_flags(u8 data)
{
	F &= ~(YF | XF);
	F |= (PC >> 8) & (YF | XF);
	if (F & CF)
	{
		F &= ~HF;
		if (data & 0x80)
		{
			F ^= (SZP[(B - 1) & 0x07] ^ PF) & PF;
			if ((B & 0x0f) == 0x00)
				F |= HF;
		}
		else
		{
			F ^= (SZP[(B + 1) & 0x07] ^ PF) & PF;
			if ((B & 0x0f) == 0x0f)
				F |= HF;
		}
	}
	else
	{
		F ^= (SZP[B & 0x07] ^ PF) & PF;
	}
}

inline void r800_device::illegal_1()
{
	LOGUNDOC("ill. opcode $%02x $%02x ($%04x)\n",
			 m_opcodes.read_byte(translate_memory_address((PCD - 1) & 0xffff)), m_opcodes.read_byte(translate_memory_address(PCD)), PCD - 1);
}

inline void r800_device::illegal_2()
{
	LOGUNDOC("ill. opcode $ed $%02x\n",
			 m_opcodes.read_byte(translate_memory_address((PCD - 1) & 0xffff)));
}


inline void r800_device::execute_cycles(u8 icount)
{
	m_icount -= icount;
}

void r800_device::do_rop()
{
//	#include "cpu/z80/z80_rop.hxx"
}

void r800_device::do_op()
{
//	#include "cpu/z80/r800.hxx"
//	m_ref = 0xffff00;

	bool check_parent = false;
	#include "cpu/z80/r800.hxx"
	if (!check_parent)
	{
		m_ref = 0xffff00;
		return;
	}
	z80_device::do_op();
}
