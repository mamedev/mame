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
	E5  (HL+)
	E6  (-HL)

	note, in cases where the address is an immediate value, not a register (x) and (HL+d) the
	immediate value is directly after the first byte of the opcode

*************************************************************************************************************/

#include "emu.h"
#include "tlcs870.h"
#include "tlcs870d.h"
#include "debugger.h"

// Main dispatch handlers for these

void tlcs870_device::decode()
{
	const uint8_t opbyte0 = READ8();

	switch (opbyte0)
	{
	case 0x00:
		do_NOP(opbyte0); break;
	case 0x01:
		do_SWAP_A(opbyte0); break;
	case 0x02:
		do_MUL_W_A(opbyte0); break;
	case 0x03:
		do_DIV_WA_C(opbyte0); break;
	case 0x04:
		do_RETI(opbyte0); break;
	case 0x05:
		do_RET(opbyte0); break;
	case 0x06:
		do_POP_PSW(opbyte0); break;
	case 0x07:
		do_PUSH_PSW(opbyte0); break;
	case 0x0a:
		do_DAA_A(opbyte0); break;
	case 0x0b:
		do_DAS_A(opbyte0); break;
	case 0x0c:
		do_CLR_CF(opbyte0); break;
	case 0x0d:
		do_SET_CF(opbyte0); break;
	case 0x0e:
		do_CPL_CF(opbyte0); break;
	case 0x0f:
		do_LD_RBS_n(opbyte0); break;
	case 0x10: case 0x11: case 0x12: case 0x13:
		do_INC_rr(opbyte0); break;
	case 0x14: case 0x15: case 0x16: case 0x17:
		do_LD_rr_mn(opbyte0); break;
	case 0x18: case 0x19: case 0x1a: case 0x1b:
		do_DEC_rr(opbyte0); break;
	case 0x1c:
		do_SHLC_A(opbyte0); break;
	case 0x1d:
		do_SHRC_A(opbyte0); break;
	case 0x1e:
		do_ROLC_A(opbyte0); break;
	case 0x1f:
		do_RORC_A(opbyte0); break;
	case 0x20:
		do_INC_inx(opbyte0); break;
	case 0x21:
		do_INC_inHL(opbyte0); break;
	case 0x22:
		do_LD_A_inx(opbyte0); break;
	case 0x23:
		do_LD_A_inHL(opbyte0); break;
	case 0x24:
		do_LDW_inx_mn(opbyte0); break;
	case 0x25:
		do_LDW_inHL_mn(opbyte0); break;
	case 0x26:
		do_LD_inx_iny(opbyte0); break;
	case 0x28:
		do_DEC_inx(opbyte0); break;
	case 0x29:
		do_DEC_inHL(opbyte0); break;
	case 0x2a:
		do_LD_inx_A(opbyte0); break;
	case 0x2b:
		do_LD_inHL_A(opbyte0); break;
	case 0x2c:
		do_LD_inx_n(opbyte0); break;
	case 0x2d:
		do_LD_inHL_n(opbyte0); break;
	case 0x2e:
		do_CLR_inx(opbyte0); break;
	case 0x2f:
		do_CLR_inHL(opbyte0); break;
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		do_LD_r_n(opbyte0); break;
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
		do_SET_inxbit(opbyte0); break;
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		do_CLR_inxbit(opbyte0); break;
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		do_LD_A_r(opbyte0); break;
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		do_LD_r_A(opbyte0); break;
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		do_INC_r(opbyte0); break;
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		do_DEC_r(opbyte0); break;
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		do_ALUOP_A_n(opbyte0); break;
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		do_ALUOP_A_inx(opbyte0); break;
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		do_JRS_T_a(opbyte0); break;
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7: case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7: case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		do_JRS_F_a(opbyte0); break;
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		do_CALLV_n(opbyte0); break;
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
		do_JR_cc_a(opbyte0); break;
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		do_LD_CF_inxbit(opbyte0); break;
	case 0xe0:
		do_e0_opcode(opbyte0); break;
	case 0xe1: case 0xe2: case 0xe3:
		do_e1_to_e3_opcode(opbyte0); break;
	case 0xe4:
		do_e4_opcode(opbyte0); break;
	case 0xe5: case 0xe6: case 0xe7:
		do_e5_to_e7_opcode(opbyte0); break;
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		do_regprefixtype_opcode(opbyte0); break;
	case 0xf0:
		do_f0_opcode(opbyte0); break;
	case 0xf2: case 0xf3:
		do_f2_to_f3_opcode(opbyte0); break;
	case 0xf4:
		do_f4_opcode(opbyte0); break;
	case 0xf6: case 0xf7:
		do_f6_to_f7_opcode(opbyte0); break;
	case 0xfa:
		do_LD_SP_mn(opbyte0); break;
	case 0xfb:
		do_JR_a(opbyte0); break;
	case 0xfc:
		do_CALL_mn(opbyte0); break;
	case 0xfd:
		do_CALLP_n(opbyte0); break;
	case 0xfe:
		do_JP_mn(opbyte0); break;
	case 0xff:
		do_ff_opcode(opbyte0); break;

	default:
		do_illegal(opbyte0); break;
	}
}


