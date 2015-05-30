// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    cubedasm.c

    Implementation of the Cube Quest AM2901-based CPUs

***************************************************************************/

#include "emu.h"
#include "cubeqcpu.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* Am2901 Instruction Fields */
static const char *const ins[] =
{
	"ADD  ",
	"SUBR ",
	"SUBS ",
	"OR   ",
	"AND  ",
	"NOTRS",
	"EXOR ",
	"EXNOR",
};

static const char *const src[] =
{
	"A,Q",
	"A,B",
	"0,Q",
	"0,B",
	"0,A",
	"D,A",
	"D,Q",
	"D,0",
};

static const char *const dst[] =
{
	"QREG ",
	"NOP  ",
	"RAMA ",
	"RAMF ",
	"RAMQD",
	"RAMD ",
	"RAMQU",
	"RAMU ",
};


/***************************************************************************
    SOUND DISASSEMBLY HOOK
***************************************************************************/

CPU_DISASSEMBLE( cquestsnd )
{
	static const char *const jmps[] =
	{
		"JUMP ",
		"     ",
		"JMSB ",
		"JNMSB",
		"     ",
		"JZERO",
		"JOVR ",
		"JLOOP",
	};


	static const char *const latches[] =
	{
		"PLTCH  ",
		"DAC    ",
		"ADLATCH",
		"       ",
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
	UINT32 inslow = inst & 0xffffffff;
	UINT32 inshig = inst >> 32;

	int t       = (inshig >> 24) & 0xff;
	int b       = (inshig >> 20) & 0xf;
	int a       = (inshig >> 16) & 0xf;
	int ci      = (inshig >> 15) & 1;
	int i5_3    = (inshig >> 12) & 7;
	int _ramen  = (inshig >> 11) & 1;
	int i2_0    = (inshig >> 8) & 7;
	int rtnltch = (inshig >> 7) & 1;
	int jmp     = (inshig >> 4) & 7;
	int inca    = (inshig >> 3) & 1;
	int i8_6    = (inshig >> 0) & 7;
	int _ipram  = (inslow >> 31) & 1;
	int _ipwrt  = (inslow >> 30) & 1;
	int latch   = (inslow >> 28) & 3;
	int rtn     = (inslow >> 27) & 1;
	int _rin    = (inslow >> 26) & 1;


	sprintf(buffer, "%s %s %s %x,%x,%c %.2x %s %s %.2x %s %s %s %c %c %c",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			_rin,
			jmps[jmp],
			rtn ? "RET" : "   ",
			t,
			latches[latch],
			rtnltch ? "RTLATCH" : "       ",
			_ramen ? "PROM" : "RAM ",
			_ipram ? ' ' : 'R',
			_ipwrt ? ' ' : 'W',
			inca ? 'I' : ' ');

	return 1 | DASMFLAG_SUPPORTED;
}


/***************************************************************************
    ROTATE DISASSEMBLY HOOK
***************************************************************************/

CPU_DISASSEMBLE( cquestrot )
{
	static const char *const jmps[] =
	{
		"       ",
		"JSEQ   ",
		"JC     ",
		"JSYNC  ",
		"JLDWAIT",
		"JMSB   ",
		"JGEONE ",
		"JZERO  ",

		"JUMP   ",
		"JNSEQ  ",
		"JNC    ",
		"JNSYNC ",
		"JNLDWAI",
		"JNMSB  ",
		"JLTONE ",
		"JNZERO ",
	};

	static const char *const youts[] =
	{
		"     ",
		"     ",
		"Y2LDA",
		"Y2LDD",
		"Y2DAD",
		"Y2DIN",
		"Y2R  ",
		"Y2D  ",
	};

	static const char *const spfs[] =
	{
		"      ",
		"      ",
		"OP    ",
		"RET   ",
		"SQLTCH",
		"SWRT  ",
		"DIV   ",
		"MULT  ",

		"DRED  ",
		"DWRT  ",
		"???   ",
		"???   ",
		"???   ",
		"???   ",
		"???   ",
		"???   "
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
	UINT32 inslow = inst & 0xffffffff;
	UINT32 inshig = inst >> 32;

	int t       = (inshig >> 20) & 0xfff;
	int jmp     = (inshig >> 16) & 0xf;
	int spf     = (inshig >> 12) & 0xf;
//  int rsrc    = (inshig >> 11) & 0x1;
	int yout    = (inshig >> 8) & 0x7;
	int sel     = (inshig >> 6) & 0x3;
//  int dsrc    = (inshig >> 4) & 0x3;
	int b       = (inshig >> 0) & 0xf;
	int a       = (inslow >> 28) & 0xf;
	int i8_6    = (inslow >> 24) & 0x7;
	int ci      = (inslow >> 23) & 0x1;
	int i5_3    = (inslow >> 20) & 0x7;
//  int _sex    = (inslow >> 19) & 0x1;
	int i2_0    = (inslow >> 16) & 0x7;

	sprintf(buffer, "%s %s,%s %x,%x,%c %d %s %s %s %.2x",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			sel,
			jmps[jmp],
			youts[yout],
			spfs[spf],
			t);

	return 1 | DASMFLAG_SUPPORTED;
}

/***************************************************************************
    LINE DRAWER DISASSEMBLY HOOK
***************************************************************************/

CPU_DISASSEMBLE( cquestlin )
{
	static const char *const jmps[] =
	{
		"     ",
		"JMSB ",
		"JSEQ ",
		"JGTZ ",
		"JC   ",
		"JZ   ",
		"?????",
		"?????",

		"JUMP ",
		"JNMSB",
		"JNSEQ",
		"JLEZ ",
		"JNC  ",
		"JNZ  ",
		"?????",
		"?????",
	};

	static const char *const latches[] =
	{
		"       ",
		"SEQLTCH",
		"XLTCH  ",
		"YLTCH  ",
		"BGLTCH ",
		"FGLTCH ",
		"CLTCH  ",
		"ZLTCH  ",
	};

	static const char *const spfs[] =
	{
		"      ",
		"FSTOP ",
		"FREG  ",
		"FSTART",
		"PWRT  ",
		"MULT  ",
		"LSTOP ",
		"BRES  ",
	};

	UINT64 inst = BIG_ENDIANIZE_INT64(*(UINT64 *)oprom);
	UINT32 inslow = inst & 0xffffffff;
	UINT32 inshig = inst >> 32;

	int t       = (inshig >> 24) & 0xff;
	int jmp     = (inshig >> 20) & 0xf;
	int latch   = (inshig >> 16) & 0x7;
	int op      = (inshig >> 15) & 0x1;
	int spf     = (inshig >> 12) & 0x7;
	int b       = (inshig >> 8) & 0xf;
	int a       = (inshig >> 4) & 0xf;
	int i8_6    = (inshig >> 0) & 0x7;
	int ci      = (inslow >> 31) & 0x1;
	int i5_3    = (inslow >> 28) & 0x7;
	int _pbcs   = (inslow >> 27) & 0x1;
	int i2_0    = (inslow >> 24) & 0x7;

	sprintf(buffer, "%s %s,%s %x,%x %c %s %.2x %s %s %s %s",
			ins[i5_3],
			src[i2_0],
			dst[i8_6],
			a,
			b,
			ci ? 'C' : ' ',
			jmps[jmp],
			t,
			latches[latch],
			op ? "OP" : "  ",
			_pbcs ? "  " : "PB",
			spfs[spf]);

	return 1 | DASMFLAG_SUPPORTED;
}
