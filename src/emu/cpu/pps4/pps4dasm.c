// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   pps4dasm.c
 *
 *   Rockwell PPS-4 CPU Disassembly
 *
 *
 * TODO: double verify all opcodes with t_Ixx flags
 *
 *****************************************************************************/
#include "emu.h"

#define OP(A)   oprom[(A) - PC]
#define ARG(A)  opram[(A) - PC]

typedef enum pps4_token_e {
	t_AD,       t_ADC,      t_ADSK,     t_ADCSK,    t_ADI,
	t_DC,       t_AND,      t_OR,       t_EOR,      t_COMP,
	t_SC,       t_RC,       t_SF1,      t_RF1,      t_SF2,
	t_RF2,      t_LD,       t_EX,       t_EXD,      t_LDI,
	t_LAX,      t_LXA,      t_LABL,     t_LBMX,     t_LBUA,
	t_XABL,     t_XBMX,     t_XAX,      t_XS,       t_CYS,
	t_LB,       t_LBL,      t_INCB,     t_DECB,     t_T,
	t_TM,       t_TL,       t_TML,      t_SKC,      t_SKZ,
	t_SKBI,     t_SKF1,     t_SKF2,     t_RTN,      t_RTNSK,
	t_IOL,      t_DIA,      t_DIB,      t_DOA,      t_SAG,
	t_COUNT,
	t_MASK = (1 << 6) - 1,
	t_I3c  = 1 <<  6,   /* immediate 3 bit constant, complemented */
	t_I4   = 1 <<  7,   /* immediate 4 bit constant */
	t_I4c  = 1 <<  8,   /* immediate 4 bit constant, complemented */
	t_I4p  = 1 <<  9,   /* immediate 4 bit offset into page 3 */
	t_I6p  = 1 << 10,   /* immediate 6 bit constant; address in current page */
	t_I6i  = 1 << 11,   /* immediate 6 bit indirect page 3 offset (16 ... 63) + followed by page 1 address */
	t_I8   = 1 << 12,   /* immediate 8 bit constant (I/O port number) */
	t_I8c  = 1 << 13,   /* immediate 8 bit constant inverted */
	t_OVER = 1 << 14,   /* Debugger step over (CALL) */
	t_OUT  = 1 << 15    /* Debugger step out (RETURN) */
}   pps4_token_e;

static const char *token_str[t_COUNT] = {
	"ad",           /* add */
	"adc",          /* add with carry-in */
	"adsk",         /* add and skip on carry-out */
	"adcsk",        /* add with carry-in and skip on carry-out */
	"adi",          /* add immediate */
	"dc",           /* decimal correction */
	"and",          /* logical and */
	"or",           /* logical or */
	"eor",          /* logical exclusive-orf */
	"comp",         /* complement */
	"sc",           /* set C flip-flop */
	"rc",           /* reset C flip-flop */
	"sf1",          /* set FF1 flip-flop */
	"rf1",          /* reset FF1 flip-flop */
	"sf2",          /* set FF2 flip-flop */
	"rf2",          /* reset FF2 flip-flop */
	"ld",           /* load accumulator from memory */
	"ex",           /* exchange accumulator and memory */
	"exd",          /* exchange accumulator and memory and decrement BL */
	"ldi",          /* load accumulator immediate */
	"lax",          /* load accumulator from X register */
	"lxa",          /* load X register from accumulator */
	"labl",         /* load accumulator with BL */
	"lbmx",         /* load BM with X */
	"lbua",         /* load BU with A */
	"xabl",         /* exchange accumulator and BL */
	"xbmx",         /* exchange BM and X */
	"xax",          /* exchange accumulator and X */
	"xs",           /* exchange SA and SB */
	"cys",          /* cycle SA register and accumulator */
	"lb",           /* load B indirect */
	"lbl",          /* load B long */
	"incb",         /* increment BL */
	"decb",         /* decrement BL */
	"t",            /* transfer */
	"tm",           /* transfer and mark indirect */
	"tl",           /* transfer long */
	"tml",          /* transfer and mark long */
	"skc",          /* skip on C flip-flop equals 1 */
	"skz",          /* skip on accumulator zero */
	"skbi",         /* skip on BL equal to immediate */
	"skf1",         /* skip on FF1 flip-flop equals 1 */
	"skf2",         /* skip on FF2 flip-flop equals 1 */
	"rtn",          /* return */
	"rtnsk",        /* return and skip */
	"iol",          /* input/output long */
	"dia",          /* discrete input group A */
	"dib",          /* discrete input group B */
	"doa",          /* discrete output */
	"sag"           /* special address generation */
};

