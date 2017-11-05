// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
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
	INH,    // inherent
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

enum class lvl {
	HMOS,
	CMOS,
	HC
};

enum class op_names {
	adca,   adda,   anda,   asl,    asla,   aslx,   asr,    asra,
	asrx,   bcc,    bclr,   bcs,    beq,    bhcc,   bhcs,   bhi,
	bih,    bil,    bit,    bls,    bmc,    bmi,    bms,    bne,
	bpl,    bra,    brclr,  brn,    brset,  bset,   bsr,    clc,
	cli,    clr,    clra,   clrx,   cmpa,   com,    coma,   comx,
	cpx,    dec,    deca,   decx,   eora,   ill,    inc,    inca,
	incx,   jmp,    jsr,    lda,    ldx,    lsr,    lsra,   lsrx,
	mul,    neg,    nega,   negx,   nop,    ora,    rol,    rola,
	rolx,   ror,    rora,   rorx,   rsp,    rti,    rts,    sbca,
	sec,    sei,    sta,    stop,   stx,    suba,   swi,    tax,
	tst,    tsta,   tstx,   txa,    wait
};

#define OP(name, mode)   { op_names::name, #name,   md::mode, lvl::HMOS }
#define OPC(name, mode)  { op_names::name, #name,   md::mode, lvl::CMOS }
#define OPHC(name, mode) { op_names::name, #name,   md::mode, lvl::HC   }
#define ILLEGAL          { op_names::ill,  nullptr, md::INH,  lvl::HMOS }
struct { op_names op; char const *name; md mode; lvl level; } const disasm[0x100] = {
	OP  (brset,BTR), OP  (brclr,BTR), OP  (brset,BTR), OP  (brclr,BTR), // 00
	OP  (brset,BTR), OP  (brclr,BTR), OP  (brset,BTR), OP  (brclr,BTR),
	OP  (brset,BTR), OP  (brclr,BTR), OP  (brset,BTR), OP  (brclr,BTR),
	OP  (brset,BTR), OP  (brclr,BTR), OP  (brset,BTR), OP  (brclr,BTR),
	OP  (bset, BIT), OP  (bclr, BIT), OP  (bset, BIT), OP  (bclr, BIT), // 10
	OP  (bset, BIT), OP  (bclr, BIT), OP  (bset, BIT), OP  (bclr, BIT),
	OP  (bset, BIT), OP  (bclr, BIT), OP  (bset, BIT), OP  (bclr, BIT),
	OP  (bset, BIT), OP  (bclr, BIT), OP  (bset, BIT), OP  (bclr, BIT),
	OP  (bra,  REL), OP  (brn,  REL), OP  (bhi,  REL), OP  (bls,  REL), // 20
	OP  (bcc,  REL), OP  (bcs,  REL), OP  (bne,  REL), OP  (beq,  REL),
	OP  (bhcc, REL), OP  (bhcs, REL), OP  (bpl,  REL), OP  (bmi,  REL),
	OP  (bmc,  REL), OP  (bms,  REL), OP  (bil,  REL), OP  (bih,  REL),
	OP  (neg,  DIR), ILLEGAL        , ILLEGAL        , OP  (com,  DIR), // 30
	OP  (lsr,  DIR), ILLEGAL        , OP  (ror,  DIR), OP  (asr,  DIR),
	OP  (asl,  DIR), OP  (rol,  DIR), OP  (dec,  DIR), ILLEGAL        ,
	OP  (inc,  DIR), OP  (tst,  DIR), ILLEGAL        , OP  (clr,  DIR),
	OP  (nega, INH), ILLEGAL        , OPHC(mul,  INH), OP  (coma, INH), // 40
	OP  (lsra, INH), ILLEGAL        , OP  (rora, INH), OP  (asra, INH),
	OP  (asla, INH), OP  (rola, INH), OP  (deca, INH), ILLEGAL        ,
	OP  (inca, INH), OP  (tsta, INH), ILLEGAL        , OP  (clra, INH),
	OP  (negx, INH), ILLEGAL        , ILLEGAL        , OP  (comx, INH), // 50
	OP  (lsrx, INH), ILLEGAL        , OP  (rorx, INH), OP  (asrx, INH),
	OP  (aslx, INH), OP  (rolx, INH), OP  (decx, INH), ILLEGAL        ,
	OP  (incx, INH), OP  (tstx, INH), ILLEGAL        , OP  (clrx, INH),
	OP  (neg,  IX1), ILLEGAL        , ILLEGAL        , OP  (com,  IX1), // 60
	OP  (lsr,  IX1), ILLEGAL        , OP  (ror,  IX1), OP  (asr,  IX1),
	OP  (asl,  IX1), OP  (rol,  IX1), OP  (dec,  IX1), ILLEGAL        ,
	OP  (inc,  IX1), OP  (tst,  IX1), OP  (jmp,  IX1), OP  (clr,  IX1),
	OP  (neg,  IDX), ILLEGAL        , ILLEGAL        , OP  (com,  IDX), // 70
	OP  (lsr,  IDX), ILLEGAL        , OP  (ror,  IDX), OP  (asr,  IDX),
	OP  (asl,  IDX), OP  (rol,  IDX), OP  (dec,  IDX), ILLEGAL        ,
	OP  (inc,  IDX), OP  (tst,  IDX), OP  (jmp,  IDX), OP  (clr,  IDX),
	OP  (rti,  INH), OP  (rts,  INH), ILLEGAL        , OP  (swi,  INH), // 80
	ILLEGAL        , ILLEGAL        , ILLEGAL        , ILLEGAL        ,
	ILLEGAL        , ILLEGAL        , ILLEGAL        , ILLEGAL        ,
	ILLEGAL        , ILLEGAL        , OPC (stop, INH), OPC (wait, INH),
	ILLEGAL        , ILLEGAL        , ILLEGAL        , ILLEGAL        , // 90
	ILLEGAL        , ILLEGAL        , ILLEGAL        , OP  (tax,  INH),
	OP  (clc,  INH), OP  (sec,  INH), OP  (cli,  INH), OP  (sei,  INH),
	OP  (rsp,  INH), OP  (nop,  INH), ILLEGAL        , OP  (txa,  INH),
	OP  (suba, IMM), OP  (cmpa, IMM), OP  (sbca, IMM), OP  (cpx,  IMM), // a0
	OP  (anda, IMM), OP  (bit,  IMM), OP  (lda,  IMM), ILLEGAL        ,
	OP  (eora, IMM), OP  (adca, IMM), OP  (ora,  IMM), OP  (adda, IMM),
	ILLEGAL        , OP  (bsr,  REL), OP  (ldx,  IMM), ILLEGAL        ,
	OP  (suba, DIR), OP  (cmpa, DIR), OP  (sbca, DIR), OP  (cpx,  DIR), // b0
	OP  (anda, DIR), OP  (bit,  DIR), OP  (lda,  DIR), OP  (sta,  DIR),
	OP  (eora, DIR), OP  (adca, DIR), OP  (ora,  DIR), OP  (adda, DIR),
	OP  (jmp,  DIR), OP  (jsr,  DIR), OP  (ldx,  DIR), OP  (stx,  DIR),
	OP  (suba, EXT), OP  (cmpa, EXT), OP  (sbca, EXT), OP  (cpx,  EXT), // c0
	OP  (anda, EXT), OP  (bit,  EXT), OP  (lda,  EXT), OP  (sta,  EXT),
	OP  (eora, EXT), OP  (adca, EXT), OP  (ora,  EXT), OP  (adda, EXT),
	OP  (jmp,  EXT), OP  (jsr,  EXT), OP  (ldx,  EXT), OP  (stx,  EXT),
	OP  (suba, IX2), OP  (cmpa, IX2), OP  (sbca, IX2), OP  (cpx,  IX2), // d0
	OP  (anda, IX2), OP  (bit,  IX2), OP  (lda,  IX2), OP  (sta,  IX2),
	OP  (eora, IX2), OP  (adca, IX2), OP  (ora,  IX2), OP  (adda, IX2),
	OP  (jmp,  IX2), OP  (jsr,  IX2), OP  (ldx,  IX2), OP  (stx,  IX2),
	OP  (suba, IX1), OP  (cmpa, IX1), OP  (sbca, IX1), OP  (cpx,  IX1), // e0
	OP  (anda, IX1), OP  (bit,  IX1), OP  (lda,  IX1), OP  (sta,  IX1),
	OP  (eora, IX1), OP  (adca, IX1), OP  (ora,  IX1), OP  (adda, IX1),
	OP  (jmp,  IX1), OP  (jsr,  IX1), OP  (ldx,  IX1), OP  (stx,  IX1),
	OP  (suba, IDX), OP  (cmpa, IDX), OP  (sbca, IDX), OP  (cpx,  IDX), // f0
	OP  (anda, IDX), OP  (bit,  IDX), OP  (lda,  IDX), OP  (sta,  IDX),
	OP  (eora, IDX), OP  (adca, IDX), OP  (ora,  IDX), OP  (adda, IDX),
	OP  (jmp,  IDX), OP  (jsr,  IDX), OP  (ldx,  IDX), OP  (stx,  IDX)
};


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


