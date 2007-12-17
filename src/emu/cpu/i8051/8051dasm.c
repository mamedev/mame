/*****************************************************************************
 *
 *   i8051dasm.c
 *   Portable MCS-51 Family Emulator
 *
 *   Chips in the family:
 *   8051 Product Line (8031,8051,8751)
 *   8052 Product Line (8032,8052,8752)
 *   8054 Product Line (8054)
 *   8058 Product Line (8058)
 *
 *   Copyright (c) 2003 Steve Ellenoff, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Intel(tm) MC51 Microcontroller Family Users Manual' and
 *  #2) 8051 simulator by Travis Marlatte
 *  #3) Portable UPI-41/8041/8741/8042/8742 emulator V0.1 by Juergen Buchmueller (MAME CORE)
 *
 *****************************************************************************
 * Symbol Memory Name Tables borrowed from:
 * D52 8052 Disassembler - Copyright (C) 1995-2002 by
 * Jeffery L. Post
 *****************************************************************************/

#include "debugger.h"
#include "i8051.h"

#define SHOW_MEMORY_NAMES	1

#ifdef SHOW_MEMORY_NAMES

/*Display the memory address names for data & bit address access*/

//SFR Names
static const char sfr[128][7] = {
    "p0",     "sp",     "dpl",    "dph",    /* 80 - 83 */
    "adat",   "85h",    "86h",    "pcon",   /* 84 - 87 */
    "tcon",   "tmod",   "tl0",    "tl1",    /* 88 - 8b */
    "th0",    "th1",    "pwcm",   "pwmp",   /* 8a - 8f */
    "p1",     "91h",    "92h",    "93h",    /* 90 - 93 */
    "94h",    "95h",    "96h",    "97h",    /* 94 - 97 */
    "scon",   "sbuf",   "9ah",    "9bh",    /* 98 - 9b */
    "9ch",    "9dh",    "9eh",    "9fh",    /* 9c - 9f */
    "p2",     "0a1h",   "0a2h",   "0a3h",   /* a0 - a3 */
    "0a4h",   "0a5h",   "0a6h",   "0a7h",   /* a4 - a7 */
    "ie",     "cml0",   "cml1",   "cml2",   /* a8 - ab */
    "ctl0",   "ctl1",   "ctl2",   "ctl3",   /* ac - af */
    "p3",     "0b1h",   "0b2h",   "0b3h",   /* b0 - b3 */
    "0b4h",   "0b5h",   "0b6h",   "0b7h",   /* b4 - b7 */
    "ip",     "0b9h",   "0bah",   "0bbh",   /* b8 - bb */
    "0bch",   "0bdh",   "0beh",   "0bfh",   /* bc - bf */
    "p4",     "0c1h",   "0c2h",   "0c3h",   /* c0 - c3 */
    "p5",     "adcon",  "adch",   "0c7h",   /* c4 - c7 */
    "t2con",  "cmh0",   "rcap2l", "rcap2h", /* c8 - cb */
    "tl2",    "th2",    "cth2",   "cth3",   /* cc - cf */
    "psw",    "0d1h",   "0d2h",   "0d3h",   /* d0 - d3 */
    "0d4h",   "0d5h",   "0d6h",   "0d7h",   /* d4 - d7 */
    "i2cfg",  "s1sta",  "s1dat",  "s1adr",  /* d8 - db */
    "0dch",   "0ddh",   "0deh",   "0dfh",   /* dc - df */
    "acc",    "0e1h",   "0e2h",   "0e3h",   /* e0 - e3 */
    "0e4h",   "0e5h",   "0e6h",   "0e7h",   /* e4 - e7 */
    "csr",    "0e9h",   "tm2con", "ctcon",  /* e8 - eb */
    "tml2",   "tmh2",   "ste",    "rte",    /* ec - ef */
    "b",      "0f1h",   "0f2h",   "0f3h",   /* f0 - f3 */
    "0f4h",   "0f5h",   "0f6h",   "0f7h",   /* f4 - f7 */
    "i2sta",  "0f9h",   "0fah",   "0fbh",   /* f8 - fb */
    "pwm0",   "pwm1",   "pwena",  "t3"      /* fc - ff */
};

