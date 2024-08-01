// license:BSD-3-Clause
// copyright-holders:Vas Crabb

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


std::string upd777_disassembler::get_300optype_name(int optype)
{
	switch (optype)
	{
	case 0x00: return "."; // 'AND' expressed as '·' in documentation, but disassembler isn't keen on that
	case 0x01: return "+";
	case 0x02: return "v"; // 'OR'
	case 0x03: return "-";
	}
	return "<invalid optype>";
}

std::string upd777_disassembler::get_200optype_name(int optype)
{
	switch (optype)
	{
	case 0x00: return "."; // 'AND' expressed as '·' in documentation, but disassembler isn't keen on that
	case 0x01: return "<invalid optype>";
	case 0x02: return "=";
	case 0x03: return "-";
	}
	return "<invalid optype>";
}

std::string upd777_disassembler::get_reg_name(int reg)
{
	switch (reg)
	{
	case 0x00: return "A1"; // general reg A1
	case 0x01: return "A2"; // general reg A2
	case 0x02: return "M"; // content of memory
	case 0x03: return "H"; // high address
	}
	return "<invalid reg>";
}

offs_t upd777_disassembler::disassemble(std::ostream &stream, offs_t pc, const upd777_disassembler::data_buffer &opcodes, const upd777_disassembler::data_buffer &params)
{
	u16 inst = opcodes.r16(pc);

	if (inst >= 0b0000'1000'0000 && inst <= 0b0000'1111'1111)
	{
		// 080 - 0ff Skip if (M[H[5:1],L[2:1]][7:1]-K[7:1]) makes borrow
		const int k = inst & 0x7f;
		util::stream_format(stream, "M-0x%02x", k);
	}
	else if (inst >= 0b0001'0000'0000 && inst <= 0b0001'0111'1111)
	{
		// 100-17f M[H[5:1],L[2:1]][7:1]+K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if carry, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		util::stream_format(stream, "M+0x%02x->M, 0x%d->L", k, n);
	}
	else if (inst >= 0b0001'1000'0000 && inst <= 0b0001'1111'1111)
	{
		// 180-1ff M[H[5:1],L[2:1]][7:1]-K[7:1]->M[H[5:1],L[2:1]][7:1], Skip if borrow, N->L[2:1]
		const int k = inst & 0x1f;
		const int n = (inst >> 5) & 0x3;
		util::stream_format(stream, "M-0x%02x->M, 0x%d->L", k, n);
	}
	else if (inst >= 0b0100'1000'0000 && inst <= 0b0100'1011'1111)
	{
		// 480-4bf H[5:1]-K[5:1]->H[5:1], Skip if borrow
		const int k = inst & 0x1f;
		util::stream_format(stream, "H-0x%02x->H BOJ", k);
	}
	else if (inst >= 0b0100'1100'0000 && inst <= 0b0100'1111'1111)
	{
		// 4c0 - 4ff H[5:1]+K[5:1]->H[5:1], Skip if carry
		const int k = inst & 0x1f;
		util::stream_format(stream, "H+0x%02x->H CAJ", k);
	}
	else if (inst >= 0b0101'0000'0000 && inst <= 0b0101'0111'1111)
	{
		// 500 - 57f
		// When (KIE=0)&(SME=0), Store K[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (KIE=1), Store KIN[7:1] to M[H[5:1],L[2:1]][7:1]
		// When (SME=1), Store HCL[7:1] to M[H[5:1],L[2:1]][7:1]
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->M", k);
	}
	else if (inst >= 0b0101'1000'0000 && inst <= 0b0101'1111'1111)
	{
		// 580 - 5ff Store K[7:6] to L[2:1] and K[5:1] to H[5:1]
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->L,H", k);
	}
	else if (inst >= 0b0110'0000'0000 && inst <= 0b0111'1111'1111)
	{
		// 600-67f Store K[7:1] to A1[7:1]
		// 680-6ff Store K[7:1] to A2[7:1]
		// 700-77f Store K[7:1] to A3[7:1]
		// 780-7ff Store K[7:1] to A4[7:1]
		const int reg = (inst & 0x180) >> 7;
		const int k = inst & 0x7f;
		util::stream_format(stream, "0x%02x->A%d", k, reg+1);
	}
	else if (inst >= 0b1000'0000'0000 && inst <= 0b1011'1111'1111)
	{
		// 800 - bff Move K[10:1] to A[10:1], Jump to A[11:1]
		u16 fulladdress = (pc & 0x400) | (inst & 0x3ff);
		util::stream_format(stream, "JP 0x%03x (%01x:%02x)", fulladdress, (fulladdress & 0x780)>>7, inst & 0x07f);
	}
	else if (inst >= 0b1100'0000'0000 && inst <= 0b1111'1111'1111)
	{
		// c00 - fff Move K[10:1] to A[10:1], 0 to A11, Jump to A[11:1], Push next A[11:1] up to ROM address stack
		const int k = inst & 0x3ff;
		util::stream_format(stream, "JS 0x%03x (%01x:%02x)", k & 0x3ff, (k & 0x380)>>7, k & 0x07f);
	}
	else if (((inst & 0b1111'0000'0000) == 0b0010'0000'0000) && ((inst & 0b0000'0000'1100) != 0b0000'0000'0100))
	{
		// 0b0010'rrnR'oonn where rr = reg1 (A1, A2, M or H), n = invert condition, R = reg2 (A1 or A2) and oo = optype (only 0,2,3 are valid, no cases here for 1) nn = next l value
		// 
		// optype · (AND)
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
		util::stream_format(stream, "%s%s%s, 0x%d->L %s%s", get_reg_name(reg1), get_200optype_name(optype), get_reg_name(reg2), n, (optype == 3) ? "BOJ" : "EQJ", non ? "/" : "");
	}
	else if ((inst & 0b1111'1010'0000) == 0b0011'0010'0000)
	{
		//   0b0011'0r1R'oonn (where r = reg1, R = reg2, o = optype, and n = next l value)
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
		util::stream_format(stream, "%s%s%s->%s, 0x%d->L %s", get_reg_name(reg1), get_300optype_name(optype), get_reg_name(reg2), get_reg_name(reg1), n, (optype == 3) ? "BOJ" : "");
	}
	else if ((inst & 0b1111'1110'0000) == 0b0011'1010'0000)
	{
		//   0b0011'101r'oonn (where r = reg, o = optype, n = next l value)
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
		util::stream_format(stream, "M%s%s->M, 0x%d->L", get_300optype_name(optype), get_reg_name(reg2), n);
	}
	else if ((inst & 0b1111'1110'0000) == 0b0011'1110'0000)
	{
		//   0b0011'111r'oonn (where r = reg, o = optype, n = next l value)
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
		util::stream_format(stream, "H%s%s->H, 0x%d->L", get_300optype_name(optype), get_reg_name(reg), n);
	}
	else if ((inst & 0b1111'1100'0010) == 0b0100'0100'0000)
	{
		//   0b0100'01dg'ks0n (where  d = DISP, G = GPE, K = KIE, S = SME, n = A11)
		// 440 Set D to DISP, G to GPE, K to KIE, S to SME, N->A[11]
		const int d = (inst >> 5) & 0x1;
		const int g = (inst >> 4) & 0x1;
		const int k = (inst >> 3) & 0x1;
		const int s = (inst >> 2) & 0x1;
		const int n = inst & 0x1;
		util::stream_format(stream, "%d->D, %d->G, %d->K, %d->S, %d->A11", d, g, k, s, n);
	}
	else if (inst == 0b0000'0000'0000)
	{
		// 000 No Operation
		util::stream_format(stream, "NOP");
	}
	else if (inst == 0b0000'0000'0100)
	{
		// 004 Skip if (Gun Port Latch) = 1
		util::stream_format(stream, "GPL");
	}
	else if (inst == 0b0000'0000'1000)
	{
		// 008 Move H[5:1] to Line Buffer Register[5:1]
		util::stream_format(stream, "H->NRM");
	}
	else if (inst == 0b0000'0001'1000)
	{
		// 018 H[5:1]<->X4[5:1], 0->X4[7:6], 0->X3[7:1], 0->X1'[1], 0->A1'[1], L[2:1]<->L'[2:1]
		util::stream_format(stream, "H<->X");
	}
	else if (inst == 0b0000'0010'0000)
	{
		// 020 Subroutine End, Pop down address stack
		util::stream_format(stream, "SRE");
	}
	else if (inst == 0b0000'0100'1001)
	{
		// 049 Skip if (4H Horizontal Blank) = 1
		util::stream_format(stream, "4H BLK");
	}
	else if (inst == 0b0000'0100'1010)
	{
		// 04a Skip if (Vertical Blank) = 1, 0->M[[18:00],[3]][1]
		util::stream_format(stream, "VBLK");
	}
	else if (inst == 0b0000'0100'1100)
	{
		// 04c Skip if (GP&SW/ input) = 1
		util::stream_format(stream, "GPSW/");
	}
	else if (inst == 0b0000'0101'0100)
	{
		// 054 Move (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) to M[H[5:1]][28:1]
		util::stream_format(stream, "A->MA");
	}
	else if (inst == 0b0000'0101'1000)
	{
		// 058 Move M[H[5:1]][28:1] to (A4[7:1],A3[7:1],A2[7:1],A1[7:1])
		util::stream_format(stream, "MA->A");
	}
	else if (inst == 0b0000'0101'1100)
	{
		// 05c Exchange (A4[7:1],A3[7:1],A2[7:1],A1[7:1]) and M[H[5:1]][28:1]
		util::stream_format(stream, "MA<->A");
	}
	else if (inst == 0b0000'0110'0000)
	{
		// 060 Subroutine End, Pop down address stack, Skip
		util::stream_format(stream, "SRE+1");
	}
	else if (inst == 0b0011'0000'1000)
	{
		// 308 Move A1[7:1] to FLS[7:1], 0->L[2:1]
		util::stream_format(stream, "A1->FLS, 0->L");
	}
	else if (inst == 0b0011'0100'1000)
	{
		// 348 Move A2[7:1] to FLS[7:1], 0->L[2:1]
		util::stream_format(stream, "A2->FLS, 0->L");
	}
	else if (inst == 0b0011'1000'1000)
	{
		// 388 Move M[H[5:1],L[2:1]][7:1] to FLS[7:1], 0->L[2:1]
		util::stream_format(stream, "M->FLS, 0->L");
	}
	else if (inst == 0b0011'0000'1001)
	{
		// 309 Move A1[7:1] to FRS[7:1], 1->L[2:1]
		util::stream_format(stream, "A1->FRS, 1->L");
	}
	else if (inst == 0b0011'0100'1001)
	{
		// 349 Move A2[7:1] to FRS[7:1], 1->L[2:1]
		util::stream_format(stream, "A2->FRS, 1->L");
	}
	else if (inst == 0b0011'1000'1001)
	{
		// 389 Move M[H[5:1],L[2:1]][7:1] to FRS[7:1], 1->L[2:1]
		util::stream_format(stream, "M->FRS, 1->L");
	}
	else if ((inst == 0b0000'0010'1000) || (inst == 0b0000'0010'1001))
	{
		// 028 Shift STB[4:1], N->STB[1]
		// STB is an input strobe / shifter
		const int n = inst & 1;
		util::stream_format(stream, "0x%d->STB", n);
	}
	else if ((inst == 0b0011'0000'1010) || (inst == 0b0011'0000'1011))
	{
		// 30a Move A1[7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		util::stream_format(stream, "A1->MODE, 0x%d->L", n);
	}
	else if ((inst == 0b0011'01001010) || (inst == 0b0011'0100'1011))
	{
		// 34a Move A2[7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		util::stream_format(stream, "A2->MODE, 0x%d->L", n);
	}
	else if ((inst == 0b0011'1000'1010) || (inst == 0b00111000'1011))
	{
		// 38a Move M[H[5:1],L[2:1]][7:1] to MODE[7:1], 1N->L[2:1]
		const int n = (inst & 0x1) + 2;
		util::stream_format(stream, "M->MODE, 0x%d->L", n);
	}

	else if ((inst == 0b0100'0000'0000) || (inst == 0b0100'0000'0001))
	{
		// 400 N->A[11]
		const int n = inst & 0x1;
		util::stream_format(stream, "%d->A11", n);
	}
	else if ((inst == 0b0100'0000'0010) || (inst == 0b0100'0000'0011))
	{
		// 402 Jump to (000,M[H[5:1],L[2:1]][5:1],1N), 0->L[2:1], N->A[11]
		const int n = inst & 0x1;
		util::stream_format(stream, "JPM, 0->L, %d->A11", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0000'0000)
	{
		// 300 N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'0000)
	{
		// 310 Move A2[7:1] to A1[7:1], N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A2->A1, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0100'0000)
	{
		// 340 Move A1[7:1] to A2[7:1], N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A1->A2, 0x%d->L", n);
	}

	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'1000)
	{
		// 318 Right shift A1[7:1], 0->A1[7], N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A1->RS, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0101'1000)
	{
		// 358 Right shift A2[7:1], 0->A2[7], N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A2->RS, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'1001'1000)
	{
		// 398 Right shift M[H[5:1],L[2:1]][7:1], 0->M[H[5:1],L[2:1]][7], N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "M->RS, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0001'1100)
	{
		// 31c Subtract A1[7:1] and A2[7:1], store to A2[7:1], Skip if borrow, N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A1-A2->A2, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1111'1100) == 0b0011'0100'1100)
	{
		// 34c Subtract A2[7:1] and A1[7:1], store to A1[7:1], Skip if borrow, N->L[2:1]
		const int n = inst & 0x3;
		util::stream_format(stream, "A2-A1->A1, 0x%d->L", n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'0000)
	{
		// 380 Move A1[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
		// 390 Move A2[7:1] to M[H[5:1],L[2:1]][7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		util::stream_format(stream, "A%d->M, 0x%d->L", reg + 1, n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'0100)
	{
		// 384 Exchange M[H[5:1],L[2:1]][7:1] and A1[7:1], N->L[2:1]
		// 394 Exchange M[H[5:1],L[2:1]][7:1] and A2[7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		util::stream_format(stream, "M<->A%d, 0x%d->L", reg + 1, n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1000'1100)
	{
		// 38c Move M[H[5:1],L[2:1]][7:1] to A1[7:1], N->L[2:1]
		// 39c Move M[H[5:1],L[2:1]][7:1] to A2[7:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		util::stream_format(stream, "M->A%d, 0x%d->L", reg + 1, n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1100'0000)
	{
		// 3c0 Move A1[5:1] to H[5:1], N->L[2:1]
		// 3d0 Move A2[5:1] to H[5:1], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		util::stream_format(stream, "A%d->H, 0x%d->L", reg + 1, n);
	}
	else if ((inst & 0b1111'1110'1100) == 0b0011'1100'1100)
	{
		// 3cc Move H[5:1] to A1[5:1], 0->A1[7:6], N->L[2:1]
		// 3dc Move H[5:1] to A2[5:1], 0->A2[7:6], N->L[2:1]
		const int reg = (inst & 0x10) >> 4;
		const int n = inst & 0x3;
		util::stream_format(stream, "H->A%d, 0x%d->L", reg + 1, n);
	}
	else if ((inst & 0b1111'1011'0011) == 0b0000'0011'0000)
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
	}
	else
	{
		util::stream_format(stream, "%04x <ILLEGAL>", inst);
	}
	
	return 1;
}
