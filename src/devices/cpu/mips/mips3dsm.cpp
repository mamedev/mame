// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    mips3dsm.c
    Disassembler for the portable MIPS 3 emulator.
    Written by Aaron Giles

***************************************************************************/

#include "emu.h"

#define USE_ABI_REG_NAMES (1)

#if USE_ABI_REG_NAMES
static const char *const reg[32] =
{
	"$0",   "$at",  "$v0",  "$v1",  "$a0",  "$a1",  "$a2",  "$a3",
	"$t0",  "$t1",  "$t2",  "$t3",  "$t4",  "$t5",  "$t6",  "$t7",
	"$s0",  "$s1",  "$s2",  "$s3",  "$s4",  "$s5",  "$s6",  "$s7",
	"$t8",  "$t9",  "$k0",  "$k1",  "$gp",  "$sp",  "$fp",  "$ra"
};
#else
static const char *const reg[32] =
{
	"r0",   "r1",   "r2",   "r3",   "r4",   "r5",   "r6",   "r7",
	"r8",   "r9",   "r10",  "r11",  "r12",  "r13",  "r14",  "r15",
	"r16",  "r17",  "r18",  "r19",  "r20",  "r21",  "r22",  "r23",
	"r24",  "r25",  "r26",  "r27",  "r28",  "r29",  "r30",  "r31"
};
#endif

static const char *const cacheop[32] =
{
	"I_Invd", "D_WBInvd", "Unknown 2", "Unknown 3", "I_IndexLoadTag", "D_IndexLoadTag", "Unknown 6", "Unknown 7",
	"I_IndexStoreTag", "D_IndexStoreTag", "Unknown 10", "Unknown 11", "Unknown 12", "D_CreateDirtyExcl", "Unknown 14", "Unknown 15",
	"I_HitInvalid", "D_HitInvalid", "Unknown 18", "Unknown 19", "I_Fill", "D_HitWBInvalid", "Unknown 22", "Unknown 23",
	"I_HitWB", "D_HitWB", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29", "Unknown 30", "Unknown 31"
};


static const char *const cpreg[4][32] =
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


static const char *const ccreg[4][32] =
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

INLINE char *signed_16bit(INT16 val)
{
	static char temp[10];
	if (val < 0)
		sprintf(temp, "-$%x", -val);
	else
		sprintf(temp, "$%x", val);
	return temp;
}

static UINT32 dasm_cop0(UINT32 pc, UINT32 op, char *buffer)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	UINT32 flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  sprintf(buffer, "mfc0   %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x01:  sprintf(buffer, "dmfc0  %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x02:  sprintf(buffer, "cfc0   %s,%s", reg[rt], ccreg[0][rd]);                 break;
		case 0x04:  sprintf(buffer, "mtc0   %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x05:  sprintf(buffer, "dmtc0  %s,%s", reg[rt], cpreg[0][rd]);                 break;
		case 0x06:  sprintf(buffer, "ctc0   %s,%s", reg[rt], ccreg[0][rd]);                 break;
		case 0x08:  /* BC */
			switch (rt)
			{
				case 0x00:  sprintf(buffer, "bc0f   $%08x", pc + 4 + ((INT16)op << 2));     break;
				case 0x01:  sprintf(buffer, "bc0t   $%08x", pc + 4 + ((INT16)op << 2));     break;
				case 0x02:  sprintf(buffer, "bc0fl [invalid]");                             break;
				case 0x03:  sprintf(buffer, "bc0tl [invalid]");                             break;
				default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                  break;
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
				case 0x01:  sprintf(buffer, "tlbr");                                            break;
				case 0x02:  sprintf(buffer, "tlbwi");                                           break;
				case 0x06:  sprintf(buffer, "tlbwr");                                           break;
				case 0x08:  sprintf(buffer, "tlbp");                                            break;
				case 0x10:  sprintf(buffer, "rfe"); flags = DASMFLAG_STEP_OUT;                  break;
				case 0x18:  sprintf(buffer, "eret [invalid]");                                  break;
				default:    sprintf(buffer, "cop0  $%07x", op & 0x01ffffff);                    break;
			}
			break;
		default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                              break;
	}
	return flags;
}

