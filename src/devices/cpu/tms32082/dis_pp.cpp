// license:BSD-3-Clause
// copyright-holders:Ville Linde
// TMS32082 PP Disassembler

#include "emu.h"


static const char *REG_NAMES[128] =
{
	// 0 - 15
	"a0",       "a1",       "a2",       "a3",       "a4",       "???",      "a6",       "a7",
	"a8",       "a9",       "a10",      "a11",      "a12",      "???",      "a14",      "a15",
	// 16 - 31
	"x0",       "x1",       "x2",       "???",      "???",      "???",      "???",      "???",
	"x8",       "x9",       "x10",      "???",      "???",      "???",      "???",      "???",
	// 32 - 47
	"d0",       "d1",       "d2",       "d3",       "d4",       "d5",       "d6",       "d7",
	"???",      "sr",       "mf",       "???",      "???",      "???",      "???",      "???",
	// 48 - 63
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"pc/call",  "ipa/br",   "ipe",      "iprs",     "inten",    "intflg",   "comm",     "lctl",
	// 64 - 79
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	// 80 - 95
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	"???",      "???",      "???",      "???",      "???",      "???",      "???",      "???",
	// 96 - 111
	"lc0",      "lc1",      "lc2",      "???",      "lr0",      "lr1",      "lr2",      "???",
	"lrse0",    "lrse1",    "lrse2",    "???",      "lrs0",     "lrs1",     "lrs2",     "???",
	// 112 - 127
	"ls0",      "ls1",      "ls2",      "???",      "le0",      "le1",      "le2",      "???",
	"???",      "???",      "???",      "???",      "tag0",     "tag1",     "tag2",     "tag3"
};

static const char *CONDITION_CODES[16] =
{
	"",         "[p] ",     "[ls] ",    "[hi] ",
	"[lt] ",    "[le] ",    "[ge] ",    "[gt] ",
	"[hs] ",    "[lo] ",    "[eq] ",    "[ne] ",
	"[v] ",     "[nv] ",    "[n] ",     "[nn] "
};

static const char *TRANSFER_SIZE[4] =
{
	"b:", "h:", "w:", ""
};


static char *output;

static void ATTR_PRINTF(1,2) print(const char *fmt, ...)
{
	va_list vl;

	va_start(vl, fmt);
	output += vsprintf(output, fmt, vl);
	va_end(vl);
}

static char *format_address_mode(int mode, int areg, int s, int limx)
{
	static char buffer[64];

	memset(buffer, 0, sizeof(char)*64);

	switch (mode)
	{
		case 0x4: sprintf(buffer, "*(a%d++=x%d)", areg, limx); break;
		case 0x5: sprintf(buffer, "*(a%d--=x%d)", areg, limx); break;
		case 0x6: sprintf(buffer, "*(a%d++0x%04X)", areg, limx); break;
		case 0x7: sprintf(buffer, "*(a%d--0x%04X)", areg, limx); break;
		case 0x8: sprintf(buffer, "*(a%d+x%d)", areg, limx); break;
		case 0x9: sprintf(buffer, "*(a%d-x%d)", areg, limx); break;
		case 0xa: sprintf(buffer, "*(a%d+0x%04X)", areg, limx); break;
		case 0xb: sprintf(buffer, "*(a%d-0x%04X)", areg, limx); break;
		case 0xc: sprintf(buffer, "*(a%d+=x%d)", areg, limx); break;
		case 0xd: sprintf(buffer, "*(a%d-=x%d)", areg, limx); break;
		case 0xe: sprintf(buffer, "*(a%d+=0x%04X)", areg, limx); break;
		case 0xf: sprintf(buffer, "*(a%d-=0x%04X)", areg, limx); break;
	}

	return buffer;
}

