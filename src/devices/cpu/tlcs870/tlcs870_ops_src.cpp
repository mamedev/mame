// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    (src) prefix ops (e0 to e7 subtable)

    (src) address depends on the first byte of the opcode

    E0  (x)
    E1  (PC+A)
    E2  (DE)
    E3  (HL)
    E4  (HL+d)
    E5  (HL+C)
    E6  (HL+)
    E7  (-HL)

    note, in cases where the address is an immediate value, not a register (x) and (HL+d) the
    immediate value is directly after the first byte of the opcode

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"

// Main dispatch handlers for these

// e0 - e7 use this table
void tlcs870_device::do_srcprefixtype_opcode(const uint8_t opbyte0)
{
	m_cycles = get_base_srcdst_cycles(opbyte0 & 0x7); // set base number of cycles based on src prefix mode

	uint16_t srcaddr;
	// (x) and (HL+d) require an immediate value
	if ((opbyte0 == 0xe0) || (opbyte0 == 0xe4))
		srcaddr = get_addr((opbyte0 & 0x7), READ8());
	else
		srcaddr = get_addr((opbyte0 & 0x7), 0);

	const uint8_t opbyte1 = READ8();

	switch (opbyte1)
	{
	case 0x08:
		do_ROLD_A_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x09:
		do_RORD_A_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x14: case 0x15: case 0x16: case 0x17:
		do_LD_rr_insrc(opbyte0, opbyte1, srcaddr);  break;
	case 0x20:
		do_INC_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x26:
		do_LD_inx_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x27:
		do_LD_inHL_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x28:
		do_DEC_insrc(opbyte0, opbyte1, srcaddr); break;
	case 0x2f:
		do_MCMP_insrc_n(opbyte0, opbyte1, srcaddr); break;
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		do_SET_insrcbit(opbyte0, opbyte1, srcaddr);  break;
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		do_CLR_insrcbit(opbyte0, opbyte1, srcaddr);  break;
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		do_LD_r_insrc(opbyte0, opbyte1, srcaddr);  break;
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		do_ALUOP_insrc_inHL(opbyte0, opbyte1, srcaddr); break;
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		do_ALUOP_insrc_n(opbyte0, opbyte1, srcaddr);  break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		do_ALUOP_A_insrc(opbyte0, opbyte1, srcaddr);  break;
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		do_XCH_r_insrc(opbyte0, opbyte1, srcaddr);  break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		do_CPL_insrcbit(opbyte0, opbyte1, srcaddr);  break;
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		do_LD_insrcbit_CF(opbyte0, opbyte1, srcaddr);  break;
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		do_XOR_CF_insrcbit(opbyte0, opbyte1, srcaddr);  break;
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		do_LD_CF_insrcbit(opbyte0, opbyte1, srcaddr);  break;
	case 0xfc:
		do_CALL_insrc(opbyte0, opbyte1, srcaddr);  break;
	case 0xfe:
		do_JP_insrc(opbyte0, opbyte1, srcaddr); break;

	default:
		do_e0_to_e7_oprand_illegal(opbyte0, opbyte1, srcaddr); break;
	}
}

// Actual handlers

void tlcs870_device::do_e0_to_e7_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	m_cycles += 1;
	logerror("illegal src prefix opcode %02x %02x (src addr %04x)\n", opbyte0, opbyte1, srcaddr);
}

/**********************************************************************************************************************/
// (16-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_rr_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD rr, (x)        1110 0000 xxxx xxxx 0001 01rr                        1  -  -  -    5
	    LD rr, (PC+A)     1110 0001           0001 01rr                        1  -  -  -    6
	    LD rr, (DE)       1110 0010           0001 01rr                        1  -  -  -    4
	    LD rr, (HL)       1110 0011           0001 01rr                        1  -  -  -    4
	    LD rr, (HL+d)     1110 0100 dddd dddd 0001 01rr                        1  -  -  -    6
	    LD rr, (HL+C)     1110 0101           0001 01rr                        1  -  -  -    6
	    LD rr, (HL+)      not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?
	    LD rr, (-HL)      not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?

	    aka LD rr, (src)
	*/
	m_cycles += 4;

	const uint16_t val = RM16(srcaddr);
	set_reg16(opbyte1 & 0x3, val);
	set_JF();
}

/**********************************************************************************************************************/
// (8-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_INC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    INC (x)           not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    INC (PC+A)        1110 0001           0010 0000                        C  Z  -  -    6
	    INC (DE)          1110 0010           0010 0000                        C  Z  -  -    4
	    INC (HL)          not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    INC (HL+d)        1110 0100 dddd dddd 0010 0000                        C  Z  -  -    6
	    INC (HL+C)        1110 0101           0010 0000                        C  Z  -  -    6
	    INC (HL+)         1110 0110           0010 0000                        C  Z  -  -    5
	    INC (-HL)         1110 0111           0010 0000                        C  Z  -  -    5

	    aka INC (src)
	*/
	m_cycles += 4;

	uint8_t val = RM8(srcaddr);
	val++;

	if (val == 0)
	{
		set_ZF();
		set_JF();
	}
	else
	{
		clear_ZF();
		clear_JF();
	}

	WM8(srcaddr, val);
}

void tlcs870_device::do_DEC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    DEC (x)           not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    DEC (PC+A)        1110 0001           0010 1000                        C  Z  -  -    6
	    DEC (DE)          1110 0010           0010 1000                        C  Z  -  -    4
	    DEC (HL)          not listed, redundant encoding?                      ?  ?  ?  ?    ?
	    DEC (HL+d)        1110 0100 dddd dddd 0010 1000                        C  Z  -  -    6
	    DEC (HL+C)        1110 0101           0010 1000                        C  Z  -  -    6
	    DEC (HL+)         1110 0110           0010 1000                        C  Z  -  -    5
	    DEC (-HL)         1110 0111           0010 1000                        C  Z  -  -    5

	    aka  DEC (src)
	*/
	m_cycles += 4;

	uint8_t val = RM8(srcaddr);
	val--;

	if (val == 0xff)
	{
		set_JF();
	}
	else
	{
		// do we clear?
		clear_JF();
	}

	if (val == 0x00) // check
	{
		set_ZF();
	}
	else
	{
		clear_ZF();
	}

	WM8(srcaddr, val);
}

