/*****************************************************************************
 *
 *   z80gbd.c
 *   Portable Z80 Gameboy disassembler
 *
 *   Copyright (C) 2000 by The MESS Team.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#include "debugger.h"
#include "z80gb.h"

enum e_mnemonics
{
	zADC,  zADD,  zAND,  zBIT,	zCALL, zCCF,  zCP,
	zCPL,  zDAA,  zDB,	 zDEC,	zDI,   zEI,   zHLT,
	zIN,   zINC,  zJP,	 zJR,	zLD,   zNOP,  zOR,
	zPOP,  zPUSH, zRES,  zRET,	zRETI, zRL,   zRLA,
	zRLC,  zRLCA, zRR,	 zRRA,	zRRC,  zRRCA, zRST,
	zSBC,  zSCF,  zSET,  zSLA,	zSLL,  zSRA,  zSRL,
	zSTOP, zSUB,  zXOR,  zSWAP
};

static const char *s_mnemonic[] =
{
	"adc", "add", "and", "bit", "call","ccf", "cp",
	"cpl", "daa", "db",  "dec", "di",  "ei",  "halt",
	"in",  "inc", "jp",  "jr",  "ld",  "nop", "or",
	"pop", "push","res", "ret", "reti","rl",  "rla",
	"rlc", "rlca","rr",  "rra", "rrc", "rrca","rst",
	"sbc", "scf", "set", "sla", "sll", "sra", "srl",
	"stop","sub", "xor", "swap"
};

#define _OVER DASMFLAG_STEP_OVER
#define _OUT  DASMFLAG_STEP_OUT

static const UINT32 s_flags[] = {
	0    ,0    ,0    ,0    ,_OVER,0    ,0    ,
	0    ,0    ,0    ,0    ,0    ,0    ,_OVER,
	0    ,0    ,0    ,0    ,0    ,0    ,0    ,
	0    ,0    ,0    ,_OUT ,_OUT ,0    ,0    ,
	0    ,0    ,0    ,0    ,0    ,0    ,_OVER,
	0    ,0    ,0    ,0    ,0    ,0    ,0    ,
	_OVER,0    ,0    ,0
};

typedef struct
{
	UINT8	mnemonic;
	const char *arguments;
}	z80gbdasm;

static z80gbdasm mnemonic_cb[256] = {
	{zRLC,"b"},     {zRLC,"c"},     {zRLC,"d"},     {zRLC,"e"},
	{zRLC,"h"},     {zRLC,"l"},     {zRLC,"(hl)"},  {zRLC,"a"},
	{zRRC,"b"},     {zRRC,"c"},     {zRRC,"d"},     {zRRC,"e"},
	{zRRC,"h"},     {zRRC,"l"},     {zRRC,"(hl)"},  {zRRC,"a"},
	{zRL,"b"},      {zRL,"c"},      {zRL,"d"},      {zRL,"e"},
	{zRL,"h"},      {zRL,"l"},      {zRL,"(hl)"},   {zRL,"a"},
	{zRR,"b"},      {zRR,"c"},      {zRR,"d"},      {zRR,"e"},
	{zRR,"h"},      {zRR,"l"},      {zRR,"(hl)"},   {zRR,"a"},
	{zSLA,"b"},     {zSLA,"c"},     {zSLA,"d"},     {zSLA,"e"},
	{zSLA,"h"},     {zSLA,"l"},     {zSLA,"(hl)"},  {zSLA,"a"},
	{zSRA,"b"},     {zSRA,"c"},     {zSRA,"d"},     {zSRA,"e"},
	{zSRA,"h"},     {zSRA,"l"},     {zSRA,"(hl)"},  {zSRA,"a"},
	{zSWAP,"b"},    {zSWAP,"c"},    {zSWAP,"d"},    {zSWAP,"e"},
	{zSWAP,"h"},    {zSWAP,"l"},    {zSWAP,"(hl)"}, {zSWAP,"a"},
	{zSRL,"b"},     {zSRL,"c"},     {zSRL,"d"},     {zSRL,"e"},
	{zSRL,"h"},     {zSRL,"l"},     {zSRL,"(hl)"},  {zSRL,"a"},
	{zBIT,"0,b"},   {zBIT,"0,c"},   {zBIT,"0,d"},   {zBIT,"0,e"},
	{zBIT,"0,h"},   {zBIT,"0,l"},   {zBIT,"0,(hl)"},{zBIT,"0,a"},
	{zBIT,"1,b"},   {zBIT,"1,c"},   {zBIT,"1,d"},   {zBIT,"1,e"},
	{zBIT,"1,h"},   {zBIT,"1,l"},   {zBIT,"1,(hl)"},{zBIT,"1,a"},
	{zBIT,"2,b"},   {zBIT,"2,c"},   {zBIT,"2,d"},   {zBIT,"2,e"},
	{zBIT,"2,h"},   {zBIT,"2,l"},   {zBIT,"2,(hl)"},{zBIT,"2,a"},
	{zBIT,"3,b"},   {zBIT,"3,c"},   {zBIT,"3,d"},   {zBIT,"3,e"},
	{zBIT,"3,h"},   {zBIT,"3,l"},   {zBIT,"3,(hl)"},{zBIT,"3,a"},
	{zBIT,"4,b"},   {zBIT,"4,c"},   {zBIT,"4,d"},   {zBIT,"4,e"},
	{zBIT,"4,h"},   {zBIT,"4,l"},   {zBIT,"4,(hl)"},{zBIT,"4,a"},
	{zBIT,"5,b"},   {zBIT,"5,c"},   {zBIT,"5,d"},   {zBIT,"5,e"},
	{zBIT,"5,h"},   {zBIT,"5,l"},   {zBIT,"5,(hl)"},{zBIT,"5,a"},
	{zBIT,"6,b"},   {zBIT,"6,c"},   {zBIT,"6,d"},   {zBIT,"6,e"},
	{zBIT,"6,h"},   {zBIT,"6,l"},   {zBIT,"6,(hl)"},{zBIT,"6,a"},
	{zBIT,"7,b"},   {zBIT,"7,c"},   {zBIT,"7,d"},   {zBIT,"7,e"},
	{zBIT,"7,h"},   {zBIT,"7,l"},   {zBIT,"7,(hl)"},{zBIT,"7,a"},
	{zRES,"0,b"},   {zRES,"0,c"},   {zRES,"0,d"},   {zRES,"0,e"},
	{zRES,"0,h"},   {zRES,"0,l"},   {zRES,"0,(hl)"},{zRES,"0,a"},
	{zRES,"1,b"},   {zRES,"1,c"},   {zRES,"1,d"},   {zRES,"1,e"},
	{zRES,"1,h"},   {zRES,"1,l"},   {zRES,"1,(hl)"},{zRES,"1,a"},
	{zRES,"2,b"},   {zRES,"2,c"},   {zRES,"2,d"},   {zRES,"2,e"},
	{zRES,"2,h"},   {zRES,"2,l"},   {zRES,"2,(hl)"},{zRES,"2,a"},
	{zRES,"3,b"},   {zRES,"3,c"},   {zRES,"3,d"},   {zRES,"3,e"},
	{zRES,"3,h"},   {zRES,"3,l"},   {zRES,"3,(hl)"},{zRES,"3,a"},
	{zRES,"4,b"},   {zRES,"4,c"},   {zRES,"4,d"},   {zRES,"4,e"},
	{zRES,"4,h"},   {zRES,"4,l"},   {zRES,"4,(hl)"},{zRES,"4,a"},
	{zRES,"5,b"},   {zRES,"5,c"},   {zRES,"5,d"},   {zRES,"5,e"},
	{zRES,"5,h"},   {zRES,"5,l"},   {zRES,"5,(hl)"},{zRES,"5,a"},
	{zRES,"6,b"},   {zRES,"6,c"},   {zRES,"6,d"},   {zRES,"6,e"},
	{zRES,"6,h"},   {zRES,"6,l"},   {zRES,"6,(hl)"},{zRES,"6,a"},
	{zRES,"7,b"},   {zRES,"7,c"},   {zRES,"7,d"},   {zRES,"7,e"},
	{zRES,"7,h"},   {zRES,"7,l"},   {zRES,"7,(hl)"},{zRES,"7,a"},
	{zSET,"0,b"},   {zSET,"0,c"},   {zSET,"0,d"},   {zSET,"0,e"},
	{zSET,"0,h"},   {zSET,"0,l"},   {zSET,"0,(hl)"},{zSET,"0,a"},
	{zSET,"1,b"},   {zSET,"1,c"},   {zSET,"1,d"},   {zSET,"1,e"},
	{zSET,"1,h"},   {zSET,"1,l"},   {zSET,"1,(hl)"},{zSET,"1,a"},
	{zSET,"2,b"},   {zSET,"2,c"},   {zSET,"2,d"},   {zSET,"2,e"},
	{zSET,"2,h"},   {zSET,"2,l"},   {zSET,"2,(hl)"},{zSET,"2,a"},
	{zSET,"3,b"},   {zSET,"3,c"},   {zSET,"3,d"},   {zSET,"3,e"},
	{zSET,"3,h"},   {zSET,"3,l"},   {zSET,"3,(hl)"},{zSET,"3,a"},
	{zSET,"4,b"},   {zSET,"4,c"},   {zSET,"4,d"},   {zSET,"4,e"},
	{zSET,"4,h"},   {zSET,"4,l"},   {zSET,"4,(hl)"},{zSET,"4,a"},
	{zSET,"5,b"},   {zSET,"5,c"},   {zSET,"5,d"},   {zSET,"5,e"},
	{zSET,"5,h"},   {zSET,"5,l"},   {zSET,"5,(hl)"},{zSET,"5,a"},
	{zSET,"6,b"},   {zSET,"6,c"},   {zSET,"6,d"},   {zSET,"6,e"},
	{zSET,"6,h"},   {zSET,"6,l"},   {zSET,"6,(hl)"},{zSET,"6,a"},
	{zSET,"7,b"},   {zSET,"7,c"},   {zSET,"7,d"},   {zSET,"7,e"},
	{zSET,"7,h"},   {zSET,"7,l"},   {zSET,"7,(hl)"},{zSET,"7,a"}
};

static z80gbdasm mnemonic_main[256]= {
	{zNOP,0},		{zLD,"bc,N"},   {zLD,"(bc),a"}, {zINC,"bc"},
	{zINC,"b"},     {zDEC,"b"},     {zLD,"b,B"},    {zRLCA,0},
	{zLD,"(W),sp"}, {zADD,"hl,bc"}, {zLD,"a,(bc)"}, {zDEC,"bc"},
	{zINC,"c"},     {zDEC,"c"},     {zLD,"c,B"},    {zRRCA,0},
	{zSTOP,0},		{zLD,"de,N"},   {zLD,"(de),a"}, {zINC,"de"},
	{zINC,"d"},     {zDEC,"d"},     {zLD,"d,B"},    {zRLA,0},
	{zJR,"O"},      {zADD,"hl,de"}, {zLD,"a,(de)"}, {zDEC,"de"},
	{zINC,"e"},     {zDEC,"e"},     {zLD,"e,B"},    {zRRA,0},
	{zJR,"nz,O"},   {zLD,"hl,N"},   {zLD,"(hl+),a"},{zINC,"hl"},
	{zINC,"h"},     {zDEC,"h"},     {zLD,"h,B"},    {zDAA,0},
	{zJR,"z,O"},    {zADD,"hl,hl"}, {zLD,"a,(hl+)"},{zDEC,"hl"},
	{zINC,"l"},     {zDEC,"l"},     {zLD,"l,B"},    {zCPL,0},
	{zJR,"nc,O"},   {zLD,"sp,N"},   {zLD,"(hl-),a"},{zINC,"sp"},
	{zINC,"(hl)"},  {zDEC,"(hl)"},  {zLD,"(hl),B"}, {zSCF,0},
	{zJR,"c,O"},    {zADD,"hl,sp"}, {zLD,"a,(hl-)"},{zDEC,"sp"},
	{zINC,"a"},     {zDEC,"a"},     {zLD,"a,B"},    {zCCF,0},
	{zLD,"b,b"},    {zLD,"b,c"},    {zLD,"b,d"},    {zLD,"b,e"},
	{zLD,"b,h"},    {zLD,"b,l"},    {zLD,"b,(hl)"}, {zLD,"b,a"},
	{zLD,"c,b"},    {zLD,"c,c"},    {zLD,"c,d"},    {zLD,"c,e"},
	{zLD,"c,h"},    {zLD,"c,l"},    {zLD,"c,(hl)"}, {zLD,"c,a"},
	{zLD,"d,b"},    {zLD,"d,c"},    {zLD,"d,d"},    {zLD,"d,e"},
	{zLD,"d,h"},    {zLD,"d,l"},    {zLD,"d,(hl)"}, {zLD,"d,a"},
	{zLD,"e,b"},    {zLD,"e,c"},    {zLD,"e,d"},    {zLD,"e,e"},
	{zLD,"e,h"},    {zLD,"e,l"},    {zLD,"e,(hl)"}, {zLD,"e,a"},
	{zLD,"h,b"},    {zLD,"h,c"},    {zLD,"h,d"},    {zLD,"h,e"},
	{zLD,"h,h"},    {zLD,"h,l"},    {zLD,"h,(hl)"}, {zLD,"h,a"},
	{zLD,"l,b"},    {zLD,"l,c"},    {zLD,"l,d"},    {zLD,"l,e"},
	{zLD,"l,h"},    {zLD,"l,l"},    {zLD,"l,(hl)"}, {zLD,"l,a"},
	{zLD,"(hl),b"}, {zLD,"(hl),c"}, {zLD,"(hl),d"}, {zLD,"(hl),e"},
	{zLD,"(hl),h"}, {zLD,"(hl),l"}, {zHLT,0},       {zLD,"(hl),a"},
	{zLD,"a,b"},    {zLD,"a,c"},    {zLD,"a,d"},    {zLD,"a,e"},
	{zLD,"a,h"},    {zLD,"a,l"},    {zLD,"a,(hl)"}, {zLD,"a,a"},
	{zADD,"a,b"},   {zADD,"a,c"},   {zADD,"a,d"},   {zADD,"a,e"},
	{zADD,"a,h"},   {zADD,"a,l"},   {zADD,"a,(hl)"},{zADD,"a,a"},
	{zADC,"a,b"},   {zADC,"a,c"},   {zADC,"a,d"},   {zADC,"a,e"},
	{zADC,"a,h"},   {zADC,"a,l"},   {zADC,"a,(hl)"},{zADC,"a,a"},
	{zSUB,"b"},     {zSUB,"c"},     {zSUB,"d"},     {zSUB,"e"},
	{zSUB,"h"},     {zSUB,"l"},     {zSUB,"(hl)"},  {zSUB,"a"},
	{zSBC,"a,b"},   {zSBC,"a,c"},   {zSBC,"a,d"},   {zSBC,"a,e"},
	{zSBC,"a,h"},   {zSBC,"a,l"},   {zSBC,"a,(hl)"},{zSBC,"a,a"},
	{zAND,"b"},     {zAND,"c"},     {zAND,"d"},     {zAND,"e"},
	{zAND,"h"},     {zAND,"l"},     {zAND,"(hl)"},  {zAND,"a"},
	{zXOR,"b"},     {zXOR,"c"},     {zXOR,"d"},     {zXOR,"e"},
	{zXOR,"h"},     {zXOR,"l"},     {zXOR,"(hl)"},  {zXOR,"a"},
	{zOR,"b"},      {zOR,"c"},      {zOR,"d"},      {zOR,"e"},
	{zOR,"h"},      {zOR,"l"},      {zOR,"(hl)"},   {zOR,"a"},
	{zCP,"b"},      {zCP,"c"},      {zCP,"d"},      {zCP,"e"},
	{zCP,"h"},      {zCP,"l"},      {zCP,"(hl)"},   {zCP,"a"},
	{zRET,"nz"},    {zPOP,"bc"},    {zJP,"nz,A"},   {zJP,"A"},
	{zCALL,"nz,A"}, {zPUSH,"bc"},   {zADD,"a,B"},   {zRST,"V"},
	{zRET,"z"},     {zRET,0},       {zJP,"z,A"},    {zDB,"cb"},
	{zCALL,"z,A"},  {zCALL,"A"},    {zADC,"a,B"},   {zRST,"V"},
	{zRET,"nc"},    {zPOP,"de"},    {zJP,"nc,A"},   {zDB,"d3"},
	{zCALL,"nc,A"}, {zPUSH,"de"},   {zSUB,"B"},     {zRST,"V"},
	{zRET,"c"},     {zRETI,0},      {zJP,"c,A"},    {zDB,"db"},
	{zCALL,"c,A"},  {zDB,"dd"},     {zSBC,"a,B"},   {zRST,"V"},
	{zLD,"(F),a"},  {zPOP,"hl"},    {zLD,"(C),a"},  {zDB,"e3"},
	{zDB,"e4"},     {zPUSH,"hl"},   {zAND,"B"},     {zRST,"V"},
	{zADD,"SP,B"},  {zJP,"(hl)"},   {zLD,"(W),a"},  {zDB,"eb"},
	{zDB,"ec"},     {zDB,"ed"},     {zXOR,"B"},     {zRST,"V"},
	{zLD,"a,(F)"},  {zPOP,"af"},    {zLD,"a,(C)"},  {zDI,0},
	{zDB,"f4"},     {zPUSH,"af"},   {zOR,"B"},      {zRST,"V"},
	{zLD,"hl,sp+B"},{zLD,"sp,hl"},  {zLD,"a,(W)"},  {zEI,0},
	{zDB,"fc"},     {zDB,"fd"},     {zCP,"B"},      {zRST,"V"}
};

/****************************************************************************
 * Disassemble opcode at PC and return number of bytes it takes
 ****************************************************************************/

