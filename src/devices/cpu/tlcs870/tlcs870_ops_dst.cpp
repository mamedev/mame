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
    F5  invalid (would be (HL+C) based on the src table, check for undefined behavior?)
    F6  (HL+)
    F7  (-HL)

    note, in cases where the address is an immediate value, not a register (x) and (HL+d) the
    immediate value is directly after the first byte of the opcode

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"

// Main dispatch handlers for these

// f0 - f7 use this table
// note, the manual shows both src/dst opcodes in the same table as there's no overlap, but they're not compatible
void tlcs870_device::do_dstprefixtype_opcode(const uint8_t opbyte0)
{
	m_cycles = get_base_srcdst_cycles(opbyte0 & 0x7); // set base number of cycles based on dst prefix mode

	uint16_t dstaddr;
	// (x) and (HL+d) require an immediate value
	if ((opbyte0 == 0xf0) || (opbyte0 == 0xf4))
	{
		dstaddr = get_addr((opbyte0 & 0x7), READ8());
	}
	else
	{
		dstaddr = get_addr((opbyte0 & 0x7), 0);
	}

	const uint8_t opbyte1 = READ8();

	// these are illegal prefixes for dst, undefined behavior at least
	if ((opbyte0 == 0xf1) || (opbyte0 == 0xf5))
	{
		do_f0_to_f7_oprand_illegal_opcode(opbyte0, opbyte1, dstaddr);
	}

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

void tlcs870_device::do_f0_to_f7_oprand_illegal_opcode(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	m_cycles += 1;
	logerror("illegal dst prefix opcode %02x %02x (dst addr %04x)\n", opbyte0, opbyte1, dstaddr);
}

// Actual handlers

/**********************************************************************************************************************/
// (16-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_indst_rr(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (x), rr        1111 0000 xxxx xxxx 0001 00rr                        1  -  -  -    5
	    LD (PC+A), rr     invalid encoding (all PC+A are invalid for dst)      ?  ?  ?  ?    ?
	    LD (DE), rr       1111 0010           0001 00rr                        1  -  -  -    4
	    LD (HL), rr       1111 0011           0001 00rr                        1  -  -  -    4
	    LD (HL+d), rr     1111 0100 dddd dddd 0001 00rr                        1  -  -  -    6
	    LD (HL+C), rr     invalid encoding (all HL+C are invalid for dst)      ?  ?  ?  ?    ?
	    LD (HL+), rr      not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?
	    LD (-HL), rr      not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?

	    aka LD (dst),rr
	    (dst) can only be  (x) (pp) or (HL+d) ?  not (HL+) or (-HL) ?
	*/
	m_cycles += 4;

	const uint16_t val = get_reg16(opbyte1 & 0x3);
	WM16(dstaddr, val);

	set_JF();
}

/**********************************************************************************************************************/
// (8-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_indst_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (x), n         not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    LD (PC+A), n      invalid encoding (all PC+A are invalid for dst)      ?  ?  ?  ?    ?
	    LD (DE), n        1111 0010           0010 1100 nnnn nnnn              1  -  -  -    4
	    LD (HL), n        not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    LD (HL+d), n      1111 0100 dddd dddd 0010 1100 nnnn nnnn              1  -  -  -    6
	    LD (HL+C), n      invalid encoding (all HL+C are invalid for dst)      ?  ?  ?  ?    ?
	    LD (HL+), n       1111 0110           0010 1100 nnnn nnnn              1  -  -  -    5
	    LD (-HL), n       1111 0111           0010 1100 nnnn nnnn              1  -  -  -    5

	    aka (dst),n
	    (dst) can only be (DE), (HL+), (-HL), or (HL+d)  because (x) and (HL) are redundant encodings?
	*/
	m_cycles += 4;

	const uint16_t n = READ8();
	WM8(dstaddr, n);

	set_JF();
}

void tlcs870_device::do_LD_indst_r(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t dstaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (x), r         1111 0000 xxxx xxxx 0101 1rrr                        1  -  -  -    4
	    LD (PC+A), r      invalid encoding (all PC+A are invalid for dst)      ?  ?  ?  ?    ?
	    LD (DE), r        1111 0010           0101 1rrr                        1  -  -  -    3
	    LD (HL), r        1111 0011           0101 1rrr                        1  -  -  -    3
	    LD (HL+d), r      1111 0100 dddd dddd 0101 1rrr                        1  -  -  -    5
	    LD (HL+C), r      invalid encoding (all HL+C are invalid for dst)      ?  ?  ?  ?    ?
	    LD (HL+), r       1111 0110           0101 0rrr                        1  -  -  -    4  (invalid if r is H or L)
	    LD (-HL), r       1111 0111           0101 0rrr                        1  -  -  -    4

	    aka LD (dst),r
	*/
	m_cycles += 3;

	const uint8_t reg = get_reg8(opbyte1 & 0x7);
	WM8(dstaddr, reg);

	set_JF();
}
