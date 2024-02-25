// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Interdata/Perkin-Elmer Series 16 disassembler

    The instrution set supported here is that of the Model 8/16E Processor,
    which is backward-compatible with practically all of its predecessors.
    Only SETM(R) and the double-precision floating point instructions were
    relatively late additions to the 16-bit architecture; most others were
    supported as least as far back as the Model 5 (1970), though
    multiply/divide and floating point support were optional on a few
    subsequent models. Interdata's first-generation Models 3 & 4 lacked
    short-form instructions and list processing, which likely made these
    processors incompatible with most later software.

    Not supported here are the unique instruction set of the low-end
    Model 1, the unusual extended instructions provided on some Model 4 and
    Model 50 configurations, or the only partially compatible instruction
    set and extended addressing modes of Interdata's 32-bit series.

***************************************************************************/

#include "emu.h"
#include "dasm16.h"

interdata16_disassembler::interdata16_disassembler()
	: util::disasm_interface()
{
}

u32 interdata16_disassembler::opcode_alignment() const
{
	return 2;
}

namespace {

static constexpr offs_t STEP_COND = interdata16_disassembler::STEP_COND;
static constexpr offs_t STEP_OVER = interdata16_disassembler::STEP_OVER;
static constexpr offs_t STEP_OUT = interdata16_disassembler::STEP_OUT;

using namespace std::literals;

static const std::pair<std::string_view, offs_t> s_inst_table[0x100] =
{
	// 00-0F
	{ {}, 0 },
	{ "BALR"sv, STEP_OVER },
	{ "BTCR"sv, STEP_COND },
	{ "BFCR"sv, STEP_COND },
	{ "NHR"sv, 0 },
	{ "CLHR"sv, 0 },
	{ "OHR"sv, 0 },
	{ "XHR"sv, 0 },
	{ "LHR"sv, 0 },
	{ "CHR"sv, 0 },
	{ "AHR"sv, 0 },
	{ "SHR"sv, 0 },
	{ "MHR"sv, 0 },
	{ "DHR"sv, 0 },
	{ "ACHR"sv, 0 },
	{ "SCHR"sv, 0 },

	// 10-1F
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "SETMR"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// 20-2F
	{ "BTBS"sv, STEP_COND },
	{ "BTFS"sv, STEP_COND },
	{ "BFBS"sv, STEP_COND },
	{ "BFFS"sv, STEP_COND },
	{ "LIS"sv, 0 },
	{ "LCS"sv, 0 },
	{ "AIS"sv, 0 },
	{ "SIS"sv, 0 },
	{ "LER"sv, 0 },
	{ "CER"sv, 0 },
	{ "AER"sv, 0 },
	{ "SER"sv, 0 },
	{ "MER"sv, 0 },
	{ "DER"sv, 0 },
	{ "FXR"sv, 0 },
	{ "FLR"sv, 0 },

	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "LPSR"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "LDR"sv, 0 },
	{ "CDR"sv, 0 },
	{ "ADR"sv, 0 },
	{ "SDR"sv, 0 },
	{ "MDR"sv, 0 },
	{ "DDR"sv, 0 },
	{ "FXDR"sv, 0 },
	{ "FLDR"sv, 0 },

	// 40-4F
	{ "STH"sv, 0 },
	{ "BAL"sv, STEP_OVER },
	{ "BTC"sv, STEP_COND },
	{ "BFC"sv, STEP_COND },
	{ "NH"sv, 0 },
	{ "CLH"sv, 0 },
	{ "OH"sv, 0 },
	{ "XH"sv, 0 },
	{ "LH"sv, 0 },
	{ "CH"sv, 0 },
	{ "AH"sv, 0 },
	{ "SH"sv, 0 },
	{ "MH"sv, 0 },
	{ "DH"sv, 0 },
	{ "ACH"sv, 0 },
	{ "SCH"sv, 0 },

	// 50-5F
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "SETM"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// 60-6F
	{ "STE"sv, 0 },
	{ "AHM"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "ATL"sv, 0 },
	{ "ABL"sv, 0 },
	{ "RTL"sv, 0 },
	{ "RBL"sv, 0 },
	{ "LE"sv, 0 },
	{ "CE"sv, 0 },
	{ "AE"sv, 0 },
	{ "SE"sv, 0 },
	{ "ME"sv, 0 },
	{ "DE"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// 70-7F
	{ "STD"sv, 0 },
	{ "STME"sv, 0 },
	{ "LME", 0 },
	{ "LPS", 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "LD"sv, 0 },
	{ "CD"sv, 0 },
	{ "AD"sv, 0 },
	{ "SD"sv, 0 },
	{ "MD"sv, 0 },
	{ "DD"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// 80-8F
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// 90-9F
	{ "SRLS"sv, 0 },
	{ "SLLS"sv, 0 },
	{ "STBR"sv, 0 },
	{ "LBR"sv, 0 },
	{ "EXBR"sv, 0 },
	{ "EPSR"sv, 0 },
	{ "WBR"sv, 0 },
	{ "RBR"sv, 0 },
	{ "WHR"sv, 0 },
	{ "RHR"sv, 0 },
	{ "WDR"sv, 0 },
	{ "RDR"sv, 0 },
	{ "MHUR"sv, 0 },
	{ "SSR"sv, 0 },
	{ "OCR"sv, 0 },
	{ "AIR"sv, 0 },

	// A0-AF
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// B0-BF
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },

	// C0-CF
	{ "BXH"sv, 0 },
	{ "BXLE"sv, 0 },
	{ "LPSW"sv, STEP_OUT },
	{ "THI"sv, 0 },
	{ "NHI"sv, 0 },
	{ "CLHI"sv, 0 },
	{ "OHI"sv, 0 },
	{ "XHI"sv, 0 },
	{ "LHI"sv, 0 },
	{ "CHI"sv, 0 },
	{ "AHI"sv, 0 },
	{ "SHI"sv, 0 },
	{ "SRHL"sv, 0 },
	{ "SLHL"sv, 0 },
	{ "SRHA"sv, 0 },
	{ "SLHA"sv, 0 },

	// D0-DF
	{ "STM"sv, 0 },
	{ "LM"sv, 0 },
	{ "STB"sv, 0 },
	{ "LB"sv, 0 },
	{ "CLB"sv, 0 },
	{ "AL"sv, 0 },
	{ "WB"sv, 0 },
	{ "RB"sv, 0 },
	{ "WH"sv, 0 },
	{ "RH"sv, 0 },
	{ "WD"sv, 0 },
	{ "RD"sv, 0 },
	{ "MHU"sv, 0 },
	{ "SS"sv, 0 },
	{ "OC"sv, 0 },
	{ "AI"sv, 0 },

	// E0-EF
	{ {}, 0 },
	{ "SVC"sv, STEP_OVER },
	{ "SINT"sv, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ "RRL"sv, 0 },
	{ "RLL"sv, 0 },
	{ "SRL"sv, 0 },
	{ "SLL"sv, 0 },
	{ "SRA"sv, 0 },
	{ "SLA"sv, 0 },

	// F0-FF
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 },
	{ {}, 0 }
};

