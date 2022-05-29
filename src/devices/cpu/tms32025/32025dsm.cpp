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
#include "32025dsm.h"

#include <cctype>
#include <stdexcept>


const char *const tms32025_disassembler::arith[8] = { "*", "*-", "*+", "??", "BR0-", "*0-", "*0+", "*BR0+" };
const char *const tms32025_disassembler::nextar[16] = { "", "", "", "", "", "", "", "", ",AR0", ",AR1", ",AR2", ",AR3", ",AR4", ",AR5", ",AR6", ",AR7" };
const char *const tms32025_disassembler::cmpmode[4] = { "0 (ARx = AR0)" , "1 (ARx < AR0)" , "2 (ARx > AR0)" , "3 (ARx <> AR0)" };


const char *const tms32025_disassembler::TMS32025Formats[] = {
	"0000tttt0aaaaaaa", "add  %A,%T",  /* 0xxx */
	"0000tttt1mmmnnnn", "add  %M,%T%N",
	"0001tttt0aaaaaaa", "sub  %A,%T",  /* 1xxx */
	"0001tttt1mmmnnnn", "sub  %M,%T%N",
	"0010tttt0aaaaaaa", "lac  %A,%T",  /* 2xxx */
	"0010tttt1mmmnnnn", "lac  %M,%T%N",
	"00110rrr0aaaaaaa", "lar  %R,%A",  /* 3xxx */
	"00110rrr1mmmnnnn", "lar  %R%M%N",
	"001110000aaaaaaa", "mpy  %A",     /* 38xx */
	"001110001mmmnnnn", "mpy  %M%N",
	"001110010aaaaaaa", "sqra %A",     /* 39xx */
	"001110011mmmnnnn", "sqra %M%N",
	"001110100aaaaaaa", "mpya %A",     /* 3Axx */
	"001110101mmmnnnn", "mpya %M%N",
	"001110110aaaaaaa", "mpys %A",     /* 3Bxx */
	"001110111mmmnnnn", "mpys %M%N",
	"001111000aaaaaaa", "lt   %A",     /* 3Cxx */
	"001111001mmmnnnn", "lt   %M%N",
	"001111010aaaaaaa", "lta  %A",     /* 3Dxx */
	"001111011mmmnnnn", "lta  %M%N",
	"001111100aaaaaaa", "ltp  %A",     /* 3Exx */
	"001111101mmmnnnn", "ltp  %M%N",
	"001111110aaaaaaa", "ltd  %A",     /* 3Fxx */
	"001111111mmmnnnn", "ltd  %M%N",
	"010000000aaaaaaa", "zalh %A",     /* 40xx */
	"010000001mmmnnnn", "zalh %M%N",
	"010000010aaaaaaa", "zals %A",     /* 41xx */
	"010000011mmmnnnn", "zals %M%N",
	"010000100aaaaaaa", "lact %A",     /* 42xx */
	"010000101mmmnnnn", "lact %M%N",
	"010000110aaaaaaa", "addc %A%S",   /* 43xx */
	"010000111mmmnnnn", "addc %M%S%N",
	"010001000aaaaaaa", "subh %A",     /* 44xx */
	"010001001mmmnnnn", "subh %M%N",
	"010001010aaaaaaa", "subs %A",     /* 45xx */
	"010001011mmmnnnn", "subs %M%N",
	"010001100aaaaaaa", "subt %A",     /* 46xx */
	"010001101mmmnnnn", "subt %M%N",
	"010001110aaaaaaa", "subc %A",     /* 47xx */
	"010001111mmmnnnn", "subc %M%N",
	"010010000aaaaaaa", "addh %A",     /* 48xx */
	"010010001mmmnnnn", "addh %M%N",
	"010010010aaaaaaa", "adds %A",     /* 49xx */
	"010010011mmmnnnn", "adds %M%N",
	"010010100aaaaaaa", "addt %A",     /* 4Axx */
	"010010101mmmnnnn", "addt %M%N",
	"010010110aaaaaaa", "rpt  %A",     /* 4Bxx */
	"010010111mmmnnnn", "rpt  %M%N",
	"010011000aaaaaaa", "xor  %A",     /* 4Cxx */
	"010011001mmmnnnn", "xor  %M%N",
	"010011010aaaaaaa", "or   %A",     /* 4Dxx */
	"010011011mmmnnnn", "or   %M%N",
	"010011100aaaaaaa", "and  %A",     /* 4Exx */
	"010011101mmmnnnn", "and  %M%N",
	"010011110aaaaaaa", "subb %A",     /* 4Fxx */
	"010011111mmmnnnn", "subb %M%N",
	"010100000aaaaaaa", "lst  %A",     /* 50xx */
	"010100001mmmnnnn", "lst  %M%N",
	"010100010aaaaaaa", "lst1 %A",     /* 51xx */
	"010100011mmmnnnn", "lst1 %M%N",
	"010100100aaaaaaa", "ldp  %A",     /* 52xx */
	"010100101mmmnnnn", "ldp  %M%N",
	"010100110aaaaaaa", "lph  %A",     /* 53xx */
	"010100111mmmnnnn", "lph  %M%N",
	"010101000aaaaaaa", "pshd %A",     /* 54xx */
	"010101001mmmnnnn", "pshd %M%N",

/*  "010101010aaaaaaa", "mar  %A",        55xx */
/*  MAR direct has been expanded out to all its variations because one of its */
/*  its opcodes is the same as NOP.  Actually MAR direct just performs a NOP */
		"0101010100000000", "nop",         /* 5500 */
		"0101010100000001", "mar  $01",
		"0101010100000010", "mar  $02",
		"0101010100000011", "mar  $03",
		"0101010100000100", "mar  $04",
		"0101010100000101", "mar  $05",
		"0101010100000110", "mar  $06",
		"0101010100000111", "mar  $07",
		"0101010100001000", "mar  $08",
		"0101010100001001", "mar  $09",
		"0101010100001010", "mar  $0A",
		"0101010100001011", "mar  $0B",
		"0101010100001100", "mar  $0C",
		"0101010100001101", "mar  $0D",
		"0101010100001110", "mar  $0E",
		"0101010100001111", "mar  $0F",
		"010101010001tttt", "mar  $1%T",
		"010101010010tttt", "mar  $2%T",
		"010101010011tttt", "mar  $3%T",
		"010101010100tttt", "mar  $4%T",
		"010101010101tttt", "mar  $5%T",
		"010101010110tttt", "mar  $6%T",
		"010101010111tttt", "mar  $7%T",

/*  "010101011mmmnnnn", "mar  %M%N",      55xx */
/*  MAR indirect has been expanded out to all its variations because one of */
/*  its opcodes, is the same as LARP (actually performs the same function) */
		"0101010110000xxx", "mar  *",      /* 558x */
		"0101010110001kkk", "larp %K",     /* 558x */
		"010101011001nnnn", "mar  *-%N",   /* 558x */
		"010101011010nnnn", "mar  *+%N",
		"010101011011nnnn", "mar  ??%N",
		"010101011100nnnn", "mar  *BR0-%N",
		"010101011101nnnn", "mar  *0-%N",
		"010101011110nnnn", "mar  *0+%N",
		"010101011111nnnn", "mar  *BR0+%N",

	"010101100aaaaaaa", "dmov %A",     /* 56xx */
	"010101101mmmnnnn", "dmov %M%N",
	"010101110aaaaaaa", "bitt %A",     /* 57xx */
	"010101111mmmnnnn", "bitt %M%N",
	"010110000aaaaaaa", "tblr %A",     /* 58xx */
	"010110001mmmnnnn", "tblr %M%N",
	"010110010aaaaaaa", "tblw %A",     /* 59xx */
	"010110011mmmnnnn", "tblw %M%N",
	"010110100aaaaaaa", "sqrs %A",     /* 5Axx */
	"010110101mmmnnnn", "sqrs %M%N",
	"010110110aaaaaaa", "lts  %A",     /* 5Bxx */
	"010110111mmmnnnn", "lts  %M%N",
	"010111000aaaaaaabbbbbbbbbbbbbbbb", "macd %B,%A",      /* 5Cxx */
	"010111001mmmnnnnbbbbbbbbbbbbbbbb", "macd %B,%M%N",
	"010111010aaaaaaabbbbbbbbbbbbbbbb", "mac  %B,%A",      /* 5Dxx */
	"010111011mmmnnnnbbbbbbbbbbbbbbbb", "mac  %B,%M%N",
	"010111101mmmnnnnbbbbbbbbbbbbbbbb", "bc   %B %M%N",    /* 5Exx */
	"010111111mmmnnnnbbbbbbbbbbbbbbbb", "bnc  %B %M%N",    /* 5Fxx */
	"01100sss0aaaaaaa", "sacl %A%S",   /* 6xxx */
	"01100sss1mmmnnnn", "sacl %M%S%N",
	"01101sss0aaaaaaa", "sach %A%S",   /* 6Xxx */
	"01101sss1mmmnnnn", "sach %M%S%N",
	"01110rrr0aaaaaaa", "sar  %R,%A",  /* 7xxx */
	"01110rrr1mmmnnnn", "sar  %R%M%N",
	"011110000aaaaaaa", "sst  %A",     /* 78xx */
	"011110001mmmnnnn", "sst  %M%N",
	"011110010aaaaaaa", "sst1 %A",     /* 79xx */
	"011110011mmmnnnn", "sst1 %M%N",
	"011110100aaaaaaa", "popd %A",     /* 7Axx */
	"011110101mmmnnnn", "popd %M%N",
	"011110110aaaaaaa", "zalr %A",     /* 7Bxx */
	"011110111mmmnnnn", "zalr %M%N",
	"011111000aaaaaaa", "spl  %A",     /* 7Cxx */
	"011111001mmmnnnn", "spl  %M%N",
	"011111010aaaaaaa", "sph  %A",     /* 7Dxx */
	"011111011mmmnnnn", "sph  %M%N",
	"011111100aaaaaaa", "adrk %A",     /* 7Exx */
	"011111101mmmnnnn", "adrk %M%N",
	"011111110aaaaaaa", "sbrk %A",     /* 7Fxx */
	"011111111mmmnnnn", "sbrk %M%N",
	"1000pppp0aaaaaaa", "in   %A,%P",  /* 8xxx */
	"1000pppp1mmmnnnn", "in   %M,%P%N",
	"1001tttt0aaaaaaa", "bit  %A,%T",  /* 9xxx */
	"1001tttt1mmmnnnn", "bit  %M,%T%N",
	"101wwwwwwwwwwwww", "mpyk %W",     /* Axxx-Bxxx */
	"11000rrrdddddddd", "lark %R,%D",  /* Cxxx */
	"1100100kdddddddd", "ldpk %K%D",   /* Cxxx */
/*  "11001010dddddddd", "lack %D",        CAxx */
/*  LACK has been expanded out to all its variations because one of its */
/*  its opcodes is the same as ZAC. Actually, it performs the same function */
		"1100101000000000", "zac",         /* CA00 */
		"1100101000000001", "lack 01h",    /* CAxx */
		"1100101000000010", "lack 02h",
		"1100101000000011", "lack 03h",
		"1100101000000100", "lack 04h",
		"1100101000000101", "lack 05h",
		"1100101000000110", "lack 06h",
		"1100101000000111", "lack 07h",
		"1100101000001000", "lack 08h",
		"1100101000001001", "lack 09h",
		"1100101000001010", "lack 0Ah",
		"1100101000001011", "lack 0Bh",
		"1100101000001100", "lack 0Ch",
		"1100101000001101", "lack 0Dh",
		"1100101000001110", "lack 0Eh",
		"1100101000001111", "lack 0Fh",
		"110010100001tttt", "lack 1%T",
		"110010100010tttt", "lack 2%T",
		"110010100011tttt", "lack 3%T",
		"110010100100tttt", "lack 4%T",
		"110010100101tttt", "lack 5%T",
		"110010100110tttt", "lack 6%T",
		"110010100111tttt", "lack 7%T",
		"110010101000tttt", "lack 8%T",
		"110010101001tttt", "lack 9%T",
		"110010101010tttt", "lack A%T",
		"110010101011tttt", "lack B%T",
		"110010101100tttt", "lack C%T",
		"110010101101tttt", "lack D%T",
		"110010101110tttt", "lack E%T",
		"110010101111tttt", "lack F%T",

	"11001011dddddddd", "rptk %D",     /* CBxx */
	"11001100dddddddd", "addk %D",     /* CCxx */
	"11001101dddddddd", "subk %D",     /* CDxx */
	"1100111000000000", "eint",        /* CE00 */
	"1100111000000001", "dint",        /* CE01 */
	"1100111000000010", "rovm",        /* CE02 */
	"1100111000000011", "sovm",        /* CE03 */
	"1100111000000100", "cnfd",        /* CE04 */
	"1100111000000101", "cnfp",        /* CE05 */
	"1100111000000110", "rsxm",        /* CE06 */
	"1100111000000111", "ssxm",        /* CE07 */
	"11001110000010kk", "spm  %K",     /* CE0x */
	"1100111000001100", "rxf",         /* CE0C */
	"1100111000001101", "sxf",         /* CE0D */
	"110011100000111k", "fort %K",     /* CE0x */
	"1100111000010100", "pac",         /* CE14 */
	"1100111000010101", "apac",        /* CE15 */
	"1100111000010110", "spac",        /* CE16 */
	"1100111000011000", "sfl",         /* CE18 */
	"1100111000011001", "sfr",         /* CE19 */
	"1100111000011011", "abs",         /* CE1B */
	"1100111000011100", "push",        /* CE1C */
	"1100111000011101", "pop",         /* CE1D */
	"1100111000011110", "trap",        /* CE1E */
	"1100111000011111", "idle",        /* CE1F */
	"1100111000100000", "rtxm",        /* CE20 */
	"1100111000100001", "stxm",        /* CE21 */
	"1100111000100011", "neg",         /* CE23 */
	"1100111000100100", "cala",        /* CE24 */
	"1100111000100101", "bacc",        /* CE25 */
	"1100111000100110", "ret",         /* CE26 */
	"1100111000100111", "cmpl",        /* CE27 */
	"1100111000110000", "rc",          /* CE30 */
	"1100111000110001", "sc",          /* CE31 */
	"1100111000110010", "rtc",         /* CE32 */
	"1100111000110011", "stc",         /* CE33 */
	"1100111000110100", "rol",         /* CE34 */
	"1100111000110101", "ror",         /* CE35 */
	"1100111000110110", "rfsm",        /* CE36 */
	"1100111000110111", "sfsm",        /* CE37 */
	"1100111000111000", "rhm",         /* CE38 */
	"1100111000111001", "shm",         /* CE39 */
	"11001110001111kk", "conf %K",     /* CE3x */
	"11001110010100cc", "cmpr %C",     /* CE5x */
	"110011101mmm0010", "norm %M",     /* CEx2 */
	"110011110aaaaaaa", "mpys %A",     /* CFxx */
	"110011111mmmnnnn", "mpys %M%N",
	"11010rrr00000000wwwwwwwwwwwwwwww", "lrlk %R,%W",      /* Dx00 */
	"1101tttt00000001wwwwwwwwwwwwwwww", "lalk %W,%T",      /* Dx01 */
	"1101tttt00000010wwwwwwwwwwwwwwww", "adlk %W,%T",      /* Dx02 */
	"1101tttt00000011wwwwwwwwwwwwwwww", "sblk %W,%T",      /* Dx03 */
	"1101tttt00000100wwwwwwwwwwwwwwww", "andk %W,%T",      /* Dx04 */
	"1101tttt00000101wwwwwwwwwwwwwwww", "ork  %W,%T",      /* Dx05 */
	"1101tttt00000110wwwwwwwwwwwwwwww", "xork %W,%T",      /* Dx06 */
	"1110pppp0aaaaaaa", "out  %A,%P",  /* Exxx */
	"1110pppp1mmmnnnn", "out  %M,%P%N",
	"111100001mmmnnnnbbbbbbbbbbbbbbbb", "bv   %B %M%N",    /* F0xx */
	"111100011mmmnnnnbbbbbbbbbbbbbbbb", "bgz  %B %M%N",    /* F1xx */
	"111100101mmmnnnnbbbbbbbbbbbbbbbb", "blez %B %M%N",    /* F2xx */
	"111100111mmmnnnnbbbbbbbbbbbbbbbb", "blz  %B %M%N",    /* F3xx */
	"111101001mmmnnnnbbbbbbbbbbbbbbbb", "bgez %B %M%N",    /* F4xx */
	"111101011mmmnnnnbbbbbbbbbbbbbbbb", "bnz  %B %M%N",    /* F5xx */
	"111101101mmmnnnnbbbbbbbbbbbbbbbb", "bz   %B %M%N",    /* F6xx */
	"111101111mmmnnnnbbbbbbbbbbbbbbbb", "bnv  %B %M%N",    /* F7xx */
	"111110001mmmnnnnbbbbbbbbbbbbbbbb", "bbz  %B %M%N",    /* F8xx */
	"111110011mmmnnnnbbbbbbbbbbbbbbbb", "bbnz %B %M%N",    /* F9xx */
	"111110101mmmnnnnbbbbbbbbbbbbbbbb", "bioz %B %M%N",    /* FAxx */
	"111110111mmmnnnnbbbbbbbbbbbbbbbb", "banz %B %M%N",    /* FBxx */
	"111111000aaaaaaabbbbbbbbbbbbbbbb", "blkp %B,%A",      /* FCxx */
	"111111001mmmnnnnbbbbbbbbbbbbbbbb", "blkp %B,%M%N",
	"111111010aaaaaaabbbbbbbbbbbbbbbb", "blkd %B,%A",      /* FDxx */
	"111111011mmmnnnnbbbbbbbbbbbbbbbb", "blkd %B,%M%N",
	"111111101mmmnnnnbbbbbbbbbbbbbbbb", "call %B %M%N",    /* FExx */
	"111111111mmmnnnnbbbbbbbbbbbbbbbb", "b    %B %M%N",    /* FFxx */
	nullptr
};