static UINT32 dasm_cop1(UINT32 pc, UINT32 op, char *buffer)
{
	static const char *const format_table[] =
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
	UINT32 flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  sprintf(buffer, "mfc1   %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x01:  sprintf(buffer, "dmfc1  %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x02:  sprintf(buffer, "cfc1   %s,%s", reg[rt], ccreg[1][rd]);                     break;
		case 0x04:  sprintf(buffer, "mtc1   %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x05:  sprintf(buffer, "dmtc1  %s,%s", reg[rt], cpreg[1][rd]);                     break;
		case 0x06:  sprintf(buffer, "ctc1   %s,%s", reg[rt], ccreg[1][rd]);                     break;
		case 0x08:  /* BC */
			switch (rt & 3)
			{
				case 0x00:  sprintf(buffer, "bc1f   $%08x,%d", pc + 4 + ((INT16)op << 2), (op >> 18) & 7);      break;
				case 0x01:  sprintf(buffer, "bc1t   $%08x,%d", pc + 4 + ((INT16)op << 2), (op >> 18) & 7);      break;
				case 0x02:  sprintf(buffer, "bc1fl  $%08x,%d", pc + 4 + ((INT16)op << 2), (op >> 18) & 7); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				case 0x03:  sprintf(buffer, "bc1tl  $%08x,%d", pc + 4 + ((INT16)op << 2), (op >> 18) & 7); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
			}
			break;
		default:    /* COP */
			switch (op & 0x3f)
			{
				case 0x00:  sprintf(buffer, "add.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x01:  sprintf(buffer, "sub.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x02:  sprintf(buffer, "mul.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x03:  sprintf(buffer, "div.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], cpreg[1][ft]); break;
				case 0x04:  sprintf(buffer, "sqrt.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x05:  sprintf(buffer, "abs.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x06:  sprintf(buffer, "mov.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x07:  sprintf(buffer, "neg.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                  break;
				case 0x08:  sprintf(buffer, "round.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x09:  sprintf(buffer, "trunc.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0a:  sprintf(buffer, "ceil.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x0b:  sprintf(buffer, "floor.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0c:  sprintf(buffer, "round.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0d:  sprintf(buffer, "trunc.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x0e:  sprintf(buffer, "ceil.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x0f:  sprintf(buffer, "floor.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);               break;
				case 0x11:  sprintf(buffer, "mov%c.%s  %s,%s,%d", ((op >> 16) & 1) ? 't' : 'f', fmt, cpreg[1][fd], cpreg[1][fs], (op >> 18) & 7);   break;
				case 0x12:  sprintf(buffer, "movz.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], reg[rt]);     break;
				case 0x13:  sprintf(buffer, "movn.%s  %s,%s,%s", fmt, cpreg[1][fd], cpreg[1][fs], reg[rt]);     break;
				case 0x15:  sprintf(buffer, "recip.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x16:  sprintf(buffer, "rsqrt.%s  %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                break;
				case 0x20:  sprintf(buffer, "cvt.s.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x21:  sprintf(buffer, "cvt.d.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x24:  sprintf(buffer, "cvt.w.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x25:  sprintf(buffer, "cvt.l.%s %s,%s", fmt, cpreg[1][fd], cpreg[1][fs]);                 break;
				case 0x30:  sprintf(buffer, "c.f.%s  %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x31:  sprintf(buffer, "c.un.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x32:  sprintf(buffer, "c.eq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x33:  sprintf(buffer, "c.ueq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x34:  sprintf(buffer, "c.olt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x35:  sprintf(buffer, "c.ult.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x36:  sprintf(buffer, "c.ole.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x37:  sprintf(buffer, "c.ule.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x38:  sprintf(buffer, "c.sf.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x39:  sprintf(buffer, "c.ngle.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);break;
				case 0x3a:  sprintf(buffer, "c.seq.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3b:  sprintf(buffer, "c.ngl.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3c:  sprintf(buffer, "c.lt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x3d:  sprintf(buffer, "c.nge.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				case 0x3e:  sprintf(buffer, "c.le.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);    break;
				case 0x3f:  sprintf(buffer, "c.ngt.%s %s,%s,%d", fmt, cpreg[1][fs], cpreg[1][ft], (op >> 8) & 7);   break;
				default:    sprintf(buffer, "cop1   $%07x", op & 0x01ffffff);                                   break;
			}
			break;
	}
	return flags;
}

static UINT32 dasm_cop1x(UINT32 pc, UINT32 op, char *buffer)
{
	static const char *const format3_table[] =
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
	UINT32 flags = 0;

	switch (op & 0x3f)
	{
		case 0x00:  sprintf(buffer, "lwxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x01:  sprintf(buffer, "ldxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x08:  sprintf(buffer, "swxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x09:  sprintf(buffer, "sdxc1   %s,%s(%s)", cpreg[1][fd], reg[rt], reg[rs]);               break;
		case 0x0f:  sprintf(buffer, "prefx   %d,%s(%s)", rd, reg[rt], reg[rs]);                         break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:  sprintf(buffer, "madd.%s  %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:  sprintf(buffer, "msub.%s  %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x30:
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:  sprintf(buffer, "nmadd.%s %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:  sprintf(buffer, "nmsub.%s %s,%s,%s,%s", fmt3, cpreg[1][fd], cpreg[1][fr], cpreg[1][fs], cpreg[1][ft]); break;
		default:    sprintf(buffer, "cop1   $%07x", op & 0x01ffffff);                                   break;
	}
	return flags;
}

static UINT32 dasm_cop2(UINT32 pc, UINT32 op, char *buffer)
{
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	UINT32 flags = 0;

	switch ((op >> 21) & 31)
	{
		case 0x00:  sprintf(buffer, "mfc2   %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x01:  sprintf(buffer, "dmfc2  %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x02:  sprintf(buffer, "cfc2   %s,%s", reg[rt], ccreg[2][rd]);                 break;
		case 0x04:  sprintf(buffer, "mtc2   %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x05:  sprintf(buffer, "dmtc2  %s,%s", reg[rt], cpreg[2][rd]);                 break;
		case 0x06:  sprintf(buffer, "ctc2   %s,%s", reg[rt], ccreg[2][rd]);                 break;
		case 0x08:  /* BC */
			switch (rt)
			{
				case 0x00:  sprintf(buffer, "bc2f   $%08x", pc + 4 + ((INT16)op << 2));     break;
				case 0x01:  sprintf(buffer, "bc2t   $%08x", pc + 4 + ((INT16)op << 2));     break;
				case 0x02:  sprintf(buffer, "bc2fl [invalid]");                             break;
				case 0x03:  sprintf(buffer, "bc2tl [invalid]");                             break;
				default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                  break;
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
			sprintf(buffer, "cop2   $%07x", op & 0x01ffffff);
			break;
		default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                                  break;
	}
	return flags;
}

unsigned dasmmips3(char *buffer, unsigned pc, UINT32 op)
{
	int rs = (op >> 21) & 31;
	int rt = (op >> 16) & 31;
	int rd = (op >> 11) & 31;
	int shift = (op >> 6) & 31;
	UINT32 flags = 0;

	switch (op >> 26)
	{
		case 0x00:  /* SPECIAL */
			switch (op & 63)
			{
				case 0x00:  if (op == 0)
							sprintf(buffer, "nop");
							else
							sprintf(buffer, "sll    %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x01:  sprintf(buffer, "mov%c   %s,%s,%d", ((op >> 16) & 1) ? 't' : 'f', reg[rd], reg[rs], (op >> 18) & 7); break;
				case 0x02:  sprintf(buffer, "srl    %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x03:  sprintf(buffer, "sra    %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x04:  sprintf(buffer, "sllv   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x06:  sprintf(buffer, "srlv   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x07:  sprintf(buffer, "srav   %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x08:  sprintf(buffer, "jr     %s", reg[rs]); if (rs == 31) flags = DASMFLAG_STEP_OUT; break;
				case 0x09:  if (rd == 31)
							sprintf(buffer, "jalr   %s", reg[rs]);
							else
							sprintf(buffer, "jalr   %s,%s", reg[rs], reg[rd]); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				case 0x0a:  sprintf(buffer, "movz   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x0b:  sprintf(buffer, "movn   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x0c:  sprintf(buffer, "syscall"); flags = DASMFLAG_STEP_OVER;                 break;
				case 0x0d:  sprintf(buffer, "break"); flags = DASMFLAG_STEP_OVER;                   break;
				case 0x0f:  sprintf(buffer, "sync");                                                break;
				case 0x10:  sprintf(buffer, "mfhi   %s", reg[rd]);                                  break;
				case 0x11:  sprintf(buffer, "mthi   %s", reg[rs]);                                  break;
				case 0x12:  sprintf(buffer, "mflo   %s", reg[rd]);                                  break;
				case 0x13:  sprintf(buffer, "mtlo   %s", reg[rs]);                                  break;
				case 0x14:  sprintf(buffer, "dsllv  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x16:  sprintf(buffer, "dsrlv  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x17:  sprintf(buffer, "dsrav  %s,%s,%s", reg[rd], reg[rt], reg[rs]);          break;
				case 0x18:  sprintf(buffer, "mult   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x19:  sprintf(buffer, "multu  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1a:  sprintf(buffer, "div    %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1b:  sprintf(buffer, "divu   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1c:  sprintf(buffer, "dmult  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1d:  sprintf(buffer, "dmultu %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1e:  sprintf(buffer, "ddiv   %s,%s", reg[rs], reg[rt]);                      break;
				case 0x1f:  sprintf(buffer, "ddivu  %s,%s", reg[rs], reg[rt]);                      break;
				case 0x20:  sprintf(buffer, "add    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x21:  sprintf(buffer, "addu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x22:  sprintf(buffer, "sub    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x23:  sprintf(buffer, "subu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x24:  sprintf(buffer, "and    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x25:  sprintf(buffer, "or     %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x26:  sprintf(buffer, "xor    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x27:  sprintf(buffer, "nor    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2a:  sprintf(buffer, "slt    %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2b:  sprintf(buffer, "sltu   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2c:  sprintf(buffer, "dadd   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2d:  sprintf(buffer, "daddu  %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2e:  sprintf(buffer, "dsub   %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x2f:  sprintf(buffer, "dsubu  %s,%s,%s", reg[rd], reg[rs], reg[rt]);          break;
				case 0x30:  sprintf(buffer, "tge    %s,%s", reg[rs], reg[rt]); flags = DASMFLAG_STEP_OVER; break;
				case 0x31:  sprintf(buffer, "tgeu   %s,%s", reg[rs], reg[rt]); flags = DASMFLAG_STEP_OVER; break;
				case 0x32:  sprintf(buffer, "tlt    %s,%s", reg[rs], reg[rt]); flags = DASMFLAG_STEP_OVER; break;
				case 0x33:  sprintf(buffer, "tltu   %s,%s", reg[rs], reg[rt]); flags = DASMFLAG_STEP_OVER; break;
				case 0x34:  sprintf(buffer, "teq    %s,%s", reg[rs], reg[rt]); flags = DASMFLAG_STEP_OVER; break;
				case 0x36:  sprintf(buffer, "tne    %s,%s", reg[rs], reg[rt]) ;flags = DASMFLAG_STEP_OVER; break;
				case 0x38:  sprintf(buffer, "dsll   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3a:  sprintf(buffer, "dsrl   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3b:  sprintf(buffer, "dsra   %s,%s,%d", reg[rd], reg[rt], shift);            break;
				case 0x3c:  sprintf(buffer, "dsll   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				case 0x3e:  sprintf(buffer, "dsrl   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				case 0x3f:  sprintf(buffer, "dsra   %s,%s,%d", reg[rd], reg[rt], shift+32);         break;
				default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                          break;
			}
			break;

		case 0x01:  /* REGIMM */
			switch ((op >> 16) & 31)
			{
				case 0x00:  sprintf(buffer, "bltz   %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x01:  sprintf(buffer, "bgez   %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x02:  sprintf(buffer, "bltzl  %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x03:  sprintf(buffer, "bgezl  %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); break;
				case 0x08:  sprintf(buffer, "tgei   %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x09:  sprintf(buffer, "tgeiu  %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x0a:  sprintf(buffer, "tlti   %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x0b:  sprintf(buffer, "tltiu  %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x0c:  sprintf(buffer, "teqi   %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x0e:  sprintf(buffer, "tnei   %s,%s", reg[rs], signed_16bit(op)); flags = DASMFLAG_STEP_OVER; break;
				case 0x10:  sprintf(buffer, "bltzal %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				case 0x11:  sprintf(buffer, "bgezal %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				case 0x12:  sprintf(buffer, "bltzall %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				case 0x13:  sprintf(buffer, "bgezall %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2)); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
				default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                          break;
			}
			break;

		case 0x02:  sprintf(buffer, "j      $%08x", (pc & 0xf0000000) | ((op & 0x03ffffff) << 2));  break;
		case 0x03:  sprintf(buffer, "jal    $%08x", (pc & 0xf0000000) | ((op & 0x03ffffff) << 2)); flags = DASMFLAG_STEP_OVER | DASMFLAG_STEP_OVER_EXTRA(1); break;
		case 0x04:  if (rs == 0 && rt == 0)
					sprintf(buffer, "b      $%08x", pc + 4 + ((INT16)op << 2));
					else
					sprintf(buffer, "beq    %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x05:  sprintf(buffer, "bne    %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x06:  sprintf(buffer, "blez   %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2));         break;
		case 0x07:  sprintf(buffer, "bgtz   %s,$%08x", reg[rs], pc + 4 + ((INT16)op << 2));         break;
		case 0x08:  sprintf(buffer, "addi   %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x09:  sprintf(buffer, "addiu  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0a:  sprintf(buffer, "slti   %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0b:  sprintf(buffer, "sltiu  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x0c:  sprintf(buffer, "andi   %s,%s,$%04x", reg[rt], reg[rs], (UINT16)op);            break;
		case 0x0d:  sprintf(buffer, "ori    %s,%s,$%04x", reg[rt], reg[rs], (UINT16)op);            break;
		case 0x0e:  sprintf(buffer, "xori   %s,%s,$%04x", reg[rt], reg[rs], (UINT16)op);            break;
		case 0x0f:  sprintf(buffer, "lui    %s,$%04x", reg[rt], (UINT16)op);                        break;
		case 0x10:  flags = dasm_cop0(pc, op, buffer);                                              break;
		case 0x11:  flags = dasm_cop1(pc, op, buffer);                                              break;
		case 0x12:  flags = dasm_cop2(pc, op, buffer);                                              break;
		case 0x13:  flags = dasm_cop1x(pc, op, buffer);                                             break;
		case 0x14:  sprintf(buffer, "beql   %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x15:  sprintf(buffer, "bnel   %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x16:  sprintf(buffer, "blezl  %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x17:  sprintf(buffer, "bgtzl  %s,%s,$%08x", reg[rs], reg[rt], pc + 4 + ((INT16)op << 2));break;
		case 0x18:  sprintf(buffer, "daddi  %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x19:  sprintf(buffer, "daddiu %s,%s,%s", reg[rt], reg[rs], signed_16bit(op));         break;
		case 0x1a:  sprintf(buffer, "ldl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x1b:  sprintf(buffer, "ldr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x1c:  /* IDT-specific opcodes: mad/madu/mul on R4640/4650, msub on RC32364 */
			switch (op & 0x1f)
			{
				case 0: sprintf(buffer, "mad    %s,%s", reg[rs], reg[rt]); break;
				case 1: sprintf(buffer, "madu   %s,%s", reg[rs], reg[rt]); break;
				case 2: sprintf(buffer, "mul    %s,%s,%s", reg[rs], reg[rt], reg[rd]); break;
				case 4: sprintf(buffer, "msub   %s,%s", reg[rs], reg[rt]); break;
				default:sprintf(buffer, "dc.l   $%08x [invalid]", op);  break;
			}
			break;
		case 0x20:  sprintf(buffer, "lb     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x21:  sprintf(buffer, "lh     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x22:  sprintf(buffer, "lwl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x23:  sprintf(buffer, "lw     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x24:  sprintf(buffer, "lbu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x25:  sprintf(buffer, "lhu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x26:  sprintf(buffer, "lwr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x27:  sprintf(buffer, "lwu    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x28:  sprintf(buffer, "sb     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x29:  sprintf(buffer, "sh     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2a:  sprintf(buffer, "swl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2b:  sprintf(buffer, "sw     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2c:  sprintf(buffer, "sdl    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2d:  sprintf(buffer, "sdr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2e:  sprintf(buffer, "swr    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x2f:  sprintf(buffer, "cache  %s,%s(%s)", cacheop[rt], reg[rs], signed_16bit(op));                    break;
		case 0x30:  sprintf(buffer, "ll     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x31:  sprintf(buffer, "lwc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x32:  sprintf(buffer, "lwc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x33:  sprintf(buffer, "pref   $%x,%s(%s)", rt, signed_16bit(op), reg[rs]);            break;
		case 0x34:  sprintf(buffer, "lld    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x35:  sprintf(buffer, "ldc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x36:  sprintf(buffer, "ldc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x37:  sprintf(buffer, "ld     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x38:  sprintf(buffer, "sc     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x39:  sprintf(buffer, "swc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3a:  sprintf(buffer, "swc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3c:  sprintf(buffer, "scd    %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		case 0x3d:  sprintf(buffer, "sdc1   %s,%s(%s)", cpreg[1][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3e:  sprintf(buffer, "sdc2   %s,%s(%s)", cpreg[2][rt], signed_16bit(op), reg[rs]);   break;
		case 0x3f:  sprintf(buffer, "sd     %s,%s(%s)", reg[rt], signed_16bit(op), reg[rs]);        break;
		default:    sprintf(buffer, "dc.l   $%08x [invalid]", op);                                  break;
	}
	return 4 | flags | DASMFLAG_SUPPORTED;
}


CPU_DISASSEMBLE( mips3be )
{
	UINT32 op = *(UINT32 *)oprom;
	op = BIG_ENDIANIZE_INT32(op);
	return dasmmips3(buffer, pc, op);
}


CPU_DISASSEMBLE( mips3le )
{
	UINT32 op = *(UINT32 *)oprom;
	op = LITTLE_ENDIANIZE_INT32(op);
	return dasmmips3(buffer, pc, op);
}