unsigned z80gb_dasm( char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram )
{
	z80gbdasm *d;
	const char *symbol, *src;
	char *dst;
	INT8 offset = 0;
	UINT8 op, op1;
	UINT16 ea = 0;
	int pos = 0;

	dst = buffer;
	symbol = NULL;

	op = oprom[pos++];
	op1 = 0; /* keep GCC happy */

	if( op == 0xcb ) {
		op = oprom[pos++];
		d = &mnemonic_cb[op];
	} else {
		d = &mnemonic_main[op];
	}

	if( d->arguments ) {
		dst += sprintf(dst, "%-4s ", s_mnemonic[d->mnemonic]);
		src = d->arguments;
		while( *src ) {
			switch( *src ) {
			case '?':   /* illegal opcode */
				dst += sprintf( dst, "$%02X,$%02X", op, op1);
				break;
			case 'A':
				ea = opram[pos] + ( opram[pos+1] << 8);
				pos += 2;
				dst += sprintf( dst, "$%04X", ea );
				break;
			case 'B':   /* Byte op arg */
				ea = opram[pos++];
				dst += sprintf( dst, "$%02X", ea );
				break;
			case '(':   /* Memory byte at (...) */
				*dst++ = *src;
				if( !strncmp( src, "(bc)", 4) ) {
				} else if( !strncmp( src, "(de)", 4) ) {
				} else if( !strncmp( src, "(hl)", 4) ) {
				} else if( !strncmp( src, "(sp)", 4) ) {
				} else if( !strncmp( src, "(F)", 3) ) {
					ea = 0xFF00 + opram[pos++];
					dst += sprintf( dst, "$%02X", ea );
					src++;
				} else if( !strncmp( src, "(C)", 3) ) {
					dst += sprintf( dst, "$FF00+c" );
					src++;
				}
				break;
			case 'N':   /* Immediate 16 bit */
				ea = opram[pos] + ( opram[pos+1] << 8 );
				pos += 2;
				dst += sprintf( dst, "$%04X", ea );
				break;
			case 'O':   /* Offset relative to PC */
				offset = (INT8) opram[pos++];
				dst += sprintf( dst, "$%04X", pc + offset + 2 );
				break;
			case 'V':   /* Restart vector */
				ea = op & 0x38;
				dst += sprintf( dst, "$%02X", ea );
				break;
			case 'W':   /* Memory address word */
				ea = opram[pos] + ( opram[pos+1] << 8 );
				pos += 2;
				dst += sprintf( dst, "$%04X", ea );
				break;
			default:
				*dst++ = *src;
			}
			src++;
		}
		*dst = '\0';
	} else {
		dst += sprintf(dst, "%s", s_mnemonic[d->mnemonic]);
	}

	return pos | s_flags[d->mnemonic] | DASMFLAG_SUPPORTED;
}
