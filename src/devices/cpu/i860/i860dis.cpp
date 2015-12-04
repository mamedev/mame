// license:BSD-3-Clause
// copyright-holders:Jason Eckhardt
/***************************************************************************

    i860dis.c

    Disassembler for the Intel i860 emulator.

    Copyright (C) 1995-present Jason Eckhardt (jle@rice.edu)

***************************************************************************/

#include "emu.h"
#include "i860.h"

/* Macros for accessing register fields in instruction word.  */
#define get_isrc1(bits) (((bits) >> 11) & 0x1f)
#define get_isrc2(bits) (((bits) >> 21) & 0x1f)
#define get_idest(bits) (((bits) >> 16) & 0x1f)
#define get_fsrc1(bits) (((bits) >> 11) & 0x1f)
#define get_fsrc2(bits) (((bits) >> 21) & 0x1f)
#define get_fdest(bits) (((bits) >> 16) & 0x1f)
#define get_creg(bits)  (((bits) >> 21) & 0x7)

/* Macros for accessing immediate fields.  */
/* 16-bit immediate.  */
#define get_imm16(insn) ((insn) & 0xffff)


/* Control register names.  */
static const char *const cr2str[] =
	{"fir", "psr", "dirbase", "db", "fsr", "epsr", "!", "!"};


/* Sign extend N-bit number.  */
static INT32 sign_ext(UINT32 x, int n)
{
	INT32 t;
	t = x >> (n - 1);
	t = ((-t) << n) | x;
	return t;
}


/* Basic integer 3-address register format:
 *   mnemonic %rs1,%rs2,%rd  */
static void int_12d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	/* Possibly prefix shrd with 'd.' */
	if (((insn & 0xfc000000) == 0xb0000000) && (insn & 0x200))
		sprintf(buf, "d.%s\t%%r%d,%%r%d,%%r%d", mnemonic,
			get_isrc1 (insn), get_isrc2 (insn), get_idest (insn));
	else
		sprintf(buf, "%s\t%%r%d,%%r%d,%%r%d", mnemonic,
			get_isrc1 (insn), get_isrc2 (insn), get_idest (insn));
}


/* Basic integer 3-address imm16 format:
 *   mnemonic #imm16,%rs2,%rd  */
static void int_i2d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	/* Sign extend the 16-bit immediate.
	   Print as hex for the bitwise operations.  */
	int upper_6bits = (insn >> 26) & 0x3f;
	if (upper_6bits >= 0x30 && upper_6bits <= 0x3f)
		sprintf(buf, "%s\t0x%04x,%%r%d,%%r%d", mnemonic,
			(UINT32)(get_imm16 (insn)), get_isrc2 (insn), get_idest (insn));
	else
		sprintf(buf, "%s\t%d,%%r%d,%%r%d", mnemonic,
			sign_ext(get_imm16 (insn), 16), get_isrc2 (insn), get_idest (insn));
}


/* Integer (mixed) 2-address  isrc1ni,fdest.  */
static void int_1d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	sprintf(buf, "%s\t%%r%d,%%f%d", mnemonic, get_isrc1 (insn), get_fdest (insn));
}


/* Integer (mixed) 2-address  csrc2,idest.  */
static void int_cd(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	sprintf(buf, "%s\t%%%s,%%r%d", mnemonic, cr2str[get_creg (insn)], get_idest (insn));
}


/* Integer (mixed) 2-address  isrc1,csrc2.  */
static void int_1c(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	sprintf(buf, "%s\t%%r%d,%%%s", mnemonic, get_isrc1(insn), cr2str[get_creg (insn)]);
}


/* Integer 1-address register format:
 *   mnemonic %rs1  */
static void int_1(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	sprintf(buf, "%s\t%%r%d", mnemonic, get_isrc1 (insn));
}


/* Integer no-address register format:
 *   mnemonic  */
static void int_0(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	sprintf(buf, "%s", mnemonic);
}


/* Basic floating-point 3-address register format:
 *   mnemonic %fs1,%fs2,%fd  */
