// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   A quick-hack 6803/6808 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 * History:
 * 990314 HJB
 * The disassembler knows about valid opcodes for M6800/1/2/3/8 and HD63701.
 * 990302 HJB
 * Changed the string array into a table of opcode names (or tokens) and
 * argument types. This second try should give somewhat better results.
 * Named the undocumented HD63701YO opcodes $12 and $13 'asx1' and 'asx2',
 * since 'add contents of stack to x register' is what they do.
 *
 */

#include "emu.h"
#include "debugger.h"
#include "m6800.h"

enum addr_mode {
	inh,    /* inherent */
	rel,    /* relative */
	imb,    /* immediate (byte) */
	imw,    /* immediate (word) */
	dir,    /* direct address */
	imd,    /* HD63701YO: immediate, direct address */
	ext,    /* extended address */
	idx,    /* x + byte offset */
	imx,    /* HD63701YO: immediate, x + byte offset */
	sx1     /* HD63701YO: undocumented opcodes: byte from (s+1) */
};

enum op_names {
	aba=0,  abx,    adca,   adcb,   adda,   addb,   addd,   aim,
	anda,   andb,   asl,    asla,   aslb,   asld,   asr,    asra,
	asrb,   bcc,    bcs,    beq,    bge,    bgt,    bhi,    bita,
	bitb,   ble,    bls,    blt,    bmi,    bne,    bpl,    bra,
	brn,    bsr,    bvc,    bvs,    cba,    clc,    cli,    clr,
	clra,   clrb,   clv,    cmpa,   cmpb,   cmpx,   com,    coma,
	comb,   daa,    dec,    deca,   decb,   des,    dex,    eim,
	eora,   eorb,   ill,    inc,    inca,   incb,   ins,    inx,
	jmp,    jsr,    lda,    ldb,    ldd,    lds,    ldx,    lsr,
	lsra,   lsrb,   lsrd,   mul,    neg,    nega,   negb,   nop,
	oim,    ora,    orb,    psha,   pshb,   pshx,   pula,   pulb,
	pulx,   rol,    rola,   rolb,   ror,    rora,   rorb,   rti,
	rts,    sba,    sbca,   sbcb,   sec,    sev,    sta,    stb,
	_std,   sei,    sts,    stx,    suba,   subb,   subd,   swi,
	wai,    tab,    tap,    tba,    tim,    tpa,    tst,    tsta,
	tstb,   tsx,    txs,    asx1,   asx2,   xgdx,   addx,   adcx
};

static const char *const op_name_str[] = {
	"aba",   "abx",   "adca",  "adcb",  "adda",  "addb",  "addd",  "aim",
	"anda",  "andb",  "asl",   "asla",  "aslb",  "asld",  "asr",   "asra",
	"asrb",  "bcc",   "bcs",   "beq",   "bge",   "bgt",   "bhi",   "bita",
	"bitb",  "ble",   "bls",   "blt",   "bmi",   "bne",   "bpl",   "bra",
	"brn",   "bsr",   "bvc",   "bvs",   "cba",   "clc",   "cli",   "clr",
	"clra",  "clrb",  "clv",   "cmpa",  "cmpb",  "cmpx",  "com",   "coma",
	"comb",  "daa",   "dec",   "deca",  "decb",  "des",   "dex",   "eim",
	"eora",  "eorb",  "illegal","inc",   "inca",  "incb",  "ins",   "inx",
	"jmp",   "jsr",   "lda",   "ldb",   "ldd",   "lds",   "ldx",   "lsr",
	"lsra",  "lsrb",  "lsrd",  "mul",   "neg",   "nega",  "negb",  "nop",
	"oim",   "ora",   "orb",   "psha",  "pshb",  "pshx",  "pula",  "pulb",
	"pulx",  "rol",   "rola",  "rolb",  "ror",   "rora",  "rorb",  "rti",
	"rts",   "sba",   "sbca",  "sbcb",  "sec",   "sev",   "sta",   "stb",
	"std",   "sei",   "sts",   "stx",   "suba",  "subb",  "subd",  "swi",
	"wai",   "tab",   "tap",   "tba",   "tim",   "tpa",   "tst",   "tsta",
	"tstb",  "tsx",   "txs",   "asx1",  "asx2",  "xgdx",  "addx",  "adcx"
};

/*
 * This table defines the opcodes:
 * byte meaning
 * 0    token (menmonic)
 * 1    addressing mode
 * 2    invalid opcode for 1:6800/6802/6808, 2:6801/6803, 4:HD63701
 */

