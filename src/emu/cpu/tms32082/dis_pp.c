// TMS32082 PP Disassembler

#include "emu.h"


static const char *REG_NAMES[128] =
{
	// 0 - 15
	"a0",		"a1",		"a2",		"a3",		"a4",		"???",		"a6",		"a7",
	"a8",		"a9",		"a10",		"a11",		"a12",		"???",		"a14",		"a15",
	// 16 - 31
	"x0",		"x1",		"x2",		"???",		"???",		"???",		"???",		"???",
	"x8",		"x9",		"x10",		"???",		"???",		"???",		"???",		"???",
	// 32 - 47
	"d0",		"d1",		"d2",		"d3",		"d4",		"d5",		"d6",		"d7",
	"???",		"sr",		"mf",		"???",		"???",		"???",		"???",		"???",
	// 48 - 63
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"pc/call",	"ipa/br",	"ipe",		"iprs",		"inten",	"intflg",	"comm",		"lctl",
	// 64 - 79
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	// 80 - 95
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	// 96 - 111
	"lc0",		"lc1",		"lc2",		"???",		"lr0",		"lr1",		"lr2",		"???",
	"lrse0",	"lrse1",	"lrse2",	"???",		"lrs0",		"lrs1",		"lrs2",		"???",
	// 112 - 127
	"ls0",		"ls1",		"ls2",		"???",		"le0",		"le1",		"le2",		"???",
	"???",		"???",		"???",		"???",		"tag0",		"tag1",		"tag2",		"tag3"
};


static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static void format_transfer(UINT64 op)
{
	int lmode = (op >> 35) & 0xf;
	int gmode = (op >> 13) & 0xf;

	print(" | ");

	switch (lmode)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			switch (gmode)
			{
				case 0x00:
				{
					int dstbank = (op >> 18) & 0xf;
					int srcbank = (op >> 6) & 0xf;
					int src = (op >> 10) & 0x7;
					int dst = (op >> 3) & 0x7;

					int dreg = (dstbank << 3) | dst;
					int sreg = (srcbank << 3) | src;

					print("cond du||cond move %s = %s", REG_NAMES[dreg], REG_NAMES[sreg]);
					break;
				}
				case 0x01:
				{
					print("cond du||cond field move");
					break;
				}
				case 0x02: case 0x03:
				{
					print("cond non-d du");
					break;
				}
				default:
				{
					if (op & 0x4)
					{
						print("cond du||cond global");
					}
					else
					{
						print("global (long offset)");
					}
					break;
				}
			}
			break;
		}

		default:
		{
			int mode = gmode | ((op & (1 << 24)) ? 0x10 : 0);

			switch (mode)
			{
				case 0x00:
				{
					print("move||local");
					break;
				}
				case 0x01:
				{
					print("field move||local");
					break;
				}
				case 0x02: case 0x03:
				{
					print("non-d du||local");
					break;
				}
				case 0x10: case 0x11: case 0x12: case 0x13:
				{
					int d = (op >> 32) & 0x7;
					int bank = (op >> 18) & 0xf;
					int s = (op & (1 << 28));
					int size = (op >> 29) & 0x3;
					int le = ((op >> 16) & 2) | ((op >> 31) & 1);
					int la = (op >> 25) & 0x7;

					int reg = (bank << 3) | d;

					UINT16 offset = 0;
					if (s)
					{
						offset = op & 0x7fff;
						if (offset & 0x4000)
							offset |= 0xc000;
					}
					else
					{
						offset = op & 0x7fff;
					}

					if (le == 0 && le == 1)
					{
						print("%s = ", REG_NAMES[reg]);
					}

					switch (size)
					{
						case 0: print("b:"); break;
						case 1: print("h:"); break;
					}

					switch (lmode)
					{
						case 0x6: print("*(a%d++=0x%04X)", la, offset); break;
						case 0x7: print("*(a%d--=0x%04X)", la, offset); break;
						case 0xa: print("*(a%d+0x%04X)", la, offset); break;
						case 0xb: print("*(a%d-0x%04X)", la, offset); break;
						case 0xe: print("*(a%d+=0x%04X)", la, offset); break;
						case 0xf: print("*(a%d-=0x%04X)", la, offset); break;
					}

					if (le == 2)
						print(" = %s", REG_NAMES[reg]);

					break;
				}
				case 0x14: case 0x15: case 0x16: case 0x17:
				case 0x18: case 0x19: case 0x1a: case 0x1b:
				case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				{
					print("double parallel");
					break;
				}
			}
		}
	}
}

