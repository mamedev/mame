// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series disassembler

***************************************************************************/

#include "util/disasmintf.h"
#include "f2mc16dasm.h"

#include "util/strformat.h"

using osd::u32;
using util::BIT;
using offs_t = u32;

u32 f2mc16_disassembler::opcode_alignment() const
{
	return 1;
}

void f2mc16_disassembler::branch_helper(std::ostream &stream, const char *insName, u16 PC, s8 offset)
{
	util::stream_format(stream, "%s $%04X", insName, PC+offset);
}

offs_t f2mc16_disassembler::ea_form1_helper(std::ostream &stream, const char *opName, u16 pc, u8 operand, u16 imm16, bool bAIsDest)
{
	offs_t bytes = 2;

	switch (operand & 0xf)
	{
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			if (bAIsDest)
			{
				util::stream_format(stream, "%s A, @RW%d + #$%02x", opName, (operand & 0x7), imm16&0xff);
			}
			else
			{
				util::stream_format(stream, "%s @RW%d + #$%02x, A", opName, (operand & 0x7), imm16&0xff);
			}
			bytes = 3;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			if (bAIsDest)
			{
				util::stream_format(stream, "%s A, @RW%d + #$%04x", opName, (operand & 0x3), imm16);
			}
			else
			{
				util::stream_format(stream, "%s @RW%d + #$%04x, A", opName, (operand & 0x3), imm16);
			}
			bytes = 4;
			break;

		case 0xc: case 0xd:
			if (bAIsDest)
			{
				util::stream_format(stream, "%s A, @RW%d + RW7", opName, (operand & 0x1));
			}
			else
			{
				util::stream_format(stream, "%s @RW%d + RW7, A", opName, (operand & 0x1));
			}
			bytes = 2;
			break;

		case 0xe:
			if (bAIsDest)
			{
				util::stream_format(stream, "%s A, @PC + #$04x ($%04X)", opName,(operand & 0x1), imm16, (s16)imm16 + (pc & 0xffff));
			}
			else
			{
				util::stream_format(stream, "%s @PC + #$04x ($%04X), A", opName,(operand & 0x1), imm16, (s16)imm16 + (pc & 0xffff));
			}
			bytes = 4;
			break;

		case 0xf:
			if (bAIsDest)
			{
				util::stream_format(stream, "%s A, $%04x", (operand & 0x1), imm16);
			}
			else
			{
				util::stream_format(stream, "%s $%04x, A", (operand & 0x1), imm16);
			}
			bytes = 4;
			break;
	}

	return bytes;
}

offs_t f2mc16_disassembler::ea_form1_helper_noA(std::ostream &stream, const char *opName, u16 pc, u8 operand, u16 imm16)
{
	offs_t bytes = 2;

	switch (operand & 0xf)
	{
		case 0x0: case 0x1: case 0x2: case 0x3: case 0x4: case 0x5: case 0x6: case 0x7:
			util::stream_format(stream, "%s @RW%d + #$%02x", opName, (operand & 0x7), imm16&0xff);
			bytes = 3;
			break;

		case 0x8: case 0x9: case 0xa: case 0xb:
			util::stream_format(stream, "%s@RW%d + #$%04x", opName, (operand & 0x3), imm16);
			bytes = 4;
			break;

		case 0xc: case 0xd:
			util::stream_format(stream, "%s@RW%d + RW7", opName, (operand & 0x1));
			bytes = 2;
			break;

		case 0xe:
			util::stream_format(stream, "%s@PC + #$04x ($%04X)", opName,(operand & 0x1), imm16, (s16)imm16 + (pc & 0xffff));
			bytes = 4;
			break;

		case 0xf:
			util::stream_format(stream, "%s$%04x", (operand & 0x1), imm16);
			bytes = 4;
			break;
	}

	return bytes;
}

