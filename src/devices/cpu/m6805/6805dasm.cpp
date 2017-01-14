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
#include "m6805.h"

#include "debugger.h"

namespace {

enum class md {
	IMP,    // implicit
	BTR,    // bit test and relative
	BIT,    // bit set/clear
	REL,    // relative
	IMM,    // immediate
	DIR,    // direct address
	EXT,    // extended address
	IDX,    // indexed
	IX1,    // indexed + byte offset
	IX2     // indexed + word offset
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

char const *const op_name_str[] = {
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

struct { u8 op; md mode; } const disasm[0x100] = {
	{brset,md::BTR}, {brclr,md::BTR}, {brset,md::BTR}, {brclr,md::BTR}, // 00
	{brset,md::BTR}, {brclr,md::BTR}, {brset,md::BTR}, {brclr,md::BTR},
	{brset,md::BTR}, {brclr,md::BTR}, {brset,md::BTR}, {brclr,md::BTR},
	{brset,md::BTR}, {brclr,md::BTR}, {brset,md::BTR}, {brclr,md::BTR},
	{bset, md::BIT}, {bclr, md::BIT}, {bset, md::BIT}, {bclr, md::BIT}, // 10
	{bset, md::BIT}, {bclr, md::BIT}, {bset, md::BIT}, {bclr, md::BIT},
	{bset, md::BIT}, {bclr, md::BIT}, {bset, md::BIT}, {bclr, md::BIT},
	{bset, md::BIT}, {bclr, md::BIT}, {bset, md::BIT}, {bclr, md::BIT},
	{bra,  md::REL}, {brn,  md::REL}, {bhi,  md::REL}, {bls,  md::REL}, // 20
	{bcc,  md::REL}, {bcs,  md::REL}, {bne,  md::REL}, {beq,  md::REL},
	{bhcc, md::REL}, {bhcs, md::REL}, {bpl,  md::REL}, {bmi,  md::REL},
	{bmc,  md::REL}, {bms,  md::REL}, {bil,  md::REL}, {bih,  md::REL},
	{neg,  md::DIR}, {ill,  md::IMP}, {ill,  md::IMP}, {com,  md::DIR}, // 30
	{lsr,  md::DIR}, {ill,  md::IMP}, {ror,  md::DIR}, {asr,  md::DIR},
	{asl,  md::DIR}, {rol,  md::DIR}, {dec,  md::DIR}, {ill,  md::IMP},
	{inc,  md::DIR}, {tst,  md::DIR}, {ill,  md::IMP}, {clr,  md::DIR},
	{nega, md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {coma, md::IMP}, // 40
	{lsra, md::IMP}, {ill,  md::IMP}, {rora, md::IMP}, {asra, md::IMP},
	{asla, md::IMP}, {rola, md::IMP}, {deca, md::IMP}, {ill,  md::IMP},
	{inca, md::IMP}, {tsta, md::IMP}, {ill,  md::IMP}, {clra, md::IMP},
	{negx, md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {comx, md::IMP}, // 50
	{lsrx, md::IMP}, {ill,  md::IMP}, {rorx, md::IMP}, {asrx, md::IMP},
	{aslx, md::IMP}, {rolx, md::IMP}, {decx, md::IMP}, {ill,  md::IMP},
	{incx, md::IMP}, {tstx, md::IMP}, {ill,  md::IMP}, {clrx, md::IMP},
	{neg,  md::IX1}, {ill,  md::IMP}, {ill,  md::IMP}, {com,  md::IX1}, // 60
	{lsr,  md::IX1}, {ill,  md::IMP}, {ror,  md::IX1}, {asr,  md::IX1},
	{asl,  md::IX1}, {rol,  md::IX1}, {dec,  md::IX1}, {ill,  md::IMP},
	{inc,  md::IX1}, {tst,  md::IX1}, {jmp,  md::IX1}, {clr,  md::IX1},
	{neg,  md::IDX}, {ill,  md::IMP}, {ill,  md::IMP}, {com,  md::IDX}, // 70
	{lsr,  md::IDX}, {ill,  md::IMP}, {ror,  md::IDX}, {asr,  md::IDX},
	{asl,  md::IDX}, {rol,  md::IDX}, {dec,  md::IDX}, {ill,  md::IMP},
	{inc,  md::IDX}, {tst,  md::IDX}, {jmp,  md::IDX}, {clr,  md::IDX},
	{rti,  md::IMP}, {rts,  md::IMP}, {ill,  md::IMP}, {swi,  md::IMP}, // 80
	{ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP},
	{ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP},
	{ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP},
	{ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, // 90
	{ill,  md::IMP}, {ill,  md::IMP}, {ill,  md::IMP}, {tax,  md::IMP},
	{clc,  md::IMP}, {sec,  md::IMP}, {cli,  md::IMP}, {sei,  md::IMP},
	{rsp,  md::IMP}, {nop,  md::IMP}, {ill,  md::IMP}, {txa,  md::IMP},
	{suba, md::IMM}, {cmpa, md::IMM}, {sbca, md::IMM}, {cpx,  md::IMM}, // a0
	{anda, md::IMM}, {bita, md::IMM}, {lda,  md::IMM}, {ill,  md::IMP},
	{eora, md::IMM}, {adca, md::IMM}, {ora,  md::IMM}, {adda, md::IMM},
	{ill,  md::IMP}, {bsr,  md::REL}, {ldx,  md::IMM}, {ill,  md::IMP},
	{suba, md::DIR}, {cmpa, md::DIR}, {sbca, md::DIR}, {cpx,  md::DIR}, // b0
	{anda, md::DIR}, {bita, md::DIR}, {lda,  md::DIR}, {sta,  md::DIR},
	{eora, md::DIR}, {adca, md::DIR}, {ora,  md::DIR}, {adda, md::DIR},
	{jmp,  md::DIR}, {jsr,  md::DIR}, {ldx,  md::DIR}, {stx,  md::DIR},
	{suba, md::EXT}, {cmpa, md::EXT}, {sbca, md::EXT}, {cpx,  md::EXT}, // c0
	{anda, md::EXT}, {bita, md::EXT}, {lda,  md::EXT}, {sta,  md::EXT},
	{eora, md::EXT}, {adca, md::EXT}, {ora,  md::EXT}, {adda, md::EXT},
	{jmp,  md::EXT}, {jsr,  md::EXT}, {ldx,  md::EXT}, {stx,  md::EXT},
	{suba, md::IX2}, {cmpa, md::IX2}, {sbca, md::IX2}, {cpx,  md::IX2}, // d0
	{anda, md::IX2}, {bita, md::IX2}, {lda,  md::IX2}, {sta,  md::IX2},
	{eora, md::IX2}, {adca, md::IX2}, {ora,  md::IX2}, {adda, md::IX2},
	{jmp,  md::IX2}, {jsr,  md::IX2}, {ldx,  md::IX2}, {stx,  md::IX2},
	{suba, md::IX1}, {cmpa, md::IX1}, {sbca, md::IX1}, {cpx,  md::IX1}, // e0
	{anda, md::IX1}, {bita, md::IX1}, {lda,  md::IX1}, {sta,  md::IX1},
	{eora, md::IX1}, {adca, md::IX1}, {ora,  md::IX1}, {adda, md::IX1},
	{jmp,  md::IX1}, {jsr,  md::IX1}, {ldx,  md::IX1}, {stx,  md::IX1},
	{suba, md::IDX}, {cmpa, md::IDX}, {sbca, md::IDX}, {cpx,  md::IDX}, // f0
	{anda, md::IDX}, {bita, md::IDX}, {lda,  md::IDX}, {sta,  md::IDX},
	{eora, md::IDX}, {adca, md::IDX}, {ora,  md::IDX}, {adda, md::IDX},
	{jmp,  md::IDX}, {jsr,  md::IDX}, {ldx,  md::IDX}, {stx,  md::IDX}
};

#if 0
char const *const opcode_strings[0x0100] =
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


template <typename T>
void format_address(
		std::ostream& stream,
		T address,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count)
{
	auto const symbol= std::lower_bound(
			&symbols[0],
			&symbols[symbol_count],
			address,
			[] (auto const &sym, u16 addr) { return sym.first < addr; });
	if ((symbol_count != (symbol - symbols)) && (symbol->first == address))
		stream << symbol->second;
	else
		util::stream_format(stream, "$%0*X", 2 * sizeof(T), address);
}

} // anonymous namespace


offs_t CPU_DISASSEMBLE_NAME(m6805)(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count)
{
	u8 const code = oprom[0];

	u32 flags = 0;
	switch (disasm[code].op)
	{
	case bsr:
	case jsr:
		flags = DASMFLAG_STEP_OVER;
		break;
	case rts:
	case rti:
		flags = DASMFLAG_STEP_OUT;
		break;
	}

	util::stream_format(stream, "%-6s", op_name_str[disasm[code].op]);

	int bit;
	u16 ea;
	switch (disasm[code].mode)
	{
	case md::IMP:   // implicit
		return 1 | flags | DASMFLAG_SUPPORTED;

	case md::BTR:   // bit test and relative branch
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,", bit);
		format_address(stream, opram[1], symbols, symbol_count);
		util::stream_format(stream, ",$%03X", pc + 3 + s8(opram[2]));
		return 3 | flags | DASMFLAG_SUPPORTED;

	case md::BIT:   // bit test
		bit = (code >> 1) & 7;
		util::stream_format(stream, "%d,", bit);
		format_address(stream, opram[1], symbols, symbol_count);
		return 2 | flags | DASMFLAG_SUPPORTED;

	case md::REL:   // relative
		util::stream_format(stream, "$%03X", pc + 2 + s8(opram[1]));
		return 2 | flags | DASMFLAG_SUPPORTED;

	case md::IMM:   // immediate
		util::stream_format(stream, "#$%02X", opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;

	case md::DIR:   // direct (zero page address)
		format_address(stream, opram[1], symbols, symbol_count);
		return 2 | flags | DASMFLAG_SUPPORTED;

	case md::EXT:   // extended (16 bit address)
		ea = (opram[1] << 8) + opram[2];
		format_address(stream, ea, symbols, symbol_count);
		return 3 | flags | DASMFLAG_SUPPORTED;

	case md::IDX:   // indexed
		util::stream_format(stream, "(x)");
		return 1 | flags | DASMFLAG_SUPPORTED;

	case md::IX1:   // indexed + byte (zero page)
		util::stream_format(stream, "(x+$%02X)", opram[1]);
		return 2 | flags | DASMFLAG_SUPPORTED;

	case md::IX2:   // indexed + word (16 bit address)
		ea = (opram[1] << 8) + opram[2];
		util::stream_format(stream, "(x+$%04X)", ea);
		return 3 | flags | DASMFLAG_SUPPORTED;
	}

	// if we fall off the switch statement something is very wrong
	throw false;
}


CPU_DISASSEMBLE(m6805) { return CPU_DISASSEMBLE_NAME(m6805)(device, stream, pc, oprom, opram, options, nullptr, 0); }
