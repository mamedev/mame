// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************************************************

    Toshiba TLCS-870 Series MCUs

    (reg) prefix ops (e8 to ef subtable)

    (reg) implies one of the follow registers

    (8-bit mode operations)
    E8  A
    E9  W
    EA  C
    EB  B
    EC  E
    ED  D
    EE  L
    EF  H

    (16-bit mode operations)
    E8  WA
    E9  BC
    EA  DE
    EB  HL
    EC  WA
    ED  BC
    EE  DE
    EF  HL

    (RETN operation - special)
    E8  RETN
    E9-EF invalid

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"

// Main dispatch handlers for these

void tlcs870_device::do_regprefixtype_opcode(const uint8_t opbyte0)
{
	// register prefix: g/gg
	uint8_t opbyte1 = READ8();

	switch (opbyte1)
	{
	case 0x01:
		do_SWAP_g(opbyte0, opbyte1); break;
	case 0x02:
		do_MUL_gg(opbyte0, opbyte1); break;
	case 0x03:
		do_DIV_gg_C(opbyte0, opbyte1); break;
	case 0x04:
		do_RETN(opbyte0, opbyte1); break;
	case 0x06:
		do_POP_gg(opbyte0, opbyte1); break;
	case 0x07:
		do_PUSH_gg(opbyte0, opbyte1); break;
	case 0x0a:
		do_DAA_g(opbyte0, opbyte1); break;
	case 0x0b:
		do_DAS_g(opbyte0, opbyte1); break;
	case 0x10: case 0x11: case 0x12: case 0x13:
		do_XCH_rr_gg(opbyte0, opbyte1);  break;
	case 0x14: case 0x15: case 0x16: case 0x17:
		do_LD_rr_gg(opbyte0, opbyte1);  break;
	case 0x1c:
		do_SHLC_g(opbyte0, opbyte1); break;
	case 0x1d:
		do_SHRC_g(opbyte0, opbyte1); break;
	case 0x1e:
		do_ROLC_g(opbyte0, opbyte1); break;
	case 0x1f:
		do_RORC_g(opbyte0, opbyte1); break;
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		do_ALUOP_WA_gg(opbyte0, opbyte1);  break;
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		do_ALUOP_gg_mn(opbyte0, opbyte1);  break;
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		do_SET_gbit(opbyte0, opbyte1);  break;
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		do_CLR_gbit(opbyte0, opbyte1);  break;
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		do_LD_r_g(opbyte0, opbyte1);  break;
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		do_ALUOP_A_g(opbyte0, opbyte1);  break;
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		do_ALUOP_g_A(opbyte0, opbyte1);  break;
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		do_ALUOP_g_n(opbyte0, opbyte1);  break;
	case 0x82: case 0x83:
		do_SET_inppbit(opbyte0, opbyte1);  break;
	case 0x8a: case 0x8b:
		do_CLR_inppbit(opbyte0, opbyte1);  break;
	case 0x92: case 0x93:
		do_CPL_inpp_indirectbit(opbyte0, opbyte1);  break;
	case 0x9a: case 0x9b:
		do_LD_inpp_indirectbit_CF(opbyte0, opbyte1);  break;
	case 0x9e: case 0x9f:
		do_LD_CF_inpp_indirectbit(opbyte0, opbyte1);  break;
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		do_XCH_r_g(opbyte0, opbyte1);  break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		do_CPL_gbit(opbyte0, opbyte1);  break;
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		do_LD_gbit_CF(opbyte0, opbyte1);  break;
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		do_XOR_CF_gbit(opbyte0, opbyte1);  break;
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		do_LD_CF_gbit(opbyte0, opbyte1);  break;
	case 0xfa:
		do_LD_SP_gg(opbyte0, opbyte1); break;
	case 0xfb:
		do_LD_gg_SP(opbyte0, opbyte1); break;
	case 0xfc:
		do_CALL_gg(opbyte0, opbyte1); break;
	case 0xfe:
		do_JP_gg(opbyte0, opbyte1); break;

	default:
		do_regprefixtype_oprand_illegal(opbyte0, opbyte1); break;
	}
}

// Actual handlers

/**********************************************************************************************************************/
// (Special)
/**********************************************************************************************************************/

void tlcs870_device::do_regprefixtype_oprand_illegal(const uint8_t opbyte0, const uint8_t opbyte1)
{
	m_cycles = 1;
	logerror("illegal reg prefix opcode %02x %02x\n", opbyte0, opbyte1);
}


