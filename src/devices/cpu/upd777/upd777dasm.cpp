// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "upd777dasm.h"

upd777_disassembler::upd777_disassembler()
	: util::disasm_interface()
{
	// init lfsr pc lut
	for (u32 i = 0, pc = 0; i < 0x7f; i++)
	{
		m_l2r[i] = pc;
		m_r2l[pc] = i;
		int top1 = (pc & 0x40) >> 6;
		int top2 = (pc & 0x20) >> 5;
		int nor = (top1 ^ top2) ^ 1;
		pc = (pc << 1) | nor;
		pc &= 0x7f;
	}
	m_l2r[0x7f] = 0x7f;
	m_r2l[0x7f] = 0x7f;
}

offs_t upd777_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd777_disassembler::data_buffer &opcodes, const upd777_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);

	if (inst >= 0x080 && inst <= 0x0ff)
	{
		// Skip if (M[H[5:1],L[2:1]][7:1]-K[7:1]) makes borrow
		util::stream_format(stream, "M-0x%02x", inst & 0x7f);
	}
	else if (inst >= 0x100 && inst <= 0x17f)
	{
		// M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]
		util::stream_format(stream, "M+0x%02x->M, 0x%d->L", inst & 0x1f, (inst >> 5) & 0x3);
	}
	else if (inst >= 0x180 && inst <= 0x1ff)
	{
		// M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
		util::stream_format(stream, "M-0x%02x->M, 0x%d->L", inst & 0x1f, (inst >> 5) & 0x3);
	}	
	else if (inst >= 0x480 && inst <= 0x4bf) // 480 - 4bf
	{
		// H[5:1]-K[5:1]->H[5:1], Skip if borrow
		util::stream_format(stream, "H-0x%02x->H BOJ", inst & 0x1f);
	}
	else if (inst >= 0x4c0 && inst <= 0x4ff) // 4c0 - 4ff
	{
		// H[5:1]+K[5:1]H[5:1], Skip if carry
		util::stream_format(stream, "H+0x%02x->H CAJ", inst & 0x1f);
	}
	else if (inst >= 0x500 && inst <= 0x57f) // 500 - 57f
	{
		// When (KIE=0)&(SME=0), Store K[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (KIE=1), Store KIN[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (SME=1), Store HCL[7:1] to M[H[5:1],L[2:1]][7:1]
		util::stream_format(stream, "0x%02x->M", inst & 0x7f);
	}
	else if (inst >= 0x580 && inst <= 0x5ff) // 580 - 5ff
	{
		// Store K[7:6] to L[2:1] and K[5:1] to H[5:1]
		util::stream_format(stream, "0x%02x->L,H", inst & 0x7f);
	}
	else if (inst >= 0x600 && inst <= 0x67f) // 600 - 67f
	{
		// Store K[7:1] to A1[7:1]
		util::stream_format(stream, "0x%02x->A1", inst & 0x7f);
	}
	else if (inst >= 0x680 && inst <= 0x6ff) // 680 - 6ff
	{
		// Store K[7:1] to A2[7:1]
		util::stream_format(stream, "0x%02x->A2", inst & 0x7f);
	}
	else if (inst >= 0x700 && inst <= 0x77f) // 680 - 6ff
	{
		// Store K[7:1] to A3[7:1]
		util::stream_format(stream, "0x%02x->A3", inst & 0x7f);
	}
	else if (inst >= 0x780 && inst <= 0x7ff)  // 780 - 7ff
	{
		// Store K[7:1] to A4[7:1]
		util::stream_format(stream, "0x%02x->A4", inst & 0x7f);
	}
	else if (inst >= 0x800 && inst < 0xc00) // 800 - bff
	{
		// Move K[10:1] to A[10:1], Jump to A[11:1]
		u16 fulladdress = (pc & 0x400) | (inst & 0x3ff);
		util::stream_format(stream, "JP 0x%03x (%01x:%02x)", fulladdress, (fulladdress & 0x780)>>7, inst & 0x07f);
	}
	else if (inst >= 0xc00 && inst < 0x1000) // c00 - fff
	{
		// Move K[10:1] to A[10:1], 0 to A11, Jump to A[11:1], Push next A[11:1] up to ROM address stack
		util::stream_format(stream, "JS 0x%03x (%01x:%02x)", inst & 0x3ff, (inst & 0x380)>>7, inst & 0x07f);
	}
	else
	{
		switch (inst)
		{
		case 0b000000000000:
		{ // No Operation
			util::stream_format(stream, "NOP");
			break;
		}

		// case 0b000000000001: case 0b000000000010: case 0b000000000011:

		case 0b000000000100:
		{ // Skip if (Gun Port Latch) = 1
			util::stream_format(stream, "GPL");
			break;
		}

		//case 0b000000000101: case 0b000000000110: case 0b000000000111:

		case 0b000000001000:
		{ // Move H[5:1] to Line Buffer Register[5:1]
			util::stream_format(stream, "H->NRM");
			break;
		}

		//case 0b000000001001: case 0b000000001010: case 0b000000001011: case 0b000000001100: case 0b000000001101: case 0b000000001110: case 0b000000001111: case 0b000000010000: case 0b000000010001: case 0b000000010010: case 0b000000010011: case 0b000000010100: case 0b000000010101: case 0b000000010110: case 0b000000010111:
		
		case 0b000000011000:
		{ // H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
			util::stream_format(stream, "H<->X");
			break;
		}

		// case 0b000000011001: case 0b000000011010: case 0b000000011011: case 0b000000011100: case 0b000000011101: case 0b000000011110: case 0b000000011111:

		case 0b000000100000:
		{ // Subroutine End, Pop down address stack
			util::stream_format(stream, "SRE");
			break;
		}

		// case 0b000000100001: case 0b000000100010: case 0b000000100011: case 0b000000100100: case 0b000000100101: case 0b000000100110: case 0b000000100111:

		case 0b000000101000: case 0b000000101001:
		{ // Shift STB[4:1], N->STB[1]
			util::stream_format(stream, "0x%d->STB", inst & 1);
			break;
		}

		// case 0b000000101010: case 0b000000101011: case 0b000000101100: case 0b000000101101: case 0b000000101110: case 0b000000101111:

		case 0b000000110000:
		{ // Skip if (PD1 input) = 1
			util::stream_format(stream, "PD1 J");
			break;
		}

		// case 0b000000110001: case 0b000000110010: case 0b000000110011:
		
		case 0b000000110100:
		{ // Skip if (PD2 input) = 1
			util::stream_format(stream, "PD2 J");
			break;
		}

		//case 0b000000110101: case 0b000000110110: case 0b000000110111:
				
		case 0b000000111000:
		{ // Skip if (PD3 input) = 1
			util::stream_format(stream, "PD3 J");
			break;
		}

		//case 0b000000111001: case 0b000000111010: case 0b000000111011:
		
		case 0b000000111100:
		{ // Skip if (PD4 input) = 1
			util::stream_format(stream, "PD4 J");
			break;
		}

		//case 0b000000111101: case 0b000000111110: case 0b000000111111: case 0b000001000000: case 0b000001000001: case 0b000001000010: case 0b000001000011: case 0b000001000100: case 0b000001000101: case 0b000001000110: case 0b000001000111: case 0b000001001000:

		case 0b000001001001:
		{ // Skip if (4H Horizontal Blank) = 1
			util::stream_format(stream, "4H BLK");
			break;
		}

		case 0b000001001010:
		{ // Skip if (Vertical Blank) = 1, 0M[[18:00],[3]][1]

			util::stream_format(stream, "VBLK");
			break;
		}

		//case 0b000001001011:

		case 0b000001001100:
		{ // Skip if (GP&SW/ input) = 1
			util::stream_format(stream, "GPSW/");
			break;
		}

		// case 0b000001001101: case 0b000001001110: case 0b000001001111: case 0b000001010000: case 0b000001010001: case 0b000001010010: case 0b000001010011:

		case 0b000001010100:
		{ // Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
			util::stream_format(stream, "A->MA");
			break;
		}

		// case 0b000001010101: case 0b000001010110: case 0b000001010111:

		case 0b000001011000:
		{ // Move M[H[5:1]][28:1] to (A4[7:1],A3[7:1],A2[7:1],A1[7:1])
			util::stream_format(stream, "MA->A");
			break;
		}

		// case 0b000001011001: case 0b000001011010: case 0b000001011011:

		case 0b000001011100:
		{ // Exchange (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) and M[H[5:1]][28:1]
			util::stream_format(stream, "MA<->A");
			break;
		}

		// case 0b000001011101: case 0b000001011110: case 0b000001011111:

		case 0b000001100000:
		{ // Subroutine End, Pop down address stack, Skip
			util::stream_format(stream, "SRE+1");
			break;
		}

		// case 0b000001100001: case 0b000001100010: case 0b000001100011: case 0b000001100100: case 0b000001100101: case 0b000001100110: case 0b000001100111: case 0b000001101000: case 0b000001101001: case 0b000001101010: case 0b000001101011: case 0b000001101100: case 0b000001101101: case 0b000001101110: case 0b000001101111:

		case 0b000001110000:
		{ // Skip if (PD1 input) = 0
			util::stream_format(stream, "PD1 /J", inst);
			break;
		}

		// case 0b000001110001: case 0b000001110010: case 0b000001110011:

		case 0b000001110100:
		{ // Skip if (PD2 input) = 0
			util::stream_format(stream, "PD2 /J", inst);
			break;
		}

		// case 0b000001110101: case 0b000001110110: case 0b000001110111:

		case 0b000001111000:
		{ // Skip if (PD3 input) = 0
			util::stream_format(stream, "PD3 /J", inst);
			break;
		}

		// case 0b000001111001: case 0b000001111010: case 0b000001111011:

		case 0b000001111100:
		{ // Skip if (PD4 input) = 0
			util::stream_format(stream, "PD4 /J", inst);
			break;
		}

		// case 0b000001111101: case 0b000001111110: case 0b000001111111:


		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		// 200
		case 0b001000000000: case 0b001000000001: case 0b001000000010: case 0b001000000011:
		{ // Skip if (A1[7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "A1·A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 204
		//case 0b001000000100: case 0b001000000101: case 0b001000000110: case 0b001000000111:

		// 208
		case 0b001000001000: case 0b001000001001: case 0b001000001010: case 0b001000001011:
		{ // Skip if (A1[7:1]-A1[7:1]) makes zero, N->L[2:1]  (typo on description?, should be =?)
			util::stream_format(stream, "A1=A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 20c
		case 0b001000001100: case 0b001000001101: case 0b001000001110:case 0b001000001111:
		{ // Skip if (A1[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "A1-A1, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 210
		case 0b001000010000: case 0b001000010001: case 0b001000010010: case 0b001000010011:
		{ // Skip if (A1[7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "A1·A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 214
		// case 0b001000010100: case 0b001000010101: case 0b001000010110: case 0b001000010111:

		// 218
		case 0b001000011000: case 0b001000011001: case 0b001000011010: case 0b001000011011:
		{ // Skip if (A1[7:1]-A2[7:1]) makes zero, N->L[2:1]  (typo on description?, should be =?)
			util::stream_format(stream, "A1=A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 21c
		case 0b001000011100: case 0b001000011101: case 0b001000011110: case 0b001000011111:
		{ // Skip if (A1[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "A1-A2, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 220
		case 0b001000100000: case 0b001000100001: case 0b001000100010: case 0b001000100011:
		{ // Skip if (A1[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "A1·A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 224
		// case 0b001000100100: case 0b001000100101: case 0b001000100110: case 0b001000100111:

		// 228
		case 0b001000101000: case 0b001000101001: case 0b001000101010:case 0b001000101011:
		{ // Skip if (A1[7:1]-A1[7:1]) makes non zero, N->L[2:1]    (typo on description?, should be =?)
			util::stream_format(stream, "A1=A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 22c
		case 0b001000101100: case 0b001000101101: case 0b001000101110: case 0b001000101111:
		{ // Skip if (A1[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "A1-A1, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 230
		case 0b001000110000: case 0b001000110001: case 0b001000110010: case 0b001000110011:
		{ // Skip if (A1[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "A1·A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 234
		//case 0b001000110100: case 0b001000110101: case 0b001000110110: case 0b001000110111:

		// 238
		case 0b001000111000: case 0b001000111001: case 0b001000111010: case 0b001000111011:
		{ // Skip if (A1[7:1]-A2[7:1]) makes non zero, N->L[2:1]    (typo on description?, should be =?)
			util::stream_format(stream, "A1=A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 23c
		case 0b001000111100: case 0b001000111101: case 0b001000111110: case 0b001000111111:
		{ // Skip if (A1[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "A1-A2, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 240
		case 0b001001000000: case 0b001001000001: case 0b001001000010: case 0b001001000011:
		{ // Skip if (A2[7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "A2·A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 244
		//case 0b001001000100: case 0b001001000101: case 0b001001000110: case 0b001001000111:

		// 248
		case 0b001001001000: case 0b001001001001: case 0b001001001010: case 0b001001001011:
		{ // Skip if (A2[7:1]-A1[7:1]) makes zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "A2=A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 24c
		case 0b001001001100: case 0b001001001101: case 0b001001001110: case 0b001001001111:
		{ // Skip if (A2[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "A2-A1, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 250
		case 0b001001010000: case 0b001001010001: case 0b001001010010: case 0b001001010011:
		{ // Skip if (A2[7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "A2·A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 254
		//case 0b001001010100: case 0b001001010101: case 0b001001010110: case 0b001001010111:

		// 258
		case 0b001001011000: case 0b001001011001: case 0b001001011010: case 0b001001011011:
		{ // Skip if (A2[7:1]-A2[7:1]) makes zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "A2=A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 25c
		case 0b001001011100: case 0b001001011101: case 0b001001011110: case 0b001001011111:
		{ // Skip if (A2[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "A2-A2, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 260
		case 0b001001100000: case 0b001001100001: case 0b001001100010: case 0b001001100011:
		{ // Skip if (A2[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "A2·A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 264
		//case 0b001001100100: case 0b001001100101: case 0b001001100110: case 0b001001100111:

		// 268
		case 0b001001101000: case 0b001001101001: case 0b001001101010: case 0b001001101011:
		{ // Skip if (A2[7:1]-A1[7:1]) makes non zero, N->L[2:1]  (typo?)
			util::stream_format(stream, "A2=A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 26c
		case 0b001001101100: case 0b001001101101: case 0b001001101110: case 0b001001101111:
		{ // Skip if (A2[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "A2-A1, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 270
		case 0b001001110000: case 0b001001110001: case 0b001001110010: case 0b001001110011:
		{ // Skip if (A2[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "A2·A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 274
		//case 0b001001110100: case 0b001001110101: case 0b001001110110: case 0b001001110111:

		// 278
		case 0b001001111000: case 0b001001111001: case 0b001001111010: case 0b001001111011:
		{ // Skip if (A2[7:1]-A2[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "A2=A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 27c
		case 0b001001111100: case 0b001001111101: case 0b001001111110: case 0b001001111111:
		{ // Skip if (A2[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "A2-A2, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 280
		case 0b001010000000: case 0b001010000001: case 0b001010000010: case 0b001010000011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "M·A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 284
		//case 0b001010000100: case 0b001010000101: case 0b001010000110: case 0b001010000111:

		// 288
		case 0b001010001000: case 0b001010001001: case 0b001010001010: case 0b001010001011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "M=A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 28c
		case 0b001010001100: case 0b001010001101: case 0b001010001110: case 0b001010001111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "M-A1, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 290
		case 0b001010010000: case 0b001010010001: case 0b001010010010: case 0b001010010011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "M·A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 294
		//case 0b001010010100: case 0b001010010101: case 0b001010010110: case 0b001010010111:

		// 298
		case 0b001010011000: case 0b001010011001: case 0b001010011010: case 0b001010011011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "M=A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 29c
		case 0b001010011100: case 0b001010011101: case 0b001010011110: case 0b001010011111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "M-A2, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 2a0
		case 0b001010100000: case 0b001010100001: case 0b001010100010: case 0b001010100011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "M·A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2a4
		//case 0b001010100100: case 0b001010100101: case 0b001010100110: case 0b001010100111:

		// 2a8
		case 0b001010101000: case 0b001010101001: case 0b001010101010: case 0b001010101011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "M=A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2ac
		case 0b001010101100: case 0b001010101101: case 0b001010101110: case 0b001010101111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "M-A1, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 2b0
		case 0b001010110000: case 0b001010110001: case 0b001010110010: case 0b001010110011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "M·A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2b4
		//case 0b001010110100: case 0b001010110101: case 0b001010110110: case 0b001010110111:

		// 2b8
		case 0b001010111000: case 0b001010111001: case 0b001010111010: case 0b001010111011:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "M=A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2bc
		case 0b001010111100: case 0b001010111101: case 0b001010111110: case 0b001010111111:
		{ // Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "M-A2, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 2c0
		case 0b001011000000: case 0b001011000001: case 0b001011000010: case 0b001011000011:
		{ // Skip if (H[5:1]·A1[5:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "H·A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 2c4
		//case 0b001011000100: case 0b001011000101: case 0b001011000110: case 0b001011000111:

		// 2c8
		case 0b001011001000: case 0b001011001001: case 0b001011001010: case 0b001011001011:
		{ // Skip if (H[5:1]-A1[5:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "H=A1, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 2cc
		case 0b001011001100: case 0b001011001101: case 0b001011001110: case 0b001011001111:
		{ // Skip if (H[5:1]-A1[5:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "H-A1, N%(d)->L BOJ", inst & 0x3);
			break;
		}

		// 2d0
		case 0b001011010000: case 0b001011010001: case 0b001011010010: case 0b001011010011:
		{ // Skip if (H[5:1]·A2[5:1]) makes zero, N->L[2:1]
			util::stream_format(stream, "H·A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 2d4
		//case 0b001011010100: case 0b001011010101: case 0b001011010110: case 0b001011010111:

		// 2d8
		case 0b001011011000: case 0b001011011001: case 0b001011011010: case 0b001011011011:
		{ // Skip if (H[5:1]-A2[5:1]) makes zero, N->L[2:1] (typo?)
			util::stream_format(stream, "H=A2, 0x%d->L EQJ", inst & 0x3);
			break;
		}

		// 2dc
		case 0b001011011100: case 0b001011011101: case 0b001011011110: case 0b001011011111:
		{ // Skip if (H[5:1]-A2[5:1]) makes borrow, N->L[2:1]
			util::stream_format(stream, "H-A2, 0x%d->L BOJ", inst & 0x3);
			break;
		}

		// 2e0
		case 0b001011100000: case 0b001011100001: case 0b001011100010: case 0b001011100011:
		{ // Skip if (H[5:1]·A1[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "H·A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2e4
		//case 0b001011100100: case 0b001011100101: case 0b001011100110: case 0b001011100111:

		// 2e8
		case 0b001011101000: case 0b001011101001: case 0b001011101010: case 0b001011101011:
		{ // Skip if (H[5:1]-A1[5:1]) makes non zero, N->L[2:1] (typo?)
			util::stream_format(stream, "H=A1, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2ec
		case 0b001011101100: case 0b001011101101: case 0b001011101110: case 0b001011101111:
		{ // Skip if (H[5:1]-A1[5:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "H-A1, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		// 2f0
		case 0b001011110000: case 0b001011110001: case 0b001011110010: case 0b001011110011:
		{ // Skip if (H[5:1]·A2[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "H·A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2f4
		//case 0b001011110100: case 0b001011110101: case 0b001011110110: case 0b001011110111:

		// 2f8
		case 0b001011111000: case 0b001011111001: case 0b001011111010: case 0b001011111011:
		{ // Skip if (H[5:1]-A2[5:1]) makes non zero, N->L[2:1]
			util::stream_format(stream, "H=A2, 0x%d->L EQJ/", inst & 0x3);
			break;
		}

		// 2fc
		case 0b001011111100: case 0b001011111101: case 0b001011111110: case 0b001011111111:
		{ // Skip if (H[5:1]-A2[5:1]) makes non borrow, N->L[2:1]
			util::stream_format(stream, "H-A2, 0x%d->L BOJ/", inst & 0x3);
			break;
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		// 300
		case 0b001100000000: case 0b001100000001: case 0b001100000010: case 0b001100000011:
		{ // N->L[2:1]
			util::stream_format(stream, "0x%d->L", inst & 0x3);
			break;
		}

		// 304
		//case 0b001100000100: case 0b001100000101: case 0b001100000110: case 0b001100000111:

		// 308
		case 0b001100001000:
		{ // Move A1[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "A1->FLS, 0->L");
			break;
		}

		// 309
		case 0b001100001001:
		{ // Move A1[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "A1->FRS, 1->L");
			break;
		}

		// 30a
		case 0b001100001010: case 0b001100001011:
		{ // Move A1[7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "A1->MODE, 0x%d->L", (inst & 0x1) + 2);
			break;
		}

		// 30c
		//case 0b001100001100: case 0b001100001101: case 0b001100001110: case 0b001100001111:

		// 310
		case 0b001100010000: case 0b001100010001: case 0b001100010010: case 0b001100010011:
		{ // Move A2[7:1] to A1[7:1], N->L[2:1
			util::stream_format(stream, "A2->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 314
		//case 0b001100010100: case 0b001100010101: case 0b001100010110: case 0b001100010111:

		// 318
		case 0b001100011000: case 0b001100011001: case 0b001100011010: case 0b001100011011:
		{ // Right shift A1[7:1], 0A1[7], N->L[2:1]
			util::stream_format(stream, "A1->RS, 0x%d->L", inst & 0x3);
			break;
		}

		// 31c
		case 0b001100011100: case 0b001100011101: case 0b001100011110: case 0b001100011111:
		{ // Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A1-A2->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 320
		case 0b001100100000: case 0b001100100001: case 0b001100100010: case 0b001100100011:
		{ // AND A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1·A1->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 324
		case 0b001100100100: case 0b001100100101: case 0b001100100110: case 0b001100100111:
		{ // Add A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1+A1->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 328
		case 0b001100101000: case 0b001100101001: case 0b001100101010: case 0b001100101011:
		{ // OR A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1vA1->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 32c
		case 0b001100101100: case 0b001100101101: case 0b001100101110: case 0b001100101111:
		{ // Subtract A1[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A1-A1->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 330
		case 0b001100110000: case 0b001100110001: case 0b001100110010: case 0b001100110011:
		{ // AND A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1·A2->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 334
		case 0b001100110100: case 0b001100110101: case 0b001100110110: case 0b001100110111:
		{ // Add A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1+A2->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 338
		case 0b001100111000: case 0b001100111001: case 0b001100111010: case 0b001100111011:
		{ // OR A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			util::stream_format(stream, "A1vA2->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 33c
		case 0b001100111100: case 0b001100111101: case 0b001100111110: case 0b001100111111:
		{ // Subtract A1[7:1] and A2[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A1-A2->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 340
		case 0b001101000000: case 0b001101000001: case 0b001101000010: case 0b001101000011:
		{ // Move A1[7:1] to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A1->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 344
		// case 0b001101000100: case 0b001101000101: case 0b001101000110: case 0b001101000111:

		// 348
		case 0b001101001000:
		{ // Move A2[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "A2->FLS, 0->L");
			break;
		}

		// 349
		case 0b001101001001:
		{ // Move A2[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "A2->FRS, 1->L");
			break;
		}

		// 34a
		case 0b001101001010: case 0b001101001011:
		{ // Move A2[7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "A2->MODE, 0x%d->L", (inst & 0x1) + 2);
			break;
		}

		// 34c
		case 0b001101001100: case 0b001101001101: case 0b001101001110: case 0b001101001111:
		{ // Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A2-A1->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 350
		// case 0b001101010000: case 0b001101010001: case 0b001101010010: case 0b001101010011: case 0b001101010100: case 0b001101010101: case 0b001101010110: case 0b001101010111:

		// 358
		case 0b001101011000: case 0b001101011001: case 0b001101011010: case 0b001101011011:
		{ // Right shift A2[7:1], 0A2[7], N->L[2:1]
			util::stream_format(stream, "A2->RS, 0x%d->L", inst & 0x3);
			break;
		}

		// 35c
		// case 0b001101011100: case 0b001101011101: case 0b001101011110: case 0b001101011111:

		// 360
		case 0b001101100000: case 0b001101100001: case 0b001101100010: case 0b001101100011:
		{ // AND A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2·A1->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 364
		case 0b001101100100: case 0b001101100101: case 0b001101100110: case 0b001101100111:
		{ // Add A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2+A1->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 368
		case 0b001101101000: case 0b001101101001: case 0b001101101010: case 0b001101101011:
		{ // OR A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2vA1->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 36c
		case 0b001101101100: case 0b001101101101: case 0b001101101110: case 0b001101101111:
		{ // Subtract A2[7:1] and A1[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A2-A1->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 370
		case 0b001101110000: case 0b001101110001: case 0b001101110010: case 0b001101110011:
		{ // AND A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2·A2->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 374
		case 0b001101110100: case 0b001101110101: case 0b001101110110: case 0b001101110111:
		{ // Add A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2+A2->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 378
		case 0b001101111000: case 0b001101111001: case 0b001101111010: case 0b001101111011:
		{ // OR A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			util::stream_format(stream, "A2vA2->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 37c
		case 0b001101111100: case 0b001101111101: case 0b001101111110: case 0b001101111111:
		{ // Subtract A2[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "A2-A2->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 380
		case 0b001110000000: case 0b001110000001: case 0b001110000010: case 0b001110000011:
		{ // Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "A1->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 384
		case 0b001110000100: case 0b001110000101: case 0b001110000110: case 0b001110000111:
		{ // Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
			util::stream_format(stream, "M<->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 388
		case 0b001110001000:
		{ // Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "M->FLS, 0->L");
			break;
		}

		// 389
		case 0b001110001001:
		{ // Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "M->FRS, 1->L");
			break;
		}

		// 38a
		case 0b001110001010: case 0b001110001011:
		{ // Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
			util::stream_format(stream, "M->MODE, 0x%d->L", (inst & 0x1) + 2);
			break;
		}

		// 38c
		case 0b001110001100: case 0b001110001101: case 0b001110001110: case 0b001110001111:
		{ // Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
			util::stream_format(stream, "M->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 390
		case 0b001110010000: case 0b001110010001: case 0b001110010010: case 0b001110010011:
		{ // Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "A2->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 394
		case 0b001110010100: case 0b001110010101: case 0b001110010110: case 0b001110010111:
		{ // Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
			util::stream_format(stream, "M<->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 398
		case 0b001110011000: case 0b001110011001: case 0b001110011010: case 0b001110011011:
		{ // Right shift M[H[5:1],L[2:1]][7:1], 0M[H[5:1],L[2:1]][7], N->L[2:1]
			util::stream_format(stream, "M->RS, 0x%d->L", inst & 0x3);
			break;
		}

		// 39c
		case 0b001110011100: case 0b001110011101: case 0b001110011110: case 0b001110011111:
		{ // Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
			util::stream_format(stream, "M<->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 3a0
		case 0b001110100000: case 0b001110100001: case 0b001110100010: case 0b001110100011:
		{ // AND M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "M·A1->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3a4
		case 0b001110100100: case 0b001110100101: case 0b001110100110: case 0b001110100111:
		{ // Add M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			util::stream_format(stream, "M+A1->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3a8
		case 0b001110101000: case 0b001110101001: case 0b001110101010: case 0b001110101011:
		{ // OR M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "MvA1->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3ac
		case 0b001110101100: case 0b001110101101: case 0b001110101110: case 0b001110101111:
		{ // Subtract M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			util::stream_format(stream, "M-A1->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3b0
		case 0b001110110000: case 0b001110110001: case 0b001110110010: case 0b001110110011:
		{ // AND M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			util::stream_format(stream, "M·A2->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3b4
		case 0b001110110100: case 0b001110110101: case 0b001110110110: case 0b001110110111:
		{ // Add M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry

			util::stream_format(stream, "M+A2->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3b8
		case 0b001110111000: case 0b001110111001: case 0b001110111010: case 0b001110111011:
		{ // OR M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]

			util::stream_format(stream, "MvA2->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3bc
		case 0b001110111100: case 0b001110111101: case 0b001110111110: case 0b001110111111:
		{ // Subtract M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow

			util::stream_format(stream, "M-A2->M, 0x%d->L", inst & 0x3);
			break;
		}

		// 3c0
		case 0b001111000000: case 0b001111000001: case 0b001111000010: case 0b001111000011:
		{ // Move A1[5:1] to H[5:1], N->L[2:1]
			util::stream_format(stream, "A1->H, 0x%d->L", inst & 0x3);
			break;
		}

		//case 0b001111000100: case 0b001111000101: case 0b001111000110: case 0b001111000111: //case 0b001111001000: case 0b001111001001: case 0b001111001010: case 0b001111001011:

		// 3cc
		case 0b001111001100: case 0b001111001101: case 0b001111001110: case 0b001111001111:
		{ // Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
			util::stream_format(stream, "H->A1, 0x%d->L", inst & 0x3);
			break;
		}

		// 3d0
		case 0b001111010000: case 0b001111010001: case 0b001111010010: case 0b001111010011:
		{ // Move A2[5:1] to H[5:1], N->L[2:1]
			util::stream_format(stream, "A2->H, 0x%d->L", inst & 0x3);
			break;
		}

		//case 0b001111010100: case 0b001111010101: case 0b001111010110: case 0b001111010111: case 0b001111011000: case 0b001111011001: case 0b001111011010: case 0b001111011011:

		// 3dc
		case 0b001111011100: case 0b001111011101: case 0b001111011110: case 0b001111011111:
		{ // Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
			util::stream_format(stream, "H->A2, 0x%d->L", inst & 0x3);
			break;
		}

		// 3e0
		case 0b001111100000: case 0b001111100001: case 0b001111100010: case 0b001111100011:
		{ // AND H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "H·A1->H, 0x%d->L", inst & 0x3);
			break;
		}

		case 0b001111100100: case 0b001111100101: case 0b001111100110: case 0b001111100111:
		{ // Add H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "H+A1->H, 0x%d->L", inst & 0x3);
			break;
		}

		case 0b001111101000: case 0b001111101001: case 0b001111101010: case 0b001111101011:
		{ // OR H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "HvA1->H, 0x%d->L", inst & 0x3);
			break;
		}

		case 0b001111101100: case 0b001111101101: case 0b001111101110: case 0b001111101111:
		{ // Subtract H[5:1] and A1[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "H-A1->H, 0x%d->L", inst & 0x3);
			break;
		}

		// 3f0
		case 0b001111110000: case 0b001111110001: case 0b001111110010: case 0b001111110011:
		{ // AND H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "H·A2->H, 0x%d->L", inst & 0x3);
			break;
		}

		// 3f4
		case 0b001111110100: case 0b001111110101: case 0b001111110110: case 0b001111110111:
		{ // Add H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "H+A2->H, 0x%d->L", inst & 0x3);
			break;
		}

		// 3f8
		case 0b001111111000: case 0b001111111001: case 0b001111111010: case 0b001111111011:
		{ // OR H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			util::stream_format(stream, "HvA2->H, 0x%d->L", inst & 0x3);
			break;
		}

		// 3fc
		case 0b001111111100: case 0b001111111101: case 0b001111111110: case 0b001111111111:
		{ // Subtract H[5:1] and A2[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			util::stream_format(stream, "H-A2->H, 0x%d->L", inst & 0x3);
			break;
		}

		//////////////////////////////////////////////////////////////////////////////////////////////

		// 400
		case 0b010000000000: case 0b010000000001:
		{ // N->A[11]
			util::stream_format(stream, "%d->A11", inst & 0x1);
			break;
		}

		// 402
		case 0b010000000010: case 0b010000000011:
		{ // Jump to (000,M[H[5:1],L[2:1]][5:1],1N),0 L  [2:1], NA[11]
			util::stream_format(stream, "JPM, 0->L, %d->A11", inst & 0x1);
			break;
		}

		// case 0b010000000100: case 0b010000000101: case 0b010000000110: case 0b010000000111: case 0b010000001000: case 0b010000001001: case 0b010000001010: case 0b010000001011: case 0b010000001100: case 0b010000001101: case 0b010000001110: case 0b010000001111: case 0b010000010000: case 0b010000010001: case 0b010000010010: case 0b010000010011: case 0b010000010100: case 0b010000010101: case 0b010000010110: case 0b010000010111: case 0b010000011000: case 0b010000011001: case 0b010000011010: case 0b010000011011: case 0b010000011100: case 0b010000011101: case 0b010000011110: case 0b010000011111: case 0b010000100000: case 0b010000100001: case 0b010000100010: case 0b010000100011: case 0b010000100100: case 0b010000100101: case 0b010000100110: case 0b010000100111: case 0b010000101000: case 0b010000101001: case 0b010000101010: case 0b010000101011: case 0b010000101100: case 0b010000101101: case 0b010000101110: case 0b010000101111: case 0b010000110000: case 0b010000110001: case 0b010000110010: case 0b010000110011: case 0b010000110100: case 0b010000110101: case 0b010000110110: case 0b010000110111: case 0b010000111000: case 0b010000111001: case 0b010000111010: case 0b010000111011: case 0b010000111100: case 0b010000111101: case 0b010000111110: case 0b010000111111:

		// 440
		case 0b010001000000: case 0b010001000001: case 0b010001000100: case 0b010001000101: case 0b010001001000: case 0b010001001001: case 0b010001001100: case 0b010001001101: case 0b010001010000: case 0b010001010001: case 0b010001010100: case 0b010001010101: case 0b010001011000: case 0b010001011001: case 0b010001011100: case 0b010001011101: case 0b010001100000: case 0b010001100001: case 0b010001100100: case 0b010001100101: case 0b010001101000: case 0b010001101001: case 0b010001101100: case 0b010001101101: case 0b010001110000: case 0b010001110001: case 0b010001110100: case 0b010001110101: case 0b010001111000: case 0b010001111001: case 0b010001111100: case 0b010001111101:
		{ // Set D to DISP, G to GPE, K to KIE, S to SME, NA[11]
			util::stream_format(stream, "%d->D, %d->G, %d->K, %d->S, %d->A11", (inst >> 5) & 0x1, (inst >> 4) & 0x1, (inst >> 3) & 0x1, (inst >> 2) & 0x1, inst & 0x1);
			break;
		}

		// 442
		//case 0b010001000010: case 0b010001000011: case 0b010001000110: case 0b010001000111: case 0b010001001010: case 0b010001001011: case 0b010001001110: case 0b010001001111: case 0b010001010010: case 0b010001010011: case 0b010001010110: case 0b010001010111: case 0b010001011010: case 0b010001011011: case 0b010001011110: case 0b010001011111: case 0b010001100010: case 0b010001100011: case 0b010001100110: case 0b010001100111: case 0b010001101010: case 0b010001101011: case 0b010001101110: case 0b010001101111: case 0b010001110010: case 0b010001110011: case 0b010001110110: case 0b010001110111: case 0b010001111010: case 0b010001111011: case 0b010001111110: case 0b010001111111:

		default:
		{
			util::stream_format(stream, "%04x <ILLEGAL>", inst);
			break;
		}
		}
	}
	return 1;
}