static void format_transfer(UINT64 op)
{
	char buffer[128];
	char *b = buffer;
	int lmode = (op >> 35) & 0xf;
	int gmode = (op >> 13) & 0xf;

	bool is_nop = false;

	memset(buffer, 0, sizeof(char)*128);

	switch (lmode)
	{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		{
			int cond = (op >> 32) & 0xf;

			switch (gmode)
			{
				case 0x00:                                      // Format 7: Conditional DU || Conditional Move
				{
					int dstbank = (op >> 18) & 0xf;
					int srcbank = (op >> 6) & 0xf;
					int src = (op >> 10) & 0x7;
					int dst = (op >> 3) & 0x7;

					int dreg = (dstbank << 3) | dst;
					int sreg = (srcbank << 3) | src;

					b += sprintf(b, "%s", CONDITION_CODES[cond]);
					b += sprintf(b, "%s = %s", REG_NAMES[dreg], REG_NAMES[sreg]);
					break;
				}
				case 0x01:                                      // Format 8: Conditional DU ||Conditional Field Move
				{
					int dstbank = (op >> 18) & 0xf;
					int src = (op >> 10) & 0x7;
					int dst = (op >> 3) & 0x7;
					int itm = (op >> 22) & 0x3;
					int size = (op >> 7) & 0x3;
			//      int e = (op & (1 << 9));

					int dreg = (dstbank << 3) | dst;
					int sreg = (4 << 3) | src;

					b += sprintf(b, "%s", CONDITION_CODES[cond]);
					b += sprintf(b, "%s = [%s%d]%s", REG_NAMES[dreg], TRANSFER_SIZE[size], itm, REG_NAMES[sreg]);
					break;
				}
				case 0x02: case 0x03:                           // Format 10: Conditional Non-D Data Unit
				{
					int as1bank = (op >> 6) & 0xf;
					int adstbank = (op >> 18) & 0xf;
					int src = (op >> 45) & 0x7;
					int dst = (op >> 48) & 0x7;

					int dreg = (adstbank << 3) | dst;
					int sreg = (as1bank << 3) | src;

					if (dreg == sreg)
					{
						is_nop = true;
					}
					else
					{
						b += sprintf(b, "%s", CONDITION_CODES[cond]);
						b += sprintf(b, "%s = %s", REG_NAMES[dreg], REG_NAMES[sreg]);
					}
					break;
				}
				default:
				{
					if (op & 0x4)                               // Format 9: Conditional DU || Conditional Global
					{
						int bank = (op >> 18) & 0xf;
						int le = ((op >> 16) & 2) | ((op >> 9) & 1);
						int size = (op >> 7) & 0x3;
						int s = (op & (1 << 6));
						int reg = (op >> 10) & 0x7;
						int ga = (op >> 3) & 0x7;
						int gimx = (op >> 22) & 0x7;

						int greg = (bank << 3) | reg;

						b += sprintf(b, "%s", CONDITION_CODES[cond]);

						switch (le)
						{
							case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx), REG_NAMES[greg]); break;
							case 1: b += sprintf(b, "%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, s, gimx)); break;
							case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx)); break;
							case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx)); break;
						}
					}
					else                                        // Format 5: Global (Long Offset)
					{
						int bank = (op >> 18) & 0xf;
						int le = ((op >> 16) & 2) | ((op >> 9) & 1);
						int size = (op >> 7) & 0x3;
						int s = (op & (1 << 6));
						int offset = (op >> 22) & 0x7fff;
						int reg = (op >> 10) & 0x7;
						//int grm = op & 0x3;
						int ga = (op >> 3) & 0x7;

						int greg = (bank << 3) | reg;

						// sign extend offset
						if (s && (offset & 0x4000))
							offset |= 0xffffc000;

						switch (le)
						{
							case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset), REG_NAMES[greg]); break;
							case 1: b += sprintf(b, "%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, s, offset)); break;
							case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset)); break;
							case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset)); break;
						}
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
				case 0x00:                                      // Format 2: Move || Local
				{
					b += sprintf(b, "move||local <TODO>");
					break;
				}
				case 0x01:                                      // Format 3: Field Move || Local
				{
					b += sprintf(b, "field move||local <TODO>");
					break;
				}
				case 0x02: case 0x03:                           // Format 6: Non-D DU || Local
				{
					int d = (op >> 32) & 0x7;
					int s = (op & (1 << 28));
					int size = (op >> 29) & 0x3;
					int le = ((op >> 16) & 2) | ((op >> 31) & 1);
					int la = (op >> 25) & 0x7;
					int limx = op & 0x7;

					int reg = (4 << 3) | d;

					switch (le)
					{
						case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx), REG_NAMES[reg]); break;
						case 1: b += sprintf(b, "%s = %s", REG_NAMES[reg], format_address_mode(lmode, la, s, limx)); break;
						case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx)); break;
						case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx)); break;
					}
					break;
				}
				case 0x10: case 0x11: case 0x12: case 0x13:     // Format 4: Local (Long Offset)
				{
					int d = (op >> 32) & 0x7;
					int bank = (op >> 18) & 0xf;
					int s = (op & (1 << 28));
					int size = (op >> 29) & 0x3;
					int le = ((op >> 16) & 2) | ((op >> 31) & 1);
					int la = (op >> 25) & 0x7;

					int reg = (bank << 3) | d;

					UINT16 offset;
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

					switch (le)
					{
						case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset), REG_NAMES[reg]); break;
						case 1: b += sprintf(b, "%s = %s", REG_NAMES[reg], format_address_mode(lmode, la, s, offset)); break;
						case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset)); break;
						case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset)); break;
					}
					break;
				}
				case 0x14: case 0x15: case 0x16: case 0x17:     // Format 1: Double Parallel
				case 0x18: case 0x19: case 0x1a: case 0x1b:
				case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				{
					int la = (op >> 25) & 0x7;
					int ga = (op >> 3) & 0x7;
					int local_imx = op & 0x7;
					int global_imx = (op >> 22) & 0x7;
					int local_s = (op & (1 << 28));
					int global_s = (op & (1 << 6));
					int local_size = (op >> 29) & 0x3;
					int global_size = (op >> 7) & 0x3;
					int local_le = ((op >> 20) & 2) | ((op >> 31) & 1);
					int global_le = ((op >> 16) & 2) | ((op >> 9) & 1);
					int gbank = (op >> 18) & 0x7;
					int reg = (op >> 10) & 0x7;
					int d = (op >> 32) & 0x7;

					int greg = (gbank << 3) | reg;
					int lreg = (4 << 3) | d;

					// local transfer
					switch (local_le)
					{
						case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx), REG_NAMES[lreg]); break;
						case 1: b += sprintf(b, "%s = %s", REG_NAMES[lreg], format_address_mode(lmode, la, local_s, local_imx)); break;
						case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[lreg], TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx)); break;
						case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[lreg], TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx)); break;
					}

					print(", ");

					// global transfer
					switch (global_le)
					{
						case 0: b += sprintf(b, "&%s%s = %s", TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx), REG_NAMES[greg]); break;
						case 1: b += sprintf(b, "%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, global_s, global_imx)); break;
						case 2: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx)); break;
						case 3: b += sprintf(b, "%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx)); break;
					}
					break;
				}
			}
		}
	}

	if (!is_nop)
		print(" || %s", buffer);
}

