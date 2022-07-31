// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*******************************************************************************

    mb88dasm.c
    Core implementation for the portable Fujitsu MB88xx series MCU disassembler.

    Written by Ernesto Corvi

*******************************************************************************/

#include "emu.h"
#include "mb88dasm.h"

u32 mb88_disassembler::opcode_alignment() const
{
	return 1;
}

offs_t mb88_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	unsigned startpc = pc;
	uint8_t op = opcodes.r8(pc++);
	uint8_t arg = opcodes.r8(pc);

	switch( op )
	{
		case 0x00: util::stream_format( stream,  "NOP" ); break;
		case 0x01: util::stream_format( stream,  "OUTO" ); break;
		case 0x02: util::stream_format( stream,  "OUTP" ); break;
		case 0x03: util::stream_format( stream,  "OUT" ); break;
		case 0x04: util::stream_format( stream,  "TAY" ); break;
		case 0x05: util::stream_format( stream,  "TATH" ); break;
		case 0x06: util::stream_format( stream,  "TATL" ); break;
		case 0x07: util::stream_format( stream,  "TAS" ); break;
		case 0x08: util::stream_format( stream,  "ICY" ); break;
		case 0x09: util::stream_format( stream,  "ICM" ); break;
		case 0x0A: util::stream_format( stream,  "STIC" ); break;
		case 0x0B: util::stream_format( stream,  "X" ); break;
		case 0x0C: util::stream_format( stream,  "ROL" ); break;
		case 0x0D: util::stream_format( stream,  "L" ); break;
		case 0x0E: util::stream_format( stream,  "ADC" ); break;
		case 0x0F: util::stream_format( stream,  "AND" ); break;
		case 0x10: util::stream_format( stream,  "DAA" ); break;
		case 0x11: util::stream_format( stream,  "DAS" ); break;
		case 0x12: util::stream_format( stream,  "INK" ); break;
		case 0x13: util::stream_format( stream,  "IN" ); break;
		case 0x14: util::stream_format( stream,  "TYA" ); break;
		case 0x15: util::stream_format( stream,  "TTHA" ); break;
		case 0x16: util::stream_format( stream,  "TTLA" ); break;
		case 0x17: util::stream_format( stream,  "TSA" ); break;
		case 0x18: util::stream_format( stream,  "DCY" ); break;
		case 0x19: util::stream_format( stream,  "DCM" ); break;
		case 0x1A: util::stream_format( stream,  "STDC" ); break;
		case 0x1B: util::stream_format( stream,  "XX" ); break;
		case 0x1C: util::stream_format( stream,  "ROR" ); break;
		case 0x1D: util::stream_format( stream,  "ST" ); break;
		case 0x1E: util::stream_format( stream,  "SBC" ); break;
		case 0x1F: util::stream_format( stream,  "OR" ); break;
		case 0x20: util::stream_format( stream,  "SETR" ); break;
		case 0x21: util::stream_format( stream,  "SETC" ); break;
		case 0x22: util::stream_format( stream,  "RSTR" ); break;
		case 0x23: util::stream_format( stream,  "RSTC" ); break;
		case 0x24: util::stream_format( stream,  "TSTR" ); break;
		case 0x25: util::stream_format( stream,  "TSTI" ); break;
		case 0x26: util::stream_format( stream,  "TSTV" ); break;
		case 0x27: util::stream_format( stream,  "TSTS" ); break;
		case 0x28: util::stream_format( stream,  "TSTC" ); break;
		case 0x29: util::stream_format( stream,  "TSTZ" ); break;
		case 0x2A: util::stream_format( stream,  "STS" ); break;
		case 0x2B: util::stream_format( stream,  "LS" ); break;
		case 0x2C: util::stream_format( stream,  "RTS" ); break;
		case 0x2D: util::stream_format( stream,  "NEG" ); break;
		case 0x2E: util::stream_format( stream,  "C" ); break;
		case 0x2F: util::stream_format( stream,  "EOR" ); break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33: util::stream_format( stream,  "SBIT  %d", op&3 ); break;
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37: util::stream_format( stream,  "RBIT  %d", op&3 ); break;
		case 0x38:
		case 0x39:
		case 0x3A:
		case 0x3B: util::stream_format( stream,  "TBIT  %d", op&3 ); break;
		case 0x3C: util::stream_format( stream,  "RTI" ); break;
		case 0x3D: util::stream_format( stream,  "JPA   #$%02X", arg ); pc++; break;
		case 0x3E: util::stream_format( stream,  "EN    #$%02X", arg ); pc++; break;
		case 0x3F: util::stream_format( stream,  "DIS   #$%02X", arg ); pc++; break;
		case 0x40:
		case 0x41:
		case 0x42:
		case 0x43: util::stream_format( stream,  "SETD  %d", op&3 ); break;
		case 0x44:
		case 0x45:
		case 0x46:
		case 0x47: util::stream_format( stream,  "RSTD  %d", op&3 ); break;
		case 0x48:
		case 0x49:
		case 0x4A:
		case 0x4B: util::stream_format( stream,  "TSTD  %d", (op&3)+8 ); break;
		case 0x4C:
		case 0x4D:
		case 0x4E:
		case 0x4F: util::stream_format( stream,  "TBA   %d", op&3 ); break;
		case 0x50:
		case 0x51:
		case 0x52:
		case 0x53: util::stream_format( stream,  "XD    %d", op&3 ); break;
		case 0x54:
		case 0x55:
		case 0x56:
		case 0x57: util::stream_format( stream,  "XYD   %d", (op&3)+4 ); break;
		case 0x58:
		case 0x59:
		case 0x5A:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F: util::stream_format( stream,  "LXI   #$%1X", op&7 ); break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x66:
		case 0x67: util::stream_format( stream,  "CALL  $%02X%02X", op&7, arg ); pc++; break;
		case 0x68:
		case 0x69:
		case 0x6A:
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x6F: util::stream_format( stream,  "JPL   $%02X%02X", op&7, arg ); pc++; break;
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
		case 0x77:
		case 0x78:
		case 0x79:
		case 0x7A:
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
		case 0x7F: util::stream_format( stream,  "AI    #$%1X", op&0xf ); break;
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8A:
		case 0x8B:
		case 0x8C:
		case 0x8D:
		case 0x8E:
		case 0x8F: util::stream_format( stream,  "LYI   #$%1X", op&0xf ); break;
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F: util::stream_format( stream,  "LI    #$%1X", op&0xf ); break;
		case 0xA0:
		case 0xA1:
		case 0xA2:
		case 0xA3:
		case 0xA4:
		case 0xA5:
		case 0xA6:
		case 0xA7:
		case 0xA8:
		case 0xA9:
		case 0xAA:
		case 0xAB:
		case 0xAC:
		case 0xAD:
		case 0xAE:
		case 0xAF: util::stream_format( stream,  "CYI   #$%1X", op&0xf ); break;
		case 0xB0:
		case 0xB1:
		case 0xB2:
		case 0xB3:
		case 0xB4:
		case 0xB5:
		case 0xB6:
		case 0xB7:
		case 0xB8:
		case 0xB9:
		case 0xBA:
		case 0xBB:
		case 0xBC:
		case 0xBD:
		case 0xBE:
		case 0xBF: util::stream_format( stream,  "CI    #$%1X", op&0xf ); break;

		default:
			/* C0-FF */
			util::stream_format( stream,  "JMP   $%04X", (pc&(~0x3f))+ op-0xC0 );
		break;
	}

	return (pc - startpc) | SUPPORTED;
}
