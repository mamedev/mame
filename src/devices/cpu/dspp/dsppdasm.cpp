// license:BSD-3-Clause
// copyright-holders:Philip Bennett, AJR
/***************************************************************************

    dsppdasm.c
    Disassembler for the portable DSPP emulator.

***************************************************************************/

#include "emu.h"
#include "dspp.h"
#include "dsppdasm.h"


/***************************************************************************
    CODE CODE
***************************************************************************/

uint32_t dspp_disassembler::opcode_alignment() const
{
	return 1;
}


//-------------------------------------------------
//  format_operand - Format the value encoded by
//  an instruction operand
//-------------------------------------------------

void dspp_disassembler::format_operand(std::ostream &stream, uint16_t operand)
{
	uint32_t numregs = 0;

	if (operand & 0x8000)
	{
		// Immediate value
		if ((operand & 0xc000) == 0xc000)
		{
			uint16_t val;

			if (operand & 0x2000)
			{
				// Left justify
				val = (operand & 0x1fff) << 3;
			}
			else
			{
				// Sign extend if right justified
				val = util::sext(operand, 13);
			}
			util::stream_format(stream, "#0x%04X", val);
		}
		else if((operand & 0xe000) == 0x8000)
		{
			// Address operand
			uint16_t addr = operand & 0x03ff;

			if (operand & 0x0400)
			{
				// Indirect
				stream << '@';
			}
			util::stream_format(stream, "[0x%03X]", addr);

			if (operand & 0x0800)
			{
				// Write Back
				stream << '!';
			}
		}
		else if ((operand & 0xe000) == 0xa000)
		{
			// 1 or 2 register operand
			numregs = (operand & 0x0400) ? 2 : 1;
		}
	}
	else
	{
		numregs = 3;
	}

	if (numregs > 0)
	{
		uint32_t shifter, regdi;

		// Shift successive register operands from a single operand word
		for (uint32_t i = 0; i < numregs; ++i)
		{
			if (i > 0)
				stream << ", ";

			shifter = ((numregs - i) - 1) * 5;
			regdi = (operand >> shifter) & 0x1f;

			if (regdi & 0x0010)
			{
				// Indirect?
				stream << '@';
			}
			util::stream_format(stream, "[R%d]", regdi & 0xf);

			if (numregs == 2)
			{
				if ((i == 0) && (operand & 0x1000))
					stream << '!';
				else if ((i == 1) && (operand & 0x0800))
					stream << '!';
			}
			else if (numregs == 1)
			{
				if (operand & 0x800)
					stream << '!';
			}
		}
	}
}



//**************************************************************************
//  OPCODE DISASSEMBLY
//**************************************************************************

//-------------------------------------------------
//  dasm_super_special - Disassemble a super
//  special control op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_super_special(std::ostream &stream, uint16_t op)
{
	uint32_t sel = (op >> 7) & 7;

	switch (sel)
	{
		case 1: // BAC
		{
			stream << "BAC";
			return 1 | SUPPORTED;
		}

		case 4: // RTS
		{
			stream << "RTS";
			return 1 | STEP_OUT | SUPPORTED;
		}

		case 5: // OP_MASK
		{
			stream << "OP_MASK";
			return 1 | SUPPORTED;
		}

		case 7: // SLEEP
		{
			stream << "SLEEP";
			return 1 | SUPPORTED;
		}

		case 0: // NOP
		{
			stream << "NOP";
			return 1 | SUPPORTED;
		}

		default: // Unused
		{
			util::stream_format(stream, "0x%04X", op);
			return 1 | SUPPORTED;
		}
	}
}


