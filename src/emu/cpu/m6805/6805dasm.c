/*
 *   A quick-hack 68(7)05 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include "debugger.h"
#include "m6805.h"

enum addr_mode {
	_imp=0, 	/* implicit */
	_btr,		/* bit test and relative */
	_bit,		/* bit set/clear */
	_rel,		/* relative */
	_imm,		/* immediate */
	_dir,		/* direct address */
	_ext,		/* extended address */
	_idx,		/* indexed */
	_ix1,		/* indexed + byte offset */
	_ix2		/* indexed + word offset */
};

enum op_names {
	adca=0, adda,	anda,	asl,	asla,	aslx,	asr,	asra,
	asrx,	bcc,	bclr,	bcs,	beq,	bhcc,	bhcs,	bhi,
	bih,	bil,	bita,	bls,	bmc,	bmi,	bms,	bne,
	bpl,	bra,	brclr,	brn,	brset,	bset,	bsr,	clc,
	cli,	clr,	clra,	clrx,	cmpa,	com,	coma,	comx,
	cpx,	dec,	deca,	decx,	eora,	ill,	inc,	inca,
	incx,	jmp,	jsr,	lda,	ldx,	lsr,	lsra,	lsrx,
	neg,	nega,	negx,	nop,	ora,	rol,	rola,	rolx,
	ror,	rora,	rorx,	rsp,	rti,	rts,	sbca,	sec,
	sei,	sta,	stx,	suba,	swi,	tax,	tst,	tsta,
	tstx,	txa
};

static const char *op_name_str[] = {
	"adca", "adda", "anda", "asl",  "asla", "aslx", "asr",  "asra",
	"asrx", "bcc",  "bclr", "bcs",  "beq",  "bhcc", "bhcs", "bhi",
	"bih",  "bil",  "bita", "bls",  "bmc",  "bmi",  "bms",  "bne",
	"bpl",  "bra",  "brclr","brn",  "brset","bset", "bsr",  "clc",
	"cli",  "clr",  "clra", "clrx", "cmpa", "com",  "coma", "comx",
	"cpx",  "dec",  "deca", "decx", "eora", "*ill", "inc",  "inca",
	"incx", "jmp",  "jsr",  "lda",  "ldx",  "lsr",  "lsra", "lsrx",
	"neg",  "nega", "negx", "nop",  "ora",  "rol",  "rola", "rolx",
	"ror",  "rora", "rorx", "rsp",  "rti",  "rts",  "sbca", "sec",
	"sei",  "sta",  "stx",  "suba", "swi",  "tax",  "tst",  "tsta",
	"tstx", "txa"
};