static void format_alu_op(int aluop, int a, const char *dst_text, const char *a_text, const char *b_text, const char *c_text)
{
	if (a)      // arithmetic
	{
		int bits = (aluop & 1) | ((aluop >> 1) & 2) | ((aluop >> 2) & 4) | ((aluop >> 3) & 8);
		switch (bits)
		{
			case 1:     print("%s = %s - %s<1<", dst_text, a_text, b_text); break;
			case 2:     print("%s = %s + %s<0<", dst_text, a_text, b_text); break;
			case 3:     print("%s = %s - %s", dst_text, a_text, c_text); break;
			case 4:     print("%s = %s - %s>1>", dst_text, a_text, b_text); break;
			case 5:     print("%s = %s - %s", dst_text, a_text, b_text); break;
			case 6:     print("?"); break;
			case 7:     print("%s = %s - %s>0>", dst_text, a_text, b_text); break;
			case 8:     print("%s = %s + %s>0>", dst_text, a_text, b_text); break;
			case 9:     print("?"); break;
			case 10:    print("%s = %s + %s", dst_text, a_text, b_text); break;
			case 11:    print("%s = %s + %s>1>", dst_text, a_text, b_text); break;
			case 12:    print("%s = %s + %s", dst_text, a_text, c_text); break;
			case 13:    print("%s = %s - %s<0<", dst_text, a_text, b_text); break;
			case 14:    print("%s = %s + %s<1<", dst_text, a_text, b_text); break;
			case 15:    print("%s = field %s + %s", dst_text, a_text, b_text); break;
		}
	}
	else        // boolean
	{
		switch (aluop)
		{
			case 0xaa:      // A & B & C | A & ~B & C | A & B & ~C | A & ~B & ~C       = A
				print("%s = %s", dst_text, a_text);
				break;

			case 0x55:      // ~A & B & C | ~A & ~B & C | ~A & B & ~C | ~A & ~B & ~C   = ~A
				print("%s = ~%s", dst_text, a_text);
				break;

			case 0xcc:      // A & B & C | ~A & B & C | A & B & ~C | ~A & B & ~C       = B
				print("%s = %s", dst_text, b_text);
				break;

			case 0x33:      // A & ~B & C | ~A & ~B & C | A & ~B & ~C | ~A & ~B & ~C   = ~B
				print("%s = %s", dst_text, b_text);
				break;

			case 0xf0:      // A & B & C | ~A & B & C | A & ~B & C | ~A & ~B & C       = C
				print("%s = %s", dst_text, c_text);
				break;

			case 0x0f:      // A & B & ~C | ~A & B & ~C | A & ~B & ~C | ~A & ~B & ~C   = ~C
				print("%s = ~%s", dst_text, c_text);
				break;

			case 0x80:      // A & B & C
				print("%s = %s & %s & %s", dst_text, a_text, b_text, c_text);
				break;

			case 0x88:      // A & B & C | A & B & ~C                                  = A & B
				print("%s = %s & %s", dst_text, a_text, b_text);
				break;

			case 0xa0:      // A & B & C | A & ~B & C                                  = A & C
				print("%s = %s & %s", dst_text, a_text, c_text);
				break;

			case 0xc0:      // A & B & C | ~A & B & C                                  = B & C
				print("%s = %s & %s", dst_text, b_text, c_text);
				break;

			case 0xea:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C |  A & ~B & ~C                             = A | C
				print("%s = %s | %s", dst_text, a_text, c_text);
				break;

			case 0xee:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C | ~A &  B & ~C |  A & ~B & ~C              = A | B
				print("%s = %s | %s", dst_text, a_text, b_text);
				break;

			case 0x44:      // ~A &  B &  C | ~A &  B & ~C                             = ~A & B
				print("%s = ~%s & %s", dst_text, a_text, b_text);
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
		case 0x7:           // Six-operand
		{
			print("A: six operand <TODO>");
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
				UINT64 parallel_xfer = (op & U64(0x0000007fffffffff));

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
					case 1:     // Base set ALU (5-bit immediate)
					{
						UINT64 parallel_xfer = (op & U64(0x0000007fffffffff));

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2imm = (op >> 39) & 0x1f;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int adstbank;
						int as1bank;

						if ((op & 0x0101c000) == 0x00004000)
						{
							adstbank = (op >> 18) & 0xf;
							as1bank = (op >> 6) & 0xf;
						}
						else
						{
							adstbank = 4;
							as1bank = 4;
						}

						int s1reg = src1 | (as1bank << 3);
						int dreg = dst | (adstbank << 3);
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

					case 2:     // Base set ALU (reg src2)
					{
						UINT64 parallel_xfer = (op & U64(0x0000007fffffffff));

						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int src2 = (op >> 39) & 7;
						int cl = (op >> 60) & 7;
						int aluop = (op >> 51) & 0xff;
						int a = (op >> 59) & 1;

						int adstbank;
						int as1bank;

						if ((op & 0x0101c000) == 0x00004000)
						{
							adstbank = (op >> 18) & 0xf;
							as1bank = (op >> 6) & 0xf;
						}
						else
						{
							adstbank = 4;
							as1bank = 4;
						}

						int s1reg = src1 | (as1bank << 3);
						int s2reg = src2 | (0x4 << 3);
						int dstcreg = dst | (0x4 << 3);
						int dreg = dst | (adstbank << 3);

						sprintf(dst_text, "%s", REG_NAMES[dreg]);
						switch (cl)
						{
							case 0:
								sprintf(a_text, "%s", REG_NAMES[s2reg]);
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
								sprintf(c_text, "%%%s", REG_NAMES[s2reg]);
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

					case 3:     // Base set ALU (32-bit immediate)
					{
						int dst = (op >> 48) & 7;
						int src1 = (op >> 45) & 7;
						int dstbank = (op >> 39) & 0xf;
						int s1bank = (op >> 36) & 7;
						int cond = (op >> 32) & 0xf;
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

						print("%s", CONDITION_CODES[cond]);

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