static void flop_12d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	const char *const suffix[4] = { "ss", "sd", "ds", "dd" };
	const char *prefix_d, *prefix_p;
	prefix_p = (insn & 0x400) ? "p" : "";
	prefix_d = (insn & 0x200) ? "d." : "";

	/* Special case: pf[m]am and pf[m]sm families are always pipelined, so they
	   do not have a prefix.  Also, for the pfmam and pfmsm families, replace
	   any 'a' in the mnemonic with 'm' and prepend an 'm'.  */
	if ((insn & 0x7f) < 0x20)
	{
		int is_pfam = insn & 0x400;
		if (!is_pfam)
		{
			char newname[256];
			char *op = mnemonic;
			char *np = newname + 1;
			newname[0] = 'm';
			while (*op)
			{
				if (*op == 'a')
					*np = 'm';
				else
					*np = *op;
				np++;
				op++;
			}
			*np = 0;
			mnemonic = newname;
		}
		prefix_p = "";
	}

	/* Special case: pfgt/pfle-- R-bit distinguishes the two.  */
	if ((insn & 0x7f) == 0x34)
	{
		const char *const mn[2] = { "fgt.", "fle." };
		int r = (insn & 0x080) >> 7;
		int s = (insn & 0x100) ? 3 : 0;
		sprintf(buf, "%s%s%s%s\t%%f%d,%%f%d,%%f%d", prefix_d, prefix_p, mn[r],
			suffix[s], get_fsrc1 (insn), get_fsrc2 (insn), get_fdest (insn));
	}
	else
	{
		int s = (insn & 0x180) >> 7;
		sprintf(buf, "%s%s%s%s\t%%f%d,%%f%d,%%f%d", prefix_d, prefix_p, mnemonic,
			suffix[s], get_fsrc1 (insn), get_fsrc2 (insn), get_fdest (insn));
	}
}


/* Floating-point 2-address register format:
 *   mnemonic %fs1,%fd  */
static void flop_1d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	const char *const suffix[4] = { "ss", "sd", "ds", "dd" };
	const char *prefix_d, *prefix_p;
	int s = (insn & 0x180) >> 7;
	prefix_p = (insn & 0x400) ? "p" : "";
	prefix_d = (insn & 0x200) ? "d." : "";
	sprintf(buf, "%s%s%s%s\t%%f%d,%%f%d", prefix_d, prefix_p, mnemonic,
		suffix[s], get_fsrc1 (insn), get_fdest (insn));
}


/* Floating-point 2-address register format:
 *   mnemonic %fs2,%fd  */
static void flop_2d(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	const char *const suffix[4] = { "ss", "sd", "ds", "dd" };
	const char *prefix_d;
	int s = (insn & 0x180) >> 7;
	prefix_d = (insn & 0x200) ? "d." : "";
	sprintf(buf, "%s%s%s\t%%f%d,%%f%d", prefix_d, mnemonic, suffix[s],
		get_fsrc2 (insn), get_fdest (insn));
}


/* Floating-point (mixed) 2-address register format:
 *  fxfr fsrc1,idest.  */
static void flop_fxfr(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	const char *prefix_d = (insn & 0x200) ? "d." : "";
	sprintf(buf, "%s%s\t%%f%d,%%r%d", prefix_d, mnemonic, get_fsrc1 (insn),
		get_idest (insn));
}


/* Branch with reg,reg,sbroff format:
 *   mnemonic %rs1,%rs2,sbroff  */
static void int_12S(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	INT32 sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	INT32 rel = (INT32)pc + (sbroff << 2) + 4;

	sprintf(buf, "%s\t%%r%d,%%r%d,0x%08x", mnemonic, get_isrc1 (insn),
		get_isrc2 (insn), (UINT32)rel);
}


/* Branch with #const5,reg,sbroff format:
 *   mnemonic #const5,%rs2,sbroff  */
static void int_i2S(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	INT32 sbroff = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);
	INT32 rel = (INT32)pc + (sbroff << 2) + 4;

	sprintf(buf, "%s\t%d,%%r%d,0x%08x", mnemonic, ((insn >> 11) & 0x1f),
		get_isrc2 (insn), (UINT32)rel);
}


/* Branch with lbroff format:
 *   mnemonic lbroff  */
static void int_L(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	INT32 lbroff =  sign_ext ((insn & 0x03ffffff), 26);
	INT32 rel = (INT32)pc + (lbroff << 2) + 4;

	sprintf(buf, "%s\t0x%08x", mnemonic, (UINT32)rel);
}


/* Integer load.
 *  ld.{b,s,l} isrc1(isrc2),idest
 *  ld.{b,s,l} #const(isrc2),idest  */
