// license:BSD-3-Clause
// copyright-holders:Sandro Ronco

#include "emu.h"
#include "hd61700d.h"

const char *const hd61700_disassembler::reg_5b[4] =  {"sx", "sy", "sz", "sz"};
const char *const hd61700_disassembler::reg_8b[8] =  {"pe", "pd", "ib", "ua", "ia", "ie", "tm", "tm"};
const char *const hd61700_disassembler::reg_16b[8] = {"ix", "iy", "iz", "us", "ss", "ky", "ky", "ky"};
const char *const hd61700_disassembler::jp_cond[8] = {"z", "nc", "lz", "uz", "nz", "c", "nlz"};

const hd61700_disassembler::dasm hd61700_disassembler::ops[256] =
{
	// 0x00
	{ "adc",  OP_MREG,    OP_MR_SIR, 1 }, { "sbc",  OP_MREG,    OP_MR_SIR, 1 },
	{ "ld",   OP_MREG,    OP_MR_SIR, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "anc",  OP_MREG,    OP_MR_SIR, 1 }, { "nac",  OP_MREG,    OP_MR_SIR, 1 },
	{ "orc",  OP_MREG,    OP_MR_SIR, 1 }, { "xrc",  OP_MREG,    OP_MR_SIR, 1 },
	{ "ad",   OP_MREG,    OP_MR_SIR, 1 }, { "sb",   OP_MREG,    OP_MR_SIR, 1 },
	{ "adb",  OP_MREG,    OP_MR_SIR, 1 }, { "sbb",  OP_MREG,    OP_MR_SIR, 1 },
	{ "an",   OP_MREG,    OP_MR_SIR, 1 }, { "na",   OP_MREG,    OP_MR_SIR, 1 },
	{ "or",   OP_MREG,    OP_MR_SIR, 1 }, { "xr",   OP_MREG,    OP_MR_SIR, 1 },

	// 0x10
	{ "st",   OP_MREG,    OP_MR_SIRI,  1 }, { "ld",   OP_MREG,  OP_MR_SIRI,1 },
	{ "stl",  OP_MREG2,   OP_NULL,     1 }, { "ldl",  OP_MREG2, OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "psr",  OP_RSIR,  OP_MREG2,  1 },
	{ "pst",  OP_REG8,   OP_MREG2,    1 }, { "pst",  OP_REG8, OP_MREG2,  1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "illegal", OP_NULL, OP_NULL, 0 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "???",  OP_MREG2, OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,     1 }, { "gsr",  OP_RSIR,  OP_MREG2,  1 },
	{ "gst",  OP_REG8,   OP_MREG2,    1 }, { "gst",  OP_REG8, OP_MREG2,  1 },

	// 0x20
	{ "st",   OP_MREG,    OP_IR_IM8_,  0 }, { "st",   OP_MREG,  OP_IR_IM8_,0 },
	{ "sti",  OP_MREG,    OP_IR_IM8_,  0 }, { "sti",  OP_MREG,  OP_IR_IM8_,0 },
	{ "std",  OP_MREG,    OP_IR_IM8_,  0 }, { "std",  OP_MREG,  OP_IR_IM8_,0 },
	{ "phs",  OP_MREG2,   OP_NULL,     0 }, { "phu",  OP_MREG2, OP_NULL,   0 },
	{ "ld",   OP_MREG,    OP_IR_IM8_,  0 }, { "ld",   OP_MREG,  OP_IR_IM8_,0 },
	{ "ldi",  OP_MREG,    OP_IR_IM8_,  0 }, { "ldi",  OP_MREG,  OP_IR_IM8_,0 },
	{ "ldd",  OP_MREG,    OP_IR_IM8_,  0 }, { "ldd",  OP_MREG,  OP_IR_IM8_,0 },
	{ "pps",  OP_MREG2,   OP_NULL,     0 }, { "ppu",  OP_MREG2, OP_NULL,   0 },

	// 0x30
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_JX_COND, OP_IM16A, 0 },
	{ "jp",   OP_JX_COND, OP_IM16A,   0 }, { "jp",   OP_IM16A,   OP_NULL,  0 },
	{ "adc",  OP_IR_IM8, OP_MREG2,   0 }, { "adc",  OP_IR_IM8, OP_MREG2, 0 },
	{ "sbc",  OP_IR_IM8, OP_MREG2,   0 }, { "sbc",  OP_IR_IM8, OP_MREG2, 0 },
	{ "ad",   OP_IR_IM8, OP_MREG2,   0 }, { "ad",   OP_IR_IM8, OP_MREG2, 0 },
	{ "sb",   OP_IR_IM8, OP_MREG2,   0 }, { "sb",   OP_IR_IM8, OP_MREG2, 0 },

	// 0x40
	{ "adc",  OP_MREG2,    OP_IM8,     1 }, { "sbc",  OP_MREG2,    OP_IM8, 1 },
	{ "ld",   OP_MREG2,    OP_IM8,     1 }, { "illegal", OP_NULL, OP_NULL, 0 },
	{ "anc",  OP_MREG2,    OP_IM8,     1 }, { "nac",  OP_MREG2,    OP_IM8, 1 },
	{ "orc",  OP_MREG2,    OP_IM8,     1 }, { "xrc",  OP_MREG2,    OP_IM8, 1 },
	{ "ad",   OP_MREG2,    OP_IM8,     1 }, { "sb",   OP_MREG2,    OP_IM8, 1 },
	{ "adb",  OP_MREG2,    OP_IM8,     1 }, { "sbb",  OP_MREG2,    OP_IM8, 1 },
	{ "an",   OP_MREG2,    OP_IM8,     1 }, { "na",   OP_MREG2,    OP_IM8, 1 },
	{ "or",   OP_MREG2,    OP_IM8,     1 }, { "xr",   OP_MREG2,    OP_IM8, 1 },

	// 0x50
	{ "st",   OP_IM8I,    OP_MREG2,  0 }, { "st",    OP_IM8,      OP_MREG2,0 },
	{ "stl",  OP_IM8_,    OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },
	{ "???",  OP_IM8_,    OP_NULL,   0 }, { "psr",   OP_RSIR,     OP_IM5,  0 },
	{ "pst",  OP_REG8_,   OP_IM8,    0 }, { "pst",   OP_REG8_,    OP_IM8,  0 },
	{ "bups", OP_IM8_,    OP_NULL,   0 }, { "bdns",  OP_IM8_,     OP_NULL, 0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },
	{ "sup",  OP_IM8_,    OP_NULL,   0 }, { "sdn",   OP_IM8_,     OP_NULL, 0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL,   OP_NULL, 0 },

	// 0x60
	{ "st",   OP_MREG2,    OP_REGIM8, 0 }, { "st",   OP_MREG2,   OP_REGIM8, 0 },
	{ "sti",  OP_MREG2,    OP_REGIM8, 0 }, { "sti",  OP_MREG2,   OP_REGIM8, 0 },
	{ "std",  OP_MREG2,    OP_REGIM8, 0 }, { "std",  OP_MREG2,   OP_REGIM8, 0 },
	{ "illegal", OP_NULL,  OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ld",   OP_MREG2,    OP_REGIM8, 0 }, { "ld",   OP_MREG2,   OP_REGIM8, 0 },
	{ "ldi",  OP_MREG2,    OP_REGIM8, 0 }, { "ldi",  OP_MREG2,   OP_REGIM8, 0 },
	{ "ldd",  OP_MREG2,    OP_REGIM8, 0 }, { "ldd",  OP_MREG2,   OP_REGIM8, 0 },
	{ "illegal", OP_NULL,  OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },

	// 0x70
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_JX_COND, OP_IM16A,  0 },
	{ "cal",  OP_JX_COND,  OP_IM16A,  0 }, { "cal",  OP_IM16A,   OP_NULL,   0 },
	{ "adc",  OP_REGIM8,   OP_MREG2,  0 }, { "adc",  OP_REGIM8,  OP_MREG2,  0 },
	{ "sbc",  OP_REGIM8,   OP_MREG2,  0 }, { "sbc",  OP_REGIM8,  OP_MREG2,  0 },
	{ "ad",   OP_REGIM8,   OP_MREG2,  0 }, { "ad ",  OP_REGIM8,  OP_MREG2,  0 },
	{ "sb",   OP_REGIM8,   OP_MREG2,  0 }, { "sb",   OP_REGIM8,  OP_MREG2,  0 },

	// 0x80
	{ "adcw", OP_MREG,    OP_MR_SIR, 1 }, { "sbcw", OP_MREG,    OP_MR_SIR, 1 },
	{ "ldw",  OP_MREG,    OP_MR_SIR, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ancw", OP_MREG,    OP_MR_SIR, 1 }, { "nacw", OP_MREG,    OP_MR_SIR, 1 },
	{ "orcw", OP_MREG,    OP_MR_SIR, 1 }, { "xrcw", OP_MREG,    OP_MR_SIR, 1 },
	{ "adw",  OP_MREG,    OP_MR_SIR, 1 }, { "sbw",  OP_MREG,    OP_MR_SIR, 1 },
	{ "adbw", OP_MREG,    OP_MR_SIR, 1 }, { "sbbw", OP_MREG,    OP_MR_SIR, 1 },
	{ "anw",  OP_MREG,    OP_MR_SIR, 1 }, { "naw",  OP_MREG,    OP_MR_SIR, 1 },
	{ "orw",  OP_MREG,    OP_MR_SIR, 1 }, { "xrw",  OP_MREG,    OP_MR_SIR, 1 },

	// 0x90
	{ "stw",  OP_MREG,    OP_MR_SIRI,1 }, { "ldw",  OP_MREG,    OP_MR_SIRI,1 },
	{ "stlw", OP_MREG2,   OP_NULL,   1 }, { "ldlw", OP_MREG2,   OP_NULL,   1 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "pre",  OP_REG16,   OP_MREG2,  1 }, { "pre",  OP_REG16,   OP_MREG2,  1 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "???",  OP_MREG2,   OP_NULL,   1 },
	{ "???",  OP_MREG2,   OP_NULL,   1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "gre",  OP_REG16,   OP_MREG2,  1 }, { "gre",  OP_REG16,   OP_MREG2,  1 },

	// 0xa0
	{ "stw",  OP_MREG,   OP_IR_IM8_, 0 }, { "stw",  OP_MREG,   OP_IR_IM8_, 0 },
	{ "stiw", OP_MREG,   OP_IR_IM8_, 0 }, { "stiw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "stdw", OP_MREG,   OP_IR_IM8_, 0 }, { "stdw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "phsw", OP_MREG2,  OP_NULL,    0 }, { "phuw", OP_MREG2,  OP_NULL,    0 },
	{ "ldw",  OP_MREG,   OP_IR_IM8_, 0 }, { "ldw",  OP_MREG,   OP_IR_IM8_, 0 },
	{ "ldiw", OP_MREG,   OP_IR_IM8_, 0 }, { "ldiw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "lddw", OP_MREG,   OP_IR_IM8_, 0 }, { "lddw", OP_MREG,   OP_IR_IM8_, 0 },
	{ "ppsw", OP_MREG2,  OP_NULL,    0 }, { "ppuw", OP_MREG2,  OP_NULL,    0 },

	// 0xb0
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_JX_COND, OP_IM7,    0 },
	{ "jr",   OP_JX_COND, OP_IM7,    0 }, { "jr",   OP_IM7,     OP_NULL,   0 },
	{ "adcw", OP_IR_IM8, OP_MREG,   0 }, { "adcw", OP_IR_IM8, OP_MREG,   0 },
	{ "sbcw", OP_IR_IM8, OP_MREG,   0 }, { "sbcw", OP_IR_IM8, OP_MREG,   0 },
	{ "adw",  OP_IR_IM8, OP_MREG,   0 }, { "adw",  OP_IR_IM8, OP_MREG,   0 },
	{ "sbw",  OP_IR_IM8, OP_MREG,   0 }, { "sbw",  OP_IR_IM8, OP_MREG,   0 },

	// 0xc0
	{ "adbcm",OP_MREG,    OP_RMSIM3, 1 }, { "sdbcm",OP_MREG,    OP_RMSIM3, 1 },
	{ "ldm",  OP_MREG,    OP_RMSIM3, 1 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "ancm", OP_MREG,    OP_RMSIM3, 1 }, { "nacm", OP_MREG,    OP_RMSIM3, 1 },
	{ "orcm", OP_MREG,    OP_RMSIM3, 1 }, { "xrcm", OP_MREG,    OP_RMSIM3, 1 },
	{ "adbm", OP_MREG,    OP_RMSIM3, 1 }, { "sbbm", OP_MREG,    OP_RMSIM3, 1 },
	{ "adbm", OP_MREG,    OP_RMSIM3, 1 }, { "sbbm", OP_MREG,    OP_RMSIM3, 1 },
	{ "anm",  OP_MREG,    OP_RMSIM3, 1 }, { "nam",  OP_MREG,    OP_RMSIM3, 1 },
	{ "orm",  OP_MREG,    OP_RMSIM3, 1 }, { "xrm",  OP_MREG,    OP_RMSIM3, 1 },

	// 0xd0
	{ "stw",  OP_IM16,    OP_MR_SIR, 0 }, { "ldw",  OP_MREG2,   OP_IM16,   0 },
	{ "stlm", OP_MREG2,   OP_IM3,    0 }, { "ldlm", OP_MREG2,   OP_IM3,    0 },
	{ "illegal", OP_NULL, OP_NULL,   0 }, { "illegal", OP_NULL, OP_NULL,   0 },
	{ "pre",  OP_REG16_,  OP_IM16,   0 }, { "pre",  OP_REG16_,  OP_IM16,   0 },
	{ "bup",  OP_NULL,    OP_NULL,   0 }, { "bnd",  OP_NULL,    OP_NULL,   0 },
	{ "???",  OP_MREG2,   OP_IM3,    0 }, { "???",  OP_MREG2,   OP_IM3,    0 },
	{ "sup",  OP_MREG,    OP_NULL,   0 }, { "sdn",  OP_MREG,    OP_NULL,   0 },
	{ "jp",   OP_MREG,    OP_NULL,   0 }, { "jp",   OP_MREG,    OP_NULL,   0 },

	// 0xe0
	{ "stm",  OP_MREG,  OP_IR_IM3,   0 }, { "stm",  OP_MREG,    OP_IR_IM3, 0 },
	{ "stim", OP_MREG,  OP_IR_IM3,   0 }, { "stim", OP_MREG,    OP_IR_IM3, 0 },
	{ "stdm", OP_MREG,  OP_IR_IM3,   0 }, { "stdm", OP_MREG,    OP_IR_IM3, 0 },
	{ "phsm", OP_MREG2, OP_IM3,      0 }, { "phum", OP_MREG2,   OP_IM3,    0 },
	{ "ldm",  OP_MREG,  OP_IR_IM3,   0 }, { "ldm",  OP_MREG,    OP_IR_IM3, 0 },
	{ "ldim", OP_MREG,  OP_IR_IM3,   0 }, { "ldim", OP_MREG,    OP_IR_IM3, 0 },
	{ "lddm", OP_MREG,  OP_IR_IM3,   0 }, { "lddm", OP_MREG,    OP_IR_IM3, 0 },
	{ "ppsm", OP_MREG2, OP_IM3,      0 }, { "ppum", OP_MREG2,   OP_IM3,    0 },

	// 0xf0
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_JX_COND, OP_NULL,   0 },
	{ "rtn",  OP_JX_COND, OP_NULL,   0 }, { "rtn",  OP_NULL,    OP_NULL,   0 },
	{ "nop",  OP_JX_COND, OP_NULL,   0 }, { "clt",  OP_NULL,    OP_NULL,   0 },
	{ "fst",  OP_NULL,    OP_NULL,   0 }, { "slw",  OP_NULL,    OP_NULL,   0 },
	{ "can",  OP_NULL,    OP_NULL,   0 }, { "rtni", OP_NULL,    OP_NULL,   0 },
	{ "off",  OP_NULL,    OP_NULL,   0 }, { "trp",  OP_NULL,    OP_NULL,   0 },
};

