// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*
 *   A quick-hack 68(7)05 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include "emu.h"
#include "debugger.h"
#include "m6805.h"

enum addr_mode {
	md_imp=0,     /* implicit */
	md_btr,       /* bit test and relative */
	md_bit,       /* bit set/clear */
	md_rel,       /* relative */
	md_imm,       /* immediate */
	md_dir,       /* direct address */
	md_ext,       /* extended address */
	md_idx,       /* indexed */
	md_ix1,       /* indexed + byte offset */
	md_ix2        /* indexed + word offset */
};

enum op_names {
	adca=0, adda,   anda,   asl,    asla,   aslx,   asr,    asra,
	asrx,   bcc,    bclr,   bcs,    beq,    bhcc,   bhcs,   bhi,
	bih,    bil,    bita,   bls,    bmc,    bmi,    bms,    bne,
	bpl,    bra,    brclr,  brn,    brset,  bset,   bsr,    clc,
	cli,    clr,    clra,   clrx,   cmpa,   com,    coma,   comx,
	cpx,    dec,    deca,   decx,   eora,   ill,    inc,    inca,
	incx,   jmp,    jsr,    lda,    ldx,    lsr,    lsra,   lsrx,
	neg,    nega,   negx,   nop,    ora,    rol,    rola,   rolx,
	ror,    rora,   rorx,   rsp,    rti,    rts,    sbca,   sec,
	sei,    sta,    stx,    suba,   swi,    tax,    tst,    tsta,
	tstx,   txa
};

static const char *const op_name_str[] = {
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
	{brset,md_btr},{brclr,md_btr},{brset,md_btr},{brclr,md_btr},/* 00 */
	{brset,md_btr},{brclr,md_btr},{brset,md_btr},{brclr,md_btr},
	{brset,md_btr},{brclr,md_btr},{brset,md_btr},{brclr,md_btr},
	{brset,md_btr},{brclr,md_btr},{brset,md_btr},{brclr,md_btr},
	{bset, md_bit},{bclr, md_bit},{bset, md_bit},{bclr, md_bit},/* 10 */
	{bset, md_bit},{bclr, md_bit},{bset, md_bit},{bclr, md_bit},
	{bset, md_bit},{bclr, md_bit},{bset, md_bit},{bclr, md_bit},
	{bset, md_bit},{bclr, md_bit},{bset, md_bit},{bclr, md_bit},
	{bra,  md_rel},{brn,  md_rel},{bhi,  md_rel},{bls,  md_rel},/* 20 */
	{bcc,  md_rel},{bcs,  md_rel},{bne,  md_rel},{beq,  md_rel},
	{bhcc, md_rel},{bhcs, md_rel},{bpl,  md_rel},{bmi,  md_rel},
	{bmc,  md_rel},{bms,  md_rel},{bil,  md_rel},{bih,  md_rel},
	{neg,  md_dir},{ill,  md_imp},{ill,  md_imp},{com,  md_dir},/* 30 */
	{lsr,  md_dir},{ill,  md_imp},{ror,  md_dir},{asr,  md_dir},
	{asl,  md_dir},{rol,  md_dir},{dec,  md_dir},{ill,  md_imp},
	{inc,  md_dir},{tst,  md_dir},{ill,  md_imp},{clr,  md_dir},
	{nega, md_imp},{ill,  md_imp},{ill,  md_imp},{coma, md_imp},/* 40 */
	{lsra, md_imp},{ill,  md_imp},{rora, md_imp},{asra, md_imp},
	{asla, md_imp},{rola, md_imp},{deca, md_imp},{ill,  md_imp},
	{inca, md_imp},{tsta, md_imp},{ill,  md_imp},{clra, md_imp},
	{negx, md_imp},{ill,  md_imp},{ill,  md_imp},{comx, md_imp},/* 50 */
	{lsrx, md_imp},{ill,  md_imp},{rorx, md_imp},{asrx, md_imp},
	{aslx, md_imp},{rolx, md_imp},{decx, md_imp},{ill,  md_imp},
	{incx, md_imp},{tstx, md_imp},{ill,  md_imp},{clrx, md_imp},
	{neg,  md_ix1},{ill,  md_imp},{ill,  md_imp},{com,  md_ix1},/* 60 */
	{lsr,  md_ix1},{ill,  md_imp},{ror,  md_ix1},{asr,  md_ix1},
	{asl,  md_ix1},{rol,  md_ix1},{dec,  md_ix1},{ill,  md_imp},
	{inc,  md_ix1},{tst,  md_ix1},{jmp,  md_ix1},{clr,  md_ix1},
	{neg,  md_idx},{ill,  md_imp},{ill,  md_imp},{com,  md_idx},/* 70 */
	{lsr,  md_idx},{ill,  md_imp},{ror,  md_idx},{asr,  md_idx},
	{asl,  md_idx},{rol,  md_idx},{dec,  md_idx},{ill,  md_imp},
	{inc,  md_idx},{tst,  md_idx},{jmp,  md_idx},{clr,  md_idx},
	{rti,  md_imp},{rts,  md_imp},{ill,  md_imp},{swi,  md_imp},/* 80 */
	{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},
	{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},
	{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},
	{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},/* 90 */
	{ill,  md_imp},{ill,  md_imp},{ill,  md_imp},{tax,  md_imp},
	{clc,  md_imp},{sec,  md_imp},{cli,  md_imp},{sei,  md_imp},
	{rsp,  md_imp},{nop,  md_imp},{ill,  md_imp},{txa,  md_imp},
	{suba, md_imm},{cmpa, md_imm},{sbca, md_imm},{cpx,  md_imm},/* a0 */
	{anda, md_imm},{bita, md_imm},{lda,  md_imm},{ill,  md_imp},
	{eora, md_imm},{adca, md_imm},{ora,  md_imm},{adda, md_imm},
	{ill,  md_imp},{bsr,  md_rel},{ldx,  md_imm},{ill,  md_imp},
	{suba, md_dir},{cmpa, md_dir},{sbca, md_dir},{cpx,  md_dir},/* b0 */
	{anda, md_dir},{bita, md_dir},{lda,  md_dir},{sta,  md_dir},
	{eora, md_dir},{adca, md_dir},{ora,  md_dir},{adda, md_dir},
	{jmp,  md_dir},{jsr,  md_dir},{ldx,  md_dir},{stx,  md_dir},
	{suba, md_ext},{cmpa, md_ext},{sbca, md_ext},{cpx,  md_ext},/* c0 */
	{anda, md_ext},{bita, md_ext},{lda,  md_ext},{sta,  md_ext},
	{eora, md_ext},{adca, md_ext},{ora,  md_ext},{adda, md_ext},
	{jmp,  md_ext},{jsr,  md_ext},{ldx,  md_ext},{stx,  md_ext},
	{suba, md_ix2},{cmpa, md_ix2},{sbca, md_ix2},{cpx,  md_ix2},/* d0 */
	{anda, md_ix2},{bita, md_ix2},{lda,  md_ix2},{sta,  md_ix2},
	{eora, md_ix2},{adca, md_ix2},{ora,  md_ix2},{adda, md_ix2},
	{jmp,  md_ix2},{jsr,  md_ix2},{ldx,  md_ix2},{stx,  md_ix2},
	{suba, md_ix1},{cmpa, md_ix1},{sbca, md_ix1},{cpx,  md_ix1},/* e0 */
	{anda, md_ix1},{bita, md_ix1},{lda,  md_ix1},{sta,  md_ix1},
	{eora, md_ix1},{adca, md_ix1},{ora,  md_ix1},{adda, md_ix1},
	{jmp,  md_ix1},{jsr,  md_ix1},{ldx,  md_ix1},{stx,  md_ix1},
	{suba, md_idx},{cmpa, md_idx},{sbca, md_idx},{cpx,  md_idx},/* f0 */
	{anda, md_idx},{bita, md_idx},{lda,  md_idx},{sta,  md_idx},
	{eora, md_idx},{adca, md_idx},{ora,  md_idx},{adda, md_idx},
	{jmp,  md_idx},{jsr,  md_idx},{ldx,  md_idx},{stx,  md_idx}
};

