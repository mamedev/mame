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
 *   A step in an ESP image is accompanied by a 24-bit GPR offset. When it is
 *   available, the disassembler shows the offset supplied to the RAM cycle.
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
	"RD tb[]", "RD dd (dump)", "RD io[]", "WR io[]"
};

constexpr char const *const RAM_CONTROL_FORMAT[8] =
{
	"RD dd[$%04x]", "WR dd[$%04x]", "RD ta[$%04x]", "WR ta[$%04x]",
	"RD tb[$%04x]", "RD dd (dump)", "RD io[$%04x]", "WR io[$%04x]"
};

u64 read_instruction(util::disasm_interface::data_buffer const &opcodes, util::disasm_interface::offs_t pc)
{
	return
			(u64(opcodes.r8(pc + 0)) << 40) |
			(u64(opcodes.r8(pc + 1)) << 32) |
			(u64(opcodes.r8(pc + 2)) << 24) |
			(u64(opcodes.r8(pc + 3)) << 16) |
			(u64(opcodes.r8(pc + 4)) << 8) |
			(u64(opcodes.r8(pc + 5)) << 0);
}

void write_register(std::ostream &stream, u8 reg, bool destination)
{
	switch (reg)
	{
	case 0xcc: stream << "DACL"; return;
	case 0xcd: stream << "DACR"; return;
	case 0xe8: stream << "PWDACR"; return;
	case 0xe9: stream << "PWDACL"; return;
	}

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

void write_source_register(std::ostream &stream, u8 reg, u32 value, bool show_value)
{
	write_register(stream, reg, false);
	if (show_value)
		util::stream_format(stream, "(%06x)", value & 0x00ffffff);
}

void write_source_delay(std::ostream &stream, char const *memory_name, u32 value, bool show_value)
{
	stream << memory_name;
	if (show_value)
		util::stream_format(stream, "(%06x)", value & 0x00ffffff);
}

char const *memory_path_name(u8 ram_control)
{
	switch (ram_control)
	{
	case 2:
	case 3:
		return "Table A";
	case 4:
		return "Table B";
	case 6:
	case 7:
		return "I/O";
	default:
		return "Delay";
	}
}

bool path_has_output(u8 path)
{
	// Every path is an output: a register-only destination of ZERO is a real
	// encoded destination (the result is discarded), not a placeholder.
	return path != 0;
}

void write_destination(std::ostream &stream, u8 reg, u8 path, char const *memory_name)
{
	bool const zero_is_placeholder = (reg == REG_ZERO) && (path & PATH_DELAY);
	bool const write_reg = (path & PATH_REG) && !zero_is_placeholder;
	if (write_reg)
		write_register(stream, reg, true);
	if (write_reg && (path & PATH_DELAY))
		stream << ", ";
	if (path & PATH_DELAY)
		stream << memory_name;
}

void write_alu(std::ostream &stream, u8 opcode, u8 aReg, u8 bReg, op_select_t const &select, es5510_disassembler::operand_values const *values, char const *memory_name)
{
	u8 const operands = ALU_OPERANDS[opcode];
	bool const show_values = values != nullptr;

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
		write_source_register(stream, bReg, show_values ? values->b : 0, show_values);
		stream << ", ";
		if (select.alu_src == PATH_REG)
			write_source_register(stream, aReg, show_values ? values->a : 0, show_values);
		else
			write_source_delay(stream, memory_name, show_values ? values->dil : 0, show_values);

		// The A register is the implied destination only when it is also
		// named as the second source operand. Otherwise it has to be shown.
		if ((opcode != ALU_OP_CMP) && ((select.alu_src != PATH_REG) || (select.alu_dst != PATH_REG)) &&
				path_has_output(select.alu_dst))
		{
			stream << " > ";
			write_destination(stream, aReg, select.alu_dst, memory_name);
		}
	}
	else
	{
		if (select.alu_src == PATH_REG)
			write_source_register(stream, bReg, show_values ? values->b : 0, show_values);
		else
			write_source_delay(stream, memory_name, show_values ? values->dil : 0, show_values);

		bool const redundant = (select.alu_src == PATH_REG) && (select.alu_dst == PATH_REG) && (aReg == bReg);
		if (!redundant && path_has_output(select.alu_dst))
		{
			stream << " > ";
			write_destination(stream, aReg, select.alu_dst, memory_name);
		}
	}
}