u8 hd61700_disassembler::opread(offs_t pc, offs_t pos, const data_buffer &opcodes)
{
	if(pc >= 0x0c00)
		return opcodes.r16(pc + pos) & 0xff;
	else
		return pos & 1 ? opcodes.r16(pc + (pos >> 1)) & 0xff : opcodes.r16(pc + (pos >> 1)) >> 8;
}

void hd61700_disassembler::dasm_im8(std::ostream &stream, offs_t pc, int arg, offs_t &pos, const data_buffer &opcodes)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		pos ++;
		util::stream_format(stream, "0x%02x", opread(pc, pos, opcodes) & 0x1f);
	}
	else
	{
		util::stream_format(stream, "%s", reg_5b[(arg>>5) & 0x03] );
	}
}


void hd61700_disassembler::dasm_im8(std::ostream &stream, offs_t pc, int arg, int arg1, offs_t &pos, const data_buffer &opcodes)
{
	if (((arg>>5) & 0x03) == 0x03)
	{
		util::stream_format(stream, "0x%02x", arg1 & 0x1f);
	}
	else
	{
		util::stream_format(stream, "%s", reg_5b[(arg>>5) & 0x03]);
	}
}


void hd61700_disassembler::dasm_arg(std::ostream &stream, uint8_t op, offs_t pc, int arg, offs_t &pos, const data_buffer &opcodes)
{
	switch( arg )
	{
		case OP_MREG:
		case OP_MREG2:
			util::stream_format( stream, "$%02u", opread(pc, pos, opcodes) & 0x1f );
			if (arg == OP_MREG2) pos ++;
			break;

		case OP_RSIR:
			util::stream_format( stream, "%s", reg_5b[(opread(pc, pos, opcodes)>>5) & 0x03] );
			break;

		case OP_REG8:
		case OP_REG8_:
			util::stream_format( stream, "%s", reg_8b[(BIT(op,0)<<2) + ((opread(pc, pos, opcodes)>>5&3))]);
			if (arg == OP_REG8_) pos ++;
			break;

		case OP_MR_SIR:
			dasm_im8(stream, pc, opread(pc, pos, opcodes), pos, opcodes);
			pos ++;
			break;

		case OP_IR_IM8:
		case OP_IR_IM8_:
			util::stream_format( stream, "(%s%s", (op&1) ? "iz": "ix", (opread(pc, pos, opcodes)&0x80) ? "-": "+");
			dasm_im8(stream, pc, opread(pc, pos, opcodes), pos, opcodes);
			util::stream_format( stream, ")");
			if (arg == OP_IR_IM8_) pos ++;
			break;

		case OP_IM8_:
			pos ++;
		case OP_IM8:
			util::stream_format( stream, "0x%02x", opread(pc, pos, opcodes) );
			pos ++;
			break;

		case OP_IM8I:
			util::stream_format( stream, "(0x%02x)", opread(pc, pos, opcodes) );
			pos ++;
			break;

		case OP_REGIM8:
			util::stream_format( stream, "(%s%s", (op&1) ? "iz": "ix", (opread(pc, pos, opcodes)&0x80) ? "-": "+");
			util::stream_format( stream, "%x)", opread(pc, pos, opcodes) & 0x1f);
			pos ++;
			break;

		case OP_JX_COND:
			util::stream_format( stream, "%s", jp_cond[op & 0x07] );
			break;


		case OP_RMSIM3:
			{
				uint8_t tmp = opread(pc, pos, opcodes);
				pos ++;
				dasm_im8(stream, pc, tmp, opread(pc, pos, opcodes), pos, opcodes);
				util::stream_format( stream, ", 0x%02x", ((tmp>>5)&7)+1);
				pos ++;
			}
			break;

		case OP_IR_IM3:
			{
				uint8_t tmp = opread(pc, pos, opcodes);
				pos ++;
				util::stream_format( stream, "(%s%s", (op&1) ? "iz": "ix", (tmp&0x80) ? "-": "+");
				dasm_im8(stream, pc, tmp, opread(pc, pos, opcodes), pos, opcodes);
				util::stream_format( stream, "), 0x%02x", ((opread(pc, pos, opcodes)>>5)&7)+1 );
				pos ++;
			}
			break;

		case OP_IM3:
			util::stream_format( stream, "0x%02x", ((opread(pc, pos, opcodes)>>5)&7)+1 );
			pos ++;
			break;

		case OP_MR_SIRI:
			util::stream_format( stream, "(");
			dasm_im8(stream, pc, opread(pc, pos, opcodes), pos, opcodes);
			util::stream_format( stream, ")");
			pos ++;
			break;

		case OP_IM7:
			{
				int tmp = opread(pc, pos, opcodes);
				if (tmp&0x80)       tmp = 0x80 - tmp;

				util::stream_format( stream, "0x%04x", (pc + tmp + (pc >= 0x0c00)) & 0xffff );
				pos ++;
			}
			break;

		case OP_IM5:
			util::stream_format( stream, "0x%02x", opread(pc, pos, opcodes)&0x1f );
			pos ++;
			break;

		case OP_REG16:
		case OP_REG16_:
			util::stream_format(stream, "%s", reg_16b[(BIT(op,0)<<2) + ((opread(pc, pos, opcodes)>>5&3))]);
			if (arg == OP_REG16_) pos ++;
			break;

		case OP_IM16:
		case OP_IM16A:
			{
				uint8_t tmp1 = opread(pc, pos, opcodes);
				pos ++;
				if (pc < 0x0c00 && arg == OP_IM16A)    pos ++;
				uint8_t tmp2 = opread(pc, pos, opcodes);
				util::stream_format(stream, "0x%04x", ((tmp2<<8) | tmp1));
				pos ++;
			}
			break;

		case OP_NULL:
			break;
	}
}

