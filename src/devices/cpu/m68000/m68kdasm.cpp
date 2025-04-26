// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.32
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright Karl Stenerud.
 *
 */



/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include "emu.h"
#include "m68kdasm.h"

std::string m68k_disassembler::make_signed_hex_str_8(u8 val)
{
	if(val == 0x80)
		return "-$80";
	else if(val & 0x80)
		return util::string_format("-$%x", (-val) & 0x7f);
	else
		return util::string_format("$%x", val & 0x7f);
}

std::string m68k_disassembler::make_signed_hex_str_16(u16 val)
{
	if(val == 0x8000)
		return "-$8000";
	else if(val & 0x8000)
		return util::string_format("-$%x", (-val) & 0x7fff);
	else
		return util::string_format("$%x", val & 0x7fff);
}

std::string m68k_disassembler::make_signed_hex_str_32(u32 val)
{
	if(val == 0x80000000)
		return "-$80000000";
	else if(val & 0x80000000)
		return util::string_format("-$%x", (-val) & 0x7fffffff);
	else
		return util::string_format("$%x", val & 0x7fffffff);
}

std::string m68k_disassembler::get_imm_str_s(u32 size)
{
	switch(size)
	{
	case 0: return util::string_format("#%s", make_signed_hex_str_8(read_imm_8()));
	case 1: return util::string_format("#%s", make_signed_hex_str_16(read_imm_16()));
	case 2: return util::string_format("#%s", make_signed_hex_str_32(read_imm_32()));
	default: abort();
	}
}

std::string m68k_disassembler::get_imm_str_u(u32 size)
{
	switch(size)
	{
	case 0: return util::string_format("#$%x", read_imm_8());
	case 1: return util::string_format("#$%x", read_imm_16());
	case 2: return util::string_format("#$%x", read_imm_32());
	default: abort();
	}
}

