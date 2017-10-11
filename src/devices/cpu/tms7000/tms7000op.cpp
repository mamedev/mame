// license:BSD-3-Clause
// copyright-holders:hap, Tim Lindner

// TMS7000 opcode handlers

#include "emu.h"
#include "tms7000.h"

// flag helpers
#define GET_C()     (m_sr >> 7 & 1)
#define SET_C(x)    m_sr = (m_sr & 0x7f) | ((x) >> 1 & 0x80)
#define SET_NZ(x)   m_sr = (m_sr & 0x9f) | ((x) >> 1 & 0x40) | (((x) & 0xff) ? 0 : 0x20)
#define SET_CNZ(x)  m_sr = (m_sr & 0x1f) | ((x) >> 1 & 0xc0) | (((x) & 0xff) ? 0 : 0x20)


// addressing modes (not all opcodes have a write cycle)
#define WB_NO -1
#define AM_WB(write_func, address, param1, param2) \
	int result = (this->*op)(param1, param2); \
	if (result > WB_NO) write_func(address, result)

void tms7000_device::am_a(op_func op)
{
	m_icount -= 5;
	AM_WB(write_r8, 0, read_r8(0), 0);
}

void tms7000_device::am_b(op_func op)
{
	m_icount -= 5;
	AM_WB(write_r8, 1, read_r8(1), 0);
}

void tms7000_device::am_r(op_func op)
{
	m_icount -= 7;
	uint8_t r = imm8();
	AM_WB(write_r8, r, read_r8(r), 0);
}

void tms7000_device::am_a2a(op_func op)
{
	m_icount -= 6;
	AM_WB(write_r8, 0, read_r8(0), read_r8(0));
}

void tms7000_device::am_a2b(op_func op)
{
	m_icount -= 6;
	AM_WB(write_r8, 1, read_r8(1), read_r8(0));
}

void tms7000_device::am_a2p(op_func op)
{
	m_icount -= 10;
	uint8_t r = imm8();
	AM_WB(write_p, r, read_p(r), read_r8(0));
}

void tms7000_device::am_a2r(op_func op)
{
	m_icount -= 8;
	uint8_t r = imm8();
	AM_WB(write_r8, r, read_r8(r), read_r8(0));
}

void tms7000_device::am_b2a(op_func op)
{
	m_icount -= 5;
	AM_WB(write_r8, 0, read_r8(0), read_r8(1));
}

void tms7000_device::am_b2b(op_func op)
{
	m_icount -= 6;
	AM_WB(write_r8, 1, read_r8(1), read_r8(1));
}

void tms7000_device::am_b2r(op_func op)
{
	m_icount -= 7;
	uint8_t r = imm8();
	AM_WB(write_r8, r, read_r8(r), read_r8(1));
}

void tms7000_device::am_b2p(op_func op)
{
	m_icount -= 9;
	uint8_t r = imm8();
	AM_WB(write_p, r, read_p(r), read_r8(1));
}

void tms7000_device::am_r2a(op_func op)
{
	m_icount -= 8;
	AM_WB(write_r8, 0, read_r8(0), read_r8(imm8()));
}

void tms7000_device::am_r2b(op_func op)
{
	m_icount -= 8;
	AM_WB(write_r8, 1, read_r8(1), read_r8(imm8()));
}

void tms7000_device::am_r2r(op_func op)
{
	m_icount -= 10;
	uint8_t param2 = read_r8(imm8());
	uint8_t r = imm8();
	AM_WB(write_r8, r, read_r8(r), param2);
}

void tms7000_device::am_i2a(op_func op)
{
	m_icount -= 7;
	AM_WB(write_r8, 0, read_r8(0), imm8());
}

void tms7000_device::am_i2b(op_func op)
{
	m_icount -= 7;
	AM_WB(write_r8, 1, read_r8(1), imm8());
}

void tms7000_device::am_i2r(op_func op)
{
	m_icount -= 9;
	uint8_t param2 = imm8();
	uint8_t r = imm8();
	AM_WB(write_r8, r, read_r8(r), param2);
}

void tms7000_device::am_i2p(op_func op)
{
	m_icount -= 11;
	uint8_t param2 = imm8();
	uint8_t r = imm8();
	AM_WB(write_p, r, read_p(r), param2);
}

