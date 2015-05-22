// license:BSD-3-Clause
// copyright-holders:Tony La Porta, hap
	/**************************************************************************\
	*              Texas Instruments TMS320x25 DSP Disassembler                *
	*                                                                          *
	*                 Copyright Tony La Porta                                  *
	*              To be used with TMS320x25 DSP Emulator engine.              *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*         Many thanks to those involved in the i8039 Disassembler          *
	*               as the structure here was borrowed from it.                *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	*                                                                          *
	* A Memory Address                                                         *
	* B Opcode Address Argument (Requires next opcode read)                    *
	* C Compare mode                                                           *
	* D Immediate byte load                                                    *
	* K Immediate bit load                                                     *
	* W Immediate word load                                                    *
	* M AR[x] register modification type (for indirect addressing)             *
	* N ARP register to change ARP pointer to (for indirect addressing)        *
	* P I/O port address number                                                *
	* R AR[R] register to use                                                  *
	* S Shift ALU left                                                         *
	* T Shift ALU left (Hex) / Nibble data                                     *
	* X Don't care bit                                                         *
	*                                                                          *
	\**************************************************************************/

#include "emu.h"
#include "debugger.h"
#include <ctype.h>

#include "tms32025.h"



typedef unsigned char byte;
typedef unsigned short int word;

#define FMT(a,b) a, b
#define PTRS_PER_FORMAT 2

static const char *const arith[8] = { "*", "*-", "*+", "??", "BR0-", "*0-", "*0+", "*BR0+" } ;
static const char *const nextar[16] = { "", "", "", "", "", "", "", "", ",AR0", ",AR1", ",AR2", ",AR3", ",AR4", ",AR5", ",AR6", ",AR7" } ;
static const char *const cmpmode[4] = { "0 (ARx = AR0)" , "1 (ARx < AR0)" , "2 (ARx > AR0)" , "3 (ARx <> AR0)" } ;