void tlcs870_device::do_ROLD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ROLD A, (x)       1110 0000 xxxx xxxx 0000 1000                        1  0  0  0    8
	    ROLD A, (PC+A)    not listed, invalid?                                 ?  ?  ?  ?    ?
	    ROLD A, (DE)      1110 0010           0000 1000                        1  0  0  0    7
	    ROLD A, (HL)      1110 0011           0000 1000                        1  0  0  0    7
	    ROLD A, (HL+d)    1110 0100 dddd dddd 0000 1000                        1  0  0  0    9
	    ROLD A, (HL+C)    1110 0101           0000 1000                        1  0  0  0    9
	    ROLD A, (HL+)     1110 0110           0000 1000                        1  0  0  0    8
	    ROLD A, (-HL)     1110 0111           0000 1000                        1  0  0  0    8

	    aka ROLD A,(src)
	    12-bit left rotation using lower 4 bits of REG_A and content of (src)
	*/
	m_cycles += 7;

	const uint8_t val = RM8(srcaddr);
	const uint8_t reg = get_reg8(REG_A);

	uint8_t tempval = (val & 0x0f) << 4;
	tempval |= reg & 0x0f;
	const uint8_t tempa = (reg & 0xf0) | (val & 0xf0) >> 4;

	set_reg8(REG_A, tempa);
	WM8(srcaddr, tempval);

	set_JF();
}

void tlcs870_device::do_RORD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    RORD A, (x)       1110 0000 xxxx xxxx 0000 1001                        1  0  0  0    8
	    RORD A, (PC+A)    not listed, invalid?                                 ?  ?  ?  ?    ?
	    RORD A, (DE)      1110 0010           0000 1001                        1  0  0  0    7
	    RORD A, (HL)      1110 0011           0000 1001                        1  0  0  0    7
	    RORD A, (HL+d)    1110 0100 dddd dddd 0000 1001                        1  0  0  0    9
	    RORD A, (HL+C)    1110 0101           0000 1001                        1  0  0  0    9
	    RORD A, (HL+)     1110 0110           0000 1001                        1  0  0  0    8
	    RORD A, (-HL)     1110 0111           0000 1001                        1  0  0  0    8

	    aka RORD A,(src)
	    12-bit right rotation using lower 4 bits of REG_A and content of (src)
	*/
	m_cycles += 7;

	const uint8_t val = RM8(srcaddr);
	const uint8_t reg = get_reg8(REG_A);

	uint8_t tempval = (val & 0xf0) >> 4;
	tempval |= ((reg & 0x0f) << 4);
	const uint8_t tempa = (reg & 0xf0) | (val & 0x0f);

	set_reg8(REG_A, tempa);
	WM8(srcaddr, tempval);

	set_JF();
}



void tlcs870_device::do_LD_inx_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (x), (x)       not listed, invalid? or redundant?                   ?  ?  ?  ?    ?
	    LD (x), (PC+A)    1110 0001           0010 0110 xxxx xxxx              1  U  -  -    7
	    LD (x), (DE)      1110 0010           0010 0110 xxxx xxxx              1  U  -  -    5
	    LD (x), (HL)      1110 0011           0010 0110 xxxx xxxx              1  U  -  -    5
	    LD (x), (HL+d)    1110 0100 dddd dddd 0010 0110 xxxx xxxx              1  U  -  -    7
	    LD (x), (HL+C)    1110 0101           0010 0110 xxxx xxxx              1  U  -  -    7
	    LD (x), (HL+)     not listed, invalid?                                 ?  ?  ?  ?    ?
	    LD (x), (-HL)     not listed, invalid?                                 ?  ?  ?  ?    ?

	    aka LD (x),(src)
	*/
	m_cycles += 5;

	const uint16_t x = READ8(); // get address x
	const uint8_t val = RM8(srcaddr);
	WM8(x, val);

	set_JF();
	// z-flag is undefined, check real behavior
}

