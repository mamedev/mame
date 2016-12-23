// license:BSD-3-Clause
// copyright-holders:hap

// E0C6200 opcode handlers

#include "e0c6200.h"


// internal data memory read/write

// MX/MY

uint8_t e0c6200_cpu_device::read_mx()
{
	uint16_t address = m_xp << 8 | m_xh << 4 | m_xl;
	return m_data->read_byte(address) & 0xf;
}

uint8_t e0c6200_cpu_device::read_my()
{
	uint16_t address = m_yp << 8 | m_yh << 4 | m_yl;
	return m_data->read_byte(address) & 0xf;
}

void e0c6200_cpu_device::write_mx(uint8_t data)
{
	uint16_t address = m_xp << 8 | m_xh << 4 | m_xl;
	m_data->write_byte(address, data);
}

void e0c6200_cpu_device::write_my(uint8_t data)
{
	uint16_t address = m_yp << 8 | m_yh << 4 | m_yl;
	m_data->write_byte(address, data);
}

// Mn(RP)

uint8_t e0c6200_cpu_device::read_mn()
{
	return m_data->read_byte(m_op & 0xf) & 0xf;
}

void e0c6200_cpu_device::write_mn(uint8_t data)
{
	m_data->write_byte(m_op & 0xf, data);
}


// common stack ops

void e0c6200_cpu_device::push(uint8_t data)
{
	m_data->write_byte(--m_sp, data);
}

uint8_t e0c6200_cpu_device::pop()
{
	return m_data->read_byte(m_sp++) & 0xf;
}

void e0c6200_cpu_device::push_pc()
{
	// the highest bit(bank bit) is not pushed onto the stack
	push(m_pc >> 8 & 0xf);
	push(m_pc >> 4 & 0xf);
	push(m_pc & 0xf);
}

void e0c6200_cpu_device::pop_pc()
{
	// the highest bit(bank bit) is unchanged
	uint16_t bank = m_pc & 0x1000;
	m_pc = pop();
	m_pc |= pop() << 4;
	m_pc |= pop() << 8;
	m_pc |= bank;
}


// misc internal helpers

void e0c6200_cpu_device::set_cf(uint8_t data)
{
	// set carry flag if bit 4 is set, reset otherwise
	m_f = (m_f & ~C_FLAG) | ((data & 0x10) ? C_FLAG : 0);
}

void e0c6200_cpu_device::set_zf(uint8_t data)
{
	// set zero flag if 4-bit data is 0, reset otherwise
	m_f = (m_f & ~Z_FLAG) | ((data & 0xf) ? 0 : Z_FLAG);
}

void e0c6200_cpu_device::inc_x()
{
	// increment X (XH.XL)
	m_xl++;
	m_xh = (m_xh + (m_xl >> 4 & 1)) & 0xf;
	m_xl &= 0xf;
}

void e0c6200_cpu_device::inc_y()
{
	// increment Y (YH.YL)
	m_yl++;
	m_yh = (m_yh + (m_yl >> 4 & 1)) & 0xf;
	m_yl &= 0xf;
}

void e0c6200_cpu_device::do_branch(int condition)
{
	// branch on condition
	if (condition)
		m_pc = m_jpc | (m_op & 0xff);
}


// common opcodes (simpler ones are handled directly)
// note: it is implied that all opcodes below except RRC take 7 clock cycles (5 already deducted)

// arithmetic instructions

uint8_t e0c6200_cpu_device::op_inc(uint8_t x)
{
	// INC x: increment x (flags: C, Z)
	m_icount -= 2;
	x++;
	set_cf(x); set_zf(x);
	return x & 0xf;
}

uint8_t e0c6200_cpu_device::op_dec(uint8_t x)
{
	// DEC x: decrement x (flags: C, Z)
	m_icount -= 2;
	x--;
	set_cf(x); set_zf(x);
	return x & 0xf;
}

uint8_t e0c6200_cpu_device::op_add(uint8_t x, uint8_t y, int decimal)
{
	// ADD x,y: add y to x (flags: C, Z)
	m_icount -= 2;
	x += y;
	set_cf(x);

	// decimal correction
	if (m_f & decimal && x >= 10)
	{
		x -= 10;
		m_f |= C_FLAG;
	}

	set_zf(x);
	return x & 0xf;
}

uint8_t e0c6200_cpu_device::op_adc(uint8_t x, uint8_t y, int decimal)
{
	// ADC x,y: add with carry y to x (flags: C, Z)
	return op_add(x, y + (m_f & 1), decimal);
}

uint8_t e0c6200_cpu_device::op_sub(uint8_t x, uint8_t y, int decimal)
{
	// SUB x,y: subtract y from x (flags: C, Z)
	m_icount -= 2;
	x -= y;
	set_cf(x);

	// decimal correction (carry remains same)
	if (m_f & decimal && m_f & C_FLAG)
		x -= 6;

	set_zf(x);
	return x & 0xf;
}

uint8_t e0c6200_cpu_device::op_sbc(uint8_t x, uint8_t y, int decimal)
{
	// SBC x,y: subtract with carry y from x (flags: C, Z)
	return op_sub(x, y + (m_f & 1), decimal);
}


// logical instructions

uint8_t e0c6200_cpu_device::op_and(uint8_t x, uint8_t y)
{
	// AND x,y: logical AND x with y (flags: Z)
	m_icount -= 2;
	x &= y;
	set_zf(x);
	return x;
}

uint8_t e0c6200_cpu_device::op_or(uint8_t x, uint8_t y)
{
	// OR x,y: logical OR x with y (flags: Z)
	m_icount -= 2;
	x |= y;
	set_zf(x);
	return x;
}

uint8_t e0c6200_cpu_device::op_xor(uint8_t x, uint8_t y)
{
	// XOR x,y: exclusive-OR x with y (flags: Z)
	m_icount -= 2;
	x ^= y;
	set_zf(x);
	return x;
}

uint8_t e0c6200_cpu_device::op_rlc(uint8_t x)
{
	// RLC x: rotate x left through carry (flags: C, Z)
	m_icount -= 2;
	x = (x << 1) | (m_f & 1);
	set_cf(x); set_zf(x);
	return x & 0xf;
}

uint8_t e0c6200_cpu_device::op_rrc(uint8_t x)
{
	// RRC x: rotate x right through carry (flags: C, Z)
	// note: RRC only takes 5 clock cycles
	int c = x & 1;
	x = (x >> 1) | (m_f << 3 & 8);
	m_f = (m_f & ~C_FLAG) | c;
	set_zf(x);
	return x & 0xf;
}
