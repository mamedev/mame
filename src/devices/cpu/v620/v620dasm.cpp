// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Varian (Data Machines) 620 series disassembler

***************************************************************************/

#include "emu.h"
#include "v620dasm.h"

#include <array>
#include <unordered_map>

v620_disassembler::v620_disassembler()
	: util::disasm_interface()
{
}

v75_disassembler::v75_disassembler()
	: v620_disassembler()
{
}

u32 v620_disassembler::opcode_alignment() const
{
	return 1;
}


namespace {

static const char *const s_alu_ops[16] =
{
	"000", "LDA", "LDB", "LDX", "INR", "STA", "STB", "STX",
	"100", "ORA", "ADD", "ERA", "SUB", "ANA", "MUL", "DIV"
};

static const char *const s_shift_ops[4] =
{
	"ASL", "LRL", "ASR", "LSR"
};

static const char *const s_in_out_ops[12] =
{
	"IME", "INA", "INB", "INAB",
	"024", "CIA", "CIB", "CIAB",
	"OME", "OAR", "OBR", "OAB"
};

static const char *const s_index_tags[2] =
{
	",X", ",B"
};

static const char *const s_reg_names[8] =
{
	"A", "B", "X", "R3", "R4", "R5", "R6", "R7"
	// A, B and X are legacy names for V75's R0, R1 and R2. These may also be written as plain
	// numbers, though 1 = R2 and 2 = R1 to remain compatible with older indexing syntax.
};

static const char *const s_reg_mem_ops[4] =
{
	"LD", "ST", "AD", "SB"
};

static const char *const s_dp_ops[7] =
{
	"DLD", "DST", "DADD", "DSUB", "DAN", "DOR", "DER"
};

static const char *const s_reg_jumps[6] =
{
	"JZ", "JNZ", "JN", "JP", "JDZ", "JDNZ"
};

static const char *const s_single_reg_ops[3] =
{
	"INC", "DEC", "COM"
};

static const char *const s_rr_ops[3] =
{
	"ADR", "SBR", "T"
};

const std::unordered_map<u16, std::array<const char *, 3>> s_cond_map =
{
	{ 0000, { "JMP", "JMPM", "XEC" } },     // unconditional
	{ 0001, { "JOF", "JOFM", "XOF" } },     // overflow set
	{ 0002, { "JAP", "JAPM", "XAP" } },     // A positive
	{ 0004, { "JAN", "JANM", "XAN" } },     // A negative
	{ 0007, { "JOFN", "JOFNM", "XOFN" } },  // overflow not set (620/f)
	{ 0010, { "JAZ", "JAZM", "XAZ" } },     // A zero
	{ 0016, { "JANZ", "JANZM", "XANZ" } },  // A not zero (620/f)
	{ 0020, { "JBZ", "JBZM", "XBZ" } },     // B zero
	{ 0026, { "JBNZ", "JBNZM", "XBNZ" } },  // B not zero (620/f)
	{ 0040, { "JXZ", "JXZM", "XXZ" } },     // X zero
	{ 0046, { "JXNZ", "JXNZM", "XXNZ" } },  // X not zero (620/f)
	{ 0100, { "JSS1", "JS1M", "XS1M" } },   // sense switch 1 set
	{ 0106, { "JS1N", "JS1NM", "XS1NM" } }, // sense switch 1 not set (620/f)
	{ 0200, { "JSS2", "JS2M", "XS2M" } },   // sense switch 2 set
	{ 0206, { "JS2N", "JS2NM", "XS2NM" } }, // sense switch 2 not set (620/f)
	{ 0400, { "JSS3", "JS3M", "XS3M" } },   // sense switch 3 set
	{ 0406, { "JS3N", "JS3NM", "XS3NM" } }  // sense switch 3 not set (620/f)
};

const std::unordered_map<u8, const char *> s_fpp_map =
{
	{ 0001, "FDV" },
	{ 0010, "FAD" },
	{ 0016, "FMU" },
	{ 0020, "FLD" },
	{ 0025, "FLT" },
	{ 0050, "FSB" },
	{ 0103, "FADD" },
	{ 0106, "FMUD" },
	{ 0122, "FLDD" },
	{ 0135, "FDVD" },
	{ 0143, "FSBD" },
	{ 0200, "FST" },
	{ 0221, "FIX" },
	{ 0310, "FSTD" }
};

} // anonymous namespace

