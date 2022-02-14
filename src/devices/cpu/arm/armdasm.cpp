// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Phil Stroffolino
/*
    ARM 2/3 disassembler

    (c) 2002-2006 Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino
*/

#include "emu.h"
#include "armdasm.h"

uint32_t arm_disassembler::ExtractImmediateOperand(uint32_t opcode) const
{
	// rrrrbbbbbbbb
	uint32_t imm = opcode & 0xff;
	uint8_t r = ((opcode >> 8) & 0xf) * 2;
	return (imm >> r) | (r ? (imm << (32 - r)) : 0);
}

void arm_disassembler::WriteDataProcessingOperand(std::ostream &stream, uint32_t opcode, bool printOp0, bool printOp1, offs_t pc) const
{
	// ccccctttmmmm
	static const char *const pRegOp[4] = { "LSL","LSR","ASR","ROR" };

	if (printOp0)
		util::stream_format(stream, "R%d, ", (opcode >> 12) & 0xf);
	if (printOp1)
		util::stream_format(stream, "R%d, ", (opcode >> 16) & 0xf);

	// Immediate Op2
	if (opcode & 0x02000000)
	{
		uint32_t imm = ExtractImmediateOperand(opcode);
		util::stream_format(stream, "#&%X", imm);
	}
	else
	{
		// Register Op2
		util::stream_format(stream, "R%d", (opcode >> 0) & 0xf);

		int shiftop = (opcode >> 5) & 3;
		if ((opcode & 0x10) != 0)
		{
			// Shift amount specified in bottom bits of RS
			util::stream_format(stream, ", %s R%d", pRegOp[shiftop], (opcode >> 8) & 0xf);
		}
		else
		{
			// Shift amount immediate 5 bit unsigned integer
			int c = (opcode >> 7) & 0x1f;
			if (c == 0)
			{
				if (shiftop == 0)
				return;
				else if (shiftop == 3)
				{
					util::stream_format(stream, ", RRX");
					return;
				}
				c = 32;
			}
			util::stream_format(stream, ", %s #%d", pRegOp[shiftop], c);
		}
	}
}

void arm_disassembler::WriteRegisterOperand1(std::ostream &stream, uint32_t opcode) const
{
	/* ccccctttmmmm */
	static const char *const pRegOp[4] = { "LSL","LSR","ASR","ROR" };

	util::stream_format(stream, ", ");
	if((opcode & 0x00800000) == 0)
		util::stream_format(stream, "-");

	int shiftop = (opcode >> 5) & 3;
	util::stream_format(
		stream,
		"R%d", /* Operand 1 register, Operand 2 register */
		(opcode >> 0) & 0xf);

	if( opcode&0x10 ) /* Shift amount specified in bottom bits of RS */
	{
		util::stream_format(stream, ", %s R%d", pRegOp[shiftop], (opcode >> 7) & 0xf);
	}
	else /* Shift amount immediate 5 bit unsigned integer */
	{
		int c = (opcode >> 7) & 0x1f;
		if (c == 0)
		{
			if (shiftop == 0)
				return;
			else if (shiftop == 3)
			{
				util::stream_format(stream, ", RRX");
				return;
			}
			c = 32;
		}
		util::stream_format(stream, ", %s #%d", pRegOp[shiftop], c);
	}
} /* WriteRegisterOperand */


void arm_disassembler::WriteBranchAddress(std::ostream &stream, uint32_t pc, uint32_t opcode) const
{
	opcode &= 0x00ffffff;
	if( opcode&0x00800000 )
	{
		opcode |= 0xff000000; /* sign-extend */
	}
	pc += 8+4*opcode;
	util::stream_format( stream, "&%07X", pc & 0x03ffffff );
} /* WriteBranchAddress */

void arm_disassembler::WritePadding(std::ostream &stream, std::streampos start_position) const
{
	std::streamoff difference = stream.tellp() - start_position;
	for (std::streamoff i = difference; i < 8; i++)
		stream << ' ';
}

