// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Oki nX-8/500S disassembler

    The decoding of instructions depends heavily on the dynamic state of
    the data descriptor flag (DD = PSWH.4 = PSW.12). This flag is
    implicitly set or cleared by accumulator load instructions.

***************************************************************************/

#include "emu.h"
#include "nx8dasm.h"

#include <sstream>


static const u16 s_dummy_psw = 0;

nx8_500s_disassembler::nx8_500s_disassembler()
	: nx8_500s_disassembler(s_dummy_psw)
{
}

nx8_500s_disassembler::nx8_500s_disassembler(const u16 &psw)
	: m_psw(psw)
{
}

offs_t nx8_500s_disassembler::opcode_alignment() const
{
	return 1;
}

u32 nx8_500s_disassembler::interface_flags() const
{
	return PAGED;
}

u32 nx8_500s_disassembler::page_address_bits() const
{
	return 16;
}


static const char *const s_reg_names[3][4] =
{
	{ "ER0", "ER1", "ER2", "ER3" },
	{ "X1", "X2", "DP", "USP" },
	{ "SSP", "LRB", "PSW", "A" }
};

static const char *const s_pr_indirect[4] =
{
	"[X1]",
	"[DP-]",
	"[DP]",
	"[DP+]"
};

static const char *const s_bit_ops[4] =
{
	"SB",
	"RB",
	"JBS",
	"JBR"
};

static const char *const s_alu_ops[2][8] =
{
	{ "SUBB", "CMPB", "ADDB", "ANDB", "ORB", "XORB", "SBCB", "ADCB" },
	{ "SUB", "CMP", "ADD", "AND", "OR", "XOR", "SBC", "ADC" }
};

static const char *const s_jconds[8] =
{
	"JGT",
	"JEQ", // alias JZ
	"JLT", // alias JCY
	"JNS",
	"JPS",
	"JGE", // alias JNC
	"JNE", // alias JNZ
	"JLE"
};

static const char *const s_signed_jconds[4] =
{
	"JLTS",
	"JLES",
	"JGTS",
	"JGES"
};

