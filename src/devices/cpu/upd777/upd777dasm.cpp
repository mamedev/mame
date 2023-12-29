// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "upd777dasm.h"

upd777_disassembler::upd777_disassembler()
	: util::disasm_interface()
{
	populate_addr_table(m_table);
}

u32 upd777_disassembler::opcode_alignment() const
{
	return 1;
}

u16 upd777_disassembler::get_table(u16 addr)
{
	if (addr < 0xfe0)
	{
		return m_table[addr];
	}
	else
	{
		return 0xffff;
	}
}

offs_t upd777_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd777_disassembler::data_buffer &opcodes, const upd777_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);

	if (inst < 0x480)
	{
		switch (inst)
		{
		case 0b000000000000:
		{ // No Operation
			util::stream_format(stream, "%04x: NOP", get_table(pc));
			break;
		}

		// case 0b000000000001: case 0b000000000010: case 0b000000000011:

		case 0b000000000100:
		{ // Skip if (Gun Port Latch) = 1
			util::stream_format(stream, "%04x: GPL", get_table(pc));
			break;
		}

		//case 0b000000000101: case 0b000000000110: case 0b000000000111:

		case 0b000000001000:
		{ // Move H[5:1] to Line Buffer Register[5:1]
			util::stream_format(stream, "%04x: H->NRM", get_table(pc));
			break;
		}

		//case 0b000000001001: case 0b000000001010: case 0b000000001011: case 0b000000001100: case 0b000000001101: case 0b000000001110: case 0b000000001111: case 0b000000010000: case 0b000000010001: case 0b000000010010: case 0b000000010011: case 0b000000010100: case 0b000000010101: case 0b000000010110: case 0b000000010111:
		
		case 0b000000011000:
		{ // H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
			util::stream_format(stream, "%04x: H<->X", get_table(pc));
			break;
		}

		// case 0b000000011001: case 0b000000011010: case 0b000000011011: case 0b000000011100: case 0b000000011101: case 0b000000011110: case 0b000000011111:

		case 0b000000100000:
		{ // Subroutine End, Pop down address stack
			util::stream_format(stream, "%04x: SRE", get_table(pc));
			break;
		}

		// case 0b000000100001: case 0b000000100010: case 0b000000100011: case 0b000000100100: case 0b000000100101: case 0b000000100110: case 0b000000100111:

		case 0b000000101000: case 0b000000101001:
		{ // Shift STB[4:1], N->STB[1]
			util::stream_format(stream, "%04x: %d->STB", get_table(pc), inst & 1);
			break;
		}

		// case 0b000000101010: case 0b000000101011: case 0b000000101100: case 0b000000101101: case 0b000000101110: case 0b000000101111:

		case 0b000000110000:
		{ // Skip if (PD1 input) = 1
			util::stream_format(stream, "%04x: PD1 J", get_table(pc));
			break;
		}

		// case 0b000000110001: case 0b000000110010: case 0b000000110011:
		
		case 0b000000110100:
		{ // Skip if (PD2 input) = 1
			util::stream_format(stream, "%04x: PD2 J", get_table(pc));
			break;
		}

		//case 0b000000110101: case 0b000000110110: case 0b000000110111:
				
		case 0b000000111000:
		{ // Skip if (PD3 input) = 1
			util::stream_format(stream, "%04x: PD3 J", get_table(pc));
			break;
		}

		//case 0b000000111001: case 0b000000111010: case 0b000000111011:
		
		case 0b000000111100:
		{ // Skip if (PD4 input) = 1
			util::stream_format(stream, "%04x: PD4 J", get_table(pc));
			break;
		}

		//case 0b000000111101: case 0b000000111110: case 0b000000111111: case 0b000001000000: case 0b000001000001: case 0b000001000010: case 0b000001000011: case 0b000001000100: case 0b000001000101: case 0b000001000110: case 0b000001000111: case 0b000001001000:

		case 0b000001001001:
		{ // Skip if (4H Horizontal Blank) = 1
			util::stream_format(stream, "%04x: 4H BLK", get_table(pc));
			break;
		}

		case 0b000001001010:
		{ // Skip if (Vertical Blank) = 1, 0M[[18:00],[3]][1]

			util::stream_format(stream, "%04x: VBLK", get_table(pc));
			break;
		}

		//case 0b000001001011:

		case 0b000001001100:
		{ // Skip if (GP&SW/ input) = 1
			util::stream_format(stream, "%04x: GPSW/", get_table(pc));
			break;
		}

		// case 0b000001001101: case 0b000001001110: case 0b000001001111: case 0b000001010000: case 0b000001010001: case 0b000001010010: case 0b000001010011:

		case 0b000001010100:
		{ // Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
			util::stream_format(stream, "%04x: A->MA", get_table(pc));
			break;
		}

		// case 0b000001010101: case 0b000001010110: case 0b000001010111:

		case 0b000001011000:
		{ // Move M[H[5:1]][28:1] to (A4[7:1],A3[7:1],A2[7:1],A1[7:1])
			util::stream_format(stream, "%04x: MA->A", get_table(pc));
			break;
		}

		// case 0b000001011001: case 0b000001011010: case 0b000001011011:

		case 0b000001011100:
		{ // Exchange (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) and M[H[5:1]][28:1]
			util::stream_format(stream, "%04x: MA<->A", get_table(pc));
			break;
		}

		// case 0b000001011101: case 0b000001011110: case 0b000001011111:

		case 0b000001100000:
		{ // Subroutine End, Pop down address stack, Skip
			util::stream_format(stream, "%04x: SRE+1", get_table(pc));
			break;
		}

		// case 0b000001100001: case 0b000001100010: case 0b000001100011: case 0b000001100100: case 0b000001100101: case 0b000001100110: case 0b000001100111: case 0b000001101000: case 0b000001101001: case 0b000001101010: case 0b000001101011: case 0b000001101100: case 0b000001101101: case 0b000001101110: case 0b000001101111:

		case 0b000001110000:
		{ // Skip if (PD1 input) = 0
			util::stream_format(stream, "%04x: PD1 /J", get_table(pc), inst);
			break;
		}

		// case 0b000001110001: case 0b000001110010: case 0b000001110011:

		case 0b000001110100:
		{ // Skip if (PD2 input) = 0
			util::stream_format(stream, "%04x: PD2 /J", get_table(pc), inst);
			break;
		}

		// case 0b000001110101: case 0b000001110110: case 0b000001110111:

		case 0b000001111000:
		{ // Skip if (PD3 input) = 0
			util::stream_format(stream, "%04x: PD3 /J", get_table(pc), inst);
			break;
		}

		// case 0b000001111001: case 0b000001111010: case 0b000001111011:

		case 0b000001111100:
		{ // Skip if (PD4 input) = 0
			util::stream_format(stream, "%04x: PD4 /J", get_table(pc), inst);
			break;
		}

		// case 0b000001111101: case 0b000001111110: case 0b000001111111:

		case 0b000010000000: case 0b000010000001: case 0b000010000010: case 0b000010000011: case 0b000010000100: case 0b000010000101: case 0b000010000110: case 0b000010000111: case 0b000010001000: case 0b000010001001: case 0b000010001010: case 0b000010001011: case 0b000010001100: case 0b000010001101: case 0b000010001110: case 0b000010001111: case 0b000010010000: case 0b000010010001: case 0b000010010010: case 0b000010010011: case 0b000010010100: case 0b000010010101: case 0b000010010110: case 0b000010010111: case 0b000010011000: case 0b000010011001: case 0b000010011010: case 0b000010011011: case 0b000010011100: case 0b000010011101: case 0b000010011110: case 0b000010011111: case 0b000010100000: case 0b000010100001: case 0b000010100010: case 0b000010100011: case 0b000010100100: case 0b000010100101: case 0b000010100110: case 0b000010100111: case 0b000010101000: case 0b000010101001: case 0b000010101010: case 0b000010101011: case 0b000010101100: case 0b000010101101: case 0b000010101110: case 0b000010101111: case 0b000010110000: case 0b000010110001: case 0b000010110010: case 0b000010110011: case 0b000010110100: case 0b000010110101: case 0b000010110110: case 0b000010110111: case 0b000010111000: case 0b000010111001: case 0b000010111010: case 0b000010111011: case 0b000010111100: case 0b000010111101: case 0b000010111110: case 0b000010111111: case 0b000011000000: case 0b000011000001: case 0b000011000010: case 0b000011000011: case 0b000011000100: case 0b000011000101: case 0b000011000110: case 0b000011000111: case 0b000011001000: case 0b000011001001: case 0b000011001010: case 0b000011001011: case 0b000011001100: case 0b000011001101: case 0b000011001110: case 0b000011001111: case 0b000011010000: case 0b000011010001: case 0b000011010010: case 0b000011010011: case 0b000011010100: case 0b000011010101: case 0b000011010110: case 0b000011010111: case 0b000011011000: case 0b000011011001: case 0b000011011010: case 0b000011011011: case 0b000011011100: case 0b000011011101: case 0b000011011110: case 0b000011011111: case 0b000011100000: case 0b000011100001: case 0b000011100010: case 0b000011100011: case 0b000011100100: case 0b000011100101: case 0b000011100110: case 0b000011100111: case 0b000011101000: case 0b000011101001: case 0b000011101010: case 0b000011101011: case 0b000011101100: case 0b000011101101: case 0b000011101110: case 0b000011101111: case 0b000011110000: case 0b000011110001: case 0b000011110010: case 0b000011110011: case 0b000011110100: case 0b000011110101: case 0b000011110110: case 0b000011110111: case 0b000011111000: case 0b000011111001: case 0b000011111010: case 0b000011111011: case 0b000011111100: case 0b000011111101: case 0b000011111110: case 0b000011111111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-K[7:1]) makes borrow
			util::stream_format(stream, "%04x: M-K(%02x)", get_table(pc), inst & 0x7f);
			break;
		}


		case 0b000100000000: case 0b000100000001: case 0b000100000010: case 0b000100000011: case 0b000100000100: case 0b000100000101: case 0b000100000110: case 0b000100000111: case 0b000100001000: case 0b000100001001: case 0b000100001010: case 0b000100001011: case 0b000100001100: case 0b000100001101: case 0b000100001110: case 0b000100001111:	case 0b000100010000: case 0b000100010001: case 0b000100010010: case 0b000100010011: case 0b000100010100: case 0b000100010101: case 0b000100010110: case 0b000100010111:	case 0b000100011000: case 0b000100011001: case 0b000100011010: case 0b000100011011: case 0b000100011100: case 0b000100011101: case 0b000100011110: case 0b000100011111:	case 0b000100100000: case 0b000100100001: case 0b000100100010: case 0b000100100011: case 0b000100100100: case 0b000100100101: case 0b000100100110: case 0b000100100111:	case 0b000100101000: case 0b000100101001: case 0b000100101010: case 0b000100101011: case 0b000100101100: case 0b000100101101: case 0b000100101110: case 0b000100101111:	case 0b000100110000: case 0b000100110001: case 0b000100110010: case 0b000100110011: case 0b000100110100: case 0b000100110101: case 0b000100110110: case 0b000100110111:	case 0b000100111000: case 0b000100111001: case 0b000100111010: case 0b000100111011: case 0b000100111100: case 0b000100111101: case 0b000100111110: case 0b000100111111:	case 0b000101000000: case 0b000101000001: case 0b000101000010: case 0b000101000011: case 0b000101000100: case 0b000101000101: case 0b000101000110: case 0b000101000111:	case 0b000101001000: case 0b000101001001: case 0b000101001010: case 0b000101001011: case 0b000101001100: case 0b000101001101: case 0b000101001110: case 0b000101001111:	case 0b000101010000: case 0b000101010001: case 0b000101010010: case 0b000101010011: case 0b000101010100: case 0b000101010101: case 0b000101010110: case 0b000101010111:	case 0b000101011000: case 0b000101011001: case 0b000101011010: case 0b000101011011: case 0b000101011100: case 0b000101011101: case 0b000101011110: case 0b000101011111:	case 0b000101100000: case 0b000101100001: case 0b000101100010: case 0b000101100011: case 0b000101100100: case 0b000101100101: case 0b000101100110: case 0b000101100111:	case 0b000101101000: case 0b000101101001: case 0b000101101010: case 0b000101101011: case 0b000101101100: case 0b000101101101: case 0b000101101110: case 0b000101101111:	case 0b000101110000: case 0b000101110001: case 0b000101110010: case 0b000101110011: case 0b000101110100: case 0b000101110101: case 0b000101110110: case 0b000101110111:	case 0b000101111000: case 0b000101111001: case 0b000101111010: case 0b000101111011: case 0b000101111100: case 0b000101111101: case 0b000101111110: case 0b000101111111:
		{ // M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]

			util::stream_format(stream, "%04x: M+K(%02x)->M, N(%01x)->L", get_table(pc), inst & 0x1f, (inst >> 5) & 0x3);
			break;
		}

		case 0b000110000000: case 0b000110000001: case 0b000110000010: case 0b000110000011: case 0b000110000100: case 0b000110000101: case 0b000110000110: case 0b000110000111:	case 0b000110001000: case 0b000110001001: case 0b000110001010: case 0b000110001011: case 0b000110001100: case 0b000110001101: case 0b000110001110: case 0b000110001111:	case 0b000110010000: case 0b000110010001: case 0b000110010010: case 0b000110010011: case 0b000110010100: case 0b000110010101: case 0b000110010110: case 0b000110010111:	case 0b000110011000: case 0b000110011001: case 0b000110011010: case 0b000110011011: case 0b000110011100: case 0b000110011101: case 0b000110011110: case 0b000110011111:	case 0b000110100000: case 0b000110100001: case 0b000110100010: case 0b000110100011: case 0b000110100100: case 0b000110100101: case 0b000110100110: case 0b000110100111:	case 0b000110101000: case 0b000110101001: case 0b000110101010: case 0b000110101011: case 0b000110101100: case 0b000110101101: case 0b000110101110: case 0b000110101111:	case 0b000110110000: case 0b000110110001: case 0b000110110010: case 0b000110110011: case 0b000110110100: case 0b000110110101: case 0b000110110110: case 0b000110110111:	case 0b000110111000: case 0b000110111001: case 0b000110111010: case 0b000110111011: case 0b000110111100: case 0b000110111101: case 0b000110111110: case 0b000110111111:	case 0b000111000000: case 0b000111000001: case 0b000111000010: case 0b000111000011: case 0b000111000100: case 0b000111000101: case 0b000111000110: case 0b000111000111:	case 0b000111001000: case 0b000111001001: case 0b000111001010: case 0b000111001011: case 0b000111001100: case 0b000111001101: case 0b000111001110: case 0b000111001111:	case 0b000111010000: case 0b000111010001: case 0b000111010010: case 0b000111010011: case 0b000111010100: case 0b000111010101: case 0b000111010110: case 0b000111010111:	case 0b000111011000: case 0b000111011001: case 0b000111011010: case 0b000111011011: case 0b000111011100: case 0b000111011101: case 0b000111011110: case 0b000111011111:	case 0b000111100000: case 0b000111100001: case 0b000111100010: case 0b000111100011: case 0b000111100100: case 0b000111100101: case 0b000111100110: case 0b000111100111:	case 0b000111101000: case 0b000111101001: case 0b000111101010: case 0b000111101011: case 0b000111101100: case 0b000111101101: case 0b000111101110: case 0b000111101111:	case 0b000111110000: case 0b000111110001: case 0b000111110010: case 0b000111110011: case 0b000111110100: case 0b000111110101: case 0b000111110110: case 0b000111110111:	case 0b000111111000: case 0b000111111001: case 0b000111111010: case 0b000111111011: case 0b000111111100: case 0b000111111101: case 0b000111111110: case 0b000111111111:
		{ // M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: M-K(%02x)->M, N(%01x)->L", get_table(pc), inst & 0x1f, (inst >> 5) & 0x3);
			break;
		}			


		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		// 200
		case 0b001000000000: case 0b001000000001: case 0b001000000010: case 0b001000000011:
		{ // Skip if (A1[7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: A1·A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 204
		//case 0b001000000100: case 0b001000000101: case 0b001000000110: case 0b001000000111:

		// 208
		case 0b001000001000: case 0b001000001001: case 0b001000001010: case 0b001000001011:
		{ // Skip if (A1[7:1]-A1[7:1]) makes zero, N->L[2:1]  (typo on description?, should be =?)
			util::stream_format(stream, "%04x: A1=A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 20c
		case 0b001000001100: case 0b001000001101: case 0b001000001110:case 0b001000001111:
		{ // Skip if (A1[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A1, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 210
		case 0b001000010000: case 0b001000010001: case 0b001000010010: case 0b001000010011:
		{ // Skip if (A1[7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: A1·A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 214
		// case 0b001000010100: case 0b001000010101: case 0b001000010110: case 0b001000010111:

		// 218
		case 0b001000011000: case 0b001000011001: case 0b001000011010: case 0b001000011011:
		{ // Skip if (A1[7:1]-A2[7:1]) makes zero, N->L[2:1]  (typo on description?, should be =?)
			util::stream_format(stream, "%04x: A1=A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 21c
		case 0b001000011100: case 0b001000011101: case 0b001000011110: case 0b001000011111:
		{ // Skip if (A1[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A2, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 220
		case 0b001000100000: case 0b001000100001: case 0b001000100010: case 0b001000100011:
		{ // Skip if (A1[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: A1·A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 224
		// case 0b001000100100: case 0b001000100101: case 0b001000100110: case 0b001000100111:

		// 228
		case 0b001000101000: case 0b001000101001: case 0b001000101010:case 0b001000101011:
		{ // Skip if (A1[7:1]-A1[7:1]) makes non zero, N->L[2:1]    (typo on description?, should be =?)
			util::stream_format(stream, "%04x: A1=A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 22c
		case 0b001000101100: case 0b001000101101: case 0b001000101110: case 0b001000101111:
		{ // Skip if (A1[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A1, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 230
		case 0b001000110000: case 0b001000110001: case 0b001000110010: case 0b001000110011:
		{ // Skip if (A1[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: A1·A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 234
		//case 0b001000110100: case 0b001000110101: case 0b001000110110: case 0b001000110111:

		// 238
		case 0b001000111000: case 0b001000111001: case 0b001000111010: case 0b001000111011:
		{ // Skip if (A1[7:1]-A2[7:1]) makes non zero, N->L[2:1]    (typo on description?, should be =?)
			util::stream_format(stream, "%04x: A1=A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 23c
		case 0b001000111100: case 0b001000111101: case 0b001000111110: case 0b001000111111:
		{ // Skip if (A1[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A2, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 240
		case 0b001001000000: case 0b001001000001: case 0b001001000010: case 0b001001000011:
		{ // Skip if (A2[7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: A2·A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 244
		//case 0b001001000100: case 0b001001000101: case 0b001001000110: case 0b001001000111:

		// 248
		case 0b001001001000: case 0b001001001001: case 0b001001001010: case 0b001001001011:
		{ // Skip if (A2[7:1]-A1[7:1]) makes zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "%04x: A2=A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 24c
		case 0b001001001100: case 0b001001001101: case 0b001001001110: case 0b001001001111:
		{ // Skip if (A2[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A1, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 250
		case 0b001001010000: case 0b001001010001: case 0b001001010010: case 0b001001010011:
		{ // Skip if (A2[7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: A2·A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 254
		//case 0b001001010100: case 0b001001010101: case 0b001001010110: case 0b001001010111:

		// 258
		case 0b001001011000: case 0b001001011001: case 0b001001011010: case 0b001001011011:
		{ // Skip if (A2[7:1]-A2[7:1]) makes zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "%04x: A2=A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 25c
		case 0b001001011100: case 0b001001011101: case 0b001001011110: case 0b001001011111:
		{ // Skip if (A2[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A2, N%(d)->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 260
		case 0b001001100000: case 0b001001100001: case 0b001001100010: case 0b001001100011:
		{ // Skip if (A2[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: A2·A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 264
		//case 0b001001100100: case 0b001001100101: case 0b001001100110: case 0b001001100111:

		// 268
		case 0b001001101000: case 0b001001101001: case 0b001001101010: case 0b001001101011:
		{ // Skip if (A2[7:1]-A1[7:1]) makes non zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "%04x: A2=A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 26c
		case 0b001001101100: case 0b001001101101: case 0b001001101110: case 0b001001101111:
		{ // Skip if (A2[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A1, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 270
		case 0b001001110000: case 0b001001110001: case 0b001001110010: case 0b001001110011:
		{ // Skip if (A2[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: A2·A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 274
		//case 0b001001110100: case 0b001001110101: case 0b001001110110: case 0b001001110111:

		// 278
		case 0b001001111000: case 0b001001111001: case 0b001001111010: case 0b001001111011:
		{ // Skip if (A2[7:1]-A2[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: A2=A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 27c
		case 0b001001111100: case 0b001001111101: case 0b001001111110: case 0b001001111111:
		{ // Skip if (A2[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A2, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 280
		case 0b001010000000: case 0b001010000001: case 0b001010000010: case 0b001010000011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: M·A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 284
		//case 0b001010000100: case 0b001010000101: case 0b001010000110: case 0b001010000111:

		// 288
		case 0b001010001000: case 0b001010001001: case 0b001010001010: case 0b001010001011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: M=A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 28c
		case 0b001010001100: case 0b001010001101: case 0b001010001110: case 0b001010001111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: M-A1, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 290
		case 0b001010010000: case 0b001010010001: case 0b001010010010: case 0b001010010011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: M·A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 294
		//case 0b001010010100: case 0b001010010101: case 0b001010010110: case 0b001010010111:

		// 298
		case 0b001010011000: case 0b001010011001: case 0b001010011010: case 0b001010011011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: M=A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 29c
		case 0b001010011100: case 0b001010011101: case 0b001010011110: case 0b001010011111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: M-A2, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2a0
		case 0b001010100000: case 0b001010100001: case 0b001010100010: case 0b001010100011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: M·A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2a4
		//case 0b001010100100: case 0b001010100101: case 0b001010100110: case 0b001010100111:

		// 2a8
		case 0b001010101000: case 0b001010101001: case 0b001010101010: case 0b001010101011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: M=A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2ac
		case 0b001010101100: case 0b001010101101: case 0b001010101110: case 0b001010101111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: M-A1, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2b0
		case 0b001010110000: case 0b001010110001: case 0b001010110010: case 0b001010110011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: M·A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2b4
		//case 0b001010110100: case 0b001010110101: case 0b001010110110: case 0b001010110111:

		// 2b8
		case 0b001010111000: case 0b001010111001: case 0b001010111010: case 0b001010111011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: M=A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2bc
		case 0b001010111100: case 0b001010111101: case 0b001010111110: case 0b001010111111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: M-A2, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2c0
		case 0b001011000000: case 0b001011000001: case 0b001011000010: case 0b001011000011:
		{ // Skip if (H[5:1]·A1[5:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: H·A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2c4
		//case 0b001011000100: case 0b001011000101: case 0b001011000110: case 0b001011000111:

		// 2c8
		case 0b001011001000: case 0b001011001001: case 0b001011001010: case 0b001011001011:
		{ // Skip if (H[5:1]-A1[5:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: H=A1, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2cc
		case 0b001011001100: case 0b001011001101: case 0b001011001110: case 0b001011001111:
		{ // Skip if (H[5:1]-A1[5:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A1, N%(d)->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2d0
		case 0b001011010000: case 0b001011010001: case 0b001011010010: case 0b001011010011:
		{ // Skip if (H[5:1]·A2[5:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "%04x: H·A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2d4
		//case 0b001011010100: case 0b001011010101: case 0b001011010110: case 0b001011010111:

		// 2d8
		case 0b001011011000: case 0b001011011001: case 0b001011011010: case 0b001011011011:
		{ // Skip if (H[5:1]-A2[5:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: H=A2, %d->L EQJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2dc
		case 0b001011011100: case 0b001011011101: case 0b001011011110: case 0b001011011111:
		{ // Skip if (H[5:1]-A2[5:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A2, %d->L BOJ", get_table(pc), inst & 0x3);
			break;
		}

		// 2e0
		case 0b001011100000: case 0b001011100001: case 0b001011100010: case 0b001011100011:
		{ // Skip if (H[5:1]·A1[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: H·A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2e4
		//case 0b001011100100: case 0b001011100101: case 0b001011100110: case 0b001011100111:

		// 2e8
		case 0b001011101000: case 0b001011101001: case 0b001011101010: case 0b001011101011:
		{ // Skip if (H[5:1]-A1[5:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "%04x: H=A1, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2ec
		case 0b001011101100: case 0b001011101101: case 0b001011101110: case 0b001011101111:
		{ // Skip if (H[5:1]-A1[5:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A1, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2f0
		case 0b001011110000: case 0b001011110001: case 0b001011110010: case 0b001011110011:
		{ // Skip if (H[5:1]·A2[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: H·A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2f4
		//case 0b001011110100: case 0b001011110101: case 0b001011110110: case 0b001011110111:

		// 2f8
		case 0b001011111000: case 0b001011111001: case 0b001011111010: case 0b001011111011:
		{ // Skip if (H[5:1]-A2[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "%04x: H=A2, %d->L EQJ/", get_table(pc), inst & 0x3);
			break;
		}

		// 2fc
		case 0b001011111100: case 0b001011111101: case 0b001011111110: case 0b001011111111:
		{ // Skip if (H[5:1]-A2[5:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A2, %d->L BOJ/", get_table(pc), inst & 0x3);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		// 300
		case 0b001100000000: case 0b001100000001: case 0b001100000010: case 0b001100000011:
		{ // N->L[2:1]
			util::stream_format(stream, "%04x: %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 304
		//case 0b001100000100: case 0b001100000101: case 0b001100000110: case 0b001100000111:

		// 308
		case 0b001100001000:
		{ // Move A1[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "%04x: A1->FLS, 0->L", get_table(pc));
			break;
		}

		// 309
		case 0b001100001001:
		{ // Move A1[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "%04x: A1->FRS, 1->L", get_table(pc));
			break;
		}

		// 30a
		case 0b001100001010: case 0b001100001011:
		{ // Move A1[7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "%04x: A1->MODE, 1%d->L", get_table(pc), inst & 0x1);
			break;
		}

		// 30c
		//case 0b001100001100: case 0b001100001101: case 0b001100001110: case 0b001100001111:

		// 310
		case 0b001100010000: case 0b001100010001: case 0b001100010010: case 0b001100010011:
		{ // Move A2[7:1] to A1[7:1], N->L[2:1
			util::stream_format(stream, "%04x: A2->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 314
		//case 0b001100010100: case 0b001100010101: case 0b001100010110: case 0b001100010111:

		// 318
		case 0b001100011000: case 0b001100011001: case 0b001100011010: case 0b001100011011:
		{ // Right shift A1[7:1], 0A1[7], N->L[2:1]
			util::stream_format(stream, "%04x: A1->RS, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 31c
		case 0b001100011100: case 0b001100011101: case 0b001100011110: case 0b001100011111:
		{ // Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A2->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 320
		case 0b001100100000: case 0b001100100001: case 0b001100100010: case 0b001100100011:
		{ // AND A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1·A1->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 324
		case 0b001100100100: case 0b001100100101: case 0b001100100110: case 0b001100100111:
		{ // Add A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1+A1->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 328
		case 0b001100101000: case 0b001100101001: case 0b001100101010: case 0b001100101011:
		{ // OR A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1vA1->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 32c
		case 0b001100101100: case 0b001100101101: case 0b001100101110: case 0b001100101111:
		{ // Subtract A1[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A1->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 330
		case 0b001100110000: case 0b001100110001: case 0b001100110010: case 0b001100110011:
		{ // AND A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1·A2->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 334
		case 0b001100110100: case 0b001100110101: case 0b001100110110: case 0b001100110111:
		{ // Add A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1+A2->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 338
		case 0b001100111000: case 0b001100111001: case 0b001100111010: case 0b001100111011:
		{ // OR A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1vA2->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 33c
		case 0b001100111100: case 0b001100111101: case 0b001100111110: case 0b001100111111:
		{ // Subtract A1[7:1] and A2[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A1-A2->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 340
		case 0b001101000000: case 0b001101000001: case 0b001101000010: case 0b001101000011:
		{ // Move A1[7:1] to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 344
		// case 0b001101000100: case 0b001101000101: case 0b001101000110: case 0b001101000111:

		// 348
		case 0b001101001000:
		{ // Move A2[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "%04x: A2->FLS, 0->L", get_table(pc));
			break;
		}

		// 349
		case 0b001101001001:
		{ // Move A2[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "%04x: A2->FRS, 1->L", get_table(pc));
			break;
		}

		// 34a
		case 0b001101001010: case 0b001101001011:
		{ // Move A2[7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "%04x: A2->MODE, 1%d->L", get_table(pc), inst & 0x1);
			break;
		}

		// 34c
		case 0b001101001100: case 0b001101001101: case 0b001101001110: case 0b001101001111:
		{ // Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A1->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 350
		// case 0b001101010000: case 0b001101010001: case 0b001101010010: case 0b001101010011: case 0b001101010100: case 0b001101010101: case 0b001101010110: case 0b001101010111:

		// 358
		case 0b001101011000: case 0b001101011001: case 0b001101011010: case 0b001101011011:
		{ // Right shift A2[7:1], 0A2[7], N->L[2:1]
			util::stream_format(stream, "%04x: A2->RS, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 35c
		// case 0b001101011100: case 0b001101011101: case 0b001101011110: case 0b001101011111:

		// 360
		case 0b001101100000: case 0b001101100001: case 0b001101100010: case 0b001101100011:
		{ // AND A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2·A1->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 364
		case 0b001101100100: case 0b001101100101: case 0b001101100110: case 0b001101100111:
		{ // Add A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2+A1->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 368
		case 0b001101101000: case 0b001101101001: case 0b001101101010: case 0b001101101011:
		{ // OR A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2vA1->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 36c
		case 0b001101101100: case 0b001101101101: case 0b001101101110: case 0b001101101111:
		{ // Subtract A2[7:1] and A1[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A1->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 370
		case 0b001101110000: case 0b001101110001: case 0b001101110010: case 0b001101110011:
		{ // AND A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2·A2->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 374
		case 0b001101110100: case 0b001101110101: case 0b001101110110: case 0b001101110111:
		{ // Add A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2+A2->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 378
		case 0b001101111000: case 0b001101111001: case 0b001101111010: case 0b001101111011:
		{ // OR A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2vA2->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 37c
		case 0b001101111100: case 0b001101111101: case 0b001101111110: case 0b001101111111:
		{ // Subtract A2[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: A2-A2->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 380
		case 0b001110000000: case 0b001110000001: case 0b001110000010: case 0b001110000011:
		{ // Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 384
		case 0b001110000100: case 0b001110000101: case 0b001110000110: case 0b001110000111:
		{ // Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M<->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 388
		case 0b001110001000:
		{ // Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "%04x: M->FLS, 0->L", get_table(pc));
			break;
		}

		// 389
		case 0b001110001001:
		{ // Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "%04x: M->FRS, 1->L", get_table(pc));
			break;
		}

		// 38a
		case 0b001110001010: case 0b001110001011:
		{ // Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "%04x: M->MODE, 1%d->L", get_table(pc), inst & 0x1);
			break;
		}

		// 38c
		case 0b001110001100: case 0b001110001101: case 0b001110001110: case 0b001110001111:
		{ // Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 390
		case 0b001110010000: case 0b001110010001: case 0b001110010010: case 0b001110010011:
		{ // Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 394
		case 0b001110010100: case 0b001110010101: case 0b001110010110: case 0b001110010111:
		{ // Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M<->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 398
		case 0b001110011000: case 0b001110011001: case 0b001110011010: case 0b001110011011:
		{ // Right shift M[H[5:1],L[2:1]][7:1], 0M[H[5:1],L[2:1]][7], N->L[2:1]
			util::stream_format(stream, "%04x: M->RS, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 39c
		case 0b001110011100: case 0b001110011101: case 0b001110011110: case 0b001110011111:
		{ // Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M<->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3a0
		case 0b001110100000: case 0b001110100001: case 0b001110100010: case 0b001110100011:
		{ // AND M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M·A1->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3a4
		case 0b001110100100: case 0b001110100101: case 0b001110100110: case 0b001110100111:
		{ // Add M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			util::stream_format(stream, "%04x: M+A1->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3a8
		case 0b001110101000: case 0b001110101001: case 0b001110101010: case 0b001110101011:
		{ // OR M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "%04x: MvA1->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3ac
		case 0b001110101100: case 0b001110101101: case 0b001110101110: case 0b001110101111:
		{ // Subtract M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			util::stream_format(stream, "%04x: M-A1->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3b0
		case 0b001110110000: case 0b001110110001: case 0b001110110010: case 0b001110110011:
		{ // AND M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "%04x: M·A2->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3b4
		case 0b001110110100: case 0b001110110101: case 0b001110110110: case 0b001110110111:
		{ // Add M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry

			util::stream_format(stream, "%04x: M+A2->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3b8
		case 0b001110111000: case 0b001110111001: case 0b001110111010: case 0b001110111011:
		{ // OR M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]

			util::stream_format(stream, "%04x: MvA2->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3bc
		case 0b001110111100: case 0b001110111101: case 0b001110111110: case 0b001110111111:
		{ // Subtract M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow

			util::stream_format(stream, "%04x: M-A2->M, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3c0
		case 0b001111000000: case 0b001111000001: case 0b001111000010: case 0b001111000011:
		{ // Move A1[5:1] to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: A1->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		//case 0b001111000100: case 0b001111000101: case 0b001111000110: case 0b001111000111: //case 0b001111001000: case 0b001111001001: case 0b001111001010: case 0b001111001011:

		// 3cc
		case 0b001111001100: case 0b001111001101: case 0b001111001110: case 0b001111001111:
		{ // Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
			util::stream_format(stream, "%04x: H->A1, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3d0
		case 0b001111010000: case 0b001111010001: case 0b001111010010: case 0b001111010011:
		{ // Move A2[5:1] to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: A2->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		//case 0b001111010100: case 0b001111010101: case 0b001111010110: case 0b001111010111: case 0b001111011000: case 0b001111011001: case 0b001111011010: case 0b001111011011:

		// 3dc
		case 0b001111011100: case 0b001111011101: case 0b001111011110: case 0b001111011111:
		{ // Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
			util::stream_format(stream, "%04x: H->A2, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3e0
		case 0b001111100000: case 0b001111100001: case 0b001111100010: case 0b001111100011:
		{ // AND H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: H·A1->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		case 0b001111100100: case 0b001111100101: case 0b001111100110: case 0b001111100111:
		{ // Add H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: H+A1->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		case 0b001111101000: case 0b001111101001: case 0b001111101010: case 0b001111101011:
		{ // OR H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: HvA1->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		case 0b001111101100: case 0b001111101101: case 0b001111101110: case 0b001111101111:
		{ // Subtract H[5:1] and A1[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A1->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3f0
		case 0b001111110000: case 0b001111110001: case 0b001111110010: case 0b001111110011:
		{ // AND H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: H·A2->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3f4
		case 0b001111110100: case 0b001111110101: case 0b001111110110: case 0b001111110111:
		{ // Add H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: H+A2->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3f8
		case 0b001111111000: case 0b001111111001: case 0b001111111010: case 0b001111111011:
		{ // OR H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "%04x: HvA2->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		// 3fc
		case 0b001111111100: case 0b001111111101: case 0b001111111110: case 0b001111111111:
		{ // Subtract H[5:1] and A2[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "%04x: H-A2->H, %d->L", get_table(pc), inst & 0x3);
			break;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		// 400
		case 0b010000000000: case 0b010000000001:
		{ // N->A[11]
			util::stream_format(stream, "%04x: %d->A11", get_table(pc), inst & 0x1);
			break;
		}

		// 402
		case 0b010000000010: case 0b010000000011:
		{ // Jump to (000,M[H[5:1],L[2:1]][5:1],1N),0 L  [2:1], NA[11]
			util::stream_format(stream, "%04x: JPM, 0->L, %d->A11", get_table(pc), inst & 0x1);
			break;
		}

		// case 0b010000000100: case 0b010000000101: case 0b010000000110: case 0b010000000111: case 0b010000001000: case 0b010000001001: case 0b010000001010: case 0b010000001011: case 0b010000001100: case 0b010000001101: case 0b010000001110: case 0b010000001111: case 0b010000010000: case 0b010000010001: case 0b010000010010: case 0b010000010011: case 0b010000010100: case 0b010000010101: case 0b010000010110: case 0b010000010111: case 0b010000011000: case 0b010000011001: case 0b010000011010: case 0b010000011011: case 0b010000011100: case 0b010000011101: case 0b010000011110: case 0b010000011111: case 0b010000100000: case 0b010000100001: case 0b010000100010: case 0b010000100011: case 0b010000100100: case 0b010000100101: case 0b010000100110: case 0b010000100111: case 0b010000101000: case 0b010000101001: case 0b010000101010: case 0b010000101011: case 0b010000101100: case 0b010000101101: case 0b010000101110: case 0b010000101111: case 0b010000110000: case 0b010000110001: case 0b010000110010: case 0b010000110011: case 0b010000110100: case 0b010000110101: case 0b010000110110: case 0b010000110111: case 0b010000111000: case 0b010000111001: case 0b010000111010: case 0b010000111011: case 0b010000111100: case 0b010000111101: case 0b010000111110: case 0b010000111111:

		// 440
		case 0b010001000000: case 0b010001000001: case 0b010001000100: case 0b010001000101: case 0b010001001000: case 0b010001001001: case 0b010001001100: case 0b010001001101: case 0b010001010000: case 0b010001010001: case 0b010001010100: case 0b010001010101: case 0b010001011000: case 0b010001011001: case 0b010001011100: case 0b010001011101: case 0b010001100000: case 0b010001100001: case 0b010001100100: case 0b010001100101: case 0b010001101000: case 0b010001101001: case 0b010001101100: case 0b010001101101: case 0b010001110000: case 0b010001110001: case 0b010001110100: case 0b010001110101: case 0b010001111000: case 0b010001111001: case 0b010001111100: case 0b010001111101:
		{ // Set D to DISP, G to GPE, K to KIE, S to SME, NA[11]
			util::stream_format(stream, "%04x: D(%d)->D, G(%d)->G, K(%d)->K, S(%d)->S, %d->A11", get_table(pc), (inst >> 5) & 0x1, (inst >> 4) & 0x1, (inst >> 3) & 0x1, (inst >> 2) & 0x1, inst & 0x1);
			break;
		}

		// 442
		//case 0b010001000010: case 0b010001000011: case 0b010001000110: case 0b010001000111: case 0b010001001010: case 0b010001001011: case 0b010001001110: case 0b010001001111: case 0b010001010010: case 0b010001010011: case 0b010001010110: case 0b010001010111: case 0b010001011010: case 0b010001011011: case 0b010001011110: case 0b010001011111: case 0b010001100010: case 0b010001100011: case 0b010001100110: case 0b010001100111: case 0b010001101010: case 0b010001101011: case 0b010001101110: case 0b010001101111: case 0b010001110010: case 0b010001110011: case 0b010001110110: case 0b010001110111: case 0b010001111010: case 0b010001111011: case 0b010001111110: case 0b010001111111:

		default:
		{
			util::stream_format(stream, "%04x: %04x <ILLEGAL>", get_table(pc), inst);
			break;
		}
		}
	}
	else if (inst >= 0x480 && inst <= 0x4bf) // 480 - 4bf
	{
		// H[5:1]-K[5:1]->H[5:1], Skip if borrow
		util::stream_format(stream, "%04x: H-K(%02x)->H BOJ", get_table(pc), inst & 0x1f);
	}
	else if (inst >= 0x4c0 && inst <= 0x4ff) // 4c0 - 4ff
	{
		// H[5:1]+K[5:1]H[5:1], Skip if carry
		util::stream_format(stream, "%04x: H+K(%02x)->H CAJ", get_table(pc), inst & 0x1f);
	}
	else if (inst >= 0x500 && inst <= 0x57f) // 500 - 57f
	{
		// When (KIE=0)&(SME=0), Store K[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (KIE=1), Store KIN[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (SME=1), Store HCL[7:1] to M[H[5:1],L[2:1]][7:1]
		util::stream_format(stream, "%04x: K(%02x)->M", get_table(pc), inst & 0x7f);
	}
	else if (inst >= 0x580 && inst <= 0x5ff) // 580 - 5ff
	{
		// Store K[7:6] to L[2:1] and K[5:1] to H[5:1]
		util::stream_format(stream, "%04x: K(%02x)->L,H", get_table(pc), inst & 0x7f);
	}
	else if (inst >= 0x600 && inst <= 0x67f) // 600 - 67f
	{
		// Store K[7:1] to A1[7:1]
		util::stream_format(stream, "%04x: K(%02x)->A1", get_table(pc), inst & 0x7f);
	}
	else if (inst >= 0x680 && inst <= 0x6ff) // 680 - 6ff
	{
		// Store K[7:1] to A2[7:1]
		util::stream_format(stream, "%04x: K(%02x)->A2", get_table(pc), inst & 0x7f);
	}
	else if (inst >= 0x700 && inst <= 0x77f) // 680 - 6ff
	{
		// Store K[7:1] to A3[7:1]
		util::stream_format(stream, "%04x: K(%02x)->A3", get_table(pc), inst & 0x7f);
	}
	else if (inst >= 0x780 && inst <= 0x7ff)  // 780 - 7ff
	{
		// Store K[7:1] to A4[7:1]
		util::stream_format(stream, "%04x: K(%02x)->A4", get_table(pc), inst & 0x7f);
	}
	else if (inst < 0xc00) // 800 - bff
	{
		// Move K[10:1] to A[10:1], Jump to A[11:1]
		util::stream_format(stream, "%04x: JP %03x", get_table(pc), inst & 0x3ff);
	}
	else if (inst < 0x1000) // c00 - fff
	{
		// Move K[10:1] to A[10:1], 0 to A11, Jump to A[11:1], Push next A[11:1] up to ROM address stack
		util::stream_format(stream, "%04x: JS %03x", get_table(pc), inst & 0x3ff);
	}

	return 1;
}