void tlcs870_device::do_LD_inHL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (HL), (x)      1110 0000 xxxx xxxx 0010 0111                        1  Z  -  -    5
	    LD (HL), (PC+A)   1110 0001           0010 0111                        1  Z  -  -    6
	    LD (HL), (DE)     1110 0010           0010 0111                        1  Z  -  -    4
	    LD (HL), (HL)     1110 0011           0010 0111                        1  Z  -  -    4
	    LD (HL), (HL+d)   1110 0100 dddd dddd 0010 0111                        1  Z  -  -    6
	    LD (HL), (HL+C)   1110 0101           0010 0111                        1  Z  -  -    6
	    LD (HL), (HL+)    not listed, invalid?                                 ?  ?  ?  ?    ?
	    LD (HL), (-HL)    not listed, invalid?                                 ?  ?  ?  ?    ?

	    aka LD (HL),(src)
	*/
	m_cycles += 4;

	const uint8_t val = RM8(srcaddr);
	const uint16_t dstaddr = get_reg16(REG_HL);
	WM8(dstaddr, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_LD_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD r, (x)         1110 0000 xxxx xxxx 0101 1rrr                        1  Z  -  -    4
	    LD r, (PC+A)      1110 0001           0101 1rrr                        1  Z  -  -    5
	    LD r, (DE)        1110 0010           0101 1rrr                        1  Z  -  -    3
	    LD r, (HL)        1110 0011           0101 1rrr                        1  Z  -  -    3
	    LD r, (HL+d)      1110 0100 dddd dddd 0101 1rrr                        1  Z  -  -    5
	    LD r, (HL+C)      1110 0101           0101 1rrr                        1  Z  -  -    5
	    LD r, (HL+)       1110 0110           0101 1rrr                        1  Z  -  -    4       (invalid if r is H or L)
	    LD r, (-HL)       1110 0111           0101 1rrr                        1  Z  -  -    4

	    aka LD r, (src)
	*/
	m_cycles += 3;

	const uint8_t val = RM8(srcaddr);

	set_reg8(opbyte1 & 0x7, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_MCMP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    MCMP (x), n       1110 0000 xxxx xxxx 0010 1111 nnnn nnnn              Z  Z  C  H    6
	    MCMP (PC+A), n    1110 0001           0010 1111 nnnn nnnn              Z  Z  C  H    7
	    MCMP (DE), n      1110 0010           0010 1111 nnnn nnnn              Z  Z  C  H    5
	    MCMP (HL), n      1110 0011           0010 1111 nnnn nnnn              Z  Z  C  H    5
	    MCMP (HL+d), n    1110 0100 dddd dddd 0010 1111 nnnn nnnn              Z  Z  C  H    7
	    MCMP (HL+C), n    1110 0101           0010 1111 nnnn nnnn              Z  Z  C  H    7
	    MCMP (HL+), n     1110 0110           0010 1111 nnnn nnnn              Z  Z  C  H    6
	    MCMP (-HL), n     1110 0111           0010 1111 nnnn nnnn              Z  Z  C  H    6

	    aka MCMP (src), n
	*/
	m_cycles += 5;

	const uint8_t n = READ8();
	const uint8_t val = RM8(srcaddr);

	const uint8_t temp = n & val;
	const uint8_t a = get_reg8(REG_A);

	// ZF and JF set conditionally
	if (a == temp)
	{
		// if n & val is equal to accumulator
		set_ZF();
		set_JF();
	}
	else
	{
		clear_ZF();
		clear_JF();
	}

	// C gets set conditionally (like CMP?)
	if (a < temp)
	{
		set_CF();
	}
	else
	{
		clear_CF();
	}

	// apparently H gets set conditionally too? (like CMP?)
}

void tlcs870_device::do_XCH_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    XCH r, (x)        1110 0000 xxxx xxxx 1010 1rrr                        1  Z  -  -    5
	    XCH r, (PC+A)     1110 0001           1010 1rrr                        1  Z  -  -    6
	    XCH r, (DE)       1110 0010           1010 1rrr                        1  Z  -  -    4
	    XCH r, (HL)       1110 0011           1010 1rrr                        1  Z  -  -    4
	    XCH r, (HL+d)     1110 0100 dddd dddd 1010 1rrr                        1  Z  -  -    6
	    XCH r, (HL+C)     1110 0101           1010 1rrr                        1  Z  -  -    6
	    XCH r, (HL+)      1110 0110           1010 1rrr                        1  Z  -  -    5       (invalid if r is H or L)
	    XCH r, (-HL)      1110 0111           1010 1rrr                        1  Z  -  -    5

	    aka XCH r,(src)
	*/
	m_cycles += 4;

	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	const uint8_t val = RM8(srcaddr);
	const uint8_t temp = get_reg8(opbyte1 & 0x7);

	WM8(srcaddr, temp);
	set_reg8(opbyte1 & 0x7, val);

	set_JF();

	if (val == 0)
	{
		set_ZF();
	}
	else
	{
		// do we clear?
		clear_ZF();
	}
}

/**********************************************************************************************************************/
// ALU Operations
/**********************************************************************************************************************/