static const UINT8 table[0x102][3] = {
	{ill, inh,7},{nop, inh,0},{ill, inh,7},{ill, inh,7},/* 00 */
	{lsrd,inh,1},{asld,inh,1},{tap, inh,0},{tpa, inh,0},
	{inx, inh,0},{dex, inh,0},{clv, inh,0},{sev, inh,0},
	{clc, inh,0},{sec, inh,0},{cli, inh,0},{sei, inh,0},
	{sba, inh,0},{cba, inh,0},{asx1,sx1,0},{asx2,sx1,0},/* 10 */
	{ill, inh,7},{ill, inh,7},{tab, inh,0},{tba, inh,0},
	{xgdx,inh,3},{daa, inh,0},{ill, inh,7},{aba, inh,0},
	{ill, inh,7},{ill, inh,7},{ill, inh,7},{ill, inh,7},
	{bra, rel,0},{brn, rel,0},{bhi, rel,0},{bls, rel,0},/* 20 */
	{bcc, rel,0},{bcs, rel,0},{bne, rel,0},{beq, rel,0},
	{bvc, rel,0},{bvs, rel,0},{bpl, rel,0},{bmi, rel,0},
	{bge, rel,0},{blt, rel,0},{bgt, rel,0},{ble, rel,0},
	{tsx, inh,0},{ins, inh,0},{pula,inh,0},{pulb,inh,0},/* 30 */
	{des, inh,0},{txs, inh,0},{psha,inh,0},{pshb,inh,0},
	{pulx,inh,1},{rts, inh,0},{abx, inh,1},{rti, inh,0},
	{pshx,inh,1},{mul, inh,1},{wai, inh,0},{swi, inh,0},
	{nega,inh,0},{ill, inh,7},{ill, inh,7},{coma,inh,0},/* 40 */
	{lsra,inh,0},{ill, inh,7},{rora,inh,0},{asra,inh,0},
	{asla,inh,0},{rola,inh,0},{deca,inh,0},{ill, inh,7},
	{inca,inh,0},{tsta,inh,0},{ill, inh,7},{clra,inh,0},
	{negb,inh,0},{ill, inh,7},{ill, inh,7},{comb,inh,0},/* 50 */
	{lsrb,inh,0},{ill, inh,7},{rorb,inh,0},{asrb,inh,0},
	{aslb,inh,0},{rolb,inh,0},{decb,inh,0},{ill, inh,7},
	{incb,inh,0},{tstb,inh,0},{ill, inh,7},{clrb,inh,0},
	{neg, idx,0},{aim, imx,3},{oim, imx,3},{com, idx,0},/* 60 */
	{lsr, idx,0},{eim, imx,3},{ror, idx,0},{asr, idx,0},
	{asl, idx,0},{rol, idx,0},{dec, idx,0},{tim, imx,3},
	{inc, idx,0},{tst, idx,0},{jmp, idx,0},{clr, idx,0},
	{neg, ext,0},{aim, imd,3},{oim, imd,3},{com, ext,0},/* 70 */
	{lsr, ext,0},{eim, imd,3},{ror, ext,0},{asr, ext,0},
	{asl, ext,0},{rol, ext,0},{dec, ext,0},{tim, imd,3},
	{inc, ext,0},{tst, ext,0},{jmp, ext,0},{clr, ext,0},
	{suba,imb,0},{cmpa,imb,0},{sbca,imb,0},{subd,imw,1},/* 80 */
	{anda,imb,0},{bita,imb,0},{lda, imb,0},{sta, imb,0},
	{eora,imb,0},{adca,imb,0},{ora, imb,0},{adda,imb,0},
	{cmpx,imw,0},{bsr, rel,0},{lds, imw,0},{sts, imw,0},
	{suba,dir,0},{cmpa,dir,0},{sbca,dir,0},{subd,dir,1},/* 90 */
	{anda,dir,0},{bita,dir,0},{lda, dir,0},{sta, dir,0},
	{eora,dir,0},{adca,dir,0},{ora, dir,0},{adda,dir,0},
	{cmpx,dir,0},{jsr, dir,0},{lds, dir,0},{sts, dir,0},
	{suba,idx,0},{cmpa,idx,0},{sbca,idx,0},{subd,idx,1},/* a0 */
	{anda,idx,0},{bita,idx,0},{lda, idx,0},{sta, idx,0},
	{eora,idx,0},{adca,idx,0},{ora, idx,0},{adda,idx,0},
	{cmpx,idx,0},{jsr, idx,0},{lds, idx,0},{sts, idx,0},
	{suba,ext,0},{cmpa,ext,0},{sbca,ext,0},{subd,ext,1},/* b0 */
	{anda,ext,0},{bita,ext,0},{lda, ext,0},{sta, ext,0},
	{eora,ext,0},{adca,ext,0},{ora, ext,0},{adda,ext,0},
	{cmpx,ext,0},{jsr, ext,0},{lds, ext,0},{sts, ext,0},
	{subb,imb,0},{cmpb,imb,0},{sbcb,imb,0},{addd,imw,1},/* c0 */
	{andb,imb,0},{bitb,imb,0},{ldb, imb,0},{stb, imb,0},
	{eorb,imb,0},{adcb,imb,0},{orb, imb,0},{addb,imb,0},
	{ldd, imw,1},{_std,imw,1},{ldx, imw,0},{stx, imw,0},
	{subb,dir,0},{cmpb,dir,0},{sbcb,dir,0},{addd,dir,1},/* d0 */
	{andb,dir,0},{bitb,dir,0},{ldb, dir,0},{stb, dir,0},
	{eorb,dir,0},{adcb,dir,0},{orb, dir,0},{addb,dir,0},
	{ldd, dir,1},{_std,dir,1},{ldx, dir,0},{stx, dir,0},
	{subb,idx,0},{cmpb,idx,0},{sbcb,idx,0},{addd,idx,1},/* e0 */
	{andb,idx,0},{bitb,idx,0},{ldb, idx,0},{stb, idx,0},
	{eorb,idx,0},{adcb,idx,0},{orb, idx,0},{addb,idx,0},
	{ldd, idx,1},{_std,idx,1},{ldx, idx,0},{stx, idx,0},
	{subb,ext,0},{cmpb,ext,0},{sbcb,ext,0},{addd,ext,1},/* f0 */
	{andb,ext,0},{bitb,ext,0},{ldb, ext,0},{stb, ext,0},
	{eorb,ext,0},{adcb,ext,0},{orb, ext,0},{addb,ext,0},
	{ldd, ext,1},{_std,ext,1},{ldx, ext,0},{stx, ext,0},

	/* extra instruction $fc for NSC-8105 */
	{addx,ext,0},
	/* extra instruction $ec for NSC-8105 */
	{adcx,imb,0}
};