//-------------------------------------------------
//  dasm_special - Disassemble a special control op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_special(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op)
{
	switch ((op >> 10) & 7)
	{
		case 0:
		{
			return dasm_super_special(stream, op);
		}
		case 1: // JUMP
		{
			util::stream_format(stream, "JUMP 0x%03X", op & 0x3ff);
			return 1 | SUPPORTED;
		}
		case 2: // JSR
		{
			util::stream_format(stream, "JSR 0x%03X", op & 0x3ff);
			return 1 | STEP_OVER | SUPPORTED;
		}
		case 3: // BFM
		{
			util::stream_format(stream, "BFM 0x%03X", op & 0x3ff);
			return 1 | SUPPORTED;
		}
		case 4: // MOVEREG
		{
			uint32_t regdi = op & 0x3f;

			util::stream_format(stream, "MOVEREG %sR%d, ", (regdi & 0x0010) ? "@" : "", regdi & 0xf);
			format_operand(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;
		}
		case 5: // RBASE
		{
			util::stream_format(stream, "RBASE%d 0x%03X", (op & 3) << 2, op & 0x3fc);
			return 1 | SUPPORTED;
		}
		case 6: // MOVED
		{
			util::stream_format(stream, "MOVE [0x%03X], ", op & 0x3ff);
			format_operand(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;
		}
		case 7: // MOVEI
		{
			util::stream_format(stream, "MOVE @[0x%03X], ", op & 0x3ff);
			format_operand(stream, opcodes.r16(pc + 1));
			return 2 | SUPPORTED;
		}

		default:
		{
			util::stream_format(stream, "0x%04X", op);
			return 1 | SUPPORTED;
		}
	}
}


//-------------------------------------------------
//  dasm_branch - Disassemble a branch control op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_branch(std::ostream &stream, uint16_t op)
{
	uint32_t mode = (op >> 13) & 3;
	uint32_t select = (op >> 12) & 1;
	uint32_t mask = (op >> 10) & 3;

	char flag0, flag1;

	if (select == 0)
	{
		flag0 = 'N';
		flag1 = 'V';
	}
	else
	{
		flag0 = 'C';
		flag1 = 'Z';
	}

	bool mask0 = (mask & 2) != 0;
	bool mask1 = (mask & 1) != 0;

	if (mode == 2)
		stream << "BFC ";
	else
		stream << "BTC ";

	if (mask0 && mask1)
	{
		stream << flag1;
		stream << flag0;
	}
	else if (mask0)
	{
		stream << flag0;
	}
	else if (mask1)
	{
		stream << flag1;
	}
	else
	{
		stream << '0';
	}

	util::stream_format(stream, ", 0x%03X", op & 0x3ff);
	return 1 | (mask0 || mask1 ? STEP_COND : 0) | SUPPORTED;
}


//-------------------------------------------------
//  dasm_complex_branch - Disassemble a complex
//  branch control op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_complex_branch(std::ostream &stream, uint16_t op)
{
	uint32_t type = (op >> 10) & 7;

	switch (type)
	{
		case 0: // BLT
			stream << "BLT";
			break;
		case 1: // BLE
			stream << "BLE";
			break;
		case 2: // BGE
			stream << "BGE";
			break;
		case 3: // BGT
			stream << "BGT";
			break;
		case 4: // BHI
			stream << "BHI";
			break;
		case 5: // BLS
			stream << "BLS";
			break;
		case 6: // BXS
			stream << "BXS";
			break;
		case 7: // BXC
			stream << "BXC";
			break;
	}

	util::stream_format(stream, " 0x%03X", op & 0x3ff);
	return 1 | STEP_COND | SUPPORTED;
}


//-------------------------------------------------
//  dasm_control - Disassemble a control op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_control(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op)
{
	uint32_t mode = (op >> 13) & 3;

	switch (mode)
	{
		// Special
		case 0:
		{
			return dasm_special(stream, pc, opcodes, op);
		}

		// Branches
		case 1: case 2:
		{
			return dasm_branch(stream, op);
		}

		// Complex branches
		case 3:
		{
			return dasm_complex_branch(stream, op);
		}

		default:
		{
			util::stream_format(stream, "0x%04X", op);
			return 1 | SUPPORTED;
		}
	}
}


//-------------------------------------------------
//  dasm_arithmetic - Disassemble an arithmetic op
//-------------------------------------------------

offs_t dspp_disassembler::dasm_arithmetic(std::ostream &stream, offs_t pc, const data_buffer &opcodes, uint16_t op)
{
	// Decode the various fields
	uint32_t numops = (op >> 13) & 3;
	uint32_t muxa = (op >> 10) & 3;
	uint32_t muxb = (op >> 8) & 3;
	uint32_t alu_op = (op >> 4) & 0xf;
	uint32_t barrel_code = op & 0xf;

	uint32_t opidx = 0;

	// Check for operand overflow
	if (numops == 0 && ((muxa == 1) || (muxa == 2) || (muxb == 1) || (muxb == 2)))
		numops = 4;

	// Implicit barrel shift
	if (barrel_code == 8)
		++numops;

	if (muxa == 3 || muxb == 3)
	{
		uint32_t mul_sel = (op >> 12) & 1;

		++opidx;
		if (mul_sel)
			++opidx;
	}

	if (alu_op < 8)
		stream << "TRA ";
	else
		stream << "TRL ";

	if (alu_op != 1)
	{
		if (alu_op == 9)
			stream << "NOT ";

		switch (muxa)
		{
			case 0:
			{
				stream << "ACC";
				break;
			}
			case 1: case 2:
			{
				format_operand(stream, opcodes.r16(pc + 1 + opidx++));
				break;
			}
			case 3:
			{
				uint32_t mul_sel = (op >> 12) & 1;

				format_operand(stream, opcodes.r16(pc + 1));
				stream << " * ";
				if (mul_sel)
					format_operand(stream, opcodes.r16(pc + 2));
				else
					stream << "ACC";
				break;
			}
		}
	}

	switch (alu_op)
	{
		case 0: // _TRA
		{
			break;
		}
		case 1: // _NEG
		{
			stream << "-";
			break;
		}
		case 2: // _+
		{
			stream << " + ";
			break;
		}
		case 3: // _+C
		{
			stream << " + C";
			break;
		}
		case 4: // _-
		{
			stream << " - ";
			break;
		}
		case 5: // _-B
		{
			stream << " - B";
			break;
		}
		case 6: // _++
		{
			stream << "++";
			break;
		}
		case 7: // _--
		{
			stream << "--";
			break;
		}
		case 8: // _TRL
		{
			break;
		}
		case 9: // _NOT
		{
			break;
		}
		case 10: // _AND
		{
			stream << " AND ";
			break;
		}
		case 11: // _NAND
		{
			stream << " NAND ";
			break;
		}
		case 12: // _OR
		{
			stream << " OR ";
			break;
		}
		case 13: // _NOR
		{
			stream << " NOR ";
			break;
		}
		case 14: // _XOR
		{
			stream << " XOR ";
			break;
		}
		case 15: // _XNOR
		{
			stream << " XNOR ";
			break;
		}
	}

	if (alu_op == 1 || alu_op == 2 || alu_op == 4 || alu_op >= 10)
	{
		switch (muxb)
		{
			case 0:
			{
				stream << "ACC";
				break;
			}
			case 1: case 2:
			{
				format_operand(stream, opcodes.r16(pc + 1 + opidx++));
				break;
			}
			case 3:
			{
				uint32_t mul_sel = (op >> 12) & 1;

				format_operand(stream, opcodes.r16(pc + 1));
				stream << " * ";
				if (mul_sel)
					format_operand(stream, opcodes.r16(pc + 2));
				else
					stream << "ACC";
				break;
			}
		}
	}

	// Barrel shift
	static const int32_t shifts[8] = { 0, 1, 2, 3, 4, 5, 8, 16 };

	if (barrel_code == 8)
	{
		stream << " SHIFT ";
		format_operand(stream, opcodes.r16(pc + 1 + opidx++));
	}

	else if (barrel_code & 8)
	{
		// Right shift
		uint32_t shift = shifts[(~barrel_code + 1) & 7];

		if (shift != 0)
		{
			util::stream_format(stream, " >> %d", shift);
		}
	}
	else
	{
		// Left shift
		uint32_t shift = shifts[barrel_code];

		if (shift == 16)
		{
			// Clip and saturate
			stream << " SAT";
		}
		else if (shift != 0)
		{
			util::stream_format(stream, " << %d", shift);
		}
	}

	if (opidx < numops)
	{
		stream << ", ";
		format_operand(stream, opcodes.r16(pc + 1 + opidx));
	}

	return (numops + 1) | SUPPORTED;
}

offs_t dspp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	// Decode and disassemble
	uint16_t op = opcodes.r16(pc);
	if (op & 0x8000)
		return dasm_control(stream, pc, opcodes, op);
	else
		return dasm_arithmetic(stream, pc, opcodes, op);
}
