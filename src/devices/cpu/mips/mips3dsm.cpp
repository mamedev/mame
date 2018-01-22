// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3dsm.c
    Disassembler for the portable MIPS 3 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"
#include "mips3dsm.h"

#define USE_ABI_REG_NAMES (1)

#if USE_ABI_REG_NAMES
const char *const mips3_disassembler::reg[32] =
{
	"$0",   "$at",  "$v0",  "$v1",  "$a0",  "$a1",  "$a2",  "$a3",
	"$t0",  "$t1",  "$t2",  "$t3",  "$t4",  "$t5",  "$t6",  "$t7",
	"$s0",  "$s1",  "$s2",  "$s3",  "$s4",  "$s5",  "$s6",  "$s7",
	"$t8",  "$t9",  "$k0",  "$k1",  "$gp",  "$sp",  "$fp",  "$ra"
};
#else
const char *const mips3_disassembler::reg[32] =
{
	"r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
	"r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
	"r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
	"r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};
#endif

const char *const mips3_disassembler::cacheop[32] =
{
	"I_Invd", "D_WBInvd", "Unknown 2", "Unknown 3", "I_IndexLoadTag", "D_IndexLoadTag", "Unknown 6", "Unknown 7",
	"I_IndexStoreTag", "D_IndexStoreTag", "Unknown 10", "Unknown 11", "Unknown 12", "D_CreateDirtyExcl", "Unknown 14", "Unknown 15",
	"I_HitInvalid", "D_HitInvalid", "Unknown 18", "Unknown 19", "I_Fill", "D_HitWBInvalid", "Unknown 22", "Unknown 23",
	"I_HitWB", "D_HitWB", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29", "Unknown 30", "Unknown 31"
};


const char *const mips3_disassembler::cpreg[4][32] =
{
	{
		"Index","Random","EntryLo0","EntryLo1","Context","PageMask","Wired","Error",
		"BadVAddr","Count","EntryHi","Compare","SR","Cause","EPC","PRId",
		"Config","LLAddr","WatchLo","WatchHi","XContext","cpr21","cpr22","cpr23",
		"cpr24","cpr25","ECC","CacheError","TagLo","TagHi","ErrorEPC","cpr31"
	},
	{
		"f0",   "f1",   "f2",   "f3",   "f4",   "f5",   "f6",   "f7",
		"f8",   "f9",   "f10",  "f11",  "f12",  "f13",  "f14",  "f15",
		"f16",  "f17",  "f18",  "f19",  "f20",  "f21",  "f22",  "f23",
		"f24",  "f25",  "f26",  "f27",  "f28",  "f29",  "f30",  "f31"
	},
	{
		"cpr0", "cpr1", "cpr2", "cpr3", "cpr4", "cpr5", "cpr6", "cpr7",
		"cpr8", "cpr9", "cpr10","cpr11","cpr12","cpr13","cpr14","cpr15",
		"cpr16","cpr17","cpr18","cpr19","cpr20","cpr21","cpr22","cpr23",
		"cpr24","cpr25","cpr26","cpr27","cpr28","cpr29","cpr30","cpr31"
	},
	{
		"cpr0", "cpr1", "cpr2", "cpr3", "cpr4", "cpr5", "cpr6", "cpr7",
		"cpr8", "cpr9", "cpr10","cpr11","cpr12","cpr13","cpr14","cpr15",
		"cpr16","cpr17","cpr18","cpr19","cpr20","cpr21","cpr22","cpr23",
		"cpr24","cpr25","cpr26","cpr27","cpr28","cpr29","cpr30","cpr31"
	}
};


const char *const mips3_disassembler::ccreg[4][32] =
{
	{
		"ccr0", "ccr1", "ccr2", "ccr3", "ccr4", "ccr5", "ccr6", "ccr7",
		"ccr8", "ccr9", "ccr10","ccr11","ccr12","ccr13","ccr14","ccr15",
		"ccr16","ccr17","ccr18","ccr19","ccr20","ccr21","ccr22","ccr23",
		"ccr24","ccr25","ccr26","ccr27","ccr28","ccr29","ccr30","ccr31"
	},
	{
		"ccr0", "ccr1", "ccr2", "ccr3", "ccr4", "ccr5", "ccr6", "ccr7",
		"ccr8", "ccr9", "ccr10","ccr11","ccr12","ccr13","ccr14","ccr15",
		"ccr16","ccr17","ccr18","ccr19","ccr20","ccr21","ccr22","ccr23",
		"ccr24","ccr25","ccr26","ccr27","ccr28","ccr29","ccr30","ccr31"
	},
	{
		"ccr0", "ccr1", "ccr2", "ccr3", "ccr4", "ccr5", "ccr6", "ccr7",
		"ccr8", "ccr9", "ccr10","ccr11","ccr12","ccr13","ccr14","ccr15",
		"ccr16","ccr17","ccr18","ccr19","ccr20","ccr21","ccr22","ccr23",
		"ccr24","ccr25","ccr26","ccr27","ccr28","ccr29","ccr30","ccr31"
	},
	{
		"ccr0", "ccr1", "ccr2", "ccr3", "ccr4", "ccr5", "ccr6", "ccr7",
		"ccr8", "ccr9", "ccr10","ccr11","ccr12","ccr13","ccr14","ccr15",
		"ccr16","ccr17","ccr18","ccr19","ccr20","ccr21","ccr22","ccr23",
		"ccr24","ccr25","ccr26","ccr27","ccr28","ccr29","ccr30","ccr31"
	}
};


/***************************************************************************
    CODE CODE
***************************************************************************/

inline std::string mips3_disassembler::signed_16bit(int16_t val)
{
	if (val < 0)
		return util::string_format("-$%x", -val);
	else
		return util::string_format("$%x", val);
}

uint32_t mips3_disassembler::dasm_cop0(uint32_t pc, uint32_t op, std::ostream &stream)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	uint32_t flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  util::stream_format(stream, "mfc0   %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x01:  util::stream_format(stream, "dmfc0  %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x02:  util::stream_format(stream, "cfc0   %s,%s", reg[rt], ccreg[0][rd]);                 break;
		case 0x04:  util::stream_format(stream, "mtc0   %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x05:  util::stream_format(stream, "dmtc0  %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x06:  util::stream_format(stream, "ctc0   %s,%s", reg[rt], ccreg[0][rd]);                 break;
		case 0x08:  /* BC */
			switch (rt)
			{
				case 0x00:  util::stream_format(stream, "bc0f   $%08x", pc + 4 + ((int16_t)op << 2));     break;
				case 0x01:  util::stream_format(stream, "bc0t   $%08x", pc + 4 + ((int16_t)op << 2));     break;
				case 0x02:  util::stream_format(stream, "bc0fl [invalid]");                             break;
				case 0x03:  util::stream_format(stream, "bc0tl [invalid]");                             break;
				default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                  break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:  util::stream_format(stream, "tlbr");                                            break;
				case 0x02:  util::stream_format(stream, "tlbwi");                                           break;
				case 0x06:  util::stream_format(stream, "tlbwr");                                           break;
				case 0x08:  util::stream_format(stream, "tlbp");                                            break;
				case 0x10:  util::stream_format(stream, "rfe"); flags = STEP_OUT;                  break;
				case 0x18:  util::stream_format(stream, "eret [invalid]");                                  break;
				default:    util::stream_format(stream, "cop0  $%07x", op & 0x01ffffff);                    break;
			}
			break;
		default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                              break;
	}
	return flags;
}

uint32_t mips3_disassembler::dasm_cop1(uint32_t pc, uint32_t op, std::ostream &stream)
{
	const char *const format_table[] =
	{
		"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
		"s","d","?","?","w","l","?","?","?","?","?","?","?","?","?","?"
	};
	const char *fmt = format_table[(op >> 21) & 31];
	int ft = (op >> 16) & 31;
	int fs = (op >> 11) & 31;
	int fd = (op >> 6) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	uint32_t flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  util::stream_format(stream, "mfc1   %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x01:  util::stream_format(stream, "dmfc1  %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x02:  util::stream_format(stream, "cfc1   %s,%s", reg[rt], ccreg[1][rd]);                     break;
		case 0x04:  util::stream_format(stream, "mtc1   %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x05:  util::stream_format(stream, "dmtc1  %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x06:  util::stream_format(stream, "ctc1   %s,%s", reg[rt], ccreg[1][rd]);                     break;
		case 0x08:  /* BC */
			switch (rt & 3)
			{
				case 0x00:  util::stream_format(stream, "bc1f   $%08x,%d", pc + 4 + ((int16_t)op << 2), (op >> 18) & 7);      break;
				case 0x01:  util::stream_format(stream, "bc1t   $%08x,%d", pc + 4 + ((int16_t)op << 2), (op >> 18) & 7);      break;
				case 0x02:  util::stream_format(stream, "bc1fl  $%08x,%d", pc + 4 + ((int16_t)op << 2), (op >> 18) & 7); flags = STEP_OVER | step_over_extra(1); break;
				case 0x03:  util::stream_format(stream, "bc1tl  $%08x,%d", pc + 4 + ((int16_t)op << 2), (op >> 18) & 7); flags = STEP_OVER | step_over_extra(1); break;
			}
			break;
		default:    /* COP */
			switch (op & 0x3f)
			{
				case 0x00:  util::stream_format(stream, "add.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x01:  util::stream_format(stream, "sub.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x02:  util::stream_format(stream, "mul.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x03:  util::stream_format(stream, "div.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x04:  util::stream_format(stream, "sqrt.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x05:  util::stream_format(stream, "abs.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x06:  util::stream_format(stream, "mov.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x07:  util::stream_format(stream, "neg.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x08:  util::stream_format(stream, "round.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x09:  util::stream_format(stream, "trunc.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0a:  util::stream_format(stream, "ceil.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x0b:  util::stream_format(stream, "floor.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0c:  util::stream_format(stream, "round.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0d:  util::stream_format(stream, "trunc.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0e:  util::stream_format(stream, "ceil.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x0f:  util::stream_format(stream, "floor.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x11:  util::stream_format(stream, "mov%c.%s  %s,%s,%d", ((op >> 16) & 1) ? 't' : 'f', fmt, cpreg[1][fd], cpreg[1][fs], (op >> 18) & 7);   break;
				case 0x12:  util::stream_format(stream, "movz.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], reg[rt]);     break;
				case 0x13:  util::stream_format(stream, "movn.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], reg[rt]);     break;
				case 0x15:  util::stream_format(stream, "recip.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x16:  util::stream_format(stream, "rsqrt.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x20:  util::stream_format(stream, "cvt.s.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x21:  util::stream_format(stream, "cvt.d.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x24:  util::stream_format(stream, "cvt.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x25:  util::stream_format(stream, "cvt.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x30:  util::stream_format(stream, "c.f.%s  %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x31:  util::stream_format(stream, "c.un.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x32:  util::stream_format(stream, "c.eq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x33:  util::stream_format(stream, "c.ueq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x34:  util::stream_format(stream, "c.olt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x35:  util::stream_format(stream, "c.ult.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x36:  util::stream_format(stream, "c.ole.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x37:  util::stream_format(stream, "c.ule.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x38:  util::stream_format(stream, "c.sf.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x39:  util::stream_format(stream, "c.ngle.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);break;
				case 0x3a:  util::stream_format(stream, "c.seq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3b:  util::stream_format(stream, "c.ngl.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3c:  util::stream_format(stream, "c.lt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x3d:  util::stream_format(stream, "c.nge.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3e:  util::stream_format(stream, "c.le.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x3f:  util::stream_format(stream, "c.ngt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				default:    util::stream_format(stream, "cop1   $%07x", op & 0x01ffffff);                                   break;
			}
			break;
	}
	return flags;
}

uint32_t mips3_disassembler::dasm_cop1x(uint32_t pc, uint32_t op, std::ostream &stream)
{
	const char *const format3_table[] =
	{
		"s","d","?","?","w","l","?","?"
	};
	const char *fmt3 = format3_table[op & 7];
	int fr = (op >> 21) & 31;
	int ft = (op >> 16) & 31;
	int fs = (op >> 11) & 31;
	int fd = (op >> 6) & 31;
	int rs = (op >> 21) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	uint32_t flags = 0;

	switch (op & 0x3f)
	{
		case 0x00:  util::stream_format(stream, "lwxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x01:  util::stream_format(stream, "ldxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x08:  util::stream_format(stream, "swxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x09:  util::stream_format(stream, "sdxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x0f:  util::stream_format(stream, "prefx   %d,%s(%s)", rd, reg[rt], reg[rs]);                         break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:  util::stream_format(stream, "madd.%s  %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:  util::stream_format(stream, "msub.%s  %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:  util::stream_format(stream, "nmadd.%s %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:  util::stream_format(stream, "nmsub.%s %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		default:    util::stream_format(stream, "cop1   $%07x", op & 0x01ffffff);                                   break;
	}
	return flags;
}

uint32_t mips3_disassembler::dasm_cop2(uint32_t pc, uint32_t op, std::ostream &stream)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	uint32_t flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  util::stream_format(stream, "mfc2   %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x01:  util::stream_format(stream, "dmfc2  %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x02:  util::stream_format(stream, "cfc2   %s,%s", reg[rt], ccreg[2][rd]);                 break;
		case 0x04:  util::stream_format(stream, "mtc2   %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x05:  util::stream_format(stream, "dmtc2  %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x06:  util::stream_format(stream, "ctc2   %s,%s", reg[rt], ccreg[2][rd]);                 break;
		case 0x08:  /* BC */
			switch (rt)
			{
				case 0x00:  util::stream_format(stream, "bc2f   $%08x", pc + 4 + ((int16_t)op << 2));     break;
				case 0x01:  util::stream_format(stream, "bc2t   $%08x", pc + 4 + ((int16_t)op << 2));     break;
				case 0x02:  util::stream_format(stream, "bc2fl [invalid]");                             break;
				case 0x03:  util::stream_format(stream, "bc2tl [invalid]");                             break;
				default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                  break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */
			util::stream_format(stream, "cop2   $%07x", op & 0x01ffffff);
			break;
		default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                                  break;
	}
	return flags;
}

u32 mips3_disassembler::opcode_alignment() const
{
	return 4;
}

offs_t mips3_disassembler::dasm_one(std::ostream &stream, offs_t pc, u32 op)
{
	int rs = (op >> 21) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int shift = (op >> 6) & 31;
	uint32_t flags = 0;

	switch (op >> 26)
	{
		case 0x00:  /* SPECIAL */
			switch (op & 63)
			{
				case 0x00:  if (op == 0)
							util::stream_format(stream, "nop");
							else
							util::stream_format(stream, "sll    %s,%s,%d", reg[rd], reg[rt], shift);
					break;
				case 0x01:  util::stream_format(stream, "mov%c   %s,%s,%d", ((op >> 16) & 1) ? 't' : 'f', reg[rd], reg[rs], (op >> 18) & 7); break;
				case 0x02:  util::stream_format(stream, "srl    %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x03:  util::stream_format(stream, "sra    %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x04:  util::stream_format(stream, "sllv   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x06:  util::stream_format(stream, "srlv   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x07:  util::stream_format(stream, "srav   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x08:  util::stream_format(stream, "jr     %s", reg[rs]); if (rs == 31) flags = STEP_OUT; break;
				case 0x09:  if (rd == 31)
							util::stream_format(stream, "jalr   %s", reg[rs]);
							else
							util::stream_format(stream, "jalr   %s,%s", reg[rs], reg[rd]);
					flags = STEP_OVER | step_over_extra(1);
					break;
				case 0x0a:  util::stream_format(stream, "movz   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x0b:  util::stream_format(stream, "movn   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x0c:  util::stream_format(stream, "syscall"); flags = STEP_OVER;                 break;
				case 0x0d:  util::stream_format(stream, "break"); flags = STEP_OVER;                   break;
				case 0x0f:  util::stream_format(stream, "sync");                                                break;
				case 0x10:  util::stream_format(stream, "mfhi   %s", reg[rd]);                                  break;
				case 0x11:  util::stream_format(stream, "mthi   %s", reg[rs]);                                  break;
				case 0x12:  util::stream_format(stream, "mflo   %s", reg[rd]);                                  break;
				case 0x13:  util::stream_format(stream, "mtlo   %s", reg[rs]);                                  break;
				case 0x14:  util::stream_format(stream, "dsllv  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x16:  util::stream_format(stream, "dsrlv  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x17:  util::stream_format(stream, "dsrav  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x18:  util::stream_format(stream, "mult   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x19:  util::stream_format(stream, "multu  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1a:  util::stream_format(stream, "div    %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1b:  util::stream_format(stream, "divu   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1c:  util::stream_format(stream, "dmult  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1d:  util::stream_format(stream, "dmultu %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1e:  util::stream_format(stream, "ddiv   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1f:  util::stream_format(stream, "ddivu  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x20:  util::stream_format(stream, "add    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x21:  util::stream_format(stream, "addu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x22:  util::stream_format(stream, "sub    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x23:  util::stream_format(stream, "subu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x24:  util::stream_format(stream, "and    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x25:  util::stream_format(stream, "or     %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x26:  util::stream_format(stream, "xor    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x27:  util::stream_format(stream, "nor    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2a:  util::stream_format(stream, "slt    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2b:  util::stream_format(stream, "sltu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2c:  util::stream_format(stream, "dadd   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2d:  util::stream_format(stream, "daddu  %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2e:  util::stream_format(stream, "dsub   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2f:  util::stream_format(stream, "dsubu  %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x30:  util::stream_format(stream, "tge    %s,%s", reg[rs], reg[rt]); flags = STEP_OVER; break;
				case 0x31:  util::stream_format(stream, "tgeu   %s,%s", reg[rs], reg[rt]); flags = STEP_OVER; break;
				case 0x32:  util::stream_format(stream, "tlt    %s,%s", reg[rs], reg[rt]); flags = STEP_OVER; break;
				case 0x33:  util::stream_format(stream, "tltu   %s,%s", reg[rs], reg[rt]); flags = STEP_OVER; break;
				case 0x34:  util::stream_format(stream, "teq    %s,%s", reg[rs], reg[rt]); flags = STEP_OVER; break;
				case 0x36:  util::stream_format(stream, "tne    %s,%s", reg[rs], reg[rt]) ;flags = STEP_OVER; break;
				case 0x38:  util::stream_format(stream, "dsll   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3a:  util::stream_format(stream, "dsrl   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3b:  util::stream_format(stream, "dsra   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3c:  util::stream_format(stream, "dsll   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				case 0x3e:  util::stream_format(stream, "dsrl   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				case 0x3f:  util::stream_format(stream, "dsra   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                          break;
			}
			break;

		case 0x01:  /* REGIMM */
			switch ((op >> 16) & 31)
			{
				case 0x00:  util::stream_format(stream, "bltz   %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x01:  util::stream_format(stream, "bgez   %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x02:  util::stream_format(stream, "bltzl  %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x03:  util::stream_format(stream, "bgezl  %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); break;
				case 0x08:  util::stream_format(stream, "tgei   %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x09:  util::stream_format(stream, "tgeiu  %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x0a:  util::stream_format(stream, "tlti   %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x0b:  util::stream_format(stream, "tltiu  %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x0c:  util::stream_format(stream, "teqi   %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x0e:  util::stream_format(stream, "tnei   %s,%s", reg[rs], signed_16bit(op)); flags = STEP_OVER; break;
				case 0x10:  util::stream_format(stream, "bltzal %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); flags = STEP_OVER | step_over_extra(1); break;
				case 0x11:  util::stream_format(stream, "bgezal %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); flags = STEP_OVER | step_over_extra(1); break;
				case 0x12:  util::stream_format(stream, "bltzall %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); flags = STEP_OVER | step_over_extra(1); break;
				case 0x13:  util::stream_format(stream, "bgezall %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2)); flags = STEP_OVER | step_over_extra(1); break;
				default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                          break;
			}
			break;

		case 0x02:  util::stream_format(stream, "j      $%08x", (pc & 0xf0000000) | ((op & 0x03ffffff) << 2));  break;
		case 0x03:  util::stream_format(stream, "jal    $%08x", (pc & 0xf0000000) | ((op & 0x03ffffff) << 2)); flags = STEP_OVER | step_over_extra(1); break;
		case 0x04:  if (rs == 0 && rt == 0)
					util::stream_format(stream, "b      $%08x", pc + 4 + ((int16_t)op << 2));
					else
					util::stream_format(stream, "beq    %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));
			break;
		case 0x05:  util::stream_format(stream, "bne    %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));break;
		case 0x06:  util::stream_format(stream, "blez   %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2));         break;
		case 0x07:  util::stream_format(stream, "bgtz   %s,$%08x", reg[rs], pc + 4 + ((int16_t)op << 2));         break;
		case 0x08:  util::stream_format(stream, "addi   %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x09:  util::stream_format(stream, "addiu  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0a:  util::stream_format(stream, "slti   %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0b:  util::stream_format(stream, "sltiu  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0c:  util::stream_format(stream, "andi   %s,%s,$%04x", reg[rt], reg[rs], (uint16_t)op);            break;
		case 0x0d:  util::stream_format(stream, "ori    %s,%s,$%04x", reg[rt], reg[rs], (uint16_t)op);            break;
		case 0x0e:  util::stream_format(stream, "xori   %s,%s,$%04x", reg[rt], reg[rs], (uint16_t)op);            break;
		case 0x0f:  util::stream_format(stream, "lui    %s,$%04x", reg[rt], (uint16_t)op);                        break;
		case 0x10:  flags = dasm_cop0(pc, op, stream);                                              break;
		case 0x11:  flags = dasm_cop1(pc, op, stream);                                              break;
		case 0x12:  flags = dasm_cop2(pc, op, stream);                                              break;
		case 0x13:  flags = dasm_cop1x(pc, op, stream);                                             break;
		case 0x14:  util::stream_format(stream, "beql   %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));break;
		case 0x15:  util::stream_format(stream, "bnel   %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));break;
		case 0x16:  util::stream_format(stream, "blezl  %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));break;
		case 0x17:  util::stream_format(stream, "bgtzl  %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((int16_t)op << 2));break;
		case 0x18:  util::stream_format(stream, "daddi  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x19:  util::stream_format(stream, "daddiu %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x1a:  util::stream_format(stream, "ldl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x1b:  util::stream_format(stream, "ldr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x1c:  /* IDT-specific opcodes: mad/madu/mul on R4640/4650, msub on RC32364 */
			switch (op & 0x1f)
			{
				case 0: util::stream_format(stream, "mad    %s,%s", reg[rs], reg[rt]); break;
				case 1: util::stream_format(stream, "madu   %s,%s", reg[rs], reg[rt]); break;
				case 2: util::stream_format(stream, "mul    %s,%s,%s", reg[rs], reg[rt], reg[rd]); break;
				case 4: util::stream_format(stream, "msub   %s,%s", reg[rs], reg[rt]); break;
				default:util::stream_format(stream, "dc.l   $%08x [invalid]", op);  break;
			}
			break;
		case 0x20:  util::stream_format(stream, "lb     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x21:  util::stream_format(stream, "lh     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x22:  util::stream_format(stream, "lwl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x23:  util::stream_format(stream, "lw     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x24:  util::stream_format(stream, "lbu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x25:  util::stream_format(stream, "lhu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x26:  util::stream_format(stream, "lwr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x27:  util::stream_format(stream, "lwu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x28:  util::stream_format(stream, "sb     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x29:  util::stream_format(stream, "sh     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2a:  util::stream_format(stream, "swl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2b:  util::stream_format(stream, "sw     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2c:  util::stream_format(stream, "sdl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2d:  util::stream_format(stream, "sdr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2e:  util::stream_format(stream, "swr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2f:  util::stream_format(stream, "cache  %s,%s(%s)", cacheop[rt], reg[rs], signed_16bit(op));                    break;
		case 0x30:  util::stream_format(stream, "ll     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x31:  util::stream_format(stream, "lwc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x32:  util::stream_format(stream, "lwc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x33:  util::stream_format(stream, "pref   $%x,%s(%s)", rt, signed_16bit(op), reg[rs]);            break;
		case 0x34:  util::stream_format(stream, "lld    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x35:  util::stream_format(stream, "ldc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x36:  util::stream_format(stream, "ldc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x37:  util::stream_format(stream, "ld     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x38:  util::stream_format(stream, "sc     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x39:  util::stream_format(stream, "swc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3a:  util::stream_format(stream, "swc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3c:  util::stream_format(stream, "scd    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x3d:  util::stream_format(stream, "sdc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3e:  util::stream_format(stream, "sdc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3f:  util::stream_format(stream, "sd     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		default:    util::stream_format(stream, "dc.l   $%08x [invalid]", op);                                  break;
	}
	return 4 | flags | SUPPORTED;
}

offs_t mips3_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u32 op = opcodes.r32(pc);
	return dasm_one(stream, pc, op);
}