static void int_ldx(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	/* Operand size, in bytes.  */
	int sizes[4] = { 1, 1, 2, 4 };
	const char *const suffix[4] = { "b", "b", "s", "l" };
	UINT32 idx = 0;

	/* Bits 28 and 0 determine the operand size.  */
	idx = ((insn >> 27) & 2) | (insn & 1);

	/* Bit 26 determines the addressing mode (reg+reg or disp+reg).  */
	if (insn & 0x04000000)
	{
		/* Chop off lower bits of displacement.  */
		INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
		int size = sizes[idx];
		immsrc1 &= ~(size - 1);
		sprintf(buf, "%s%s\t%d(%%r%d),%%r%d", mnemonic, suffix[idx],
			immsrc1, get_isrc2 (insn), get_idest (insn));
	}
	else
		sprintf(buf, "%s%s\t%%r%d(%%r%d),%%r%d", mnemonic, suffix[idx],
			get_isrc1 (insn), get_isrc2 (insn), get_idest (insn));
}


/* Integer store: st.b isrc1ni,#const(isrc2)  */
static void int_stx(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	/* Operand size, in bytes.  */
	int sizes[4] = { 1, 1, 2, 4 };
	const char *const suffix[4] = { "b", "b", "s", "l" };
	int idx = 0;
	int size;
	INT32 immsrc = sign_ext ((((insn >> 5) & 0xf800) | (insn & 0x07ff)), 16);

	/* Bits 28 and 0 determine the operand size.  */
	idx = ((insn >> 27) & 2) | (insn & 1);

	/* Chop off lower bits of displacement.  */
	size = sizes[idx];
	immsrc &= ~(size - 1);
	sprintf(buf, "%s%s\t%%r%d,%d(%%r%d)", mnemonic, suffix[idx],
		get_isrc1 (insn), immsrc, get_isrc2 (insn));
}


/* Disassemble:
 *  "[p]fld.y isrc1(isrc2),fdest", "[p]fld.y isrc1(isrc2)++,idest",
 *  "[p]fld.y #const(isrc2),fdest" or "[p]fld.y #const(isrc2)++,idest".
 *  "fst.y fdest,isrc1(isrc2)", "fst.y fdest,isrc1(isrc2)++",
 *  "fst.y fdest,#const(isrc2)" or "fst.y fdest,#const(isrc2)++"
 *  Where y = {l,d,q}.  Note, there is no pfld.q, though.  */
static void int_fldst(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	INT32 immsrc1 = sign_ext (get_imm16 (insn), 16);
	/* Operand size, in bytes.  */
	int sizes[4] = { 8, 4, 16, 4 };
	const char *const suffix[4] = { "d", "l", "q", "l" };
	int idx = 0;
	int size = 0;
	int auto_inc = (insn & 1);
	const char *const auto_suff[2] = { "", "++" };
	int piped = (insn & 0x40000000) >> 29;
	const char *const piped_suff[2] = { "", "p" };
	int upper_6bits = (insn >> 26) & 0x3f;
	int is_load = (upper_6bits == 8 || upper_6bits == 9 || upper_6bits == 24
					|| upper_6bits == 25);

	/* Bits 2 and 1 determine the operand size.  */
	idx = ((insn >> 1) & 3);
	size = sizes[idx];

	/* There is no pipelined load quad on XR.  */
	if (piped && size == 16)
	{
		sprintf (buf, ".long\t%#08x; *", insn);
		return;
	}

	/* There is only a 64-bit pixel store.  */
	if ((upper_6bits == 15) && size != 8)
	{
		sprintf (buf, ".long\t%#08x", insn);
		return;
	}

	/* Bit 26 determines the addressing mode (reg+reg or disp+reg).  */
	if (insn & 0x04000000)
	{
		/* Chop off lower bits of displacement.  */
		immsrc1 &= ~(size - 1);
		if (is_load)
			sprintf(buf, "%s%s%s\t%d(%%r%d)%s,%%f%d", piped_suff[piped], mnemonic,
				suffix[idx], immsrc1, get_isrc2 (insn), auto_suff[auto_inc],
				get_fdest (insn));
		else
			sprintf(buf, "%s%s\t%%f%d,%d(%%r%d)%s", mnemonic, suffix[idx],
				get_fdest (insn), immsrc1, get_isrc2 (insn), auto_suff[auto_inc]);
	}
	else
	{
		if (is_load)
			sprintf(buf, "%s%s%s\t%%r%d(%%r%d)%s,%%f%d", piped_suff[piped],
				mnemonic, suffix[idx], get_isrc1 (insn), get_isrc2 (insn),
				auto_suff[auto_inc], get_fdest (insn));
		else
			sprintf(buf, "%s%s\t%%f%d,%%r%d(%%r%d)%s", mnemonic, suffix[idx],
				get_fdest (insn), get_isrc1 (insn), get_isrc2 (insn),
				auto_suff[auto_inc]);
	}
}