// FIXME: these lists combine MSM66573 and MSM66577 SFRs, including a few existing only on one or the other
static const char *const s_msm6657x_byte_sfr_names[256] =
{
	nullptr, nullptr, "LRBL", "LRBH", "PSWL", "PSWH", "ACCL", "ACCH",
	"TSR", "DSR", nullptr, "ROMWIN", "ROMRDY", "RAMRDY", "STPACP", "SBYCON",
	"MEMSACP", "MEMSCON", nullptr, "RTCCON", nullptr, "PRPHCON", nullptr, nullptr,
	"P0", "P1", "P2", "P3", "P4", "P5", "P6", "P7",
	"P0IO", "P1IO", "P2IO", "P3IO", "P4IO", "P5IO", "P6IO", "P7IO",
	"P0SF", "P1SF", "P2SF", "P3SF", "P4SF", "P5SF", "P6SF", "P7SF",
	"IRQ0", "IRQ1", "IRQ2", "IRQ3", "IE0", "IE1", "IE2", "IE3",
	"IP0", "IP1", "IP2", "IP3", "IP4", "IP5", "IP6", "IP7",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"FRCON", "CAPCON", nullptr, nullptr, nullptr, "CPCMCON0", "CPCMCON1", nullptr,
	"EXI0CON", "EXI1CON", "EXI2CON", nullptr, "IRQ4", "IE4", "IP8", "IP9",
	"TBCKDVR", nullptr, nullptr, nullptr, nullptr, nullptr, "TM0CON", nullptr,
	"TM1C", "TM2C", "TM1R", "TM2R", "TM1CON", "TM2CON", nullptr, nullptr,
	"TM3C", "TM3R", "TM3CON", nullptr, "TM4C", "TM4R", "TM4CON", nullptr,
	"TM5C", "TM5R", "TM5CON", nullptr, "TM6C", "TM6R", "TM6CON", nullptr,
	"ST0CON", "SR0CON", "S0BUF", "S0STAT", "ST1CON", "SR1CON", "S1CON", "S1STAT",
	nullptr, nullptr, "SIO3CON", "SIO3R", "SIO4CON", "FIFOCON", "SIN4", "SOUT4",
	"PWR0", "PWR1", "PWR2", "PWR3", "PWCY0", "PWCY1", "PWC0", "PWC1",
	"PWCON0", "PWCON1", nullptr, nullptr, "ADCON0L", "ADCON0H", "ADINT0", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"P8", "P9", "P10", "P11", "P12", nullptr, "P14", "P15",
	"P8IO", "P9IO", "P10IO", "P11IO", "P14IO", "P15IO", "P14SF", "P15SF",
	"P8SF", "P9SF", "P10SF", "P11SF", "TM9C", "TM9R", "TM9CON", "FIFOMOD",
	nullptr, nullptr, nullptr, nullptr, nullptr, "SIO5CON", "SIN5", "SOUT5",
	nullptr, nullptr, nullptr, nullptr, nullptr, "DACON", "DAR0", "DAR1",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"FLAACP", "FLACON", nullptr, nullptr, "ST6CON", "SR6CON", "S6BUF", "S6STAT",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

static const char *const s_msm6657x_word_sfr_names[128] =
{
	"SSP", "LRB", "PSW", "ACC", "DSTSR", nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"FRC", nullptr, nullptr, nullptr, nullptr, "CPCMR0", "CPCMR1", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"TBCKDV", "TM0C", "TM0R", nullptr, "TM12C", "TM12R", nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	"PWR01", "PWR23", "PWCY", "PWC", nullptr, nullptr, nullptr, nullptr,
	"ADR00", "ADR01", "ADR02", "ADR03", "ADR04", "ADR05", "ADR06", "ADR07",
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, "CPCMBFR0", "CPCMBFR0", nullptr,
	nullptr, "FLAADRS", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

void nx8_500s_disassembler::format_n8(std::ostream &stream, u8 n8)
{
	if (n8 >= 0xa0)
		stream << '0';
	util::stream_format(stream, "%02XH", n8);
}

void nx8_500s_disassembler::format_n16(std::ostream &stream, u16 n16)
{
	if (n16 >= 0xa000)
		stream << '0';
	util::stream_format(stream, "%04XH", n16);
}

void nx8_500s_disassembler::format_fix8(std::ostream &stream, u8 n8)
{
	format_n16(stream, 0x0200 + n8);
}

void nx8_500s_disassembler::format_off8(std::ostream &stream, u8 n8)
{
	stream << '\\';
	format_n8(stream, n8);
}

void nx8_500s_disassembler::format_dir16(std::ostream &stream, u16 n16)
{
	if ((n16 & 0xff00) == 0 || (n16 & 0xff00) == 0x0200)
		stream << "dir ";
	format_n16(stream, n16);
}

void nx8_500s_disassembler::format_sfr8(std::ostream &stream, u8 n8, bool word)
{
	const char *name = nullptr;
	if (word && !BIT(n8, 0))
		name = s_msm6657x_word_sfr_names[n8 >> 1];
	else if (!word)
		name = s_msm6657x_byte_sfr_names[n8];
	if (name != nullptr)
		stream << name;
	else
	{
		stream << "sfr ";
		format_n8(stream, n8);
	}
}

offs_t nx8_500s_disassembler::dasm_composite(std::ostream &stream, offs_t pc, offs_t prefix, const nx8_500s_disassembler::data_buffer &opcodes, std::string obj, bool word) const
{
	const u8 code = opcodes.r8(pc + prefix);
	switch (code)
	{
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		util::stream_format(stream, "%-8s%s.%d", s_bit_ops[BIT(~code, 3)], obj, code & 0x07);
		return (prefix + 1) | SUPPORTED;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		util::stream_format(stream, "%-8s", "MB");
		if (!BIT(code, 3))
			stream << "C,";
		util::stream_format(stream, "%s.%d", obj, code & 0x07);
		if (BIT(code, 3))
			stream << ",C";
		return (prefix + 1) | SUPPORTED;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "%-8s%s.%d,", s_bit_ops[2 + BIT(~code, 3)], obj, code & 0x07);
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | STEP_COND | SUPPORTED;

	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		util::stream_format(stream, "%-8s%s.%d,", BIT(code, 3) ? "JBSR" : "JBRS", obj, code & 0x07);
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | STEP_COND | SUPPORTED;

	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
		util::stream_format(stream, "%-8sC,%s.%d", BIT(code, 3) ? "BANDN" : "BAND", obj, code & 0x07);
		return (prefix + 1) | SUPPORTED;

	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
		util::stream_format(stream, "%-8sC,%s.%d", BIT(code, 3) ? "BORN" : "BOR", obj, code & 0x07);
		return (prefix + 1) | SUPPORTED;

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		util::stream_format(stream, "%-8sC,%s.%d", "BXOR", obj, code & 0x07);
		return (prefix + 1) | SUPPORTED;

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		if (word)
			util::stream_format(stream, "%-8s%s,%s", "MOV", s_reg_names[BIT(code, 2)][code & 0x03], obj);
		else
			util::stream_format(stream, "%-8sR%d,%s", "MOVB", code & 0x07, obj);
		return (prefix + 1) | SUPPORTED;

	case 0x80: case 0x90: case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: case 0xf0:
		util::stream_format(stream, "%-8s%s,", s_alu_ops[word][(code >> 4) & 0x07], obj);
		format_fix8(stream, opcodes.r8(pc + prefix + 1));
		return (prefix + 2) | SUPPORTED;

	case 0x81: case 0x91: case 0xa1: case 0xb1: case 0xc1: case 0xd1: case 0xe1: case 0xf1:
		util::stream_format(stream, "%-8s%s,", s_alu_ops[word][(code >> 4) & 0x07], obj);
		format_off8(stream, opcodes.r8(pc + prefix + 1));
		return (prefix + 2) | SUPPORTED;

	case 0x82: case 0x92: case 0xa2: case 0xb2: case 0xc2: case 0xd2: case 0xe2: case 0xf2:
		util::stream_format(stream, "%-8s%s,", s_alu_ops[word][(code >> 4) & 0x07], obj);
		format_sfr8(stream, opcodes.r8(pc + prefix + 1), word);
		return (prefix + 2) | SUPPORTED;

	case 0x83: case 0x93: case 0xa3: case 0xb3: case 0xc3: case 0xd3: case 0xe3: case 0xf3:
		util::stream_format(stream, "%-8s%s,#", s_alu_ops[word][(code >> 4) & 0x07], obj);
		if (word)
		{
			format_n16(stream, opcodes.r16(pc + prefix + 1));
			return (prefix + 3) | SUPPORTED;
		}
		else
		{
			format_n8(stream, opcodes.r8(pc + prefix + 1));
			return (prefix + 2) | SUPPORTED;
		}

	case 0x84: case 0x94: case 0xa4: case 0xb4: case 0xc4: case 0xd4: case 0xe4: case 0xf4:
		util::stream_format(stream, "%-8s%s,A", s_alu_ops[word][(code >> 4) & 0x07], obj);
		return (prefix + 1) | SUPPORTED;

	case 0x85: case 0x95: case 0xa5: case 0xb5: case 0xc5: case 0xd5: case 0xe5: case 0xf5:
		util::stream_format(stream, "%-8sA,%s", s_alu_ops[word][(code >> 4) & 0x07], obj);
		return (prefix + 1) | SUPPORTED;

	case 0x86:
		util::stream_format(stream, "%-8s", word ? "MOV" : "MOVB");
		format_fix8(stream, opcodes.r8(pc + prefix + 1));
		util::stream_format(stream, ",%s", obj);
		return (prefix + 2) | SUPPORTED;

	case 0x87:
		util::stream_format(stream, "%-8s", word ? "MOV" : "MOVB");
		format_off8(stream, opcodes.r8(pc + prefix + 1));
		util::stream_format(stream, ",%s", obj);
		return (prefix + 2) | SUPPORTED;

	case 0x88: case 0x89: case 0x8a: case 0x8b:
		util::stream_format(stream, "%-8s%s,%s", word ? "MOV" : "MOVB", s_pr_indirect[code & 0x03], obj);
		return (prefix + 1) | SUPPORTED;

	case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		util::stream_format(stream, "%-8s%s,%d", word ? "SLL" : "SLLB", obj, (code & 0x03) + 1);
		return (prefix + 1) | SUPPORTED;

	case 0x96:
		util::stream_format(stream, "%-8s", word ? "MOV" : "MOVB");
		format_sfr8(stream, opcodes.r8(pc + prefix + 1), word);
		util::stream_format(stream, ",%s", obj);
		return (prefix + 2) | SUPPORTED;

	case 0x97:
		util::stream_format(stream, "%-8sA,%s", word ? "MOV" : "MOVB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0x98: case 0x99:
		util::stream_format(stream, "%-8s", word ? "MOV" : "MOVB");
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		util::stream_format(stream, "[%s],%s", s_reg_names[1][code & 0x01], obj);
		return (prefix + 3) | SUPPORTED;

	case 0x9a:
	{
		u8 disp = opcodes.r8(pc + prefix + 1);
		util::stream_format(stream, "%-8s%d[%s],%s", word ? "MOV" : "MOVB", util::sext(disp, 7), BIT(disp, 7) ? "USP" : "DP", obj);
		return (prefix + 2) | SUPPORTED;
	}

	case 0x9b:
		util::stream_format(stream, "%-8s", word ? "MOV" : "MOVB");
		format_dir16(stream, opcodes.r16(pc + prefix + 1));
		util::stream_format(stream, ",%s", obj);
		return (prefix + 3) | SUPPORTED;

	case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		util::stream_format(stream, "%-8s%s,%d", word ? "SRL" : "SRLB", obj, (code & 0x03) + 1);
		return (prefix + 1) | SUPPORTED;

	case 0xa6:
		util::stream_format(stream, "%-8s%s,", word ? "TJNZ" : "TJNZB", obj);
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | SUPPORTED;

	case 0xa7:
		util::stream_format(stream, "%-8s%s,", word ? "TJZ" : "TJZB", obj);
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | SUPPORTED;

	case 0xa8:
		util::stream_format(stream, "%-8s%s", word ? "DIV" : "DIVB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xa9:
		if (obj == "A")
			util::stream_format(stream, "%-8sA", word ? "SQR" : "SQRB");
		else
			util::stream_format(stream, "%-8s%s", word ? "MUL" : "MULB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xaa:
		util::stream_format(stream, "%-8s%s,A", word ? "MOV" : "MOVB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xab:
		util::stream_format(stream, "%-8s%s,#", word ? "MOV" : "MOVB", obj);
		if (word)
		{
			format_n16(stream, opcodes.r16(pc + prefix + 1));
			return (prefix + 3) | SUPPORTED;
		}
		else
		{
			format_n8(stream, opcodes.r8(pc + prefix + 1));
			return (prefix + 2) | SUPPORTED;
		}

	case 0xac: case 0xad: case 0xae: case 0xaf:
		util::stream_format(stream, "%-8s%s,%d", word ? "ROL" : "ROLB", obj, (code & 0x03) + 1);
		return (prefix + 1) | SUPPORTED;

	case 0xb6: // dummy prefix
		util::stream_format(stream, "%-8sA,", word ? "CMPC" : "CMPCB");
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		return (prefix + 3) | SUPPORTED;

	case 0xb7: // dummy prefix
		util::stream_format(stream, "%-8sA,", word ? "LC" : "LCB");
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		return (prefix + 3) | SUPPORTED;

	case 0xb8:
		util::stream_format(stream, "%-8s%s", "SBR", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xb9:
		util::stream_format(stream, "%-8s%s", "RBR", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xba:
		util::stream_format(stream, "%-8s%s", "TBR", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xbb:
		util::stream_format(stream, "%-8s%s,C", "MBR", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		util::stream_format(stream, "%-8s%s,%d", word ? "ROR" : "RORB", obj, (code & 0x03) + 1);
		return (prefix + 1) | SUPPORTED;

	case 0xc6:
		util::stream_format(stream, "%-8s%s", word ? "INC" : "INCB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xc7:
		util::stream_format(stream, "%-8s%s", word ? "CLR" : "CLRB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xc8:
		util::stream_format(stream, "%-8sA,%s", word ? "XCHG" : "XCHGB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xc9:
		util::stream_format(stream, "%-8s[%s]", "J", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xd6:
		util::stream_format(stream, "%-8s%s", word ? "DEC" : "DECB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xd7:
		util::stream_format(stream, "%-8s%s", word ? "FILL" : "FILLB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xd8: case 0xd9:
		util::stream_format(stream, "%-8sA,[%s]", BIT(code, 0) ? "CMPCB" : "CMPC", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xda: case 0xdb:
		util::stream_format(stream, "%-8sA,[%s]", BIT(code, 0) ? "LCB" : "LC", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xe6: case 0xf6:
		util::stream_format(stream, "%-8sA,", BIT(code, 4) ? "CMPCB" : "CMPC");
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		util::stream_format(stream, "[%s]", obj);
		return (prefix + 3) | SUPPORTED;

	case 0xe7: case 0xf7:
		util::stream_format(stream, "%-8sA,", BIT(code, 4) ? "LCB" : "LC");
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		util::stream_format(stream, "[%s]", obj);
		return (prefix + 3) | SUPPORTED;

	case 0xea:
		util::stream_format(stream, "%-8s%s", "DJNZ", obj);
		if (word)
			stream << 'L';
		stream << ',';
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | STEP_COND | SUPPORTED;

	case 0xeb:
		util::stream_format(stream, "%-8s[%s]", "CAL", obj);
		return (prefix + 1) | STEP_OVER | SUPPORTED;

	case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "%-8s%s,%d", word ? "SRA" : "SRAB", obj, (code & 0x03) + 1);
		return (prefix + 1) | SUPPORTED;

	case 0xf8:
		util::stream_format(stream, "%-8s[X1+A],%s", word ? "MOV" : "MOVB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xf9:
		util::stream_format(stream, "%-8s[X1+R0],%s", word ? "MOV" : "MOVB", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xfa: // dummy prefix
		util::stream_format(stream, "%-8s", "FJ");
		format_n8(stream, opcodes.r8(pc + prefix + 3));
		stream << ':';
		format_n16(stream, opcodes.r16(pc + prefix + 1));
		return (prefix + 4) | SUPPORTED;

	case 0xfb:
		util::stream_format(stream, "%-8s%s", "DIVQ", obj);
		return (prefix + 1) | SUPPORTED;

	case 0xfc: case 0xfd: case 0xfe: case 0xff: // dummy prefix
		util::stream_format(stream, "%-8s", s_signed_jconds[code & 0x03]);
		format_n16(stream, pc + prefix + 2 + s8(opcodes.r8(pc + prefix + 1)));
		return (prefix + 2) | STEP_COND | SUPPORTED;

	default:
		stream << "unknown composite instruction";
		break;
	}
	return (prefix + 1) | SUPPORTED;
}

offs_t nx8_500s_disassembler::disassemble(std::ostream &stream, offs_t pc, const nx8_500s_disassembler::data_buffer &opcodes, const nx8_500s_disassembler::data_buffer &params)
{
	const u8 byte1 = opcodes.r8(pc);
	switch (byte1)
	{
	case 0x00:
		stream << "NOP";
		return 1 | SUPPORTED;

	case 0x01:
		stream << "RT";
		return 1 | STEP_OUT | SUPPORTED;

	case 0x02:
		stream << "RTI";
		return 1 | STEP_OUT | SUPPORTED;

	case 0x03:
		util::stream_format(stream, "%-8s", "J");
		format_n16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x04:
		util::stream_format(stream, "%-8s", "SJ");
		format_n16(stream, pc + 2 + s8(opcodes.r8(pc + 1)));
		return 2 | SUPPORTED;

	case 0x05:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sR%d,", "DJNZ", 4 + BIT(byte2, 7));
		format_n16(stream, pc + 2 + s8(byte2 | 0x80));
		return 2 | STEP_COND | SUPPORTED;
	}

	case 0x06:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		if (byte2 == 0x38)
		{
			// FCAL encoding is documented as 07 08 FadrL FadrM FadrH, but 06 38 seems be to used instead
			util::stream_format(stream, "%-8s", "FCAL");
			format_n8(stream, opcodes.r8(pc + 4));
			stream << ':';
			format_n16(stream, opcodes.r16(pc + 2));
			return 5 | STEP_OVER | SUPPORTED;
		}
		else if (byte2 == 0x39)
		{
			// FRT encoding is documented as 07 09, but 06 39 may be used instead
			stream << "FRT";
			return 2 | STEP_OUT | SUPPORTED;
		}
		else if ((byte2 & 0x30) != 0x30 && (byte2 & 0x0f) != 0)
		{
			util::stream_format(stream, "%-8s", BIT(byte2, 7) ? (BIT(byte2, 6) ? "POPU" : "POPS") : (BIT(byte2, 6) ? "PUSHU" : "PUSHS"));
			switch (byte2 & 0x3f)
			{
			case 0x0f:
				stream << "ER";
				break;

			case 0x1f:
				stream << "PR";
				break;

			case 0x2e:
				stream << "CR";
				break;

			default:
				for (int n = 0; n < 4; n++)
				{
					if (BIT(byte2, n))
					{
						if ((byte2 & ((1 << n) - 1)) != 0)
							stream << ',';
						stream << s_reg_names[(byte2 & 0x30) >> 4][n];
					}
				}
				break;
			}
			return 2 | SUPPORTED;
		}
		else
		{
			util::stream_format(stream, "%-8s", "DB");
			format_n8(stream, byte1);
			stream << ',';
			format_n8(stream, byte2);
			return 2 | SUPPORTED;
		}
	}

	case 0x07:
		util::stream_format(stream, "%-8sA", "PUSHS");
		return 1 | SUPPORTED;

	case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		util::stream_format(stream, "%-8sA,", s_alu_ops[BIT(m_psw, 12)][byte1 >> 4]);
		if (BIT(m_psw, 12))
			stream << s_reg_names[BIT(byte1, 2)][byte1 & 0x03];
		else
			util::stream_format(stream, "R%d", byte1 & 0x07);
		return 1 | SUPPORTED;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		util::stream_format(stream, "%-8sR%d,#", "MOVB", byte1 & 0x07);
		format_n8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		util::stream_format(stream, "%-8s%s,#", "MOV", s_reg_names[BIT(~byte1, 2)][byte1 & 0x03]);
		format_n16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x30: case 0x31: case 0x32: case 0x33:
		util::stream_format(stream, "%-8sA,%s", BIT(m_psw, 12) ? "ST" : "STB", s_pr_indirect[byte1 & 0x03]);
		return 1 | SUPPORTED;

	case 0x34:
		util::stream_format(stream, "%-8sA,", BIT(m_psw, 12) ? "ST" : "STB");
		format_fix8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x35:
		util::stream_format(stream, "%-8sA,", BIT(m_psw, 12) ? "ST" : "STB");
		format_fix8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x36:
		util::stream_format(stream, "%-8sA,", BIT(m_psw, 12) ? "ST" : "STB");
		format_sfr8(stream, opcodes.r8(pc + 1), BIT(m_psw, 12));
		return 2 | SUPPORTED;

	case 0x37:
		util::stream_format(stream, "%-8sA,", BIT(m_psw, 12) ? "ST" : "STB");
		format_dir16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		if (BIT(m_psw, 12))
			util::stream_format(stream, "%-8sA,%s", "ST", s_reg_names[BIT(byte1, 2)][byte1 & 0x03]);
		else
			util::stream_format(stream, "%-8sA,R%d", "STB", byte1 & 0x07);
		return 1 | SUPPORTED;

	case 0x40: case 0x41: case 0x42: case 0x43:
		util::stream_format(stream, "%-8s%s", "INC", s_reg_names[1][byte1 & 0x03]);
		return 1 | SUPPORTED;

	case 0x44: case 0x45: case 0x46: case 0x47: case 0x54: case 0x55: case 0x56: case 0x57:
		util::stream_format(stream, "%-8s", "ACAL");
		format_n16(stream, 0x1000 | (byte1 & 0x10) << 6 | (byte1 & 0x03) << 8 | opcodes.r8(pc + 1));
		return 2 | STEP_OVER | SUPPORTED;

	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8s", s_bit_ops[byte2 >> 6]);
		if (BIT(byte1, 4))
			format_fix8(stream, 0xc0 | byte2);
		else
			format_off8(stream, 0xc0 | byte2);
		util::stream_format(stream, ".%d", byte1 & 0x07);
		if (BIT(byte2, 7))
		{
			stream << ',';
			format_n16(stream, pc + 3 + s8(opcodes.r8(pc + 2)));
			return 3 | STEP_COND | SUPPORTED;
		}
		else
			return 2 | SUPPORTED;
	}

	case 0x50: case 0x51: case 0x52: case 0x53:
		util::stream_format(stream, "%-8s%s", "DEC", s_reg_names[1][byte1 & 0x03]);
		return 1 | SUPPORTED;

	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		return dasm_composite(stream, pc, 1, opcodes, s_reg_names[BIT(~byte1, 2)][byte1 & 0x03], true);

	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
		return dasm_composite(stream, pc, 1, opcodes, util::string_format("R%d", byte1 & 0x07), false);

	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		util::stream_format(stream, "%-8sA,%s", "L", s_reg_names[BIT(~byte1, 2)][byte1 & 0x03]);
		return 1 | SUPPORTED;

	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
		util::stream_format(stream, "%-8sA,R%d", "LB", byte1 & 0x07);
		return 1 | SUPPORTED;

	case 0x80: case 0x81: case 0x82: case 0x83:
	case 0x90: case 0x91: case 0x92: case 0x93:
		util::stream_format(stream, "%-8sA,%s", BIT(byte1, 4) ? "LB" : "L", s_pr_indirect[byte1 & 0x03]);
		return 1 | SUPPORTED;

	case 0x84: case 0x94:
		util::stream_format(stream, "%-8sA,", BIT(byte1, 4) ? "LB" : "L");
		format_fix8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x85: case 0x95:
		util::stream_format(stream, "%-8sA,", BIT(byte1, 4) ? "LB" : "L");
		format_off8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x86: case 0x96:
		util::stream_format(stream, "%-8sA,", BIT(byte1, 4) ? "LB" : "L");
		format_sfr8(stream, opcodes.r8(pc + 1), !BIT(byte1, 4));
		return 2 | SUPPORTED;

	case 0x87: case 0x97:
		util::stream_format(stream, "%-8sA,", BIT(byte1, 4) ? "LB" : "L");
		format_dir16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0x88: case 0x98:
		util::stream_format(stream, "%-8sA,", BIT(byte1, 4) ? "LB" : "L");
		format_n16(stream, opcodes.r16(pc + 1));
		stream << "[X1]";
		return 3 | SUPPORTED;

	case 0x89: case 0x99:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sA,%d", BIT(byte1, 4) ? "LB" : "L", util::sext(byte2, 7));
		// Documentation has these the other way around, but actual code suggests otherwise
		if (BIT(byte2, 7))
			stream << "[USP]";
		else
			stream << "[DP]";
		return 2 | SUPPORTED;
	}

	case 0x8a: case 0x9a:
		return dasm_composite(stream, pc, 1, opcodes, util::string_format("PSW%c", "LH"[BIT(byte1, 4)]), false);

	case 0x8b: case 0x9b:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		return dasm_composite(stream, pc, 2, opcodes, util::string_format("%d[%s]", util::sext(byte2, 7), BIT(byte2, 7) ? "USP" : "DP"), !BIT(byte1, 4));
	}

	case 0x8c: case 0x9c: case 0xac:
		util::stream_format(stream, "%-8sA,", s_alu_ops[BIT(m_psw, 12)][(byte1 >> 4) & 0x07]);
		format_fix8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x8d: case 0x9d: case 0xad: case 0xbd: case 0xcd: case 0xdd:
		util::stream_format(stream, "%-8sA,", s_alu_ops[BIT(m_psw, 12)][(byte1 >> 4) & 0x07]);
		format_off8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0x8e: case 0x9e: case 0xae: case 0xbe: case 0xce: case 0xde:
		util::stream_format(stream, "%-8sA,#", s_alu_ops[BIT(m_psw, 12)][(byte1 >> 4) & 0x07]);
		if (BIT(m_psw, 12))
		{
			format_n16(stream, opcodes.r16(pc + 1));
			return 3 | SUPPORTED;
		}
		else
		{
			format_n8(stream, opcodes.r8(pc + 1));
			return 2 | SUPPORTED;
		}

	case 0x8f:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "SLL" : "SLLB");
		return 1 | SUPPORTED;

	case 0x9f:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "SRL" : "SRLB");
		return 1 | SUPPORTED;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		return dasm_composite(stream, pc, 1, opcodes, s_pr_indirect[byte1 & 0x03], !BIT(byte1, 4));

	case 0xa4: case 0xb4:
	{
		std::ostringstream string;
		format_fix8(string, opcodes.r8(pc + 1));
		return dasm_composite(stream, pc, 2, opcodes, string.str(), !BIT(byte1, 4));
	}

	case 0xa5: case 0xb5:
	{
		std::ostringstream string;
		format_off8(string, opcodes.r8(pc + 1));
		return dasm_composite(stream, pc, 2, opcodes, string.str(), !BIT(byte1, 4));
	}

	case 0xa6: case 0xb6:
	{
		std::ostringstream string;
		format_sfr8(string, opcodes.r8(pc + 1), !BIT(byte1, 4));
		return dasm_composite(stream, pc, 2, opcodes, string.str(), !BIT(byte1, 4));
	}

	case 0xa7: case 0xb7:
	{
		std::ostringstream string;
		format_dir16(string, opcodes.r16(pc + 1));
		return dasm_composite(stream, pc, 3, opcodes, string.str(), !BIT(byte1, 4));
	}

	case 0xa8: case 0xa9: case 0xb8: case 0xb9:
	{
		std::ostringstream string;
		format_n16(string, opcodes.r16(pc + 1));
		util::stream_format(string, "[%s]", s_reg_names[1][byte1 & 0x01]);
		return dasm_composite(stream, pc, 3, opcodes, string.str(), !BIT(byte1, 4));
	}

	case 0xaa: case 0xba:
		return dasm_composite(stream, pc, 1, opcodes, "[X1+A]", !BIT(byte1, 4));

	case 0xab: case 0xbb:
		return dasm_composite(stream, pc, 1, opcodes, "[X1+R0]", !BIT(byte1, 4));

	case 0xaf:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "ROL" : "ROLB");
		return 1 | SUPPORTED;

	case 0xbc:
		return dasm_composite(stream, pc, 1, opcodes, "A", BIT(m_psw, 12));

	case 0xbf:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "ROR" : "RORB");
		return 1 | SUPPORTED;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3:
		util::stream_format(stream, "%-8sR%d", "INCB", byte1 & 0x03);
		return 1 | SUPPORTED;

	case 0xc4:
		util::stream_format(stream, "%-8s", "CMP");
		format_fix8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xc5:
		util::stream_format(stream, "%-8s", "CMP");
		format_off8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xc6:
		util::stream_format(stream, "%-8s", "MOV");
		format_sfr8(stream, opcodes.r8(pc + 1), true);
		stream << ",#";
		format_n16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xc7:
		util::stream_format(stream, "%-8s", "MOV");
		format_off8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n16(stream, opcodes.r16(pc + 2));
		return 4 | SUPPORTED;

	case 0xc8:
		util::stream_format(stream, "%-8sA,", BIT(m_psw, 12) ? "ST" : "STB");
		format_n16(stream, opcodes.r16(pc + 1));
		stream << "[X1]";
		return 3 | SUPPORTED;

	case 0xc9:
	{
		const u8 byte2 = opcodes.r8(pc + 1);
		util::stream_format(stream, "%-8sA,%d", BIT(m_psw, 12) ? "ST" : "STB", util::sext(byte2, 7));
		// Documentation has these the other way around, but actual code suggests otherwise
		if (BIT(byte2, 7))
			stream << "[USP]";
		else
			stream << "[DP]";
		return 2 | SUPPORTED;
	}

	case 0xca:
		stream << "RC";
		return 1 | SUPPORTED;

	case 0xcb:
		stream << "SC";
		return 1 | SUPPORTED;

	case 0xcc:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "INC" : "INCB");
		return 1 | SUPPORTED;

	case 0xcf:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "NEG" : "NEGB");
		return 1 | SUPPORTED;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3:
		util::stream_format(stream, "%-8sR%d", "DECB", byte1 & 0x03);
		return 1 | SUPPORTED;

	case 0xd4:
		util::stream_format(stream, "%-8s", "CMPB");
		format_fix8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xd5:
		util::stream_format(stream, "%-8s", "CMPB");
		format_off8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xd6:
		util::stream_format(stream, "%-8s", "MOVB");
		format_sfr8(stream, opcodes.r8(pc + 1), false);
		stream << ",#";
		format_n8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xd7:
		util::stream_format(stream, "%-8s", "MOVB");
		format_off8(stream, opcodes.r8(pc + 1));
		stream << ",#";
		format_n8(stream, opcodes.r8(pc + 2));
		return 3 | SUPPORTED;

	case 0xd8:
		stream << "RDD";
		return 1 | SUPPORTED;

	case 0xd9:
		stream << "SDD";
		return 1 | SUPPORTED;

	case 0xda:
		stream << "DI";
		return 1 | SUPPORTED;

	case 0xdb:
		stream << "EI";
		return 1 | SUPPORTED;

	case 0xdc:
		util::stream_format(stream, "%-8sA", BIT(m_psw, 12) ? "DEC" : "DECB");
		return 1 | SUPPORTED;

	case 0xdf:
		stream << "SWAP";
		return 1 | SUPPORTED;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "%-8s0:", "VCAL");
		format_n16(stream, 0x004a + (byte1 & 0x0f) * 2);
		return 1 | STEP_OVER | SUPPORTED;

	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
		util::stream_format(stream, "%-8s", s_jconds[byte1 & 0x07]);
		format_n16(stream, pc + 2 + s8(opcodes.r8(pc + 1)));
		return 2 | STEP_COND | SUPPORTED;

	case 0xf8:
		util::stream_format(stream, "%-8sA,#", "L");
		format_n16(stream, opcodes.r16(pc + 1));
		return 3 | SUPPORTED;

	case 0xf9:
		util::stream_format(stream, "%-8sA,#", "LB");
		format_n8(stream, opcodes.r8(pc + 1));
		return 2 | SUPPORTED;

	case 0xfa: case 0xfb:
		util::stream_format(stream, "%-8sA", BIT(byte1, 0) ? "CLRB" : "CLR");
		return 1 | SUPPORTED;

	case 0xfc:
		stream << "EXTND";
		return 1 | SUPPORTED;

	case 0xfd:
		util::stream_format(stream, "%-8sC", "CPL");
		return 1 | SUPPORTED;

	case 0xfe:
		util::stream_format(stream, "%-8s", "CAL");
		format_n16(stream, opcodes.r16(pc + 1));
		return 3 | STEP_OVER | SUPPORTED;

	case 0xff:
		stream << "BRK";
		return 1 | SUPPORTED;

	default: // actually can't happen
		util::stream_format(stream, "%-8s", "DB");
		format_n8(stream, byte1);
		return 1 | SUPPORTED;
	}
}