static const std::string_view s_b_mnemonics[32] =
{
	"NOP"sv, "BM"sv, "BP"sv, "BNZ"sv, "BO"sv, {}, {}, {}, "BC"sv, {}, {}, {}, {}, {}, {}, {},
	"B"sv, "BNM"sv, "BNP"sv, "BZ"sv, "BNO"sv, {}, {}, {}, "BNC"sv, {}, {}, {}, {}, {}, {}, {}
};

} // anonymous namespace

void interdata16_disassembler::format_s16(std::ostream &stream, s16 halfword)
{
	if (halfword < 0)
	{
		stream << '-';
		halfword = -halfword;
	}
	if (u16(halfword) <= 9)
		util::stream_format(stream, "%d", halfword);
	else
		util::stream_format(stream, "X'%X'", u16(halfword));
}

offs_t interdata16_disassembler::disassemble(std::ostream &stream, offs_t pc, const interdata16_disassembler::data_buffer &opcodes, const interdata16_disassembler::data_buffer &params)
{
	const u16 i = opcodes.r16(pc);
	auto const inst = s_inst_table[BIT(i, 8, 8)];
	offs_t flags = inst.second;
	if (inst.first.empty())
	{
		// Illegal opcodes
		util::stream_format(stream, "%-6sX'%04X'", "DC", i);
		return 2 | SUPPORTED;
	}
	else if ((i & 0xfc00) == 0x2000)
	{
		// Short conditional branches
		const std::string_view bm = s_b_mnemonics[(i & 0x0200) >> 5 | (i & 0x00f0) >> 4];
		if (!bm.empty())
			util::stream_format(stream, "%-6sX'%04X'", util::string_format("%sS", bm), (BIT(i, 8) ? pc + BIT(i, 0, 4) * 2 : pc - BIT(i, 0, 4) * 2) & 0xffff);
		else
			util::stream_format(stream, "%-6s%d,%d", inst.first, BIT(i, 4, 4), BIT(i, 0, 4));
		if (BIT(i, 4, 4) == 0)
			flags = 0;
		return 2 | flags | SUPPORTED;
	}
	else
	{
		if ((i & 0xbe00) == 0x0200)
		{
			// Conditional branches
			const std::string_view bm = s_b_mnemonics[BIT(i, 4, 5)];
			if (!bm.empty())
			{
				if (BIT(i, 14))
					util::stream_format(stream, "%-6s", bm);
				else
					util::stream_format(stream, "%-6s", util::string_format("%sR", bm));
				if (BIT(i, 4, 4) == 0)
					flags = 0;
			}
			else
				util::stream_format(stream, "%-6s%d,", inst.first, BIT(i, 4, 4));
		}
		else if ((i & 0xbf00) == 0x3300 || (i & 0xdf00) == 0xc200 || (i & 0xff00) == 0xd500) // LPS(R), LPSW, SINT, AL
			util::stream_format(stream, "%-6s", inst.first, BIT(i, 4, 4));
		else if ((i & 0xff00) == 0xe100) // SVC
			util::stream_format(stream, "%-6s%d,", inst.first, BIT(i, 4, 4));
		else
			util::stream_format(stream, "%-6sR%d,", inst.first, BIT(i, 4, 4));

		if (BIT(i, 14))
		{
			// RX or RI
			if (BIT(i, 0, 4) != 0)
			{
				format_s16(stream, opcodes.r16(pc + 2));
				util::stream_format(stream, "(R%d)", BIT(i, 0, 4));
			}
			else if ((i >= 0xc900 && i < 0xd000) || i >= 0xea00)
				format_s16(stream, opcodes.r16(pc + 2));
			else
				util::stream_format(stream, "X'%04X'", opcodes.r16(pc + 2));
			return 4 | flags | SUPPORTED;
		}
		else
		{
			// RR or SF
			if ((i & 0xf800) == 0x2000 || (i & 0xfe00) == 0x9000)
				util::stream_format(stream, "%d", BIT(i, 0, 4));
			else
				util::stream_format(stream, "R%d", BIT(i, 0, 4));
			return 2 | flags | SUPPORTED;
		}
	}
}