void tlcs870_device::do_ALUOP_insrc_inHL(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC (x), (HL)    1110 0000 xxxx xxxx 0110 0000                        C  Z  C  H    7
	    ADDC (PC+A), (HL) 1110 0001           0110 0000                        C  Z  C  H    8
	    ADDC (DE), (HL)   1110 0010           0110 0000                        C  Z  C  H    6
	    ADDC (HL), (HL)   1110 0011           0110 0000                        C  Z  C  H    6
	    ADDC (HL+d), (HL) 1110 0100 dddd dddd 0110 0000                        C  Z  C  H    8
	    ADDC (HL+C), (HL) 1110 0101           0110 0000                        C  Z  C  H    8
	    ADDC (HL+), (HL)  not listed, invalid?                                 ?  ?  ?  ?    ?
	    ADDC (-HL), (HL)  not listed, invalid?                                 ?  ?  ?  ?    ?

	    ADD (x), (HL)     1110 0000 xxxx xxxx 0110 0001                        C  Z  C  H    7
	    ADD (PC+A), (HL)  1110 0001           0110 0001                        C  Z  C  H    8
	    ADD (DE), (HL)    1110 0010           0110 0001                        C  Z  C  H    6
	    ADD (HL), (HL)    1110 0011           0110 0001                        C  Z  C  H    6
	    ADD (HL+d), (HL)  1110 0100 dddd dddd 0110 0001                        C  Z  C  H    8
	    ADD (HL+C), (HL)  1110 0101           0110 0001                        C  Z  C  H    8
	    ADD (HL+), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?
	    ADD (-HL), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?

	    SUBB (x), (HL)    1110 0000 xxxx xxxx 0110 0010                        C  Z  C  H    7
	    SUBB (PC+A), (HL) 1110 0001           0110 0010                        C  Z  C  H    8
	    SUBB (DE), (HL)   1110 0010           0110 0010                        C  Z  C  H    6
	    SUBB (HL), (HL)   1110 0011           0110 0010                        C  Z  C  H    6
	    SUBB (HL+d), (HL) 1110 0100 dddd dddd 0110 0010                        C  Z  C  H    8
	    SUBB (HL+C), (HL) 1110 0101           0110 0010                        C  Z  C  H    8
	    SUBB (HL+), (HL)  not listed, invalid?                                 ?  ?  ?  ?    ?
	    SUBB (-HL), (HL)  not listed, invalid?                                 ?  ?  ?  ?    ?

	    SUB (x), (HL)     1110 0000 xxxx xxxx 0110 0011                        C  Z  C  H    7
	    SUB (PC+A), (HL)  1110 0001           0110 0011                        C  Z  C  H    8
	    SUB (DE), (HL)    1110 0010           0110 0011                        C  Z  C  H    6
	    SUB (HL), (HL)    1110 0011           0110 0011                        C  Z  C  H    6
	    SUB (HL+d), (HL)  1110 0100 dddd dddd 0110 0011                        C  Z  C  H    8
	    SUB (HL+C), (HL)  1110 0101           0110 0011                        C  Z  C  H    8
	    SUB (HL+), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?
	    SUB (-HL), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?

	    AND (x), (HL)     1110 0000 xxxx xxxx 0110 0100                        Z  Z  -  -    7
	    AND (PC+A), (HL)  1110 0001           0110 0100                        Z  Z  -  -    8
	    AND (DE), (HL)    1110 0010           0110 0100                        Z  Z  -  -    6
	    AND (HL), (HL)    1110 0011           0110 0100                        Z  Z  -  -    6
	    AND (HL+d), (HL)  1110 0100 dddd dddd 0110 0100                        Z  Z  -  -    8
	    AND (HL+C), (HL)  1110 0101           0110 0100                        Z  Z  -  -    8
	    AND (HL+), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?
	    AND (-HL), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?

	    XOR (x), (HL)     1110 0000 xxxx xxxx 0110 0101                        Z  Z  -  -    7
	    XOR (PC+A), (HL)  1110 0001           0110 0101                        Z  Z  -  -    8
	    XOR (DE), (HL)    1110 0010           0110 0101                        Z  Z  -  -    6
	    XOR (HL), (HL)    1110 0011           0110 0101                        Z  Z  -  -    6
	    XOR (HL+d), (HL)  1110 0100 dddd dddd 0110 0101                        Z  Z  -  -    8
	    XOR (HL+C), (HL)  1110 0101           0110 0101                        Z  Z  -  -    8
	    XOR (HL+), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?
	    XOR (-HL), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?

	    OR (x), (HL)      1110 0000 xxxx xxxx 0110 0110                        Z  Z  -  -    7
	    OR (PC+A), (HL)   1110 0001           0110 0110                        Z  Z  -  -    8
	    OR (DE), (HL)     1110 0010           0110 0110                        Z  Z  -  -    6
	    OR (HL), (HL)     1110 0011           0110 0110                        Z  Z  -  -    6
	    OR (HL+d), (HL)   1110 0100 dddd dddd 0110 0110                        Z  Z  -  -    8
	    OR (HL+C), (HL)   1110 0101           0110 0110                        Z  Z  -  -    8
	    OR (HL+), (HL)    not listed, invalid?                                 ?  ?  ?  ?    ?
	    OR (-HL), (HL)    not listed, invalid?                                 ?  ?  ?  ?    ?

	    CMP (x), (HL)     1110 0000 xxxx xxxx 0110 0111                        Z  Z  C  H    6
	    CMP (PC+A), (HL)  1110 0001           0110 0111                        Z  Z  C  H    7
	    CMP (DE), (HL)    1110 0010           0110 0111                        Z  Z  C  H    5
	    CMP (HL), (HL)    1110 0011           0110 0111                        Z  Z  C  H    5
	    CMP (HL+d), (HL)  1110 0100 dddd dddd 0110 0111                        Z  Z  C  H    7
	    CMP (HL+C), (HL)  1110 0101           0110 0111                        Z  Z  C  H    7
	    CMP (HL+), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?
	    CMP (-HL), (HL)   not listed, invalid?                                 ?  ?  ?  ?    ?

	    aka (ALU OP) (src), (HL)
	*/
	m_cycles += 6;

	const int aluop = (opbyte1 & 0x7);
	if (aluop != 0x07)
		m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	const uint8_t val = RM8(srcaddr);
	m_read_input_port = 1;

	const uint16_t HL = get_reg16(REG_HL);

	const uint8_t result = do_alu_8bit(aluop, val, RM8(HL));

	if (aluop != 0x07) // CMP doesn't write back
	{
		m_cycles -= 1; // one less for CMP here?
		WM8(srcaddr, result);
	}
}