/* flush #const(isrc2)[++].  */
static void int_flush(char *buf, char *mnemonic, UINT32 pc, UINT32 insn)
{
	const char *const auto_suff[2] = { "", "++" };
	INT32 immsrc = sign_ext (get_imm16 (insn), 16);
	immsrc &= ~(16-1);
	sprintf(buf, "%s\t%d(%%r%d)%s", mnemonic, immsrc, get_isrc2 (insn),
		auto_suff[(insn & 1)]);
}


/* Flags for the decode table.  */
enum
{
	DEC_MORE    = 1,    /* More decoding necessary.  */
	DEC_DECODED = 2     /* Fully decoded, go.  */
};


struct decode_tbl_t
{
	/* Disassembly function for this opcode.
	   Call with buffer, mnemonic, pc, insn.  */
	void (*insn_dis)(char *, char *, UINT32, UINT32);

	/* Flags for this opcode.  */
	char flags;

	/* Mnemonic of this opcode (sometimes partial when more decode is
	   done in disassembly routines-- e.g., loads and stores).  */
	const char *mnemonic;
};


/* First-level decode table (i.e., for the 6 primary opcode bits).  */
static const decode_tbl_t decode_tbl[64] =
{
	/* A slight bit of decoding for loads and stores is done in the
	   execution routines (operand size and addressing mode), which
	   is why their respective entries are identical.  */
	{ int_ldx,   DEC_DECODED, "ld."        }, /* ld.b isrc1(isrc2),idest.  */
	{ int_ldx,   DEC_DECODED, "ld."        }, /* ld.b #const(isrc2),idest.  */
	{ int_1d,    DEC_DECODED, "ixfr"       }, /* ixfr isrc1ni,fdest.        */
	{ int_stx,   DEC_DECODED, "st."        }, /* st.b isrc1ni,#const(isrc2).  */
	{ int_ldx,   DEC_DECODED, "ld."        }, /* ld.{s,l} isrc1(isrc2),idest.  */
	{ int_ldx,   DEC_DECODED, "ld."        }, /* ld.{s,l} #const(isrc2),idest.  */
	{ nullptr,         0           , nullptr           },
	{ int_stx,   DEC_DECODED, "st."        }, /* st.{s,l} isrc1ni,#const(isrc2),idest.*/
	{ int_fldst, DEC_DECODED, "fld."       }, /* fld.{l,d,q} isrc1(isrc2)[++],fdest.  */
	{ int_fldst, DEC_DECODED, "fld."       }, /* fld.{l,d,q} #const(isrc2)[++],fdest. */
	{ int_fldst, DEC_DECODED, "fst."       }, /* fst.{l,d,q} fdest,isrc1(isrc2)[++]   */
	{ int_fldst, DEC_DECODED, "fst."       }, /* fst.{l,d,q} fdest,#const(isrc2)[++]  */
	{ int_cd,    DEC_DECODED, "ld.c"       }, /* ld.c csrc2,idest.                    */
	{ int_flush, DEC_DECODED, "flush"      }, /* flush #const(isrc2) (or autoinc).    */
	{ int_1c,    DEC_DECODED, "st.c"       }, /* st.c isrc1,csrc2.                    */
	{ int_fldst, DEC_DECODED, "pstd."      }, /* pst.d fdest,#const(isrc2)[++].       */
	{ int_1,     DEC_DECODED, "bri"        }, /* bri isrc1ni.                         */
	{ int_12d,   DEC_DECODED, "trap"       }, /* trap isrc1ni,isrc2,idest.            */
	{ nullptr,         DEC_MORE,    nullptr            }, /* FP ESCAPE FORMAT, more decode.       */
	{ nullptr,         DEC_MORE,    nullptr            }, /* CORE ESCAPE FORMAT, more decode.     */
	{ int_12S,   DEC_DECODED, "btne"       }, /* btne isrc1,isrc2,sbroff.             */
	{ int_i2S,   DEC_DECODED, "btne"       }, /* btne #const,isrc2,sbroff.            */
	{ int_12S,   DEC_DECODED, "bte"        }, /* bte isrc1,isrc2,sbroff.              */
	{ int_i2S,   DEC_DECODED, "bte"        }, /* bte #const5,isrc2,idest.             */
	{ int_fldst, DEC_DECODED, "pfld."      }, /* pfld.{l,d,q} isrc1(isrc2)[++],fdest. */
	{ int_fldst, DEC_DECODED, "pfld."      }, /* pfld.{l,d,q} #const(isrc2)[++],fdest.*/
	{ int_L,     DEC_DECODED, "br"         }, /* br lbroff.    */
	{ int_L,     DEC_DECODED, "call"       }, /* call lbroff . */
	{ int_L,     DEC_DECODED, "bc"         }, /* bc lbroff.    */
	{ int_L,     DEC_DECODED, "bc.t"       }, /* bc.t lbroff.  */
	{ int_L,     DEC_DECODED, "bnc"        }, /* bnc lbroff.   */
	{ int_L,     DEC_DECODED, "bnc.t"      }, /* bnc.t lbroff. */
	{ int_12d,   DEC_DECODED, "addu"       }, /* addu isrc1,isrc2,idest.    */
	{ int_i2d,   DEC_DECODED, "addu"       }, /* addu #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "subu"       }, /* subu isrc1,isrc2,idest.    */
	{ int_i2d,   DEC_DECODED, "subu"       }, /* subu #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "adds"       }, /* adds isrc1,isrc2,idest.    */
	{ int_i2d,   DEC_DECODED, "adds"       }, /* adds #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "subs"       }, /* subs isrc1,isrc2,idest.    */
	{ int_i2d,   DEC_DECODED, "subs"       }, /* subs #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "shl"        }, /* shl isrc1,isrc2,idest.     */
	{ int_i2d,   DEC_DECODED, "shl"        }, /* shl #const,isrc2,idest.    */
	{ int_12d,   DEC_DECODED, "shr"        }, /* shr isrc1,isrc2,idest.     */
	{ int_i2d,   DEC_DECODED, "shr"        }, /* shr #const,isrc2,idest.    */
	{ int_12d,   DEC_DECODED, "shrd"       }, /* shrd isrc1ni,isrc2,idest.  */
	{ int_12S,   DEC_DECODED, "bla"        }, /* bla isrc1ni,isrc2,sbroff.  */
	{ int_12d,   DEC_DECODED, "shra"       }, /* shra isrc1,isrc2,idest.    */
	{ int_i2d,   DEC_DECODED, "shra"       }, /* shra #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "and"        }, /* and isrc1,isrc2,idest.     */
	{ int_i2d,   DEC_DECODED, "and"        }, /* and #const,isrc2,idest.    */
	{ nullptr,         0           , nullptr           },
	{ int_i2d,   DEC_DECODED, "andh"       }, /* andh #const,isrc2,idest.   */
	{ int_12d,   DEC_DECODED, "andnot"     }, /* andnot isrc1,isrc2,idest.  */
	{ int_i2d,   DEC_DECODED, "andnot"     }, /* andnot #const,isrc2,idest. */
	{ nullptr,         0           , nullptr           },
	{ int_i2d,   DEC_DECODED, "andnoth"    }, /* andnoth #const,isrc2,idest.*/
	{ int_12d,   DEC_DECODED, "or"         }, /* or isrc1,isrc2,idest.      */
	{ int_i2d,   DEC_DECODED, "or"         }, /* or #const,isrc2,idest.     */
	{ nullptr,         0           , nullptr           },
	{ int_i2d,   DEC_DECODED, "orh"        }, /* orh #const,isrc2,idest.    */
	{ int_12d,   DEC_DECODED, "xor"        }, /* xor isrc1,isrc2,idest.     */
	{ int_i2d,   DEC_DECODED, "xor"        }, /* xor #const,isrc2,idest.    */
	{ nullptr,         0           , nullptr           },
	{ int_i2d,   DEC_DECODED, "xorh"       }, /* xorh #const,isrc2,idest.   */
};