#if 0
static const char *const opcode_strings[0x0100] =
{
	"brset0",   "brclr0",   "brset1",   "brclr1",   "brset2",   "brclr2",   "brset3",   "brclr3",       /*00*/
	"brset4",   "brclr4",   "brset5",   "brclr5",   "brset6",   "brclr6",   "brset7",   "brclr7",
	"bset0",    "bclr0",    "bset1",    "bclr1",    "bset2",    "bclr2",    "bset3",    "bclr3",        /*10*/
	"bset4",    "bclr4",    "bset5",    "bclr5",    "bset6",    "bclr6",    "bset7",    "bclr7",
	"bra",      "brn",      "bhi",      "bls",      "bcc",      "bcs",      "bne",      "beq",          /*20*/
	"bhcc",     "bhcs",     "bpl",      "bmi",      "bmc",      "bms",      "bil",      "bih",
	"neg_di",   "illegal",  "illegal",  "com_di",   "lsr_di",   "illegal",  "ror_di",   "asr_di",       /*30*/
	"asl_di",   "rol_di",   "dec_di",   "illegal",  "inc_di",   "tst_di",   "illegal",  "clr_di",
	"nega",     "illegal",  "illegal",  "coma",     "lsra",     "illegal",  "rora",     "asra",         /*40*/
	"asla",     "rola",     "deca",     "illegal",  "inca",     "tsta",     "illegal",  "clra",
	"negx",     "illegal",  "illegal",  "comx",     "lsrx",     "illegal",  "rorx",     "asrx",         /*50*/
	"aslx",     "rolx",     "decx",     "illegal",  "incx",     "tstx",     "illegal",  "clrx",
	"neg_ix1",  "illegal",  "illegal",  "com_ix1",  "lsr_ix1",  "illegal",  "ror_ix1",  "asr_ix1",      /*60*/
	"asl_ix1",  "rol_ix1",  "dec_ix1",  "illegal",  "inc_ix1",  "tst_ix1",  "jmp_ix1",  "clr_ix1",
	"neg_ix",   "illegal",  "illegal",  "com_ix",   "lsr_ix",   "illegal",  "ror_ix",   "asr_ix",       /*70*/
	"asl_ix",   "rol_ix",   "dec_ix",   "illegal",  "inc_ix",   "tst_ix",   "jmp_ix",   "clr_ix",
	"rti",      "rts",      "illegal",  "swi",      "illegal",  "illegal",  "illegal",  "illegal",      /*80*/
	"illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "illegal",
	"illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "illegal",  "tax",          /*90*/
	"clc",      "sec",      "cli",      "sei",      "rsp",      "nop",      "illegal",  "txa",
	"suba_im",  "cmpa_im",  "sbca_im",  "cpx_im",   "anda_im",  "bita_im",  "lda_im",   "illegal",      /*A0*/
	"eora_im",  "adca_im",  "ora_im",   "adda_im",  "illegal",  "bsr",      "ldx_im",   "illegal",
	"suba_di",  "cmpa_di",  "sbca_di",  "cpx_di",   "anda_di",  "bita_di",  "lda_di",   "sta_di",       /*B0*/
	"eora_di",  "adca_di",  "ora_di",   "adda_di",  "jmp_di",   "jsr_di",   "ldx_di",   "stx_di",
	"suba_ex",  "cmpa_ex",  "sbca_ex",  "cpx_ex",   "anda_ex",  "bita_ex",  "lda_ex",   "sta_ex",       /*C0*/
	"eora_ex",  "adca_ex",  "ora_ex",   "adda_ex",  "jmp_ex",   "jsr_ex",   "ldx_ex",   "stx_ex",
	"suba_ix2", "cmpa_ix2", "sbca_ix2", "cpx_ix2",  "anda_ix2", "bita_ix2", "lda_ix2",  "sta_ix2",      /*D0*/
	"eora_ix2", "adca_ix2", "ora_ix2",  "adda_ix2", "jmp_ix2",  "jsr_ix2",  "ldx_ix2",  "stx_ix2",
	"suba_ix1", "cmpa_ix1", "sbca_ix1", "cpx_ix1",  "anda_ix1", "bita_ix1", "lda_ix1",  "sta_ix1",      /*E0*/
	"eora_ix1", "adca_ix1", "ora_ix1",  "adda_ix1", "jmp_ix1",  "jsr_ix1",  "ldx_ix1",  "stx_ix1",
	"suba_ix",  "cmpa_ix",  "sbca_ix",  "cpx_ix",   "anda_ix",  "bita_ix",  "lda_ix",   "sta_ix",       /*F0*/
	"eora_ix",  "adca_ix",  "ora_ix",   "adda_ix",  "jmp_ix",   "jsr_ix",   "ldx_ix",   "stx_ix"
};
#endif

