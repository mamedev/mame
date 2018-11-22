// license:BSD-3-Clause
// copyright-holders:Ville Linde
// TMS32082 PP Disassembler

#include "emu.h"
#include "dis_pp.h"

char const *const tms32082_pp_disassembler::REG_NAMES[128] =
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

char const *const tms32082_pp_disassembler::CONDITION_CODES[16] =
{
	"",         "[p] ",     "[ls] ",    "[hi] ",
	"[lt] ",    "[le] ",    "[ge] ",    "[gt] ",
	"[hs] ",    "[lo] ",    "[eq] ",    "[ne] ",
	"[v] ",     "[nv] ",    "[n] ",     "[nn] "
};

char const *const tms32082_pp_disassembler::TRANSFER_SIZE[4] =
{
	"b:", "h:", "w:", ""
};

std::string tms32082_pp_disassembler::format_address_mode(int mode, int areg, int s, int limx)
{
	switch (mode)
	{
		case 0x4: return util::string_format("*(a%d++=x%d)", areg, limx);
		case 0x5: return util::string_format("*(a%d--=x%d)", areg, limx);
		case 0x6: return util::string_format("*(a%d++0x%04X)", areg, limx);
		case 0x7: return util::string_format("*(a%d--0x%04X)", areg, limx);
		case 0x8: return util::string_format("*(a%d+x%d)", areg, limx);
		case 0x9: return util::string_format("*(a%d-x%d)", areg, limx);
		case 0xa: return util::string_format("*(a%d+0x%04X)", areg, limx);
		case 0xb: return util::string_format("*(a%d-0x%04X)", areg, limx);
		case 0xc: return util::string_format("*(a%d+=x%d)", areg, limx);
		case 0xd: return util::string_format("*(a%d-=x%d)", areg, limx);
		case 0xe: return util::string_format("*(a%d+=0x%04X)", areg, limx);
		case 0xf: return util::string_format("*(a%d-=0x%04X)", areg, limx);
	}

	return "";
}

void tms32082_pp_disassembler::format_transfer(uint64_t op)
{
	std::string buffer;
	int lmode = (op >> 35) & 0xf;
	int gmode = (op >> 13) & 0xf;

	bool is_nop = false;

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

					buffer += util::string_format("%s", CONDITION_CODES[cond]);
					buffer += util::string_format("%s = %s", REG_NAMES[dreg], REG_NAMES[sreg]);
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

					buffer += util::string_format("%s", CONDITION_CODES[cond]);
					buffer += util::string_format("%s = [%s%d]%s", REG_NAMES[dreg], TRANSFER_SIZE[size], itm, REG_NAMES[sreg]);
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
						buffer += util::string_format("%s", CONDITION_CODES[cond]);
						buffer += util::string_format("%s = %s", REG_NAMES[dreg], REG_NAMES[sreg]);
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

						buffer += util::string_format("%s", CONDITION_CODES[cond]);

						switch (le)
						{
							case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx), REG_NAMES[greg]); break;
							case 1: buffer += util::string_format("%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, s, gimx)); break;
							case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx)); break;
							case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, gimx)); break;
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
							case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset), REG_NAMES[greg]); break;
							case 1: buffer += util::string_format("%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, s, offset)); break;
							case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset)); break;
							case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[size], format_address_mode(gmode, ga, s, offset)); break;
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
					buffer += util::string_format("move||local <TODO>");
					break;
				}
				case 0x01:                                      // Format 3: Field Move || Local
				{
					buffer += util::string_format("field move||local <TODO>");
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
						case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx), REG_NAMES[reg]); break;
						case 1: buffer += util::string_format("%s = %s", REG_NAMES[reg], format_address_mode(lmode, la, s, limx)); break;
						case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx)); break;
						case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, limx)); break;
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

					uint16_t offset;
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
						case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset), REG_NAMES[reg]); break;
						case 1: buffer += util::string_format("%s = %s", REG_NAMES[reg], format_address_mode(lmode, la, s, offset)); break;
						case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset)); break;
						case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[reg], TRANSFER_SIZE[size], format_address_mode(lmode, la, s, offset)); break;
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
						case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx), REG_NAMES[lreg]); break;
						case 1: buffer += util::string_format("%s = %s", REG_NAMES[lreg], format_address_mode(lmode, la, local_s, local_imx)); break;
						case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[lreg], TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx)); break;
						case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[lreg], TRANSFER_SIZE[local_size], format_address_mode(lmode, la, local_s, local_imx)); break;
					}

					util::stream_format(*output, ", ");

					// global transfer
					switch (global_le)
					{
						case 0: buffer += util::string_format("&%s%s = %s", TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx), REG_NAMES[greg]); break;
						case 1: buffer += util::string_format("%s = %s", REG_NAMES[greg], format_address_mode(gmode, ga, global_s, global_imx)); break;
						case 2: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx)); break;
						case 3: buffer += util::string_format("%s = &%s%s", REG_NAMES[greg], TRANSFER_SIZE[global_size], format_address_mode(gmode, ga, global_s, global_imx)); break;
					}
					break;
				}
			}
		}
	}

	if (!is_nop)
		util::stream_format(*output, " || %s", buffer);
}