void tlcs870_device::do_ALUOP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC (x), n       1110 0000 xxxx xxxx 0111 0000 nnnn nnnn              C  Z  C  H    6
	    ADDC (PC+A), n    1110 0001           0111 0000 nnnn nnnn              C  Z  C  H    7
	    ADDC (DE), n      1110 0010           0111 0000 nnnn nnnn              C  Z  C  H    5
	    ADDC (HL), n      1110 0011           0111 0000 nnnn nnnn              C  Z  C  H    5
	    ADDC (HL+d), n    1110 0100 dddd dddd 0111 0000 nnnn nnnn              C  Z  C  H    7
	    ADDC (HL+C), n    1110 0101           0111 0000 nnnn nnnn              C  Z  C  H    7
	    ADDC (HL+), n     1110 0110           0111 0000 nnnn nnnn              C  Z  C  H    6
	    ADDC (-HL), n     1110 0111           0111 0000 nnnn nnnn              C  Z  C  H    6

	    ADD (x), n        1110 0000 xxxx xxxx 0111 0001 nnnn nnnn              C  Z  C  H    6
	    ADD (PC+A), n     1110 0001           0111 0001 nnnn nnnn              C  Z  C  H    7
	    ADD (DE), n       1110 0010           0111 0001 nnnn nnnn              C  Z  C  H    5
	    ADD (HL), n       1110 0011           0111 0001 nnnn nnnn              C  Z  C  H    5
	    ADD (HL+d), n     1110 0100 dddd dddd 0111 0001 nnnn nnnn              C  Z  C  H    7
	    ADD (HL+C), n     1110 0101           0111 0001 nnnn nnnn              C  Z  C  H    7
	    ADD (HL+), n      1110 0110           0111 0001 nnnn nnnn              C  Z  C  H    6
	    ADD (-HL), n      1110 0111           0111 0001 nnnn nnnn              C  Z  C  H    6

	    SUBB (x), n       1110 0000 xxxx xxxx 0111 0010 nnnn nnnn              C  Z  C  H    6
	    SUBB (PC+A), n    1110 0001           0111 0010 nnnn nnnn              C  Z  C  H    7
	    SUBB (DE), n      1110 0010           0111 0010 nnnn nnnn              C  Z  C  H    5
	    SUBB (HL), n      1110 0011           0111 0010 nnnn nnnn              C  Z  C  H    5
	    SUBB (HL+d), n    1110 0100 dddd dddd 0111 0010 nnnn nnnn              C  Z  C  H    7
	    SUBB (HL+C), n    1110 0101           0111 0010 nnnn nnnn              C  Z  C  H    7
	    SUBB (HL+), n     1110 0110           0111 0010 nnnn nnnn              C  Z  C  H    6
	    SUBB (-HL), n     1110 0111           0111 0010 nnnn nnnn              C  Z  C  H    6

	    SUB (x), n        1110 0000 xxxx xxxx 0111 0011 nnnn nnnn              C  Z  C  H    6
	    SUB (PC+A), n     1110 0001           0111 0011 nnnn nnnn              C  Z  C  H    7
	    SUB (DE), n       1110 0010           0111 0011 nnnn nnnn              C  Z  C  H    5
	    SUB (HL), n       1110 0011           0111 0011 nnnn nnnn              C  Z  C  H    5
	    SUB (HL+d), n     1110 0100 dddd dddd 0111 0011 nnnn nnnn              C  Z  C  H    7
	    SUB (HL+C), n     1110 0101           0111 0011 nnnn nnnn              C  Z  C  H    7
	    SUB (HL+), n      1110 0110           0111 0011 nnnn nnnn              C  Z  C  H    6
	    SUB (-HL), n      1110 0111           0111 0011 nnnn nnnn              C  Z  C  H    6

	    AND (x), n        1110 0000 xxxx xxxx 0111 0100 nnnn nnnn              Z  Z  -  -    6
	    AND (PC+A), n     1110 0001           0111 0100 nnnn nnnn              Z  Z  -  -    7
	    AND (DE), n       1110 0010           0111 0100 nnnn nnnn              Z  Z  -  -    5
	    AND (HL), n       1110 0011           0111 0100 nnnn nnnn              Z  Z  -  -    5
	    AND (HL+d), n     1110 0100 dddd dddd 0111 0100 nnnn nnnn              Z  Z  -  -    7
	    AND (HL+C), n     1110 0101           0111 0100 nnnn nnnn              Z  Z  -  -    7
	    AND (HL+), n      1110 0110           0111 0100 nnnn nnnn              Z  Z  -  -    6
	    AND (-HL), n      1110 0111           0111 0100 nnnn nnnn              Z  Z  -  -    6

	    XOR (x), n        1110 0000 xxxx xxxx 0111 0101 nnnn nnnn              Z  Z  -  -    6
	    XOR (PC+A), n     1110 0001           0111 0101 nnnn nnnn              Z  Z  -  -    7
	    XOR (DE), n       1110 0010           0111 0101 nnnn nnnn              Z  Z  -  -    5
	    XOR (HL), n       1110 0011           0111 0101 nnnn nnnn              Z  Z  -  -    5
	    XOR (HL+d), n     1110 0100 dddd dddd 0111 0101 nnnn nnnn              Z  Z  -  -    7
	    XOR (HL+C), n     1110 0101           0111 0101 nnnn nnnn              Z  Z  -  -    7
	    XOR (HL+), n      1110 0110           0111 0101 nnnn nnnn              Z  Z  -  -    6
	    XOR (-HL), n      1110 0111           0111 0101 nnnn nnnn              Z  Z  -  -    6

	    OR (x), n         1110 0000 xxxx xxxx 0111 0110 nnnn nnnn              Z  Z  -  -    6
	    OR (PC+A), n      1110 0001           0111 0110 nnnn nnnn              Z  Z  -  -    7
	    OR (DE), n        1110 0010           0111 0110 nnnn nnnn              Z  Z  -  -    5
	    OR (HL), n        1110 0011           0111 0110 nnnn nnnn              Z  Z  -  -    5
	    OR (HL+d), n      1110 0100 dddd dddd 0111 0110 nnnn nnnn              Z  Z  -  -    7
	    OR (HL+C), n      1110 0101           0111 0110 nnnn nnnn              Z  Z  -  -    7
	    OR (HL+), n       1110 0110           0111 0110 nnnn nnnn              Z  Z  -  -    6
	    OR (-HL), n       1110 0111           0111 0110 nnnn nnnn              Z  Z  -  -    6

	    CMP (x), n        1110 0000 xxxx xxxx 0111 0111 nnnn nnnn              Z  Z  C  H    5
	    CMP (PC+A), n     1110 0001           0111 0111 nnnn nnnn              Z  Z  C  H    6
	    CMP (DE), n       1110 0010           0111 0111 nnnn nnnn              Z  Z  C  H    4
	    CMP (HL), n       1110 0011           0111 0111 nnnn nnnn              Z  Z  C  H    4
	    CMP (HL+d), n     1110 0100 dddd dddd 0111 0111 nnnn nnnn              Z  Z  C  H    6
	    CMP (HL+C), n     1110 0101           0111 0111 nnnn nnnn              Z  Z  C  H    6
	    CMP (HL+), n      1110 0110           0111 0111 nnnn nnnn              Z  Z  C  H    5
	    CMP (-HL), n      1110 0111           0111 0111 nnnn nnnn              Z  Z  C  H    5

	    aka (ALU OP) (src), n
	*/
	m_cycles += 5;

	const uint8_t n = READ8();

	const int aluop = (opbyte1 & 0x7);

	if (aluop != 0x07)
		m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports

	const uint8_t val = RM8(srcaddr);

	const uint8_t result = do_alu_8bit(aluop, val, n);

	if (aluop != 0x07) // CMP doesn't write back
	{
		m_cycles -= 1; // one less for CMP here?
		WM8(srcaddr, result);
	}
}