static const UINT16 table[] = {
/* 00 */ t_LBL | t_I8c,
/* 01 */ t_TML | t_I4 | t_I8,
/* 02 */ t_TML | t_I4 | t_I8,
/* 03 */ t_TML | t_I4 | t_I8,
/* 04 */ t_LBUA,
/* 05 */ t_RTN | t_OUT,
/* 06 */ t_XS,
/* 07 */ t_RTNSK | t_OUT,
/* 08 */ t_ADCSK,
/* 09 */ t_ADSK,
/* 0a */ t_ADC,
/* 0b */ t_AD,
/* 0c */ t_EOR,
/* 0d */ t_AND,
/* 0e */ t_COMP,
/* 0f */ t_OR,

/* 10 */ t_LBMX,
/* 11 */ t_LABL,
/* 12 */ t_LAX,
/* 13 */ t_SAG,
/* 14 */ t_SKF2,
/* 15 */ t_SKC,
/* 16 */ t_SKF1,
/* 17 */ t_INCB,
/* 18 */ t_XBMX,
/* 19 */ t_XABL,
/* 1a */ t_XAX,
/* 1b */ t_LXA,
/* 1c */ t_IOL | t_I8,
/* 1d */ t_DOA,
/* 1e */ t_SKZ,
/* 1f */ t_DECB,

/* 20 */ t_SC,
/* 21 */ t_SF2,
/* 22 */ t_SF1,
/* 23 */ t_DIB,
/* 24 */ t_RC,
/* 25 */ t_RF2,
/* 26 */ t_RF1,
/* 27 */ t_DIA,
/* 28 */ t_EXD | t_I3c,
/* 29 */ t_EXD | t_I3c,
/* 2a */ t_EXD | t_I3c,
/* 2b */ t_EXD | t_I3c,
/* 2c */ t_EXD | t_I3c,
/* 2d */ t_EXD | t_I3c,
/* 2e */ t_EXD | t_I3c,
/* 2f */ t_EXD | t_I3c,

/* 30 */ t_LD | t_I3c,
/* 31 */ t_LD | t_I3c,
/* 32 */ t_LD | t_I3c,
/* 33 */ t_LD | t_I3c,
/* 34 */ t_LD | t_I3c,
/* 35 */ t_LD | t_I3c,
/* 36 */ t_LD | t_I3c,
/* 37 */ t_LD | t_I3c,
/* 38 */ t_EX | t_I3c,
/* 39 */ t_EX | t_I3c,
/* 3a */ t_EX | t_I3c,
/* 3b */ t_EX | t_I3c,
/* 3c */ t_EX | t_I3c,
/* 3d */ t_EX | t_I3c,
/* 3e */ t_EX | t_I3c,
/* 3f */ t_EX | t_I3c,

/* 40 */ t_SKBI | t_I4,
/* 41 */ t_SKBI | t_I4,
/* 42 */ t_SKBI | t_I4,
/* 43 */ t_SKBI | t_I4,
/* 44 */ t_SKBI | t_I4,
/* 45 */ t_SKBI | t_I4,
/* 46 */ t_SKBI | t_I4,
/* 47 */ t_SKBI | t_I4,
/* 48 */ t_SKBI | t_I4,
/* 49 */ t_SKBI | t_I4,
/* 4a */ t_SKBI | t_I4,
/* 4b */ t_SKBI | t_I4,
/* 4c */ t_SKBI | t_I4,
/* 4d */ t_SKBI | t_I4,
/* 4e */ t_SKBI | t_I4,
/* 4f */ t_SKBI | t_I4,

/* 50 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 51 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 52 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 53 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 54 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 55 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 56 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 57 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 58 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 59 */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5a */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5b */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5c */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5d */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5e */ t_TL | t_I4 | t_I8 | t_OVER,
/* 5f */ t_TL | t_I4 | t_I8 | t_OVER,

/* 60 */ t_ADI | t_I4c,
/* 61 */ t_ADI | t_I4c,
/* 62 */ t_ADI | t_I4c,
/* 63 */ t_ADI | t_I4c,
/* 64 */ t_ADI | t_I4c,
/* 65 */ t_DC,
/* 66 */ t_ADI | t_I4c,
/* 67 */ t_ADI | t_I4c,
/* 68 */ t_ADI | t_I4c,
/* 69 */ t_ADI | t_I4c,
/* 6a */ t_ADI | t_I4c,
/* 6b */ t_ADI | t_I4c,
/* 6c */ t_ADI | t_I4c,
/* 6d */ t_ADI | t_I4c,
/* 6e */ t_ADI | t_I4c,
/* 6f */ t_CYS,

/* 70 */ t_LDI | t_I4c,
/* 71 */ t_LDI | t_I4c,
/* 72 */ t_LDI | t_I4c,
/* 73 */ t_LDI | t_I4c,
/* 74 */ t_LDI | t_I4c,
/* 75 */ t_LDI | t_I4c,
/* 76 */ t_LDI | t_I4c,
/* 77 */ t_LDI | t_I4c,
/* 78 */ t_LDI | t_I4c,
/* 79 */ t_LDI | t_I4c,
/* 7a */ t_LDI | t_I4c,
/* 7b */ t_LDI | t_I4c,
/* 7c */ t_LDI | t_I4c,
/* 7d */ t_LDI | t_I4c,
/* 7e */ t_LDI | t_I4c,
/* 7f */ t_LDI | t_I4c,

/* 80 */ t_T | t_I6p,
/* 81 */ t_T | t_I6p,
/* 82 */ t_T | t_I6p,
/* 83 */ t_T | t_I6p,
/* 84 */ t_T | t_I6p,
/* 85 */ t_T | t_I6p,
/* 86 */ t_T | t_I6p,
/* 87 */ t_T | t_I6p,
/* 88 */ t_T | t_I6p,
/* 89 */ t_T | t_I6p,
/* 8a */ t_T | t_I6p,
/* 8b */ t_T | t_I6p,
/* 8c */ t_T | t_I6p,
/* 8d */ t_T | t_I6p,
/* 8e */ t_T | t_I6p,
/* 8f */ t_T | t_I6p,

/* 90 */ t_T | t_I6p,
/* 91 */ t_T | t_I6p,
/* 92 */ t_T | t_I6p,
/* 93 */ t_T | t_I6p,
/* 94 */ t_T | t_I6p,
/* 95 */ t_T | t_I6p,
/* 96 */ t_T | t_I6p,
/* 97 */ t_T | t_I6p,
/* 98 */ t_T | t_I6p,
/* 99 */ t_T | t_I6p,
/* 9a */ t_T | t_I6p,
/* 9b */ t_T | t_I6p,
/* 9c */ t_T | t_I6p,
/* 9d */ t_T | t_I6p,
/* 9e */ t_T | t_I6p,
/* 9f */ t_T | t_I6p,

/* a0 */ t_T | t_I6p,
/* a1 */ t_T | t_I6p,
/* a2 */ t_T | t_I6p,
/* a3 */ t_T | t_I6p,
/* a4 */ t_T | t_I6p,
/* a5 */ t_T | t_I6p,
/* a6 */ t_T | t_I6p,
/* a7 */ t_T | t_I6p,
/* a8 */ t_T | t_I6p,
/* a9 */ t_T | t_I6p,
/* aa */ t_T | t_I6p,
/* ab */ t_T | t_I6p,
/* ac */ t_T | t_I6p,
/* ad */ t_T | t_I6p,
/* ae */ t_T | t_I6p,
/* af */ t_T | t_I6p,

/* b0 */ t_T | t_I6p,
/* b1 */ t_T | t_I6p,
/* b2 */ t_T | t_I6p,
/* b3 */ t_T | t_I6p,
/* b4 */ t_T | t_I6p,
/* b5 */ t_T | t_I6p,
/* b6 */ t_T | t_I6p,
/* b7 */ t_T | t_I6p,
/* b8 */ t_T | t_I6p,
/* b9 */ t_T | t_I6p,
/* ba */ t_T | t_I6p,
/* bb */ t_T | t_I6p,
/* bc */ t_T | t_I6p,
/* bd */ t_T | t_I6p,
/* be */ t_T | t_I6p,
/* bf */ t_T | t_I6p,

/* c0 */ t_LB | t_I4p,
/* c1 */ t_LB | t_I4p,
/* c2 */ t_LB | t_I4p,
/* c3 */ t_LB | t_I4p,
/* c4 */ t_LB | t_I4p,
/* c5 */ t_LB | t_I4p,
/* c6 */ t_LB | t_I4p,
/* c7 */ t_LB | t_I4p,
/* c8 */ t_LB | t_I4p,
/* c9 */ t_LB | t_I4p,
/* ca */ t_LB | t_I4p,
/* cb */ t_LB | t_I4p,
/* cc */ t_LB | t_I4p,
/* cd */ t_LB | t_I4p,
/* ce */ t_LB | t_I4p,
/* cf */ t_LB | t_I4p,

/* d0 */ t_TM | t_I6i | t_OVER,
/* d1 */ t_TM | t_I6i | t_OVER,
/* d2 */ t_TM | t_I6i | t_OVER,
/* d3 */ t_TM | t_I6i | t_OVER,
/* d4 */ t_TM | t_I6i | t_OVER,
/* d5 */ t_TM | t_I6i | t_OVER,
/* d6 */ t_TM | t_I6i | t_OVER,
/* d7 */ t_TM | t_I6i | t_OVER,
/* d8 */ t_TM | t_I6i | t_OVER,
/* d9 */ t_TM | t_I6i | t_OVER,
/* da */ t_TM | t_I6i | t_OVER,
/* db */ t_TM | t_I6i | t_OVER,
/* dc */ t_TM | t_I6i | t_OVER,
/* dd */ t_TM | t_I6i | t_OVER,
/* de */ t_TM | t_I6i | t_OVER,
/* df */ t_TM | t_I6i | t_OVER,

/* e0 */ t_TM | t_I6i | t_OVER,
/* e1 */ t_TM | t_I6i | t_OVER,
/* e2 */ t_TM | t_I6i | t_OVER,
/* e3 */ t_TM | t_I6i | t_OVER,
/* e4 */ t_TM | t_I6i | t_OVER,
/* e5 */ t_TM | t_I6i | t_OVER,
/* e6 */ t_TM | t_I6i | t_OVER,
/* e7 */ t_TM | t_I6i | t_OVER,
/* e8 */ t_TM | t_I6i | t_OVER,
/* e9 */ t_TM | t_I6i | t_OVER,
/* ea */ t_TM | t_I6i | t_OVER,
/* eb */ t_TM | t_I6i | t_OVER,
/* ec */ t_TM | t_I6i | t_OVER,
/* ed */ t_TM | t_I6i | t_OVER,
/* ee */ t_TM | t_I6i | t_OVER,
/* ef */ t_TM | t_I6i | t_OVER,

/* f0 */ t_TM | t_I6i | t_OVER,
/* f1 */ t_TM | t_I6i | t_OVER,
/* f2 */ t_TM | t_I6i | t_OVER,
/* f3 */ t_TM | t_I6i | t_OVER,
/* f4 */ t_TM | t_I6i | t_OVER,
/* f5 */ t_TM | t_I6i | t_OVER,
/* f6 */ t_TM | t_I6i | t_OVER,
/* f7 */ t_TM | t_I6i | t_OVER,
/* f8 */ t_TM | t_I6i | t_OVER,
/* f9 */ t_TM | t_I6i | t_OVER,
/* fa */ t_TM | t_I6i | t_OVER,
/* fb */ t_TM | t_I6i | t_OVER,
/* fc */ t_TM | t_I6i | t_OVER,
/* fd */ t_TM | t_I6i | t_OVER,
/* fe */ t_TM | t_I6i | t_OVER,
/* ff */ t_TM | t_I6i | t_OVER
};