static const unsigned char disasm[0x100][2] = {
	{brset,_btr},{brclr,_btr},{brset,_btr},{brclr,_btr},/* 00 */
	{brset,_btr},{brclr,_btr},{brset,_btr},{brclr,_btr},
	{brset,_btr},{brclr,_btr},{brset,_btr},{brclr,_btr},
	{brset,_btr},{brclr,_btr},{brset,_btr},{brclr,_btr},
	{bset, _bit},{bclr, _bit},{bset, _bit},{bclr, _bit},/* 10 */
	{bset, _bit},{bclr, _bit},{bset, _bit},{bclr, _bit},
	{bset, _bit},{bclr, _bit},{bset, _bit},{bclr, _bit},
	{bset, _bit},{bclr, _bit},{bset, _bit},{bclr, _bit},
	{bra,  _rel},{brn,  _rel},{bhi,  _rel},{bls,  _rel},/* 20 */
	{bcc,  _rel},{bcs,  _rel},{bne,  _rel},{beq,  _rel},
	{bhcc, _rel},{bhcs, _rel},{bpl,  _rel},{bmi,  _rel},
	{bmc,  _rel},{bms,  _rel},{bil,  _rel},{bih,  _rel},
	{neg,  _dir},{ill,  _imp},{ill,  _imp},{com,  _dir},/* 30 */
	{lsr,  _dir},{ill,  _imp},{ror,  _dir},{asr,  _dir},
	{asl,  _dir},{rol,  _dir},{dec,  _dir},{ill,  _imp},
	{inc,  _dir},{tst,  _dir},{ill,  _imp},{clr,  _dir},
	{nega, _imp},{ill,  _imp},{ill,  _imp},{coma, _imp},/* 40 */
	{lsra, _imp},{ill,  _imp},{rora, _imp},{asra, _imp},
	{asla, _imp},{rola, _imp},{deca, _imp},{ill,  _imp},
	{inca, _imp},{tsta, _imp},{ill,  _imp},{clra, _imp},
	{negx, _imp},{ill,  _imp},{ill,  _imp},{comx, _imp},/* 50 */
	{lsrx, _imp},{ill,  _imp},{rorx, _imp},{asrx, _imp},
	{aslx, _imp},{rolx, _imp},{decx, _imp},{ill,  _imp},
	{incx, _imp},{tstx, _imp},{ill,  _imp},{clrx, _imp},
	{neg,  _ix1},{ill,  _imp},{ill,  _imp},{com,  _ix1},/* 60 */
	{lsr,  _ix1},{ill,  _imp},{ror,  _ix1},{asr,  _ix1},
	{asl,  _ix1},{rol,  _ix1},{dec,  _ix1},{ill,  _imp},
	{inc,  _ix1},{tst,  _ix1},{jmp,  _ix1},{clr,  _ix1},
	{neg,  _idx},{ill,  _imp},{ill,  _imp},{com,  _idx},/* 70 */
	{lsr,  _idx},{ill,  _imp},{ror,  _idx},{asr,  _idx},
	{asl,  _idx},{rol,  _idx},{dec,  _idx},{ill,  _imp},
	{inc,  _idx},{tst,  _idx},{jmp,  _idx},{clr,  _idx},
	{rti,  _imp},{rts,  _imp},{ill,  _imp},{swi,  _imp},/* 80 */
	{ill,  _imp},{ill,  _imp},{ill,  _imp},{ill,  _imp},
	{ill,  _imp},{ill,  _imp},{ill,  _imp},{ill,  _imp},
	{ill,  _imp},{ill,  _imp},{ill,  _imp},{ill,  _imp},
	{ill,  _imp},{ill,  _imp},{ill,  _imp},{ill,  _imp},/* 90 */
	{ill,  _imp},{ill,  _imp},{ill,  _imp},{tax,  _imp},
	{clc,  _imp},{sec,  _imp},{cli,  _imp},{sei,  _imp},
	{rsp,  _imp},{nop,  _imp},{ill,  _imp},{txa,  _imp},
	{suba, _imm},{cmpa, _imm},{sbca, _imm},{cpx,  _imm},/* a0 */
	{anda, _imm},{bita, _imm},{lda,  _imm},{ill,  _imp},
	{eora, _imm},{adca, _imm},{ora,  _imm},{adda, _imm},
	{ill,  _imp},{bsr,  _rel},{ldx,  _imm},{ill,  _imp},
	{suba, _dir},{cmpa, _dir},{sbca, _dir},{cpx,  _dir},/* b0 */
	{anda, _dir},{bita, _dir},{lda,  _dir},{sta,  _dir},
	{eora, _dir},{adca, _dir},{ora,  _dir},{adda, _dir},
	{jmp,  _dir},{jsr,  _dir},{ldx,  _dir},{stx,  _dir},
	{suba, _ext},{cmpa, _ext},{sbca, _ext},{cpx,  _ext},/* c0 */
	{anda, _ext},{bita, _ext},{lda,  _ext},{sta,  _ext},
	{eora, _ext},{adca, _ext},{ora,  _ext},{adda, _ext},
	{jmp,  _ext},{jsr,  _ext},{ldx,  _ext},{stx,  _ext},
	{suba, _ix2},{cmpa, _ix2},{sbca, _ix2},{cpx,  _ix2},/* d0 */
	{anda, _ix2},{bita, _ix2},{lda,  _ix2},{sta,  _ix2},
	{eora, _ix2},{adca, _ix2},{ora,  _ix2},{adda, _ix2},
	{jmp,  _ix2},{jsr,  _ix2},{ldx,  _ix2},{stx,  _ix2},
	{suba, _ix1},{cmpa, _ix1},{sbca, _ix1},{cpx,  _ix1},/* e0 */
	{anda, _ix1},{bita, _ix1},{lda,  _ix1},{sta,  _ix1},
	{eora, _ix1},{adca, _ix1},{ora,  _ix1},{adda, _ix1},
	{jmp,  _ix1},{jsr,  _ix1},{ldx,  _ix1},{stx,  _ix1},
	{suba, _idx},{cmpa, _idx},{sbca, _idx},{cpx,  _idx},/* f0 */
	{anda, _idx},{bita, _idx},{lda,  _idx},{sta,  _idx},
	{eora, _idx},{adca, _idx},{ora,  _idx},{adda, _idx},
	{jmp,  _idx},{jsr,  _idx},{ldx,  _idx},{stx,  _idx}
};

