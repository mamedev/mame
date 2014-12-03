// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller <pullmoll@t-online.de>
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
    t_I3c = 1 << 6,   /* immediate 3 bit constant, complemented */
    t_I4  = 1 << 7,   /* immediate 4 bit constant */
    t_I4c = 1 << 8,   /* immediate 4 bit constant, complemented */
    t_I4p = 1 << 9,   /* immediate 4 bit offset into page 3 */
    t_I6p = 1 << 10,  /* immediate 6 bit constant; address in current page */
    t_I8  = 1 << 11,  /* immediate 8 bit constant (I/O port number) */
    t_I8c = 1 << 12,  /* immediate 8 bit constant inverted */
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
    t_LBL | t_I8c,       /* 00 */
    t_TML | t_I4 | t_I8, /* 01 */
    t_TML | t_I4 | t_I8, /* 02 */
    t_TML | t_I4 | t_I8, /* 03 */
    t_LBUA,              /* 04 */
    t_RTN,               /* 05 */
    t_XS,                /* 06 */
    t_RTNSK,             /* 07 */
    t_ADCSK,             /* 08 */
    t_ADSK,              /* 09 */
    t_ADC,               /* 0a */
    t_AD,                /* 0b */
    t_EOR,               /* 0c */
    t_AND,               /* 0d */
    t_COMP,              /* 0e */
    t_OR,                /* 0f */

    t_LBMX,              /* 10 */
    t_LABL,              /* 11 */
    t_LAX,               /* 12 */
    t_SAG,               /* 13 */
    t_SKF2,              /* 14 */
    t_SKC,               /* 15 */
    t_SKF1,              /* 16 */
    t_INCB,              /* 17 */
    t_XBMX,              /* 18 */
    t_XABL,              /* 19 */
    t_XAX,               /* 1a */
    t_LXA,               /* 1b */
    t_IOL | t_I8,        /* 1c */
    t_DOA,               /* 1d */
    t_SKZ,               /* 1e */
    t_DECB,              /* 1f */

    t_SC,                /* 20 */
    t_SF2,               /* 21 */
    t_SF1,               /* 22 */
    t_DIB,               /* 23 */
    t_RC,                /* 24 */
    t_RF2,               /* 25 */
    t_RF1,               /* 26 */
    t_DIA,               /* 27 */
    t_EXD | t_I3c,       /* 28 */
    t_EXD | t_I3c,       /* 29 */
    t_EXD | t_I3c,       /* 2a */
    t_EXD | t_I3c,       /* 2b */
    t_EXD | t_I3c,       /* 2c */
    t_EXD | t_I3c,       /* 2d */
    t_EXD | t_I3c,       /* 2e */
    t_EXD | t_I3c,       /* 2f */

    t_LD | t_I3c,        /* 30 */
    t_LD | t_I3c,        /* 31 */
    t_LD | t_I3c,        /* 32 */
    t_LD | t_I3c,        /* 33 */
    t_LD | t_I3c,        /* 34 */
    t_LD | t_I3c,        /* 35 */
    t_LD | t_I3c,        /* 36 */
    t_LD | t_I3c,        /* 37 */
    t_EX | t_I3c,        /* 38 */
    t_EX | t_I3c,        /* 39 */
    t_EX | t_I3c,        /* 3a */
    t_EX | t_I3c,        /* 3b */
    t_EX | t_I3c,        /* 3c */
    t_EX | t_I3c,        /* 3d */
    t_EX | t_I3c,        /* 3e */
    t_EX | t_I3c,        /* 3f */

    t_SKBI | t_I4,       /* 40 */
    t_SKBI | t_I4,       /* 41 */
    t_SKBI | t_I4,       /* 42 */
    t_SKBI | t_I4,       /* 43 */
    t_SKBI | t_I4,       /* 44 */
    t_SKBI | t_I4,       /* 45 */
    t_SKBI | t_I4,       /* 46 */
    t_SKBI | t_I4,       /* 47 */
    t_SKBI | t_I4,       /* 48 */
    t_SKBI | t_I4,       /* 49 */
    t_SKBI | t_I4,       /* 4a */
    t_SKBI | t_I4,       /* 4b */
    t_SKBI | t_I4,       /* 4c */
    t_SKBI | t_I4,       /* 4d */
    t_SKBI | t_I4,       /* 4e */
    t_SKBI | t_I4,       /* 4f */

    t_TL | t_I4 | t_I8,  /* 50 */
    t_TL | t_I4 | t_I8,  /* 51 */
    t_TL | t_I4 | t_I8,  /* 52 */
    t_TL | t_I4 | t_I8,  /* 53 */
    t_TL | t_I4 | t_I8,  /* 54 */
    t_TL | t_I4 | t_I8,  /* 55 */
    t_TL | t_I4 | t_I8,  /* 56 */
    t_TL | t_I4 | t_I8,  /* 57 */
    t_TL | t_I4 | t_I8,  /* 58 */
    t_TL | t_I4 | t_I8,  /* 59 */
    t_TL | t_I4 | t_I8,  /* 5a */
    t_TL | t_I4 | t_I8,  /* 5b */
    t_TL | t_I4 | t_I8,  /* 5c */
    t_TL | t_I4 | t_I8,  /* 5d */
    t_TL | t_I4 | t_I8,  /* 5e */
    t_TL | t_I4 | t_I8,  /* 5f */

    t_ADI | t_I4c,       /* 60 */
    t_ADI | t_I4c,       /* 61 */
    t_ADI | t_I4c,       /* 62 */
    t_ADI | t_I4c,       /* 63 */
    t_ADI | t_I4c,       /* 64 */
    t_DC,                /* 65 */
    t_ADI | t_I4c,       /* 66 */
    t_ADI | t_I4c,       /* 67 */
    t_ADI | t_I4c,       /* 68 */
    t_ADI | t_I4c,       /* 69 */
    t_ADI | t_I4c,       /* 6a */
    t_ADI | t_I4c,       /* 6b */
    t_ADI | t_I4c,       /* 6c */
    t_ADI | t_I4c,       /* 6d */
    t_ADI | t_I4c,       /* 6e */
    t_CYS,               /* 6f */

    t_LDI | t_I4c,       /* 70 */
    t_LDI | t_I4c,       /* 71 */
    t_LDI | t_I4c,       /* 72 */
    t_LDI | t_I4c,       /* 73 */
    t_LDI | t_I4c,       /* 74 */
    t_LDI | t_I4c,       /* 75 */
    t_LDI | t_I4c,       /* 76 */
    t_LDI | t_I4c,       /* 77 */
    t_LDI | t_I4c,       /* 78 */
    t_LDI | t_I4c,       /* 79 */
    t_LDI | t_I4c,       /* 7a */
    t_LDI | t_I4c,       /* 7b */
    t_LDI | t_I4c,       /* 7c */
    t_LDI | t_I4c,       /* 7d */
    t_LDI | t_I4c,       /* 7e */
    t_LDI | t_I4c,       /* 7f */

    t_T | t_I6p,         /* 80 */
    t_T | t_I6p,         /* 81 */
    t_T | t_I6p,         /* 82 */
    t_T | t_I6p,         /* 83 */
    t_T | t_I6p,         /* 84 */
    t_T | t_I6p,         /* 85 */
    t_T | t_I6p,         /* 86 */
    t_T | t_I6p,         /* 87 */
    t_T | t_I6p,         /* 88 */
    t_T | t_I6p,         /* 89 */
    t_T | t_I6p,         /* 8a */
    t_T | t_I6p,         /* 8b */
    t_T | t_I6p,         /* 8c */
    t_T | t_I6p,         /* 8d */
    t_T | t_I6p,         /* 8e */
    t_T | t_I6p,         /* 8f */

    t_T | t_I6p,         /* 90 */
    t_T | t_I6p,         /* 91 */
    t_T | t_I6p,         /* 92 */
    t_T | t_I6p,         /* 93 */
    t_T | t_I6p,         /* 94 */
    t_T | t_I6p,         /* 95 */
    t_T | t_I6p,         /* 96 */
    t_T | t_I6p,         /* 97 */
    t_T | t_I6p,         /* 98 */
    t_T | t_I6p,         /* 99 */
    t_T | t_I6p,         /* 9a */
    t_T | t_I6p,         /* 9b */
    t_T | t_I6p,         /* 9c */
    t_T | t_I6p,         /* 9d */
    t_T | t_I6p,         /* 9e */
    t_T | t_I6p,         /* 9f */

    t_T | t_I6p,         /* a0 */
    t_T | t_I6p,         /* a1 */
    t_T | t_I6p,         /* a2 */
    t_T | t_I6p,         /* a3 */
    t_T | t_I6p,         /* a4 */
    t_T | t_I6p,         /* a5 */
    t_T | t_I6p,         /* a6 */
    t_T | t_I6p,         /* a7 */
    t_T | t_I6p,         /* a8 */
    t_T | t_I6p,         /* a9 */
    t_T | t_I6p,         /* aa */
    t_T | t_I6p,         /* ab */
    t_T | t_I6p,         /* ac */
    t_T | t_I6p,         /* ad */
    t_T | t_I6p,         /* ae */
    t_T | t_I6p,         /* af */

    t_T | t_I6p,         /* b0 */
    t_T | t_I6p,         /* b1 */
    t_T | t_I6p,         /* b2 */
    t_T | t_I6p,         /* b3 */
    t_T | t_I6p,         /* b4 */
    t_T | t_I6p,         /* b5 */
    t_T | t_I6p,         /* b6 */
    t_T | t_I6p,         /* b7 */
    t_T | t_I6p,         /* b8 */
    t_T | t_I6p,         /* b9 */
    t_T | t_I6p,         /* ba */
    t_T | t_I6p,         /* bb */
    t_T | t_I6p,         /* bc */
    t_T | t_I6p,         /* bd */
    t_T | t_I6p,         /* be */
    t_T | t_I6p,         /* bf */

    t_LB | t_I4p,        /* c0 */
    t_LB | t_I4p,        /* c1 */
    t_LB | t_I4p,        /* c2 */
    t_LB | t_I4p,        /* c3 */
    t_LB | t_I4p,        /* c4 */
    t_LB | t_I4p,        /* c5 */
    t_LB | t_I4p,        /* c6 */
    t_LB | t_I4p,        /* c7 */
    t_LB | t_I4p,        /* c8 */
    t_LB | t_I4p,        /* c9 */
    t_LB | t_I4p,        /* ca */
    t_LB | t_I4p,        /* cb */
    t_LB | t_I4p,        /* cc */
    t_LB | t_I4p,        /* cd */
    t_LB | t_I4p,        /* ce */
    t_LB | t_I4p,        /* cf */

    t_TM | t_I6p,        /* d0 */
    t_TM | t_I6p,        /* d1 */
    t_TM | t_I6p,        /* d2 */
    t_TM | t_I6p,        /* d3 */
    t_TM | t_I6p,        /* d4 */
    t_TM | t_I6p,        /* d5 */
    t_TM | t_I6p,        /* d6 */
    t_TM | t_I6p,        /* d7 */
    t_TM | t_I6p,        /* d8 */
    t_TM | t_I6p,        /* d9 */
    t_TM | t_I6p,        /* da */
    t_TM | t_I6p,        /* db */
    t_TM | t_I6p,        /* dc */
    t_TM | t_I6p,        /* dd */
    t_TM | t_I6p,        /* de */
    t_TM | t_I6p,        /* df */

    t_TM | t_I6p,        /* e0 */
    t_TM | t_I6p,        /* e1 */
    t_TM | t_I6p,        /* e2 */
    t_TM | t_I6p,        /* e3 */
    t_TM | t_I6p,        /* e4 */
    t_TM | t_I6p,        /* e5 */
    t_TM | t_I6p,        /* e6 */
    t_TM | t_I6p,        /* e7 */
    t_TM | t_I6p,        /* e8 */
    t_TM | t_I6p,        /* e9 */
    t_TM | t_I6p,        /* ea */
    t_TM | t_I6p,        /* eb */
    t_TM | t_I6p,        /* ec */
    t_TM | t_I6p,        /* ed */
    t_TM | t_I6p,        /* ee */
    t_TM | t_I6p,        /* ef */

    t_TM | t_I6p,        /* f0 */
    t_TM | t_I6p,        /* f1 */
    t_TM | t_I6p,        /* f2 */
    t_TM | t_I6p,        /* f3 */
    t_TM | t_I6p,        /* f4 */
    t_TM | t_I6p,        /* f5 */
    t_TM | t_I6p,        /* f6 */
    t_TM | t_I6p,        /* f7 */
    t_TM | t_I6p,        /* f8 */
    t_TM | t_I6p,        /* f9 */
    t_TM | t_I6p,        /* fa */
    t_TM | t_I6p,        /* fb */
    t_TM | t_I6p,        /* fc */
    t_TM | t_I6p,        /* fd */
    t_TM | t_I6p,        /* fe */
    t_TM | t_I6p,        /* ff */
};

CPU_DISASSEMBLE( pps4 )
{
    UINT32 flags = 0;
    unsigned PC = pc;
    UINT8 op = OP(pc++);
    UINT32 tok = table[op];
    char *dst = 0;

    if (0 == (tok & t_MASK))
        sprintf(buffer, "%s", token_str[tok & t_MASK]);
    else
        dst = buffer + sprintf(buffer, "%-7s", token_str[tok & t_MASK]);

    if (tok & t_I3c) {
        // 3 bit immediate, complemented
        UINT8 i = ~op & 7;
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

    if (tok & t_I8) {
        // 8 bit immediate I/O port address
        UINT8 arg = ARG(pc++);
        dst += sprintf(dst, "%02x", arg);
    }

    if (tok & t_I8c) {
        // 8 bit immediate offset into page
        UINT16 arg = ~ARG(pc++) & 255;
        dst += sprintf(dst, "%03x", arg);
    }

    if (0x05 == op || 0x07 == op)   // RTN or RTNSK
            flags |= DASMFLAG_STEP_OUT;

    return (pc - PC) | flags | DASMFLAG_SUPPORTED;
}