static void format_alu_op(int aluop, int a, const char *dst_text, const char *a_text, const char *b_text, const char *c_text)
{
	if (a)		// arithmetic
	{
		int bits = (aluop & 1) | ((aluop >> 1) & 2) | ((aluop >> 2) & 4) | ((aluop >> 3) & 8);
		switch (bits)
		{
			case 1:		print("%s = %s - %s<1<", dst_text, a_text, b_text); break;
			case 2:		print("%s = %s + %s<0<", dst_text, a_text, b_text); break;
			case 3:		print("%s = %s - %s", dst_text, a_text, c_text); break;
			case 4:		print("%s = %s - %s>1>", dst_text, a_text, b_text); break;
			case 5:		print("%s = %s - %s", dst_text, a_text, b_text); break;
			case 6:		print("?"); break;
			case 7:		print("%s = %s - %s>0>", dst_text, a_text, b_text); break;
			case 8:		print("%s = %s + %s>0>", dst_text, a_text, b_text); break;
			case 9:		print("?"); break;
			case 10:	print("%s = %s + %s", dst_text, a_text, b_text); break;
			case 11:	print("%s = %s + %s>1>", dst_text, a_text, b_text); break;
			case 12:	print("%s = %s + %s", dst_text, a_text, c_text); break;
			case 13:	print("%s = %s - %s<0<", dst_text, a_text, b_text); break;
			case 14:	print("%s = %s + %s<1<", dst_text, a_text, b_text); break;
			case 15:	print("%s = field %s + %s", dst_text, a_text, b_text); break;
		}
	}
	else		// boolean
	{
		switch (aluop)
		{
			case 0xaa:		// A & B & C | A & ~B & C | A & B & ~C | A & ~B & ~C       = A
				print("%s = %s", dst_text, a_text);
				break;

			case 0x55:		// ~A & B & C | ~A & ~B & C | ~A & B & ~C | ~A & ~B & ~C   = ~A
				print("%s = ~%s", dst_text, a_text);
				break;

			case 0xcc:		// A & B & C | ~A & B & C | A & B & ~C | ~A & B & ~C       = B
				print("%s = %s", dst_text, b_text);
				break;

			case 0x33:		// A & ~B & C | ~A & ~B & C | A & ~B & ~C | ~A & ~B & ~C   = ~B
				print("%s = %s", dst_text, b_text);
				break;

			case 0xf0:		// A & B & C | ~A & B & C | A & ~B & C | ~A & ~B & C       = C
				print("%s = %s", dst_text, c_text);
				break;

			case 0x0f:		// A & B & ~C | ~A & B & ~C | A & ~B & ~C | ~A & ~B & ~C   = ~C
				print("%s = ~%s", dst_text, c_text);
				break;

			case 0x80:		// A & B & C
				print("%s = %s & %s & %s", dst_text, a_text, b_text, c_text);
				break;

			case 0x88:		// A & B & C | A & B & ~C                                  = A & B
				print("%s = %s & %s", dst_text, a_text, b_text);
				break;

			case 0xa0:		// A & B & C | A & ~B & C                                  = A & C
				print("%s = %s & %s", dst_text, a_text, c_text);
				break;

			case 0xc0:		// A & B & C | ~A & B & C                                  = B & C
				print("%s = %s & %s", dst_text, b_text, c_text);
				break;

			default:
				print("%s = b%02X(%s, %s, %s)", dst_text, aluop, a_text, b_text, c_text);
				break;
		}
	}
}