offs_t arm_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	static const char *const pConditionCodeTable[16] =
	{
		"EQ","NE","CS","CC",
		"MI","PL","VS","VC",
		"HI","LS","GE","LT",
		"GT","LE","","NV"
	};
	static const char *const pOperation[16] =
	{
		"AND","EOR","SUB","RSB",
		"ADD","ADC","SBC","RSC",
		"TST","TEQ","CMP","CMN",
		"ORR","MOV","BIC","MVN"
	};
	const char *pConditionCode;
	uint32_t dasmflags = 0;
	std::streampos start_position = stream.tellp();

	u32 opcode = opcodes.r32(pc);

	pConditionCode= pConditionCodeTable[opcode>>28];

	if( (opcode&0x0fc000f0)==0x00000090u )
	{
		/* multiply */
		/* xxxx0000 00ASdddd nnnnssss 1001mmmm */
		if( opcode&0x00200000 )
		{
			stream << "MLA";
		}
		else
		{
			stream << "MUL";
		}
		stream << pConditionCode;
		if ((opcode & 0x00100000) != 0)
			stream << 'S';

		WritePadding(stream, start_position);

		util::stream_format(stream,
			"R%d, R%d, R%d",
			(opcode>>16)&0xf,
			(opcode&0xf),
			(opcode>>8)&0xf);

		if( opcode&0x00200000 )
		{
			util::stream_format( stream, ", R%d", (opcode>>12)&0xf );
		}
	}
	else if( (opcode&0x0c000000)==0 )
	{
		int op=(opcode>>21)&0xf;

		/* Data Processing */
		/* xxxx001a aaaSnnnn ddddrrrr bbbbbbbb */
		/* xxxx000a aaaSnnnn ddddcccc ctttmmmm */

		// ADR Rn, offset = ADD/SUB Rn, R15, #imm
		bool adr = (op == 0x02 || op == 0x04) && (opcode & 0x020f0000) == 0x020f0000;

		util::stream_format(stream,
			"%s%s",
			adr ? "ADR" : pOperation[op],
			pConditionCode);

		if ((opcode & 0x00100000) != 0)
		{
			if ((op & 0x0c) != 0x08)
				stream << 'S';
			else if (((opcode >> 12) & 0xf) == 0xf)
				stream << 'P';
		}

		WritePadding(stream, start_position);

		switch (op) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x0c:
		case 0x0e:
			if (adr)
			{
				uint32_t imm = ExtractImmediateOperand(opcode);
				util::stream_format(stream, "R%d, &%07X", (opcode >> 12) & 0xf, (op == 0x02) ? pc + 8 - imm : pc + 8 + imm);
			}
			else
				WriteDataProcessingOperand(stream, opcode, true, true, pc);
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			WriteDataProcessingOperand(stream, opcode, false, true, pc);
			break;
		case 0x0d:
			/* look for mov pc,lr */
			if (((opcode >> 12) & 0x0f) == 15 && ((opcode >> 0) & 0x0f) == 14 && (opcode & 0x02000000) == 0)
				dasmflags = STEP_OUT;
			[[fallthrough]];
		case 0x0f:
			WriteDataProcessingOperand(stream, opcode, true, false, pc);
			break;
		}
	}
	else if( (opcode&0x0c000000)==0x04000000 )
	{
		/* Data Transfer */

		/* xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate form */
		/* xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register form */
		if( opcode&0x00100000 )
		{
			stream << "LDR";
		}
		else
		{
			stream << "STR";
		}
		stream << pConditionCode;

		if( opcode&0x00400000 )
		{
			stream << 'B';
		}

		if(( opcode&0x01200000 ) == 0x00200000 )
		{
			stream << 'T';
		}

		int memreg = (opcode >> 16) & 0xf;
		WritePadding(stream, start_position);
		util::stream_format(stream, "R%d, ", (opcode >> 12) & 0xf);

		if ((memreg == 15) && ( opcode&0x03200000 ) == 0x01000000)
		{
			uint16_t displacement = opcode & 0xfff;

			if( opcode&0x00800000 )
			{
				util::stream_format(stream, "&%07X", pc + 8 + displacement);
			}
			else
			{
				util::stream_format(stream, "&%07X", pc + 8 - displacement);
			}
		}
		else
		{
			util::stream_format(stream, "[R%d", memreg);

			if(( opcode&0x01000000 ) == 0)
			{
				/* post-indexed addressing */
				stream << ']';
			}

			if( opcode&0x02000000 )
			{
				/* register form */
				WriteRegisterOperand1(stream, opcode);
			}
			else
			{
				/* immediate form */
				uint16_t displacement = opcode & 0xfff;
				if( displacement != 0 )
				{
					if( opcode&0x00800000 )
					{
						util::stream_format(stream, ", #&%X", displacement);
					}
					else
					{
						util::stream_format(stream, ", -#&%X", displacement);
					}
				}
			}

			if( opcode&0x01000000 )
			{
				/* pre-indexed addressing */
				stream << ']';

				/* writeback addr */
				if( opcode&0x00200000 )
				{
					stream << '!';
				}
			}
		}
	}
	else if( (opcode&0x0e000000) == 0x08000000 )
	{
		/* xxxx100P USWLnnnn llllllll llllllll */
		/* Block Data Transfer */

		uint8_t d = (opcode >> 16) & 0xf;
		if( opcode&0x00100000 )
		{
			util::stream_format( stream, "LDM%s", pConditionCode );
			if (d == 13)
				util::stream_format( stream, "%c%c", ( opcode&0x01000000 ) ? 'E' : 'F', ( opcode&0x00800000 ) ? 'D' : 'A');
			else
				util::stream_format( stream, "%c%c", ( opcode&0x00800000 ) ? 'I' : 'D', ( opcode&0x01000000 ) ? 'B' : 'A');

			if( opcode & 0x00008000 )
				dasmflags = STEP_OUT;
		}
		else
		{
			util::stream_format( stream, "STM%s", pConditionCode );
			if (d == 13)
				util::stream_format( stream, "%c%c", ( opcode&0x01000000 ) ? 'F' : 'E', ( opcode&0x00800000 ) ? 'A' : 'D');
			else
				util::stream_format( stream, "%c%c", ( opcode&0x00800000 ) ? 'I' : 'D', ( opcode&0x01000000 ) ? 'B' : 'A');
		}

		WritePadding(stream, start_position);
		util::stream_format( stream, "R%d", d );
		if( opcode&0x00200000 )
		{
			stream << '!';
		}
		stream << ", {";

		{
			int last=0,found=0;
			for (int j=0; j<16; j++) {
				if (opcode&(1<<j) && found==0) {
					if ((opcode&((1<<j)-1)) != 0)
						stream << ", ";
					found=1;
					last=j;
				}
				else if ((opcode&(1<<j))==0 && found) {
					util::stream_format( stream, "R%d",last);
					if (last!=j-1)
						util::stream_format( stream, "-R%d",j-1);
					found=0;
				}
			}
			if (found && last==15)
				util::stream_format( stream, "R15");
			else if (found)
				util::stream_format( stream, "R%d-R%d",last,15);
		}

		stream << '}';
		if( opcode&0x00400000 )
		{
			stream << '^';
		}
	}
	else if( (opcode&0x0e000000)==0x0a000000 )
	{
		/* branch instruction */
		/* xxxx101L oooooooo oooooooo oooooooo */
		if( opcode&0x01000000 )
		{
			stream << "BL";
			dasmflags = STEP_OVER;
		}
		else
		{
			stream << "B";
		}

		stream << pConditionCode;

		WritePadding(stream, start_position);

		WriteBranchAddress( stream, pc, opcode );
	}
	else if( (opcode&0x0f000000) == 0x0e000000 )
	{
		/* co-processor */
		/* xxxx1110 oooLNNNN ddddpppp qqq1MMMM MRC/MCR */
		if( (opcode&0x0f100000)==0x0e100000 )
		{
			if( (opcode&0x0f100010)==0x0e100010 )
			{
				stream << "MRC";
			}
			else if( (opcode&0x0f100010)==0x0e000010 )
			{
				stream << "MCR";
			}
			else
			{
				stream << "???";
			}

			stream << pConditionCode;
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, CR%d {CRM%d, q%d}",(opcode>>12)&0xf, (opcode>>16)&0xf, (opcode>>0)&0xf, (opcode>>5)&0x7);
			/* Nb:  full form should be o, p, R, CR, CRM, q */
		}
		else if( (opcode&0x0f000010)==0x0e000000 )
		{
			util::stream_format( stream, "CDP%s", pConditionCode );
			WritePadding(stream, start_position);
			util::stream_format( stream, "R%d, CR%d {CRM%d, q%d}",(opcode>>12)&0xf, (opcode>>16)&0xf, (opcode>>0)&0xf, (opcode>>5)&0x7);
		}
		else
		{
			stream << "???";
		}
	}
	else if( (opcode&0x0f000000) == 0x0f000000 )
	{
		/* Software Interrupt */
		util::stream_format( stream, "SWI%s", pConditionCode );
		WritePadding(stream, start_position);
		util::stream_format( stream, "&%X", opcode&0x00ffffff );
		dasmflags = STEP_OVER;
	}
	else
	{
		stream << "Undefined";
	}

	return 4 | dasmflags | SUPPORTED;
}

u32 arm_disassembler::opcode_alignment() const
{
	return 4;
}

arm_disassembler::arm_disassembler()
{
}