std::string m68k_disassembler::get_ea_mode_str(u16 instruction, u32 size)
{
	switch(instruction & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		/* data register direct */
			return util::string_format("D%d", instruction&7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		/* address register direct */
			return util::string_format("A%d", instruction&7);
			break;
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		/* address register indirect */
			return util::string_format("(A%d)", instruction&7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		/* address register indirect with postincrement */
			return util::string_format("(A%d)+", instruction&7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		/* address register indirect with predecrement */
			return util::string_format("-(A%d)", instruction&7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		/* address register indirect with displacement*/
			return util::string_format("(%s,A%d)", make_signed_hex_str_16(read_imm_16()), instruction&7);
			break;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		{
		/* address register indirect with index */
			u16 extension = read_imm_16();

			if((m_cpu_type & M68010_LESS) && ext_index_scale(extension))
				break;

			if(ext_full(extension))
			{
				if(m_cpu_type & M68010_LESS)
					break;

				if(ext_effective_zero(extension))
					return "0";

				u32 base = ext_base_displacement_present(extension) ? (ext_base_displacement_long(extension) ? read_imm_32() : read_imm_16()) : 0;
				u32 outer = ext_outer_displacement_present(extension) ? (ext_outer_displacement_long(extension) ? read_imm_32() : read_imm_16()) : 0;
				std::string base_reg = ext_base_register_present(extension) ? util::string_format("A%d", instruction&7) : "";
				std::string index_reg = ext_index_register_present(extension) ?
					util::string_format("%c%d.%c%s", ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w',
										ext_index_scale(extension) ? util::string_format("*%d", 1 << ext_index_scale(extension)) : "")
					: "";

				bool preindex = (extension&7) > 0 && (extension&7) < 4;
				bool postindex = (extension&7) > 4;

				std::string mode = "(";
				if(preindex || postindex)
					mode += '[';
				bool comma = false;
				if(base)
				{
					if(ext_base_displacement_long(extension))
						mode += make_signed_hex_str_32(base);
					else
						mode += make_signed_hex_str_16(base);
					comma = true;
				}
				if(!base_reg.empty())
				{
					if(comma)
						mode += ',';
					mode += base_reg;
					comma = true;
				}
				if(postindex)
				{
					mode += ']';
					comma = true;
				}
				if(!index_reg.empty())
				{
					if(comma)
						mode += ',';
					mode += index_reg;
					comma = true;
				}
				if(preindex)
				{
					mode += ']';
				}
				if(outer)
				{
					if(comma)
						mode += ',';
					mode += make_signed_hex_str_16(outer);
				}
				mode += ')';
				return mode;
			}

			std::string mode = ext_8bit_displacement(extension) ?
				util::string_format("(%s,A%d,%c%d.%c", make_signed_hex_str_8(extension), instruction&7, ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w') :
				util::string_format("(A%d,%c%d.%c", instruction&7, ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w');
			if(ext_index_scale(extension))
				mode += util::string_format("*%d", 1 << ext_index_scale(extension));
			mode += ')';
			return mode;
		}
		case 0x38:
		/* absolute short address */
			return util::string_format("$%x.w", read_imm_16());
			break;
		case 0x39:
		/* absolute long address */
			return util::string_format("$%x.l", read_imm_32());
			break;
		case 0x3a:
		{
		/* program counter with displacement */
			u16 temp_value = read_imm_16();
			return util::string_format("(%s,PC) ; ($%x)", make_signed_hex_str_16(temp_value),
									   (make_int_16(temp_value) + m_cpu_pc-2) & 0xffffffff);
		}
		case 0x3b:
		{
		/* program counter with index */
			u16 extension = read_imm_16();

			if((m_cpu_type & M68010_LESS) && ext_index_scale(extension))
				break;

			if(ext_full(extension))
			{
				if(m_cpu_type & M68010_LESS)
					break;

				if(ext_effective_zero(extension))
					return "0";

				u32 base = ext_base_displacement_present(extension) ? (ext_base_displacement_long(extension) ? read_imm_32() : read_imm_16()) : 0;
				u32 outer = ext_outer_displacement_present(extension) ? (ext_outer_displacement_long(extension) ? read_imm_32() : read_imm_16()) : 0;
				std::string base_reg = ext_base_register_present(extension) ? "PC" : "";
				std::string index_reg =
					ext_index_register_present(extension) ?
					util::string_format("%c%d.%c%s", ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w',
										ext_index_scale(extension) ?
										util::string_format("*%d", 1 << ext_index_scale(extension)) : "") : "";
				bool preindex = (extension&7) > 0 && (extension&7) < 4;
				bool postindex = (extension&7) > 4;

				bool comma = false;
				std::string mode = "(";
				if(preindex || postindex)
					mode += '[';
				if(base)
				{
					mode += make_signed_hex_str_16(base);
					comma = true;
				}
				if(!base_reg.empty())
				{
					if(comma)
						mode += ',';
					mode += base_reg;
					comma = true;
				}
				if(postindex)
				{
					mode += ']';
					comma = true;
				}
				if(!index_reg.empty())
				{
					if(comma)
						mode += ',';
					mode += index_reg;
					comma = true;
				}
				if(preindex)
				{
					mode += ']';
					comma = true;
				}
				if(outer)
				{
					if(comma)
						mode += ',';
					mode += make_signed_hex_str_16(outer);
				}
				mode += ')';
				return mode;
			}

			std::string mode = ext_8bit_displacement(extension) ?
				util::string_format("(%s,PC,%c%d.%c", make_signed_hex_str_8(extension), ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w') :
				util::string_format("(PC,%c%d.%c", ext_index_ar(extension) ? 'A' : 'D', ext_index_register(extension), ext_index_long(extension) ? 'l' : 'w');

			if(ext_index_scale(extension))
				mode += util::string_format("*%d", 1 << ext_index_scale(extension));
			mode += ')';
			return mode;
		}
		case 0x3c:
		/* Immediate */
			return get_imm_str_u(size);
	}

	return util::string_format("INVALID %x", instruction & 0x3f);
}

/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* used by ops like asr, ror, addq, etc */
const u32 m68k_disassembler::m_3bit_qdata_table[8] = {8, 1, 2, 3, 4, 5, 6, 7};

const u32 m68k_disassembler::m_5bit_data_table[32] =
{
	32,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

const char *const m68k_disassembler::m_cc[16] =
{
	"t", "f", "hi", "ls", "cc", "cs", "ne", "eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le"
};

const char *const m68k_disassembler::m_cpcc[64] =
{/* 000     001    010    011    100    101    110    111 */
	"f",    "eq",  "ogt", "oge", "olt", "ole", "ogl", "or",  /* 000 */
	"un",   "ueq", "ugt", "uge", "ult", "ule", "ne",  "t",   /* 001 */
	"sf",   "seq", "gt",  "ge",  "lt",  "le",  "gl",  "gle", /* 010 */
	"ngle", "ngl", "nle", "nlt", "nge", "ngt", "sne", "st",  /* 011 */
	"?",    "?",   "?",   "?",   "?",   "?",   "?",   "?",   /* 100 */
	"?",    "?",   "?",   "?",   "?",   "?",   "?",   "?",   /* 101 */
	"?",    "?",   "?",   "?",   "?",   "?",   "?",   "?",   /* 110 */
	"?",    "?",   "?",   "?",   "?",   "?",   "?",   "?"    /* 111 */
};

const char *const m68k_disassembler::m_mmuregs[8] =
{
	"tc", "drp", "srp", "crp", "cal", "val", "sccr", "acr"
};

const char *const m68k_disassembler::m_mmucond[16] =
{
	"bs", "bc", "ls", "lc", "ss", "sc", "as", "ac",
	"ws", "wc", "is", "ic", "gs", "gc", "cs", "cc"
};

std::pair<bool, std::string> m68k_disassembler::limit_cpu_types(u32 allowed)
{
	if(!(m_cpu_type & allowed))
	{
		if((m_cpu_ir & 0xf000) == 0xf000)
			return std::make_pair(true, d68000_1111());
		else
			return std::make_pair(true, d68000_illegal());
	}
	return std::make_pair(false, std::string());
}

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */
/* Instruction handler function names follow this convention:
 *
 * d68000_NAME_EXTENSIONS(void)
 * where NAME is the name of the opcode it handles and EXTENSIONS are any
 * extensions for special instances of that opcode.
 *
 * Examples:
 *   d68000_add_er_8(): add opcode, from effective address to register,
 *                      size = byte
 *
 *   d68000_asr_s_8(): arithmetic shift right, static count, size = byte
 *
 *
 * Common extensions:
 * 8   : size = byte
 * 16  : size = word
 * 32  : size = long
 * rr  : register to register
 * mm  : memory to memory
 * r   : register
 * s   : static
 * er  : effective address -> register
 * re  : register -> effective address
 * ea  : using effective address mode of operation
 * d   : data register direct
 * a   : address register direct
 * ai  : address register indirect
 * pi  : address register indirect with postincrement
 * pd  : address register indirect with predecrement
 * di  : address register indirect with displacement
 * ix  : address register indirect with index
 * aw  : absolute word
 * al  : absolute long
 */

std::string m68k_disassembler::d68000_illegal()
{
	return util::string_format("dc.w    $%04x; ILLEGAL", m_cpu_ir);
}

std::string m68k_disassembler::d68000_1010()
{
	return util::string_format("dc.w    $%04x; opcode 1010", m_cpu_ir);
}


std::string m68k_disassembler::d68000_1111()
{
	return util::string_format("dc.w    $%04x; opcode 1111", m_cpu_ir);
}


std::string m68k_disassembler::d68000_abcd_rr()
{
	return util::string_format("abcd    D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}


std::string m68k_disassembler::d68000_abcd_mm()
{
	return util::string_format("abcd    -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_add_er_8()
{
	return util::string_format("add.b   %s, D%d", get_ea_mode_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}


std::string m68k_disassembler::d68000_add_er_16()
{
	return util::string_format("add.w   %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_add_er_32()
{
	return util::string_format("add.l   %s, D%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_add_re_8()
{
	return util::string_format("add.b   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_add_re_16()
{
	return util::string_format("add.w   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_add_re_32()
{
	return util::string_format("add.l   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_adda_16()
{
	return util::string_format("adda.w  %s, A%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_adda_32()
{
	return util::string_format("adda.l  %s, A%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addi_8()
{
	std::string str = get_imm_str_s8();
	return util::string_format("addi.b  %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addi_16()
{
	std::string str = get_imm_str_s16();
	return util::string_format("addi.w  %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addi_32()
{
	std::string str = get_imm_str_s32();
	return util::string_format("addi.l  %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addq_8()
{
	return util::string_format("addq.b  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addq_16()
{
	return util::string_format("addq.w  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addq_32()
{
	return util::string_format("addq.l  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_addx_rr_8()
{
	return util::string_format("addx.b  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addx_rr_16()
{
	return util::string_format("addx.w  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addx_rr_32()
{
	return util::string_format("addx.l  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addx_mm_8()
{
	return util::string_format("addx.b  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addx_mm_16()
{
	return util::string_format("addx.w  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_addx_mm_32()
{
	return util::string_format("addx.l  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_and_er_8()
{
	return util::string_format("and.b   %s, D%d", get_ea_mode_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_and_er_16()
{
	return util::string_format("and.w   %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_and_er_32()
{
	return util::string_format("and.l   %s, D%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_and_re_8()
{
	return util::string_format("and.b   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_and_re_16()
{
	return util::string_format("and.w   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_and_re_32()
{
	return util::string_format("and.l   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_andi_8()
{
	std::string str = get_imm_str_u8();
	return util::string_format("andi.b  %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_andi_16()
{
	std::string str = get_imm_str_u16();
	return util::string_format("andi.w  %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_andi_32()
{
	std::string str = get_imm_str_u32();
	return util::string_format("andi.l  %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_andi_to_ccr()
{
	return util::string_format("andi    %s, CCR", get_imm_str_u8());
}

std::string m68k_disassembler::d68000_andi_to_sr()
{
	return util::string_format("andi    %s, SR", get_imm_str_u16());
}

std::string m68k_disassembler::d68000_asr_s_8()
{
	return util::string_format("asr.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_s_16()
{
	return util::string_format("asr.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_s_32()
{
	return util::string_format("asr.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_r_8()
{
	return util::string_format("asr.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_r_16()
{
	return util::string_format("asr.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_r_32()
{
	return util::string_format("asr.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asr_ea()
{
	return util::string_format("asr.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_asl_s_8()
{
	return util::string_format("asl.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_s_16()
{
	return util::string_format("asl.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_s_32()
{
	return util::string_format("asl.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_r_8()
{
	return util::string_format("asl.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_r_16()
{
	return util::string_format("asl.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_r_32()
{
	return util::string_format("asl.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_asl_ea()
{
	return util::string_format("asl.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bcc_8()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_COND;
	return util::string_format("b%-2s     $%x", m_cc[(m_cpu_ir>>8)&0xf], temp_pc + make_int_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bcc_16()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_COND;
	return util::string_format("b%-2s     $%x", m_cc[(m_cpu_ir>>8)&0xf], temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68020_bcc_32()
{
	u32 temp_pc = m_cpu_pc;
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_COND;
	return util::string_format("b%-2s     $%x; (2+)", m_cc[(m_cpu_ir>>8)&0xf], temp_pc + read_imm_32());
}

std::string m68k_disassembler::d68000_bchg_r()
{
	return util::string_format("bchg    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bchg_s()
{
	std::string str = get_imm_str_u8();
	return util::string_format("bchg    %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bclr_r()
{
	return util::string_format("bclr    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bclr_s()
{
	std::string str = get_imm_str_u8();
	return util::string_format("bclr    %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68010_bkpt()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("bkpt #%d; (1+)", m_cpu_ir&7);
}

std::string m68k_disassembler::d68020_bfchg()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bfchg   %s {%s:%s}; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68020_bfclr()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bfclr   %s {%s:%s}; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68020_bfexts()
{
	auto const limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 const extension = read_imm_16();

	std::string const offset = BIT(extension, 11)
			? util::string_format("D%d", (extension >> 6) & 7)
			: util::string_format("%d", (extension >> 6) & 31);

	std::string const width = BIT(extension, 5)
			? util::string_format("D%d", extension & 7)
			: util::string_format("%d", m_5bit_data_table[extension & 31]);

	return util::string_format("bfexts  %s {%s:%s}, D%d; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width, (extension >> 12) & 7);
}

std::string m68k_disassembler::d68020_bfextu()
{
	auto const limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 const extension = read_imm_16();

	std::string const offset = BIT(extension, 11)
			? util::string_format("D%d", (extension >> 6) & 7)
			: util::string_format("%d", (extension >> 6) & 31);

	std::string const width = BIT(extension, 5)
			? util::string_format("D%d", extension & 7)
			: util::string_format("%d", m_5bit_data_table[extension & 31]);

	return util::string_format("bfextu  %s {%s:%s}, D%d; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width, (extension >> 12) & 7);
}

std::string m68k_disassembler::d68020_bfffo()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bfffo   D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68020_bfins()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bfins   D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68020_bfset()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bfset   %s {%s:%s}; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68020_bftst()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	std::string offset = BIT(extension, 11) ? util::string_format("D%d", (extension>>6)&7) : util::string_format("%d", (extension>>6)&31);

	std::string width = BIT(extension, 5) ? util::string_format("D%d", extension&7) : util::string_format("%d", m_5bit_data_table[extension&31]);
	return util::string_format("bftst   %s {%s:%s}; (2+)", get_ea_mode_str_8(m_cpu_ir), offset, width);
}

std::string m68k_disassembler::d68000_bra_8()
{
	u32 temp_pc = m_cpu_pc;
	return util::string_format("bra     $%x", temp_pc + make_int_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bra_16()
{
	u32 temp_pc = m_cpu_pc;
	return util::string_format("bra     $%x", temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68020_bra_32()
{
	u32 temp_pc = m_cpu_pc;
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("bra     $%x; (2+)", temp_pc + read_imm_32());
}

std::string m68k_disassembler::d68000_bset_r()
{
	return util::string_format("bset    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bset_s()
{
	std::string str = get_imm_str_u8();
	return util::string_format("bset    %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bsr_8()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_OVER;
	return util::string_format("bsr     $%x", temp_pc + make_int_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_bsr_16()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_OVER;
	return util::string_format("bsr     $%x", temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68020_bsr_32()
{
	u32 temp_pc = m_cpu_pc;
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OVER;
	return util::string_format("bsr     $%x; (2+)", temp_pc + read_imm_32());
}

std::string m68k_disassembler::d68000_btst_r()
{
	return util::string_format("btst    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_btst_s()
{
	std::string str = get_imm_str_u8();
	return util::string_format("btst    %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_callm()
{
	auto limit = limit_cpu_types(M68020_ONLY);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_u8();

	return util::string_format("callm   %s, %s; (2)", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cas_8()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("cas.b   D%d, D%d, %s; (2+)", extension&7, (extension>>6)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cas_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("cas.w   D%d, D%d, %s; (2+)", extension&7, (extension>>6)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cas_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("cas.l   D%d, D%d, %s; (2+)", extension&7, (extension>>6)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cas2_16()
{
/* CAS2 Dc1:Dc2,Du1:Dc2:(Rn1):(Rn2)
f e d c b a 9 8 7 6 5 4 3 2 1 0
 DARn1  0 0 0  Du1  0 0 0  Dc1
 DARn2  0 0 0  Du2  0 0 0  Dc2
*/

	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u32 extension = read_imm_32();
	return util::string_format("cas2.w  D%d:D%d:D%d:D%d, (%c%d):(%c%d); (2+)",
		(extension>>16)&7, extension&7, (extension>>22)&7, (extension>>6)&7,
		BIT(extension, 31) ? 'A' : 'D', (extension>>28)&7,
		BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68020_cas2_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u32 extension = read_imm_32();
	return util::string_format("cas2.l  D%d:D%d:D%d:D%d, (%c%d):(%c%d); (2+)",
		(extension>>16)&7, extension&7, (extension>>22)&7, (extension>>6)&7,
		BIT(extension, 31) ? 'A' : 'D', (extension>>28)&7,
		BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68000_chk_16()
{
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("chk.w   %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68020_chk_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("chk.l   %s, D%d; (2+)", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68020_chk2_cmp2_8()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("%s.b  %s, %c%d; (2+)", BIT(extension, 11) ? "chk2" : "cmp2", get_ea_mode_str_8(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68020_chk2_cmp2_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("%s.w  %s, %c%d; (2+)", BIT(extension, 11) ? "chk2" : "cmp2", get_ea_mode_str_16(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68020_chk2_cmp2_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	return util::string_format("%s.l  %s, %c%d; (2+)", BIT(extension, 11) ? "chk2" : "cmp2", get_ea_mode_str_32(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68040_cinv()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;

	static char const *const cachetype[4] = { "nop", "data", "inst", "both" };

	switch((m_cpu_ir>>3)&3)
	{
		case 0:
			return util::string_format("cinv (illegal scope); (4)");
			break;
		case 1:
			return util::string_format("cinvl   %s, (A%d); (4)", cachetype[(m_cpu_ir>>6)&3], m_cpu_ir&7);
			break;
		case 2:
			return util::string_format("cinvp   %s, (A%d); (4)", cachetype[(m_cpu_ir>>6)&3], m_cpu_ir&7);
			break;
		case 3: default:
			return util::string_format("cinva   %s; (4)", cachetype[(m_cpu_ir>>6)&3]);
			break;
	}
}

std::string m68k_disassembler::d68000_clr_8()
{
	return util::string_format("clr.b   %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_clr_16()
{
	return util::string_format("clr.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_clr_32()
{
	return util::string_format("clr.l   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_cmp_8()
{
	return util::string_format("cmp.b   %s, D%d", get_ea_mode_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmp_16()
{
	return util::string_format("cmp.w   %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmp_32()
{
	return util::string_format("cmp.l   %s, D%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmpa_16()
{
	return util::string_format("cmpa.w  %s, A%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmpa_32()
{
	return util::string_format("cmpa.l  %s, A%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmpi_8()
{
	std::string str = get_imm_str_s8();
	return util::string_format("cmpi.b  %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcdi_8()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s8();
	return util::string_format("cmpi.b  %s, %s; (2+)", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcix_8()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s8();
	return util::string_format("cmpi.b  %s, %s; (2+)", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_cmpi_16()
{
	std::string str = get_imm_str_s16();
	return util::string_format("cmpi.w  %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcdi_16()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s16();
	return util::string_format("cmpi.w  %s, %s; (2+)", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcix_16()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s16();
	return util::string_format("cmpi.w  %s, %s; (2+)", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_cmpi_32()
{
	std::string str = get_imm_str_s32();
	return util::string_format("cmpi.l  %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcdi_32()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s32();
	return util::string_format("cmpi.l  %s, %s; (2+)", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_cmpi_pcix_32()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	std::string str = get_imm_str_s32();
	return util::string_format("cmpi.l  %s, %s; (2+)", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_cmpm_8()
{
	return util::string_format("cmpm.b  (A%d)+, (A%d)+", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmpm_16()
{
	return util::string_format("cmpm.w  (A%d)+, (A%d)+", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_cmpm_32()
{
	return util::string_format("cmpm.l  (A%d)+, (A%d)+", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68020_cpbcc_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u32 new_pc = m_cpu_pc;
	u16 extension = read_imm_16();
	new_pc += make_int_16(read_imm_16());
	return util::string_format("%db%-4s  %s; %x (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[m_cpu_ir&0x3f], get_imm_str_s16(), new_pc, extension);
}

std::string m68k_disassembler::d68020_cpbcc_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u32 new_pc = m_cpu_pc;
	u16 extension = read_imm_16();
	new_pc += read_imm_32();
	return util::string_format("%db%-4s  %s; %x (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[m_cpu_ir&0x3f], get_imm_str_s16(), new_pc, extension);
}

std::string m68k_disassembler::d68020_cpdbcc()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u32 new_pc = m_cpu_pc;
	u16 extension1 = read_imm_16();
	u16 extension2 = read_imm_16();
	new_pc += make_int_16(read_imm_16());
	return util::string_format("%ddb%-4s D%d,%s; %x (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[extension1&0x3f], m_cpu_ir&7, get_imm_str_s16(), new_pc, extension2);
}

std::string m68k_disassembler::d68020_cpgen()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("%dgen    %s; (2-3)", (m_cpu_ir>>9)&7, get_imm_str_u32());
}

std::string m68k_disassembler::d68020_cprestore()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	if (((m_cpu_ir>>9)&7) == 1)
	{
		return util::string_format("frestore %s", get_ea_mode_str_8(m_cpu_ir));
	}
	else
	{
		return util::string_format("%drestore %s; (2-3)", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
	}
}

std::string m68k_disassembler::d68020_cpsave()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	if (((m_cpu_ir>>9)&7) == 1)
	{
		return util::string_format("fsave   %s", get_ea_mode_str_8(m_cpu_ir));
	}
	else
	{
		return util::string_format("%dsave   %s; (2-3)", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
	}
}

std::string m68k_disassembler::d68020_cpscc()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension1 = read_imm_16();
	u16 extension2 = read_imm_16();
	return util::string_format("%ds%-4s  %s; (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[extension1&0x3f], get_ea_mode_str_8(m_cpu_ir), extension2);
}

std::string m68k_disassembler::d68020_cptrapcc_0()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension1 = read_imm_16();
	u16 extension2 = read_imm_16();
	return util::string_format("%dtrap%-4s; (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[extension1&0x3f], extension2);
}

std::string m68k_disassembler::d68020_cptrapcc_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension1 = read_imm_16();
	u16 extension2 = read_imm_16();
	return util::string_format("%dtrap%-4s %s; (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[extension1&0x3f], get_imm_str_u16(), extension2);
}

std::string m68k_disassembler::d68020_cptrapcc_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension1 = read_imm_16();
	u16 extension2 = read_imm_16();
	return util::string_format("%dtrap%-4s %s; (extension = %x) (2-3)", (m_cpu_ir>>9)&7, m_cpcc[extension1&0x3f], get_imm_str_u32(), extension2);
}

std::string m68k_disassembler::d68040_cpush()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;

	static char const *const cachetype[4] = { "nop", "data", "inst", "both" };

	switch((m_cpu_ir>>3)&3)
	{
		case 0:
			return util::string_format("cpush (illegal scope); (4)");
			break;
		case 1:
			return util::string_format("cpushl  %s, (A%d); (4)", cachetype[(m_cpu_ir>>6)&3], m_cpu_ir&7);
			break;
		case 2:
			return util::string_format("cpushp  %s, (A%d); (4)", cachetype[(m_cpu_ir>>6)&3], m_cpu_ir&7);
			break;
		case 3: default:
			return util::string_format("cpusha  %s; (4)", cachetype[(m_cpu_ir>>6)&3]);
			break;
	}
}

std::string m68k_disassembler::d68000_dbra()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_COND;
	return util::string_format("dbra    D%d, $%x", m_cpu_ir & 7, temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68000_dbcc()
{
	u32 temp_pc = m_cpu_pc;
	m_flags = STEP_COND;
	return util::string_format("db%-2s    D%d, $%x", m_cc[(m_cpu_ir>>8)&0xf], m_cpu_ir & 7, temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68000_divs()
{
	return util::string_format("divs.w  %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_divu()
{
	return util::string_format("divu.w  %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68020_divl()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 extension = read_imm_16();

	if(BIT(extension, 10))
		return util::string_format("div%c.l  %s, D%d:D%d; (2+)", BIT(extension, 11) ? 's' : 'u', get_ea_mode_str_32(m_cpu_ir), extension&7, (extension>>12)&7);
	else if((extension&7) == ((extension>>12)&7))
		return util::string_format("div%c.l  %s, D%d; (2+)", BIT(extension, 11) ? 's' : 'u', get_ea_mode_str_32(m_cpu_ir), (extension>>12)&7);
	else
		return util::string_format("div%cl.l %s, D%d:D%d; (2+)", BIT(extension, 11) ? 's' : 'u', get_ea_mode_str_32(m_cpu_ir), extension&7, (extension>>12)&7);
}

std::string m68k_disassembler::d68000_eor_8()
{
	return util::string_format("eor.b   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eor_16()
{
	return util::string_format("eor.w   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eor_32()
{
	return util::string_format("eor.l   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eori_8()
{
	std::string str = get_imm_str_u8();
	return util::string_format("eori.b  %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eori_16()
{
	std::string str = get_imm_str_u16();
	return util::string_format("eori.w  %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eori_32()
{
	std::string str = get_imm_str_u32();
	return util::string_format("eori.l  %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_eori_to_ccr()
{
	return util::string_format("eori    %s, CCR", get_imm_str_u8());
}

std::string m68k_disassembler::d68000_eori_to_sr()
{
	return util::string_format("eori    %s, SR", get_imm_str_u16());
}

std::string m68k_disassembler::d68000_exg_dd()
{
	return util::string_format("exg     D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_exg_aa()
{
	return util::string_format("exg     A%d, A%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_exg_da()
{
	return util::string_format("exg     D%d, A%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ext_16()
{
	return util::string_format("ext.w   D%d", m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ext_32()
{
	return util::string_format("ext.l   D%d", m_cpu_ir&7);
}

std::string m68k_disassembler::d68020_extb_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("extb.l  D%d; (2+)", m_cpu_ir&7);
}

std::string m68k_disassembler::d68881_ftrap()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 w2 = read_imm_16();

	switch (m_cpu_ir & 0x7)
	{
		case 2: // word operand
			return util::string_format("ftrap%s.w   $%04x", m_cpcc[w2 & 0x3f], read_imm_16());

		case 3: // long word operand
			return util::string_format("ftrap%s.l   $%08x", m_cpcc[w2 & 0x3f], read_imm_32());

		case 4: // no operand
			return util::string_format("ftrap%s", m_cpcc[w2 & 0x3f]);
	}
	return util::string_format("ftrap%s<%d>?", m_cpcc[w2 & 0x3f], m_cpu_ir & 7);

}

std::string m68k_disassembler::d68040_fpu()
{
	char float_data_format[8][3] =
	{
		".l", ".s", ".x", ".p", ".w", ".d", ".b", ".p"
	};

	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;

	u16 w2 = read_imm_16();

	u16 src = (w2 >> 10) & 0x7;
	u16 dst_reg = (w2 >> 7) & 0x7;

	// special override for FMOVECR
	if ((((w2 >> 13) & 0x7) == 2) && (((w2>>10)&0x7) == 7))
	{
		return util::string_format("fmovecr   #$%0x, fp%d", (w2&0x7f), dst_reg);
	}

	std::string mnemonic;
	switch ((w2 >> 13) & 0x7)
	{
		case 0x0:
		case 0x2:
		{
			switch(w2 & 0x7f)
			{
				case 0x00:  mnemonic = "fmove"; break;
				case 0x01:  mnemonic = "fint"; break;
				case 0x02:  mnemonic = "fsinh"; break;
				case 0x03:  mnemonic = "fintrz"; break;
				case 0x04:  mnemonic = "fsqrt"; break;
				case 0x06:  mnemonic = "flognp1"; break;
				case 0x08:  mnemonic = "fetoxm1"; break;
				case 0x09:  mnemonic = "ftanh1"; break;
				case 0x0a:  mnemonic = "fatan"; break;
				case 0x0c:  mnemonic = "fasin"; break;
				case 0x0d:  mnemonic = "fatanh"; break;
				case 0x0e:  mnemonic = "fsin"; break;
				case 0x0f:  mnemonic = "ftan"; break;
				case 0x10:  mnemonic = "fetox"; break;
				case 0x11:  mnemonic = "ftwotox"; break;
				case 0x12:  mnemonic = "ftentox"; break;
				case 0x14:  mnemonic = "flogn"; break;
				case 0x15:  mnemonic = "flog10"; break;
				case 0x16:  mnemonic = "flog2"; break;
				case 0x18:  mnemonic = "fabs"; break;
				case 0x19:  mnemonic = "fcosh"; break;
				case 0x1a:  mnemonic = "fneg"; break;
				case 0x1c:  mnemonic = "facos"; break;
				case 0x1d:  mnemonic = "fcos"; break;
				case 0x1e:  mnemonic = "fgetexp"; break;
				case 0x1f:  mnemonic = "fgetman"; break;
				case 0x20:  mnemonic = "fdiv"; break;
				case 0x21:  mnemonic = "fmod"; break;
				case 0x22:  mnemonic = "fadd"; break;
				case 0x23:  mnemonic = "fmul"; break;
				case 0x24:  mnemonic = "fsgldiv"; break;
				case 0x25:  mnemonic = "frem"; break;
				case 0x26:  mnemonic = "fscale"; break;
				case 0x27:  mnemonic = "fsglmul"; break;
				case 0x28:  mnemonic = "fsub"; break;
				case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
							mnemonic = "fsincos"; break;
				case 0x38:  mnemonic = "fcmp"; break;
				case 0x3a:  mnemonic = "ftst"; break;
				case 0x41:  mnemonic = "fssqrt"; break;
				case 0x45:  mnemonic = "fdsqrt"; break;
				case 0x58:  mnemonic = "fsabs"; break;
				case 0x5a:  mnemonic = "fsneg"; break;
				case 0x5c:  mnemonic = "fdabs"; break;
				case 0x5e:  mnemonic = "fdneg"; break;
				case 0x60:  mnemonic = "fsdiv"; break;
				case 0x62:  mnemonic = "fsadd"; break;
				case 0x63:  mnemonic = "fsmul"; break;
				case 0x64:  mnemonic = "fddiv"; break;
				case 0x66:  mnemonic = "fdadd"; break;
				case 0x67:  mnemonic = "fdmul"; break;
				case 0x68:  mnemonic = "fssub"; break;
				case 0x6c:  mnemonic = "fdsub"; break;

				default:    mnemonic = "FPU (?)"; break;
			}

			if (w2 & 0x4000)
			{
				return util::string_format("%s%s   %s, FP%d", mnemonic, float_data_format[src], get_ea_mode_str_32(m_cpu_ir), dst_reg);
			}
			else
			{
				return util::string_format("%s.x   FP%d, FP%d", mnemonic, src, dst_reg);
			}
		}

		case 0x3:
		{
			switch ((w2>>10)&7)
			{
				case 3:     // packed decimal w/fixed k-factor
					return util::string_format("fmove%s   FP%d, %s {#%d}", float_data_format[(w2>>10)&7], dst_reg, get_ea_mode_str_32(m_cpu_ir), util::sext(w2&0x7f, 7));

				case 7:     // packed decimal w/dynamic k-factor (register)
					return util::string_format("fmove%s   FP%d, %s {D%d}", float_data_format[(w2>>10)&7], dst_reg, get_ea_mode_str_32(m_cpu_ir), (w2>>4)&7);

				default:
					return util::string_format("fmove%s   FP%d, %s", float_data_format[(w2>>10)&7], dst_reg, get_ea_mode_str_32(m_cpu_ir));
					break;
			}
			break;
		}

		case 0x4:   // ea to control
		{
			std::string dasm = util::string_format("fmovem.l   %s, ", get_ea_mode_str_32(m_cpu_ir));
			if (w2 & 0x1000) return dasm + "fpcr";
			if (w2 & 0x0800) return dasm + "/fpsr";
			if (w2 & 0x0400) return dasm + "/fpiar";
			return dasm;
		}

		case 0x5:   // control to ea
		{
			std::string dasm = "fmovem.l   ";
			if (w2 & 0x1000) dasm += "fpcr";
			if (w2 & 0x0800) dasm += "/fpsr";
			if (w2 & 0x0400) dasm += "/fpiar";
			return dasm + ", " + get_ea_mode_str_32(m_cpu_ir);
		}

		case 0x6:   // memory to FPU, list
		{
			if ((w2>>11) & 1)   // dynamic register list
			{
				return util::string_format("fmovem.x   %s, D%d", get_ea_mode_str_32(m_cpu_ir), (w2>>4)&7);
			}
			else    // static register list
			{
				std::string dasm = util::string_format("fmovem.x   %s, ", get_ea_mode_str_32(m_cpu_ir));

				for (int i = 0; i < 8; i++)
				{
					if (w2 & (1<<i))
					{
						if ((w2>>12) & 1)   // postincrement or control
						{
							dasm += util::string_format("FP%d ", 7-i);
						}
						else            // predecrement
						{
							dasm += util::string_format("FP%d ", i);
						}
					}
				}
				return dasm;
			}
		}

		case 0x7:   // FPU to memory, list
		{
			if ((w2>>11) & 1)   // dynamic register list
			{
				return util::string_format("fmovem.x   D%d, %s", (w2>>4)&7, get_ea_mode_str_32(m_cpu_ir));
			}
			else    // static register list
			{
				std::string dasm = util::string_format("fmovem.x   ");

				for (int i = 0; i < 8; i++)
				{
					if (w2 & (1<<i))
					{
						if ((w2>>12) & 1)   // postincrement or control
						{
							dasm += util::string_format("FP%d ", 7-i);
						}
						else            // predecrement
						{
							dasm += util::string_format("FP%d ", i);
						}
					}
				}

				return dasm + ", " + get_ea_mode_str_32(m_cpu_ir);
			}
		}
	}
	return util::string_format("FPU (?) ");
}

std::string m68k_disassembler::d68000_jmp()
{
	return util::string_format("jmp     %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_jsr()
{
	m_flags = STEP_OVER;
	return util::string_format("jsr     %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_lea()
{
	return util::string_format("lea     %s, A%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_link_16()
{
	return util::string_format("link    A%d, %s", m_cpu_ir&7, get_imm_str_s16());
}

std::string m68k_disassembler::d68020_link_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("link    A%d, %s; (2+)", m_cpu_ir&7, get_imm_str_s32());
}

std::string m68k_disassembler::d68000_lsr_s_8()
{
	return util::string_format("lsr.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_s_16()
{
	return util::string_format("lsr.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_s_32()
{
	return util::string_format("lsr.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_r_8()
{
	return util::string_format("lsr.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_r_16()
{
	return util::string_format("lsr.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_r_32()
{
	return util::string_format("lsr.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsr_ea()
{
	return util::string_format("lsr.w   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_lsl_s_8()
{
	return util::string_format("lsl.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_s_16()
{
	return util::string_format("lsl.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_s_32()
{
	return util::string_format("lsl.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_r_8()
{
	return util::string_format("lsl.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_r_16()
{
	return util::string_format("lsl.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_r_32()
{
	return util::string_format("lsl.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_lsl_ea()
{
	return util::string_format("lsl.w   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_move_8()
{
	std::string str = get_ea_mode_str_8(m_cpu_ir);
	return util::string_format("move.b  %s, %s", str, get_ea_mode_str_8(((m_cpu_ir>>9) & 7) | ((m_cpu_ir>>3) & 0x38)));
}

std::string m68k_disassembler::d68000_move_16()
{
	std::string str = get_ea_mode_str_16(m_cpu_ir);
	return util::string_format("move.w  %s, %s", str, get_ea_mode_str_16(((m_cpu_ir>>9) & 7) | ((m_cpu_ir>>3) & 0x38)));
}

std::string m68k_disassembler::d68000_move_32()
{
	std::string str = get_ea_mode_str_32(m_cpu_ir);
	return util::string_format("move.l  %s, %s", str, get_ea_mode_str_32(((m_cpu_ir>>9) & 7) | ((m_cpu_ir>>3) & 0x38)));
}

std::string m68k_disassembler::d68000_movea_16()
{
	return util::string_format("movea.w %s, A%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_movea_32()
{
	return util::string_format("movea.l %s, A%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_move_to_ccr()
{
	return util::string_format("move    %s, CCR", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68010_move_fr_ccr()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move    CCR, %s; (1+)", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_move_fr_sr()
{
	return util::string_format("move    SR, %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_move_to_sr()
{
	return util::string_format("move    %s, SR", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_move_fr_usp()
{
	return util::string_format("move    USP, A%d", m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_move_to_usp()
{
	return util::string_format("move    A%d, USP", m_cpu_ir&7);
}

std::string m68k_disassembler::d68010_movec()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;

	std::string reg_name, processor;
	u16 extension = read_imm_16();

	switch(extension & 0xfff)
	{
		case 0x000:
			reg_name = "SFC";
			processor = "1+";
			break;
		case 0x001:
			reg_name = "DFC";
			processor = "1+";
			break;
		case 0x800:
			reg_name = "USP";
			processor = "1+";
			break;
		case 0x801:
			reg_name = "VBR";
			processor = "1+";
			break;
		case 0x002:
			reg_name = "CACR";
			processor = "2+";
			break;
		case 0x802:
			reg_name = "CAAR";
			processor = "2,3";
			break;
		case 0x803:
			reg_name = "MSP";
			processor = "2+";
			break;
		case 0x804:
			reg_name = "ISP";
			processor = "2+";
			break;
		case 0x003:
			reg_name = "TC";
			processor = "4+";
			break;
		case 0x004:
			if(m_cpu_type & COLDFIRE)
			{
				reg_name = "ACR0";
				processor = "CF";
			}
			else
			{
				reg_name = "ITT0";
				processor = "4+";
			}
			break;
		case 0x005:
			if(m_cpu_type & COLDFIRE)
			{
				reg_name = "ACR1";
				processor = "CF";
			}
			else
			{
				reg_name = "ITT1";
				processor = "4+";
			}
			break;
		case 0x006:
			if(m_cpu_type & COLDFIRE)
			{
				reg_name = "ACR2";
				processor = "CF";
			}
			else
			{
				reg_name = "DTT0";
				processor = "4+";
			}
			break;
		case 0x007:
			if(m_cpu_type & COLDFIRE)
			{
				reg_name = "ACR3";
				processor = "CF";
			}
			else
			{
				reg_name = "DTT1";
				processor = "4+";
			}
			break;
		case 0x805:
			reg_name = "MMUSR";
			processor = "4+";
			break;
		case 0x806:
			reg_name = "URP";
			processor = "4+";
			break;
		case 0x807:
			reg_name = "SRP";
			processor = "4+";
			break;
		case 0xc00:
			reg_name = "ROMBAR0";
			processor = "CF";
			break;
		case 0xc01:
			reg_name = "ROMBAR1";
			processor = "CF";
			break;
		case 0xc04:
			reg_name = "RAMBAR0";
			processor = "CF";
			break;
		case 0xc05:
			reg_name = "RAMBAR1";
			processor = "CF";
			break;
		case 0xc0c:
			reg_name = "MPCR";
			processor = "CF";
			break;
		case 0xc0d:
			reg_name = "EDRAMBAR";
			processor = "CF";
			break;
		case 0xc0e:
			reg_name = "SECMBAR";
			processor = "CF";
			break;
		case 0xc0f:
			reg_name = "MBAR";
			processor = "CF";
			break;
		default:
			reg_name = make_signed_hex_str_16(extension & 0xfff);
			processor = "?";
	}

	if(BIT(m_cpu_ir, 0))
		return util::string_format("movec   %c%d, %s; (%s)", BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7, reg_name, processor);
	else
		return util::string_format("movec   %s, %c%d; (%s)", reg_name, BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7, processor);
}

std::string m68k_disassembler::d68000_movem_pd_16()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(15-i)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(15-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(7-i)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(7-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.w %s, %s", regs, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_movem_pd_32()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(15-i)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(15-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(7-i)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(7-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.l %s, %s", regs, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_movem_er_16()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.w %s, %s", get_ea_mode_str_16(m_cpu_ir), regs);
}

std::string m68k_disassembler::d68000_movem_er_32()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.l %s, %s", get_ea_mode_str_32(m_cpu_ir), regs);
}

std::string m68k_disassembler::d68000_movem_re_16()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.w %s, %s", regs, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_movem_re_32()
{
	u32 data = read_imm_16();

	std::string regs;
	for(int i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("D%d", first);
			if(run_length > 0)
				regs += util::string_format("-D%d", first + run_length);
		}
	}
	for(int i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			int first = i;
			int run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(!regs.empty())
				regs += '/';
			regs += util::string_format("A%d", first);
			if(run_length > 0)
				regs += util::string_format("-A%d", first + run_length);
		}
	}
	return util::string_format("movem.l %s, %s", regs, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_movep_re_16()
{
	return util::string_format("movep.w D%d, ($%x,A%d)", (m_cpu_ir>>9)&7, read_imm_16(), m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_movep_re_32()
{
	return util::string_format("movep.l D%d, ($%x,A%d)", (m_cpu_ir>>9)&7, read_imm_16(), m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_movep_er_16()
{
	return util::string_format("movep.w ($%x,A%d), D%d", read_imm_16(), m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_movep_er_32()
{
	return util::string_format("movep.l ($%x,A%d), D%d", read_imm_16(), m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68010_moves_8()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	if(BIT(extension, 11))
		return util::string_format("moves.b %c%d, %s; (1+)", BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_8(m_cpu_ir));
	else
		return util::string_format("moves.b %s, %c%d; (1+)", get_ea_mode_str_8(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68010_moves_16()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	if(BIT(extension, 11))
		return util::string_format("moves.w %c%d, %s; (1+)", BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_16(m_cpu_ir));
	else
		return util::string_format("moves.w %s, %c%d; (1+)", get_ea_mode_str_16(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68010_moves_32()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();
	if(BIT(extension, 11))
		return util::string_format("moves.l %c%d, %s; (1+)", BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_32(m_cpu_ir));
	else
		return util::string_format("moves.l %s, %c%d; (1+)", get_ea_mode_str_32(m_cpu_ir), BIT(extension, 15) ? 'A' : 'D', (extension>>12)&7);
}

std::string m68k_disassembler::d68000_moveq()
{
	return util::string_format("moveq   #%s, D%d", make_signed_hex_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68040_move16_pi_pi()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move16  (A%d)+, (A%d)+; (4)", m_cpu_ir&7, (read_imm_16()>>12)&7);
}

std::string m68k_disassembler::d68040_move16_pi_al()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move16  (A%d)+, %s; (4)", m_cpu_ir&7, get_imm_str_u32());
}

std::string m68k_disassembler::d68040_move16_al_pi()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move16  %s, (A%d)+; (4)", get_imm_str_u32(), m_cpu_ir&7);
}

std::string m68k_disassembler::d68040_move16_ai_al()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move16  (A%d), %s; (4)", m_cpu_ir&7, get_imm_str_u32());
}

std::string m68k_disassembler::d68040_move16_al_ai()
{
	auto limit = limit_cpu_types(M68040_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("move16  %s, (A%d); (4)", get_imm_str_u32(), m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_muls()
{
	return util::string_format("muls.w  %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_mulu()
{
	return util::string_format("mulu.w  %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68020_mull()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	u16 extension = read_imm_16();

	if(BIT(extension, 10))
		return util::string_format("mul%c.l %s, D%d-D%d; (2+)", BIT(extension, 11) ? 's' : 'u', get_ea_mode_str_32(m_cpu_ir), extension&7, (extension>>12)&7);
	else
		return util::string_format("mul%c.l  %s, D%d; (2+)", BIT(extension, 11) ? 's' : 'u', get_ea_mode_str_32(m_cpu_ir), (extension>>12)&7);
}

std::string m68k_disassembler::d68000_nbcd()
{
	return util::string_format("nbcd    %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_neg_8()
{
	return util::string_format("neg.b   %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_neg_16()
{
	return util::string_format("neg.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_neg_32()
{
	return util::string_format("neg.l   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_negx_8()
{
	return util::string_format("negx.b  %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_negx_16()
{
	return util::string_format("negx.w  %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_negx_32()
{
	return util::string_format("negx.l  %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_nop()
{
	return util::string_format("nop");
}

std::string m68k_disassembler::d68000_not_8()
{
	return util::string_format("not.b   %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_not_16()
{
	return util::string_format("not.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_not_32()
{
	return util::string_format("not.l   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_or_er_8()
{
	return util::string_format("or.b    %s, D%d", get_ea_mode_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_or_er_16()
{
	return util::string_format("or.w    %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_or_er_32()
{
	return util::string_format("or.l    %s, D%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_or_re_8()
{
	return util::string_format("or.b    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_or_re_16()
{
	return util::string_format("or.w    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_or_re_32()
{
	return util::string_format("or.l    D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_ori_8()
{
	std::string str = get_imm_str_u8();
	return util::string_format("ori.b   %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_ori_16()
{
	std::string str = get_imm_str_u16();
	return util::string_format("ori.w   %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_ori_32()
{
	std::string str = get_imm_str_u32();
	return util::string_format("ori.l   %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_ori_to_ccr()
{
	return util::string_format("ori     %s, CCR", get_imm_str_u8());
}

std::string m68k_disassembler::d68000_ori_to_sr()
{
	return util::string_format("ori     %s, SR", get_imm_str_u16());
}

std::string m68k_disassembler::d68020_pack_rr()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("pack    D%d, D%d, %s; (2+)", m_cpu_ir&7, (m_cpu_ir>>9)&7, get_imm_str_u16());
}

std::string m68k_disassembler::d68020_pack_mm()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("pack    -(A%d), -(A%d), %s; (2+)", m_cpu_ir&7, (m_cpu_ir>>9)&7, get_imm_str_u16());
}

std::string m68k_disassembler::d68000_pea()
{
	return util::string_format("pea     %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_reset()
{
	return util::string_format("reset");
}

std::string m68k_disassembler::d68000_ror_s_8()
{
	return util::string_format("ror.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_s_16()
{
	return util::string_format("ror.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7],m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_s_32()
{
	return util::string_format("ror.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_r_8()
{
	return util::string_format("ror.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_r_16()
{
	return util::string_format("ror.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_r_32()
{
	return util::string_format("ror.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_ror_ea()
{
	return util::string_format("ror.w   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_rol_s_8()
{
	return util::string_format("rol.b   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_s_16()
{
	return util::string_format("rol.w   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_s_32()
{
	return util::string_format("rol.l   #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_r_8()
{
	return util::string_format("rol.b   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_r_16()
{
	return util::string_format("rol.w   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_r_32()
{
	return util::string_format("rol.l   D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rol_ea()
{
	return util::string_format("rol.w   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_roxr_s_8()
{
	return util::string_format("roxr.b  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxr_s_16()
{
	return util::string_format("roxr.w  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}


std::string m68k_disassembler::d68000_roxr_s_32()
{
	return util::string_format("roxr.l  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxr_r_8()
{
	return util::string_format("roxr.b  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxr_r_16()
{
	return util::string_format("roxr.w  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxr_r_32()
{
	return util::string_format("roxr.l  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxr_ea()
{
	return util::string_format("roxr.w  %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_roxl_s_8()
{
	return util::string_format("roxl.b  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_s_16()
{
	return util::string_format("roxl.w  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_s_32()
{
	return util::string_format("roxl.l  #%d, D%d", m_3bit_qdata_table[(m_cpu_ir>>9)&7], m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_r_8()
{
	return util::string_format("roxl.b  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_r_16()
{
	return util::string_format("roxl.w  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_r_32()
{
	return util::string_format("roxl.l  D%d, D%d", (m_cpu_ir>>9)&7, m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_roxl_ea()
{
	return util::string_format("roxl.w  %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68010_rtd()
{
	auto limit = limit_cpu_types(M68010_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OUT;
	return util::string_format("rtd     %s; (1+)", get_imm_str_s16());
}

std::string m68k_disassembler::d68000_rte()
{
	m_flags = STEP_OUT;
	return util::string_format("rte");
}

std::string m68k_disassembler::d68020_rtm()
{
	auto limit = limit_cpu_types(M68020_ONLY);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OUT;
	return util::string_format("rtm     %c%d; (2+)", BIT(m_cpu_ir, 3) ? 'A' : 'D', m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_rtr()
{
	m_flags = STEP_OUT;
	return util::string_format("rtr");
}

std::string m68k_disassembler::d68000_rts()
{
	m_flags = STEP_OUT;
	return util::string_format("rts");
}

std::string m68k_disassembler::d68000_sbcd_rr()
{
	return util::string_format("sbcd    D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_sbcd_mm()
{
	return util::string_format("sbcd    -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_scc()
{
	return util::string_format("s%-2s     %s", m_cc[(m_cpu_ir>>8)&0xf], get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_stop()
{
	return util::string_format("stop    %s", get_imm_str_s16());
}

std::string m68k_disassembler::d68000_sub_er_8()
{
	return util::string_format("sub.b   %s, D%d", get_ea_mode_str_8(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_sub_er_16()
{
	return util::string_format("sub.w   %s, D%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_sub_er_32()
{
	return util::string_format("sub.l   %s, D%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_sub_re_8()
{
	return util::string_format("sub.b   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_sub_re_16()
{
	return util::string_format("sub.w   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_sub_re_32()
{
	return util::string_format("sub.l   D%d, %s", (m_cpu_ir>>9)&7, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_suba_16()
{
	return util::string_format("suba.w  %s, A%d", get_ea_mode_str_16(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_suba_32()
{
	return util::string_format("suba.l  %s, A%d", get_ea_mode_str_32(m_cpu_ir), (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subi_8()
{
	std::string str = get_imm_str_s8();
	return util::string_format("subi.b  %s, %s", str, get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subi_16()
{
	std::string str = get_imm_str_s16();
	return util::string_format("subi.w  %s, %s", str, get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subi_32()
{
	std::string str = get_imm_str_s32();
	return util::string_format("subi.l  %s, %s", str, get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subq_8()
{
	return util::string_format("subq.b  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subq_16()
{
	return util::string_format("subq.w  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subq_32()
{
	return util::string_format("subq.l  #%d, %s", m_3bit_qdata_table[(m_cpu_ir>>9)&7], get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_subx_rr_8()
{
	return util::string_format("subx.b  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subx_rr_16()
{
	return util::string_format("subx.w  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subx_rr_32()
{
	return util::string_format("subx.l  D%d, D%d", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subx_mm_8()
{
	return util::string_format("subx.b  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subx_mm_16()
{
	return util::string_format("subx.w  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_subx_mm_32()
{
	return util::string_format("subx.l  -(A%d), -(A%d)", m_cpu_ir&7, (m_cpu_ir>>9)&7);
}

std::string m68k_disassembler::d68000_swap()
{
	return util::string_format("swap    D%d", m_cpu_ir&7);
}

std::string m68k_disassembler::d68000_tas()
{
	return util::string_format("tas     %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_trap()
{
	return util::string_format("trap    #$%x", m_cpu_ir&0xf);
}

std::string m68k_disassembler::d68020_trapcc_0()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("trap%-2s; (2+)", m_cc[(m_cpu_ir>>8)&0xf]);
}

std::string m68k_disassembler::d68020_trapcc_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("trap%-2s  %s; (2+)", m_cc[(m_cpu_ir>>8)&0xf], get_imm_str_u16());
}

std::string m68k_disassembler::d68020_trapcc_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("trap%-2s  %s; (2+)", m_cc[(m_cpu_ir>>8)&0xf], get_imm_str_u32());
}

std::string m68k_disassembler::d68000_trapv()
{
	m_flags = STEP_OVER | STEP_COND;
	return util::string_format("trapv");
}

std::string m68k_disassembler::d68000_tst_8()
{
	return util::string_format("tst.b   %s", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcdi_8()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.b   %s; (2+)", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcix_8()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.b   %s; (2+)", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_i_8()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.b   %s; (2+)", get_ea_mode_str_8(m_cpu_ir));
}

std::string m68k_disassembler::d68000_tst_16()
{
	return util::string_format("tst.w   %s", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_a_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.w   %s; (2+)", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcdi_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.w   %s; (2+)", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcix_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.w   %s; (2+)", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_i_16()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.w   %s; (2+)", get_ea_mode_str_16(m_cpu_ir));
}

std::string m68k_disassembler::d68000_tst_32()
{
	return util::string_format("tst.l   %s", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_a_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.l   %s; (2+)", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcdi_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.l   %s; (2+)", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_pcix_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.l   %s; (2+)", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68020_tst_i_32()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("tst.l   %s; (2+)", get_ea_mode_str_32(m_cpu_ir));
}

std::string m68k_disassembler::d68000_unlk()
{
	return util::string_format("unlk    A%d", m_cpu_ir&7);
}

std::string m68k_disassembler::d68020_unpk_rr()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("unpk    D%d, D%d, %s; (2+)", m_cpu_ir&7, (m_cpu_ir>>9)&7, get_imm_str_u16());
}

std::string m68k_disassembler::d68020_unpk_mm()
{
	auto limit = limit_cpu_types(M68020_PLUS);
	if(limit.first)
		return limit.second;
	return util::string_format("unpk    -(A%d), -(A%d), %s; (2+)", m_cpu_ir&7, (m_cpu_ir>>9)&7, get_imm_str_u16());
}

std::string m68k_disassembler::fc_to_string(u16 modes)
{
	u16 fc = modes & 0x1f;

	if (fc == 0)
	{
		return "%sfc";
	}
	else if (fc == 1)
	{
		return "%dfc";
	}
	else if ((fc >> 3) == 1)
	{
		return util::string_format("D%d", fc & 7);
	}
	else if ((fc >> 3) == 2)
	{
		return util::string_format("#%d", fc & 7);
	}
	return util::string_format("unknown fc %x", fc);
}

std::string m68k_disassembler::d68040_p000()
{

	if ((m_cpu_ir & 0xffd8) == 0xf548) // 68040 PTEST
	{
		return util::string_format("ptest%c  (A%d)", (m_cpu_ir & 0x20) ? 'r' : 'w', m_cpu_ir & 7);
	}

	if ((m_cpu_ir & 0xffe0) == 0xf500) // 68040 PFLUSH
	{
		switch((m_cpu_ir >> 3) & 3)
		{
		case 0:
			return util::string_format("pflushn (A%d)", m_cpu_ir & 7);
		case 1:
			return util::string_format("pflush  (A%d)", m_cpu_ir & 7);
		case 2:
			return "pflushan";
		case 3:
			return "pflusha";
		}
	}
	return util::string_format("unknown instruction: %04x", m_cpu_ir);
}
// PFLUSH:  001xxx0xxxxxxxxx
// PLOAD:   001000x0000xxxxx
// PVALID1: 0010100000000000
// PVALID2: 0010110000000xxx
// PMOVE 1: 010xxxx000000000
// PMOVE 2: 011xxxx0000xxx00
// PMOVE 3: 011xxxx000000000
// PTEST:   100xxxxxxxxxxxxx
// PFLUSHR:  1010000000000000
std::string m68k_disassembler::d68851_p000()
{
	u16 modes = read_imm_16();

	// do this after fetching the second PMOVE word so we properly get the 3rd if necessary
	std::string str = get_ea_mode_str_32(m_cpu_ir);

	if ((modes & 0xfde0) == 0x2000) // PLOAD
	{
		if (modes & 0x0200)
		{
			return util::string_format("pload   #%d, %s", (modes>>10)&7, str);
		}
		else
		{
			return util::string_format("pload   %s, #%d", str, (modes>>10)&7);
		}
	}

	if ((modes & 0xe200) == 0x2000) // PFLUSH
	{
		return util::string_format("pflushr %x, %x, %s", modes & 0x1f, (modes>>5)&0xf, str);
	}

	if (modes == 0xa000)    // PFLUSHR
	{
		return util::string_format("pflushr %s", str);
	}

	if (modes == 0x2800)    // PVALID (FORMAT 1)
	{
		return util::string_format("pvalid  VAL, %s", str);
	}

	if ((modes & 0xfff8) == 0x2c00) // PVALID (FORMAT 2)
	{
		return util::string_format("pvalid  A%d, %s", modes & 0xf, str);
	}

	if ((modes & 0xe000) == 0x8000) // PTEST
	{
		if (modes & 0x100)
		{
			return util::string_format("ptest%c  %s, %s, %d, @A%d",
					(modes & 0x200) ? 'r' : 'w',
							fc_to_string(modes),
							str,
							(modes >> 10) & 7,
							(modes >> 5) & 7);
		}
		else
		{
			return util::string_format("ptest%c  %s, %s, %d",
					(modes & 0x200) ? 'r' : 'w',
							fc_to_string(modes),
							str,
							(modes >> 10) & 7);
		}
	}

	switch ((modes>>13) & 0x7)
	{
		case 0: // MC68030/040 form with FD bit
		case 2: // MC68881 form, FD never set
			if (modes & 0x0100)
			{
				if (modes & 0x0200)
				{
					return util::string_format("pmovefd %s, %s", m_mmuregs[(modes>>10)&7], str);
				}
				else
				{
					return util::string_format("pmovefd %s, %s", str, m_mmuregs[(modes>>10)&7]);
				}
			}
			else
			{
				if (modes & 0x0200)
				{
					return util::string_format("pmove   %s, %s", m_mmuregs[(modes>>10)&7], str);
				}
				else
				{
					return util::string_format("pmove   %s, %s", str, m_mmuregs[(modes>>10)&7]);
				}
			}
			break;

		case 3: // MC68030 to/from status reg
			if (modes & 0x0200)
			{
				return util::string_format("pmove   mmusr, %s", str);
			}
			else
			{
				return util::string_format("pmove   %s, mmusr", str);
			}
			break;

	}
	return util::string_format("pmove [unknown form] %s", str);
}

std::string m68k_disassembler::d68851_pbcc16()
{
	u32 temp_pc = m_cpu_pc;

	return util::string_format("pb%s %x", m_mmucond[m_cpu_ir&0xf], temp_pc + make_int_16(read_imm_16()));
}

std::string m68k_disassembler::d68851_pbcc32()
{
	u32 temp_pc = m_cpu_pc;

	return util::string_format("pb%s %x", m_mmucond[m_cpu_ir&0xf], temp_pc + make_int_32(read_imm_32()));
}

std::string m68k_disassembler::d68851_pdbcc()
{
	u32 temp_pc = m_cpu_pc;
	u16 modes = read_imm_16();

	return util::string_format("pb%s %x", m_mmucond[modes&0xf], temp_pc + make_int_16(read_imm_16()));
}

// PScc:  0000000000xxxxxx
std::string m68k_disassembler::d68851_p001()
{
	return util::string_format("MMU 001 group");
}

// fbcc is 68040 and 68881
std::string m68k_disassembler::d68040_fbcc_16()
{
	auto limit = limit_cpu_types(M68030_PLUS);
	if(limit.first)
		return limit.second;
	u32 temp_pc = m_cpu_pc;
	s16 disp = make_int_16(read_imm_16());
	return util::string_format("fb%-s   $%x", m_cpcc[m_cpu_ir & 0x3f], temp_pc + disp);
}

std::string m68k_disassembler::d68040_fbcc_32()
{
	auto limit = limit_cpu_types(M68030_PLUS);
	if(limit.first)
		return limit.second;
	u32 temp_pc = m_cpu_pc;
	u32 disp = read_imm_32();
	return util::string_format("fb%-s   $%x", m_cpcc[m_cpu_ir & 0x3f], temp_pc + disp);
}

/* ======================================================================== */
/* ======================= INSTRUCTION TABLE BUILDER ====================== */
/* ======================================================================== */

/* EA Masks:
800 = data register direct
400 = address register direct
200 = address register indirect
100 = ARI postincrement
 80 = ARI pre-decrement
 40 = ARI displacement
 20 = ARI index
 10 = absolute short
  8 = absolute long
  4 = immediate / sr
  2 = pc displacement
  1 = pc idx
*/

const m68k_disassembler::opcode_struct m68k_disassembler::m_opcode_info[] =
{
/*  opcode handler                             mask    match   ea mask */
	{&m68k_disassembler::d68000_1010         , 0xf000, 0xa000, 0x000},
	{&m68k_disassembler::d68000_1111         , 0xf000, 0xf000, 0x000},
	{&m68k_disassembler::d68000_abcd_rr      , 0xf1f8, 0xc100, 0x000},
	{&m68k_disassembler::d68000_abcd_mm      , 0xf1f8, 0xc108, 0x000},
	{&m68k_disassembler::d68000_add_er_8     , 0xf1c0, 0xd000, 0xbff},
	{&m68k_disassembler::d68000_add_er_16    , 0xf1c0, 0xd040, 0xfff},
	{&m68k_disassembler::d68000_add_er_32    , 0xf1c0, 0xd080, 0xfff},
	{&m68k_disassembler::d68000_add_re_8     , 0xf1c0, 0xd100, 0x3f8},
	{&m68k_disassembler::d68000_add_re_16    , 0xf1c0, 0xd140, 0x3f8},
	{&m68k_disassembler::d68000_add_re_32    , 0xf1c0, 0xd180, 0x3f8},
	{&m68k_disassembler::d68000_adda_16      , 0xf1c0, 0xd0c0, 0xfff},
	{&m68k_disassembler::d68000_adda_32      , 0xf1c0, 0xd1c0, 0xfff},
	{&m68k_disassembler::d68000_addi_8       , 0xffc0, 0x0600, 0xbf8},
	{&m68k_disassembler::d68000_addi_16      , 0xffc0, 0x0640, 0xbf8},
	{&m68k_disassembler::d68000_addi_32      , 0xffc0, 0x0680, 0xbf8},
	{&m68k_disassembler::d68000_addq_8       , 0xf1c0, 0x5000, 0xbf8},
	{&m68k_disassembler::d68000_addq_16      , 0xf1c0, 0x5040, 0xff8},
	{&m68k_disassembler::d68000_addq_32      , 0xf1c0, 0x5080, 0xff8},
	{&m68k_disassembler::d68000_addx_rr_8    , 0xf1f8, 0xd100, 0x000},
	{&m68k_disassembler::d68000_addx_rr_16   , 0xf1f8, 0xd140, 0x000},
	{&m68k_disassembler::d68000_addx_rr_32   , 0xf1f8, 0xd180, 0x000},
	{&m68k_disassembler::d68000_addx_mm_8    , 0xf1f8, 0xd108, 0x000},
	{&m68k_disassembler::d68000_addx_mm_16   , 0xf1f8, 0xd148, 0x000},
	{&m68k_disassembler::d68000_addx_mm_32   , 0xf1f8, 0xd188, 0x000},
	{&m68k_disassembler::d68000_and_er_8     , 0xf1c0, 0xc000, 0xbff},
	{&m68k_disassembler::d68000_and_er_16    , 0xf1c0, 0xc040, 0xbff},
	{&m68k_disassembler::d68000_and_er_32    , 0xf1c0, 0xc080, 0xbff},
	{&m68k_disassembler::d68000_and_re_8     , 0xf1c0, 0xc100, 0x3f8},
	{&m68k_disassembler::d68000_and_re_16    , 0xf1c0, 0xc140, 0x3f8},
	{&m68k_disassembler::d68000_and_re_32    , 0xf1c0, 0xc180, 0x3f8},
	{&m68k_disassembler::d68000_andi_to_ccr  , 0xffff, 0x023c, 0x000},
	{&m68k_disassembler::d68000_andi_to_sr   , 0xffff, 0x027c, 0x000},
	{&m68k_disassembler::d68000_andi_8       , 0xffc0, 0x0200, 0xbf8},
	{&m68k_disassembler::d68000_andi_16      , 0xffc0, 0x0240, 0xbf8},
	{&m68k_disassembler::d68000_andi_32      , 0xffc0, 0x0280, 0xbf8},
	{&m68k_disassembler::d68000_asr_s_8      , 0xf1f8, 0xe000, 0x000},
	{&m68k_disassembler::d68000_asr_s_16     , 0xf1f8, 0xe040, 0x000},
	{&m68k_disassembler::d68000_asr_s_32     , 0xf1f8, 0xe080, 0x000},
	{&m68k_disassembler::d68000_asr_r_8      , 0xf1f8, 0xe020, 0x000},
	{&m68k_disassembler::d68000_asr_r_16     , 0xf1f8, 0xe060, 0x000},
	{&m68k_disassembler::d68000_asr_r_32     , 0xf1f8, 0xe0a0, 0x000},
	{&m68k_disassembler::d68000_asr_ea       , 0xffc0, 0xe0c0, 0x3f8},
	{&m68k_disassembler::d68000_asl_s_8      , 0xf1f8, 0xe100, 0x000},
	{&m68k_disassembler::d68000_asl_s_16     , 0xf1f8, 0xe140, 0x000},
	{&m68k_disassembler::d68000_asl_s_32     , 0xf1f8, 0xe180, 0x000},
	{&m68k_disassembler::d68000_asl_r_8      , 0xf1f8, 0xe120, 0x000},
	{&m68k_disassembler::d68000_asl_r_16     , 0xf1f8, 0xe160, 0x000},
	{&m68k_disassembler::d68000_asl_r_32     , 0xf1f8, 0xe1a0, 0x000},
	{&m68k_disassembler::d68000_asl_ea       , 0xffc0, 0xe1c0, 0x3f8},
	{&m68k_disassembler::d68000_bcc_8        , 0xf000, 0x6000, 0x000},
	{&m68k_disassembler::d68000_bcc_16       , 0xf0ff, 0x6000, 0x000},
	{&m68k_disassembler::d68020_bcc_32       , 0xf0ff, 0x60ff, 0x000},
	{&m68k_disassembler::d68000_bchg_r       , 0xf1c0, 0x0140, 0xbf8},
	{&m68k_disassembler::d68000_bchg_s       , 0xffc0, 0x0840, 0xbf8},
	{&m68k_disassembler::d68000_bclr_r       , 0xf1c0, 0x0180, 0xbf8},
	{&m68k_disassembler::d68000_bclr_s       , 0xffc0, 0x0880, 0xbf8},
	{&m68k_disassembler::d68020_bfchg        , 0xffc0, 0xeac0, 0xa78},
	{&m68k_disassembler::d68020_bfclr        , 0xffc0, 0xecc0, 0xa78},
	{&m68k_disassembler::d68020_bfexts       , 0xffc0, 0xebc0, 0xa7b},
	{&m68k_disassembler::d68020_bfextu       , 0xffc0, 0xe9c0, 0xa7b},
	{&m68k_disassembler::d68020_bfffo        , 0xffc0, 0xedc0, 0xa7b},
	{&m68k_disassembler::d68020_bfins        , 0xffc0, 0xefc0, 0xa78},
	{&m68k_disassembler::d68020_bfset        , 0xffc0, 0xeec0, 0xa78},
	{&m68k_disassembler::d68020_bftst        , 0xffc0, 0xe8c0, 0xa7b},
	{&m68k_disassembler::d68881_ftrap        , 0xfff8, 0xf278, 0x000},
	{&m68k_disassembler::d68010_bkpt         , 0xfff8, 0x4848, 0x000},
	{&m68k_disassembler::d68000_bra_8        , 0xff00, 0x6000, 0x000},
	{&m68k_disassembler::d68000_bra_16       , 0xffff, 0x6000, 0x000},
	{&m68k_disassembler::d68020_bra_32       , 0xffff, 0x60ff, 0x000},
	{&m68k_disassembler::d68000_bset_r       , 0xf1c0, 0x01c0, 0xbf8},
	{&m68k_disassembler::d68000_bset_s       , 0xffc0, 0x08c0, 0xbf8},
	{&m68k_disassembler::d68000_bsr_8        , 0xff00, 0x6100, 0x000},
	{&m68k_disassembler::d68000_bsr_16       , 0xffff, 0x6100, 0x000},
	{&m68k_disassembler::d68020_bsr_32       , 0xffff, 0x61ff, 0x000},
	{&m68k_disassembler::d68000_btst_r       , 0xf1c0, 0x0100, 0xbff},
	{&m68k_disassembler::d68000_btst_s       , 0xffc0, 0x0800, 0xbfb},
	{&m68k_disassembler::d68020_callm        , 0xffc0, 0x06c0, 0x27b},
	{&m68k_disassembler::d68020_cas_8        , 0xffc0, 0x0ac0, 0x3f8},
	{&m68k_disassembler::d68020_cas_16       , 0xffc0, 0x0cc0, 0x3f8},
	{&m68k_disassembler::d68020_cas_32       , 0xffc0, 0x0ec0, 0x3f8},
	{&m68k_disassembler::d68020_cas2_16      , 0xffff, 0x0cfc, 0x000},
	{&m68k_disassembler::d68020_cas2_32      , 0xffff, 0x0efc, 0x000},
	{&m68k_disassembler::d68000_chk_16       , 0xf1c0, 0x4180, 0xbff},
	{&m68k_disassembler::d68020_chk_32       , 0xf1c0, 0x4100, 0xbff},
	{&m68k_disassembler::d68020_chk2_cmp2_8  , 0xffc0, 0x00c0, 0x27b},
	{&m68k_disassembler::d68020_chk2_cmp2_16 , 0xffc0, 0x02c0, 0x27b},
	{&m68k_disassembler::d68020_chk2_cmp2_32 , 0xffc0, 0x04c0, 0x27b},
	{&m68k_disassembler::d68040_cinv         , 0xff20, 0xf400, 0x000},
	{&m68k_disassembler::d68000_clr_8        , 0xffc0, 0x4200, 0xbf8},
	{&m68k_disassembler::d68000_clr_16       , 0xffc0, 0x4240, 0xbf8},
	{&m68k_disassembler::d68000_clr_32       , 0xffc0, 0x4280, 0xbf8},
	{&m68k_disassembler::d68000_cmp_8        , 0xf1c0, 0xb000, 0xbff},
	{&m68k_disassembler::d68000_cmp_16       , 0xf1c0, 0xb040, 0xfff},
	{&m68k_disassembler::d68000_cmp_32       , 0xf1c0, 0xb080, 0xfff},
	{&m68k_disassembler::d68000_cmpa_16      , 0xf1c0, 0xb0c0, 0xfff},
	{&m68k_disassembler::d68000_cmpa_32      , 0xf1c0, 0xb1c0, 0xfff},
	{&m68k_disassembler::d68000_cmpi_8       , 0xffc0, 0x0c00, 0xbf8},
	{&m68k_disassembler::d68020_cmpi_pcdi_8  , 0xffff, 0x0c3a, 0x000},
	{&m68k_disassembler::d68020_cmpi_pcix_8  , 0xffff, 0x0c3b, 0x000},
	{&m68k_disassembler::d68000_cmpi_16      , 0xffc0, 0x0c40, 0xbf8},
	{&m68k_disassembler::d68020_cmpi_pcdi_16 , 0xffff, 0x0c7a, 0x000},
	{&m68k_disassembler::d68020_cmpi_pcix_16 , 0xffff, 0x0c7b, 0x000},
	{&m68k_disassembler::d68000_cmpi_32      , 0xffc0, 0x0c80, 0xbf8},
	{&m68k_disassembler::d68020_cmpi_pcdi_32 , 0xffff, 0x0cba, 0x000},
	{&m68k_disassembler::d68020_cmpi_pcix_32 , 0xffff, 0x0cbb, 0x000},
	{&m68k_disassembler::d68000_cmpm_8       , 0xf1f8, 0xb108, 0x000},
	{&m68k_disassembler::d68000_cmpm_16      , 0xf1f8, 0xb148, 0x000},
	{&m68k_disassembler::d68000_cmpm_32      , 0xf1f8, 0xb188, 0x000},
	{&m68k_disassembler::d68020_cpbcc_16     , 0xf1c0, 0xf080, 0x000},
	{&m68k_disassembler::d68020_cpbcc_32     , 0xf1c0, 0xf0c0, 0x000},
	{&m68k_disassembler::d68020_cpdbcc       , 0xf1f8, 0xf048, 0x000},
	{&m68k_disassembler::d68020_cpgen        , 0xf1c0, 0xf000, 0x000},
	{&m68k_disassembler::d68020_cprestore    , 0xf1c0, 0xf140, 0x37f},
	{&m68k_disassembler::d68020_cpsave       , 0xf1c0, 0xf100, 0x2f8},
	{&m68k_disassembler::d68020_cpscc        , 0xf1c0, 0xf040, 0xbf8},
	{&m68k_disassembler::d68020_cptrapcc_0   , 0xf1ff, 0xf07c, 0x000},
	{&m68k_disassembler::d68020_cptrapcc_16  , 0xf1ff, 0xf07a, 0x000},
	{&m68k_disassembler::d68020_cptrapcc_32  , 0xf1ff, 0xf07b, 0x000},
	{&m68k_disassembler::d68040_cpush        , 0xff20, 0xf420, 0x000},
	{&m68k_disassembler::d68000_dbcc         , 0xf0f8, 0x50c8, 0x000},
	{&m68k_disassembler::d68000_dbra         , 0xfff8, 0x51c8, 0x000},
	{&m68k_disassembler::d68000_divs         , 0xf1c0, 0x81c0, 0xbff},
	{&m68k_disassembler::d68000_divu         , 0xf1c0, 0x80c0, 0xbff},
	{&m68k_disassembler::d68020_divl         , 0xffc0, 0x4c40, 0xbff},
	{&m68k_disassembler::d68000_eor_8        , 0xf1c0, 0xb100, 0xbf8},
	{&m68k_disassembler::d68000_eor_16       , 0xf1c0, 0xb140, 0xbf8},
	{&m68k_disassembler::d68000_eor_32       , 0xf1c0, 0xb180, 0xbf8},
	{&m68k_disassembler::d68000_eori_to_ccr  , 0xffff, 0x0a3c, 0x000},
	{&m68k_disassembler::d68000_eori_to_sr   , 0xffff, 0x0a7c, 0x000},
	{&m68k_disassembler::d68000_eori_8       , 0xffc0, 0x0a00, 0xbf8},
	{&m68k_disassembler::d68000_eori_16      , 0xffc0, 0x0a40, 0xbf8},
	{&m68k_disassembler::d68000_eori_32      , 0xffc0, 0x0a80, 0xbf8},
	{&m68k_disassembler::d68000_exg_dd       , 0xf1f8, 0xc140, 0x000},
	{&m68k_disassembler::d68000_exg_aa       , 0xf1f8, 0xc148, 0x000},
	{&m68k_disassembler::d68000_exg_da       , 0xf1f8, 0xc188, 0x000},
	{&m68k_disassembler::d68020_extb_32      , 0xfff8, 0x49c0, 0x000},
	{&m68k_disassembler::d68000_ext_16       , 0xfff8, 0x4880, 0x000},
	{&m68k_disassembler::d68000_ext_32       , 0xfff8, 0x48c0, 0x000},
	{&m68k_disassembler::d68040_fpu          , 0xffc0, 0xf200, 0x000},
	{&m68k_disassembler::d68000_illegal      , 0xffff, 0x4afc, 0x000},
	{&m68k_disassembler::d68000_jmp          , 0xffc0, 0x4ec0, 0x27b},
	{&m68k_disassembler::d68000_jsr          , 0xffc0, 0x4e80, 0x27b},
	{&m68k_disassembler::d68000_lea          , 0xf1c0, 0x41c0, 0x27b},
	{&m68k_disassembler::d68000_link_16      , 0xfff8, 0x4e50, 0x000},
	{&m68k_disassembler::d68020_link_32      , 0xfff8, 0x4808, 0x000},
	{&m68k_disassembler::d68000_lsr_s_8      , 0xf1f8, 0xe008, 0x000},
	{&m68k_disassembler::d68000_lsr_s_16     , 0xf1f8, 0xe048, 0x000},
	{&m68k_disassembler::d68000_lsr_s_32     , 0xf1f8, 0xe088, 0x000},
	{&m68k_disassembler::d68000_lsr_r_8      , 0xf1f8, 0xe028, 0x000},
	{&m68k_disassembler::d68000_lsr_r_16     , 0xf1f8, 0xe068, 0x000},
	{&m68k_disassembler::d68000_lsr_r_32     , 0xf1f8, 0xe0a8, 0x000},
	{&m68k_disassembler::d68000_lsr_ea       , 0xffc0, 0xe2c0, 0x3f8},
	{&m68k_disassembler::d68000_lsl_s_8      , 0xf1f8, 0xe108, 0x000},
	{&m68k_disassembler::d68000_lsl_s_16     , 0xf1f8, 0xe148, 0x000},
	{&m68k_disassembler::d68000_lsl_s_32     , 0xf1f8, 0xe188, 0x000},
	{&m68k_disassembler::d68000_lsl_r_8      , 0xf1f8, 0xe128, 0x000},
	{&m68k_disassembler::d68000_lsl_r_16     , 0xf1f8, 0xe168, 0x000},
	{&m68k_disassembler::d68000_lsl_r_32     , 0xf1f8, 0xe1a8, 0x000},
	{&m68k_disassembler::d68000_lsl_ea       , 0xffc0, 0xe3c0, 0x3f8},
	{&m68k_disassembler::d68000_move_8       , 0xf000, 0x1000, 0xbff},
	{&m68k_disassembler::d68000_move_16      , 0xf000, 0x3000, 0xfff},
	{&m68k_disassembler::d68000_move_32      , 0xf000, 0x2000, 0xfff},
	{&m68k_disassembler::d68000_movea_16     , 0xf1c0, 0x3040, 0xfff},
	{&m68k_disassembler::d68000_movea_32     , 0xf1c0, 0x2040, 0xfff},
	{&m68k_disassembler::d68000_move_to_ccr  , 0xffc0, 0x44c0, 0xbff},
	{&m68k_disassembler::d68010_move_fr_ccr  , 0xffc0, 0x42c0, 0xbf8},
	{&m68k_disassembler::d68000_move_to_sr   , 0xffc0, 0x46c0, 0xbff},
	{&m68k_disassembler::d68000_move_fr_sr   , 0xffc0, 0x40c0, 0xbf8},
	{&m68k_disassembler::d68000_move_to_usp  , 0xfff8, 0x4e60, 0x000},
	{&m68k_disassembler::d68000_move_fr_usp  , 0xfff8, 0x4e68, 0x000},
	{&m68k_disassembler::d68010_movec        , 0xfffe, 0x4e7a, 0x000},
	{&m68k_disassembler::d68000_movem_pd_16  , 0xfff8, 0x48a0, 0x000},
	{&m68k_disassembler::d68000_movem_pd_32  , 0xfff8, 0x48e0, 0x000},
	{&m68k_disassembler::d68000_movem_re_16  , 0xffc0, 0x4880, 0x2f8},
	{&m68k_disassembler::d68000_movem_re_32  , 0xffc0, 0x48c0, 0x2f8},
	{&m68k_disassembler::d68000_movem_er_16  , 0xffc0, 0x4c80, 0x37b},
	{&m68k_disassembler::d68000_movem_er_32  , 0xffc0, 0x4cc0, 0x37b},
	{&m68k_disassembler::d68000_movep_er_16  , 0xf1f8, 0x0108, 0x000},
	{&m68k_disassembler::d68000_movep_er_32  , 0xf1f8, 0x0148, 0x000},
	{&m68k_disassembler::d68000_movep_re_16  , 0xf1f8, 0x0188, 0x000},
	{&m68k_disassembler::d68000_movep_re_32  , 0xf1f8, 0x01c8, 0x000},
	{&m68k_disassembler::d68010_moves_8      , 0xffc0, 0x0e00, 0x3f8},
	{&m68k_disassembler::d68010_moves_16     , 0xffc0, 0x0e40, 0x3f8},
	{&m68k_disassembler::d68010_moves_32     , 0xffc0, 0x0e80, 0x3f8},
	{&m68k_disassembler::d68000_moveq        , 0xf100, 0x7000, 0x000},
	{&m68k_disassembler::d68040_move16_pi_pi , 0xfff8, 0xf620, 0x000},
	{&m68k_disassembler::d68040_move16_pi_al , 0xfff8, 0xf600, 0x000},
	{&m68k_disassembler::d68040_move16_al_pi , 0xfff8, 0xf608, 0x000},
	{&m68k_disassembler::d68040_move16_ai_al , 0xfff8, 0xf610, 0x000},
	{&m68k_disassembler::d68040_move16_al_ai , 0xfff8, 0xf618, 0x000},
	{&m68k_disassembler::d68000_muls         , 0xf1c0, 0xc1c0, 0xbff},
	{&m68k_disassembler::d68000_mulu         , 0xf1c0, 0xc0c0, 0xbff},
	{&m68k_disassembler::d68020_mull         , 0xffc0, 0x4c00, 0xbff},
	{&m68k_disassembler::d68000_nbcd         , 0xffc0, 0x4800, 0xbf8},
	{&m68k_disassembler::d68000_neg_8        , 0xffc0, 0x4400, 0xbf8},
	{&m68k_disassembler::d68000_neg_16       , 0xffc0, 0x4440, 0xbf8},
	{&m68k_disassembler::d68000_neg_32       , 0xffc0, 0x4480, 0xbf8},
	{&m68k_disassembler::d68000_negx_8       , 0xffc0, 0x4000, 0xbf8},
	{&m68k_disassembler::d68000_negx_16      , 0xffc0, 0x4040, 0xbf8},
	{&m68k_disassembler::d68000_negx_32      , 0xffc0, 0x4080, 0xbf8},
	{&m68k_disassembler::d68000_nop          , 0xffff, 0x4e71, 0x000},
	{&m68k_disassembler::d68000_not_8        , 0xffc0, 0x4600, 0xbf8},
	{&m68k_disassembler::d68000_not_16       , 0xffc0, 0x4640, 0xbf8},
	{&m68k_disassembler::d68000_not_32       , 0xffc0, 0x4680, 0xbf8},
	{&m68k_disassembler::d68000_or_er_8      , 0xf1c0, 0x8000, 0xbff},
	{&m68k_disassembler::d68000_or_er_16     , 0xf1c0, 0x8040, 0xbff},
	{&m68k_disassembler::d68000_or_er_32     , 0xf1c0, 0x8080, 0xbff},
	{&m68k_disassembler::d68000_or_re_8      , 0xf1c0, 0x8100, 0x3f8},
	{&m68k_disassembler::d68000_or_re_16     , 0xf1c0, 0x8140, 0x3f8},
	{&m68k_disassembler::d68000_or_re_32     , 0xf1c0, 0x8180, 0x3f8},
	{&m68k_disassembler::d68000_ori_to_ccr   , 0xffff, 0x003c, 0x000},
	{&m68k_disassembler::d68000_ori_to_sr    , 0xffff, 0x007c, 0x000},
	{&m68k_disassembler::d68000_ori_8        , 0xffc0, 0x0000, 0xbf8},
	{&m68k_disassembler::d68000_ori_16       , 0xffc0, 0x0040, 0xbf8},
	{&m68k_disassembler::d68000_ori_32       , 0xffc0, 0x0080, 0xbf8},
	{&m68k_disassembler::d68020_pack_rr      , 0xf1f8, 0x8140, 0x000},
	{&m68k_disassembler::d68020_pack_mm      , 0xf1f8, 0x8148, 0x000},
	{&m68k_disassembler::d68000_pea          , 0xffc0, 0x4840, 0x27b},
	{&m68k_disassembler::d68040_p000         , 0xff80, 0xf500, 0x000},
	{&m68k_disassembler::d68000_reset        , 0xffff, 0x4e70, 0x000},
	{&m68k_disassembler::d68000_ror_s_8      , 0xf1f8, 0xe018, 0x000},
	{&m68k_disassembler::d68000_ror_s_16     , 0xf1f8, 0xe058, 0x000},
	{&m68k_disassembler::d68000_ror_s_32     , 0xf1f8, 0xe098, 0x000},
	{&m68k_disassembler::d68000_ror_r_8      , 0xf1f8, 0xe038, 0x000},
	{&m68k_disassembler::d68000_ror_r_16     , 0xf1f8, 0xe078, 0x000},
	{&m68k_disassembler::d68000_ror_r_32     , 0xf1f8, 0xe0b8, 0x000},
	{&m68k_disassembler::d68000_ror_ea       , 0xffc0, 0xe6c0, 0x3f8},
	{&m68k_disassembler::d68000_rol_s_8      , 0xf1f8, 0xe118, 0x000},
	{&m68k_disassembler::d68000_rol_s_16     , 0xf1f8, 0xe158, 0x000},
	{&m68k_disassembler::d68000_rol_s_32     , 0xf1f8, 0xe198, 0x000},
	{&m68k_disassembler::d68000_rol_r_8      , 0xf1f8, 0xe138, 0x000},
	{&m68k_disassembler::d68000_rol_r_16     , 0xf1f8, 0xe178, 0x000},
	{&m68k_disassembler::d68000_rol_r_32     , 0xf1f8, 0xe1b8, 0x000},
	{&m68k_disassembler::d68000_rol_ea       , 0xffc0, 0xe7c0, 0x3f8},
	{&m68k_disassembler::d68000_roxr_s_8     , 0xf1f8, 0xe010, 0x000},
	{&m68k_disassembler::d68000_roxr_s_16    , 0xf1f8, 0xe050, 0x000},
	{&m68k_disassembler::d68000_roxr_s_32    , 0xf1f8, 0xe090, 0x000},
	{&m68k_disassembler::d68000_roxr_r_8     , 0xf1f8, 0xe030, 0x000},
	{&m68k_disassembler::d68000_roxr_r_16    , 0xf1f8, 0xe070, 0x000},
	{&m68k_disassembler::d68000_roxr_r_32    , 0xf1f8, 0xe0b0, 0x000},
	{&m68k_disassembler::d68000_roxr_ea      , 0xffc0, 0xe4c0, 0x3f8},
	{&m68k_disassembler::d68000_roxl_s_8     , 0xf1f8, 0xe110, 0x000},
	{&m68k_disassembler::d68000_roxl_s_16    , 0xf1f8, 0xe150, 0x000},
	{&m68k_disassembler::d68000_roxl_s_32    , 0xf1f8, 0xe190, 0x000},
	{&m68k_disassembler::d68000_roxl_r_8     , 0xf1f8, 0xe130, 0x000},
	{&m68k_disassembler::d68000_roxl_r_16    , 0xf1f8, 0xe170, 0x000},
	{&m68k_disassembler::d68000_roxl_r_32    , 0xf1f8, 0xe1b0, 0x000},
	{&m68k_disassembler::d68000_roxl_ea      , 0xffc0, 0xe5c0, 0x3f8},
	{&m68k_disassembler::d68010_rtd          , 0xffff, 0x4e74, 0x000},
	{&m68k_disassembler::d68000_rte          , 0xffff, 0x4e73, 0x000},
	{&m68k_disassembler::d68020_rtm          , 0xfff0, 0x06c0, 0x000},
	{&m68k_disassembler::d68000_rtr          , 0xffff, 0x4e77, 0x000},
	{&m68k_disassembler::d68000_rts          , 0xffff, 0x4e75, 0x000},
	{&m68k_disassembler::d68000_sbcd_rr      , 0xf1f8, 0x8100, 0x000},
	{&m68k_disassembler::d68000_sbcd_mm      , 0xf1f8, 0x8108, 0x000},
	{&m68k_disassembler::d68000_scc          , 0xf0c0, 0x50c0, 0xbf8},
	{&m68k_disassembler::d68000_stop         , 0xffff, 0x4e72, 0x000},
	{&m68k_disassembler::d68000_sub_er_8     , 0xf1c0, 0x9000, 0xbff},
	{&m68k_disassembler::d68000_sub_er_16    , 0xf1c0, 0x9040, 0xfff},
	{&m68k_disassembler::d68000_sub_er_32    , 0xf1c0, 0x9080, 0xfff},
	{&m68k_disassembler::d68000_sub_re_8     , 0xf1c0, 0x9100, 0x3f8},
	{&m68k_disassembler::d68000_sub_re_16    , 0xf1c0, 0x9140, 0x3f8},
	{&m68k_disassembler::d68000_sub_re_32    , 0xf1c0, 0x9180, 0x3f8},
	{&m68k_disassembler::d68000_suba_16      , 0xf1c0, 0x90c0, 0xfff},
	{&m68k_disassembler::d68000_suba_32      , 0xf1c0, 0x91c0, 0xfff},
	{&m68k_disassembler::d68000_subi_8       , 0xffc0, 0x0400, 0xbf8},
	{&m68k_disassembler::d68000_subi_16      , 0xffc0, 0x0440, 0xbf8},
	{&m68k_disassembler::d68000_subi_32      , 0xffc0, 0x0480, 0xbf8},
	{&m68k_disassembler::d68000_subq_8       , 0xf1c0, 0x5100, 0xbf8},
	{&m68k_disassembler::d68000_subq_16      , 0xf1c0, 0x5140, 0xff8},
	{&m68k_disassembler::d68000_subq_32      , 0xf1c0, 0x5180, 0xff8},
	{&m68k_disassembler::d68000_subx_rr_8    , 0xf1f8, 0x9100, 0x000},
	{&m68k_disassembler::d68000_subx_rr_16   , 0xf1f8, 0x9140, 0x000},
	{&m68k_disassembler::d68000_subx_rr_32   , 0xf1f8, 0x9180, 0x000},
	{&m68k_disassembler::d68000_subx_mm_8    , 0xf1f8, 0x9108, 0x000},
	{&m68k_disassembler::d68000_subx_mm_16   , 0xf1f8, 0x9148, 0x000},
	{&m68k_disassembler::d68000_subx_mm_32   , 0xf1f8, 0x9188, 0x000},
	{&m68k_disassembler::d68000_swap         , 0xfff8, 0x4840, 0x000},
	{&m68k_disassembler::d68000_tas          , 0xffc0, 0x4ac0, 0xbf8},
	{&m68k_disassembler::d68000_trap         , 0xfff0, 0x4e40, 0x000},
	{&m68k_disassembler::d68020_trapcc_0     , 0xf0ff, 0x50fc, 0x000},
	{&m68k_disassembler::d68020_trapcc_16    , 0xf0ff, 0x50fa, 0x000},
	{&m68k_disassembler::d68020_trapcc_32    , 0xf0ff, 0x50fb, 0x000},
	{&m68k_disassembler::d68000_trapv        , 0xffff, 0x4e76, 0x000},
	{&m68k_disassembler::d68000_tst_8        , 0xffc0, 0x4a00, 0xbf8},
	{&m68k_disassembler::d68020_tst_pcdi_8   , 0xffff, 0x4a3a, 0x000},
	{&m68k_disassembler::d68020_tst_pcix_8   , 0xffff, 0x4a3b, 0x000},
	{&m68k_disassembler::d68020_tst_i_8      , 0xffff, 0x4a3c, 0x000},
	{&m68k_disassembler::d68000_tst_16       , 0xffc0, 0x4a40, 0xbf8},
	{&m68k_disassembler::d68020_tst_a_16     , 0xfff8, 0x4a48, 0x000},
	{&m68k_disassembler::d68020_tst_pcdi_16  , 0xffff, 0x4a7a, 0x000},
	{&m68k_disassembler::d68020_tst_pcix_16  , 0xffff, 0x4a7b, 0x000},
	{&m68k_disassembler::d68020_tst_i_16     , 0xffff, 0x4a7c, 0x000},
	{&m68k_disassembler::d68000_tst_32       , 0xffc0, 0x4a80, 0xbf8},
	{&m68k_disassembler::d68020_tst_a_32     , 0xfff8, 0x4a88, 0x000},
	{&m68k_disassembler::d68020_tst_pcdi_32  , 0xffff, 0x4aba, 0x000},
	{&m68k_disassembler::d68020_tst_pcix_32  , 0xffff, 0x4abb, 0x000},
	{&m68k_disassembler::d68020_tst_i_32     , 0xffff, 0x4abc, 0x000},
	{&m68k_disassembler::d68000_unlk         , 0xfff8, 0x4e58, 0x000},
	{&m68k_disassembler::d68020_unpk_rr      , 0xf1f8, 0x8180, 0x000},
	{&m68k_disassembler::d68020_unpk_mm      , 0xf1f8, 0x8188, 0x000},
	{&m68k_disassembler::d68851_p000         , 0xffc0, 0xf000, 0x000},
	{&m68k_disassembler::d68851_pbcc16       , 0xffc0, 0xf080, 0x000},
	{&m68k_disassembler::d68851_pbcc32       , 0xffc0, 0xf0c0, 0x000},
	{&m68k_disassembler::d68851_pdbcc        , 0xfff8, 0xf048, 0x000},
	{&m68k_disassembler::d68851_p001         , 0xffc0, 0xf040, 0x000},
	{&m68k_disassembler::d68040_fbcc_16      , 0xffc0, 0xf280, 0x000},
	{&m68k_disassembler::d68040_fbcc_32      , 0xffc0, 0xf2c0, 0x000},
	{nullptr, 0, 0, 0}
};

/* Check if opcode is using a valid ea mode */
bool m68k_disassembler::valid_ea(u32 opcode, u32 mask)
{
	if(mask == 0)
		return true;

	switch(opcode & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			return (mask & 0x800) != 0;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			return (mask & 0x400) != 0;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			return (mask & 0x200) != 0;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return (mask & 0x100) != 0;
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			return (mask & 0x080) != 0;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			return (mask & 0x040) != 0;
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			return (mask & 0x020) != 0;
		case 0x38:
			return (mask & 0x010) != 0;
		case 0x39:
			return (mask & 0x008) != 0;
		case 0x3a:
			return (mask & 0x002) != 0;
		case 0x3b:
			return (mask & 0x001) != 0;
		case 0x3c:
			return (mask & 0x004) != 0;
	}
	return false;
}

/* Used by qsort */
bool m68k_disassembler::compare_nof_true_bits(const opcode_struct *aptr, const opcode_struct *bptr)
{
	u32 a = aptr->mask;
	u32 b = bptr->mask;

	a = ((a & 0xAAAA) >> 1) + (a & 0x5555);
	a = ((a & 0xCCCC) >> 2) + (a & 0x3333);
	a = ((a & 0xF0F0) >> 4) + (a & 0x0F0F);
	a = ((a & 0xFF00) >> 8) + (a & 0x00FF);

	b = ((b & 0xAAAA) >> 1) + (b & 0x5555);
	b = ((b & 0xCCCC) >> 2) + (b & 0x3333);
	b = ((b & 0xF0F0) >> 4) + (b & 0x0F0F);
	b = ((b & 0xFF00) >> 8) + (b & 0x00FF);

	return b < a; /* reversed to get greatest to least sorting */
}

/* build the opcode handler jump table */
void m68k_disassembler::build_opcode_table()
{
	std::vector<const opcode_struct *> opcode_info;
	for(unsigned int i=0; m_opcode_info[i].handler; i++)
		opcode_info.push_back(m_opcode_info + i);
	std::sort(opcode_info.begin(), opcode_info.end(), compare_nof_true_bits);

	for(u32 opcode = 0; opcode != 0x10000; opcode++)
	{
		m_instruction_table[opcode] = &m68k_disassembler::d68000_illegal; /* default to illegal */
		/* search through opcode info for a match */
		for(const opcode_struct *ostruct : opcode_info)
		{
			/* match opcode mask and allowed ea modes */
			if((opcode & ostruct->mask) == ostruct->match)
			{
				/* Handle destination ea for move instructions */
				if((ostruct->handler == &m68k_disassembler::d68000_move_8 ||
					ostruct->handler == &m68k_disassembler::d68000_move_16 ||
					ostruct->handler == &m68k_disassembler::d68000_move_32) &&
				   !valid_ea(((opcode>>9)&7) | ((opcode>>3)&0x38), 0xbf8))
					continue;
				if(valid_ea(opcode, ostruct->ea_mask))
				{
					m_instruction_table[opcode] = ostruct->handler;
					break;
				}
			}
		}
	}
}



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

m68k_disassembler::m68k_disassembler(u32 type) : m_cpu_type(type)
{
	build_opcode_table();
}

u32 m68k_disassembler::opcode_alignment() const
{
	return 2;
}

offs_t m68k_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	m_cpu_pc = pc;
	m_buffer = &opcodes;
	m_cpu_ir = read_imm_16();
	m_flags = 0;
	stream << (this->*m_instruction_table[m_cpu_ir])();
	return (m_cpu_pc - pc) | m_flags | SUPPORTED;
}