//SFR Addressable bits
static const char sfrbits[128][8] = {
    "p0.0",   "p0.1",   "p0.2",   "p0.3",   /* 80 - 83 */
    "p0.4",   "p0.5",   "p0.6",   "p0.7",   /* 84 - 87 */
    "it0",    "ie0",    "it1",    "ie1",    /* 88 - 8b */
    "tr0",    "tf0",    "tr1",    "tf1",    /* 8c - 8f */
    "p1.0",   "p1.1",   "p1.2",   "p1.3",   /* 90 - 93 */
    "p1.4",   "p1.5",   "p1.6",   "p1.7",   /* 94 - 97 */
    "ri",     "ti",     "rb8",    "tb8",    /* 98 - 9b */
    "ren",    "sm2",    "sm1",    "sm0",    /* 9c - 9f */
    "p2.0",   "p2.1",   "p2.2",   "p2.3",   /* a0 - a3 */
    "p2.4",   "p2.5",   "p2.6",   "p2.7",   /* a4 - a7 */
    "ex0",    "et0",    "ex1",    "et1",    /* a8 - ab */
    "es",     "ie.5",   "ie.6",   "ea",     /* ac - af */
    "rxd",    "txd",    "int0",   "int1",   /* b0 - b3 */
    "t0",     "t1",     "wr",     "rd",     /* b4 - b7 */
    "px0",    "pt0",    "px1",    "pt1",    /* b8 - bb */
    "ps",     "ip.5",   "ip.6",   "ip.7",   /* bc - bf */
    "0c0h.0", "0c0h.1", "0c0h.2", "0c0h.3", /* c0 - c3 */
    "0c0h.4", "0c0h.5", "0c0h.6", "0c0h.7", /* c4 - c7 */
    "cprl2",  "ct2",    "tr2",    "exen2",  /* c8 - cb */
    "tclk",   "rclk",   "exf2",   "tf2",    /* cc - cf */
    "p",      "psw.1",  "ov",     "rs0",    /* d0 - d3 */
    "rs1",    "f0",     "ac",     "cy",     /* d4 - d7 */
    "ct0",    "ct1",    "i2cfg.2","i2cfg.3",/* d8 - db */
    "tirun",  "clrti",  "mastrq", "slaven", /* dc - df */
    "acc.0",  "acc.1",  "acc.2",  "acc.3",  /* e0 - e3 */
    "acc.4",  "acc.5",  "acc.6",  "acc.7",  /* e4 - e7 */
    "ibf",    "obf",    "idsm",   "obfc",   /* e8 - eb */
    "ma0",    "ma1",    "mb0",    "mb1",    /* ec - ef */
    "b.0",    "b.1",    "b.2",    "b.3",    /* f0 - f3 */
    "b.4",    "b.5",    "b.6",    "b.7",    /* f4 - f7 */
    "xstp",   "xstr",   "makstp", "makstr", /* f8 - fb */
    "xactv",  "xdata",  "idle",   "i2sta.7" /* fc - ff */
} ;