void write_mac(std::ostream &stream, bool accumulate, u8 cReg, u8 dReg, op_select_t const &select, es5510_disassembler::operand_values const *values, char const *memory_name)
{
	bool const show_values = values != nullptr;

	write_source_register(stream, dReg, show_values ? values->d : 0, show_values);

	stream << (accumulate ? " X+ " : " X ");

	if (select.mac_src == PATH_REG)
		write_source_register(stream, cReg, show_values ? values->c : 0, show_values);
	else
		write_source_delay(stream, memory_name, show_values ? values->dil : 0, show_values);

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
			stream << memory_name;
	}
}

bool mac_visible(bool accumulate, u8 cReg, u8 dReg, op_select_t const &select)
{
	// "ZERO X+ ZERO > MAC" leaves the multiplier accumulator untouched.
	return !(accumulate && (cReg == REG_ZERO) && (dReg == REG_ZERO) && (select.mac_dst == PATH_REG));
}

bool ram_visible(u8 ram_control, op_select_t const &select)
{
	return (ram_control != 0) ||
			(select.alu_src & PATH_DELAY) || (select.alu_dst & PATH_DELAY) ||
			(select.mac_src & PATH_DELAY) || (select.mac_dst & PATH_DELAY);
}

void write_ram_control(std::ostream &stream, u8 ram_control, bool have_gpr, u32 gpr)
{
	if (!have_gpr)
		stream << RAM_CONTROL_NAME[ram_control];
	else if (ram_control == 5)
		stream << RAM_CONTROL_FORMAT[ram_control];
	else
		util::stream_format(stream, RAM_CONTROL_FORMAT[ram_control], (gpr >> 8) & 0xffff);
}

} // anonymous namespace

es5510_disassembler::es5510_disassembler(format fmt) : m_format(fmt)
{
}

es5510_disassembler::~es5510_disassembler() = default;

void es5510_disassembler::describe(std::ostream &stream, u64 instr, u32 gpr, bool have_gpr, operand_values const *values)
{
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
	char const *const memory_name = memory_path_name(ramControl);

	if (skip)
		stream << '*';

	write_alu(stream, aluOpcode, aReg, bReg, select, values, memory_name);

	if (mac_visible(accumulate, cReg, dReg, select))
	{
		stream << "; ";
		write_mac(stream, accumulate, cReg, dReg, select, values, memory_name);
	}

	if (ram_visible(ramControl, select))
	{
		stream << "; ";
		write_ram_control(stream, ramControl, have_gpr, gpr);
	}
}

u32 es5510_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t es5510_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u64 instr;
	u32 gpr = 0;
	bool have_gpr = false;
	offs_t length;

	switch (m_format)
	{
	case format::PARAMS:
		instr = opcodes.r64(pc);
		gpr = params.r32(pc);
		have_gpr = true;
		length = 1;
		break;

	case format::GPR_BYTES:
		gpr =
				(u32(opcodes.r8(pc + 0)) << 16) |
				(u32(opcodes.r8(pc + 1)) << 8) |
				(u32(opcodes.r8(pc + 2)) << 0);
		instr = read_instruction(opcodes, pc + 3);
		have_gpr = true;
		length = 9;
		break;

	case format::BYTES:
	default:
		instr = read_instruction(opcodes, pc);
		length = 6;
		break;
	}

	describe(stream, instr, gpr, have_gpr);

	return length;
}