void tlcs870_device::do_RETN(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// with E8 only
	if (opbyte0 == 0xe8)
	{
		/*
		    Return from non-maskable interrupt service (how does this differ from RETI?)
		    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
		    RETN              1110 1000           0000 0100                        *  *  *  *    7
		*/
		m_cycles = 7;

		m_sp.d += 3;
		m_addr = RM16(m_sp.d - 2);
		set_PSW(RM8(m_sp.d - 1));
	}
	else
	{
		do_regprefixtype_oprand_illegal(opbyte0, opbyte1);
	}
}

/**********************************************************************************************************************/
// (8-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_SWAP_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SWAP g            1110 1ggg           0000 0001                        1  -  -  -    4
	*/
	m_cycles = 4;

	const uint8_t reg = opbyte0 & 0x7;
	handle_swap(reg);
}

void tlcs870_device::do_DAA_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    DAS g             1110 1ggg           0000 1010                        C  Z  C  H    3
	*/
	m_cycles = 3;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_DAA(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_DAS_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    DAS g             1110 1ggg           0000 1011                        C  Z  C  H    3
	*/
	m_cycles = 3;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_DAS(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_SHLC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    Logical Shift Left with Carry Flag
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SHLC g            1110 1ggg           0001 1100                        C  Z  *  -    2
	*/
	m_cycles = 2;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_SHLC(val);

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_SHRC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    Logical Shift Right with Carry Flag
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SHRC g            1110 1ggg           0001 1101                        C  Z  *  -    2
	*/
	m_cycles = 2;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_SHRC(val);

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_ROLC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    Rotate Left through Carry flag
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ROLC g            1110 1ggg           0001 1110                        C  Z  *  -    2
	*/
	m_cycles = 2;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_ROLC(val);

	set_reg8(opbyte0 & 0x7, val);

}

void tlcs870_device::do_RORC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    Rotate Right through Carry flag
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    RORC g            1110 1ggg           0001 1111                        C  Z  *  -    2
	*/
	m_cycles = 2;

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_RORC(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_LD_r_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD r, g           1110 1ggg           0101 1rrr                        1  Z  -  -    2
	*/
	m_cycles = 2;

	const uint8_t val = get_reg8(opbyte0 & 0x7);
	set_reg8(opbyte1 & 0x7, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_XCH_r_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    XCG r, g          1110 1ggg           1010 1rrr                        1  Z  -  -    3
	*/
	m_cycles = 3;

	const uint8_t val = get_reg8(opbyte0 & 0x7);
	const uint8_t r = get_reg8(opbyte1 & 0x7);

	set_reg8(opbyte1 & 0x7, val);
	set_reg8(opbyte0 & 0x7, r);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

/**********************************************************************************************************************/
// (ALU handlers)
/**********************************************************************************************************************/

void tlcs870_device::do_ALUOP_A_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC A, g         1110 1ggg           0110 0000                        C  Z  C  H    2
	    ADD A, g          1110 1ggg           0110 0001                        C  Z  C  H    2
	    SUBB A, g         1110 1ggg           0110 0010                        C  Z  C  H    2
	    SUB A, g          1110 1ggg           0110 0011                        C  Z  C  H    2
	    AND A, g          1110 1ggg           0110 0100                        Z  Z  -  -    2
	    XOR A, g          1110 1ggg           0110 0101                        Z  Z  -  -    2
	    OR A, g           1110 1ggg           0110 0110                        Z  Z  -  -    2
	    CMP A, g          1110 1ggg           0110 0111                        Z  Z  C  H    2
	*/
	m_cycles = 2;

	const int aluop = (opbyte1 & 0x7);

	const uint8_t result = do_alu_8bit(aluop, get_reg8(REG_A), get_reg8(opbyte0 & 0x7));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(REG_A, result);
	}
}

void tlcs870_device::do_ALUOP_g_A(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC g, A         1110 1ggg           0110 1000                        C  Z  C  H    3
	    ADD g, A          1110 1ggg           0110 1001                        C  Z  C  H    3
	    SUBB g, A         1110 1ggg           0110 1010                        C  Z  C  H    3
	    SUB g, A          1110 1ggg           0110 1011                        C  Z  C  H    3
	    AND g, A          1110 1ggg           0110 1100                        Z  Z  -  -    3
	    XOR g, A          1110 1ggg           0110 1101                        Z  Z  -  -    3
	    OR g, A           1110 1ggg           0110 1110                        Z  Z  -  -    3
	    CMP g, A          1110 1ggg           0110 1111                        Z  Z  C  H    3
	*/
	m_cycles = 3;

	const int aluop = (opbyte1 & 0x7);
	const uint8_t result = do_alu_8bit(aluop, get_reg8(opbyte0 & 0x7), get_reg8(REG_A));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(opbyte0 & 0x7, result);
	}
}