tms32025_disassembler::tms32025_disassembler()
{
	const char *p;
	const char *const *ops;
	u16 mask, bits;
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
					bit--;
					break;
				default:
					throw std::logic_error(util::string_format("Invalid instruction encoding '%s %s'\n", ops[0],ops[1]));
			}
		}
		if (bit != -1 )
		{
			throw std::logic_error(util::string_format("not enough bits in encoding '%s %s' %d\n", ops[0],ops[1],bit));
		}
		while (isspace((uint8_t)*p)) p++;
		Op.emplace_back(mask, bits, *p, ops[0], ops[1]);

		ops += 2;
		i++;
	}
}

offs_t tms32025_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	uint32_t flags = 0;
	int a, b, c, d, k, m, n, p, r, s, t, w; /* these can all be filled in by parsing an instruction */
	int i;
	int op;
	int cnt = 1;
	int code;
	int bit;
	//char *buffertmp;
	const char *cp;             /* character pointer in OpFormats */

	op = -1;                /* no matching opcode */
	code = opcodes.r16(pc);
	for ( i = 0; i < int(Op.size()); i++)
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
		util::stream_format(stream, "???? dw %04Xh",code);
		return cnt | SUPPORTED;
	}
	//buffertmp = buffer;
	if (Op[op].extcode)
	{
		bit = 31;
		code <<= 16;
		code |= opcodes.r16(pc+1);
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
			case '\0': throw std::logic_error(util::string_format("premature end of parse string, opcode %x, bit = %d\n",code,bit));
		}
		cp++;
	}

	/* now traverse format string */
	cp = Op[op].fmt;

	if (!strncmp(cp, "cal", 3))
		flags = STEP_OVER;
	else if (!strncmp(cp, "ret", 3))
		flags = STEP_OUT;
	else if ((code & 0xfe000000) == 0x5e000000 || (code >= 0xf0000000 && code < 0xfc000000))
		flags = STEP_COND;

	while (*cp)
	{
		if (*cp == '%')
		{
			char num[30];
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
					throw std::logic_error(util::string_format("illegal escape character in format '%s'\n",Op[op].fmt));
			}
			stream << num;
		}
		else
		{
			stream << *cp++;
		}
	}
	return cnt | flags | SUPPORTED;
}

u32 tms32025_disassembler::opcode_alignment() const
{
	return 1;
}