void tlcs870_device::do_illegal(const uint8_t opbyte0)
{
	logerror("illegal opcode %02x\n", opbyte0);
}

void tlcs870_device::do_NOP(const uint8_t opbyte0)
{
	// NOP
}

void tlcs870_device::do_SWAP_A(const uint8_t opbyte0)
{
	// SWAP A
	handle_swap(REG_A);
}

void tlcs870_device::do_MUL_W_A(const uint8_t opbyte0)
{
	// MUL W,A
	handle_mul(REG_WA);
}

void tlcs870_device::do_DIV_WA_C(const uint8_t opbyte0)
{
	// DIV WA,C
	handle_div(REG_WA);
}

void tlcs870_device::do_RETI(const uint8_t opbyte0)
{
	// RETI
	// Return from maskable interrupt service (how does this differ from RETN?)
	m_sp.d += 3;
	m_addr = RM16(m_sp.d - 2);
	set_PSW(RM8(m_sp.d - 1));
}

void tlcs870_device::do_RET(const uint8_t opbyte0)
{
	// RET
	m_sp.d += 2;
	m_addr = RM16(m_sp.d - 1);
};

void tlcs870_device::do_POP_PSW(const uint8_t opbyte0)
{
	// POP PSW
	m_sp.d += 2;
	const uint16_t val = RM16(m_sp.d - 1);
	set_PSW(val);
}

void tlcs870_device::do_PUSH_PSW(const uint8_t opbyte0)
{
	// PUSH PSW:
	const uint16_t val = get_PSW();
	WM16(m_sp.d - 1, val);
	m_sp.d -= 2;
}

