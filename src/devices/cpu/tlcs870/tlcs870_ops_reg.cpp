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
    E8  invalid
	E9  invalid
	EA  invalid
	EB  invalid
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
		do_CPL_inppbit(opbyte0, opbyte1);  break;
	case 0x9a: case 0x9b:
		do_LD_inppbit_CF(opbyte0, opbyte1);  break;
	case 0x9e: case 0x9f:
		do_LD_CF_inppbit(opbyte0, opbyte1);  break;
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
	logerror("illegal reg prefix opcode %02x %02x\n", opbyte0, opbyte1);
}


void tlcs870_device::do_RETN(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// with E8 only
	if (opbyte0 == 0xe8)
	{
		// RETN
		// Return from non-maskable interrupt service (how does this differ from RETI?)
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
	// SWAP g
	const uint8_t reg = opbyte0 & 0x7;
	handle_swap(reg);
}

void tlcs870_device::do_DAA_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// DAA g
	// 1110 1ggg 0000 1010
	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_DAA(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_DAS_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// DAS g
	// 1110 1ggg 0000 1011
	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_DAS(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_SHLC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// SHLC g
	// Logical Shift Left with Carry Flag
	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_SHLC(val);

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_SHRC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// SHRC g
	// Logical Shift Right with Carry Flag

	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_SHRC(val);

	set_reg8(opbyte0 & 0x7, val);
}

void tlcs870_device::do_ROLC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// ROLC g
	// Rotate Left through Carry flag
	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_ROLC(val);

	set_reg8(opbyte0 & 0x7, val);

}

void tlcs870_device::do_RORC_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// RORC g
	// Rotate Right through Carry flag
	uint8_t val = get_reg8(opbyte0 & 0x7);

	val = handle_RORC(val);

	set_reg8(opbyte0 & 0x7, val);
}


void tlcs870_device::do_LD_r_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD r,g
	// 1110 1ggg 0101 1rrr
	const uint8_t val = get_reg8(opbyte0 & 0x7);
	set_reg8(opbyte1 & 0x7, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_XCH_r_g(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// XCH r,g
	// 1110 1ggg 1010 1rrr
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
	// (ALU OP) A,g
	/*
	1110 1ggg 0110 0000 ADDC A,g
	1110 1ggg 0110 0001 ADD A,g
	1110 1ggg 0110 0010 SUBB A,g
	1110 1ggg 0110 0011 SUB A,g
	1110 1ggg 0110 0100 AND A,g
	1110 1ggg 0110 0101 XOR A,g
	1110 1ggg 0110 0110 OR A,g
	1110 1ggg 0110 0111 CMP A,g
	*/

	const int aluop = (opbyte1 & 0x7);

	const uint8_t result = do_alu(aluop, get_reg8(REG_A), get_reg8(opbyte0 & 0x7));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(REG_A, result);
	}
}

void tlcs870_device::do_ALUOP_g_A(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// (ALU OP) g,A
	/*
	1110 1ggg 0110 1000 ADDC A,g
	1110 1ggg 0110 1001 ADD A,g
	1110 1ggg 0110 1010 SUBB A,g
	1110 1ggg 0110 1011 SUB A,g
	1110 1ggg 0110 1100 AND A,g
	1110 1ggg 0110 1101 XOR A,g
	1110 1ggg 0110 1110 OR A,g
	1110 1ggg 0110 1111 CMP A,g
	*/

	const int aluop = (opbyte1 & 0x7);
	const uint8_t result = do_alu(aluop, get_reg8(opbyte0 & 0x7), get_reg8(REG_A));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(opbyte0 & 0x7, result);
	}
}

void tlcs870_device::do_ALUOP_g_n(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// (ALU OP) g,n
	const int aluop = (opbyte1 & 0x7);

	const uint8_t n = READ8();

	const uint8_t result = do_alu(aluop, get_reg8(opbyte0 & 0x7), n);

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
	// (ALU OP) WA,gg
	/*
	1110 10gg 0011 0000  ADDC WA,gg
	1110 10gg 0011 0001  ADD WA,gg
	1110 10gg 0011 0010  SUBB WA,gg
	1110 10gg 0011 0011  SUB WA,gg
	1110 10gg 0011 0100  AND WA,gg
	1110 10gg 0011 0101  XOR WA,gg
	1110 10gg 0011 0110  OR WA,gg
	1110 10gg 0011 0111  CMP WA,gg
	*/

	const int aluop = (opbyte1 & 0x7);

	const uint16_t result = do_alu(aluop, get_reg16(REG_WA), get_reg16(opbyte0 & 0x3));

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg16(REG_WA, result);
	}

}