void v620_disassembler::format_number(std::ostream &stream, u16 n) const
{
	if (n > 7)
		stream << '0';
	util::stream_format(stream, "%o", n);
}

void v620_disassembler::format_address(std::ostream &stream, u16 addr) const
{
	if (addr >= 010000)
		stream << '0';
	util::stream_format(stream, "%05o", addr);
}

offs_t v620_disassembler::dasm_004xxx(std::ostream &stream, u16 inst, offs_t pc, const v620_disassembler::data_buffer &opcodes) const
{
	if (inst < 004600)
	{
		// Shift instruction group
		if (BIT(inst, 8))
			util::stream_format(stream, "L%-7s", s_shift_ops[BIT(inst, 5, 2)]);
		else
			util::stream_format(stream, "%s%-5c", s_shift_ops[BIT(inst, 5, 2)], BIT(inst, 7) ? 'A' : 'B');
		util::stream_format(stream, "%d", BIT(inst, 0, 5));
	}
	else
	{
		util::stream_format(stream, "%-8s", "DATA");
		format_number(stream, inst);
	}
	return 1 | SUPPORTED;
}

offs_t v75_disassembler::dasm_004xxx(std::ostream &stream, u16 inst, offs_t pc, const v75_disassembler::data_buffer &opcodes) const
{
	if (inst >= 004600 && BIT(inst, 3, 3) != 7)
	{
		// Double-precision instructions
		u16 addr = opcodes.r16(pc + 1);
		util::stream_format(stream, "%-7s ", util::string_format("%s%s,%s", s_dp_ops[BIT(inst, 3, 3)], BIT(addr, 15) ? "*" : "", s_reg_names[BIT(inst, 6) ? 4 : 0]));
		if (BIT(inst, 0, 3) == 0)
			format_address(stream, addr & 077777);
		else
		{
			format_number(stream, addr & 077777);
			stream << ',' << s_reg_names[BIT(inst, 0, 3)];
		}
		return 2 | SUPPORTED;
	}
	else
		return v620_disassembler::dasm_004xxx(stream, inst, pc, opcodes);
}

offs_t v620_disassembler::dasm_misc(std::ostream &stream, u16 inst, offs_t pc, const v620_disassembler::data_buffer &opcodes) const
{
	if ((inst & 0177700) == 006400)
	{
		// Bit test (620/f)
		u16 dest = opcodes.r16(pc + 1);
		util::stream_format(stream, "%s%-6c", "BT", BIT(dest, 15) ? '*' : ' ');
		format_number(stream, BIT(inst, 0, 6));
		stream << ',';
		format_address(stream, dest & 077777);
		return 2 | STEP_COND | SUPPORTED;
	}
	else if (inst == 006505 || inst == 006506)
	{
		// Jump and set return in index (620/f)
		u16 dest = opcodes.r16(pc + 1);
		util::stream_format(stream, "%s%-5c", "JSR", BIT(dest, 15) ? '*' : ' ');
		format_address(stream, dest & 077777);
		stream << s_index_tags[inst - 006505];
		return 2 | STEP_OVER | SUPPORTED;
	}
	else if ((inst & 0177704) == 006604)
	{
		// Skip on register equal (620/f)
		u8 mode = BIT(inst, 0, 3);
		u16 addr = opcodes.r16(pc + 1);
		util::stream_format(stream, "%s%-5c", "SRE", BIT(addr, 15) ? '*' : ' ');
		if (mode == 5 || mode == 6)
		{
			format_number(stream, addr & 077777);
			stream << s_index_tags[mode - 5];
		}
		else
			format_address(stream, (mode == 4 ? pc + 1 + addr : addr) & 077777);
		stream << ',';
		format_number(stream, inst & 000070);
		return 2 | STEP_COND | SUPPORTED;
	}
	else if ((inst & 0177774) == 006704)
	{
		// Indexed jump (620/f)
		u8 mode = BIT(inst, 0, 3);
		u16 dest = opcodes.r16(pc + 1);
		util::stream_format(stream, "%s%-4c", "IJMP", BIT(dest, 15) ? '*' : ' ');
		if (mode == 5 || mode == 6)
		{
			format_number(stream, dest & 077777);
			stream << s_index_tags[mode - 5];
		}
		else
			format_address(stream, (mode == 4 ? pc + 1 + dest : dest) & 077777);
		return 2 | SUPPORTED;
	}
	else
	{
		if (inst == 007400)
			stream << "ROF";
		else if (inst == 007401)
			stream << "SOF";
		else if (inst == 007402)
			stream << "TSA"; // switches to A (620/f)
		else
		{
			util::stream_format(stream, "%-8s", "DATA");
			format_number(stream, inst);
		}
		return 1 | SUPPORTED;
	}
}