void tlcs870_device::do_ALUOP_g_n(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC g, n         1110 1ggg           0111 0000 nnnn nnnn              C  Z  C  H    3
	    ADD g, n          1110 1ggg           0111 0001 nnnn nnnn              C  Z  C  H    3
	    SUBB g, n         1110 1ggg           0111 0010 nnnn nnnn              C  Z  C  H    3
	    SUB g, n          1110 1ggg           0111 0011 nnnn nnnn              C  Z  C  H    3
	    AND g, n          1110 1ggg           0111 0100 nnnn nnnn              Z  Z  -  -    3
	    XOR g, n          1110 1ggg           0111 0101 nnnn nnnn              Z  Z  -  -    3
	    OR g, n           1110 1ggg           0111 0110 nnnn nnnn              Z  Z  -  -    3
	    CMP g, n          1110 1ggg           0111 0111 nnnn nnnn              Z  Z  C  H    3
	*/
	m_cycles = 3;

	const int aluop = (opbyte1 & 0x7);

	const uint8_t n = READ8();

	const uint8_t result = do_alu_8bit(aluop, get_reg8(opbyte0 & 0x7), n);

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(opbyte0 & 0x7, result);
	}
}

/**********************************************************************************************************************/
// (16-bit ALU handlers)
/**********************************************************************************************************************/

void tlcs870_device::do_ALUOP_WA_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC WA, gg       1110 10gg           0011 0000                        C  Z  C  U    4
	    ADD WA, gg        1110 10gg           0011 0001                        C  Z  C  U    4
	    SUBB WA, gg       1110 10gg           0011 0010                        C  Z  C  U    4
	    SUB WA, gg        1110 10gg           0011 0011                        C  Z  C  U    4
	    AND WA, gg        1110 10gg           0011 0100                        Z  Z  -  -    4
	    XOR WA, gg        1110 10gg           0011 0101                        Z  Z  -  -    4
	    OR WA, gg         1110 10gg           0011 0110                        Z  Z  -  -    4
	    CMP WA, gg        1110 10gg           0011 0111                        Z  Z  C  U    4
	*/
	m_cycles = 4;

	const int aluop = (opbyte1 & 0x7);

	const uint16_t result = do_alu_16bit(aluop, get_reg16(REG_WA), get_reg16(opbyte0 & 0x3));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg16(REG_WA, result);
	}

}

void tlcs870_device::do_ALUOP_gg_mn(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    ADDC gg, mn       1110 10gg           0011 1000 nnnn nnnn mmmm mmmm    C  Z  C  U    4
	    ADD gg, mn        1110 10gg           0011 1001 nnnn nnnn mmmm mmmm    C  Z  C  U    4
	    SUBB gg, mn       1110 10gg           0011 1010 nnnn nnnn mmmm mmmm    C  Z  C  U    4
	    SUB gg, mn        1110 10gg           0011 1011 nnnn nnnn mmmm mmmm    C  Z  C  U    4
	    AND gg, mn        1110 10gg           0011 1100 nnnn nnnn mmmm mmmm    Z  Z  -  -    4
	    XOR gg, mn        1110 10gg           0011 1101 nnnn nnnn mmmm mmmm    Z  Z  -  -    4
	    OR gg, mn         1110 10gg           0011 1110 nnnn nnnn mmmm mmmm    Z  Z  -  -    4
	    CMP gg, mn        1110 10gg           0011 1111 nnnn nnnn mmmm mmmm    Z  Z  C  U    4
	*/
	m_cycles = 4;

	const int aluop = (opbyte1 & 0x7);

	const uint16_t mn = READ16();

	const uint16_t result = do_alu_16bit(aluop, get_reg16(opbyte0 & 0x3), mn);

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg16(opbyte0 & 0x3, result);
	}
}

/**********************************************************************************************************************/
// bit accesses
/**********************************************************************************************************************/

