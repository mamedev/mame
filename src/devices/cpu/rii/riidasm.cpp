// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series disassembler


***************************************************************************/

#include "emu.h"
#include "util/disasmintf.h"
#include "riidasm.h"

#include "util/strformat.h"

using osd::u32;
using util::BIT;
using offs_t = u32;

// FIXME: this set is an amalgam of ePG3231-EM202, EPD3332 and EPD3338
const char *const riscii_disassembler::s_regs[0x60] =
{
	"INDF0", "FSR0", "PCL", "PCM", "PCH", "BSR", "STKPTR", "BSR1",
	"INDF1", "FSR1", "ACC", "TABPTRL", "TABPTRM", "TABPTRH", "CPUCON", "STATUS",
	"TRL2", "PRODL", "PRODH", "ADOTL", "ADOTH", "UARTTX", "UARTRX", "PORTA",
	"PORTB", "PORTC", "PORTD", "PORTE", "PORTF", "PORTG", "PORTH", "PORTI",
	"PFS", "STBCON", "INTCON", "INTSTA", "TRL0L", "TRL0H", "TRL1", "TR01CON",
	"TR2CON", "TRLIR", nullptr, "POST_ID", "ADCON", "PAINTEN", "PAINTSTA", "PAWAKE",
	"UARTCON", "UARTSTA", "PORTJ", "PORTK", "DCRB", "DCRC", "DCRDE", "DCRFG",
	"DCRHI", "DCRJK", "PBCON", "PCCON", "PLLF", "T0CL", "T0CH", "SPICON",
	"SPISTA", "SPRL", "SPRM", "SPRH", "SFCR", "ADDL1~ADDL4", "ADDM1~ADDM4", "ADDH1~ADDH4",
	"ENV1~4/SPHDR", "MTCON1~4/SPHTCON", "MTRL1~4/SPHTRL", "VOCON", "TR1C", "TR2C", "ADCF", nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};

riscii_disassembler::riscii_disassembler(const char *const regs[])
	: util::disasm_interface()
	, m_regs(regs)
{
}

u32 riscii_disassembler::opcode_alignment() const
{
	return 1;
}

void riscii_disassembler::format_register(std::ostream &stream, u8 reg) const
{
	if (reg < 0x60 && m_regs[reg] != nullptr)
		stream << m_regs[reg];
	else
		util::stream_format(stream, "R%02Xh", reg);
}

void riscii_disassembler::format_immediate(std::ostream &stream, u8 data) const
{
	stream << "#";
	if (data >= 0xa0)
		stream << "0";
	util::stream_format(stream, "%02Xh", data);
}