offs_t v75_disassembler::dasm_misc(std::ostream &stream, u16 inst, offs_t pc, const v75_disassembler::data_buffer &opcodes) const
{
	if (inst >= 007500)
	{
		// Register-to-register instructions
		util::stream_format(stream, "%s,%s,%s", s_rr_ops[BIT(inst, 6, 2) - 1], s_reg_names[BIT(inst, 3, 3)], s_reg_names[BIT(inst, 0, 3)]);
		return 1 | SUPPORTED;
	}
	else if (inst >= 007460)
	{
		// Byte instructions (register is always R0, i.e. A)
		u16 addr = opcodes.r16(pc + 1);
		util::stream_format(stream, "%-8s", util::string_format("%cBT%c", BIT(inst, 3) ? 'S' : 'L', BIT(addr, 15) ? '*' : ' '));
		if (BIT(inst, 0, 3) == 0)
			format_address(stream, addr & 077777);
		else
		{
			format_number(stream, addr & 077777);
			stream << ',' << s_reg_names[BIT(inst, 0, 3)];
		}
		return 2 | SUPPORTED;
	}
	else if (inst >= 007440)
	{
		// Immediate instructions
		util::stream_format(stream, "%-8s", util::string_format("%sI,%s", BIT(inst, 3) ? "AD" : "LD", s_reg_names[BIT(inst, 0, 3)]));
		format_number(stream, opcodes.r16(pc + 1));
		return 2 | SUPPORTED;
	}
	else if (inst >= 007410)
	{
		// Single-register instructions
		util::stream_format(stream, "%s,%s", s_single_reg_ops[BIT(inst, 3, 2) - 1], s_reg_names[BIT(inst, 0, 3)]);
		return 1 | SUPPORTED;
	}
	else if ((inst & 0177400) == 007000)
	{
		// Register-to-memory instructions
		u16 addr = opcodes.r16(pc + 1);
		util::stream_format(stream, "%-8s", util::string_format("%s%s,%s", s_reg_mem_ops[BIT(inst, 6, 2)], BIT(addr, 15) ? "*" : "", s_reg_names[BIT(inst, 3, 3)]));
		if (BIT(inst, 0, 3) == 0)
			format_address(stream, addr & 077777);
		else
		{
			format_number(stream, addr & 077777);
			stream << ',' << s_reg_names[BIT(inst, 0, 3)];
		}
		return 2 | SUPPORTED;
	}
	else if (inst >= 06720 && inst < 07000)
	{
		// Jump-if instructions
		u16 dest = opcodes.r16(pc + 1);
		util::stream_format(stream, "%-8s", util::string_format("%s%s,%s", s_reg_jumps[BIT(inst, 3, 3) - 2], BIT(dest, 15) ? "*" : "", s_reg_names[BIT(inst, 0, 3)]));
		format_address(stream, dest & 077777);
		return 2 | STEP_COND | SUPPORTED;
	}
	else
		return v620_disassembler::dasm_misc(stream, inst, pc, opcodes);
}