uint32_t hd61700_disassembler::get_dasmflags(uint8_t op)
{
	switch (op)
	{
		case 0x30: case 0x31: case 0x32: case 0x33: //jp
		case 0x34: case 0x35: case 0x36: case 0x37: //jp
		case 0x70: case 0x71: case 0x72: case 0x73: //cal
		case 0x74: case 0x75: case 0x76: case 0x77: //cal
		case 0xb0: case 0xb1: case 0xb2: case 0xb3: //jr
		case 0xb4: case 0xb5: case 0xb6: case 0xb7: //jr
		case 0xde:                                  //jp
		case 0xdf:                                  //jp
			return STEP_OVER;
		case 0xf0: case 0xf1: case 0xf2: case 0xf3: //rtn
		case 0xf4: case 0xf5: case 0xf6: case 0xf7: //rtn
		case 0xfd:                                  //rtni
			return STEP_OUT;
	}

	return 0;
}

u32 hd61700_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t hd61700_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const dasm *inst;
	uint32_t dasmflags;
	uint8_t op, op1;
	offs_t pos = 0;
	int type = pc >= 0x0c00;

	op = opread(pc, pos, opcodes);
	pos++;

	dasmflags = get_dasmflags(op);

	op1 = opread(pc, pos, opcodes);
	pos++;

	inst = &ops[op];

	util::stream_format(stream, "%-8s", inst->str);

	//dasm first arg
	dasm_arg(stream, op, pc, inst->arg1, pos, opcodes);

	//if present dasm second arg
	if (inst->arg2 != OP_NULL)
	{
		util::stream_format(stream, ", ");
		dasm_arg(stream, op, pc, inst->arg2, pos, opcodes);
	}

	//if required add the optional jr
	if (inst->optjr == true && BIT(op1, 7))
	{
		util::stream_format(stream, ", jr ");
		dasm_arg(stream, op, pc+1, OP_IM7, pos, opcodes);

		dasmflags = STEP_OVER;
	}

	if (pos&1) pos += type+1;

	return (pos>>1) | dasmflags | SUPPORTED;
}