offs_t riscii_disassembler::disassemble(std::ostream &stream, offs_t pc, const riscii_disassembler::data_buffer &opcodes, const riscii_disassembler::data_buffer &params)
{
	u16 opcode = opcodes.r16(pc);
	offs_t words = 1;

	if (BIT(opcode, 15))
	{
		if (BIT(opcode, 14))
		{
			u32 dst = (pc & 0x3e000) | (opcode & 0x1fff);
			util::stream_format(stream, "%-8s%05X", BIT(opcode, 13) ? "SCALL" : "SJMP", dst);
			if (BIT(opcode, 13))
				words |= STEP_OVER;
		}
		else
		{
			u8 preg = (opcode & 0x1f00) >> 8;
			if (BIT(opcode, 13))
			{
				util::stream_format(stream, "%-8s", "MOVPR");
				format_register(stream, opcode & 0x00ff);
				stream << ",";
				format_register(stream, preg);
			}
			else
			{
				util::stream_format(stream, "%-8s", "MOVRP");
				format_register(stream, preg);
				stream << ",";
				format_register(stream, opcode & 0x00ff);
			}
		}
	}
	else switch (opcode & 0xff00)
	{
	case 0x0000:
		if (opcode == 0x0000)
			stream << "NOP";
		else if (opcode == 0x0001)
			stream << "WDTC";
		else if (opcode == 0x0002)
			stream << "SLEP";
		else if ((opcode & 0x00f0) == 0x0020)
		{
			u32 dst = u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 1);
			util::stream_format(stream, "%-8s%05X", "LJMP", dst);
			words = 2;
		}
		else if ((opcode & 0x00f0) == 0x0030)
		{
			u32 dst = u32(opcode & 0x000f) << 16 | opcodes.r16(pc + 1);
			util::stream_format(stream, "%-8s%05X", "LCALL", dst);
			words = 2 | STEP_OVER;
		}
		else
			stream << "???";
		break;

	case 0x0200:
		util::stream_format(stream, "%-8sA,", "OR");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0300:
		util::stream_format(stream, "%-8s", "OR");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x0400:
		util::stream_format(stream, "%-8sA,", "AND");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0500:
		util::stream_format(stream, "%-8s", "AND");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x0600:
		util::stream_format(stream, "%-8sA,", "XOR");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0700:
		util::stream_format(stream, "%-8s", "XOR");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x0800:
		util::stream_format(stream, "%-8s", "COMA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0900:
		util::stream_format(stream, "%-8s", "COM");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0a00:
		util::stream_format(stream, "%-8s", "RRCA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0b00:
		util::stream_format(stream, "%-8s", "RRC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0c00:
		util::stream_format(stream, "%-8s", "RLCA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0d00:
		util::stream_format(stream, "%-8s", "RLC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0e00:
		util::stream_format(stream, "%-8s", "SWAPA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x0f00:
		util::stream_format(stream, "%-8s", "SWAP");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1000:
		util::stream_format(stream, "%-8sA,", "ADD");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1100:
		util::stream_format(stream, "%-8s", "ADD");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1200:
		util::stream_format(stream, "%-8sA,", "ADC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1300:
		util::stream_format(stream, "%-8s", "ADC");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1400:
		util::stream_format(stream, "%-8sA,", "ADDDC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1500:
		util::stream_format(stream, "%-8s", "ADDDC");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1600:
		util::stream_format(stream, "%-8sA,", "SUB");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1700:
		util::stream_format(stream, "%-8s", "SUB");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1800:
		util::stream_format(stream, "%-8sA,", "SUBB");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1900:
		util::stream_format(stream, "%-8s", "SUBB");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1a00:
		util::stream_format(stream, "%-8sA,", "SUBDB");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1b00:
		util::stream_format(stream, "%-8s", "SUBDB");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x1c00:
		util::stream_format(stream, "%-8s", "INCA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1d00:
		util::stream_format(stream, "%-8s", "INC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1e00:
		util::stream_format(stream, "%-8s", "DECA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x1f00:
		util::stream_format(stream, "%-8s", "DEC");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2000:
		util::stream_format(stream, "%-8sA,", "MOV");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2100:
		util::stream_format(stream, "%-8s", "MOV");
		format_register(stream, opcode & 0x00ff);
		stream << ",A";
		break;

	case 0x2200:
		util::stream_format(stream, "%-8s", "SHRA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2300:
		util::stream_format(stream, "%-8s", "SHLA");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2400:
		util::stream_format(stream, "%-8s", "CLR");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2500:
		util::stream_format(stream, "%-8s", "TEST");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2600:
		util::stream_format(stream, "%-8sA,", "MUL");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2700:
		util::stream_format(stream, "%-8s", "RPT");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2b00:
		switch (opcode & 0x00ff)
		{
		case 0xfe:
			stream << "RET";
			words |= STEP_OUT;
			break;

		case 0xff:
			stream << "RETI";
			words |= STEP_OUT;
			break;

		default:
			stream << "???";
			break;
		}
		break;

	case 0x2c00:
		util::stream_format(stream, "%-8s0,", "TBRD");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2d00:
		util::stream_format(stream, "%-8s1,", "TBRD"); // increment mode
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2e00:
		util::stream_format(stream, "%-8s2,", "TBRD"); // decrement mode
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x2f00:
		util::stream_format(stream, "%-8sA,", "TBRD");
		format_register(stream, opcode & 0x00ff);
		break;

	case 0x3000: case 0x3100: case 0x3200: case 0x3300:
	case 0x3400: case 0x3500: case 0x3600: case 0x3700:
	case 0x3800: case 0x3900: case 0x3a00: case 0x3b00:
	case 0x3c00: case 0x3d00: case 0x3e00: case 0x3f00:
		util::stream_format(stream, "%-8s%05X", "S0CALL", opcode & 0x0fff);
		words |= STEP_OVER;
		break;

	case 0x4000:
		util::stream_format(stream, "%-8s", "TBPTL");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4100:
		util::stream_format(stream, "%-8s", "TBPTM");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4200:
		util::stream_format(stream, "%-8s", "TBPTH");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4300:
		util::stream_format(stream, "%-8s", "BANK");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4400:
		util::stream_format(stream, "%-8sA,", "OR");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4500:
		util::stream_format(stream, "%-8sA,", "AND");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4600:
		util::stream_format(stream, "%-8sA,", "XOR");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4700:
		util::stream_format(stream, "%-8sA,", "JGE");
		format_immediate(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x4800:
		util::stream_format(stream, "%-8sA,", "JLE");
		format_immediate(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x4900:
		util::stream_format(stream, "%-8sA,", "JE");
		format_immediate(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x4a00:
		util::stream_format(stream, "%-8sA,", "ADD");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4b00:
		util::stream_format(stream, "%-8sA,", "ADC");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4c00:
		util::stream_format(stream, "%-8sA,", "SUB");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4d00:
		util::stream_format(stream, "%-8sA,", "SUBB");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4e00:
		util::stream_format(stream, "%-8sA,", "MOV");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x4f00:
		util::stream_format(stream, "%-8sA,", "MUL");
		format_immediate(stream, opcode & 0x00ff);
		break;

	case 0x5000:
		util::stream_format(stream, "%-8sA,", "JDNZ");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5100:
		util::stream_format(stream, "%-8s", "JDNZ");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5200:
		util::stream_format(stream, "%-8sA,", "JINZ");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5300:
		util::stream_format(stream, "%-8s", "JINZ");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5500:
		util::stream_format(stream, "%-8sA,", "JGE");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5600:
		util::stream_format(stream, "%-8sA,", "JLE");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5700:
		util::stream_format(stream, "%-8sA,", "JE");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%05X", (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x5800: case 0x5900: case 0x5a00: case 0x5b00:
	case 0x5c00: case 0x5d00: case 0x5e00: case 0x5f00:
		util::stream_format(stream, "%-8s", "JBC");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d,%05X", (opcode & 0x0700) >> 8, (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x6000: case 0x6100: case 0x6200: case 0x6300:
	case 0x6400: case 0x6500: case 0x6600: case 0x6700:
		util::stream_format(stream, "%-8s", "JBS");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d,%05X", (opcode & 0x0700) >> 8, (pc & 0x30000) | opcodes.r16(pc + 1));
		words = 2;
		break;

	case 0x6800: case 0x6900: case 0x6a00: case 0x6b00:
	case 0x6c00: case 0x6d00: case 0x6e00: case 0x6f00:
		util::stream_format(stream, "%-8s", "BC");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d", (opcode & 0x0700) >> 8);
		break;

	case 0x7000: case 0x7100: case 0x7200: case 0x7300:
	case 0x7400: case 0x7500: case 0x7600: case 0x7700:
		util::stream_format(stream, "%-8s", "BS");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d", (opcode & 0x0700) >> 8);
		break;

	case 0x7800: case 0x7900: case 0x7a00: case 0x7b00:
	case 0x7c00: case 0x7d00: case 0x7e00: case 0x7f00:
		util::stream_format(stream, "%-8s", "BTG");
		format_register(stream, opcode & 0x00ff);
		util::stream_format(stream, ",%d", (opcode & 0x0700) >> 8);
		break;

	default:
		stream << "???";
		break;

	}

	return words | SUPPORTED;
}
