// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
#include "inst.h"
#include "emu.h"

namespace DSP56K
{
// Factory
Instruction* Instruction::decodeInstruction(const Opcode* opc,
											const UINT16 word0,
											const UINT16 word1,
											bool shifted)
{
	UINT16 w0 = word0;
	UINT16 w1 = word1;

	if (shifted)
	{
		w0 = w1;
		w1 = 0x0000;
	}

	/**************************************************************************/
	/* The very funky case of the XMemoryDataMoveWithShortDisplacement        */
	/**************************************************************************/
	if ((w0 & 0xff00) == 0x0500)
	{
		// Avoid "05-- 05--" recursion
		if (shifted) return NULL;

		auto_pointer<Instruction> op(decodeInstruction(opc, w0, w1, true));
		if (op)
		{
			// This parallel move only works for certain trailing instructions.
			if (dynamic_cast<Add*>(op.get())  ||
				dynamic_cast<Asr*>(op.get())  ||
				dynamic_cast<Eor*>(op.get())  ||
				dynamic_cast<Mac*>(op.get())  ||
				dynamic_cast<Macr*>(op.get()) ||
				dynamic_cast<Mpy*>(op.get())  ||
				dynamic_cast<Neg*>(op.get())  ||
				dynamic_cast<Sub*>(op.get())  ||
				dynamic_cast<Tfr*>(op.get())  ||
				dynamic_cast<Tst*>(op.get())
				/* TODO: More? */)
			{
				op->m_sizeIncrement = 1;
				return op;
			}
		}
	}


	/**************************************************************************/
	/* Dual X Memory Data Read : 011m mKKK .rr. .... : A-142 */
	/* Quote: (MOVE, MAC(R), MPY(R), ADD, SUB, TFR) */
	/**************************************************************************/
	/* TFR : 011m mKKK 0rr1 F0DD : A-212 */
	if ((w0 & 0xe094) == 0x6010)
	{
		return global_alloc(Tfr_2(opc, w0, w1));
	}
	/* MOVE : 011m mKKK 0rr1 0000 : A-128 */
	if ((w0 & 0xe097) == 0x6017)
	{
		return global_alloc(Move_2(opc, w0, w1));
	}
	/* MAC : 011m mKKK 1xx0 F1QQ : A-122 */
	else if ((w0 & 0xe094) == 0x6084)
	{
		return global_alloc(Mac_3(opc, w0, w1));
	}
	/* MACR: 011m mKKK 1--1 F1QQ : A-124 */
	else if ((w0 & 0xe094) == 0x6094)
	{
		return global_alloc(Macr_2(opc, w0, w1));
	}
	/* MPY : 011m mKKK 1xx0 F0QQ : A-160 */
	else if ((w0 & 0xe094) == 0x6080)
	{
		return global_alloc(Mpy_2(opc, w0, w1));
	}
	/* MPYR : 011m mKKK 1--1 F0QQ : A-162 */
	else if ((w0 & 0xe094) == 0x6090)
	{
		return global_alloc(Mpyr_2(opc, w0, w1));
	}
	/* ADD : 011m mKKK 0rru Fuuu : A-22 */
	/* SUB : 011m mKKK 0rru Fuuu : A-202 */
	else if ((w0 & 0xe080) == 0x6000)
	{
		return global_alloc(Add_2(opc, w0, w1));
	}

	/****************************************************************************/
	/* X Memory Data Write and Register Data Move : 0001 011k RRDD .... : A-140 */
	/* Quote: (MPY or MAC) */
	/****************************************************************************/
	/* MPY : 0001 0110 RRDD FQQQ : A-160 */
	else if ((w0 & 0xff00) == 0x1600)
	{
		return global_alloc(Mpy_3(opc, w0, w1));
	}
	/* MAC : 0001 0111 RRDD FQQQ : A-122 */
	else if ((w0 & 0xff00) == 0x1700)
	{
		return global_alloc(Mac_3(opc, w0, w1));
	}

	/****************************************************************/
	/* No Parallel Data Move :   0100 1010 .... .... : A-131 */
	/* Register to Register Data Move : 0100 IIII .... .... : A-133 */
	/* Address Register Update : 0011 0zRR .... .... : A-135 */
	/* X Memory Data Move :   1mRR HHHW .... .... : A-137 */
	/* X Memory Data Move :   0101 HHHW .... .... : A-137 */
	/* Quote: (32 General parallel move instructions) */
	/****************************************************************/
	else if (((w0 & 0xff00) == 0x4a00) ||
				((w0 & 0xf000) == 0x4000) ||
				((w0 & 0xf800) == 0x3000) ||
				((w0 & 0x8000) == 0x8000) ||
				((w0 & 0xf000) == 0x5000))
	{
		/* Note: There is much overlap down here, so certain ops must come before others */

		/* CLR : .... .... 0000 F001 : A-60 */
		if ((w0 & 0x00f7) == 0x0001)
		{
			return global_alloc(Clr(opc, w0, w1));
		}
		/* ADD : .... .... 0000 FJJJ : A-22 */
		else if ((w0 & 0x00f0) == 0x0000)
		{
			return global_alloc(Add(opc, w0, w1));
		}

		/* MOVE : .... .... 0001 0001 : A-128 */
		else if ((w0 & 0x00ff) == 0x0011 || (w0 & 0x00ff) == 0x0019)
		// NEW // else if ((w0 & 0x00ff) == 0x0011)
		{
			return global_alloc(Move(opc, w0, w1));
		}
		/* TFR : .... .... 0001 FJJJ : A-212 */
		else if ((w0 & 0x00f0) == 0x0010)
		{
			return global_alloc(Tfr(opc, w0, w1));
		}

		/* RND : .... .... 0010 F000 : A-188 */
		else if ((w0 & 0x00f7) == 0x0020)
		{
			return global_alloc(Rnd(opc, w0, w1));
		}
		/* TST : .... .... 0010 F001 : A-218 */
		else if ((w0 & 0x00f7) == 0x0021)
		{
			return global_alloc(Tst(opc, w0, w1));
		}
		/* INC : .... .... 0010 F010 : A-104 */
		else if ((w0 & 0x00f7) == 0x0022)
		{
			return global_alloc(Inc(opc, w0, w1));
		}
		/* INC24 : .... .... 0010 F011 : A-106 */
		else if ((w0 & 0x00f7) == 0x0023)
		{
			return global_alloc(Inc24(opc, w0, w1));
		}
		/* OR : .... .... 0010 F1JJ : A-176 */
		else if ((w0 & 0x00f4) == 0x0024)
		{
			return global_alloc(Or(opc, w0, w1));
		}

		/* ASR : .... .... 0011 F000 : A-32 */
		else if ((w0 & 0x00f7) == 0x0030)
		{
			return global_alloc(Asr(opc, w0, w1));
		}
		/* ASL : .... .... 0011 F001 : A-28 */
		else if ((w0 & 0x00f7) == 0x0031)
		{
			return global_alloc(Asl(opc, w0, w1));
		}
		/* LSR : .... .... 0011 F010 : A-120 */
		else if ((w0 & 0x00f7) == 0x0032)
		{
			return global_alloc(Lsr(opc, w0, w1));
		}
		/* LSL : .... .... 0011 F011 : A-118 */
		else if ((w0 & 0x00f7) == 0x0033)
		{
			return global_alloc(Lsl(opc, w0, w1));
		}
		/* EOR : .... .... 0011 F1JJ : A-94 */
		else if ((w0 & 0x00f4) == 0x0034)
		{
			return global_alloc(Eor(opc, w0, w1));
		}

		/* SUBL : .... .... 0100 F001 : A-204 */
		else if ((w0 & 0x00f7) == 0x0041)
		{
			return global_alloc(Subl(opc, w0, w1));
		}
		/* SUB : .... .... 0100 FJJJ : A-202 */
		else if ((w0 & 0x00f0) == 0x0040)
		{
			return global_alloc(Sub(opc, w0, w1));
		}

		/* CLR24 : .... .... 0101 F001 : A-62 */
		else if ((w0 & 0x00f7) == 0x0051)
		{
			return global_alloc(Clr24(opc, w0, w1));
		}
		/* SBC : .... .... 0101 F01J : A-198 */
		else if ((w0 & 0x00f6) == 0x0052)
		{
			return global_alloc(Sbc(opc, w0, w1));
		}
		/* CMP : .... .... 0101 FJJJ : A-64 */
		else if ((w0 & 0x00f0) == 0x0050)
		{
			return global_alloc(Cmp(opc, w0, w1));
		}

		/* NEG : .... .... 0110 F000 : A-166 */
		else if ((w0 & 0x00f7) == 0x0060)
		{
			return global_alloc(Neg(opc, w0, w1));
		}
		/* NOT : .... .... 0110 F001 : A-174 */
		else if ((w0 & 0x00f7) == 0x0061)
		{
			return global_alloc(Not(opc, w0, w1));
		}
		/* DEC : .... .... 0110 F010 : A-72 */
		else if ((w0 & 0x00f7) == 0x0062)
		{
			return global_alloc(Dec(opc, w0, w1));
		}
		/* DEC24 : .... .... 0110 F011 : A-74 */
		else if ((w0 & 0x00f7) == 0x0063)
		{
			return global_alloc(Dec24(opc, w0, w1));
		}
		/* AND : .... .... 0110 F1JJ : A-24 */
		else if ((w0 & 0x00f4) == 0x0064)
		{
			return global_alloc(And(opc, w0, w1));
		}

		/* ABS : .... .... 0111 F001 : A-18 */
		if ((w0 & 0x00f7) == 0x0071)
		{
			return global_alloc(Abs(opc, w0, w1));
		}
		/* ROR : .... .... 0111 F010 : A-192 */
		else if ((w0 & 0x00f7) == 0x0072)
		{
			return global_alloc(Ror(opc, w0, w1));
		}
		/* ROL : .... .... 0111 F011 : A-190 */
		else if ((w0 & 0x00f7) == 0x0073)
		{
			return global_alloc(Rol(opc, w0, w1));
		}
		/* CMPM : .... .... 0111 FJJJ : A-66 */
		else if ((w0 & 0x00f0) == 0x0070)
		{
			return global_alloc(Cmpm(opc, w0, w1));
		}

		/* MPY : .... .... 1k00 FQQQ : A-160 */
		else if ((w0 & 0x00b0) == 0x0080)
		{
			return global_alloc(Mpy(opc, w0, w1));
		}
		/* MPYR : .... .... 1k01 FQQQ : A-162 */
		else if ((w0 & 0x00b0) == 0x0090)
		{
			return global_alloc(Mpyr(opc, w0, w1));
		}
		/* MAC : .... .... 1k10 FQQQ : A-122 */
		else if ((w0 & 0x00b0) == 0x00a0)
		{
			return global_alloc(Mac(opc, w0, w1));
		}
		/* MACR : .... .... 1k11 FQQQ : A-124 */
		else if ((w0 & 0x00b0) == 0x00b0)
		{
			return global_alloc(Macr(opc, w0, w1));
		}
	}

	/******************************/
	/* Remaining non-parallel ops */
	/******************************/
	/* ADC : 0001 0101 0000 F01J : A-20 */
	else if ((w0 & 0xfff6) == 0x1502)
	{
		return global_alloc(Adc(opc, w0, w1));
	}
	/* ANDI : 0001 1EE0 iiii iiii : A-26 */
	/* Note: MoveP sneaks in here if you don't check 0x0600 */
	else if (((w0 & 0xf900) == 0x1800) & ((w0 & 0x0600) != 0x0000))
	{
		return global_alloc(Andi(opc, w0, w1));
	}
	/* ASL4 : 0001 0101 0011 F001 : A-30 */
	else if ((w0 & 0xfff7) == 0x1531)
	{
		return global_alloc(Asl4(opc, w0, w1));
	}
	/* ASR4 : 0001 0101 0011 F000 : A-34 */
	else if ((w0 & 0xfff7) == 0x1530)
	{
		return global_alloc(Asr4(opc, w0, w1));
	}
	/* ASR16 : 0001 0101 0111 F000 : A-36 */
	else if ((w0 & 0xfff7) == 0x1570)
	{
		return global_alloc(Asr16(opc, w0, w1));
	}
	/* BFCHG : 0001 0100 11Pp pppp BBB1 0010 iiii iiii : A-38 */
	else if (((w0 & 0xffc0) == 0x14c0) && ((w1 & 0x1f00) == 0x1200))
	{
		return global_alloc(BfInstruction(opc, w0, w1));
	}
	/* BFCHG : 0001 0100 101- --RR BBB1 0010 iiii iiii : A-38 */
	else if (((w0 & 0xfff0) == 0x14b0) && ((w1 & 0x1f00) == 0x1200))
	// NEW // else if (((w0 & 0xffe0) == 0x14a0) && ((w1 & 0x1f00) == 0x1200))
	{
		return global_alloc(BfInstruction_2(opc, w0, w1));
	}
	/* BFCHG : 0001 0100 100D DDDD BBB1 0010 iiii iiii : A-38 */
	else if (((w0 & 0xffe0) == 0x1480) && ((w1 & 0x1f00) == 0x1200))
	{
		return global_alloc(BfInstruction_3(opc, w0, w1));
	}
	/* BFCLR : 0001 0100 11Pp pppp BBB0 0100 iiii iiii : A-40 */
	else if (((w0 & 0xffc0) == 0x14c0) && ((w1 & 0x1f00) == 0x0400))
	{
		return global_alloc(BfInstruction(opc, w0, w1));
	}
	/* BFCLR : 0001 0100 101- --RR BBB0 0100 iiii iiii : A-40 */
	else if (((w0 & 0xfff0) == 0x14b0) && ((w1 & 0x1f00) == 0x0400))
	// NEW // else if (((w0 & 0xffe0) == 0x14a0) && ((w1 & 0x1f00) == 0x0400))
	{
		return global_alloc(BfInstruction_2(opc, w0, w1));
	}
	/* BFCLR : 0001 0100 100D DDDD BBB0 0100 iiii iiii : A-40 */
	else if (((w0 & 0xffe0) == 0x1480) && ((w1 & 0x1f00) == 0x0400))
	{
		return global_alloc(BfInstruction_3(opc, w0, w1));
	}
	/* BFSET : 0001 0100 11Pp pppp BBB1 1000 iiii iiii : A-42 */
	else if (((w0 & 0xffc0) == 0x14c0) && ((w1 & 0x1f00) == 0x1800))
	{
		return global_alloc(BfInstruction(opc, w0, w1));
	}
	/* BFSET : 0001 0100 101- --RR BBB1 1000 iiii iiii : A-42 */
	else if (((w0 & 0xfff0) == 0x14b0) && ((w1 & 0x1f00) == 0x1800))
	// NEW // else if (((w0 & 0xffe0) == 0x14a0) && ((w1 & 0x1f00) == 0x1800))
	{
		return global_alloc(BfInstruction_2(opc, w0, w1));
	}
	/* BFSET : 0001 0100 100D DDDD BBB1 1000 iiii iiii : A-42 */
	else if (((w0 & 0xffe0) == 0x1480) && ((w1 & 0x1f00) == 0x1800))
	{
		return global_alloc(BfInstruction_3(opc, w0, w1));
	}
	/* BFTSTH : 0001 0100 01Pp pppp BBB1 0000 iiii iiii : A-44 */
	else if (((w0 & 0xffc0) == 0x1440) && ((w1 & 0x1f00) == 0x1000))
	{
		return global_alloc(BfInstruction(opc, w0, w1));
	}
	/* BFTSTH : 0001 0100 001- --RR BBB1 0000 iiii iiii : A-44 */
	else if (((w0 & 0xfff0) == 0x1430) && ((w1 & 0x1f00) == 0x1000))
	// NEW // else if (((w0 & 0xffe0) == 0x1420) && ((w1 & 0x1f00) == 0x1000))
	{
		return global_alloc(BfInstruction_2(opc, w0, w1));
	}
	/* BFTSTH : 0001 0100 000D DDDD BBB1 0000 iiii iiii : A-44 */
	else if (((w0 & 0xffe0) == 0x1400) && ((w1 & 0x1f00) == 0x1000))
	{
		return global_alloc(BfInstruction_3(opc, w0, w1));
	}
	/* BFTSTL : 0001 0100 01Pp pppp BBB0 0000 iiii iiii : A-46 */
	else if (((w0 & 0xffc0) == 0x1440) && ((w1 & 0x1f00) == 0x0000))
	{
		return global_alloc(BfInstruction(opc, w0, w1));
	}
	/* BFTSTL : 0001 0100 001- --RR BBB0 0000 iiii iiii : A-46 */
	else if (((w0 & 0xfff0) == 0x1430) && ((w1 & 0x1f00) == 0x0000))
	// NEW // else if (((w0 & 0xffe0) == 0x1420) && ((w1 & 0x1f00) == 0x0000))
	{
		return global_alloc(BfInstruction_2(opc, w0, w1));
	}
	/* BFTSTL : 0001 0100 000D DDDD BBB0 0000 iiii iiii : A-46 */
	else if (((w0 & 0xffe0) == 0x1400) && ((w1 & 0x1f00) == 0x0000))
	{
		return global_alloc(BfInstruction_3(opc, w0, w1));
	}
	/* Bcc : 0000 0111 --11 cccc xxxx xxxx xxxx xxxx : A-48 */
	else if (((w0 & 0xff30) == 0x0730) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Bcc(opc, w0, w1));
	}
	/* Bcc : 0010 11cc ccee eeee : A-48 */
	else if ((w0 & 0xfc00) == 0x2c00)
	{
		return global_alloc(Bcc_2(opc, w0, w1));
	}
	/* Bcc : 0000 0111 RR10 cccc : A-48 */
	else if ((w0 & 0xff30) == 0x0720)
	{
		return global_alloc(Bcc_3(opc, w0, w1));
	}
	/* BRA : 0000 0001 0011 11-- xxxx xxxx xxxx xxxx : A-50 */
	else if (((w0 & 0xfffc) == 0x013c) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Bra(opc, w0, w1));
	}
	/* BRA : 0000 1011 aaaa aaaa : A-50 */
	else if ((w0 & 0xff00) == 0x0b00)
	{
		return global_alloc(Bra_2(opc, w0, w1));
	}
	/* BRA : 0000 0001 0010 11RR : A-50 */
	else if ((w0 & 0xfffc) == 0x012c)
	{
		return global_alloc(Bra_3(opc, w0, w1));
	}
	/* BRKc : 0000 0001 0001 cccc : A-52 */
	else if ((w0 & 0xfff0) == 0x0110)
	{
		return global_alloc(Brkcc(opc, w0, w1));
	}
	/* BScc : 0000 0111 --01 cccc xxxx xxxx xxxx xxxx : A-54 */
	else if (((w0 & 0xff30) == 0x0710) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Bscc(opc, w0, w1));
	}
	/* BScc : 0000 0111 RR00 cccc : A-54 */
	else if ((w0 & 0xff30) == 0x0700)
	{
		return global_alloc(Bscc_2(opc, w0, w1));
	}
	/* BSR : 0000 0001 0011 10-- xxxx xxxx xxxx xxxx : A-56 */
	else if (((w0 & 0xfffc) == 0x0138) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Bsr(opc, w0, w1));
	}
	/* BSR : 0000 0001 0010 10RR : A-56 */
	else if ((w0 & 0xfffc) == 0x0128)
	{
		return global_alloc(Bsr_2(opc, w0, w1));
	}
	/* CHKAAU : 0000 0000 0000 0100 : A-58 */
	else if ((w0 & 0xffff) == 0x0004)
	{
		return global_alloc(Chkaau(opc, w0, w1));
	}
	/* DEBUG : 0000 0000 0000 0001 : A-68 */
	else if ((w0 & 0xffff) == 0x0001)
	{
		return global_alloc(Debug(opc, w0, w1));
	}
	/* DEBUGcc : 0000 0000 0101 cccc : A-70 */
	else if ((w0 & 0xfff0) == 0x0050)
	{
		return global_alloc(Debugcc(opc, w0, w1));
	}
	/* DIV : 0001 0101 0--0 F1DD : A-76 */
	else if ((w0 & 0xfff4) == 0x1504)
	// NEW // else if ((w0 & 0xff94) == 0x1504)
	{
		return global_alloc(Div(opc, w0, w1));
	}
	/* DMAC : 0001 0101 10s1 FsQQ : A-80 */
	else if ((w0 & 0xffd0) == 0x1590)
	{
		return global_alloc(Dmac(opc, w0, w1));
	}
	/* DO : 0000 0000 110- --RR xxxx xxxx xxxx xxxx : A-82 */
	else if (((w0 & 0xffe0) == 0x00c0) && ((w1 & 0x0000) == 0x0000))       // Wait.  Huh?
	{
		return global_alloc(Do(opc, w0, w1));
	}
	/* DO : 0000 1110 iiii iiii xxxx xxxx xxxx xxxx : A-82 */
	else if (((w0 & 0xff00) == 0x0e00) && ((w1 & 0x0000) == 0x0000))       // Wait.  Huh?
	{
		return global_alloc(Do_2(opc, w0, w1));
	}
	/* DO : 0000 0100 000D DDDD xxxx xxxx xxxx xxxx : A-82 */
	else if (((w0 & 0xffe0) == 0x0400) && ((w1 & 0x0000) == 0x0000))       // Wait.  Huh?
	{
		return global_alloc(Do_3(opc, w0, w1));
	}
	/* DO FOREVER : 0000 0000 0000 0010 xxxx xxxx xxxx xxxx : A-88 */
	else if (((w0 & 0xffff) == 0x0002) && ((w1 & 0x0000) == 0x0000))       // Wait.  Huh?
	{
		return global_alloc(DoForever(opc, w0, w1));
	}
	/* ENDDO : 0000 0000 0000 1001 : A-92 */
	else if ((w0 & 0xffff) == 0x0009)
	{
		return global_alloc(Enddo(opc, w0, w1));
	}
	/* EXT : 0001 0101 0101 F010 : A-96 */
	else if ((w0 & 0xfff7) == 0x1552)
	{
		return global_alloc(Ext(opc, w0, w1));
	}
	/* ILLEGAL : 0000 0000 0000 1111 : A-98 */
	else if ((w0 & 0xffff) == 0x000f)
	{
		return global_alloc(Illegal(opc, w0, w1));
	}
	/* IMAC : 0001 0101 1010 FQQQ : A-100 */
	else if ((w0 & 0xfff0) == 0x15a0)
	{
		return global_alloc(Imac(opc, w0, w1));
	}
	/* IMPY : 0001 0101 1000 FQQQ : A-102 */
	else if ((w0 & 0xfff0) == 0x1580)
	{
		return global_alloc(Impy(opc, w0, w1));
	}
	/* Jcc : 0000 0110 --11 cccc xxxx xxxx xxxx xxxx : A-108 */
	else if (((w0 & 0xff30) == 0x0630) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Jcc(opc, w0, w1));
	}
	/* Jcc : 0000 0110 RR10 cccc : A-108 */
	else if ((w0 & 0xff30) == 0x0620 )
	{
		return global_alloc(Jcc_2(opc, w0, w1));
	}
	/* JMP : 0000 0001 0011 01-- xxxx xxxx xxxx xxxx : A-110 */
	else if (((w0 & 0xfffc) == 0x0134) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Jmp(opc, w0, w1));
	}
	/* JMP : 0000 0001 0010 01RR : A-110 */
	else if ((w0 & 0xfffc) == 0x0124)
	{
		//JMP2->m_oco = opc;
		//JMP2->decode(w0, w1);
		//return JMP2;
		return global_alloc(Jmp_2(opc, w0, w1));
	}
	/* JScc : 0000 0110 --01 cccc xxxx xxxx xxxx xxxx : A-112 */
	else if (((w0 & 0xff30) == 0x0610) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Jscc(opc, w0, w1));
	}
	/* JScc : 0000 0110 RR00 cccc : A-112 */
	else if ((w0 & 0xff30) == 0x0600)
	{
		return global_alloc(Jscc_2(opc, w0, w1));
	}
	/* JSR : 0000 0001 0011 00-- xxxx xxxx xxxx xxxx : A-114 */
	else if (((w0 & 0xfffc) == 0x0130) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Jsr(opc, w0, w1));
	}
	/* JSR : 0000 1010 AAAA AAAA : A-114 */
	else if ((w0 & 0xff00) == 0x0a00)
	{
		return global_alloc(Jsr_2(opc, w0, w1));
	}
	/* JSR : 0000 0001 0010 00RR : A-114 */
	else if ((w0 & 0xfffc) == 0x0120)
	{
		return global_alloc(Jsr_3(opc, w0, w1));
	}
	/* LEA : 0000 0001 11TT MMRR : A-116 */
	else if ((w0 & 0xffc0) == 0x01c0)
	{
		return global_alloc(Lea(opc, w0, w1));
	}
	/* LEA : 0000 0001 10NN MMRR : A-116 */
	else if ((w0 & 0xffc0) == 0x0180)
	{
		return global_alloc(Lea_2(opc, w0, w1));
	}
	/* MAC(su,uu) : 0001 0101 1110 FsQQ : A-126 */
	else if ((w0 & 0xfff0) == 0x15e0)
	{
		return global_alloc(Macsuuu(opc, w0, w1));
	}
	/* MOVE : 0000 0101 BBBB BBBB ---- HHHW 0001 0001 : A-128 */