offs_t disassemble(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		lvl level,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count)
{
	u8 const code = oprom[0];

	if (!disasm[code].name || (disasm[code].level > level))
	{
		util::stream_format(stream, "%-6s$%02X", "fcb", code);
		return 1 | DASMFLAG_SUPPORTED;
	}
	else
	{
		u32 flags;
		switch (disasm[code].op)
		{
		case op_names::bsr:
		case op_names::jsr:
			flags = DASMFLAG_STEP_OVER;
			break;
		case op_names::rts:
		case op_names::rti:
			flags = DASMFLAG_STEP_OUT;
			break;
		default:
			flags = 0;
		}

		util::stream_format(stream, "%-6s", disasm[code].name);

		int bit;
		u16 ea;
		switch (disasm[code].mode)
		{
		case md::INH:   // inherent
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
	return disassemble(device, stream, pc, oprom, opram, options, lvl::HMOS, symbols, symbol_count);
}

offs_t CPU_DISASSEMBLE_NAME(m146805)(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count)
{
	return disassemble(device, stream, pc, oprom, opram, options, lvl::CMOS, symbols, symbol_count);
}

offs_t CPU_DISASSEMBLE_NAME(m68hc05)(
		cpu_device *device,
		std::ostream &stream,
		offs_t pc,
		const u8 *oprom,
		const u8 *opram,
		int options,
		std::pair<u16, char const *> const symbols[],
		std::size_t symbol_count)
{
	return disassemble(device, stream, pc, oprom, opram, options, lvl::HC, symbols, symbol_count);
}

CPU_DISASSEMBLE(m6805)   { return CPU_DISASSEMBLE_NAME(m6805)  (device, stream, pc, oprom, opram, options, nullptr, 0); }
CPU_DISASSEMBLE(m146805) { return CPU_DISASSEMBLE_NAME(m146805)(device, stream, pc, oprom, opram, options, nullptr, 0); }
CPU_DISASSEMBLE(m68hc05) { return CPU_DISASSEMBLE_NAME(m68hc05)(device, stream, pc, oprom, opram, options, nullptr, 0); }