static const char *const TMS32025Formats[] = {
	FMT("0000tttt0aaaaaaa", "add  %A,%T"),  /* 0xxx */
	FMT("0000tttt1mmmnnnn", "add  %M,%T%N"),
	FMT("0001tttt0aaaaaaa", "sub  %A,%T"),  /* 1xxx */
	FMT("0001tttt1mmmnnnn", "sub  %M,%T%N"),
	FMT("0010tttt0aaaaaaa", "lac  %A,%T"),  /* 2xxx */
	FMT("0010tttt1mmmnnnn", "lac  %M,%T%N"),
	FMT("00110rrr0aaaaaaa", "lar  %R,%A"),  /* 3xxx */
	FMT("00110rrr1mmmnnnn", "lar  %R%M%N"),
	FMT("001110000aaaaaaa", "mpy  %A"),     /* 38xx */
	FMT("001110001mmmnnnn", "mpy  %M%N"),
	FMT("001110010aaaaaaa", "sqra %A"),     /* 39xx */
	FMT("001110011mmmnnnn", "sqra %M%N"),
	FMT("001110100aaaaaaa", "mpya %A"),     /* 3Axx */
	FMT("001110101mmmnnnn", "mpya %M%N"),
	FMT("001110110aaaaaaa", "mpys %A"),     /* 3Bxx */
	FMT("001110111mmmnnnn", "mpys %M%N"),
	FMT("001111000aaaaaaa", "lt   %A"),     /* 3Cxx */
	FMT("001111001mmmnnnn", "lt   %M%N"),
	FMT("001111010aaaaaaa", "lta  %A"),     /* 3Dxx */
	FMT("001111011mmmnnnn", "lta  %M%N"),
	FMT("001111100aaaaaaa", "ltp  %A"),     /* 3Exx */
	FMT("001111101mmmnnnn", "ltp  %M%N"),
	FMT("001111110aaaaaaa", "ltd  %A"),     /* 3Fxx */
	FMT("001111111mmmnnnn", "ltd  %M%N"),
	FMT("010000000aaaaaaa", "zalh %A"),     /* 40xx */
	FMT("010000001mmmnnnn", "zalh %M%N"),
	FMT("010000010aaaaaaa", "zals %A"),     /* 41xx */
	FMT("010000011mmmnnnn", "zals %M%N"),
	FMT("010000100aaaaaaa", "lact %A"),     /* 42xx */
	FMT("010000101mmmnnnn", "lact %M%N"),
	FMT("010000110aaaaaaa", "addc %A%S"),   /* 43xx */
	FMT("010000111mmmnnnn", "addc %M%S%N"),
	FMT("010001000aaaaaaa", "subh %A"),     /* 44xx */
	FMT("010001001mmmnnnn", "subh %M%N"),
	FMT("010001010aaaaaaa", "subs %A"),     /* 45xx */
	FMT("010001011mmmnnnn", "subs %M%N"),
	FMT("010001100aaaaaaa", "subt %A"),     /* 46xx */
	FMT("010001101mmmnnnn", "subt %M%N"),
	FMT("010001110aaaaaaa", "subc %A"),     /* 47xx */
	FMT("010001111mmmnnnn", "subc %M%N"),
	FMT("010010000aaaaaaa", "addh %A"),     /* 48xx */
	FMT("010010001mmmnnnn", "addh %M%N"),
	FMT("010010010aaaaaaa", "adds %A"),     /* 49xx */
	FMT("010010011mmmnnnn", "adds %M%N"),
	FMT("010010100aaaaaaa", "addt %A"),     /* 4Axx */
	FMT("010010101mmmnnnn", "addt %M%N"),
	FMT("010010110aaaaaaa", "rpt  %A"),     /* 4Bxx */
	FMT("010010111mmmnnnn", "rpt  %M%N"),
	FMT("010011000aaaaaaa", "xor  %A"),     /* 4Cxx */
	FMT("010011001mmmnnnn", "xor  %M%N"),
	FMT("010011010aaaaaaa", "or   %A"),     /* 4Dxx */
	FMT("010011011mmmnnnn", "or   %M%N"),
	FMT("010011100aaaaaaa", "and  %A"),     /* 4Exx */
	FMT("010011101mmmnnnn", "and  %M%N"),
	FMT("010011110aaaaaaa", "subb %A"),     /* 4Fxx */
	FMT("010011111mmmnnnn", "subb %M%N"),
	FMT("010100000aaaaaaa", "lst  %A"),     /* 50xx */
	FMT("010100001mmmnnnn", "lst  %M%N"),
	FMT("010100010aaaaaaa", "lst1 %A"),     /* 51xx */
	FMT("010100011mmmnnnn", "lst1 %M%N"),
	FMT("010100100aaaaaaa", "ldp  %A"),     /* 52xx */
	FMT("010100101mmmnnnn", "ldp  %M%N"),
	FMT("010100110aaaaaaa", "lph  %A"),     /* 53xx */
	FMT("010100111mmmnnnn", "lph  %M%N"),
	FMT("010101000aaaaaaa", "pshd %A"),     /* 54xx */
	FMT("010101001mmmnnnn", "pshd %M%N"),

/*  FMT("010101010aaaaaaa", "mar  %A"),        55xx */
/*  MAR direct has been expanded out to all its variations because one of its */
/*  its opcodes is the same as NOP.  Actually MAR direct just performs a NOP */
		FMT("0101010100000000", "nop"),         /* 5500 */
		FMT("0101010100000001", "mar  $01"),
		FMT("0101010100000010", "mar  $02"),
		FMT("0101010100000011", "mar  $03"),
		FMT("0101010100000100", "mar  $04"),
		FMT("0101010100000101", "mar  $05"),
		FMT("0101010100000110", "mar  $06"),
		FMT("0101010100000111", "mar  $07"),
		FMT("0101010100001000", "mar  $08"),
		FMT("0101010100001001", "mar  $09"),
		FMT("0101010100001010", "mar  $0A"),
		FMT("0101010100001011", "mar  $0B"),
		FMT("0101010100001100", "mar  $0C"),
		FMT("0101010100001101", "mar  $0D"),
		FMT("0101010100001110", "mar  $0E"),
		FMT("0101010100001111", "mar  $0F"),
		FMT("010101010001tttt", "mar  $1%T"),
		FMT("010101010010tttt", "mar  $2%T"),
		FMT("010101010011tttt", "mar  $3%T"),
		FMT("010101010100tttt", "mar  $4%T"),
		FMT("010101010101tttt", "mar  $5%T"),
		FMT("010101010110tttt", "mar  $6%T"),
		FMT("010101010111tttt", "mar  $7%T"),

/*  FMT("010101011mmmnnnn", "mar  %M%N"),      55xx */
/*  MAR indirect has been expanded out to all its variations because one of */
/*  its opcodes, is the same as LARP (actually performs the same function) */
		FMT("0101010110000xxx", "mar  *"),      /* 558x */
		FMT("0101010110001kkk", "larp %K"),     /* 558x */
		FMT("010101011001nnnn", "mar  *-%N"),   /* 558x */
		FMT("010101011010nnnn", "mar  *+%N"),
		FMT("010101011011nnnn", "mar  ??%N"),
		FMT("010101011100nnnn", "mar  *BR0-%N"),
		FMT("010101011101nnnn", "mar  *0-%N"),
		FMT("010101011110nnnn", "mar  *0+%N"),
		FMT("010101011111nnnn", "mar  *BR0+%N"),

	FMT("010101100aaaaaaa", "dmov %A"),     /* 56xx */
	FMT("010101101mmmnnnn", "dmov %M%N"),
	FMT("010101110aaaaaaa", "bitt %A"),     /* 57xx */
	FMT("010101111mmmnnnn", "bitt %M%N"),
	FMT("010110000aaaaaaa", "tblr %A"),     /* 58xx */
	FMT("010110001mmmnnnn", "tblr %M%N"),
	FMT("010110010aaaaaaa", "tblw %A"),     /* 59xx */
	FMT("010110011mmmnnnn", "tblw %M%N"),
	FMT("010110100aaaaaaa", "sqrs %A"),     /* 5Axx */
	FMT("010110101mmmnnnn", "sqrs %M%N"),
	FMT("010110110aaaaaaa", "lts  %A"),     /* 5Bxx */
	FMT("010110111mmmnnnn", "lts  %M%N"),
	FMT("010111000aaaaaaabbbbbbbbbbbbbbbb", "macd %B,%A"),      /* 5Cxx */
	FMT("010111001mmmnnnnbbbbbbbbbbbbbbbb", "macd %B,%M%N"),
	FMT("010111010aaaaaaabbbbbbbbbbbbbbbb", "mac  %B,%A"),      /* 5Dxx */
	FMT("010111011mmmnnnnbbbbbbbbbbbbbbbb", "mac  %B,%M%N"),
	FMT("010111101mmmnnnnbbbbbbbbbbbbbbbb", "bc   %B %M%N"),    /* 5Exx */
	FMT("010111111mmmnnnnbbbbbbbbbbbbbbbb", "bnc  %B %M%N"),    /* 5Fxx */
	FMT("01100sss0aaaaaaa", "sacl %A%S"),   /* 6xxx */
	FMT("01100sss1mmmnnnn", "sacl %M%S%N"),
	FMT("01101sss0aaaaaaa", "sach %A%S"),   /* 6Xxx */
	FMT("01101sss1mmmnnnn", "sach %M%S%N"),
	FMT("01110rrr0aaaaaaa", "sar  %R,%A"),  /* 7xxx */
	FMT("01110rrr1mmmnnnn", "sar  %R%M%N"),
	FMT("011110000aaaaaaa", "sst  %A"),     /* 78xx */
	FMT("011110001mmmnnnn", "sst  %M%N"),
	FMT("011110010aaaaaaa", "sst1 %A"),     /* 79xx */
	FMT("011110011mmmnnnn", "sst1 %M%N"),
	FMT("011110100aaaaaaa", "popd %A"),     /* 7Axx */
	FMT("011110101mmmnnnn", "popd %M%N"),
	FMT("011110110aaaaaaa", "zalr %A"),     /* 7Bxx */
	FMT("011110111mmmnnnn", "zalr %M%N"),
	FMT("011111000aaaaaaa", "spl  %A"),     /* 7Cxx */
	FMT("011111001mmmnnnn", "spl  %M%N"),
	FMT("011111010aaaaaaa", "sph  %A"),     /* 7Dxx */
	FMT("011111011mmmnnnn", "sph  %M%N"),
	FMT("011111100aaaaaaa", "adrk %A"),     /* 7Exx */
	FMT("011111101mmmnnnn", "adrk %M%N"),
	FMT("011111110aaaaaaa", "sbrk %A"),     /* 7Fxx */
	FMT("011111111mmmnnnn", "sbrk %M%N"),
	FMT("1000pppp0aaaaaaa", "in   %A,%P"),  /* 8xxx */
	FMT("1000pppp1mmmnnnn", "in   %M,%P%N"),
	FMT("1001tttt0aaaaaaa", "bit  %A,%T"),  /* 9xxx */
	FMT("1001tttt1mmmnnnn", "bit  %M,%T%N"),
	FMT("101wwwwwwwwwwwww", "mpyk %W"),     /* Axxx-Bxxx */
	FMT("11000rrrdddddddd", "lark %R,%D"),  /* Cxxx */
	FMT("1100100kdddddddd", "ldpk %K%D"),   /* Cxxx */
/*  FMT("11001010dddddddd", "lack %D"),        CAxx */
/*  LACK has been expanded out to all its variations because one of its */
/*  its opcodes is the same as ZAC. Actually, it performs the same function */
		FMT("1100101000000000", "zac"),         /* CA00 */
		FMT("1100101000000001", "lack 01h"),    /* CAxx */
		FMT("1100101000000010", "lack 02h"),
		FMT("1100101000000011", "lack 03h"),
		FMT("1100101000000100", "lack 04h"),
		FMT("1100101000000101", "lack 05h"),
		FMT("1100101000000110", "lack 06h"),
		FMT("1100101000000111", "lack 07h"),
		FMT("1100101000001000", "lack 08h"),
		FMT("1100101000001001", "lack 09h"),
		FMT("1100101000001010", "lack 0Ah"),
		FMT("1100101000001011", "lack 0Bh"),
		FMT("1100101000001100", "lack 0Ch"),
		FMT("1100101000001101", "lack 0Dh"),
		FMT("1100101000001110", "lack 0Eh"),
		FMT("1100101000001111", "lack 0Fh"),
		FMT("110010100001tttt", "lack 1%T"),
		FMT("110010100010tttt", "lack 2%T"),
		FMT("110010100011tttt", "lack 3%T"),
		FMT("110010100100tttt", "lack 4%T"),
		FMT("110010100101tttt", "lack 5%T"),
		FMT("110010100110tttt", "lack 6%T"),
		FMT("110010100111tttt", "lack 7%T"),
		FMT("110010101000tttt", "lack 8%T"),
		FMT("110010101001tttt", "lack 9%T"),
		FMT("110010101010tttt", "lack A%T"),
		FMT("110010101011tttt", "lack B%T"),
		FMT("110010101100tttt", "lack C%T"),
		FMT("110010101101tttt", "lack D%T"),
		FMT("110010101110tttt", "lack E%T"),
		FMT("110010101111tttt", "lack F%T"),

	FMT("11001011dddddddd", "rptk %D"),     /* CBxx */
	FMT("11001100dddddddd", "addk %D"),     /* CCxx */
	FMT("11001101dddddddd", "subk %D"),     /* CDxx */
	FMT("1100111000000000", "eint"),        /* CE00 */
	FMT("1100111000000001", "dint"),        /* CE01 */
	FMT("1100111000000010", "rovm"),        /* CE02 */
	FMT("1100111000000011", "sovm"),        /* CE03 */
	FMT("1100111000000100", "cnfd"),        /* CE04 */
	FMT("1100111000000101", "cnfp"),        /* CE05 */
	FMT("1100111000000110", "rsxm"),        /* CE06 */
	FMT("1100111000000111", "ssxm"),        /* CE07 */
	FMT("11001110000010kk", "spm  %K"),     /* CE0x */
	FMT("1100111000001100", "rxf"),         /* CE0C */
	FMT("1100111000001101", "sxf"),         /* CE0D */
	FMT("110011100000111k", "fort %K"),     /* CE0x */
	FMT("1100111000010100", "pac"),         /* CE14 */
	FMT("1100111000010101", "apac"),        /* CE15 */
	FMT("1100111000010110", "spac"),        /* CE16 */
	FMT("1100111000011000", "sfl"),         /* CE18 */
	FMT("1100111000011001", "sfr"),         /* CE19 */
	FMT("1100111000011011", "abs"),         /* CE1B */
	FMT("1100111000011100", "push"),        /* CE1C */
	FMT("1100111000011101", "pop"),         /* CE1D */
	FMT("1100111000011110", "trap"),        /* CE1E */
	FMT("1100111000011111", "idle"),        /* CE1F */
	FMT("1100111000100000", "rtxm"),        /* CE20 */
	FMT("1100111000100001", "stxm"),        /* CE21 */
	FMT("1100111000100011", "neg"),         /* CE23 */
	FMT("1100111000100100", "cala"),        /* CE24 */
	FMT("1100111000100101", "bacc"),        /* CE25 */
	FMT("1100111000100110", "ret"),         /* CE26 */
	FMT("1100111000100111", "cmpl"),        /* CE27 */
	FMT("1100111000110000", "rc"),          /* CE30 */
	FMT("1100111000110001", "sc"),          /* CE31 */
	FMT("1100111000110010", "rtc"),         /* CE32 */
	FMT("1100111000110011", "stc"),         /* CE33 */
	FMT("1100111000110100", "rol"),         /* CE34 */
	FMT("1100111000110101", "ror"),         /* CE35 */
	FMT("1100111000110110", "rfsm"),        /* CE36 */
	FMT("1100111000110111", "sfsm"),        /* CE37 */
	FMT("1100111000111000", "rhm"),         /* CE38 */
	FMT("1100111000111001", "shm"),         /* CE39 */
	FMT("11001110001111kk", "conf %K"),     /* CE3x */
	FMT("11001110010100cc", "cmpr %C"),     /* CE5x */
	FMT("110011101mmm0010", "norm %M"),     /* CEx2 */
	FMT("110011110aaaaaaa", "mpys %A"),     /* CFxx */
	FMT("110011111mmmnnnn", "mpys %M%N"),
	FMT("11010rrr00000000wwwwwwwwwwwwwwww", "lrlk %R,%W"),      /* Dx00 */
	FMT("1101tttt00000001wwwwwwwwwwwwwwww", "lalk %W,%T"),      /* Dx01 */
	FMT("1101tttt00000010wwwwwwwwwwwwwwww", "adlk %W,%T"),      /* Dx02 */
	FMT("1101tttt00000011wwwwwwwwwwwwwwww", "sblk %W,%T"),      /* Dx03 */
	FMT("1101tttt00000100wwwwwwwwwwwwwwww", "andk %W,%T"),      /* Dx04 */
	FMT("1101tttt00000101wwwwwwwwwwwwwwww", "ork  %W,%T"),      /* Dx05 */
	FMT("1101tttt00000110wwwwwwwwwwwwwwww", "xork %W,%T"),      /* Dx06 */
	FMT("1110pppp0aaaaaaa", "out  %A,%P"),  /* Exxx */
	FMT("1110pppp1mmmnnnn", "out  %M,%P%N"),
	FMT("111100001mmmnnnnbbbbbbbbbbbbbbbb", "bv   %B %M%N"),    /* F0xx */
	FMT("111100011mmmnnnnbbbbbbbbbbbbbbbb", "bgz  %B %M%N"),    /* F1xx */
	FMT("111100101mmmnnnnbbbbbbbbbbbbbbbb", "blez %B %M%N"),    /* F2xx */
	FMT("111100111mmmnnnnbbbbbbbbbbbbbbbb", "blz  %B %M%N"),    /* F3xx */
	FMT("111101001mmmnnnnbbbbbbbbbbbbbbbb", "bgez %B %M%N"),    /* F4xx */
	FMT("111101011mmmnnnnbbbbbbbbbbbbbbbb", "bnz  %B %M%N"),    /* F5xx */
	FMT("111101101mmmnnnnbbbbbbbbbbbbbbbb", "bz   %B %M%N"),    /* F6xx */
	FMT("111101111mmmnnnnbbbbbbbbbbbbbbbb", "bnv  %B %M%N"),    /* F7xx */
	FMT("111110001mmmnnnnbbbbbbbbbbbbbbbb", "bbz  %B %M%N"),    /* F8xx */
	FMT("111110011mmmnnnnbbbbbbbbbbbbbbbb", "bbnz %B %M%N"),    /* F9xx */
	FMT("111110101mmmnnnnbbbbbbbbbbbbbbbb", "bioz %B %M%N"),    /* FAxx */
	FMT("111110111mmmnnnnbbbbbbbbbbbbbbbb", "banz %B %M%N"),    /* FBxx */
	FMT("111111000aaaaaaabbbbbbbbbbbbbbbb", "blkp %B,%A"),      /* FCxx */
	FMT("111111001mmmnnnnbbbbbbbbbbbbbbbb", "blkp %B,%M%N"),
	FMT("111111010aaaaaaabbbbbbbbbbbbbbbb", "blkd %B,%A"),      /* FDxx */
	FMT("111111011mmmnnnnbbbbbbbbbbbbbbbb", "blkd %B,%M%N"),
	FMT("111111101mmmnnnnbbbbbbbbbbbbbbbb", "call %B %M%N"),    /* FExx */
	FMT("111111111mmmnnnnbbbbbbbbbbbbbbbb", "b    %B %M%N"),    /* FFxx */
	NULL
};

