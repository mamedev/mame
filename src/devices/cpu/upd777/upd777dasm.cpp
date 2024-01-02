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
		const int top1 = (pc & 0x40) >> 6;
		const int top2 = (pc & 0x20) >> 5;
		const int nor = (top1 ^ top2) ^ 1;
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
		const int k = inst & 0x7f;
		util::stream_format(stream, "M-0x%02x", k);
	}
	else if (inst >= 0x100 && inst <= 0x17f)
	{
		// M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		util::stream_format(stream, "M+0x%02x->M, 0x%d->L", k, n);
	}
	else if (inst >= 0x180 && inst <= 0x1ff)
	{
		// M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		util::stream_format(stream, "M-0x%02x->M, 0x%d->L", k, n);
	}
	else if (inst >= 0x480 && inst <= 0x4bf) // 480 - 4bf
	{
		// H[5:1]-K[5:1]->H[5:1], Skip if borrow
		util::stream_format(stream, "H-0x%02x->H BOJ", inst & 0x1f);
	}
	else if (inst >= 0x4c0 && inst <= 0x4ff) // 4c0 - 4ff
	{
		// H[5:1]+K[5:1]->H[5:1], Skip if carry
		const int k = inst & 0x1f;
		util::stream_format(stream, "H+0x%02x->H CAJ", k);
	}
	else if (inst >= 0x500 && inst <= 0x57f) // 500 - 57f
	{
		// When (KIE=0)&(SME=0), Store K[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (KIE=1), Store KIN[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (SME=1), Store HCL[7:1] to M[H[5:1],L[2:1]][7:1]
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->M", k);
	}
	else if (inst >= 0x580 && inst <= 0x5ff) // 580 - 5ff
	{
		// Store K[7:6] to L[2:1] and K[5:1] to H[5:1]
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->L,H", k);
	}
	else if (inst >= 0x600 && inst <= 0x7ff) // 600 - 7ff
	{
		// 600-67f Store K[7:1] to A1[7:1]
		// 680-6ff Store K[7:1] to A2[7:1]
		// 700-77f Store K[7:1] to A3[7:1]
		// 780-7ff Store K[7:1] to A4[7:1]
		const int reg = (inst & 0x180) >> 7;
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->A%d", k, reg+1);
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
		const int k = inst & 0x3ff;
		util::stream_format(stream, "JS 0x%03x (%01x:%02x)", k & 0x3ff, (k & 0x380)>>7, k & 0x07f);
	}
	else
	{
		switch (inst)
		{
		case 0b000000000000:
		{
			// 000 No Operation
			util::stream_format(stream, "NOP");
			break;
		}
		case 0b000000000100:
		{
			// 004 Skip if (Gun Port Latch) = 1
			util::stream_format(stream, "GPL");
			break;
		}
		case 0b000000001000:
		{
			// 008 Move H[5:1] to Line Buffer Register[5:1]
			util::stream_format(stream, "H->NRM");
			break;
		}
		case 0b000000011000:
		{
			// 018 H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
			util::stream_format(stream, "H<->X");
			break;
		}
		case 0b000000100000:
		{
			// 020 Subroutine End, Pop down address stack
			util::stream_format(stream, "SRE");
			break;
		}
		case 0b000000101000: case 0b000000101001:
		{
			// 028 Shift STB[4:1], N->STB[1]
			const int n = inst & 1;
			util::stream_format(stream, "0x%d->STB", n);
			break;
		}
		case 0b000000110000:
		case 0b000000110100:
		case 0b000000111000:
		case 0b000000111100:
		case 0b000001110000:
		case 0b000001110100:
		case 0b000001111000:
		case 0b000001111100:
		{
			// 30 Skip if (PD1 input) = 1
			// 34 Skip if (PD2 input) = 1
			// 38 Skip if (PD3 input) = 1
			// 3c Skip if (PD4 input) = 1
			// 70 Skip if (PD1 input) = 0
			// 74 Skip if (PD2 input) = 0
			// 78 Skip if (PD3 input) = 0
			// 7c Skip if (PD4 input) = 0
			const int which = (inst & 0x00c) >> 2;
			const int inv = inst & 0x40;
			util::stream_format(stream, "PD%d %sJ", which + 1, inv ? "/" : "");
			break;
		}
		case 0b000001001001:
		{
			// 049 Skip if (4H Horizontal Blank) = 1
			util::stream_format(stream, "4H BLK");
			break;
		}
		case 0b000001001010:
		{
			// 04a Skip if (Vertical Blank) = 1, 0->M[[18:00],[3]][1]
			util::stream_format(stream, "VBLK");
			break;
		}
		case 0b000001001100:
		{
			// 04c Skip if (GP&SW/ input) = 1
			util::stream_format(stream, "GPSW/");
			break;
		}
		case 0b000001010100:
		{
			// 054 Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
			util::stream_format(stream, "A->MA");
			break;
		}
		case 0b000001011000:
		{
			// 058 Move M[H[5:1]][28:1] to (A4[7:1],A3[7:1],A2[7:1],A1[7:1])
			util::stream_format(stream, "MA->A");
			break;
		}
		case 0b000001011100:
		{
			// 05c Exchange (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) and M[H[5:1]][28:1]
			util::stream_format(stream, "MA<->A");
			break;
		}
		case 0b000001100000:
		{
			// 060 Subroutine End, Pop down address stack, Skip
			util::stream_format(stream, "SRE+1");
			break;
		}

		// unused < 0x80 cases
		// case 0b000000000001: case 0b000000000010: case 0b000000000011:
		// case 0b000000000101: case 0b000000000110: case 0b000000000111:
		// case 0b000000001001: case 0b000000001010: case 0b000000001011: case 0b000000001100: case 0b000000001101: case 0b000000001110: case 0b000000001111: case 0b000000010000: case 0b000000010001: case 0b000000010010: case 0b000000010011: case 0b000000010100: case 0b000000010101: case 0b000000010110: case 0b000000010111:
		// case 0b000000011001: case 0b000000011010: case 0b000000011011: case 0b000000011100: case 0b000000011101: case 0b000000011110: case 0b000000011111:
		// case 0b000000100001: case 0b000000100010: case 0b000000100011: case 0b000000100100: case 0b000000100101: case 0b000000100110: case 0b000000100111:
		// case 0b000000101010: case 0b000000101011: case 0b000000101100: case 0b000000101101: case 0b000000101110: case 0b000000101111:
		// unused instructions after the PDx J and PDx /J opcodes
		// case 0b000000110001: case 0b000000110010: case 0b000000110011:
		// case 0b000000110101: case 0b000000110110: case 0b000000110111:
		// case 0b000000111001: case 0b000000111010: case 0b000000111011:
		// case 0b000000111101: case 0b000000111110: case 0b000000111111:
		//
		// case 0b000001110001: case 0b000001110010: case 0b000001110011:
		// case 0b000001110101: case 0b000001110110: case 0b000001110111:
		// case 0b000001111001: case 0b000001111010: case 0b000001111011:
		// case 0b000001111101: case 0b000001111110: case 0b000001111111:
		// case 0b000001000000: case 0b000001000001: case 0b000001000010: case 0b000001000011: case 0b000001000100: case 0b000001000101: case 0b000001000110: case 0b000001000111: case 0b000001001000:
		// case 0b000001001011:
		// case 0b000001001101: case 0b000001001110: case 0b000001001111: case 0b000001010000: case 0b000001010001: case 0b000001010010: case 0b000001010011:
		// case 0b000001010101: case 0b000001010110: case 0b000001010111:
		// case 0b000001011001: case 0b000001011010: case 0b000001011011:
		// case 0b000001011101: case 0b000001011110: case 0b000001011111:
		// case 0b000001100001: case 0b000001100010: case 0b000001100011: case 0b000001100100: case 0b000001100101: case 0b000001100110: case 0b000001100111: case 0b000001101000: case 0b000001101001: case 0b000001101010: case 0b000001101011: case 0b000001101100: case 0b000001101101: case 0b000001101110: case 0b000001101111:

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		case 0b001000000000: case 0b001000000001: case 0b001000000010: case 0b001000000011:
		case 0b001000100000: case 0b001000100001: case 0b001000100010: case 0b001000100011:
		case 0b001000001000: case 0b001000001001: case 0b001000001010: case 0b001000001011:
		case 0b001000101000: case 0b001000101001: case 0b001000101010: case 0b001000101011:
		case 0b001000001100: case 0b001000001101: case 0b001000001110: case 0b001000001111:
		case 0b001000101100: case 0b001000101101: case 0b001000101110: case 0b001000101111:
		case 0b001000010000: case 0b001000010001: case 0b001000010010: case 0b001000010011:
		case 0b001000110000: case 0b001000110001: case 0b001000110010: case 0b001000110011:
		case 0b001000011000: case 0b001000011001: case 0b001000011010: case 0b001000011011:
		case 0b001000111000: case 0b001000111001: case 0b001000111010: case 0b001000111011:
		case 0b001000011100: case 0b001000011101: case 0b001000011110: case 0b001000011111:
		case 0b001000111100: case 0b001000111101: case 0b001000111110: case 0b001000111111:
		case 0b001001000000: case 0b001001000001: case 0b001001000010: case 0b001001000011:
		case 0b001001100000: case 0b001001100001: case 0b001001100010: case 0b001001100011:
		case 0b001001001000: case 0b001001001001: case 0b001001001010: case 0b001001001011:
		case 0b001001101000: case 0b001001101001: case 0b001001101010: case 0b001001101011:
		case 0b001001001100: case 0b001001001101: case 0b001001001110: case 0b001001001111:
		case 0b001001101100: case 0b001001101101: case 0b001001101110: case 0b001001101111:
		case 0b001001010000: case 0b001001010001: case 0b001001010010: case 0b001001010011:
		case 0b001001110000: case 0b001001110001: case 0b001001110010: case 0b001001110011:
		case 0b001001011000: case 0b001001011001: case 0b001001011010: case 0b001001011011:
		case 0b001001111000: case 0b001001111001: case 0b001001111010: case 0b001001111011:
		case 0b001001011100: case 0b001001011101: case 0b001001011110: case 0b001001011111:
		case 0b001001111100: case 0b001001111101: case 0b001001111110: case 0b001001111111:
		case 0b001010000000: case 0b001010000001: case 0b001010000010: case 0b001010000011:
		case 0b001010100000: case 0b001010100001: case 0b001010100010: case 0b001010100011:
		case 0b001010001000: case 0b001010001001: case 0b001010001010: case 0b001010001011:
		case 0b001010101000: case 0b001010101001: case 0b001010101010: case 0b001010101011:
		case 0b001010001100: case 0b001010001101: case 0b001010001110: case 0b001010001111:
		case 0b001010101100: case 0b001010101101: case 0b001010101110: case 0b001010101111:
		case 0b001010010000: case 0b001010010001: case 0b001010010010: case 0b001010010011:
		case 0b001010110000: case 0b001010110001: case 0b001010110010: case 0b001010110011:
		case 0b001010011000: case 0b001010011001: case 0b001010011010: case 0b001010011011:
		case 0b001010111000: case 0b001010111001: case 0b001010111010: case 0b001010111011:
		case 0b001010011100: case 0b001010011101: case 0b001010011110: case 0b001010011111:
		case 0b001010111100: case 0b001010111101: case 0b001010111110: case 0b001010111111:
		case 0b001011000000: case 0b001011000001: case 0b001011000010: case 0b001011000011:
		case 0b001011100000: case 0b001011100001: case 0b001011100010: case 0b001011100011:
		case 0b001011001000: case 0b001011001001: case 0b001011001010: case 0b001011001011:
		case 0b001011101000: case 0b001011101001: case 0b001011101010: case 0b001011101011:
		case 0b001011001100: case 0b001011001101: case 0b001011001110: case 0b001011001111:
		case 0b001011101100: case 0b001011101101: case 0b001011101110: case 0b001011101111:
		case 0b001011010000: case 0b001011010001: case 0b001011010010: case 0b001011010011:
		case 0b001011110000: case 0b001011110001: case 0b001011110010: case 0b001011110011:
		case 0b001011011000: case 0b001011011001: case 0b001011011010: case 0b001011011011:
		case 0b001011111000: case 0b001011111001: case 0b001011111010: case 0b001011111011:
		case 0b001011011100: case 0b001011011101: case 0b001011011110: case 0b001011011111:
		case 0b001011111100: case 0b001011111101: case 0b001011111110: case 0b001011111111:
		{
			// optype ·
			// 200 Skip if (A1[7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 220 Skip if (A1[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 210 Skip if (A1[7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 230 Skip if (A1[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 240 Skip if (A2[7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 260 Skip if (A2[7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 250 Skip if (A2[7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 270 Skip if (A2[7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 280 Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes zero, N->L[2:1]
			// 2a0 Skip if (M[H[5:1],L[2:1]][7:1]·A1[7:1]) makes non zero, N->L[2:1]
			// 290 Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes zero, N->L[2:1]
			// 2b0 Skip if (M[H[5:1],L[2:1]][7:1]·A2[7:1]) makes non zero, N->L[2:1]
			// 2c0 Skip if (H[5:1]·A1[5:1]) makes zero, N->L[2:1]
			// 2e0 Skip if (H[5:1]·A1[5:1]) makes non zero, N->L[2:1]
			// 2d0 Skip if (H[5:1]·A2[5:1]) makes zero, N->L[2:1]
			// 2f0 Skip if (H[5:1]·A2[5:1]) makes non zero, N->L[2:1]

			// optype = (these are expressed as x=y in the opcopde syntax, but x-y in the description, in reality it seems to act as 'CMP' so x-y = 0)
			// 208 Skip if (A1[7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 228 Skip if (A1[7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 218 Skip if (A1[7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 238 Skip if (A1[7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 248 Skip if (A2[7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 268 Skip if (A2[7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 258 Skip if (A2[7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 278 Skip if (A2[7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 288 Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes zero, N->L[2:1]
			// 2a8 Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non zero, N->L[2:1]
			// 298 Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes zero, N->L[2:1]
			// 2b8 Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non zero, N->L[2:1]
			// 2c8 Skip if (H[5:1]-A1[5:1]) makes zero, N->L[2:1]
			// 2e8 Skip if (H[5:1]-A1[5:1]) makes non zero, N->L[2:1]
			// 2d8 Skip if (H[5:1]-A2[5:1]) makes zero, N->L[2:1]
			// 2f8 Skip if (H[5:1]-A2[5:1]) makes non zero, N->L[2:1]

			// optype -
			// 20c Skip if (A1[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 22c Skip if (A1[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 21c Skip if (A1[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 23c Skip if (A1[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 24c Skip if (A2[7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 26c Skip if (A2[7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 25c Skip if (A2[7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 27c Skip if (A2[7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 28c Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes borrow, N->L[2:1]
			// 2ac Skip if (M[H[5:1],L[2:1]][7:1]-A1[7:1]) makes non borrow, N->L[2:1]
			// 29c Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes borrow, N->L[2:1]
			// 2bc Skip if (M[H[5:1],L[2:1]][7:1]-A2[7:1]) makes non borrow, N->L[2:1]
			// 2cc Skip if (H[5:1]-A1[5:1]) makes borrow, N->L[2:1]
			// 2ec Skip if (H[5:1]-A1[5:1]) makes non borrow, N->L[2:1]
			// 2dc Skip if (H[5:1]-A2[5:1]) makes borrow, N->L[2:1]
			// 2fc Skip if (H[5:1]-A2[5:1]) makes non borrow, N->L[2:1]
			const int non = inst & 0x20;
			const int optype = (inst & 0x0c) >> 2;
			const int reg1 = (inst & 0xc0) >> 6;
			const int reg2 = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "%s%s%s, 0x%d->L %s%s", m_200_reg1[reg1], m_200_optypes[optype], m_200_reg2[reg2], n, (optype == 3) ? "BOJ" : "EQJ", non ? "/" : "");
			break;
		}

		// unused 2xx - these would be 'invalid' optype
		// 204, 224, 214, 234, 244, 264, 254, 274, 284, 2a4, 294, 2b4, 2c4, 2e4, 2d4, 2f4
		// case 0b001000000100: case 0b001000000101: case 0b001000000110: case 0b001000000111:
		// case 0b001000100100: case 0b001000100101: case 0b001000100110: case 0b001000100111:
		// case 0b001000010100: case 0b001000010101: case 0b001000010110: case 0b001000010111:
		// case 0b001000110100: case 0b001000110101: case 0b001000110110: case 0b001000110111:
		// case 0b001001000100: case 0b001001000101: case 0b001001000110: case 0b001001000111:
		// case 0b001001100100: case 0b001001100101: case 0b001001100110: case 0b001001100111:
		// case 0b001001010100: case 0b001001010101: case 0b001001010110: case 0b001001010111:
		// case 0b001001110100: case 0b001001110101: case 0b001001110110: case 0b001001110111:
		// case 0b001010000100: case 0b001010000101: case 0b001010000110: case 0b001010000111:
		// case 0b001010100100: case 0b001010100101: case 0b001010100110: case 0b001010100111:
		// case 0b001010010100: case 0b001010010101: case 0b001010010110: case 0b001010010111:
		// case 0b001010110100: case 0b001010110101: case 0b001010110110: case 0b001010110111:
		// case 0b001011000100: case 0b001011000101: case 0b001011000110: case 0b001011000111:
		// case 0b001011100100: case 0b001011100101: case 0b001011100110: case 0b001011100111:
		// case 0b001011010100: case 0b001011010101: case 0b001011010110: case 0b001011010111:
		// case 0b001011110100: case 0b001011110101: case 0b001011110110: case 0b001011110111:

		/////////////////////////////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////////////

		case 0b001100000000: case 0b001100000001: case 0b001100000010: case 0b001100000011:
		{
			// 300 N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "0x%d->L", n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001100001000:
		{
			// 308 Move A1[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "A1->FLS, 0->L");
			break;
		}
		case 0b001101001000:
		{
			// 348 Move A2[7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "A2->FLS, 0->L");
			break;
		}
		case 0b001110001000:
		{
			// 388 Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
			util::stream_format(stream, "M->FLS, 0->L");
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001100001001:
		{
			// 309 Move A1[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "A1->FRS, 1->L");
			break;
		}
		case 0b001101001001:
		{
			// 349 Move A2[7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "A2->FRS, 1->L");
			break;
		}
		case 0b001110001001:
		{
			// 389 Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
			util::stream_format(stream, "M->FRS, 1->L");
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001100001010: case 0b001100001011:
		{
			// 30a Move A1[7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			util::stream_format(stream, "A1->MODE, 0x%d->L", n);
			break;
		}
		case 0b001101001010: case 0b001101001011:
		{
			// 34a Move A2[7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			util::stream_format(stream, "A2->MODE, 0x%d->L", n);
			break;
		}
		case 0b001110001010: case 0b001110001011:
		{
			// 38a Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
			const int n = (inst & 0x1) + 2;
			util::stream_format(stream, "M->MODE, 0x%d->L", n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001100010000: case 0b001100010001: case 0b001100010010: case 0b001100010011:
		{
			// 310 Move A2[7:1] to A1[7:1], N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A2->A1, 0x%d->L", n);
			break;
		}
		case 0b001101000000: case 0b001101000001: case 0b001101000010: case 0b001101000011:
		{
			// 340 Move A1[7:1] to A2[7:1], N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A1->A2, 0x%d->L", n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		// 318
		case 0b001100011000: case 0b001100011001: case 0b001100011010: case 0b001100011011:
		{
			// Right shift A1[7:1], 0->A1[7], N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A1->RS, 0x%d->L", n);
			break;
		}
		case 0b001101011000: case 0b001101011001: case 0b001101011010: case 0b001101011011:
		{
			// 358 Right shift A2[7:1], 0->A2[7], N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A2->RS, 0x%d->L", n);
			break;
		}
		case 0b001110011000: case 0b001110011001: case 0b001110011010: case 0b001110011011:
		{
			// 398 Right shift M[H[5:1],L[2:1]][7:1], 0->M[H[5:1],L[2:1]][7], N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "M->RS, 0x%d->L", n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001100011100: case 0b001100011101: case 0b001100011110: case 0b001100011111:
		{
			// 31c Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A1-A2->A2, 0x%d->L", n);
			break;
		}
		case 0b001101001100: case 0b001101001101: case 0b001101001110: case 0b001101001111:
		{
			// 34c Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			const int n = inst & 0x3;
			util::stream_format(stream, "A2-A1->A1, 0x%d->L", n);
			break;
		}

		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////

		case 0b001100100000: case 0b001100100001: case 0b001100100010: case 0b001100100011:
		case 0b001100100100: case 0b001100100101: case 0b001100100110: case 0b001100100111:
		case 0b001100101000: case 0b001100101001: case 0b001100101010: case 0b001100101011:
		case 0b001100101100: case 0b001100101101: case 0b001100101110: case 0b001100101111:
		case 0b001100110000: case 0b001100110001: case 0b001100110010: case 0b001100110011:
		case 0b001100110100: case 0b001100110101: case 0b001100110110: case 0b001100110111:
		case 0b001100111000: case 0b001100111001: case 0b001100111010: case 0b001100111011:
		case 0b001100111100: case 0b001100111101: case 0b001100111110: case 0b001100111111:
		case 0b001101100000: case 0b001101100001: case 0b001101100010: case 0b001101100011:
		case 0b001101100100: case 0b001101100101: case 0b001101100110: case 0b001101100111:
		case 0b001101101000: case 0b001101101001: case 0b001101101010: case 0b001101101011:
		case 0b001101101100: case 0b001101101101: case 0b001101101110: case 0b001101101111:
		case 0b001101110000: case 0b001101110001: case 0b001101110010: case 0b001101110011:
		case 0b001101110100: case 0b001101110101: case 0b001101110110: case 0b001101110111:
		case 0b001101111000: case 0b001101111001: case 0b001101111010: case 0b001101111011:
		case 0b001101111100: case 0b001101111101: case 0b001101111110: case 0b001101111111:
		{
			// 320 AND A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 324 Add A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 328 OR A1[7:1] and A1[7:1], store to A1[7:1], N->L[2:1]
			// 32c Subtract A1[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			// 330 AND A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 334 Add A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 338 OR A1[7:1] and A2[7:1], store to A1[7:1], N->L[2:1]
			// 33c Subtract A1[7:1] and A2[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
			// 360 AND A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 364 Add A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 368 OR A2[7:1] and A1[7:1], store to A2[7:1], N->L[2:1]
			// 36c Subtract A2[7:1] and A1[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			// 370 AND A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 374 Add A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 378 OR A2[7:1] and A2[7:1], store to A2[7:1], N->L[2:1]
			// 37c Subtract A2[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
			const int optype = (inst & 0x0c) >> 2;
			const int reg2 = (inst & 0x10) >> 4;
			const int reg1 = (inst & 0x40) >> 6;
			const int n = inst & 0x3;
			util::stream_format(stream, "%s%s%s->%s, 0x%d->L %s", m_320_reg[reg1], m_320_optypes[optype], m_320_reg[reg2], m_320_reg[reg1], n, (optype == 3) ? "BOJ" : "");
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001110000000: case 0b001110000001: case 0b001110000010: case 0b001110000011:
		case 0b001110010000: case 0b001110010001: case 0b001110010010: case 0b001110010011:
		{
			// 380 Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 390 Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "A%d->M, 0x%d->L", reg + 1, n);
			break;
		}

		case 0b001110000100: case 0b001110000101: case 0b001110000110: case 0b001110000111:
		case 0b001110010100: case 0b001110010101: case 0b001110010110: case 0b001110010111:
		{
			// 384 Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
			// 394 Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "M<->A%d, 0x%d->L", reg + 1, n);
			break;
		}

		case 0b001110001100: case 0b001110001101: case 0b001110001110: case 0b001110001111:
		case 0b001110011100: case 0b001110011101: case 0b001110011110: case 0b001110011111:
		{
			// 38c Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
			// 39c Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "M->A%d, 0x%d->L", reg + 1, n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001110100000: case 0b001110100001: case 0b001110100010: case 0b001110100011:
		case 0b001110100100: case 0b001110100101: case 0b001110100110: case 0b001110100111:
		case 0b001110101000: case 0b001110101001: case 0b001110101010: case 0b001110101011:
		case 0b001110101100: case 0b001110101101: case 0b001110101110: case 0b001110101111:
		case 0b001110110000: case 0b001110110001: case 0b001110110010: case 0b001110110011:
		case 0b001110110100: case 0b001110110101: case 0b001110110110: case 0b001110110111:
		case 0b001110111000: case 0b001110111001: case 0b001110111010: case 0b001110111011:
		case 0b001110111100: case 0b001110111101: case 0b001110111110: case 0b001110111111:
		{
			// 3a0 AND M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3a4 Add M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			// 3a8 OR M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3ac Subtract M[H[5:1],L[2:1]][7:1] and A1[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			// 3b0 AND M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3b4 Add M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if carry
			// 3b8 OR M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1]
			// 3bc Subtract M[H[5:1],L[2:1]][7:1] and A2[7:1], store to M[H[5:1],L[2:1]][7:1], N->L[2:1] Skip if borrow
			const int optype = (inst & 0x0c) >> 2;
			const int reg2 = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "M%s%s->M, 0x%d->L", m_320_optypes[optype], m_320_reg[reg2], n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001111100000: case 0b001111100001: case 0b001111100010: case 0b001111100011:
		case 0b001111100100: case 0b001111100101: case 0b001111100110: case 0b001111100111:
		case 0b001111101000: case 0b001111101001: case 0b001111101010: case 0b001111101011:
		case 0b001111101100: case 0b001111101101: case 0b001111101110: case 0b001111101111:
		case 0b001111110000: case 0b001111110001: case 0b001111110010: case 0b001111110011:
		case 0b001111110100: case 0b001111110101: case 0b001111110110: case 0b001111110111:
		case 0b001111111000: case 0b001111111001: case 0b001111111010: case 0b001111111011:
		case 0b001111111100: case 0b001111111101: case 0b001111111110: case 0b001111111111:
		{
			// 3e0 AND H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3e4 Add H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3e8 OR H[5:1] and A1[5:1], store to H[5:1], N->L[2:1]
			// 3ec Subtract H[5:1] and A1[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			// 3f0 AND H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3f4 Add H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3f8 OR H[5:1] and A2[5:1], store to H[5:1], N->L[2:1]
			// 3fc Subtract H[5:1] and A2[5:1], store to H[5:1], Skip if borrow, N->L[2:1]
			const int optype = (inst & 0x0c) >> 2;
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "H%s%s->H, 0x%d->L", m_320_optypes[optype], m_320_reg[reg], n);
			break;
		}

		//////////////////////////////////////////
		//////////////////////////////////////////

		case 0b001111000000: case 0b001111000001: case 0b001111000010: case 0b001111000011:
		case 0b001111010000: case 0b001111010001: case 0b001111010010: case 0b001111010011:
		{
			// 3c0 Move A1[5:1] to H[5:1], N->L[2:1]
			// 3d0 Move A2[5:1] to H[5:1], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "A%d->H, 0x%d->L", reg + 1, n);
			break;
		}
		case 0b001111001100: case 0b001111001101: case 0b001111001110: case 0b001111001111:
		case 0b001111011100: case 0b001111011101: case 0b001111011110: case 0b001111011111:
		{
			// 3cc Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
			// 3dc Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
			const int reg = (inst & 0x10) >> 4;
			const int n = inst & 0x3;
			util::stream_format(stream, "H->A%d, 0x%d->L", reg + 1, n);
			break;
		}

		// unused 3xx
		// 304
		// case 0b001100000100: case 0b001100000101: case 0b001100000110: case 0b001100000111:
		// 344
		// case 0b001101000100: case 0b001101000101: case 0b001101000110: case 0b001101000111:
		// 30c
		//case 0b001100001100: case 0b001100001101: case 0b001100001110: case 0b001100001111:
		// 314
		//case 0b001100010100: case 0b001100010101: case 0b001100010110: case 0b001100010111:
		// 350
		// case 0b001101010000: case 0b001101010001: case 0b001101010010: case 0b001101010011:
		// case 0b001101010100: case 0b001101010101: case 0b001101010110: case 0b001101010111:
		// 35c
		// case 0b001101011100: case 0b001101011101: case 0b001101011110: case 0b001101011111:
		// case 0b001111000100: case 0b001111000101: case 0b001111000110: case 0b001111000111:
		// case 0b001111001000: case 0b001111001001: case 0b001111001010: case 0b001111001011:
		// case 0b001111010100: case 0b001111010101: case 0b001111010110: case 0b001111010111:
		// case 0b001111011000: case 0b001111011001: case 0b001111011010: case 0b001111011011:

		//////////////////////////////////////////////////////////////////////////////////////////////

		case 0b010000000000: case 0b010000000001:
		{
			// 400 N->A[11]
			const int n = inst & 0x1;
			util::stream_format(stream, "%d->A11", n);
			break;
		}
		case 0b010000000010: case 0b010000000011:
		{
			// 402 Jump to (000,M[H[5:1],L[2:1]][5:1],1N), 0->L[2:1], N->A[11]
			const int n = inst & 0x1;
			util::stream_format(stream, "JPM, 0->L, %d->A11", n);
			break;
		}
		case 0b010001000000: case 0b010001000001: case 0b010001000100: case 0b010001000101: case 0b010001001000: case 0b010001001001: case 0b010001001100: case 0b010001001101: case 0b010001010000: case 0b010001010001: case 0b010001010100: case 0b010001010101: case 0b010001011000: case 0b010001011001: case 0b010001011100: case 0b010001011101: case 0b010001100000: case 0b010001100001: case 0b010001100100: case 0b010001100101: case 0b010001101000: case 0b010001101001: case 0b010001101100: case 0b010001101101: case 0b010001110000: case 0b010001110001: case 0b010001110100: case 0b010001110101: case 0b010001111000: case 0b010001111001: case 0b010001111100: case 0b010001111101:
		{
			// 440 Set D to DISP, G to GPE, K to KIE, S to SME, N->A[11]
			const int d = (inst >> 5) & 0x1;
			const int g = (inst >> 4) & 0x1;
			const int k = (inst >> 3) & 0x1;
			const int s = (inst >> 2) & 0x1;
			const int n = inst & 0x1;
			util::stream_format(stream, "%d->D, %d->G, %d->K, %d->S, %d->A11", d, g, k, s, n);
			break;
		}

		// unused 4xx
		// 404
		// case 0b010000000100: case 0b010000000101: case 0b010000000110: case 0b010000000111: case 0b010000001000: case 0b010000001001: case 0b010000001010: case 0b010000001011: case 0b010000001100: case 0b010000001101: case 0b010000001110: case 0b010000001111: case 0b010000010000: case 0b010000010001: case 0b010000010010: case 0b010000010011: case 0b010000010100: case 0b010000010101: case 0b010000010110: case 0b010000010111: case 0b010000011000: case 0b010000011001: case 0b010000011010: case 0b010000011011: case 0b010000011100: case 0b010000011101: case 0b010000011110: case 0b010000011111: case 0b010000100000: case 0b010000100001: case 0b010000100010: case 0b010000100011: case 0b010000100100: case 0b010000100101: case 0b010000100110: case 0b010000100111: case 0b010000101000: case 0b010000101001: case 0b010000101010: case 0b010000101011: case 0b010000101100: case 0b010000101101: case 0b010000101110: case 0b010000101111: case 0b010000110000: case 0b010000110001: case 0b010000110010: case 0b010000110011: case 0b010000110100: case 0b010000110101: case 0b010000110110: case 0b010000110111: case 0b010000111000: case 0b010000111001: case 0b010000111010: case 0b010000111011: case 0b010000111100: case 0b010000111101: case 0b010000111110: case 0b010000111111:
		// 442
		// case 0b010001000010: case 0b010001000011: case 0b010001000110: case 0b010001000111: case 0b010001001010: case 0b010001001011: case 0b010001001110: case 0b010001001111: case 0b010001010010: case 0b010001010011: case 0b010001010110: case 0b010001010111: case 0b010001011010: case 0b010001011011: case 0b010001011110: case 0b010001011111: case 0b010001100010: case 0b010001100011: case 0b010001100110: case 0b010001100111: case 0b010001101010: case 0b010001101011: case 0b010001101110: case 0b010001101111: case 0b010001110010: case 0b010001110011: case 0b010001110110: case 0b010001110111: case 0b010001111010: case 0b010001111011: case 0b010001111110: case 0b010001111111:

		default:
		{
			util::stream_format(stream, "%04x <ILLEGAL>", inst);
			break;
		}
		}
	}
	return 1;
}