void tlcs870_device::do_ALUOP_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC A, (x)       1110 0000 xxxx xxxx 0111 1000                        C  Z  C  H    4
	    ADDC A, (PC+A)    1110 0001           0111 1000                        C  Z  C  H    5
	    ADDC A, (DE)      1110 0010           0111 1000                        C  Z  C  H    3
	    ADDC A, (HL)      1110 0011           0111 1000                        C  Z  C  H    3
	    ADDC A, (HL+d)    1110 0100 dddd dddd 0111 1000                        C  Z  C  H    5
	    ADDC A, (HL+C)    1110 0101           0111 1000                        C  Z  C  H    5
	    ADDC A, (HL+)     1110 0110           0111 1000                        C  Z  C  H    4
	    ADDC A, (-HL)     1110 0111           0111 1000                        C  Z  C  H    4

	    ADD A, (x)        1110 0000 xxxx xxxx 0111 1001                        C  Z  C  H    4
	    ADD A, (PC+A)     1110 0001           0111 1001                        C  Z  C  H    5
	    ADD A, (DE)       1110 0010           0111 1001                        C  Z  C  H    3
	    ADD A, (HL)       1110 0011           0111 1001                        C  Z  C  H    3
	    ADD A, (HL+d)     1110 0100 dddd dddd 0111 1001                        C  Z  C  H    5
	    ADD A, (HL+C)     1110 0101           0111 1001                        C  Z  C  H    5
	    ADD A, (HL+)      1110 0110           0111 1001                        C  Z  C  H    4
	    ADD A, (-HL)      1110 0111           0111 1001                        C  Z  C  H    4

	    SUBB A, (x)       1110 0000 xxxx xxxx 0111 1010                        C  Z  C  H    4
	    SUBB A, (PC+A)    1110 0001           0111 1010                        C  Z  C  H    5
	    SUBB A, (DE)      1110 0010           0111 1010                        C  Z  C  H    3
	    SUBB A, (HL)      1110 0011           0111 1010                        C  Z  C  H    3
	    SUBB A, (HL+d)    1110 0100 dddd dddd 0111 1010                        C  Z  C  H    5
	    SUBB A, (HL+C)    1110 0101           0111 1010                        C  Z  C  H    5
	    SUBB A, (HL+)     1110 0110           0111 1010                        C  Z  C  H    4
	    SUBB A, (-HL)     1110 0111           0111 1010                        C  Z  C  H    4

	    SUB A, (x)        1110 0000 xxxx xxxx 0111 1011                        C  Z  C  H    4
	    SUB A, (PC+A)     1110 0001           0111 1011                        C  Z  C  H    5
	    SUB A, (DE)       1110 0010           0111 1011                        C  Z  C  H    3
	    SUB A, (HL)       1110 0011           0111 1011                        C  Z  C  H    3
	    SUB A, (HL+d)     1110 0100 dddd dddd 0111 1011                        C  Z  C  H    5
	    SUB A, (HL+C)     1110 0101           0111 1011                        C  Z  C  H    5
	    SUB A, (HL+)      1110 0110           0111 1011                        C  Z  C  H    4
	    SUB A, (-HL)      1110 0111           0111 1011                        C  Z  C  H    4

	    AND A, (x)        1110 0000 xxxx xxxx 0111 1100                        Z  Z  -  -    4
	    AND A, (PC+A)     1110 0001           0111 1100                        Z  Z  -  -    5
	    AND A, (DE)       1110 0010           0111 1100                        Z  Z  -  -    3
	    AND A, (HL)       1110 0011           0111 1100                        Z  Z  -  -    3
	    AND A, (HL+d)     1110 0100 dddd dddd 0111 1100                        Z  Z  -  -    5
	    AND A, (HL+C)     1110 0101           0111 1100                        Z  Z  -  -    5
	    AND A, (HL+)      1110 0110           0111 1100                        Z  Z  -  -    4
	    AND A, (-HL)      1110 0111           0111 1100                        Z  Z  -  -    4

	    XOR A, (x)        1110 0000 xxxx xxxx 0111 1101                        Z  Z  -  -    4
	    XOR A, (PC+A)     1110 0001           0111 1101                        Z  Z  -  -    5
	    XOR A, (DE)       1110 0010           0111 1101                        Z  Z  -  -    3
	    XOR A, (HL)       1110 0011           0111 1101                        Z  Z  -  -    3
	    XOR A, (HL+d)     1110 0100 dddd dddd 0111 1101                        Z  Z  -  -    5
	    XOR A, (HL+C)     1110 0101           0111 1101                        Z  Z  -  -    5
	    XOR A, (HL+)      1110 0110           0111 1101                        Z  Z  -  -    4
	    XOR A, (-HL)      1110 0111           0111 1101                        Z  Z  -  -    4

	    OR A, (x)         1110 0000 xxxx xxxx 0111 1110                        Z  Z  -  -    4
	    OR A, (PC+A)      1110 0001           0111 1110                        Z  Z  -  -    5
	    OR A, (DE)        1110 0010           0111 1110                        Z  Z  -  -    3
	    OR A, (HL)        1110 0011           0111 1110                        Z  Z  -  -    3
	    OR A, (HL+d)      1110 0100 dddd dddd 0111 1110                        Z  Z  -  -    5
	    OR A, (HL+C)      1110 0101           0111 1110                        Z  Z  -  -    5
	    OR A, (HL+)       1110 0110           0111 1110                        Z  Z  -  -    4
	    OR A, (-HL)       1110 0111           0111 1110                        Z  Z  -  -    4

	    CMP A, (x)        1110 0000 xxxx xxxx 0111 1111                        Z  Z  C  H    4
	    CMP A, (PC+A)     1110 0001           0111 1111                        Z  Z  C  H    5
	    CMP A, (DE)       1110 0010           0111 1111                        Z  Z  C  H    3
	    CMP A, (HL)       1110 0011           0111 1111                        Z  Z  C  H    3
	    CMP A, (HL+d)     1110 0100 dddd dddd 0111 1111                        Z  Z  C  H    5
	    CMP A, (HL+C)     1110 0101           0111 1111                        Z  Z  C  H    5
	    CMP A, (HL+)      1110 0110           0111 1111                        Z  Z  C  H    4
	    CMP A, (-HL)      1110 0111           0111 1111                        Z  Z  C  H    4

	    aka (ALU OP) A, (src)
	*/
	m_cycles += 3;

	const int aluop = (opbyte1 & 0x7);
	const uint8_t val = RM8(srcaddr);

	const uint8_t result = do_alu_8bit(aluop, get_reg8(REG_A), val);

	if (aluop != 0x07) // CMP doesn't write back
	{
		// NOT one less for CMP here?
		set_reg8(REG_A, result);
	}
}