CPU_DISASSEMBLE(m6805)
{
	int code, bit;
	uint16_t ea;
	uint32_t flags = 0;
	offs_t result;

	code = oprom[0];

	if (disasm[code][0] == bsr || disasm[code][0] == jsr)
		flags = DASMFLAG_STEP_OVER;
	else if (disasm[code][0] == rts || disasm[code][0] == rti)
		flags = DASMFLAG_STEP_OUT;

	util::stream_format(stream, "%-6s", op_name_str[disasm[code][0]]);

	switch( disasm[code][1] )
	{
	case md_btr:  /* bit test and relative branch */
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,$%02X,$%03X", bit, opram[1], pc + 3 + (int8_t)opram[2]);
		result = 3 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_bit:  /* bit test */
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,$%03X", bit, opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_rel:  /* relative */
		util::stream_format(stream, "$%03X", pc + 2 + (int8_t)opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_imm:  /* immediate */
		util::stream_format(stream, "#$%02X", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_dir:  /* direct (zero page address) */
		util::stream_format(stream, "$%02X", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_ext:  /* extended (16 bit address) */
		ea = (opram[1] << 8) + opram[2];
		util::stream_format(stream, "$%04X", ea);
		result = 3 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_idx:  /* indexed */
		util::stream_format(stream, "(x)");
		result = 1 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_ix1:  /* indexed + byte (zero page) */
		util::stream_format(stream, "(x+$%02X)", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case md_ix2:  /* indexed + word (16 bit address) */
		ea = (opram[1] << 8) + opram[2];
		util::stream_format(stream, "(x+$%04X)", ea);
		result = 3 | flags | DASMFLAG_SUPPORTED;
		break;
	default:    /* implicit */
		result = 1 | flags | DASMFLAG_SUPPORTED;
		break;
	}
	return result;
}