offs_t v620_disassembler::dasm_io(std::ostream &stream, u16 inst, offs_t pc, const v620_disassembler::data_buffer &opcodes) const
{
	if (inst < 0101000)
	{
		// External control
		util::stream_format(stream, "%-8s", "EXC");
		format_number(stream, BIT(inst, 0, 9));
		return 1 | SUPPORTED;
	}
	else if (inst < 0102000)
	{
		u16 dest = opcodes.r16(pc + 1);
		util::stream_format(stream, "%s%-5s", "SEN", BIT(dest, 15) ? '*' : ' ');
		format_number(stream, BIT(inst, 0, 9));
		stream << ',';
		format_address(stream, dest & 077777);
		return 2 | STEP_COND | SUPPORTED;
	}
	else if (inst < 0103400 && BIT(inst, 9, 4) != 4)
	{
		util::stream_format(stream, "%-8s", s_in_out_ops[BIT(inst, 9, 4)]);
		format_number(stream, BIT(inst, 0, 6));
		if (BIT(inst, 9, 3) == 0)
		{
			stream << ',';
			format_address(stream, opcodes.r16(pc + 1) & 077777);
			return 2 | SUPPORTED;
		}
		else
			return 1 | SUPPORTED;
	}
	else
	{
		util::stream_format(stream, "%-8s", "DATA");
		format_number(stream, inst);
		return 1 | SUPPORTED;
	}
}

offs_t v75_disassembler::dasm_io(std::ostream &stream, u16 inst, offs_t pc, const v620_disassembler::data_buffer &opcodes) const
{
	if ((inst & 0177000) == 0104000)
	{
		util::stream_format(stream, "%-8s", "EXC2");
		format_number(stream, BIT(inst, 0, 9));
		return 1 | SUPPORTED;
	}

	if ((inst & 0177400) == 0105400)
	{
		// Floating point processor option
		auto lookup = s_fpp_map.find(BIT(inst, 0, 8));
		if (lookup != s_fpp_map.end())
		{
			u16 addr = opcodes.r16(pc + 1);
			if (BIT(addr, 15))
				util::stream_format(stream, "%-8s", std::string(lookup->second) + "*");
			else
				util::stream_format(stream, "%-8s", lookup->second);
			format_address(stream, addr & 077777);
			return 2 | SUPPORTED;
		}
	}

	return v620_disassembler::dasm_io(stream, inst, pc, opcodes);
}

