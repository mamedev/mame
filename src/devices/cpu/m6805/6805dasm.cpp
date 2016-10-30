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
	m_imp=0,     /* implicit */
	m_btr,       /* bit test and relative */
	m_bit,       /* bit set/clear */
	m_rel,       /* relative */
	m_imm,       /* immediate */
	m_dir,       /* direct address */
	m_ext,       /* extended address */
	m_idx,       /* indexed */
	m_ix1,       /* indexed + byte offset */
	m_ix2        /* indexed + word offset */
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
	{brset,m_btr},{brclr,m_btr},{brset,m_btr},{brclr,m_btr},/* 00 */
	{brset,m_btr},{brclr,m_btr},{brset,m_btr},{brclr,m_btr},
	{brset,m_btr},{brclr,m_btr},{brset,m_btr},{brclr,m_btr},
	{brset,m_btr},{brclr,m_btr},{brset,m_btr},{brclr,m_btr},
	{bset, m_bit},{bclr, m_bit},{bset, m_bit},{bclr, m_bit},/* 10 */
	{bset, m_bit},{bclr, m_bit},{bset, m_bit},{bclr, m_bit},
	{bset, m_bit},{bclr, m_bit},{bset, m_bit},{bclr, m_bit},
	{bset, m_bit},{bclr, m_bit},{bset, m_bit},{bclr, m_bit},
	{bra,  m_rel},{brn,  m_rel},{bhi,  m_rel},{bls,  m_rel},/* 20 */
	{bcc,  m_rel},{bcs,  m_rel},{bne,  m_rel},{beq,  m_rel},
	{bhcc, m_rel},{bhcs, m_rel},{bpl,  m_rel},{bmi,  m_rel},
	{bmc,  m_rel},{bms,  m_rel},{bil,  m_rel},{bih,  m_rel},
	{neg,  m_dir},{ill,  m_imp},{ill,  m_imp},{com,  m_dir},/* 30 */
	{lsr,  m_dir},{ill,  m_imp},{ror,  m_dir},{asr,  m_dir},
	{asl,  m_dir},{rol,  m_dir},{dec,  m_dir},{ill,  m_imp},
	{inc,  m_dir},{tst,  m_dir},{ill,  m_imp},{clr,  m_dir},
	{nega, m_imp},{ill,  m_imp},{ill,  m_imp},{coma, m_imp},/* 40 */
	{lsra, m_imp},{ill,  m_imp},{rora, m_imp},{asra, m_imp},
	{asla, m_imp},{rola, m_imp},{deca, m_imp},{ill,  m_imp},
	{inca, m_imp},{tsta, m_imp},{ill,  m_imp},{clra, m_imp},
	{negx, m_imp},{ill,  m_imp},{ill,  m_imp},{comx, m_imp},/* 50 */
	{lsrx, m_imp},{ill,  m_imp},{rorx, m_imp},{asrx, m_imp},
	{aslx, m_imp},{rolx, m_imp},{decx, m_imp},{ill,  m_imp},
	{incx, m_imp},{tstx, m_imp},{ill,  m_imp},{clrx, m_imp},
	{neg,  m_ix1},{ill,  m_imp},{ill,  m_imp},{com,  m_ix1},/* 60 */
	{lsr,  m_ix1},{ill,  m_imp},{ror,  m_ix1},{asr,  m_ix1},
	{asl,  m_ix1},{rol,  m_ix1},{dec,  m_ix1},{ill,  m_imp},
	{inc,  m_ix1},{tst,  m_ix1},{jmp,  m_ix1},{clr,  m_ix1},
	{neg,  m_idx},{ill,  m_imp},{ill,  m_imp},{com,  m_idx},/* 70 */
	{lsr,  m_idx},{ill,  m_imp},{ror,  m_idx},{asr,  m_idx},
	{asl,  m_idx},{rol,  m_idx},{dec,  m_idx},{ill,  m_imp},
	{inc,  m_idx},{tst,  m_idx},{jmp,  m_idx},{clr,  m_idx},
	{rti,  m_imp},{rts,  m_imp},{ill,  m_imp},{swi,  m_imp},/* 80 */
	{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},
	{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},
	{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},
	{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},/* 90 */
	{ill,  m_imp},{ill,  m_imp},{ill,  m_imp},{tax,  m_imp},
	{clc,  m_imp},{sec,  m_imp},{cli,  m_imp},{sei,  m_imp},
	{rsp,  m_imp},{nop,  m_imp},{ill,  m_imp},{txa,  m_imp},
	{suba, m_imm},{cmpa, m_imm},{sbca, m_imm},{cpx,  m_imm},/* a0 */
	{anda, m_imm},{bita, m_imm},{lda,  m_imm},{ill,  m_imp},
	{eora, m_imm},{adca, m_imm},{ora,  m_imm},{adda, m_imm},
	{ill,  m_imp},{bsr,  m_rel},{ldx,  m_imm},{ill,  m_imp},
	{suba, m_dir},{cmpa, m_dir},{sbca, m_dir},{cpx,  m_dir},/* b0 */
	{anda, m_dir},{bita, m_dir},{lda,  m_dir},{sta,  m_dir},
	{eora, m_dir},{adca, m_dir},{ora,  m_dir},{adda, m_dir},
	{jmp,  m_dir},{jsr,  m_dir},{ldx,  m_dir},{stx,  m_dir},
	{suba, m_ext},{cmpa, m_ext},{sbca, m_ext},{cpx,  m_ext},/* c0 */
	{anda, m_ext},{bita, m_ext},{lda,  m_ext},{sta,  m_ext},
	{eora, m_ext},{adca, m_ext},{ora,  m_ext},{adda, m_ext},
	{jmp,  m_ext},{jsr,  m_ext},{ldx,  m_ext},{stx,  m_ext},
	{suba, m_ix2},{cmpa, m_ix2},{sbca, m_ix2},{cpx,  m_ix2},/* d0 */
	{anda, m_ix2},{bita, m_ix2},{lda,  m_ix2},{sta,  m_ix2},
	{eora, m_ix2},{adca, m_ix2},{ora,  m_ix2},{adda, m_ix2},
	{jmp,  m_ix2},{jsr,  m_ix2},{ldx,  m_ix2},{stx,  m_ix2},
	{suba, m_ix1},{cmpa, m_ix1},{sbca, m_ix1},{cpx,  m_ix1},/* e0 */
	{anda, m_ix1},{bita, m_ix1},{lda,  m_ix1},{sta,  m_ix1},
	{eora, m_ix1},{adca, m_ix1},{ora,  m_ix1},{adda, m_ix1},
	{jmp,  m_ix1},{jsr,  m_ix1},{ldx,  m_ix1},{stx,  m_ix1},
	{suba, m_idx},{cmpa, m_idx},{sbca, m_idx},{cpx,  m_idx},/* f0 */
	{anda, m_idx},{bita, m_idx},{lda,  m_idx},{sta,  m_idx},
	{eora, m_idx},{adca, m_idx},{ora,  m_idx},{adda, m_idx},
	{jmp,  m_idx},{jsr,  m_idx},{ldx,  m_idx},{stx,  m_idx}
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

