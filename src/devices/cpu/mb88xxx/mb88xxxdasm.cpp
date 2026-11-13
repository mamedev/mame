// license:BSD-3-Clause
// copyright-holders:trwgQ26xxx
/*******************************************************************************

	Fujitsu MB8840x / MB8850xH series 4-bit MCU disassembler.

	Written by trwgQ26xxx, based on Ernesto Corvi's MB88xx series MCU disassembler.

	Instruction set differences vs. the older MB88xx (MB8841/MB8843 etc.):
		- 0x3D is a two-byte EXT prefix, not the single-byte JPA
		- CALL uses opcodes 0x60-0x6F (16 variants, 4-bit page), not 0x60-0x67
		- JPL  uses opcodes 0x70-0x7F (16 variants, 4-bit page), not 0x68-0x6F
		- AI / ICA / DCA live under the EXT prefix (3D 8x), not at 0x70-0x7F
		- Added: JPXY, LRXA, LXID, SBA, RBA, ICX, RST (all under EXT)

*******************************************************************************/

#include "emu.h"
#include "mb88xxxdasm.h"

offs_t mb88xxx_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	const offs_t startpc = pc;
	const u8 op = opcodes.r8(pc++);
	const u8 arg = opcodes.r8(pc);	// second byte - peeked, may not be consumed

	switch (op)
	{
		// -- 1-byte / 1-cycle -------------------------------------------------
		case 0x00: util::stream_format(stream, "NOP"); break;
		case 0x01: util::stream_format(stream, "OUTO"); break;
		case 0x02: util::stream_format(stream, "OUTP"); break;
		case 0x03: util::stream_format(stream, "OUT"); break;
		case 0x04: util::stream_format(stream, "TAY"); break;
		case 0x05: util::stream_format(stream, "TATH"); break;
		case 0x06: util::stream_format(stream, "TATL"); break;
		case 0x07: util::stream_format(stream, "TAS"); break;
		case 0x08: util::stream_format(stream, "ICY"); break;
		case 0x09: util::stream_format(stream, "ICM"); break;
		case 0x0a: util::stream_format(stream, "STIC"); break;
		case 0x0b: util::stream_format(stream, "X"); break;
		case 0x0c: util::stream_format(stream, "ROL"); break;
		case 0x0d: util::stream_format(stream, "L"); break;
		case 0x0e: util::stream_format(stream, "ADC"); break;
		case 0x0f: util::stream_format(stream, "AND"); break;
		case 0x10: util::stream_format(stream, "DAA"); break;
		case 0x11: util::stream_format(stream, "DAS"); break;
		case 0x12: util::stream_format(stream, "INK"); break;
		case 0x13: util::stream_format(stream, "IN"); break;
		case 0x14: util::stream_format(stream, "TYA"); break;
		case 0x15: util::stream_format(stream, "TTHA"); break;
		case 0x16: util::stream_format(stream, "TTLA"); break;
		case 0x17: util::stream_format(stream, "TSA"); break;
		case 0x18: util::stream_format(stream, "DCY"); break;
		case 0x19: util::stream_format(stream, "DCM"); break;
		case 0x1a: util::stream_format(stream, "STDC"); break;
		case 0x1b: util::stream_format(stream, "XX"); break;
		case 0x1c: util::stream_format(stream, "ROR"); break;
		case 0x1d: util::stream_format(stream, "ST"); break;
		case 0x1e: util::stream_format(stream, "SBC"); break;
		case 0x1f: util::stream_format(stream, "OR"); break;
		case 0x20: util::stream_format(stream, "SETR"); break;
		case 0x21: util::stream_format(stream, "SETC"); break;
		case 0x22: util::stream_format(stream, "RSTR"); break;
		case 0x23: util::stream_format(stream, "RSTC"); break;
		case 0x24: util::stream_format(stream, "TSTR"); break;
		case 0x25: util::stream_format(stream, "TSTI"); break;
		case 0x26: util::stream_format(stream, "TSTV"); break;
		case 0x27: util::stream_format(stream, "TSTS"); break;
		case 0x28: util::stream_format(stream, "TSTC"); break;
		case 0x29: util::stream_format(stream, "TSTZ"); break;
		case 0x2a: util::stream_format(stream, "STS"); break;
		case 0x2b: util::stream_format(stream, "LS"); break;
		case 0x2c: util::stream_format(stream, "RTS"); break;
		case 0x2d: util::stream_format(stream, "NEG"); break;
		case 0x2e: util::stream_format(stream, "C"); break;
		case 0x2f: util::stream_format(stream, "EOR"); break;

		case 0x30: case 0x31: case 0x32: case 0x33:
			util::stream_format(stream, "SBIT  %d", op & 0x3); break;
		case 0x34: case 0x35: case 0x36: case 0x37:
			util::stream_format(stream, "RBIT  %d", op & 0x3); break;
		case 0x38: case 0x39: case 0x3a: case 0x3b:
			util::stream_format(stream, "TBIT  %d", op & 0x3); break;

		case 0x3c: util::stream_format(stream, "RTI"); break;

		// -- EXT prefix (2 bytes / 2 cycles; LRXA = 2 bytes / 3 cycles) ------
		case 0x3d:
		{
			const u8 ext = opcodes.r8(pc++);

			if ((ext >= 0x00) && (ext <= 0x1f))
			{
				// JPXY $page  - unconditional branch to page ext, PC from X/Y
				util::stream_format(stream, "JPXY  $%02X", ext & 0x1f);
			}
			else if ((ext >= 0x20) && (ext <= 0x3f))
			{
				// LRXA #imm  - load ROM nibble pair into X (high) and A (low)
				util::stream_format(stream, "LRXA  #$%02X", ext & 0x1f);
			}
			else if ((ext >= 0x80) && (ext <= 0x8f))
			{
				// AI / ICA / DCA
				const u8 imm = ext & 0x0f;
				if (imm == 0x1)
					util::stream_format(stream, "ICA");
				else if (imm == 0xf)
					util::stream_format(stream, "DCA");
				else
					util::stream_format(stream, "AI    #$%1X", imm);
			}
			else if ((ext >= 0x90) && (ext <= 0x9f))
			{
				util::stream_format(stream, "LXID  #$%1X", ext & 0xf);
			}
			else if ((ext >= 0xa0) && (ext <= 0xa3))
			{
				util::stream_format(stream, "SBA   %d", ext & 0x3);
			}
			else if ((ext >= 0xa4) && (ext <= 0xa7))
			{
				util::stream_format(stream, "RBA   %d", ext & 0x3);
			}
			else if (ext == 0xac)
			{
				util::stream_format(stream, "ICX");
			}
			else if (ext == 0xad)
			{
				util::stream_format(stream, "RST");
			}
			else
			{
				util::stream_format(stream, "EXT   #$%02X", ext); // undefined
			}
			break;
		}

		// -- EN / DIS (2 bytes / 2 cycles) ------------------------------------
		case 0x3e: util::stream_format(stream, "EN    #$%02X", arg); pc++; break;
		case 0x3f: util::stream_format(stream, "DIS   #$%02X", arg); pc++; break;

		// -- Port bit operations (1 byte / 1 cycle) ---------------------------
		case 0x40: case 0x41: case 0x42: case 0x43:
			util::stream_format(stream, "SETD  %d", op & 0x3); break;
		case 0x44: case 0x45: case 0x46: case 0x47:
			util::stream_format(stream, "RSTD  %d", op & 0x3); break;
		case 0x48: case 0x49: case 0x4a: case 0x4b:
			util::stream_format(stream, "TSTD  %d", (op & 0x3) + 8); break;
		case 0x4c: case 0x4d: case 0x4e: case 0x4f:
			util::stream_format(stream, "TBA   %d", op & 0x3); break;

		// -- Memory exchange (1 byte / 1 cycle) -------------------------------
		case 0x50: case 0x51: case 0x52: case 0x53:
			util::stream_format(stream, "XD    %d", op & 0x3); break;
		case 0x54: case 0x55: case 0x56: case 0x57:
			util::stream_format(stream, "XYD   %d", (op & 0x3) + 4); break;

		// -- LXI (1 byte / 1 cycle) -------------------------------------------
		case 0x58: case 0x59: case 0x5a: case 0x5b:
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			util::stream_format(stream, "LXI   #$%1X", op & 0x7); break;

		// -- CALL (2 bytes / 2 cycles) - 4-bit page from opcode ---------------
		case 0x60: case 0x61: case 0x62: case 0x63:
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b:
		case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			util::stream_format(stream, "CALL  $%04X", (u16(op & 0xf) << 8) | arg); pc++; break;

		// -- JPL (2 bytes / 2 cycles) - 4-bit page from opcode ----------------
		case 0x70: case 0x71: case 0x72: case 0x73:
		case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b:
		case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			util::stream_format(stream, "JPL   $%04X", (u16(op & 0xf) << 8) | arg); pc++; break;

		// -- Immediate loads / compares (1 byte / 1 cycle) --------------------
		case 0x80: case 0x81: case 0x82: case 0x83:
		case 0x84: case 0x85: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8a: case 0x8b:
		case 0x8c: case 0x8d: case 0x8e: case 0x8f:
			util::stream_format(stream, "LYI   #$%1X", op & 0xf); break;

		case 0x90: // CLA (alias for LI #0)
			util::stream_format(stream, "CLA"); break;
		case 0x91: case 0x92: case 0x93:
		case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9a: case 0x9b:
		case 0x9c: case 0x9d: case 0x9e: case 0x9f:
			util::stream_format(stream, "LI    #$%1X", op & 0xf); break;

		case 0xa0: case 0xa1: case 0xa2: case 0xa3:
		case 0xa4: case 0xa5: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xaa: case 0xab:
		case 0xac: case 0xad: case 0xae: case 0xaf:
			util::stream_format(stream, "CYI   #$%1X", op & 0xf); break;

		case 0xb0: case 0xb1: case 0xb2: case 0xb3:
		case 0xb4: case 0xb5: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xba: case 0xbb:
		case 0xbc: case 0xbd: case 0xbe: case 0xbf:
			util::stream_format(stream, "CI    #$%1X", op & 0xf); break;

		// -- JMP (1 byte / 1 cycle) - short branch within current 64-byte page
		default: // 0xC0-0xFF
			util::stream_format(stream, "JMP   $%04X", (pc & ~offs_t(0x3f)) | (op & 0x3f)); break;
	}

	return (pc - startpc) | SUPPORTED;
}