/* Second-level decode table (i.e., for the 3 core escape opcode bits).  */
static const decode_tbl_t core_esc_decode_tbl[8] =
{
	{ nullptr,         0          , nullptr           },
	{ int_0,     DEC_DECODED, "lock"      }, /* lock.           */
	{ int_1,     DEC_DECODED, "calli"     }, /* calli isrc1ni.  */
	{ nullptr,         0          , nullptr           },
	{ int_0,     DEC_DECODED, "intovr"    }, /* intovr.         */
	{ nullptr,         0          , nullptr           },
	{ nullptr,         0          , nullptr           },
	{ int_0,     DEC_DECODED, "unlock"    }, /* unlock.         */
};


/* Second-level decode table (i.e., for the 7 FP extended opcode bits).  */
static const decode_tbl_t fp_decode_tbl[128] =
{
	/* Floating point instructions.  The least significant 7 bits are
	   the (extended) opcode and bits 10:7 are P,D,S,R respectively
	   ([p]ipelined, [d]ual, [s]ource prec., [r]esult prec.).
	   For some operations, I defer decoding the P,S,R bits to the
	   emulation routine for them.  */
	{ flop_12d,  DEC_DECODED, "r2p1."     }, /* 0x00 pf[m]am */
	{ flop_12d,  DEC_DECODED, "r2pt."     }, /* 0x01 pf[m]am */
	{ flop_12d,  DEC_DECODED, "r2ap1."    }, /* 0x02 pf[m]am */
	{ flop_12d,  DEC_DECODED, "r2apt."    }, /* 0x03 pf[m]am */
	{ flop_12d,  DEC_DECODED, "i2p1."     }, /* 0x04 pf[m]am */
	{ flop_12d,  DEC_DECODED, "i2pt."     }, /* 0x05 pf[m]am */
	{ flop_12d,  DEC_DECODED, "i2ap1."    }, /* 0x06 pf[m]am */
	{ flop_12d,  DEC_DECODED, "i2apt."    }, /* 0x07 pf[m]am */
	{ flop_12d,  DEC_DECODED, "rat1p2."   }, /* 0x08 pf[m]am */
	{ flop_12d,  DEC_DECODED, "m12apm."   }, /* 0x09 pf[m]am */
	{ flop_12d,  DEC_DECODED, "ra1p2."    }, /* 0x0A pf[m]am */
	{ flop_12d,  DEC_DECODED, "m12ttpa."  }, /* 0x0B pf[m]am */
	{ flop_12d,  DEC_DECODED, "iat1p2."   }, /* 0x0C pf[m]am */
	{ flop_12d,  DEC_DECODED, "m12tpm."   }, /* 0x0D pf[m]am */
	{ flop_12d,  DEC_DECODED, "ia1p2."    }, /* 0x0E pf[m]am */
	{ flop_12d,  DEC_DECODED, "m12tpa."   }, /* 0x0F pf[m]am */
	{ flop_12d,  DEC_DECODED, "r2s1."     }, /* 0x10 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "r2st."     }, /* 0x11 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "r2as1."    }, /* 0x12 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "r2ast."    }, /* 0x13 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "i2s1."     }, /* 0x14 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "i2st."     }, /* 0x15 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "i2as1."    }, /* 0x16 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "i2ast."    }, /* 0x17 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "rat1s2."   }, /* 0x18 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "m12asm."   }, /* 0x19 pf[m]sm */
	{ flop_12d,  DEC_DECODED, "ra1s2."    }, /* 0x1A pf[m]sm */
	{ flop_12d,  DEC_DECODED, "m12ttsa."  }, /* 0x1B pf[m]sm */
	{ flop_12d,  DEC_DECODED, "iat1s2."   }, /* 0x1C pf[m]sm */
	{ flop_12d,  DEC_DECODED, "m12tsm."   }, /* 0x1D pf[m]sm */
	{ flop_12d,  DEC_DECODED, "ia1s2."    }, /* 0x1E pf[m]sm */
	{ flop_12d,  DEC_DECODED, "m12tsa."   }, /* 0x1F pf[m]sm */
	{ flop_12d,  DEC_DECODED, "fmul."     }, /* 0x20 [p]fmul */
	{ flop_12d,  DEC_DECODED, "fmlow."    }, /* 0x21 fmlow.dd */
	{ flop_2d,   DEC_DECODED, "frcp."     }, /* 0x22 frcp.{ss,sd,dd} */
	{ flop_2d,   DEC_DECODED, "frsqr."    }, /* 0x23 frsqr.{ss,sd,dd} */
	{ flop_12d,  DEC_DECODED, "pfmul3.dd" }, /* 0x24 pfmul3.dd */
	{ nullptr,         0          , nullptr           }, /* 0x25 */
	{ nullptr,         0          , nullptr           }, /* 0x26 */
	{ nullptr,         0          , nullptr           }, /* 0x27 */
	{ nullptr,         0          , nullptr           }, /* 0x28 */
	{ nullptr,         0          , nullptr           }, /* 0x29 */
	{ nullptr,         0          , nullptr           }, /* 0x2A */
	{ nullptr,         0          , nullptr           }, /* 0x2B */
	{ nullptr,         0          , nullptr           }, /* 0x2C */
	{ nullptr,         0          , nullptr           }, /* 0x2D */
	{ nullptr,         0          , nullptr           }, /* 0x2E */
	{ nullptr,         0          , nullptr           }, /* 0x2F */
	{ flop_12d,  DEC_DECODED, "fadd."     }, /* 0x30, [p]fadd.{ss,sd,dd} */
	{ flop_12d,  DEC_DECODED, "fsub."     }, /* 0x31, [p]fsub.{ss,sd,dd} */
	{ flop_1d,   DEC_DECODED, "fix."      }, /* 0x32, [p]fix.{ss,sd,dd} */
	{ flop_1d,   DEC_DECODED, "famov."    }, /* 0x33, [p]famov.{ss,sd,ds,dd} */
	{ flop_12d,  DEC_DECODED, "f{gt,le}"  }, /* 0x34, pf{gt,le}.{ss,dd} */
	{ flop_12d,  DEC_DECODED, "feq."      }, /* 0x35, pfeq.{ss,dd} */
	{ nullptr,         0          , nullptr           }, /* 0x36 */
	{ nullptr,         0          , nullptr           }, /* 0x37 */
	{ nullptr,         0          , nullptr           }, /* 0x38 */
	{ nullptr,         0          , nullptr           }, /* 0x39 */
	{ flop_1d,   DEC_DECODED, "ftrunc."   }, /* 0x3A, [p]ftrunc.{ss,sd,dd} */
	{ nullptr,         0          , nullptr           }, /* 0x3B */
	{ nullptr,         0          , nullptr           }, /* 0x3C */
	{ nullptr,         0          , nullptr           }, /* 0x3D */
	{ nullptr,         0          , nullptr           }, /* 0x3E */
	{ nullptr,         0          , nullptr           }, /* 0x3F */
	{ flop_fxfr, DEC_DECODED, "fxfr"      }, /* 0x40, fxfr fsrc1,idest. */
	{ nullptr,         0          , nullptr           }, /* 0x41 */
	{ nullptr,         0          , nullptr           }, /* 0x42 */
	{ nullptr,         0          , nullptr           }, /* 0x43 */
	{ nullptr,         0          , nullptr           }, /* 0x44 */
	{ nullptr,         0          , nullptr           }, /* 0x45 */
	{ nullptr,         0          , nullptr           }, /* 0x46 */
	{ nullptr,         0          , nullptr           }, /* 0x47 */
	{ nullptr,         0          , nullptr           }, /* 0x48 */
	{ flop_12d,  DEC_DECODED, "fiadd."    }, /* 0x49, [p]fiadd.{ss,dd} */
	{ nullptr,         0          , nullptr           }, /* 0x4A */
	{ nullptr,         0          , nullptr           }, /* 0x4B */
	{ nullptr,         0          , nullptr           }, /* 0x4C */
	{ flop_12d,  DEC_DECODED, "fisub."    }, /* 0x4D, [p]fisub.{ss,dd} */
	{ nullptr,         0          , nullptr           }, /* 0x4E */
	{ nullptr,         0          , nullptr           }, /* 0x4F */
	{ flop_12d,  DEC_DECODED, "faddp"     }, /* 0x50, [p]faddp */
	{ flop_12d,  DEC_DECODED, "faddz"     }, /* 0x51, [p]faddz */
	{ nullptr,         0          , nullptr           }, /* 0x52 */
	{ nullptr,         0          , nullptr           }, /* 0x53 */
	{ nullptr,         0          , nullptr           }, /* 0x54 */
	{ nullptr,         0          , nullptr           }, /* 0x55 */
	{ nullptr,         0          , nullptr           }, /* 0x56 */
	{ flop_12d,  DEC_DECODED, "fzchkl"    }, /* 0x57, [p]fzchkl */
	{ nullptr,         0          , nullptr           }, /* 0x58 */
	{ nullptr,         0          , nullptr           }, /* 0x59 */
	{ flop_1d,   DEC_DECODED, "form"      }, /* 0x5A, [p]form.dd */
	{ nullptr,         0          , nullptr           }, /* 0x5B */
	{ nullptr,         0          , nullptr           }, /* 0x5C */
	{ nullptr,         0          , nullptr           }, /* 0x5D */
	{ nullptr,         0          , nullptr           }, /* 0x5E */
	{ flop_12d,  DEC_DECODED, "fzchks"    }, /* 0x5F, [p]fzchks */
	{ nullptr,         0          , nullptr           }, /* 0x60 */
	{ nullptr,         0          , nullptr           }, /* 0x61 */
	{ nullptr,         0          , nullptr           }, /* 0x62 */
	{ nullptr,         0          , nullptr           }, /* 0x63 */
	{ nullptr,         0          , nullptr           }, /* 0x64 */
	{ nullptr,         0          , nullptr           }, /* 0x65 */
	{ nullptr,         0          , nullptr           }, /* 0x66 */
	{ nullptr,         0          , nullptr           }, /* 0x67 */
	{ nullptr,         0          , nullptr           }, /* 0x68 */
	{ nullptr,         0          , nullptr           }, /* 0x69 */
	{ nullptr,         0          , nullptr           }, /* 0x6A */
	{ nullptr,         0          , nullptr           }, /* 0x6B */
	{ nullptr,         0          , nullptr           }, /* 0x6C */
	{ nullptr,         0          , nullptr           }, /* 0x6D */
	{ nullptr,         0          , nullptr           }, /* 0x6E */
	{ nullptr,         0          , nullptr           }, /* 0x6F */
	{ nullptr,         0          , nullptr           }, /* 0x70 */
	{ nullptr,         0          , nullptr           }, /* 0x71 */
	{ nullptr,         0          , nullptr           }, /* 0x72 */
	{ nullptr,         0          , nullptr           }, /* 0x73 */
	{ nullptr,         0          , nullptr           }, /* 0x74 */
	{ nullptr,         0          , nullptr           }, /* 0x75 */
	{ nullptr,         0          , nullptr           }, /* 0x76 */
	{ nullptr,         0          , nullptr           }, /* 0x77 */
	{ nullptr,         0          , nullptr           }, /* 0x78 */
	{ nullptr,         0          , nullptr           }, /* 0x79 */
	{ nullptr,         0          , nullptr           }, /* 0x7A */
	{ nullptr,         0          , nullptr           }, /* 0x7B */
	{ nullptr,         0          , nullptr           }, /* 0x7C */
	{ nullptr,         0          , nullptr           }, /* 0x7D */
	{ nullptr,         0          , nullptr           }, /* 0x7E */
	{ nullptr,         0          , nullptr           }, /* 0x7F */
};