void tms32082_pp_disassembler::format_alu_op(int aluop, int a, const char *dst_text, const char *a_text, const char *b_text, const char *c_text)
{
	if (a)      // arithmetic
	{
		int bits = (aluop & 1) | ((aluop >> 1) & 2) | ((aluop >> 2) & 4) | ((aluop >> 3) & 8);
		switch (bits)
		{
			case 1:     util::stream_format(*output, "%s = %s - %s<1<", dst_text, a_text, b_text); break;
			case 2:     util::stream_format(*output, "%s = %s + %s<0<", dst_text, a_text, b_text); break;
			case 3:     util::stream_format(*output, "%s = %s - %s", dst_text, a_text, c_text); break;
			case 4:     util::stream_format(*output, "%s = %s - %s>1>", dst_text, a_text, b_text); break;
			case 5:     util::stream_format(*output, "%s = %s - %s", dst_text, a_text, b_text); break;
			case 6:     util::stream_format(*output, "?"); break;
			case 7:     util::stream_format(*output, "%s = %s - %s>0>", dst_text, a_text, b_text); break;
			case 8:     util::stream_format(*output, "%s = %s + %s>0>", dst_text, a_text, b_text); break;
			case 9:     util::stream_format(*output, "?"); break;
			case 10:    util::stream_format(*output, "%s = %s + %s", dst_text, a_text, b_text); break;
			case 11:    util::stream_format(*output, "%s = %s + %s>1>", dst_text, a_text, b_text); break;
			case 12:    util::stream_format(*output, "%s = %s + %s", dst_text, a_text, c_text); break;
			case 13:    util::stream_format(*output, "%s = %s - %s<0<", dst_text, a_text, b_text); break;
			case 14:    util::stream_format(*output, "%s = %s + %s<1<", dst_text, a_text, b_text); break;
			case 15:    util::stream_format(*output, "%s = field %s + %s", dst_text, a_text, b_text); break;
		}
	}
	else        // boolean
	{
		switch (aluop)
		{
			case 0xaa:      // A & B & C | A & ~B & C | A & B & ~C | A & ~B & ~C       = A
				util::stream_format(*output, "%s = %s", dst_text, a_text);
				break;

			case 0x55:      // ~A & B & C | ~A & ~B & C | ~A & B & ~C | ~A & ~B & ~C   = ~A
				util::stream_format(*output, "%s = ~%s", dst_text, a_text);
				break;

			case 0xcc:      // A & B & C | ~A & B & C | A & B & ~C | ~A & B & ~C       = B
				util::stream_format(*output, "%s = %s", dst_text, b_text);
				break;

			case 0x33:      // A & ~B & C | ~A & ~B & C | A & ~B & ~C | ~A & ~B & ~C   = ~B
				util::stream_format(*output, "%s = %s", dst_text, b_text);
				break;

			case 0xf0:      // A & B & C | ~A & B & C | A & ~B & C | ~A & ~B & C       = C
				util::stream_format(*output, "%s = %s", dst_text, c_text);
				break;

			case 0x0f:      // A & B & ~C | ~A & B & ~C | A & ~B & ~C | ~A & ~B & ~C   = ~C
				util::stream_format(*output, "%s = ~%s", dst_text, c_text);
				break;

			case 0x80:      // A & B & C
				util::stream_format(*output, "%s = %s & %s & %s", dst_text, a_text, b_text, c_text);
				break;

			case 0x88:      // A & B & C | A & B & ~C                                  = A & B
				util::stream_format(*output, "%s = %s & %s", dst_text, a_text, b_text);
				break;

			case 0xa0:      // A & B & C | A & ~B & C                                  = A & C
				util::stream_format(*output, "%s = %s & %s", dst_text, a_text, c_text);
				break;

			case 0xc0:      // A & B & C | ~A & B & C                                  = B & C
				util::stream_format(*output, "%s = %s & %s", dst_text, b_text, c_text);
				break;

			case 0xea:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C |  A & ~B & ~C                             = A | C
				util::stream_format(*output, "%s = %s | %s", dst_text, a_text, c_text);
				break;

			case 0xee:      //  A &  B &  C | ~A &  B &  C |  A & ~B &  C |
							//  A &  B & ~C | ~A &  B & ~C |  A & ~B & ~C              = A | B
				util::stream_format(*output, "%s = %s | %s", dst_text, a_text, b_text);
				break;

			case 0x44:      // ~A &  B &  C | ~A &  B & ~C                             = ~A & B
				util::stream_format(*output, "%s = ~%s & %s", dst_text, a_text, b_text);
				break;

			default:
				util::stream_format(*output, "%s = b%02X(%s, %s, %s)", dst_text, aluop, a_text, b_text, c_text);
				break;
		}
	}
}

u32 tms32082_pp_disassembler::opcode_alignment() const
{
	return 8;
}

offs_t tms32082_pp_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	output = &stream;
	uint32_t flags = 0;

	uint64_t op = opcodes.r64(pc);

	switch (op >> 60)
	{
		case 0x6:
		case 0x7:           // Six-operand
		{
			util::stream_format(*output, "A: six operand <TODO>");
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
			if ((op & 0xfaa8100000000000U) == 0x8800000000000000U)
			{
				int operation = (op >> 39) & 0x1f;
				uint64_t parallel_xfer = (op & 0x0000007fffffffffU);

				switch (operation)
				{
					case 0x00: util::stream_format(*output, "nop"); break;
					case 0x02: util::stream_format(*output, "eint"); break;
					case 0x03: util::stream_format(*output, "dint"); break;
					default:   util::stream_format(*output, "<reserved>"); break;
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
						uint64_t parallel_xfer = (op & 0x0000007fffffffffU);

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
						uint64_t parallel_xfer = (op & 0x0000007fffffffffU);

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
						uint32_t imm32 = (uint32_t)(op);

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

						util::stream_format(*output, "%s", CONDITION_CODES[cond]);

						format_alu_op(aluop, a, dst_text, a_text, b_text, c_text);
						break;
					}
				}
			}
			break;
		}

		default:
			util::stream_format(*output, "??? (%02X)", (uint32_t)(op >> 60));
			break;
	}

	return 8 | flags | SUPPORTED;
}