static offs_t tms32082_disasm_pp(char *buffer, offs_t pc, const UINT8 *oprom)
{
	output = buffer;
	UINT32 flags = 0;
	
	UINT64 op = ((UINT64)(oprom[0]) << 56) | ((UINT64)(oprom[1]) << 48) | ((UINT64)(oprom[2]) << 40) | ((UINT64)(oprom[3]) << 32) |
				((UINT64)(oprom[4]) << 24) | ((UINT64)(oprom[5]) << 16) | ((UINT64)(oprom[6]) << 8) | ((UINT64)(oprom[7]));

	switch (op >> 60)
	{
		case 0x6:
		case 0x7:			// Six-operand
		{
			print("A: six operand");
			break;
		}

		case 0x8:
		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
		{
			if ((op & U64(0xfaa8100000000000)) == U64(0x8800000000000000))
			{
				int operation = (op >> 39) & 0x1f;
				UINT64 parallel_xfer = (op & U64(0x0000003fffffffff));

				switch (operation)
				{
					case 0x00: print("nop"); break;
					case 0x02: print("eint"); break;
					case 0x03: print("dint"); break;
					default:   print("<reserved>"); break;	
				}

				format_transfer(parallel_xfer);
			}
			else
			{
				char dst_text[32];
				char a_text[32];
				char b_text[32];
				char c_text[32];

				switch ((op >> 43) & 3)
				{
					case 0:
					case 1:		// Base set ALU (5-bit immediate)
					{
						UINT64 parallel_xfer = (op & U64(0x0000003fffffffff));

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2imm = (op >> 39) & 0x1f;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int s1reg = src1 | (0x4 << 3);
						int dreg = dst | (0x4 << 3);
						int dstcreg = dst | (0x4 << 3);

						sprintf(dst_text, "%s", REG_NAMES[dreg]);
						switch (cl)
						{
							case 0:
								sprintf(a_text, "0x%02X", src2imm);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 1:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "0x%02X", src2imm);
								break;
							case 2:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "%%0x%02X", src2imm);
								break;
							case 3:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\0x%02X", REG_NAMES[s1reg], src2imm);
								sprintf(c_text, "%%0x%02X", src2imm);
								break;
							case 4:
								sprintf(a_text, "0x%02X", src2imm);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "%%d0");
								break;
							case 5:
								sprintf(a_text, "0x%02X", src2imm);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 6:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "0x%02X", src2imm);
								break;
							case 7:
								sprintf(a_text, "%s", REG_NAMES[s1reg]);
								sprintf(b_text, "1\\\\0x%02X", src2imm);
								sprintf(c_text, "0x%02X", src2imm);
								break;
						}

						format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);
						format_transfer(parallel_xfer);
						break;
					}
	
					case 2:		// Base set ALU (reg src2)
					{
						UINT64 parallel_xfer = (op & U64(0x0000003fffffffff));

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2 = (op >> 39) & 7;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int s1reg = src1 | (0x4 << 3);
						int s2reg = src2 | (0x4 << 3);
						int dstcreg = dst | (0x4 << 3);
						int dreg = dst | (0x4 << 3);

						sprintf(dst_text, "%s", REG_NAMES[dreg]);
						switch (cl)
						{
							case 0:
								sprintf(a_text, "%s", REG_NAMES[s1reg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 1:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "%s", REG_NAMES[s2reg]);
								break;
							case 2:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "%%%s", REG_NAMES[s1reg]);
								break;
							case 3:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\%s", REG_NAMES[s1reg], REG_NAMES[s2reg]);
								sprintf(c_text, "%%%s", REG_NAMES[s2reg]);
								break;
							case 4:
								sprintf(a_text, "%s", REG_NAMES[s2reg]);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "%%d0");
								break;
							case 5:
								sprintf(a_text, "%s", REG_NAMES[s2reg]);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 6:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "%s", REG_NAMES[s2reg]);
								break;
							case 7:
								sprintf(a_text, "%s", REG_NAMES[s1reg]);
								sprintf(b_text, "1\\\\%s", REG_NAMES[s2reg]);
								sprintf(c_text, "%s", REG_NAMES[s2reg]);
								break;
						}

						format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);
						format_transfer(parallel_xfer);
						break;
					}
	
					case 3:		// Base set ALU (32-bit immediate)
					{
						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int dstbank = (op >> 39) & 0xf;
						int s1bank = (op >> 36) & 7;
				//		int cond = (op >> 32) & 0xf;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;
						UINT32 imm32 = (UINT32)(op);

						int dreg = dst | (dstbank << 3);
						int s1reg = src1 | (s1bank << 3);
						int dstcreg = dst | (0x4 << 3);

						sprintf(dst_text, "%s", REG_NAMES[dreg]);
						switch (cl)
						{
							case 0:
								sprintf(a_text, "0x%08X", imm32);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 1:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "0x%08X", imm32);
								break;
							case 2:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "%%0x%08X", imm32);
								break;
							case 3:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s\\\\0x%08X", REG_NAMES[s1reg], imm32);
								sprintf(c_text, "%%0x%08X", imm32);
								break;
							case 4:
								sprintf(a_text, "0x%08X", imm32);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "%%d0");
								break;
							case 5:
								sprintf(a_text, "0x%08X", imm32);
								sprintf(b_text, "%s\\\\d0", REG_NAMES[s1reg]);
								sprintf(c_text, "@mf");
								break;
							case 6:
								sprintf(a_text, "%s", REG_NAMES[dstcreg]);
								sprintf(b_text, "%s", REG_NAMES[s1reg]);
								sprintf(c_text, "0x%08X", imm32);
								break;
							case 7:
								sprintf(a_text, "%s", REG_NAMES[s1reg]);
								sprintf(b_text, "1\\\\0x%08X", imm32);
								sprintf(c_text, "0x%08X", imm32);
								break;
						}

						format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);
						break;
					}
				}
			}
			break;
		}

		default:
			print("??? (%02X)", (UINT32)(op >> 60));
			break;
	}

	return 8 | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE(tms32082_pp)
{
	return tms32082_disasm_pp(buffer, pc, oprom);
}