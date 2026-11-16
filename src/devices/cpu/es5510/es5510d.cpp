// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/***************************************************************************
 *
 *   es5510d.cpp - Ensoniq ES5510 (ESP) disassembler
 *
 *   The ESP executes 48-bit microinstructions. Only the most significant
 *   45 bits are used; the low three bits are always zero. The instruction
 *   fields are, from most to least significant:
 *
 *     D, C, B, A       24-bit operand register addresses
 *     ALU opcode       operation performed by the ALU
 *     operand select   operand addressing mode for the ALU and multiplier
 *     skip             instruction is subject to the skip condition
 *     MAC              multiplier accumulates into MAC
 *     RAM control      external RAM/table/I/O access for this cycle
 *
 *   Disassembly reads six bytes per instruction, most significant first.
 *   The GPR address that accompanies each step in an ESP image is stored
 *   separately and is not part of the instruction stream.
 *
 ***************************************************************************/

#include "emu.h"
#include "es5510d.h"

namespace {

// Operand select mode bits.
constexpr u8 PATH_REG = 1;
constexpr u8 PATH_DELAY = 2;
constexpr u8 PATH_BOTH = PATH_REG | PATH_DELAY;

// Operand select field contents.
struct op_select_t
{
	u8 alu_src;
	u8 alu_dst;
	u8 mac_src;
	u8 mac_dst;
};

constexpr op_select_t OPERAND_SELECT[16] =
{
	{ PATH_REG,   PATH_REG,   PATH_REG,   PATH_REG   },
	{ PATH_REG,   PATH_REG,   PATH_REG,   PATH_DELAY },
	{ PATH_REG,   PATH_REG,   PATH_REG,   PATH_BOTH  },
	{ PATH_REG,   PATH_REG,   PATH_DELAY, PATH_REG   },
	{ PATH_REG,   PATH_REG,   PATH_DELAY, PATH_BOTH  },
	{ PATH_REG,   PATH_DELAY, PATH_REG,   PATH_REG   },
	{ PATH_REG,   PATH_DELAY, PATH_DELAY, PATH_REG   },
	{ PATH_REG,   PATH_BOTH,  PATH_REG,   PATH_REG   },
	{ PATH_REG,   PATH_BOTH,  PATH_DELAY, PATH_REG   },
	{ PATH_DELAY, PATH_REG,   PATH_REG,   PATH_REG   },
	{ PATH_DELAY, PATH_REG,   PATH_REG,   PATH_DELAY },
	{ PATH_DELAY, PATH_REG,   PATH_REG,   PATH_BOTH  },
	{ PATH_DELAY, PATH_REG,   PATH_DELAY, PATH_REG   },
	{ PATH_DELAY, PATH_REG,   PATH_DELAY, PATH_BOTH  },
	{ PATH_DELAY, PATH_BOTH,  PATH_REG,   PATH_REG   },
	{ PATH_DELAY, PATH_BOTH,  PATH_DELAY, PATH_REG   }
};

constexpr char const *const ALU_NAME[16] =
{
	"ADD", "SUB", "ADDU", "SUBU", "CMP", "AND", "OR", "XOR",
	"ABS", "MOV", "ASL2", "ASL8", "LS15", "DIFF", "ASR", "END"
};

constexpr u8 ALU_OPERANDS[16] =
{
	2, 2, 2, 2, 2, 2, 2, 2,
	1, 1, 1, 1, 1, 1, 1, 0
};

constexpr u8 ALU_OP_CMP = 0x4;
constexpr u8 ALU_OP_MOV = 0x9;

constexpr u8 REG_DIL_MEMSIZ = 0xf4;
constexpr u8 REG_ZERO = 0xff;

// Names for registers $ea through $ff. $f4 is handled separately because
// reading it returns DIL while writing it sets MEMSIZ.
constexpr char const *const SPECIAL_REG_NAME[22] =
{
	"SER0R", "SER0L", "SER1R", "SER1L", "SER2R", "SER2L", "SER3R", "SER3L",
	"MACL", "MAC", "DIL/MEMSIZ", "DLENGTH", "ABASE", "BBASE", "DBASE",
	"SIGREG", "CCR", "CMR", "MINUS1", "MIN", "MAX", "ZERO"
};

constexpr char const *const RAM_CONTROL_NAME[8] =
{
	"RD dd[]", "WR dd[]", "RD ta[]", "WR ta[]",
	"RD tb[]", "RD dd[] (dump)", "RD io[]", "WR io[]"
};

void write_register(std::ostream &stream, u8 reg, bool destination)
{
	if (reg < 0xea)
	{
		util::stream_format(stream, "r%02x", reg);
	}
	else if (reg == REG_DIL_MEMSIZ)
	{
		stream << (destination ? "MEMSIZ" : "DIL");
	}
	else
	{
		stream << SPECIAL_REG_NAME[reg - 0xea];
	}
}

void write_path(std::ostream &stream, u8 reg, u8 path, bool destination)
{
	if (path & PATH_REG)
		write_register(stream, reg, destination);
	if (path == PATH_BOTH)
		stream << ", ";
	if (path & PATH_DELAY)
		stream << "Delay";
}

void write_alu(std::ostream &stream, u8 opcode, u8 aReg, u8 bReg, op_select_t const &select)
{
	u8 const operands = ALU_OPERANDS[opcode];

	if (!operands)
	{
		stream << ALU_NAME[opcode];
		return;
	}

	if ((opcode == ALU_OP_MOV) && (select.alu_src == PATH_REG) && (select.alu_dst == PATH_REG) &&
			(aReg == REG_ZERO) && (bReg == REG_ZERO))
	{
		stream << "NOP";
		return;
	}

	stream << ALU_NAME[opcode] << ' ';

	// Two-operand instructions are written as "op b, a"; a one-operand
	// instruction names only its source operand.
	if (operands == 2)
	{
		write_register(stream, bReg, false);
		stream << ", ";
		if (select.alu_src == PATH_REG)
			write_register(stream, aReg, false);
		else
			stream << "Delay";

		// The A register is the implied destination only when it is also
		// named as the second source operand. Otherwise it has to be shown.
		if ((opcode != ALU_OP_CMP) && ((select.alu_src != PATH_REG) || (select.alu_dst != PATH_REG)))
		{
			stream << " > ";
			write_path(stream, aReg, select.alu_dst, true);
		}
	}
	else
	{
		if (select.alu_src == PATH_REG)
			write_register(stream, bReg, false);
		else
			stream << "Delay";

		stream << " > ";
		write_path(stream, aReg, select.alu_dst, true);
	}
}

bool write_mac(std::ostream &stream, bool accumulate, u8 cReg, u8 dReg, op_select_t const &select)
{
	// "ZERO X+ ZERO > MAC" leaves the multiplier accumulator untouched.
	if (accumulate && (cReg == REG_ZERO) && (dReg == REG_ZERO) && (select.mac_dst == PATH_REG))
		return false;

	write_register(stream, dReg, false);

	stream << (accumulate ? " X+ " : " X ");

	if (select.mac_src == PATH_REG)
		write_register(stream, cReg, false);
	else
		stream << "Delay";

	// C is the implied destination. An explicit destination is needed when
	// the result also goes to the delay line, or when the dummy ZERO register
	// is selected and the result is kept in the multiplier accumulator.
	if ((select.mac_dst != PATH_REG) || (cReg == REG_ZERO))
	{
		stream << " > ";
		if ((select.mac_dst & PATH_REG) && (cReg == REG_ZERO))
			stream << "MAC";
		else if (select.mac_dst & PATH_REG)
			write_register(stream, cReg, true);
		if (select.mac_dst == PATH_BOTH)
			stream << ", ";
		if (select.mac_dst & PATH_DELAY)
			stream << "Delay";
	}

	return true;
}

} // anonymous namespace