// NEW //   else if (((w0 & 0xff00) == 0x0500) && ((w1 & 0x00ff) == 0x0011))
// NEW //   {
// NEW //       return global_alloc(Move_3(opc, w0, w1));
// NEW //   }
	/* MOVE(C) : 0011 1WDD DDD0 MMRR : A-144 */
	else if ((w0 & 0xf810) == 0x3800)
	{
		return global_alloc(Movec(opc, w0, w1));
	}
	/* MOVE(C) : 0011 1WDD DDD1 q0RR : A-144 */
	else if ((w0 & 0xf814) == 0x3810)
	{
		return global_alloc(Movec_2(opc, w0, w1));
	}
	/* MOVE(C) : 0011 1WDD DDD1 Z11- : A-144 */
	else if ((w0 & 0xf816) == 0x3816)
	{
		return global_alloc(Movec_3(opc, w0, w1));
	}
	/* MOVE(C) : 0011 1WDD DDD1 t10- xxxx xxxx xxxx xxxx : A-144 */
	else if (((w0 & 0xf816) == 0x3814) && ((w1 & 0x0000) == 0x0000))
	{
		return global_alloc(Movec_4(opc, w0, w1));
	}
	/* MOVE(C) : 0010 10dd dddD DDDD : A-144 */
	else if ((w0 & 0xfc00) == 0x2800)
	{
		return global_alloc(Movec_5(opc, w0, w1));
	}
	/* MOVE(C) : 0000 0101 BBBB BBBB 0011 1WDD DDD0 ---- : A-144 */
	else if (((w0 & 0xff00) == 0x0500) && ((w1 & 0xf810) == 0x3800))
	{
		return global_alloc(Movec_6(opc, w0, w1));
	}
	/* MOVE(I) : 0010 00DD BBBB BBBB : A-150 */
	else if ((w0 & 0xfc00) == 0x2000)
	{
		return global_alloc(Movei(opc, w0, w1));
	}
	/* MOVE(M) : 0000 001W RR0M MHHH : A-152 */
	else if ((w0 & 0xfe20) == 0x0200)
	{
		return global_alloc(Movem(opc, w0, w1));
	}
	/* MOVE(M) : 0000 001W RR11 mmRR : A-152 */
	else if ((w0 & 0xfe30) == 0x0230)
	{
		return global_alloc(Movem_2(opc, w0, w1));
	}
	/* MOVE(M) : 0000 0101 BBBB BBBB 0000 001W --0- -HHH : A-152 */
	else if (((w0 & 0xff00) == 0x0500) && ((w1 & 0xfe20) == 0x0200))
	{
		return global_alloc(Movem_3(opc, w0, w1));
	}
	/* MOVE(P) : 0001 100W HH1p pppp : A-156 */
	else if ((w0 & 0xfe20) == 0x1820)
	{
		return global_alloc(Movep(opc, w0, w1));
	}
	/* MOVE(P) : 0000 110W RRmp pppp : A-156 */
	else if ((w0 & 0xfe00) == 0x0c00)
	{
		return global_alloc(Movep_2(opc, w0, w1));
	}
	/* MOVE(S) : 0001 100W HH0a aaaa : A-158 */
	else if ((w0 & 0xfe20) == 0x1800)
	{
		return global_alloc(Moves(opc, w0, w1));
	}
	/* MPY(su,uu) : 0001 0101 1100 FsQQ : A-164 */
	else if ((w0 & 0xfff0) == 0x15c0)
	{
		return global_alloc(Mpysuuu(opc, w0, w1));
	}
	/* NEGC : 0001 0101 0110 F000 : A-168 */
	else if ((w0 & 0xfff7) == 0x1560)
	{
		return global_alloc(Negc(opc, w0, w1));
	}
	/* NOP : 0000 0000 0000 0000 : A-170 */
	else if ((w0 & 0xffff) == 0x0000)
	{
		return global_alloc(Nop(opc, w0, w1));
	}
	/* NORM : 0001 0101 0010 F0RR : A-172 */
	else if ((w0 & 0xfff4) == 0x1520)
	{
		return global_alloc(Norm(opc, w0, w1));
	}
	/* ORI : 0001 1EE1 iiii iiii : A-178 */
	else if ((w0 & 0xf900) == 0x1900)
	{
		return global_alloc(Ori(opc, w0, w1));
	}
	/* REP : 0000 0000 111- --RR : A-180 */
	else if ((w0 & 0xffe0) == 0x00e0)
	{
		return global_alloc(Rep(opc, w0, w1));
	}
	/* REP : 0000 1111 iiii iiii : A-180 */
	else if ((w0 & 0xff00) == 0x0f00)
	{
		return global_alloc(Rep_2(opc, w0, w1));
	}
	/* REP : 0000 0100 001D DDDD : A-180 */
	else if ((w0 & 0xffe0) == 0x0420)
	{
		return global_alloc(Rep_3(opc, w0, w1));
	}
	/* REPcc : 0000 0001 0101 cccc : A-184 */
	else if ((w0 & 0xfff0) == 0x0150)
	{
		return global_alloc(Repcc(opc, w0, w1));
	}
	/* RESET : 0000 0000 0000 1000 : A-186 */
	else if ((w0 & 0xffff) == 0x0008)
	{
		return global_alloc(Reset(opc, w0, w1));
	}
	/* RTI : 0000 0000 0000 0111 : A-194 */
	else if ((w0 & 0xffff) == 0x0007)
	{
		return global_alloc(Rti(opc, w0, w1));
	}
	/* RTS : 0000 0000 0000 0110 : A-196 */
	else if ((w0 & 0xffff) == 0x0006)
	{
		return global_alloc(Rts(opc, w0, w1));
	}
	/* STOP : 0000 0000 0000 1010 : A-200 */
	else if ((w0 & 0xffff) == 0x000a)
	{
		return global_alloc(Stop(opc, w0, w1));
	}
	/* SWAP : 0001 0101 0111 F001 : A-206 */
	else if ((w0 & 0xfff7) == 0x1571)
	{
		return global_alloc(Swap(opc, w0, w1));
	}
	/* SWI : 0000 0000 0000 0101 : A-208 */
	else if ((w0 & 0xffff) == 0x0005)
	{
		return global_alloc(Swi(opc, w0, w1));
	}
	/* Tcc : 0001 00cc ccTT Fh0h : A-210 */
	else if ((w0 & 0xfc02) == 0x1000)
	{
		return global_alloc(Tcc(opc, w0, w1));
	}
	/* TFR(2) : 0001 0101 0000 F00J : A-214 */
	else if ((w0 & 0xfff6) == 0x1500)
	{
		return global_alloc(Tfr2(opc, w0, w1));
	}
	/* TFR(3) : 0010 01mW RRDD FHHH : A-216 */
	else if ((w0 & 0xfc00) == 0x2400)
	{
		return global_alloc(Tfr3(opc, w0, w1));
	}
	/* TST(2) : 0001 0101 0001 -1DD : A-220 */
	else if ((w0 & 0xfffc) == 0x1514)
	// NEW // else if ((w0 & 0xfff4) == 0x1514)
	{
		return global_alloc(Tst2(opc, w0, w1));
	}
	/* WAIT : 0000 0000 0000 1011 : A-222 */
	else if ((w0 & 0xffff) == 0x000b)
	{
		return global_alloc(Wait(opc, w0, w1));
	}
	/* ZERO : 0001 0101 0101 F000 : A-224 */
	else if ((w0 & 0xfff7) == 0x1550)
	{
		return global_alloc(Zero(opc, w0, w1));
	}
	/* SHFL : 0001 0101 1101 FQQQ : !!UNDOCUMENTED!! */
	else if ((w0 & 0xfff0) ==  0x15d0)
	{
		return global_alloc(Shfl(opc, w0, w1));
	}
	/* SHFR : 0001 0101 1111 FQQQ : !!UNDOCUMENTED!! */
	else if ((w0 & 0xfff0) ==  0x15f0)
	{
		return global_alloc(Shfr(opc, w0, w1));
	}

	return NULL;
}

}