// ops using (pp).g

void tlcs870_device::do_SET_inppbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SET (DE).g        1110 1ggg           1000 0010                        Z  *  -  -    5
	    SET (HL).g        1110 1ggg           1000 0011                        Z  *  -  -    5
	*/
	m_cycles = 5;

	const uint8_t bitpos = get_reg8(opbyte0 & 7) & 0x7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
	uint8_t val = RM8(addr);

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

	WM8(addr, val);
}

void tlcs870_device::do_CLR_inppbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CLR (DE).g        1110 1ggg           1000 1010                        Z  *  -  -    5
	    CLR (HL).g        1110 1ggg           1000 1011                        Z  *  -  -    5
	*/
	m_cycles = 5;

	const uint8_t bitpos = get_reg8(opbyte0 & 7) & 0x7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(addr);

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

	WM8(addr, val);
}

void tlcs870_device::do_CPL_inpp_indirectbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CPL (DE).g        1110 1ggg           1001 0010                        Z  *  -  -    5
	    CPL (HL).g        1110 1ggg           1001 0011                        Z  *  -  -    5
	*/
	m_cycles = 5;

	const uint8_t bitpos = get_reg8(opbyte0 & 7) & 0x7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
	m_read_input_port = 0; // reads output latch, not actual ports if accessing memory mapped ports
	uint8_t val = RM8(addr);

	uint8_t bit = val & bitused;

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

	WM8(addr, val);
}

void tlcs870_device::do_LD_inpp_indirectbit_CF(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD (DE).g, CF     1110 1ggg           1001 1010                        1  -  -  -    5
	    LD (HL).g, CF     1110 1ggg           1001 1011                        1  -  -  -    5
	*/
	m_cycles = 5;

	const uint8_t bitpos = get_reg8(opbyte0 & 7) & 0x7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
	uint8_t val = RM8(addr);

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

	WM8(addr, val);

}

void tlcs870_device::do_LD_CF_inpp_indirectbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD CF, (DE).g     1110 1ggg           1001 1110                        ~C -  *  -    4
	    LD CF, (HL).g     1110 1ggg           1001 1111                        ~C -  *  -    4

	    aka aka TEST (pp).g
	*/
	m_cycles = 4;

	const uint8_t bitpos = get_reg8(opbyte0 & 7) & 0x7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
	const uint8_t val = RM8(addr);

	const uint8_t bit = val & bitused;

	bit ? set_CF() : clear_CF();
	// for this optype of operation ( LD CF, *.b ) the Jump Flag always ends up the inverse of the Carry Flag
	bit ? clear_JF() : set_JF();
}

// ops using g.b

void tlcs870_device::do_SET_gbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    SET g.b           1110 1ggg           0100 0bbb                        Z  *  -  -    3
	*/
	m_cycles = 3;

	uint8_t val = get_reg8(opbyte0 & 0x7);
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

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_CLR_gbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CLR g.b           1110 1ggg           0100 1bbb                        Z  *  -  -    3
	*/
	m_cycles = 3;

	uint8_t val = get_reg8(opbyte0 & 0x7);
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

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_CPL_gbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CPL g.b           1110 1ggg           1100 0bbb                        Z  *  -  -    3
	*/
	m_cycles = 3;

	uint8_t val = get_reg8(opbyte0 & 0x7);
	const uint8_t bitpos = opbyte1 & 0x7;

	const int bitused = (1 << bitpos);

	uint8_t bit = val & bitused;

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

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_LD_gbit_CF(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD g.b, CF        1110 1ggg           1100 1bbb                        1  -  -  -    2
	*/
	m_cycles = 2;

	uint8_t val = get_reg8(opbyte0 & 0x7);
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

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_XOR_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    XOR CF, g.b       1110 1ggg           1101 0bbb                        ~C -  *  -    2
	*/
	m_cycles = 2;

	const uint8_t bitpos = opbyte1 & 0x7;
	const uint8_t bitused = 1 << bitpos;

	const uint8_t g = get_reg8(opbyte0 & 0x7);

	const uint8_t bit = g & bitused;

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


void tlcs870_device::do_LD_CF_gbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD CF, g.b        1110 1ggg           1101 1bbb                        ~C -  *  -    2

	    aka TEST g.b
	*/
	m_cycles = 2;

	const uint8_t bitpos = opbyte1 & 0x7;
	const uint8_t bitused = 1 << bitpos;

	const uint8_t g = get_reg8(opbyte0 & 0x7);

	const uint8_t bit = g & bitused;

	bit ? set_CF() : clear_CF();
	// for this optype of operation ( LD CF, *.b ) the Jump Flag always ends up the inverse of the Carry Flag
	bit ? clear_JF() : set_JF();
}