u32 es5510_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t es5510_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 const instr =
			(u64(opcodes.r8(pc + 0)) << 40) |
			(u64(opcodes.r8(pc + 1)) << 32) |
			(u64(opcodes.r8(pc + 2)) << 24) |
			(u64(opcodes.r8(pc + 3)) << 16) |
			(u64(opcodes.r8(pc + 4)) << 8) |
			(u64(opcodes.r8(pc + 5)) << 0);

	u8 const dReg = u8(instr >> 40);
	u8 const cReg = u8(instr >> 32);
	u8 const bReg = u8(instr >> 24);
	u8 const aReg = u8(instr >> 16);
	u8 const aluOpcode = u8(instr >> 12) & 0x0f;
	u8 const operandSelect = u8(instr >> 8) & 0x0f;
	bool const skip = BIT(instr, 7);
	bool const accumulate = BIT(instr, 6);
	u8 const ramControl = u8(instr >> 3) & 0x07;

	op_select_t const &select = OPERAND_SELECT[operandSelect];

	if (skip)
		stream << '*';

	write_alu(stream, aluOpcode, aReg, bReg, select);

	stream << "; ";
	if (write_mac(stream, accumulate, cReg, dReg, select))
		stream << "; ";

	stream << RAM_CONTROL_NAME[ramControl];

	return 6;
}