void tlcs870_device::do_DAA_A(const uint8_t opbyte0)
{
	// DAA A
	uint8_t val = get_reg8(REG_A);

	val = handle_DAA(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_DAS_A(const uint8_t opbyte0)
{
	// DAS A
	uint8_t val = get_reg8(REG_A);

	val = handle_DAS(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_CLR_CF(const uint8_t opbyte0)
{
	// CLR CF
	clear_CF();
	set_JF();
}

void tlcs870_device::do_SET_CF(const uint8_t opbyte0)
{
	// SET CF
	set_CF();
	clear_JF();
}

void tlcs870_device::do_CPL_CF(const uint8_t opbyte0)
{
	// CPL CF
	if (is_CF())
	{
		set_JF();
		clear_CF();
	}
	else
	{
		clear_JF();
		set_CF();
	}
}

void tlcs870_device::do_LD_RBS_n(const uint8_t opbyte0) // register bank switching
{
	// LD RBS,n
	const uint8_t param = READ8();
	m_RBS = param & 0x0f;
	set_JF();
}


void tlcs870_device::do_INC_rr(const uint8_t opbyte0)
{
	// INC rr
	const int reg = opbyte0 & 3;
	uint16_t temp = get_reg16(reg);
	temp++;
	set_reg16(reg, temp);

	if (temp == 0x0000)
	{
		set_ZF();
		set_JF();
	}
	else
	{
		// do we clear?
		clear_ZF();
		clear_JF();
	}
}


void tlcs870_device::do_LD_rr_mn(const uint8_t opbyte0)
{
	// LD rr,mn
	const uint16_t val = READ16(); // 16-bit
	set_reg16(opbyte0 & 3, val);
	set_JF();
}


void tlcs870_device::do_DEC_rr(const uint8_t opbyte0)
{
	// DEC rr
	const int reg = opbyte0 & 3;
	uint16_t temp = get_reg16(reg);
	temp--;
	set_reg16(reg, temp);

	if (temp == 0xffff)
	{
		set_JF();
	}
	else
	{
		// do we clear?
		clear_JF();
	}

	if (temp == 0x0000) // check
	{
		set_ZF();
	}
	else
	{
		set_ZF();
	}
}


void tlcs870_device::do_SHLC_A(const uint8_t opbyte0)
{
	// SHLC A
	uint8_t val = get_reg8(REG_A);

	val = handle_SHLC(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_SHRC_A(const uint8_t opbyte0)
{
	// SHRC A
	uint8_t val = get_reg8(REG_A);

	val = handle_SHRC(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_ROLC_A(const uint8_t opbyte0)
{
	// ROLC A
	uint8_t val = get_reg8(REG_A);

	val = handle_ROLC(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_RORC_A(const uint8_t opbyte0)
{
	// RORC A
	uint8_t val = get_reg8(REG_A);

	val = handle_RORC(val);

	set_reg8(REG_A, val);
}

void tlcs870_device::do_INC_inx(const uint8_t opbyte0)
{
	// INC (x)
	const uint16_t srcaddr = READ8();
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

	// WRITE
	WM8(srcaddr, val);
}

void tlcs870_device::do_INC_inHL(const uint8_t opbyte0)
{
	// INC (HL)
	const uint16_t addr = get_reg16(REG_HL);
	uint8_t val = RM8(addr);

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

	WM8(addr, val);
}

void tlcs870_device::do_LD_A_inx(const uint8_t opbyte0)
{
	// LD A,(x)
	const uint16_t srcaddr = READ8();
	const uint8_t val = RM8(srcaddr);

	set_reg8(REG_A, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_LD_A_inHL(const uint8_t opbyte0)
{
	// LD A,(HL)
	const uint16_t srcaddr = get_reg16(REG_HL);
	const uint8_t val = RM8(srcaddr);

	set_reg8(REG_A, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_LD_inx_iny(const uint8_t opbyte0)
{
	// LD (x),(y)
	const uint16_t srcaddr = READ8();
	const uint16_t dstaddr = READ8();

	const uint8_t val = RM8(srcaddr);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();

	WM8(dstaddr, val);
}

void tlcs870_device::do_DEC_inx(const uint8_t opbyte0)
{
	// DEC (x)
	const uint16_t addr = READ8();

	uint8_t temp = RM8(addr);
	temp--;
	WM8(addr, temp);

	if (temp == 0xff)
	{
		set_JF();
	}
	else
	{
		// do we clear?
		clear_JF();
	}

	if (temp == 0x00)
	{
		set_ZF();
	}
	else
	{
		set_ZF();
	}
}

void tlcs870_device::do_DEC_inHL(const uint8_t opbyte0)
{
	// DEC (HL)
	const uint16_t addr = get_reg16(REG_HL);

	uint8_t temp = RM8(addr);
	temp--;
	WM8(addr, temp);

	if (temp == 0xff)
	{
		set_JF();
	}
	else
	{
		// do we clear?
		clear_JF();
	}

	if (temp == 0x00)
	{
		set_ZF();
	}
	else
	{
		set_ZF();
	}
}

void tlcs870_device::do_LD_inx_A(const uint8_t opbyte0)
{
	// LD (x),A
	const uint16_t dstaddr = READ8();
	const uint8_t val = get_reg8(REG_A);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();

	WM8(dstaddr, val);
}

void tlcs870_device::do_LD_inHL_A(const uint8_t opbyte0)
{
	// LD (HL),A
	const uint8_t val = get_reg8(REG_A);

	set_JF();

	const uint16_t addr = get_reg16(REG_HL);
	WM8(addr, val);
}

void tlcs870_device::do_LD_inx_n(const uint8_t opbyte0)
{
	// LD (x),n
	const uint16_t dstaddr = READ8();
	const uint8_t val = READ8();

	set_JF();

	WM8(dstaddr, val);
}

void tlcs870_device::do_LD_inHL_n(const uint8_t opbyte0)
{
	// LD (HL),n
	const uint8_t val = READ8();
	const uint16_t addr = get_reg16(REG_HL);

	set_JF();

	WM8(addr, val);
}

void tlcs870_device::do_CLR_inx(const uint8_t opbyte0)
{
	// CLR (x)
	const uint16_t addr = READ8();

	WM8(addr, 0);

	set_JF();
}

void tlcs870_device::do_CLR_inHL(const uint8_t opbyte0)
{
	// CLR (HL)
	const uint16_t addr = get_reg16(REG_HL);
	WM8(addr, 0);

	set_JF();
}

void tlcs870_device::do_LD_r_n(const uint8_t opbyte0)
{
	// LD r,n
	const uint8_t param1 = opbyte0 & 7;
	const uint8_t param2 = READ8();

	set_reg8(param1, param2);

	set_JF();
}

void tlcs870_device::do_SET_inxbit(const uint8_t opbyte0)
{
	// SET (x).b
	// 0100 0bbb xxxx xxxx

	// (opbyte0 == 0x40) && (opval == 0x3a) is EI
	const uint8_t srcaddr = READ8();

	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte0 & 0x7;

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

void tlcs870_device::do_CLR_inxbit(const uint8_t opbyte0)
{
	// CLR (x).b
	// 0100 1bbb xxxx xxxx

	// (opbyte0 == 0x48) && (opval == 0x3a) is DI
	const uint8_t srcaddr = READ8();

	uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte0 & 0x7;

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

void tlcs870_device::do_LD_A_r(const uint8_t opbyte0)
{
	// LD A,r
	// 0101 0rrr
	const uint8_t val = get_reg8(opbyte0 & 0x7);
	set_reg8(REG_A, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_LD_r_A(const uint8_t opbyte0)
{
	// LD r,A
	// 0101 1rrr
	const uint8_t val = get_reg8(REG_A);
	set_reg8(opbyte0 & 0x7, val);

	set_JF();

	if (val == 0x00) set_ZF();
	else clear_ZF();
}

void tlcs870_device::do_INC_r(const uint8_t opbyte0)
{
	// INC r
	const int reg = opbyte0 & 7;
	uint8_t temp = get_reg8(reg);
	temp++;
	set_reg8(reg, temp);

	if (temp == 0x00)
	{
		set_ZF();
		set_JF();
	}
	else
	{
		// do we clear?
		clear_ZF();
		clear_JF();
	}
}

void tlcs870_device::do_DEC_r(const uint8_t opbyte0)
{
	// DEC r
	const int reg = opbyte0 & 7;
	uint8_t temp = get_reg8(reg);
	temp--;
	set_reg8(reg, temp);

	if (temp == 0xff)
	{
		set_JF();
	}
	else
	{
		// do we clear?
		clear_JF();
	}

	if (temp == 0x00)
	{
		set_ZF();
	}
	else
	{
		set_ZF();
	}
}

void tlcs870_device::do_JRS_T_a(const uint8_t opbyte0)
{
	// JRS T,a
	const int param1 = 6;

	int val = opbyte0 & 0x1f;
	if (val & 0x10) val -= 0x20;

	const bool takejump = check_jump_condition(param1);

	if (takejump)
	{
		m_addr = m_tmppc + 2 + val;
		set_JF();
	}
}

void tlcs870_device::do_JRS_F_a(const uint8_t opbyte0)
{
	// JRS F,a
	const int param1 = 7;

	int val = opbyte0 & 0x1f;
	if (val & 0x10) val -= 0x20;

	const bool takejump = check_jump_condition(param1);

	if (takejump)
	{
		m_addr = m_tmppc + 2 + val;
		set_JF();
	}
}

void tlcs870_device::do_CALLV_n(const uint8_t opbyte0)
{
	// CALLV n
	const uint16_t addr = 0xffc0 + ((opbyte0 & 0xf) * 2);

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = addr;
}

void tlcs870_device::do_JR_cc_a(const uint8_t opbyte0)
{
	// JR cc,a
	const int param1 = opbyte0 & 0x7;

	int val = READ8();
	if (val & 0x80) val -= 0x100;

	bool takejump = check_jump_condition(param1);

	if (takejump)
	{
		m_addr = m_tmppc + 2 + val;
		set_JF();
	}
}

void tlcs870_device::do_LD_CF_inxbit(const uint8_t opbyte0)
{
	// LD CF, (x).b  aka TEST (x).b
	const uint16_t srcaddr = READ8();
	const uint8_t val = RM8(srcaddr);
	const uint8_t bitpos = opbyte0 & 0x7;

	const int bitused = (1 << bitpos);

	uint8_t bit = val & bitused;

	bit ? set_CF() : clear_CF();
	// for this optype of operation ( LD CF, *.b ) the Jump Flag always ends up the inverse of the Carry Flag
	bit ? clear_JF() : set_JF();
}

void tlcs870_device::do_LD_SP_mn(const uint8_t opbyte0)
{
	// LD SP,mn
	// Flags / Cycles  1--- / 3
	const uint16_t param = READ16();
	m_sp.d = param;
	set_JF();
}

void tlcs870_device::do_JR_a(const uint8_t opbyte0)
{
	// JR a
	int val = READ8();
	if (val & 0x80) val -= 0x100;

	m_addr = m_tmppc + 2 + val;;

	set_JF();
}

void tlcs870_device::do_CALL_mn(const uint8_t opbyte0)
{
	// CALL mn
	const uint16_t addr = READ16();

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = addr;
}

void tlcs870_device::do_CALLP_n(const uint8_t opbyte0)
{
	// CALLP n
	const uint16_t addr = READ8() + 0xff00;

	WM16(m_sp.d - 1, m_addr);
	m_sp.d -= 2;

	m_addr = addr;
}

void tlcs870_device::do_JP_mn(const uint8_t opbyte0)
{
	// JP mn
	const int param2 = READ16();
	m_addr = param2;
	set_JF();
}

void tlcs870_device::do_ff_opcode(const uint8_t opbyte0)
{
	// SWI
	handle_take_interrupt(0x0e);
}

/**********************************************************************************************************************/
// ALU Operations
/**********************************************************************************************************************/

void tlcs870_device::do_ALUOP_A_n(const uint8_t opbyte0)
{
	// (ALU OP) A,n
	// 0111 0000 nnnn nnnn  ADDC A,n
	// 0111 0001 nnnn nnnn  ADD A,n

	const int aluop = (opbyte0 & 0x7);
	const uint8_t val = READ8();

	const uint8_t result = do_alu(aluop, get_reg8(REG_A), val);

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(REG_A, result);
	}
}

void tlcs870_device::do_ALUOP_A_inx(const uint8_t opbyte0)
{
	// (ALU OP) A,(x)
	// 0111 1000 nnnn nnnn  ADDC A,(x)
	// 0111 1001 nnnn nnnn  ADD A,(x)

	const int aluop = (opbyte0 & 0x7);
	const uint16_t addr = READ8();
	const uint8_t val = RM8(addr);

	const uint8_t result = do_alu(aluop, get_reg8(REG_A), val);

	if (aluop != 0x07) // CMP doesn't write back
	{
		set_reg8(REG_A, result);
	}
}

/**********************************************************************************************************************/
// 16-bit loads
/**********************************************************************************************************************/

void tlcs870_device::do_LDW_inx_mn(const uint8_t opbyte0)
{
	// LDW (x),mn
	const uint16_t dstaddr = READ8();

	const uint16_t val = READ16();

	WM16(dstaddr, val);

	set_JF(); // only JF changes
}

void tlcs870_device::do_LDW_inHL_mn(const uint8_t opbyte0)
{
	// LDW (HL),mn
	// m_op = LDW;
	const uint16_t dstaddr = get_reg16(REG_HL);

	const uint16_t val = READ16();

	WM16(dstaddr, val);

	set_JF(); // only JF changes
}