static offs_t internal_disasm_m6805(cpu_device *device, std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, int options)
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
	case m_btr:  /* bit test and relative branch */
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,$%02X,$%03X", bit, opram[1], pc + 3 + (int8_t)opram[2]);
		result = 3 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_bit:  /* bit test */
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,$%03X", bit, opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_rel:  /* relative */
		util::stream_format(stream, "$%03X", pc + 2 + (int8_t)opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_imm:  /* immediate */
		util::stream_format(stream, "#$%02X", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_dir:  /* direct (zero page address) */
		util::stream_format(stream, "$%02X", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_ext:  /* extended (16 bit address) */
		ea = (opram[1] << 8) + opram[2];
		util::stream_format(stream, "$%04X", ea);
		result = 3 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_idx:  /* indexed */
		util::stream_format(stream, "(x)");
		result = 1 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_ix1:  /* indexed + byte (zero page) */
		util::stream_format(stream, "(x+$%02X)", opram[1]);
		result = 2 | flags | DASMFLAG_SUPPORTED;
		break;
	case m_ix2:  /* indexed + word (16 bit address) */
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


CPU_DISASSEMBLE(m6805)
{
	std::ostringstream stream;
	offs_t result = internal_disasm_m6805(device, stream, pc, oprom, opram, options);
	std::string stream_str = stream.str();
	strcpy(buffer, stream_str.c_str());
	return result;
}