CPU_DISASSEMBLE( pps4 )
{
	UINT32 flags = 0;
	unsigned PC = pc;
	UINT8 op = OP(pc++);
	UINT32 tok = table[op];
	char *dst = 0;

	if (0 == (tok & t_MASK)) {
		sprintf(buffer, "%s", token_str[tok & t_MASK]);
	} else {
		dst = buffer + sprintf(buffer, "%-7s", token_str[tok & t_MASK]);
	}

	if (tok & t_I3c) {
		// 3 bit immediate, complemented
		UINT8 i = ~op & 7;
		if (0 != i)  // only print if non-zero
			dst += sprintf(dst, "%x", i);
	}

	if (tok & t_I4) {
		// 4 bit immediate
		UINT8 i = op & 15;
		dst += sprintf(dst, "%x", i);
	}

	if (tok & t_I4c) {
		// 4 bit immediate, complemented
		UINT8 i = ~op & 15;
		dst += sprintf(dst, "%x", i);
	}

	if (tok & t_I4p) {
		// 4 bit immediate offset into page 3
		UINT8 i = op & 15;
		dst += sprintf(dst, "[%x]", 0x0c0 | i);
	}

	if (tok & t_I6p) {
		// 6 bit immediate offset into current page
		UINT8 i = op & 63;
		dst += sprintf(dst, "%x", (PC & ~63) | i);
	}

	if (tok & t_I6i) {
		// 6 bit immediate offset into page 3
		UINT16 i6p3 = (3 << 6) | (op & 63);
		// 8 bit absolute offset at 0x0100
		UINT16 addr = (1 << 8) | 0;     // ROM[ip3] can't be reached!?
		(void)addr; // avoid unused variable warning
		dst += sprintf(dst, "[%x]", i6p3);
	}

	if (tok & t_I8) {
		// 8 bit immediate I/O port address
		UINT8 arg = ARG(pc++);
		dst += sprintf(dst, "%02x", arg);
	}

	if (tok & t_I8c) {
		// 8 bit immediate offset into page
		UINT16 arg = ~ARG(pc++) & 255;
		dst += sprintf(dst, "%02x", arg);
	}

	if (tok & t_OVER)  // TL or TML
			flags |= DASMFLAG_STEP_OVER;

	if (tok & t_OUT)   // RTN or RTNSK
			flags |= DASMFLAG_STEP_OUT;

	return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}