/**********************************************************************************************************************/
// 16-bit
/**********************************************************************************************************************/

void tlcs870_device::do_MUL_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    MUL W, A          not listed (redundant encoding?)                     ?  ?  ?  ?    ?
	    MUL B, C          1110 1010           0000 0010                        Z  Z  -  -    8
	    MUL D, E          1110 1010           0000 0010                        Z  Z  -  -    8
	    MUL H, L          1110 1011           0000 0010                        Z  Z  -  -    8

	    aka MUL ggH, ggL (odd syntax, basically MUL gg)
	*/
	m_cycles = 8;

	const uint8_t reg = opbyte0 & 0x3; // opbyte0 & 4 = invalid?
	handle_mul(reg);  // flag changes in handler
}

void tlcs870_device::do_DIV_gg_C(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    DIV WA, C         not listed (redundant encoding?)                     ?  ?  ?  ?    ?
	    DIV BC, C         not listed (illegal?)                                ?  ?  ?  ?    ?
	    DIV DE, C         1110 1010           0000 0011                        Z  Z  C  -    8
	    DIV HL, C         1110 1011           0000 0011                        Z  Z  C  -    8
	*/
	m_cycles = 8;

	const uint8_t reg = opbyte0 & 0x3; // opbyte0 & 4 = invalid?
	handle_div(reg); // flag changes in handler
}

void tlcs870_device::do_POP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    POP gg            1110 10gg           0000 0110                        -  -  -  -    5
	*/
	m_cycles = 5;

	m_sp.d += 2;
	const uint16_t val = RM16(m_sp.d - 1);
	set_reg16(opbyte0 & 3, val);
	// no flag changes
}

void tlcs870_device::do_PUSH_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    PUSH gg           1110 10gg           0000 0111                        -  -  -  -    4
	*/
	m_cycles = 4;

	const uint16_t val = get_reg16(opbyte0 & 3);
	WM16(m_sp.d - 1, val);
	m_sp.d -= 2;
	// no flag changes
}


void tlcs870_device::do_LD_SP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD SP, gg         1110 10gg           1111 1010                        1  -  -  -    3
	*/
	m_cycles = 3;

	m_sp.d = get_reg16(opbyte0 & 0x3);
	set_JF(); // no other flag changes for this type of LD
}

void tlcs870_device::do_LD_gg_SP(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD gg, SP         1110 10gg           1111 1011                        1  -  -  -    3
	*/
	m_cycles = 3;

	set_reg16(opbyte0 & 0x3, m_sp.d);
	set_JF(); // no other flag changes for this type of LD
}

void tlcs870_device::do_LD_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    LD rr, gg         1110 10gg           0001 00rr                        1  -  -  -    2
	*/
	m_cycles = 2;

	const uint16_t gg = get_reg16(opbyte0 & 0x3);
	set_reg16(opbyte1 & 0x3, gg);

	set_JF(); // no other flag changes for this type of LD
}


void tlcs870_device::do_XCH_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    XCH rr, gg        1110 10gg           0001 01rr                        1  -  -  -    3
	*/
	m_cycles = 3;

	const uint16_t gg = get_reg16(opbyte0 & 0x3);
	const uint16_t rr = get_reg16(opbyte1 & 0x3);

	set_reg16(opbyte1 & 0x3, gg);
	set_reg16(opbyte0 & 0x3, rr);

	set_JF();
}


void tlcs870_device::do_CALL_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    CALL gg           1110 10gg           1111 1100                        -  -  -  -    6
	*/
	m_cycles = 6;

	const uint16_t val = get_reg16(opbyte0 & 3);

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = val;

	// no flag changes on call
}

void tlcs870_device::do_JP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	/*
	    OP                (opbyte0) (immval0) (opbyte1) (immval1) (immval2)    JF ZF CF HF   cycles
	    JP gg             1110 10gg           1111 1110                        1  -  -  -    3
	*/
	m_cycles = 3;

	const uint16_t val = get_reg16(opbyte0 & 3);

	m_addr = val;
	set_JF();
}