/**********************************************************************************************************************/
// jumps / calls
/**********************************************************************************************************************/

void tlcs870_device::do_CALL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CALL (x)          1110 0000 xxxx xxxx 1111 1100                        -  -  -  -    9
	    CALL (PC+A)       1110 0001           1111 1100                        -  -  -  -    10
	    CALL (DE)         1110 0010           1111 1100                        -  -  -  -    8
	    CALL (HL)         1110 0011           1111 1100                        -  -  -  -    8
	    CALL (HL+d)       1110 0100 dddd dddd 1111 1100                        -  -  -  -    10
	    CALL (HL+C)       1110 0101           1111 1100                        -  -  -  -    10
	    CALL (HL+)        not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?
	    CALL (-HL)        not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?

	    aka CALL (src)
	*/
	m_cycles += 8;

	const uint16_t val = RM16(srcaddr);

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = val;

	// no flag changes on call
}

void tlcs870_device::do_JP_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    JP (x)            1110 0000 xxxx xxxx 1111 1110                        1  -  -  -    6
	    JP (PC+A)         1110 0001           1111 1110                        1  -  -  -    7
	    JP (DE)           1110 0010           1111 1110                        1  -  -  -    5
	    JP (HL)           1110 0011           1111 1110                        1  -  -  -    5
	    JP (HL+d)         1110 0100 dddd dddd 1111 1110                        1  -  -  -    7
	    JP (HL+C)         1110 0101           1111 1110                        1  -  -  -    7
	    JP (HL+)          not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?
	    JP (-HL)          not listed, invalid due to 16-bit op?                ?  ?  ?  ?    ?

	    aka JP (src)
	*/
	m_cycles += 5;

	const uint16_t val = RM16(srcaddr);
	m_addr = val;
	set_JF();
}

/**********************************************************************************************************************/
// (8-bit) bit operations
/**********************************************************************************************************************/

void tlcs870_device::do_XOR_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    XOR CF, (x).b     1110 0000 xxxx xxxx 1101 0bbb                        ~C -  *  -    4
	    XOR CF, (PC+A).b  1110 0001           1101 0bbb                        ~C -  *  -    5
	    XOR CF, (DE).b    1110 0010           1101 0bbb                        ~C -  *  -    3
	    XOR CF, (HL).b    1110 0011           1101 0bbb                        ~C -  *  -    3
	    XOR CF, (HL+d).b  1110 0100 dddd dddd 1101 0bbb                        ~C -  *  -    5
	    XOR CF, (HL+C).b  1110 0101           1101 0bbb                        ~C -  *  -    5
	    XOR CF, (HL+).b   1110 0110           1101 0bbb                        ~C -  *  -    4
	    XOR CF, (-HL).b   1110 0111           1101 0bbb                        ~C -  *  -    4

	    aka XOR CF,(src).b
	*/
	m_cycles += 3;

	const uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	int bitused = (1 << bitpos);

	const uint8_t bit = val & bitused;

	if (is_CF())
	{
		if (bit)
		{
			clear_CF();
		}
		else
		{
			set_CF();
		}
	}
	else
	{
		if (bit)
		{
			set_CF();
		}
		else
		{
			clear_CF();
		}
	}

	// JF ends up being whatever the new value of CF is
	if (is_CF())
		set_JF();
	else
		clear_JF();
}