offs_t f2mc16_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 opcode = opcodes.r8(pc);
	u8 operand = opcodes.r8(pc+1);
	offs_t bytes = 1;

	switch (opcode)
	{
	case 0x00:
		stream << "NOP";
		break;

	case 0x01:
		stream << "INT9";
		break;

	case 0x03:
		util::stream_format(stream, "NEG    A");
		break;

	case 0x04:
		stream << "PCB ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x05:
		stream << "DTB ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x06:
		stream << "ADB ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x07:
		stream << "SPB ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x08:
		util::stream_format(stream, "LINK   #$%02x", operand);
		bytes = 2;
		break;

	case 0x09:
		stream << "UNLINK";
		break;

	case 0x0a:
		util::stream_format(stream, "MOV    RP, #$%02x", operand);
		bytes = 2;
		break;

	case 0x0b:
		stream << "NEGW   A";
		break;

	case 0x0c:
		stream << "LSLW   A";
		break;

	case 0x0e:
		stream << "ASRW   A";
		break;

	case 0x0f:
		stream << "LSRW   A";
		break;

	case 0x10:
		stream << "CMR ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x11:
		stream << "NCC ";
		bytes = 1 + (disassemble(stream, pc + 1, opcodes, params) & LENGTHMASK);
		break;

	case 0x12:
		stream << "SUBDC  A";
		break;

	case 0x14:
		stream << "EXT";
		break;

	case 0x15:
		stream << "ZEXT";
		break;

	case 0x16:
		stream << "SWAP";
		break;

	case 0x17:
		util::stream_format(stream, "ADDSP  #$%02x", operand);
		bytes = 2;
		break;

	case 0x18:
		util::stream_format(stream, "ADDL   A, #$%08x", opcodes.r16(pc+1) | (opcodes.r16(pc+3)<<16));
		bytes = 5;
		break;

	case 0x19:
		util::stream_format(stream, "SUBL   A, #$%08x", opcodes.r16(pc+1) | (opcodes.r16(pc+3)<<16));
		bytes = 5;
		break;

	case 0x1a:
		util::stream_format(stream, "MOV    ILM, #$%02x", operand);
		bytes = 2;
		break;

	case 0x1c:
		stream << "EXTW";
		break;

	case 0x1d:
		stream << "ZEXTW";
		break;

	case 0x1e:
		stream << "SWAPW";
		break;

	case 0x20:
		util::stream_format(stream, "ADD    A, $%02x", operand);
		bytes = 2;
		break;

	case 0x21:
		util::stream_format(stream, "SUB    A, $%02x", operand);
		bytes = 2;
		break;

	case 0x22:
		stream << "ADDC   A";
		break;

	case 0x23:
		stream << "CMP    A";
		break;

	case 0x24:
		util::stream_format(stream, "AND    CCR, #$%02x", operand);
		bytes = 2;
		break;

	case 0x25:
		util::stream_format(stream, "OR     CCR, #$%02x", operand);
		bytes = 2;
		break;

	case 0x26:
		stream << "DIVU   A";
		break;

	case 0x27:
		stream << "MULU   A";
		break;

	case 0x28:
		stream << "ADDW   A";
		break;

	case 0x29:
		stream << "SUBW   A";
		break;

	case 0x2a:
		util::stream_format(stream, "CBNE   A, #$%02X, $%04X", operand, (s8)opcodes.r8(pc+2) + (pc & 0xffff) + 3);
		bytes = 3;
		break;

	case 0x2b:
		stream << "CMPW   A";
		break;

	case 0x2c:
		stream << "ANDW   A";
		break;

	case 0x2d:
		stream << "ORW    A";
		break;

	case 0x2e:
		stream << "XORW   A";
		break;

	case 0x2f:
		stream << "MULUW  A";
		break;

	case 0x30:
		util::stream_format(stream, "ADD    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x31:
		util::stream_format(stream, "SUB    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x32:
		stream << "SUBC   A";
		break;

	case 0x33:
		util::stream_format(stream, "CMP    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x34:
		util::stream_format(stream, "AND    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x35:
		util::stream_format(stream, "OR     A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x36:
		util::stream_format(stream, "XOR    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x37:
		stream << "NOT    A";
		break;

	case 0x38:
		util::stream_format(stream, "ADDW   A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x39:
		util::stream_format(stream, "SUBW   A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x3a:
		util::stream_format(stream, "CWBNE  A, #$%04X, $%04X", opcodes.r16(pc+1), (s8)opcodes.r8(pc+3) + (pc & 0xffff) + 4);
		bytes = 4;
		break;

	case 0x3b:
		util::stream_format(stream, "CMPW   A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x3c:
		util::stream_format(stream, "CMPW   A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x3d:
		util::stream_format(stream, "ORW    A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x3f:
		stream << "NOTW   A";
		break;

	case 0x40:
		util::stream_format(stream, "MOV    A, dir(%02x)", operand);
		bytes = 2;
		break;

	case 0x41:
		util::stream_format(stream, "MOV    dir(%02x), A", operand);
		bytes = 2;
		break;

	case 0x42:
		util::stream_format(stream, "MOV    A, #$%02x", operand);
		bytes = 2;
		break;

	case 0x44:
		util::stream_format(stream, "MOV    dir(%02x), #$%02x", operand, opcodes.r8(pc+2));
		bytes = 3;
		break;

	case 0x46:
		util::stream_format(stream, "MOVW   A, SP");
		break;

	case 0x47:
		util::stream_format(stream, "MOVW   SP, A");
		break;

	case 0x48:
		util::stream_format(stream, "MOVW   A, dir(%02x)", operand);
		bytes = 2;
		break;

	case 0x49:
		util::stream_format(stream, "MOVW   dir(%02x), A", operand);
		bytes = 2;
		break;

	case 0x4a:
		util::stream_format(stream, "MOVW   A, #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x4b:
		util::stream_format(stream, "MOVL   A, #$%08x", opcodes.r32(pc+1));
		bytes = 5;
		break;

	case 0x4c:
		stream << "PUSHW  A";
		break;

	case 0x4d:
		stream << "PUSHW  AH";
		break;

	case 0x4e:
		stream << "PUSHW  PS";
		break;

	case 0x4f:
		util::stream_format(stream, "PUSHW   ");
		for (int i = 0; i < 8; i++)
		{
			if (operand & (1<<i))
			{
				util::stream_format(stream, "RW%d, ", i);
			}
		}
		bytes = 2;
		break;

	case 0x52:
		util::stream_format(stream, "MOV    A, $%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x53:
		util::stream_format(stream, "MOV    $%04x, A", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x57:
		util::stream_format(stream, "MOVX   A, $%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x5a:
		util::stream_format(stream, "MOVW   A, $%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x5b:
		util::stream_format(stream, "MOVW   $%04x, A", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x5c:
		stream << "POPW   A";
		break;

	case 0x5d:
		stream << "POPW   AH";
		break;

	case 0x5e:
		stream << "POPW   PS";
		break;

	case 0x5f:
		util::stream_format(stream, "POPW    ");
		for (int i = 0; i < 8; i++)
		{
			if (operand & (1<<i))
			{
				util::stream_format(stream, "RW%d, ", i);
			}
		}
		bytes = 2;
		break;

	case 0x60: branch_helper(stream, "BRA   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;

	case 0x61:
		stream << "JMP    @A";
		break;

	case 0x62:
		util::stream_format(stream, "JMP    #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x63:
		util::stream_format(stream, "JMPP   #$%06x", opcodes.r8(pc+3)<<16|opcodes.r8(pc+2)<<8|opcodes.r8(pc+1));
		bytes = 4;
		break;

	case 0x64:
		util::stream_format(stream, "CALL   #$%04x", opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0x65:
		util::stream_format(stream, "CALLP  #$%06x", opcodes.r8(pc+3)<<16|opcodes.r8(pc+2)<<8|opcodes.r8(pc+1));
		bytes = 4;
		break;

	case 0x66:
		util::stream_format(stream, "RETP");
		break;

	case 0x67:
		stream << "RET";
		break;

	case 0x6b:
		stream << "RETI";
		break;

	case 0x6c:  // bit operations
		switch (operand)
		{
			case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
				util::stream_format(stream, "CLRB   $%04x, %d", opcodes.r16(pc+2), operand & 7);
				bytes = 2;
				break;

			case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
				util::stream_format(stream, "SETB   $%04x, %d", opcodes.r16(pc+2), operand & 7);
				bytes = 2;
				break;

			case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				util::stream_format(stream, "BBC    dir(%02x), %d, $%x", opcodes.r8(pc+2), operand & 7, pc+4+(s8)opcodes.r8(pc+3));
				bytes = 4;
				break;

			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				util::stream_format(stream, "BBC    $%04x, %d, $%x", opcodes.r16(pc+2), operand & 7, pc+5+(s8)opcodes.r8(pc+4));
				bytes = 5;
				break;

			case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
				util::stream_format(stream, "BBS    dir(%02x), %d, $%x", opcodes.r8(pc+2), operand & 7, pc+4+(s8)opcodes.r8(pc+3));
				bytes = 4;
				break;

			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				util::stream_format(stream, "BBS    $%04x, %d, $%x", opcodes.r16(pc+2), operand & 7, pc+5+(s8)opcodes.r8(pc+4));
				bytes = 5;
				break;

			default:
				stream << "UNK 6C bit op";
				break;
		}
		break;

	case 0x6e:  // string instructions
		switch (operand)
		{
			case 0x09:
				util::stream_format(stream, "MOVSI  ADB, DTB");
				bytes = 2;
				break;

			default:
				stream << "UNK string op";
				break;
		}
		break;

	case 0x6f:  // 2-byte instructions
		switch (operand)
		{
			case 0x00: stream << "MOV   A, DTB"; bytes = 2; break;
			case 0x01: stream << "MOV   A, ADB"; bytes = 2; break;
			case 0x02: stream << "MOV   A, SSB"; bytes = 2; break;
			case 0x03: stream << "MOV   A, USB"; bytes = 2; break;
			case 0x04: stream << "MOV   A, DPR"; bytes = 2; break;

			case 0x05:
				util::stream_format(stream, "MOV    A, @A");
				bytes = 2;
				break;

			case 0x06: stream << "MOV   A, PCB"; bytes = 2; break;
			case 0x07: stream << "ROLC  A"; bytes = 2; break;

			case 0x0c:
				util::stream_format(stream, "LSLW   A, R0");
				bytes = 2;
				break;

			case 0x0d:
				util::stream_format(stream, "MOVW   A, @A");
				bytes = 2;
				break;

			case 0x0e:
				util::stream_format(stream, "ASRW   A, R0");
				bytes = 2;
				break;

			case 0x0f:
				util::stream_format(stream, "LSRW   A, R0");
				bytes = 2;
				break;

			case 0x10:
				util::stream_format(stream, "MOV    DTB, A");
				bytes = 2;
				break;

			case 0x11:
				util::stream_format(stream, "MOV    ADB, A");
				bytes = 2;
				break;

			case 0x12:
				util::stream_format(stream, "MOV    SSB, A");
				bytes = 2;
				break;

			case 0x13:
				util::stream_format(stream, "MOV    USB, A");
				bytes = 2;
				break;

			case 0x14:
				util::stream_format(stream, "MOV    DPR, A");
				bytes = 2;
				break;

			case 0x1c:
				util::stream_format(stream, "LSLL   A, R0");
				bytes = 2;
				break;

			case 0x1d:
				util::stream_format(stream, "MOVW   @AL, AH");
				bytes = 2;
				break;

			case 0x1e:
				util::stream_format(stream, "ASRL   A, R0");
				bytes = 2;
				break;

			case 0x1f:
				util::stream_format(stream, "LSRL   A, R0");
				bytes = 2;
				break;

			case 0x2c:
				util::stream_format(stream, "LSL    A, R0");
				bytes = 2;
				break;

			case 0x2d:
				util::stream_format(stream, "NRML   A, R0");
				bytes = 2;
				break;

			case 0x2e:
				util::stream_format(stream, "ASR    A, R0");
				bytes = 2;
				break;

			case 0x2f:
				util::stream_format(stream, "LSR    A, R0");
				bytes = 2;
				break;

			case 0x30: case 0x32: case 0x34: case 0x36:
				util::stream_format(stream, "MOV    @RL%d + #$%02x, A", ((operand>>1) & 0x3), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x38: case 0x3a: case 0x3c: case 0x3e:
				util::stream_format(stream, "MOVW   @RL%d + #$%02x, A", ((operand>>1) & 0x3), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x40: case 0x42: case 0x44: case 0x46:
				util::stream_format(stream, "MOV    A, @RL%d + #$%02x", ((operand>>1) & 0x3), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x48: case 0x4a: case 0x4c: case 0x4e:
				util::stream_format(stream, "MOVW   A, @RL%d + #$%02x", ((operand>>1) & 0x3), opcodes.r8(pc+2));
				bytes = 3;
				break;

			default:
				stream << "UNK 2-byte 6F";
				break;
		}
		break;

	case 0x70:  // ea-type instructions
		switch ((operand >> 4) & 0xf)
		{
			case 0x0:
				switch (operand)
				{
					case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
						util::stream_format(stream, "ADDL   A, RL%d", (operand & 0x6)>>1);
						bytes = 2;
						break;

					default:
						stream << "UNK ea-type 70 0x";
						break;
				}
				break;

			case 0x1: bytes = ea_form1_helper(stream, "ADDL  ", pc, operand, opcodes.r16(pc+2), true); break;
			case 0x3: bytes = ea_form1_helper(stream, "SUBL  ", pc, operand, opcodes.r16(pc+2), true); break;
			case 0x7: bytes = ea_form1_helper(stream, "CMPL  ", pc, operand, opcodes.r16(pc+2), true); break;
			case 0x9: bytes = ea_form1_helper(stream, "ANDL  ", pc, operand, opcodes.r16(pc+2), true); break;
			case 0xb: bytes = ea_form1_helper(stream, "ORL   ", pc, operand, opcodes.r16(pc+2), true); break;
			case 0xd: bytes = ea_form1_helper(stream, "XORL  ", pc, operand, opcodes.r16(pc+2), true); break;

			case 0x6:
				switch (operand & 0xf)
				{
					case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
						util::stream_format(stream, "CMPL   A, RL%d", (operand & 0x6)>>1);
						bytes = 2;
						break;

					default:
						stream << "UNK ea-type 70 6x";
						break;
				}
				break;

			default:
				stream << "UNK ea-type 70";
				break;
		}
		break;

	case 0x71:  // ea-type instructions
		switch (operand)
		{
			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
			case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
				bytes = ea_form1_helper_noA(stream, "CALLP @", pc, operand, opcodes.r16(pc+2));
				break;

			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				util::stream_format(stream, "INCL   @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x58: case 0x59: case 0x5a: case 0x5b:
				util::stream_format(stream, "INCL   @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x5c: case 0x5d:
				util::stream_format(stream, "INCL   @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x5e:
				util::stream_format(stream, "INCL   @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x5f:
				util::stream_format(stream, "INCL   $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
				util::stream_format(stream, "MOVL   A, RL%d", (operand & 0x6)>>1);
				bytes = 2;
				break;

			case 0x88: case 0x89: case 0x8a: case 0x8b:
				util::stream_format(stream, "MOVL   A, @RW%d", (operand & 0x3));
				bytes = 2;
				break;

			case 0x8c: case 0x8d: case 0x8e: case 0x8f:
				util::stream_format(stream, "MOVL   A, @RW%d+", (operand & 0x3));
				bytes = 2;
				break;

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				util::stream_format(stream, "MOVL   A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x98: case 0x99: case 0x9a: case 0x9b:
				util::stream_format(stream, "MOVL   A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x9c: case 0x9d:
				util::stream_format(stream, "MOVL   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x9e:
				util::stream_format(stream, "MOVL   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x9f:
				util::stream_format(stream, "MOVL   A, $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				util::stream_format(stream, "MOVL   RL%d, A", (operand & 0x6)>>1);
				bytes = 2;
				break;

			case 0xa8: case 0xa9: case 0xaa: case 0xab:
				util::stream_format(stream, "MOVL   @RW%d, A", (operand & 0x3));
				bytes = 2;
				break;

			case 0xac: case 0xad: case 0xae: case 0xaf:
				util::stream_format(stream, "MOVL   @RW%d+, A", (operand & 0x3));
				bytes = 2;
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				util::stream_format(stream, "MOVL   @RW%d + #$%02x, A", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0xbf:
				util::stream_format(stream, "MOVL   $%04x, A", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xc8: case 0xc9: case 0xca: case 0xcb:
				util::stream_format(stream, "MOV    @RW%d, #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			default:
				stream << "UNK ea-type 71";
				break;
		}
		break;

	case 0x72:  // ea-type instructions
		switch (operand)
		{
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				util::stream_format(stream, "INC    @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				util::stream_format(stream, "MOV    A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x98: case 0x99: case 0x9a: case 0x9b:
				util::stream_format(stream, "MOV    A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x9c: case 0x9d:
				util::stream_format(stream, "MOV    A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x9e:
				util::stream_format(stream, "MOV    A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x9f:
				util::stream_format(stream, "MOVW   A, $%04x", (operand & 0x1), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
				util::stream_format(stream, "MOV    @RW%d + #$%02x, A", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				util::stream_format(stream, "MOV    @RW%d + #$%04x, A", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			default:
				stream << "UNK ea-type 72";
				break;
		}
		break;

	case 0x73:  // ea-type instructions
		switch (operand)
		{
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
				util::stream_format(stream, "INCW   @RW%d + #$%02x, A", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x58: case 0x59: case 0x5a: case 0x5b:
				util::stream_format(stream, "INCW   @RW%d + #$%04x, A", (operand & 0x3), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x5c: case 0x5d:
				util::stream_format(stream, "INCW   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x5e:
				util::stream_format(stream, "INCW   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x5f:
				util::stream_format(stream, "INCW   $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
				util::stream_format(stream, "DECW   @RW%d + #$%02x, A", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x78: case 0x79: case 0x7a: case 0x7b:
				util::stream_format(stream, "DECW   @RW%d + #$%04x, A", (operand & 0x3), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x7c: case 0x7d:
				util::stream_format(stream, "DECW   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x7e:
				util::stream_format(stream, "DECW   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x7f:
				util::stream_format(stream, "DECW   $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
				util::stream_format(stream, "MOVW   @RW%d + #$%02x, A", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
				util::stream_format(stream, "MOVW   @RW%d + #$%04x, A", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
			case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
				bytes = ea_form1_helper(stream, "MOVW  ", pc, operand, opcodes.r16(pc+2), false);
				break;

			case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
				util::stream_format(stream, "MOVW   @RW%d + #$%02x, $%04x", (operand & 0x7), opcodes.r8(pc+2), opcodes.r16(pc+3));
				bytes = 5;
				break;

			case 0xd8: case 0xd9: case 0xda: case 0xdb:
				util::stream_format(stream, "MOVW   @RW%d + #$%04x, $%04x", (operand & 0x7), opcodes.r16(pc+2), opcodes.r16(pc+4));
				bytes = 6;
				break;

			case 0xdc: case 0xdd:
				util::stream_format(stream, "MOVW   @RW%d + RW7, $%04x", (operand & 0x1), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xde:
				util::stream_format(stream, "MOVW   @PC + #$04x, $%04x", (operand & 0x1), opcodes.r16(pc+2), opcodes.r16(pc+4));
				bytes = 6;
				break;

			case 0xdf:
				util::stream_format(stream, "MOVW   $%04x, #$%04x", opcodes.r16(pc+2), opcodes.r16(pc+4));
				bytes = 6;
				break;

			default:
				stream << "UNK ea-type 73";
				break;
		}
		break;

	case 0x74:  // ea-type instructions
		switch (operand)
		{
			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x57:
				util::stream_format(stream, "CMP    A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x78: case 0x79: case 0x7a: case 0x7b:
				util::stream_format(stream, "CMP    A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x7c: case 0x7d:
				util::stream_format(stream, "CMP    A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x7e:
				util::stream_format(stream, "CMP    A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x7f:
				util::stream_format(stream, "CMP    A, $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			default:
				stream << "UNK ea-type 74";
				break;
		}
		break;

	case 0x75:  // ea-type instructions
		switch (operand)
		{
			default:
				stream << "UNK ea-type 75";
				break;
		}
		break;

	case 0x76:  // ea-type instructions
		switch (operand)
		{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
				util::stream_format(stream, "ADDW  A, RW%d", (operand & 0x7));
				bytes = 2;
				break;

			case 0x08: case 0x09: case 0x0a: case 0x0b:
				util::stream_format(stream, "ADDW  A, @RW%d", (operand & 0x3));
				bytes = 2;
				break;

			case 0x0c: case 0x0d: case 0x0e: case 0x0f:
				util::stream_format(stream, "ADDW  A, @RW%d+", (operand & 0x3));
				bytes = 2;
				break;

			case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
				util::stream_format(stream, "ADDW   A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x18: case 0x19: case 0x1a: case 0x1b:
				util::stream_format(stream, "ADDW   A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x1c: case 0x1d:
				util::stream_format(stream, "ADDW   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x1e:
				util::stream_format(stream, "ADDW   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x1f:
				util::stream_format(stream, "ADDW  $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				util::stream_format(stream, "SUBW   A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x38: case 0x39: case 0x3a: case 0x3b:
				util::stream_format(stream, "SUBW   A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x3c: case 0x3d:
				util::stream_format(stream, "SUBW   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x3e:
				util::stream_format(stream, "SUBW   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x3f:
				util::stream_format(stream, "SUBW  $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
				util::stream_format(stream, "CMPW  A, RW%d", (operand & 0x7));
				bytes = 2;
				break;

			case 0x68: case 0x69: case 0x6a: case 0x6b:
				util::stream_format(stream, "CMPW  A, @RW%d", (operand & 0x3));
				bytes = 2;
				break;

			case 0x6c: case 0x6d: case 0x6e: case 0x6f:
				util::stream_format(stream, "CMPW  A, @RW%d+", (operand & 0x3));
				bytes = 2;
				break;

			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x57:
				util::stream_format(stream, "CMPW   A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			case 0x78: case 0x79: case 0x7a: case 0x7b:
				util::stream_format(stream, "CMPW   A, @RW%d + #$%04x", (operand & 0x7), opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0x7c: case 0x7d:
				util::stream_format(stream, "CMPW   A, @RW%d + RW7", (operand & 0x1));
				bytes = 2;
				break;

			case 0x7e:
				util::stream_format(stream, "CMPW   A, @PC + #$04x ($%04X)", (operand & 0x1), opcodes.r16(pc+2), (s16)opcodes.r16(pc+2) + (pc & 0xffff));
				bytes = 4;
				break;

			case 0x7f:
				util::stream_format(stream, "CMPW  $%04x", opcodes.r16(pc+2));
				bytes = 4;
				break;

			case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
				util::stream_format(stream, "ORW    A, @RW%d", (operand & 0x1));
				bytes = 2;
				break;

			default:
				stream << "UNK ea-type 76";
				break;
		}
		break;

	case 0x77:  // ea-type instructions
		switch (operand)
		{
			default:
				stream << "UNK ea-type 77";
				break;
		}
		break;

	case 0x78:  // ea-type instructions
		switch (operand)
		{
			case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
				util::stream_format(stream, "MULUW  A, RW%d", (operand & 0x7));
				bytes = 2;
				break;

			case 0x28: case 0x29: case 0x2a: case 0x2b:
				util::stream_format(stream, "MULUW  A, @RW%d", (operand & 0x3));
				bytes = 2;
				break;

			case 0x2c: case 0x2d: case 0x2e: case 0x2f:
				util::stream_format(stream, "MULUW  A, @RW%d+", (operand & 0x3));
				bytes = 2;
				break;

			case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
				util::stream_format(stream, "MULUW  A, @RW%d + #$%02x", (operand & 0x7), opcodes.r8(pc+2));
				bytes = 3;
				break;

			default:
				stream << "UNK ea-type 78";
				break;
		}
		break;

	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
		util::stream_format(stream, "MOV    A, R%d", (opcode & 0x7));
		break;

	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
		util::stream_format(stream, "MOVW   A, RW%d", (opcode & 0x7));
		break;

	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		util::stream_format(stream, "MOV    R%d, A", (opcode & 0x7));
		break;

	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
		util::stream_format(stream, "MOVW   RW%d, A", (opcode & 0x7));
		break;

	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		util::stream_format(stream, "MOV    R%d, #$%02x", (opcode & 0x7), operand);
		bytes = 2;
		break;

	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
		util::stream_format(stream, "MOVW   RW%d, #$%04x", (opcode & 0x7), opcodes.r16(pc+1));
		bytes = 3;
		break;

	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		util::stream_format(stream, "MOVX   A, R%d", (opcode & 0x7));
		break;

	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
		util::stream_format(stream, "MOVW   A, @RW%d + #$%02x", (opcode & 0x7), operand);
		bytes = 2;
		break;

	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
		util::stream_format(stream, "MOVX   A, @RW%d + #$%02x", (opcode & 0x7), operand);
		bytes = 2;
		break;

	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
		util::stream_format(stream, "MOVW   @RW%d + #$%02x, A", (opcode & 0x7), operand);
		bytes = 2;
		break;

	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
		util::stream_format(stream, "MOVN   A, #$%01x", (opcode & 0xf));
		break;

	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		util::stream_format(stream, "CALL   #$%01x", (opcode & 0xf));
		break;

	case 0xf0: branch_helper(stream, "BEQ   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf1: branch_helper(stream, "BNE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf2: branch_helper(stream, "BC    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf3: branch_helper(stream, "BNC   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf4: branch_helper(stream, "BN    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf5: branch_helper(stream, "BP    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf6: branch_helper(stream, "BV    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf7: branch_helper(stream, "BNV   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf8: branch_helper(stream, "BT    ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xf9: branch_helper(stream, "BNT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xfa: branch_helper(stream, "BLT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xfb: branch_helper(stream, "BGE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xfc: branch_helper(stream, "BLE   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xfd: branch_helper(stream, "BGT   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xfe: branch_helper(stream, "BLS   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;
	case 0xff: branch_helper(stream, "BHI   ", (pc & 0xffff)+2, (s8)operand); bytes = 2; break;

	default:
		stream << "???";
		break;

	}

	return bytes | SUPPORTED;
}