#if 0
static char *opcode_strings[0x0100] =
{
	"brset0", 	"brclr0", 	"brset1", 	"brclr1", 	"brset2", 	"brclr2",	"brset3",	"brclr3",		/*00*/
	"brset4",	"brclr4",	"brset5",	"brclr5",	"brset6",	"brclr6",	"brset7",	"brclr7",
	"bset0",	"bclr0",	"bset1", 	"bclr1", 	"bset2", 	"bclr2", 	"bset3",	"bclr3",		/*10*/
	"bset4", 	"bclr4",	"bset5", 	"bclr5",	"bset6", 	"bclr6", 	"bset7", 	"bclr7",
	"bra",		"brn",		"bhi",		"bls",		"bcc",		"bcs",		"bne",		"beq",			/*20*/
	"bhcc",		"bhcs",		"bpl",		"bmi",		"bmc",		"bms",		"bil",		"bih",
	"neg_di",   "illegal",  "illegal",  "com_di",   "lsr_di",   "illegal",  "ror_di",   "asr_di",       /*30*/
	"asl_di",	"rol_di",	"dec_di",	"illegal", 	"inc_di",	"tst_di",	"illegal", 	"clr_di",
	"nega",		"illegal", 	"illegal", 	"coma",		"lsra",		"illegal", 	"rora",		"asra",			/*40*/
	"asla",		"rola",		"deca",		"illegal", 	"inca",		"tsta",		"illegal", 	"clra",
	"negx",		"illegal", 	"illegal", 	"comx",		"lsrx",		"illegal", 	"rorx",		"asrx",			/*50*/
	"aslx",		"rolx",		"decx",		"illegal", 	"incx",		"tstx",		"illegal", 	"clrx",
	"neg_ix1",	"illegal", 	"illegal", 	"com_ix1",	"lsr_ix1",	"illegal", 	"ror_ix1",	"asr_ix1",		/*60*/
	"asl_ix1",	"rol_ix1",	"dec_ix1",	"illegal", 	"inc_ix1",	"tst_ix1",	"jmp_ix1",	"clr_ix1",
	"neg_ix",	"illegal", 	"illegal", 	"com_ix",	"lsr_ix",	"illegal", 	"ror_ix",	"asr_ix",		/*70*/
	"asl_ix",	"rol_ix",	"dec_ix",	"illegal", 	"inc_ix",	"tst_ix",	"jmp_ix",	"clr_ix",
	"rti",		"rts",		"illegal",	"swi",		"illegal",	"illegal",	"illegal",	"illegal",		/*80*/
	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",
	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"illegal",	"tax",			/*90*/
	"clc",		"sec",		"cli",		"sei",		"rsp",		"nop",		"illegal",	"txa",
	"suba_im",	"cmpa_im",	"sbca_im",	"cpx_im", 	"anda_im",	"bita_im",	"lda_im",	"illegal",		/*A0*/
	"eora_im",	"adca_im",	"ora_im",	"adda_im",	"illegal",	"bsr",		"ldx_im",	"illegal",
	"suba_di",	"cmpa_di",	"sbca_di",	"cpx_di", 	"anda_di",	"bita_di",	"lda_di",	"sta_di",		/*B0*/
	"eora_di",	"adca_di",	"ora_di",	"adda_di",	"jmp_di",	"jsr_di",	"ldx_di",	"stx_di",
	"suba_ex",	"cmpa_ex",	"sbca_ex",	"cpx_ex", 	"anda_ex",	"bita_ex",	"lda_ex",	"sta_ex",		/*C0*/
	"eora_ex",	"adca_ex",	"ora_ex",	"adda_ex",	"jmp_ex",	"jsr_ex",	"ldx_ex",	"stx_ex",
	"suba_ix2",	"cmpa_ix2",	"sbca_ix2",	"cpx_ix2", 	"anda_ix2",	"bita_ix2",	"lda_ix2",	"sta_ix2",		/*D0*/
	"eora_ix2",	"adca_ix2",	"ora_ix2",	"adda_ix2",	"jmp_ix2",	"jsr_ix2",	"ldx_ix2",	"stx_ix2",
	"suba_ix1",	"cmpa_ix1",	"sbca_ix1",	"cpx_ix1", 	"anda_ix1",	"bita_ix1",	"lda_ix1",	"sta_ix1",		/*E0*/
	"eora_ix1",	"adca_ix1",	"ora_ix1",	"adda_ix1",	"jmp_ix1",	"jsr_ix1",	"ldx_ix1",	"stx_ix1",
	"suba_ix",	"cmpa_ix",	"sbca_ix",	"cpx_ix", 	"anda_ix",	"bita_ix",	"lda_ix",	"sta_ix",		/*F0*/
	"eora_ix",	"adca_ix",	"ora_ix",	"adda_ix",	"jmp_ix",	"jsr_ix",	"ldx_ix",	"stx_ix"
};
#endif

