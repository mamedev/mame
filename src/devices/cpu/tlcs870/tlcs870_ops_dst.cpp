// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    (dst) prefix ops (f0 to f7 subtable)

	(dst) address depends on the first byte of the opcode

	F0  (x)
	F1  invalid (would be (PC+A) based on the src table, check for undefined behavior?)
	F2  (DE)
	F3  (HL)
	F4  (HL+d)
	F5  invalid (would be (HL+) based on the src table, check for undefined behavior?)
	F6  (-HL)

	note, in cases where the address is an immediate value, not a register (x) and (HL+d) the
	immediate value is directly after the first byte of the opcode

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"

// Main dispatch handlers for these

void tlcs870_device::do_f0_opcode(const uint8_t opbyte0)
{
	// 1111 0000 xxxx xxxx 0101 0rrr
	// destination memory prefix (dst)
	const uint16_t dstaddr = get_addr((opbyte0 & 0x7), READ8());
	do_f0_to_f7_opcode(opbyte0, dstaddr);
}

void tlcs870_device::do_f2_to_f3_opcode(const uint8_t opbyte0)
{
	//	0xf2: 1111 001p 0101 0rrr
	//	0xf3: 1111 001p 0101 0rrr
	// destination memory prefix (dst)
	const uint16_t dstaddr = get_addr((opbyte0 & 0x7), 0);
	do_f0_to_f7_opcode(opbyte0, dstaddr);
}


void tlcs870_device::do_f4_opcode(const uint8_t opbyte0)
{
	//	0xf4: 1111 0100 dddd dddd 0101 0rrr
	// destination memory prefix (dst)
	const uint16_t dstaddr = get_addr((opbyte0 & 0x7), READ8());
	do_f0_to_f7_opcode(opbyte0, dstaddr);
}

void tlcs870_device::do_f6_to_f7_opcode(const uint8_t opbyte0)
{
	//	0xf6: 1110 0110 0101 0rrr
	//	0xf7: 1111 0111 0101 0rrr
	// destination memory prefix (dst)
	const uint16_t dstaddr = get_addr((opbyte0 & 0x7), 0);
	do_f0_to_f7_opcode(opbyte0, dstaddr);
}

// f0 - f7 use this table
// note, same table is shown as above in manual, there's no overlap between src/dest, but they're not compatible
void tlcs870_device::do_f0_to_f7_opcode(const uint8_t opbyte0, const uint16_t dstaddr)
{
	const uint8_t opbyte1 = READ8();

	switch (opbyte1)
	{
	case 0x10: case 0x11: case 0x12: case 0x13:
		do_LD_indst_rr(opbyte0, opbyte1, dstaddr); break;
	case 0x2c:
		do_LD_indst_n(opbyte0, opbyte1, dstaddr); break;
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		do_LD_indst_r(opbyte0, opbyte1, dstaddr); break;
	default:
		do_f0_to_f7_oprand_illegal_opcode(opbyte0, opbyte1, dstaddr); break;
	}
}

// Actual handlers

/**********************************************************************************************************************/
// (16-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_indst_rr(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	// LD (dst),rr
	// (dst) can only be  (x) (pp) or (HL+d) ?  not (HL+) or (-HL) ?
	const uint16_t val = get_reg16(opbyte1 & 0x3);
	WM16(dstaddr, val);

	set_JF();
}

/**********************************************************************************************************************/
// (8-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_indst_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	// LD (dst),n
	// (dst) can only be (DE), (HL+), (-HL), or (HL+d)  because (x) and (HL) are redundant encodings?
	const uint16_t n = READ8();
	WM8(dstaddr, n);

	set_JF();
}

void tlcs870_device::do_LD_indst_r(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	// LD (dst),r
	const uint8_t reg = get_reg8(opbyte1 & 0x7);
	WM8(dstaddr, reg);

	set_JF();
}

void tlcs870_device::do_f0_to_f7_oprand_illegal_opcode(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	logerror("illegal dst prefix opcode %02x %02x (dst addr %04x)\n", opbyte0, opbyte1, dstaddr);
}
