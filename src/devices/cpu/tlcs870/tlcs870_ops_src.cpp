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
#include "debugger.h"

// Main dispatch handlers for these

void tlcs870_device::do_e0_opcode(const uint8_t opbyte0)
{
	const uint16_t srcaddr = get_addr((opbyte0 & 0x7), READ8());
	do_e0_to_e7_opcode(opbyte0, srcaddr);
}

void tlcs870_device::do_e1_to_e3_opcode(const uint8_t opbyte0)
{
	const uint16_t srcaddr = get_addr((opbyte0 & 0x7), 0);
	do_e0_to_e7_opcode(opbyte0, srcaddr);
}

void tlcs870_device::do_e4_opcode(const uint8_t opbyte0)
{
	const uint16_t srcaddr = get_addr((opbyte0 & 0x7), READ8());
	do_e0_to_e7_opcode(opbyte0, srcaddr);
}

void tlcs870_device::do_e5_to_e7_opcode(const uint8_t opbyte0)
{
	const uint16_t srcaddr = get_addr((opbyte0 & 0x7), 0);
	do_e0_to_e7_opcode(opbyte0, srcaddr);
}

// e0 - e7 use this table
void tlcs870_device::do_e0_to_e7_opcode(uint8_t opbyte0, uint16_t srcaddr)
{
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
	logerror("illegal src prefix opcode %02x %02x (src addr %04x)\n", opbyte0, opbyte1, srcaddr);
}

/**********************************************************************************************************************/
// (16-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_LD_rr_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// LD rr, (src)
	const uint16_t val = RM16(srcaddr);
	set_reg16(opbyte1 & 0x3, val);
	set_JF();
}

/**********************************************************************************************************************/
// (8-bit)
/**********************************************************************************************************************/

void tlcs870_device::do_INC_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// INC (src)
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
	// DEC (src)
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
		set_ZF();
	}

	WM8(srcaddr, val);
}

void tlcs870_device::do_ROLD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// ROLD A,(src)
	// 12-bit left rotation using lower 4 bits of REG_A and content of (src)
	const uint8_t val = RM8(srcaddr);
	const uint8_t reg = get_reg8(REG_A);

	uint8_t tempval = (val & 0x0f) << 4;
	tempval |= reg & 0x0f;
	const uint8_t tempa = (reg & 0xf0) | (val & 0xf0) >> 4;

	set_reg8(REG_A, tempa);
	WM8(srcaddr, tempval);

	// TODO: flags
}

void tlcs870_device::do_RORD_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// RORD A,(src)
	// 12-bit right rotation using lower 4 bits of REG_A and content of (src)
	const uint8_t val = RM8(srcaddr);
	const uint8_t reg = get_reg8(REG_A);

	uint8_t tempval = (val & 0xf0) >> 4;
	tempval |= ((reg & 0x0f) << 4);
	const uint8_t tempa = (reg & 0xf0) | (val & 0x0f);

	set_reg8(REG_A, tempa);
	WM8(srcaddr, tempval);

	// TODO: flags
}



void tlcs870_device::do_LD_inx_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// LD (x),(src)
	// invalid if (src) is also (x) ? (not specified)
	const uint16_t x = READ8(); // get address x
	const uint8_t val = RM8(srcaddr);
	WM8(x, val);

	set_JF();
	// z-flag is undefined, check real behavior
}

void tlcs870_device::do_LD_inHL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// LD (HL),(src)
	const uint8_t val = RM8(srcaddr);

	const uint16_t dstaddr = get_reg16(REG_HL);

	WM8(dstaddr, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_LD_r_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// LD r, (src)
	const uint8_t val = RM8(srcaddr);

	set_reg8(opbyte1 & 0x7, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_MCMP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// MCMP (src), n
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
	// XCH r,(src)
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
	// (ALU OP) (src), (HL)
	const int aluop = (opbyte1 & 0x7);
	const uint8_t val = RM8(srcaddr);

	const uint16_t HL = get_reg16(REG_HL);

	const uint8_t result = do_alu(aluop, val, RM8(HL));

	if (aluop != 0x07) // CMP doesn't write back
	{
		WM8(srcaddr, result);
	}
}

void tlcs870_device::do_ALUOP_insrc_n(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// (ALU OP) (src), n
	const uint8_t n = READ8();

	const int aluop = (opbyte1 & 0x7);
	const uint8_t val = RM8(srcaddr);

	const uint8_t result = do_alu(aluop, val, n);

	if (aluop != 0x07) // CMP doesn't write back
	{
		WM8(srcaddr, result);
	}
}

void tlcs870_device::do_ALUOP_A_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// (ALU OP) A, (src)
	const int aluop = (opbyte1 & 0x7);
	const uint8_t val = RM8(srcaddr);

	const uint8_t result = do_alu(aluop, get_reg8(REG_A), val);

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(REG_A, result);
	}
}

/**********************************************************************************************************************/
// jumps / calls
/**********************************************************************************************************************/

void tlcs870_device::do_CALL_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// CALL (src)
	const uint16_t val = RM16(srcaddr);

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = val;

	// no flag changes on call
}

void tlcs870_device::do_JP_insrc(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// JP (src)
	const uint16_t val = RM16(srcaddr);
	m_addr = val;
	set_JF();
}

/**********************************************************************************************************************/
// (8-bit) bit operations
/**********************************************************************************************************************/

void tlcs870_device::do_XOR_CF_insrcbit(const uint8_t opbyte0, const uint8_t opbyte1, const uint16_t srcaddr)
{
	// XOR CF,(src).b
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
	// LD (src).b,CF
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
	// CPL (src).b
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
	// LD CF,(src).b  aka  TEST (src).b
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
	// SET (src).b
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
	// CLR (src).b
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