//Names for bit addressable memory
static const char membits[128][6] = {
    "20h.0",  "20h.1",  "20h.2",  "20h.3",
    "20h.4",  "20h.5",  "20h.6",  "20h.7",
    "21h.0",  "21h.1",  "21h.2",  "21h.3",
    "21h.4",  "21h.5",  "21h.6",  "21h.7",
    "22h.0",  "22h.1",  "22h.2",  "22h.3",
    "22h.4",  "22h.5",  "22h.6",  "22h.7",
    "23h.0",  "23h.1",  "23h.2",  "23h.3",
    "23h.4",  "23h.5",  "23h.6",  "23h.7",
    "24h.0",  "24h.1",  "24h.2",  "24h.3",
    "24h.4",  "24h.5",  "24h.6",  "24h.7",
    "25h.0",  "25h.1",  "25h.2",  "25h.3",
    "25h.4",  "25h.5",  "25h.6",  "25h.7",
    "26h.0",  "26h.1",  "26h.2",  "26h.3",
    "26h.4",  "26h.5",  "26h.6",  "26h.7",
    "27h.0",  "27h.1",  "27h.2",  "27h.3",
    "27h.4",  "27h.5",  "27h.6",  "27h.7",
    "28h.0",  "28h.1",  "28h.2",  "28h.3",
    "28h.4",  "28h.5",  "28h.6",  "28h.7",
    "29h.0",  "29h.1",  "29h.2",  "29h.3",
    "29h.4",  "29h.5",  "29h.6",  "29h.7",
    "2ah.0",  "2ah.1",  "2ah.2",  "2ah.3",
    "2ah.4",  "2ah.5",  "2ah.6",  "2ah.7",
    "2bh.0",  "2bh.1",  "2bh.2",  "2bh.3",
    "2bh.4",  "2bh.5",  "2bh.6",  "2bh.7",
    "2ch.0",  "2ch.1",  "2ch.2",  "2ch.3",
    "2ch.4",  "2ch.5",  "2ch.6",  "2ch.7",
    "2dh.0",  "2dh.1",  "2dh.2",  "2dh.3",
    "2dh.4",  "2dh.5",  "2dh.6",  "2dh.7",
    "2eh.0",  "2eh.1",  "2eh.2",  "2eh.3",
    "2eh.4",  "2eh.5",  "2eh.6",  "2eh.7",
    "2fh.0",  "2fh.1",  "2fh.2",  "2fh.3",
    "2fh.4",  "2fh.5",  "2fh.6",  "2fh.7"
};

//Regiser Bank memory names
static const char regbank[][6] = {
    "rb0r0",  "rb0r1",  "rb0r2",  "rb0r3",
    "rb0r4",  "rb0r5",  "rb0r6",  "rb0r7",
    "rb1r0",  "rb1r1",  "rb1r2",  "rb1r3",
    "rb1r4",  "rb1r5",  "rb1r6",  "rb1r7",
    "rb2r0",  "rb2r1",  "rb2r2",  "rb2r3",
    "rb2r4",  "rb2r5",  "rb2r6",  "rb2r7",
    "rb3r0",  "rb3r1",  "rb3r2",  "rb3r3",
    "rb3r4",  "rb3r5",  "rb3r6",  "rb3r7"
};

static const char *get_data_address( UINT8 arg )
{
	char *buffer = cpuintrf_temp_str();

	if(arg < 0x80)
	{
		//Ram locations 0-0x1F are considered register access in 3 banks
		if(arg < 0x1f)
			sprintf(buffer,"%s",regbank[arg]);
		else
			sprintf(buffer,"$%02X",arg);
	}
	else
		sprintf(buffer,"%s",sfr[arg-0x80]);
	return buffer;
}

static const char *get_bit_address( UINT8 arg )
{
	char *buffer = cpuintrf_temp_str();

	if(arg < 0x80)
	{
		//Bit address 0-7F can be referred to as 20.0, 20.1, to 20.7 for address 0, and 2f.0,2f.1 to 2f.7 for address 7f
		if(arg < 0x7f)
			sprintf(buffer,"%s",membits[arg]);
		else
			sprintf(buffer,"$%02X",arg);
	}
	else
		sprintf(buffer,"%s",sfrbits[arg-0x80]);
	return buffer;
}

#else

/*Just display the actual memory address for data & bit address access*/

static const char *get_data_address( UINT8 arg )
{
	char *buffer = cpuintrf_temp_str();
	sprintf(buffer,"$%02X",arg);
	return buffer;
}

static const char *get_bit_address( UINT8 arg )
{
	char *buffer = cpuintrf_temp_str();
	sprintf(buffer,"$%02X",arg);
	return buffer;
}

#endif