/* some macros to keep things short */
#define OP      oprom[0]
#define ARG1    opram[1]
#define ARG2    opram[2]
#define ARGW    (opram[1]<<8) + opram[2]

static unsigned Dasm680x (int subtype, char *buf, unsigned pc, const UINT8 *oprom, const UINT8 *opram)
{
	UINT32 flags = 0;
	int invalid_mask;
	int code = OP;
	UINT8 opcode, args, invalid;

	switch( subtype )
	{
		case 6800: case 6802: case 6808: case 8105:
			invalid_mask = 1;
			break;
		case 6801: case 6803:
			invalid_mask = 2;
			break;
		default:
			invalid_mask = 4;
	}

	/* NSC-8105 is a special case */
	if (subtype == 8105)
	{
		/* swap bits */
		code = (code & 0x3c) | ((code & 0x41) << 1) | ((code & 0x82) >> 1);

		/* and check for extra instruction */
		if (code == 0xfc)  code = 0x0100;
		if (code == 0xec)  code = 0x0101;
	}

	opcode = table[code][0];
	args = table[code][1];
	invalid = table[code][2];

	if (opcode == bsr || opcode == jsr)
		flags = DASMFLAG_STEP_OVER;
	else if (opcode == rti || opcode == rts)
		flags = DASMFLAG_STEP_OUT;

	if ( invalid & invalid_mask )   /* invalid for this cpu type ? */
	{
		strcpy(buf, "illegal");
		return 1 | flags | DASMFLAG_SUPPORTED;
	}

	buf += sprintf(buf, "%-5s", op_name_str[opcode]);

	switch( args )
	{
		case rel:  /* relative */
			sprintf (buf, "$%04X", pc + (INT8)ARG1 + 2);
			return 2 | flags | DASMFLAG_SUPPORTED;
		case imb:  /* immediate (byte) */
			sprintf (buf, "#$%02X", ARG1);
			return 2 | flags | DASMFLAG_SUPPORTED;
		case imw:  /* immediate (word) */
			sprintf (buf, "#$%04X", ARGW);
			return 3 | flags | DASMFLAG_SUPPORTED;
		case idx:  /* indexed + byte offset */
			sprintf (buf, "(x+$%02X)", ARG1 );
			return 2 | flags | DASMFLAG_SUPPORTED;
		case imx:  /* immediate, indexed + byte offset */
			sprintf (buf, "#$%02X,(x+$%02x)", ARG1, ARG2 );
			return 3 | flags | DASMFLAG_SUPPORTED;
		case dir:  /* direct address */
			sprintf (buf, "$%02X", ARG1 );
			return 2 | flags | DASMFLAG_SUPPORTED;
		case imd:  /* immediate, direct address */
			sprintf (buf, "#$%02X,$%02X", ARG1, ARG2);
			return 3 | flags | DASMFLAG_SUPPORTED;
		case ext:  /* extended address */
			sprintf (buf, "$%04X", ARGW);
			return 3 | flags | DASMFLAG_SUPPORTED;
		case sx1:  /* byte from address (s + 1) */
			sprintf (buf, "(s+1)");
			return 1 | flags | DASMFLAG_SUPPORTED;
		default:
			return 1 | flags | DASMFLAG_SUPPORTED;
	}
}

CPU_DISASSEMBLE( m6800 )
{
	return Dasm680x(6800,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( m6801 )
{
	return Dasm680x(6801,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( m6802 )
{
	return Dasm680x(6802,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( m6803 )
{
	return Dasm680x(6803,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( m6808 )
{
	return Dasm680x(6808,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( hd6301 )
{
	return Dasm680x(6301,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( hd63701 )
{
	return Dasm680x(63701,buffer,pc,oprom,opram);
}

CPU_DISASSEMBLE( nsc8105 )
{
	return Dasm680x(8105,buffer,pc,oprom,opram);
}