#define MAX_OPS ((ARRAY_LENGTH(TMS32025Formats) - 1) / PTRS_PER_FORMAT)

struct TMS32025Opcode  {
	word mask;          /* instruction mask */
	word bits;          /* constant bits */
	word extcode;       /* value that gets extension code */
	const char *parse;  /* how to parse bits */
	const char *fmt;    /* instruction format */
};

static TMS32025Opcode Op[MAX_OPS+1];
static int OpInizialized = 0;

static void InitDasm32025(void)
{
	const char *p;
	const char *const *ops;
	word mask, bits;
	int bit;
	int i;

	ops = TMS32025Formats; i = 0;
	while (*ops)
	{
		p = *ops;
		mask = 0; bits = 0; bit = 15;
		while (*p && bit >= 0)
		{
			switch (*p++)
			{
				case '1': mask |= 1<<bit; bits |= 1<<bit; bit--; break;
				case '0': mask |= 1<<bit; bit--; break;
				case ' ': break;
				case 'a':
				case 'b':
				case 'c':
				case 'd':
				case 'k':
				case 'm':
				case 'n':
				case 'p':
				case 'r':
				case 's':
				case 't':
				case 'w':
				case 'x':
					bit --;
					break;
				default: fatalerror("Invalid instruction encoding '%s %s'\n",
					ops[0],ops[1]);
			}
		}
		if (bit != -1 )
		{
			fatalerror("not enough bits in encoding '%s %s' %d\n",
				ops[0],ops[1],bit);
		}
		while (isspace((UINT8)*p)) p++;
		if (*p) Op[i].extcode = *p;
		Op[i].bits = bits;
		Op[i].mask = mask;
		Op[i].fmt = ops[1];
		Op[i].parse = ops[0];

		ops += PTRS_PER_FORMAT;
		i++;
	}

	OpInizialized = 1;
}