unsigned i8051_dasm(char *dst, unsigned pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	unsigned PC = pc;
	const char *sym, *sym2;
    UINT8 op, data;
	UINT16 addr;
	INT8 rel;

	op = oprom[PC++ - pc];
	switch( op )
	{
		//NOP
		case 0x00:				/* 1: 0000 0000 */
			sprintf(dst, "nop");
			break;

		//AJMP code addr        /* 1: aaa0 0001 */
		case 0x01:
		case 0x21:
		case 0x41:
		case 0x61:
		case 0x81:
		case 0xa1:
		case 0xc1:
		case 0xe1:
			addr = opram[PC++ - pc];
			addr|= (PC++ & 0xf800) | ((op & 0xe0) << 3);
			sprintf(dst, "ajmp  $%04X", addr);
			break;

		//LJMP code addr
		case 0x02:				/* 1: 0000 0010 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "ljmp  $%04X", addr);
			break;

		//RR A
		case 0x03:				/* 1: 0000 0011 */
			sprintf(dst, "rr    a");
			break;

		//INC A
		case 0x04:				/* 1: 0000 0100 */
			sprintf(dst, "inc   a");
			break;

		//INC data addr
		case 0x05:				/* 1: 0000 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "inc   %s", sym);
			break;

		//INC @R0/@R1           /* 1: 0000 011i */
		case 0x06:
		case 0x07:
			sprintf(dst, "inc   @r%d", op&1);
			break;

		//INC R0 to R7          /* 1: 0000 1rrr */
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			sprintf(dst, "inc   r%d", op&7);
			break;

		//JBC bit addr, code addr
		case 0x10:				/* 1: 0001 0000 */
			sym = get_bit_address(opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jbc   %s,$%04X", sym, PC + rel);
			break;

		//ACALL code addr       /* 1: aaa1 0001 */
		case 0x11:
		case 0x31:
		case 0x51:
		case 0x71:
		case 0x91:
		case 0xb1:
		case 0xd1:
		case 0xf1:
			sprintf(dst, "acall $%04X", (PC & 0xf800) | ((op & 0xe0) << 3) | opram[PC - pc]);
			PC++;
			flags = DASMFLAG_STEP_OVER;
			break;

		//LCALL code addr
		case 0x12:				/* 1: 0001 0010 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "lcall $%04X", addr);
			flags = DASMFLAG_STEP_OVER;
			break;

		//RRC A
		case 0x13:				/* 1: 0001 0011 */
			sprintf(dst, "rrc   a");
			break;

		//DEC A
		case 0x14:				/* 1: 0001 0100 */
			sprintf(dst, "dec   a");
			break;

		//DEC data addr
		case 0x15:				/* 1: 0001 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "dec   %s", sym);
			break;

		//Unable to test
		//DEC @R0/@R1           /* 1: 0001 011i */
		case 0x16:
		case 0x17:
			sprintf(dst, "dec   @r%d", op&1);
			break;

		//DEC R0 to R7          /* 1: 0001 1rrr */
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:
			sprintf(dst, "dec   r%d", op&7);
			break;

		//JB  bit addr, code addr
		case 0x20:				/* 1: 0010 0000 */
			sym = get_bit_address(opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jb    %s,$%04X", sym, (PC + rel));
			break;

		//RET
		case 0x22:				/* 1: 0010 0010 */
			sprintf(dst, "ret");
			flags = DASMFLAG_STEP_OUT;
			break;

		//RL A
		case 0x23:				/* 1: 0010 0011 */
			sprintf(dst, "rl    a");
			break;

		//ADD A, #data
		case 0x24:				/* 1: 0010 0100 */
			sprintf(dst, "add   a,#$%02X", opram[PC++ - pc]);
			break;

		//ADD A, data addr
		case 0x25:				/* 1: 0010 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "add   a,%s", sym);
			break;

		//Unable to Test
		//ADD A, @R0/@R1        /* 1: 0010 011i */
		case 0x26:
		case 0x27:
			sprintf(dst, "add   a,@r%d", op&1);
			break;

		//ADD A, R0 to R7       /* 1: 0010 1rrr */
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d:
		case 0x2e:
		case 0x2f:
			sprintf(dst, "add   a,r%d", op&7);
			break;

		//JNB bit addr, code addr
		case 0x30:				/* 1: 0011 0000 */
			sym = get_bit_address(opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "jnb   %s,$%04X", sym, (PC + rel));
			break;

		//RETI
		case 0x32:				/* 1: 0011 0010 */
			sprintf(dst, "reti");
			flags = DASMFLAG_STEP_OUT;
			break;

		//RLC A
		case 0x33:				/* 1: 0011 0011 */
			sprintf(dst, "rlc   a");
			break;

		//ADDC A, #data
		case 0x34:				/* 1: 0011 0100 */
			sprintf(dst, "addc  a,#$%02X", opram[PC++ - pc]);
			break;

		//ADDC A, data addr
		case 0x35:				/* 1: 0011 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "addc  a,%s", sym);
			break;

		//ADDC A, @R0/@R1       /* 1: 0011 011i */
		case 0x36:
		case 0x37:
			sprintf(dst, "addc  a,@r%d", op&1);
			break;

		//ADDC A, R0 to R7      /* 1: 0011 1rrr */
		case 0x38:
		case 0x39:
		case 0x3a:
		case 0x3b:
		case 0x3c:
		case 0x3d:
		case 0x3e:
		case 0x3f:
			sprintf(dst, "addc  a,r%d", op&7);
			break;

		//JC code addr
		case 0x40:				/* 1: 0100 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jc    $%04X", PC + rel);
			break;

		//ORL data addr, A
		case 0x42:				/* 1: 0100 0010 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "orl   %s,a", sym);
			break;

		//ORL data addr, #data
		case 0x43:				/* 1: 0100 0011 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "orl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//Unable to Test
		//ORL A, #data
		case 0x44:				/* 1: 0100 0100 */
			sprintf(dst, "orl   a,#$%02X", opram[PC++ - pc]);
			break;

		//ORL A, data addr
		case 0x45:				/* 1: 0100 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "orl   a,%s", sym);
			break;

		//ORL A, @RO/@R1        /* 1: 0100 011i */
		case 0x46:
		case 0x47:
			sprintf(dst, "orl   a,@r%d", op&1);
			break;

		//ORL A, RO to R7       /* 1: 0100 1rrr */
		case 0x48:
		case 0x49:
		case 0x4a:
		case 0x4b:
		case 0x4c:
		case 0x4d:
		case 0x4e:
		case 0x4f:
			sprintf(dst, "orl   a,r%d", op&7);
			break;

		//JNC code addr
		case 0x50:				/* 1: 0101 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jnc   $%04X", PC + rel);
			break;

		//Unable to test
		//ANL data addr, A
		case 0x52:				/* 1: 0101 0010 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "anl   %s,a", sym);
			break;

		//Unable to test
		//ANL data addr, #data
		case 0x53:				/* 1: 0101 0011 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "anl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//ANL A, #data
		case 0x54:				/* 1: 0101 0100 */
			sprintf(dst, "anl   a,#$%02X", opram[PC++ - pc]);
			break;

		//ANL A, data addr
		case 0x55:				/* 1: 0101 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "anl   a,%s", sym);
			break;

		//Unable to test
		//ANL A, @RO/@R1        /* 1: 0101 011i */
		case 0x56:
		case 0x57:
			sprintf(dst, "anl   a,@r%d", op&1);
			break;

		//ANL A, RO to R7       /* 1: 0101 1rrr */
		case 0x58:
		case 0x59:
		case 0x5a:
		case 0x5b:
		case 0x5c:
		case 0x5d:
		case 0x5e:
		case 0x5f:
			sprintf(dst, "anl   a,r%d", op&7);
			break;

		//JZ code addr
		case 0x60:				/* 1: 0110 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jz    $%04X", PC + rel);
			break;

		//Unable to test
		//XRL data addr, A
		case 0x62:				/* 1: 0110 0010 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "xrl   %s,a", sym);
			break;

		//XRL data addr, #data
		case 0x63:				/* 1: 0110 0011 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "xrl   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//XRL A, #data
		case 0x64:				/* 1: 0110 0100 */
			sprintf(dst, "xrl   a,#$%02X", opram[PC++ - pc]);
			break;

		//XRL A, data addr
		case 0x65:				/* 1: 0110 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "xrl   a,%s", sym);
			break;

		//Unable to test
		//XRL A, @R0/@R1        /* 1: 0110 011i */
		case 0x66:
		case 0x67:
			sprintf(dst, "xrl   a,@r%d", op&1);
			break;

		//XRL A, R0 to R7       /* 1: 0110 1rrr */
		case 0x68:
		case 0x69:
		case 0x6a:
		case 0x6b:
		case 0x6c:
		case 0x6d:
		case 0x6e:
		case 0x6f:
			sprintf(dst, "xrl   a,r%d", op&7);
			break;

		//JNZ code addr
		case 0x70:				/* 1: 0111 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "jnz   $%04X", PC + rel);
			break;

		//Unable to test
		//ORL C, bit addr
		case 0x72:				/* 1: 0111 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "orl   c,%s", sym);
			break;

		//Unable to test
		//JMP @A+DPTR
		case 0x73:				/* 1: 0111 0011 */
			sprintf(dst, "jmp   @a+dptr");
			break;

		//MOV A, #data
		case 0x74:				/* 1: 0111 0100 */
			sprintf(dst, "mov   a,#$%02X", opram[PC++ - pc]);
			break;

		//MOV data addr, #data
		case 0x75:				/* 1: 0111 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,#$%02X", sym, opram[PC++ - pc]);
			break;

		//Unable to test
		//MOV @R0/@R1, #data    /* 1: 0111 011i */
		case 0x76:
		case 0x77:
			sprintf(dst, "mov   @r%d,#$%02X", op&1, opram[PC++ - pc]);
			break;

		//MOV R0 to R7, #data   /* 1: 0111 1rrr */
		case 0x78:
		case 0x79:
		case 0x7a:
		case 0x7b:
		case 0x7c:
		case 0x7d:
		case 0x7e:
		case 0x7f:
			sprintf(dst, "mov   r%d,#$%02X", (op & 7), opram[PC++ - pc]);
			break;

		//SJMP code addr
		case 0x80:				/* 1: 1000 0000 */
			rel = opram[PC++ - pc];
			sprintf(dst, "sjmp  $%04X", PC + rel);
			break;

		//ANL C, bit addr
		case 0x82:				/* 1: 1000 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "anl   c,%s", sym);
			break;

		//MOVC A, @A + PC
		case 0x83:				/* 1: 1000 0011 */
			sprintf(dst, "movc  a,@a+pc");
			break;

		//DIV AB
		case 0x84:				/* 1: 1000 0100 */
			sprintf(dst, "div   ab");
			break;

		//MOV data addr, data addr  (Note: 1st address is src, 2nd is dst, but the mov command works as mov dst,src)
		case 0x85:				/* 1: 1000 0101 */
			sym  = get_data_address(opram[PC++ - pc]);
			sym2 = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,%s", sym2, sym);
			break;

		//Unable to test
		//MOV data addr, @R0/@R1/* 1: 1000 011i */
		case 0x86:
		case 0x87:
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,@r%d", sym, op&1);
			break;

		//MOV data addr,R0 to R7/* 1: 1000 1rrr */
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0x8f:
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,r%d", sym, op&7);
			break;

		//MOV DPTR, #data16
		case 0x90:				/* 1: 1001 0000 */
			addr = (opram[PC++ - pc]<<8) & 0xff00;
			addr|= opram[PC++ - pc];
			sprintf(dst, "mov   dptr,#$%04X", addr);
			break;

		//MOV bit addr, C
		case 0x92:				/* 1: 1001 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,c", sym);
			break;

		//MOVC A, @A + DPTR
		case 0x93:				/* 1: 1001 0011 */
			sprintf(dst, "movc  a,@a+dptr");
			break;

		//SUBB A, #data
		case 0x94:				/* 1: 1001 0100 */
			sprintf(dst, "subb  a,#$%02X", opram[PC++ - pc]);
			break;

		//SUBB A, data addr
		case 0x95:				/* 1: 1001 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "subb  a,%s", sym);
			break;

		//Unable to test
		//SUBB A, @R0/@R1       /* 1: 1001 011i */
		case 0x96:
		case 0x97:
			sprintf(dst, "subb  a,@r%d", op&1);
			break;

		//SUBB A, R0 to R7      /* 1: 1001 1rrr */
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
			sprintf(dst, "subb  a,r%d", op&7);
			break;

		//Unable to test
		//ORL C, /bit addr
		case 0xa0:				  /* 1: 1010 0000 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "orl   c,/%s", sym);
			break;

		//MOV C, bit addr
		case 0xa2:				  /* 1: 1010 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "mov   c,%s", sym);
			break;

		//INC DPTR
		case 0xa3:				  /* 1: 1010 0011 */
			sprintf(dst, "inc   dptr");
			break;

		//MUL AB
		case 0xa4:				  /* 1: 1010 0100 */
			sprintf(dst, "mul   ab");
			break;

		//reserved
		case 0xa5:				  /* 1: 1010 0101 */
			sprintf(dst, "ill/rsv");
			break;

		//Unable to test
		//MOV @R0/@R1, data addr  /* 1: 1010 011i */
		case 0xa6:
		case 0xa7:
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   @r%d,%s", op&1, sym);
			break;

		//MOV R0 to R7, data addr /* 1: 1010 1rrr */
		case 0xa8:
		case 0xa9:
		case 0xaa:
		case 0xab:
		case 0xac:
		case 0xad:
		case 0xae:
		case 0xaf:
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   r%d,%s", op&7, sym);
			break;

		//ANL C,/bit addr
		case 0xb0:						 /* 1: 1011 0000 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "anl   c,/%s", sym);
			break;

		//CPL bit addr
		case 0xb2:						 /* 1: 1011 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "cpl   %s", sym);
			break;

		//Unable to test
		//CPL C
		case 0xb3:						 /* 1: 1011 0011 */
			sprintf(dst, "cpl   c");
			break;

		//CJNE A, #data, code addr
		case 0xb4:						 /* 1: 1011 0100 */
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  a,#$%02X,$%04X", data, PC + rel);
			break;

		//CJNE A, data addr, code addr
		case 0xb5:						 /* 1: 1011 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  a,%s,$%04X", sym, PC + rel);
			break;

		//Unable to test
		//CJNE @R0/@R1, #data, code addr /* 1: 1011 011i */
		case 0xb6:
		case 0xb7:
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  @r%d,#$%02X,$%04X", op&1, data, PC + rel);
			break;

		//CJNE R0 to R7, #data, code addr/* 1: 1011 1rrr */
		case 0xb8:
		case 0xb9:
		case 0xba:
		case 0xbb:
		case 0xbc:
		case 0xbd:
		case 0xbe:
		case 0xbf:
			data = opram[PC++ - pc];
			rel  = opram[PC++ - pc];
			sprintf(dst, "cjne  r%d,#$%02X,$%04X", op&7, data, PC + rel);
			break;

		//PUSH data addr
		case 0xc0:						/* 1: 1100 0000 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "push  %s", sym);
			break;

		//CLR bit addr
		case 0xc2:						/* 1: 1100 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "clr   %s", sym);
			break;

		//CLR C
		case 0xc3:						/* 1: 1100 0011 */
			sprintf(dst, "clr   c");
			break;

		//SWAP A
		case 0xc4:						/* 1: 1100 0100 */
			sprintf(dst, "swap  a");
			break;

		//XCH A, data addr
		case 0xc5:						/* 1: 1100 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "xch   a,%s", sym);
			break;

		//XCH A, @RO/@R1                /* 1: 1100 011i */
		case 0xc6:
		case 0xc7:
			sprintf(dst, "xch   a,@r%d", op&1);
			break;

		//XCH A, RO to R7               /* 1: 1100 1rrr */
		case 0xc8:
		case 0xc9:
		case 0xca:
		case 0xcb:
		case 0xcc:
		case 0xcd:
		case 0xce:
		case 0xcf:
			sprintf(dst, "xch   a,r%d", op&7);
			break;

		//POP data addr
		case 0xd0:						/* 1: 1101 0000 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "pop   %s", sym);
			break;

		//SETB bit addr
		case 0xd2:						/* 1: 1101 0010 */
			sym = get_bit_address(opram[PC++ - pc]);
			sprintf(dst, "setb  %s", sym);
			break;

		//SETB C
		case 0xd3:						/* 1: 1101 0011 */
			sprintf(dst, "setb  c");
			break;

		//Unable to test
		//DA A
		case 0xd4:						/* 1: 1101 0100 */
			sprintf(dst, "da   a");
			break;

		//DJNZ data addr, code addr
		case 0xd5:						/* 1: 1101 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			rel  = opram[PC++ - pc];
			sprintf(dst, "djnz  %s,$%04X", sym, PC + rel);
			flags = DASMFLAG_STEP_OVER;
			break;

		//XCHD A, @R0/@R1               /* 1: 1101 011i */
		case 0xd6:
		case 0xd7:
			sprintf(dst, "xchd  a,@r%d", op&1);
			break;

		//DJNZ R0 to R7,code addr       /* 1: 1101 1rrr */
		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdb:
		case 0xdc:
		case 0xdd:
		case 0xde:
		case 0xdf:
			rel = opram[PC++ - pc];
			sprintf(dst, "djnz  r%d,$%04X", op&7, (PC + rel));
			flags = DASMFLAG_STEP_OVER;
			break;

		//MOVX A,@DPTR
		case 0xe0:						/* 1: 1110 0000 */
			sprintf(dst, "movx  a,@dptr");
			break;

		//Unable to test
		//MOVX A, @R0/@R1               /* 1: 1110 001i */
		case 0xe2:
		case 0xe3:
			sprintf(dst, "movx  a,@r%d", op&1);
			break;

		//CLR A
		case 0xe4:						/* 1: 1110 0100 */
			sprintf(dst, "clr   a");
			break;

		//MOV A, data addr
		case 0xe5:						/* 1: 1110 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   a,%s", sym);
			break;

		//Unable to test
		//MOV A,@RO/@R1                 /* 1: 1110 011i */
		case 0xe6:
		case 0xe7:
			sprintf(dst, "mov   a,@r%d", op&1);
			break;

		//MOV A,R0 to R7                /* 1: 1110 1rrr */
		case 0xe8:
		case 0xe9:
		case 0xea:
		case 0xeb:
		case 0xec:
		case 0xed:
		case 0xee:
		case 0xef:
			sprintf(dst, "mov   a,r%d", op&7);
			break;

		//MOVX @DPTR,A
		case 0xf0:						/* 1: 1111 0000 */
			sprintf(dst, "movx  @dptr,a");
			break;

		//Unable to test
		//MOVX @R0/@R1,A                /* 1: 1111 001i */
		case 0xf2:
		case 0xf3:
			sprintf(dst, "movx  @r%d,a", op&1);
			break;

		//CPL A
		case 0xf4:						/* 1: 1111 0100 */
			sprintf(dst, "cpl   a");
			break;

		//MOV data addr, A
		case 0xf5:						/* 1: 1111 0101 */
			sym = get_data_address(opram[PC++ - pc]);
			sprintf(dst, "mov   %s,a", sym);
			break;

		//MOV @R0/@R1, A                /* 1: 1111 011i */
		case 0xf6:
		case 0xf7:
			sprintf(dst, "mov   @r%d,a", op&1);
			break;

		//MOV R0 to R7, A               /* 1: 1111 1rrr */
		case 0xf8:
		case 0xf9:
		case 0xfa:
		case 0xfb:
		case 0xfc:
		case 0xfd:
		case 0xfe:
		case 0xff:
			sprintf(dst, "mov   r%d,a", op&7);
			break;

		default:
			sprintf(dst, "illegal");
    }
	return (PC - pc) | flags | DASMFLAG_SUPPORTED;
}