void tlcs870_device::do_LD_insrcbit_CF(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (x).b, CF      1110 0000 xxxx xxxx 1100 1bbb                        1  -  -  -    5
	    LD (PC+A).b, CF   1110 0001           1100 1bbb                        1  -  -  -    6
	    LD (DE).b, CF     1110 0010           1100 1bbb                        1  -  -  -    4
	    LD (HL).b, CF     1110 0011           1100 1bbb                        1  -  -  -    4
	    LD (HL+d).b, CF   1110 0100 dddd dddd 1100 1bbb                        1  -  -  -    6
	    LD (HL+C).b, CF   1110 0101           1100 1bbb                        1  -  -  -    6
	    LD (HL+).b, CF    1110 0110           1100 1bbb                        1  -  -  -    5
	    LD (-HL).b, CF    1110 0111           1100 1bbb                        1  -  -  -    5

	    aka LD (src).b,CF
	*/
	m_cycles += 4;

	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	if (is_CF()) // if carry flag is set, set the bit in val
	{
		val |= bitused;
	}
	else // if carry flag isn't set, clear the bit in val
	{
		val &= ~bitused;
	}

	// for this optype of operation ( LD *.b, CF ) the Jump Flag always ends up being 1
	set_JF();

	WM8(srcaddr, val);
}

void tlcs870_device::do_CPL_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CPL (x).b         1110 0000 xxxx xxxx 1100 0bbb                        Z  *  -  -    5
	    CPL (PC+A).b      1110 0001           1100 0bbb                        Z  *  -  -    6
	    CPL (DE).b        1110 0010           1100 0bbb                        Z  *  -  -    4
	    CPL (HL).b        1110 0011           1100 0bbb                        Z  *  -  -    4
	    CPL (HL+d).b      1110 0100 dddd dddd 1100 0bbb                        Z  *  -  -    6
	    CPL (HL+C).b      1110 0101           1100 0bbb                        Z  *  -  -    6
	    CPL (HL+).b       1110 0110           1100 0bbb                        Z  *  -  -    5
	    CPL (-HL).b       1110 0111           1100 0bbb                        Z  *  -  -    5

	    aka CPL (src).b
	*/
	m_cycles += 4;

	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	const uint8_t bit = val & bitused;

	if (bit) // if the bit is set, clear the zero/jump flags and unset the bit
	{
		clear_ZF();
		clear_JF();

		val &= ~bitused;
	}
	else  // if the bit isn't set, set the zero/jump flags and set the bit
	{
		set_ZF();
		set_JF();

		val |= bitused;
	}

	WM8(srcaddr, val);
}

void tlcs870_device::do_LD_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD CF, (x).b      1110 0000 xxxx xxxx 1101 1bbb                        ~C -  *  -    4
	    LD CF, (PC+A).b   1110 0001           1101 1bbb                        ~C -  *  -    5
	    LD CF, (DE).b     1110 0010           1101 1bbb                        ~C -  *  -    3
	    LD CF, (HL).b     1110 0011           1101 1bbb                        ~C -  *  -    3
	    LD CF, (HL+d).b   1110 0100 dddd dddd 1101 1bbb                        ~C -  *  -    5
	    LD CF, (HL+C).b   1110 0101           1101 1bbb                        ~C -  *  -    5
	    LD CF, (HL+).b    1110 0110           1101 1bbb                        ~C -  *  -    4
	    LD CF, (-HL).b    1110 0111           1101 1bbb                        ~C -  *  -    4

	    aka LD CF,(src).b  or  TEST (src).b
	*/
	m_cycles += 3;

	const uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	const uint8_t bit = val & bitused;

	bit ? set_CF() : clear_CF();
	// for this optype of operation ( LD CF, *.b ) the Jump Flag always ends up the inverse of the Carry Flag
	bit ? clear_JF() : set_JF();
}

void tlcs870_device::do_SET_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SET (x).b         1110 0000 xxxx xxxx 0100 0bbb                        Z  *  -  -    5
	    SET (PC+A).b      1110 0001           0100 0bbb                        Z  *  -  -    6
	    SET (DE).b        1110 0010           0100 0bbb                        Z  *  -  -    4
	    SET (HL).b        1110 0011           0100 0bbb                        Z  *  -  -    4
	    SET (HL+d).b      1110 0100 dddd dddd 0100 0bbb                        Z  *  -  -    6
	    SET (HL+C).b      1110 0101           0100 0bbb                        Z  *  -  -    6
	    SET (HL+).b       1110 0110           0100 0bbb                        Z  *  -  -    5
	    SET (-HL).b       1110 0111           0100 0bbb                        Z  *  -  -    5

	    aka SET (src).b
	*/
	m_cycles += 4;

	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	if (val & bitused) // Zero flag gets set based on original value of bit?
	{
		clear_ZF();
		clear_JF(); // 'Z' (so copy Z flag?)
	}
	else
	{
		set_ZF();
		set_JF();  // 'Z'
	}

	val |= bitused;

	WM8(srcaddr, val);
}

void tlcs870_device::do_CLR_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CLR (x).b         1110 0000 xxxx xxxx 0100 1bbb                        Z  *  -  -    5
	    CLR (PC+A).b      1110 0001           0100 1bbb                        Z  *  -  -    6
	    CLR (DE).b        1110 0010           0100 1bbb                        Z  *  -  -    4
	    CLR (HL).b        1110 0011           0100 1bbb                        Z  *  -  -    4
	    CLR (HL+d).b      1110 0100 dddd dddd 0100 1bbb                        Z  *  -  -    6
	    CLR (HL+C).b      1110 0101           0100 1bbb                        Z  *  -  -    6
	    CLR (HL+).b       1110 0110           0100 1bbb                        Z  *  -  -    5
	    CLR (-HL).b       1110 0111           0100 1bbb                        Z  *  -  -    5

	    aka CLR (src).b
	*/
	m_cycles += 4;

	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	if (val & bitused) // Zero flag gets set based on original value of bit?
	{
		clear_ZF();
		clear_JF(); // 'Z' (so copy Z flag?)
	}
	else
	{
		set_ZF();
		set_JF();  // 'Z'
	}

	val &= ~bitused;

	WM8(srcaddr, val);
}