CPU_DISASSEMBLE( tms32025 )
{
	UINT32 flags = 0;
	int a, b, c, d, k, m, n, p, r, s, t, w; /* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	//char *buffertmp;
	const char *cp;             /* character pointer in OpFormats */

	if (!OpInizialized) InitDasm32025();

	op = -1;                /* no matching opcode */
	code = (oprom[0] << 8) | oprom[1];
	for ( i = 0; i < MAX_OPS; i++)
	{
		if ((code & Op[i].mask) == Op[i].bits)
		{
			if (op != -1)
			{
				osd_printf_debug("Error: opcode %04Xh matches %d (%s) and %d (%s)\n",
					code,i,Op[i].fmt,op,Op[op].fmt);
			}
			op = i;
		}
	}
	if (op == -1)
	{
		sprintf(buffer,"???? dw %04Xh",code);
		return cnt | DASMFLAG_SUPPORTED;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)
	{
		bit = 31;
		code <<= 16;
		code |= (opram[2] << 8) | opram[3];
		cnt++;
	}
	else
	{
		bit = 15;
	}

	/* shift out operands */
	cp = Op[op].parse;
	a = b = c = d = k = m = n = p = r = s = t = w = 0;

	while (bit >= 0)
	{
		/* osd_printf_debug("{%c/%d}",*cp,bit); */
		switch(*cp)
		{
			case 'a': a <<=1; a |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'b': b <<=1; b |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'c': c <<=1; c |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'd': d <<=1; d |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'k': k <<=1; k |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'm': m <<=1; m |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'n': n <<=1; n |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'p': p <<=1; p |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'r': r <<=1; r |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 's': s <<=1; s |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 't': t <<=1; t |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'w': w <<=1; w |= ((code & (1<<bit)) ? 1 : 0); bit--; break;
			case 'x': bit--; break;
			case ' ': break;
			case '1': case '0': bit--; break;
			case '\0': fatalerror("premature end of parse string, opcode %x, bit = %d\n",code,bit);
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;

	if (!strncmp(cp, "cal", 3))
		flags = DASMFLAG_STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = DASMFLAG_STEP_OUT;

	while (*cp)
	{
		if (*cp == '%')
		{
			char num[30], *q;
			cp++;
			switch (*cp++)
			{
				case 'A': sprintf(num,"%02Xh",a); break;
				case 'B': sprintf(num,"%04Xh",b); break;
				case 'C': sprintf(num,"%s",cmpmode[c]); break;
				case 'D': sprintf(num,"%02Xh",d); break;
				case 'K': sprintf(num,"%d",k); break;
				case 'M': sprintf(num,"%s",arith[m]); break;
				case 'N': sprintf(num,"%s",nextar[n]); break;
				case 'P': sprintf(num,"PA$%01X",p); break;
				case 'R': sprintf(num,"AR%d",r); break;
				case 'S': sprintf(num,",%d",s); break;
				case 'T': sprintf(num,"%01Xh",t); break;
				case 'W': sprintf(num,"%04Xh",w); break;
				case 'X': break;
				default:
					fatalerror("illegal escape character in format '%s'\n",Op[op].fmt);
			}
			q = num; while (*q) *buffer++ = *q++;
			*buffer = '\0';
		}
		else
		{
			*buffer++ = *cp++;
			*buffer = '\0';
		}
	}
	return cnt | flags | DASMFLAG_SUPPORTED;
}