offs_t v620_disassembler::disassemble(std::ostream &stream, offs_t pc, const v620_disassembler::data_buffer &opcodes, const v620_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);
	if (BIT(inst, 12, 3) != 0)
	{
		// Single-word addressing instructions
		u8 mode = BIT(inst, 9, 3);
		util::stream_format(stream, "%s%-5c", s_alu_ops[BIT(inst, 12, 4)], mode == 7 ? '*' : ' ');
		switch (mode)
		{
		case 0: case 1: case 2: case 3:
			format_address(stream, BIT(inst, 0, 11));
			break;

		case 4:
			format_address(stream, (pc + 1 + BIT(inst, 0, 9)) & 077777);
			break;

		case 5: case 6:
			format_number(stream, BIT(inst, 0, 9));
			stream << s_index_tags[mode - 5];
			break;

		case 7:
			format_address(stream, BIT(inst, 0, 9));
			break;
		}
		return 1 | SUPPORTED;
	}
	else if (inst >= 0100000)
		return dasm_io(stream, inst, pc, opcodes);
	else if (inst >= 006000)
	{
		if (inst < (BIT(inst, 2) ? 006400 : 006200) && BIT(inst, 3, 3) != 0)
		{
			// Extended-addressing instruction group
			if (BIT(inst, 2))
			{
				u8 mode = BIT(inst, 0, 3);
				u16 addr = opcodes.r16(pc + 1);
				util::stream_format(stream, "%sE%-4c", s_alu_ops[BIT(inst, 3, 4)], BIT(addr, 15) ? '*' : ' ');
				if (mode == 5 || mode == 6)
				{
					format_number(stream, addr & 077777);
					stream << s_index_tags[mode - 5];
				}
				else
					format_address(stream, (mode == 4 ? pc + 1 + addr : addr) & 077777);
				if (BIT(inst, 7))
				{
					// Postindexing (620/f)
					stream << ',';
					format_number(stream, 0200);
				}
			}
			else
			{
				util::stream_format(stream, "%s%-5c", s_alu_ops[BIT(inst, 3, 4)], 'I');
				format_number(stream, opcodes.r16(pc + 1));
			}
			return 2 | SUPPORTED;
		}
		else
			return dasm_misc(stream, inst, pc, opcodes);
	}
	else if (inst >= 005000)
	{
		// Register change group
		if (BIT(inst, 6))
		{
			// Increment or decrement
			switch (inst & 000477)
			{
			case 011:
				util::stream_format(stream, "%cAR", BIT(inst, 7) ? 'D' : 'I');
				break;

			case 022:
				util::stream_format(stream, "%cBR", BIT(inst, 7) ? 'D' : 'I');
				break;

			case 044:
				util::stream_format(stream, "%cXR", BIT(inst, 7) ? 'D' : 'I');
				break;

			case 0411:
				util::stream_format(stream, "%cOFA", BIT(inst, 7) ? 'S' : 'A');
				break;

			case 0422:
				util::stream_format(stream, "%cOFB", BIT(inst, 7) ? 'S' : 'A');
				break;

			case 0444:
				util::stream_format(stream, "%cOFX", BIT(inst, 7) ? 'S' : 'A');
				break;

			default:
				util::stream_format(stream, "%-8s", BIT(inst, 7) ? "DECR" : "INCR");
				format_number(stream, inst & 000477);
				break;
			}
		}
		else
		{
			// Transfer true or complement
			switch (inst & 000677)
			{
			case 000:
				stream << "NOP";
				break;

			case 001:
				stream << "TZA";
				break;

			case 002:
				stream << "TZB";
				break;

			case 004:
				stream << "TZX";
				break;

			case 012:
				stream << "TAB";
				break;

			case 014:
				stream << "TAX";
				break;

			case 021:
				stream << "TBA";
				break;

			case 024:
				stream << "TBX";
				break;

			case 041:
				stream << "TXA";
				break;

			case 042:
				stream << "TXB";
				break;

			case 0211:
				stream << "CPA";
				break;

			case 0222:
				stream << "CPB";
				break;

			case 0244:
				stream << "CPX";
				break;

			default:
				util::stream_format(stream, "%-8s", BIT(inst, 7) ? "COMP" : BIT(inst, 3, 3) == 0 ? "ZERO" : "MERG");
				format_number(stream, inst & 000477);
				break;
			}
		}
		return 1 | SUPPORTED;
	}
	else if (inst >= 004000)
		return dasm_004xxx(stream, inst, pc, opcodes);
	else if (inst >= 001000 && inst < 004000)
	{
		// Jump and execute instructions
		u16 dest = opcodes.r16(pc + 1);
		u16 cond = BIT(inst, 0, 9);
		auto lookup = s_cond_map.find(cond);
		if (lookup != s_cond_map.end())
		{
			if (BIT(dest, 15))
				util::stream_format(stream, "%-8s", std::string(lookup->second[BIT(inst, 9, 2) - 1]) + "*");
			else
				util::stream_format(stream, "%-8s", lookup->second[BIT(inst, 9, 2) - 1]);
		}
		else
		{
			std::string name = util::string_format("%cIF", inst < 03000 ? 'J' : 'X');
			if (!BIT(inst, 9))
				name += 'M';
			if (BIT(dest, 15))
				name += '*';
			util::stream_format(stream, "%-8s", name);
			format_number(stream, cond);
			stream << ',';
		}
		format_address(stream, dest & 077777);
		if (inst == 001000 && BIT(dest, 15))
			return 2 | STEP_OUT | SUPPORTED;
		else
			return 2 | (BIT(inst, 10) ? STEP_OVER : 0) | (cond != 0 ? STEP_COND : 0) | SUPPORTED;
	}
	else
	{
		if (inst == 0)
			stream << "HLT";
		else
		{
			util::stream_format(stream, "%-8s", "DATA");
			format_number(stream, inst);
		}
		return 1 | SUPPORTED;
	}
}