/* Replaces tabs with spaces.  */
static void i860_dasm_tab_replacer(char* buf, int tab_size)
{
	int i = 0;
	int tab_count = 0;
	char tab_buf[1024];
	memset(tab_buf, 0, 1024);

	while (i != strlen(buf))
	{
		if (buf[i] != '\t')
		{
			tab_buf[tab_count] = buf[i];
			tab_count++;
		}
		else
		{
			while (tab_count % tab_size != 0)
			{
				strcat(tab_buf, " ");
				tab_count++;
			}
		}
		i++;
	}

	tab_buf[tab_count] = 0x00;
	strcpy(buf, tab_buf);
}


CPU_DISASSEMBLE( i860 )
{
	UINT32 insn = (oprom[0] << 0) |
		(oprom[1] << 8)  |
		(oprom[2] << 16) |
		(oprom[3] << 24);

	int unrecognized_op = 1;
	int upper_6bits = (insn >> 26) & 0x3f;
	char flags = decode_tbl[upper_6bits].flags;
	if (flags & DEC_DECODED)
	{
		const char *s = decode_tbl[upper_6bits].mnemonic;
		decode_tbl[upper_6bits].insn_dis (buffer, (char *)s, pc, insn);
		unrecognized_op = 0;
	}
	else if (flags & DEC_MORE)
	{
		if (upper_6bits == 0x12)
		{
			/* FP instruction format handled here.  */
			char fp_flags = fp_decode_tbl[insn & 0x7f].flags;
			const char *s = fp_decode_tbl[insn & 0x7f].mnemonic;
			if (fp_flags & DEC_DECODED)
			{
				fp_decode_tbl[insn & 0x7f].insn_dis (buffer, (char *)s, pc, insn);
				unrecognized_op = 0;
			}
		}
		else if (upper_6bits == 0x13)
		{
			/* Core escape instruction format handled here.  */
			char esc_flags = core_esc_decode_tbl[insn & 0x3].flags;
			const char *s = core_esc_decode_tbl[insn & 0x3].mnemonic;
			if (esc_flags & DEC_DECODED)
			{
				core_esc_decode_tbl[insn & 0x3].insn_dis (buffer, (char *)s, pc, insn);
				unrecognized_op = 0;
			}
		}
	}

	if (unrecognized_op)
		sprintf (buffer, ".long\t%#08x", insn);

	/* Replace tabs with spaces */
	i860_dasm_tab_replacer(buffer, 10);

	/* Return number of bytes disassembled.  */
	/* MAME dasm flags haven't been added yet */
	return (4);
}