void tms7000_device::am_p2a(op_func op)
{
	m_icount -= 9;
	AM_WB(write_r8, 0, read_r8(0), read_p(imm8()));
}

void tms7000_device::am_p2b(op_func op)
{
	m_icount -= 8;
	AM_WB(write_r8, 1, read_r8(1), read_p(imm8()));
}



// common opcodes
// 1 param
int tms7000_device::op_clr(uint8_t param1, uint8_t param2)
{
	uint8_t t = 0;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_dec(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 - 1;
	SET_NZ(t);
	SET_C(~t);
	return t;
}

int tms7000_device::op_inc(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 + 1;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_inv(uint8_t param1, uint8_t param2)
{
	uint8_t t = ~param1;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_rl(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 << 1 | param1 >> 7;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_rlc(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 << 1 | GET_C();
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_rr(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 >> 1 | param1 << 8 | (param1 << 7 & 0x80);
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_rrc(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 >> 1 | param1 << 8 | GET_C() << 7;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_swap(uint8_t param1, uint8_t param2)
{
	m_icount -= 3;
	uint16_t t = param1 >> 4 | param1 << 4;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_xchb(uint8_t param1, uint8_t param2)
{
	m_icount -= 1;
	uint8_t t = read_r8(1);
	SET_CNZ(t);
	write_r8(1, param1);
	return t;
}

// 2 params
int tms7000_device::op_adc(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 + param2 + GET_C();
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_add(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 + param2;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_and(uint8_t param1, uint8_t param2)
{
	uint8_t t = param1 & param2;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_cmp(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 - param2;
	SET_NZ(t);
	SET_C(~t);
	return WB_NO;
}

int tms7000_device::op_mpy(uint8_t param1, uint8_t param2)
{
	m_icount -= 39;
	uint16_t t = param1 * param2;
	SET_CNZ(t >> 8);
	write_mem16(0, t); // always writes result to regs A-B
	return WB_NO;
}

int tms7000_device::op_mov(uint8_t param1, uint8_t param2)
{
	uint8_t t = param2;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_or(uint8_t param1, uint8_t param2)
{
	uint8_t t = param1 | param2;
	SET_CNZ(t);
	return t;
}

int tms7000_device::op_sbb(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 - param2 - (!GET_C());
	SET_NZ(t);
	SET_C(~t);
	return t;
}

int tms7000_device::op_sub(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 - param2;
	SET_NZ(t);
	SET_C(~t);
	return t;
}

int tms7000_device::op_xor(uint8_t param1, uint8_t param2)
{
	uint8_t t = param1 ^ param2;
	SET_CNZ(t);
	return t;
}

// BCD arthrimetic handling
static const uint8_t lut_bcd_out[6] = { 0x00, 0x06, 0x00, 0x66, 0x60, 0x66 };

int tms7000_device::op_dac(uint8_t param1, uint8_t param2)
{
	m_icount -= 2;
	int c = GET_C();

	uint8_t h1 = param1 >> 4 & 0xf;
	uint8_t l1 = param1 >> 0 & 0xf;
	uint8_t h2 = param2 >> 4 & 0xf;
	uint8_t l2 = param2 >> 0 & 0xf;

	// compute bcd constant
	uint8_t d = ((l1 + l2 + c) < 10) ? 0 : 1;
	if ((h1 + h2) == 9)
		d |= 2;
	else if ((h1 + h2) > 9)
		d |= 4;

	uint8_t t = param1 + param2 + c + lut_bcd_out[d];
	SET_CNZ(t);
	if (d > 2)
		m_sr |= SR_C;

	return t;
}

int tms7000_device::op_dsb(uint8_t param1, uint8_t param2)
{
	m_icount -= 2;
	int c = !GET_C();

	uint8_t h1 = param1 >> 4 & 0xf;
	uint8_t l1 = param1 >> 0 & 0xf;
	uint8_t h2 = param2 >> 4 & 0xf;
	uint8_t l2 = param2 >> 0 & 0xf;

	// compute bcd constant
	uint8_t d = ((l1 - c) >= l2) ? 0 : 1;
	if (h1 == h2)
		d |= 2;
	else if (h1 < h2)
		d |= 4;

	uint8_t t = param1 - param2 - c - lut_bcd_out[d];
	SET_CNZ(t);
	if (d <= 2)
		m_sr |= SR_C;

	return t;
}

// branches
void tms7000_device::shortbranch(bool check)
{
	m_icount -= 2;
	int8_t d = (int8_t)imm8();

	if (check)
	{
		m_pc += d;
		m_icount -= 2;
	}
}

void tms7000_device::jmp(bool check)
{
	m_icount -= 3;
	shortbranch(check);
}

int tms7000_device::op_djnz(uint8_t param1, uint8_t param2)
{
	uint16_t t = param1 - 1;
	shortbranch(t != 0);
	return t;
}

int tms7000_device::op_btjo(uint8_t param1, uint8_t param2)
{
	uint8_t t = param1 & param2;
	SET_CNZ(t);
	shortbranch(t != 0);
	return WB_NO;
}

int tms7000_device::op_btjz(uint8_t param1, uint8_t param2)
{
	uint8_t t = ~param1 & param2;
	SET_CNZ(t);
	shortbranch(t != 0);
	return WB_NO;
}



// other opcodes
// dec double
void tms7000_device::decd_a()
{
	m_icount -= 9;
	uint32_t t = read_r16(0) - 1;
	write_r16(0, t);
	SET_NZ(t >> 8);
	SET_C(~(t >> 8));
}

void tms7000_device::decd_b()
{
	m_icount -= 9;
	uint32_t t = read_r16(1) - 1;
	write_r16(1, t);
	SET_NZ(t >> 8);
	SET_C(~(t >> 8));
}

void tms7000_device::decd_r()
{
	m_icount -= 11;
	uint8_t r = imm8();
	uint32_t t = read_r16(r) - 1;
	write_r16(r, t);
	SET_NZ(t >> 8);
	SET_C(~(t >> 8));
}

// cmpa extended
void tms7000_device::cmpa_dir()
{
	m_icount -= 12;
	uint16_t t = read_r8(0) - read_mem8(imm16());
	SET_NZ(t);
	SET_C(~t);
}

void tms7000_device::cmpa_inx()
{
	m_icount -= 14;
	uint16_t t = read_r8(0) - read_mem8(imm16() + read_r8(1));
	SET_NZ(t);
	SET_C(~t);
}

void tms7000_device::cmpa_ind()
{
	m_icount -= 11;
	uint16_t t = read_r8(0) - read_mem8(read_r16(imm8()));
	SET_NZ(t);
	SET_C(~t);
}

// lda extended
void tms7000_device::lda_dir()
{
	m_icount -= 11;
	uint8_t t = read_mem8(imm16());
	write_r8(0, t);
	SET_CNZ(t);
}

void tms7000_device::lda_inx()
{
	m_icount -= 13;
	uint8_t t = read_mem8(imm16() + read_r8(1));
	write_r8(0, t);
	SET_CNZ(t);
}

void tms7000_device::lda_ind()
{
	m_icount -= 10;
	uint8_t t = read_mem8(read_r16(imm8()));
	write_r8(0, t);
	SET_CNZ(t);
}

// sta extended
void tms7000_device::sta_dir()
{
	m_icount -= 11;
	uint8_t t = read_r8(0);
	write_mem8(imm16(), t);
	SET_CNZ(t);
}

void tms7000_device::sta_inx()
{
	m_icount -= 13;
	uint8_t t = read_r8(0);
	write_mem8(imm16() + read_r8(1), t);
	SET_CNZ(t);
}

void tms7000_device::sta_ind()
{
	m_icount -= 10;
	uint8_t t = read_r8(0);
	write_mem8(read_r16(imm8()), t);
	SET_CNZ(t);
}

// mov double
void tms7000_device::movd_dir()
{
	m_icount -= 15;
	uint16_t t = imm16();
	write_r16(imm8(), t);
	SET_CNZ(t >> 8);
}

void tms7000_device::movd_inx()
{
	m_icount -= 17;
	uint16_t t = imm16() + read_r8(1);
	write_r16(imm8(), t);
	SET_CNZ(t >> 8);
}

void tms7000_device::movd_ind()
{
	m_icount -= 14;
	uint16_t t = read_r16(imm8());
	write_r16(imm8(), t);
	SET_CNZ(t >> 8);
}

// long branch
void tms7000_device::br_dir()
{
	m_icount -= 10;
	m_pc = imm16();
}

void tms7000_device::br_inx()
{
	m_icount -= 12;
	m_pc = imm16() + read_r8(1);
}

void tms7000_device::br_ind()
{
	m_icount -= 9;
	m_pc = read_r16(imm8());
}

// call/return
void tms7000_device::call_dir()
{
	m_icount -= 14;
	uint16_t t = imm16();
	push16(m_pc);
	m_pc = t;
}

void tms7000_device::call_inx()
{
	m_icount -= 16;
	uint16_t t = imm16() + read_r8(1);
	push16(m_pc);
	m_pc = t;
}

void tms7000_device::call_ind()
{
	m_icount -= 13;
	uint16_t t = read_r16(imm8());
	push16(m_pc);
	m_pc = t;
}

void tms7000_device::trap(uint8_t address)
{
	m_icount -= 14;
	push16(m_pc);
	m_pc = read_mem16(0xff00 | address);
}

void tms7000_device::reti()
{
	m_icount -= 9;
	m_pc = pull16();
	m_sr = pull8() & 0xf0;
	check_interrupts();
}

void tms7000_device::rets()
{
	m_icount -= 7;
	m_pc = pull16();
}

// pop
void tms7000_device::pop_a()
{
	m_icount -= 6;
	uint8_t t = pull8();
	write_r8(0, t);
	SET_CNZ(t);
}

void tms7000_device::pop_b()
{
	m_icount -= 6;
	uint8_t t = pull8();
	write_r8(1, t);
	SET_CNZ(t);
}

void tms7000_device::pop_r()
{
	m_icount -= 8;
	uint8_t t = pull8();
	write_r8(imm8(), t);
	SET_CNZ(t);
}

void tms7000_device::pop_st()
{
	m_icount -= 6;
	m_sr = pull8() & 0xf0;
	check_interrupts();
}

// push
void tms7000_device::push_a()
{
	m_icount -= 6;
	uint8_t t = read_r8(0);
	push8(t);
	SET_CNZ(t);
}

void tms7000_device::push_b()
{
	m_icount -= 6;
	uint8_t t = read_r8(1);
	push8(t);
	SET_CNZ(t);
}

void tms7000_device::push_r()
{
	m_icount -= 8;
	uint8_t t = read_r8(imm8());
	push8(t);
	SET_CNZ(t);
}

void tms7000_device::push_st()
{
	m_icount -= 6;
	push8(m_sr);
}

// other
void tms7000_device::nop()
{
	m_icount -= 5;
}

void tms7000_device::idle()
{
	m_icount -= 6;
	m_pc--;
	m_idle_state = true;
}

void tms7000_device::dint()
{
	m_icount -= 5;
	m_sr &= ~(SR_N | SR_Z | SR_C | SR_I);
}

void tms7000_device::eint()
{
	m_icount -= 5;
	m_sr |= (SR_N | SR_Z | SR_C | SR_I);
	check_interrupts();
}

void tms7000_device::ldsp()
{
	m_icount -= 5;
	m_sp = read_r8(1);
}

void tms7000_device::stsp()
{
	m_icount -= 6;
	write_r8(1, m_sp);
}

void tms7000_device::setc()
{
	m_icount -= 5;
	m_sr = (m_sr & ~SR_N) | SR_C | SR_Z;
}

// not standard
void tms7020_exl_device::lvdp()
{
	/* on EXL100, opcode D7 ?? (SWAP R) was changed to LVDP, mostly equivalent to:
	* MOVP P40,xx
	* MOVP P36,A
	*/
	m_icount -= 10; // TODO: check real timing
	imm8(); // always 0x28? discarded?
	read_p(0x28);
	uint8_t t = read_p(0x24);
	write_r8(0, t);
	SET_CNZ(t);
}

// illegal opcode handling
void tms7000_device::illegal(uint8_t op)
{
	m_icount -= 5; // guessed
	logerror("%s: illegal opcode $%02X @ $%04x\n", tag(), op, m_pc);
}