offs_t m6805_dasm(char *buf, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
    int code, bit;
	UINT16 ea;
	UINT32 flags = 0;

	code = oprom[0];

	if (disasm[code][0] == bsr || disasm[code][0] == jsr)
		flags = DASMFLAG_STEP_OVER;
	else if (disasm[code][0] == rts || disasm[code][0] == rti)
		flags = DASMFLAG_STEP_OUT;

	buf += sprintf(buf, "%-6s", op_name_str[disasm[code][0]]);

	switch( disasm[code][1] )
	{
	case _btr:	/* bit test and relative branch */
		bit = (code >> 1) & 7;
		sprintf (buf, "%d,$%02X,$%03X", bit, opram[1], pc + 3 + (INT8)opram[2]);
		return 3 | flags | DASMFLAG_SUPPORTED;
	case _bit:	/* bit test */
		bit = (code >> 1) & 7;
		sprintf (buf, "%d,$%03X", bit, opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;
	case _rel:	/* relative */
		sprintf (buf, "$%03X", pc + 2 + (INT8)opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;
	case _imm:	/* immediate */
		sprintf (buf, "#$%02X", opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;
	case _dir:	/* direct (zero page address) */
        sprintf (buf, "$%02X", opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;
	case _ext:	/* extended (16 bit address) */
		ea = (opram[1] << 8) + opram[2];
		sprintf (buf, "$%04X", ea);
		return 3 | flags | DASMFLAG_SUPPORTED;
	case _idx:	/* indexed */
		sprintf (buf, "(x)");
		return 1 | flags | DASMFLAG_SUPPORTED;
	case _ix1:	/* indexed + byte (zero page) */
		sprintf (buf, "(x+$%02X)", opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;
	case _ix2:	/* indexed + word (16 bit address) */
		ea = (opram[1] << 8) + opram[2];
		sprintf (buf, "(x+$%04X)", ea);
		return 3 | flags | DASMFLAG_SUPPORTED;
    default:    /* implicit */
		return 1 | flags | DASMFLAG_SUPPORTED;
    }
}