void tlcs870_device::do_ALUOP_gg_mn(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// (ALU OP) gg,mn
	const int aluop = (opbyte1 & 0x7);

	const uint16_t mn = READ16();

	const uint16_t result = do_alu(aluop, get_reg16(opbyte0 & 0x3), mn);

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
	// SET (pp).g
	const uint8_t bitpos = opbyte0 & 7;
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
	// CLR (pp).g
	const uint8_t bitpos = opbyte0 & 7;
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

	val &= ~bitused;

	WM8(addr, val);
}

void tlcs870_device::do_CPL_inppbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// CPL (pp).g
	const uint8_t bitpos = opbyte0 & 7;
	const uint8_t bitused = 1 << bitpos;
	const uint16_t addr = get_reg16((opbyte1 & 1) + 2); // DE or HL
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

void tlcs870_device::do_LD_inppbit_CF(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD (pp).g,CF
	const uint8_t bitpos = opbyte0 & 7;
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

void tlcs870_device::do_LD_CF_inppbit(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD CF,(pp).g   aka TEST (pp).g
	// 1110 1ggg 1001 111p
	const uint8_t bitpos = opbyte0 & 7;
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
	// SET g.b
	// 1110 1ggg 0100 0bbb
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
	// CLR g.b
	// 1110 1ggg 0100 1bbb
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
	// CPL g.b
	// 1110 1ggg 1100 0bbb
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
	// LD g.b,CF
	// m_op = LD;    // Flags / Cycles  1--- / 2
	//m_flagsaffected |= FLAG_J;
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
	// XOR CF,g.b
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
	// LD CF,g.b aka TEST g.b
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
	// MUL ggH, ggL (odd syntax, basically MUL gg)
	const uint8_t reg = opbyte0 & 0x3; // opbyte0 & 4 = invalid?
	handle_mul(reg);  // flag changes in handler
}

void tlcs870_device::do_DIV_gg_C(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// DIV gg,C
	// (DIV BC,C is presumably an illegal / undefined result)
	// (DIV WA,C is a redundant encoding)
	const uint8_t reg = opbyte0 & 0x3; // opbyte0 & 4 = invalid?
	handle_div(reg); // flag changes in handler
}

void tlcs870_device::do_POP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// POP gg
	m_sp.d += 2;
	const uint16_t val = RM16(m_sp.d - 1);
	set_reg16(opbyte0 & 3, val);
	// no flag changes
}

void tlcs870_device::do_PUSH_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// PUSH gg
	const uint16_t val = get_reg16(opbyte0 & 3);
	WM16(m_sp.d - 1, val);
	m_sp.d -= 2;
	// no flag changes
}


void tlcs870_device::do_LD_SP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD SP,gg
	m_sp.d = get_reg16(opbyte0 & 0x3);
	set_JF(); // no other flag changes for this type of LD
}

void tlcs870_device::do_LD_gg_SP(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD gg,SP
	set_reg16(opbyte0 & 0x3, m_sp.d);
	set_JF(); // no other flag changes for this type of LD
}

void tlcs870_device::do_LD_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// LD rr,gg
	const uint16_t gg = get_reg16(opbyte0 & 0x3);
	set_reg16(opbyte1 & 0x3, gg);

	set_JF(); // no other flag changes for this type of LD
}


void tlcs870_device::do_XCH_rr_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// XCH rr,gg
	const uint16_t gg = get_reg16(opbyte0 & 0x3);
	const uint16_t rr = get_reg16(opbyte1 & 0x3);

	set_reg16(opbyte1 & 0x3, gg);
	set_reg16(opbyte0 & 0x3, rr);

	// flags not done
}


void tlcs870_device::do_CALL_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// CALL gg
	const uint16_t val = get_reg16(opbyte0 & 3);

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = val;

	// no flag changes on call
}

void tlcs870_device::do_JP_gg(const uint8_t opbyte0, const uint8_t opbyte1)
{
	// JP gg
	const uint16_t val = get_reg16(opbyte0 & 3);

	m_addr = val;
	set_JF();
}
