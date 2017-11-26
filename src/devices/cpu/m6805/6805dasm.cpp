// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
/*
 *   A quick-hack 68(7)05 disassembler
 *
 *   Note: this is not the good and proper way to disassemble anything, but it works
 *
 *   Secondary note: it actually stopped being not nice a while ago
 *
 *   I'm afraid to put my name on it, but I feel obligated:
 *   This code written by Aaron Giles (agiles@sirius.com) for the MAME project
 *
 */

#include "emu.h"
#include "6805dasm.h"

#define OP(name, mode)   { op_names::name, #name,   md::mode, lvl::HMOS }
#define OPC(name, mode)  { op_names::name, #name,   md::mode, lvl::CMOS }
#define OPHC(name, mode) { op_names::name, #name,   md::mode, lvl::HC   }
#define ILLEGAL          { op_names::ill,  nullptr, md::INH,  lvl::HMOS }
const m6805_base_disassembler::info m6805_base_disassembler::disasm[0x100] = {
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

m6805_base_disassembler::m6805_base_disassembler(lvl level, std::pair<u16, char const *> const symbols[], std::size_t symbol_count) : m_level(level), m_symbols(symbols), m_symbol_count(symbol_count)
{
}

u32 m6805_base_disassembler::opcode_alignment() const
{
	return 1;
}

template <typename T> std::string m6805_base_disassembler::address(T offset) const
{
	auto const symbol = std::lower_bound(m_symbols,
										 m_symbols + m_symbol_count,
										 offset,
										 [] (auto const &sym, u16 addr) { return sym.first < addr; });
	if ((m_symbol_count != (symbol - m_symbols)) && (symbol->first == offset))
		return symbol->second;
	else
		return util::string_format("$%0*X", 2 * sizeof(T), offset);
}

offs_t m6805_base_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	u8 const code = opcodes.r8(pc);

	if (!disasm[code].name || (disasm[code].level > m_level))
	{
		util::stream_format(stream, "%-6s$%02X", "fcb", code);
		return 1 | SUPPORTED;
	}
	else
	{
		u32 flags;
		switch (disasm[code].op)
		{
		case op_names::bsr:
		case op_names::jsr:
			flags = STEP_OVER;
			break;
		case op_names::rts:
		case op_names::rti:
			flags = STEP_OUT;
			break;
		default:
			flags = 0;
		}

		util::stream_format(stream, "%-6s", disasm[code].name);

		switch (disasm[code].mode)
		{
		case md::INH:   // inherent
			return 1 | flags | SUPPORTED;

		case md::BTR:   // bit test and relative branch
			util::stream_format(stream, "%d, %s, $%03X", (code >> 1) & 7, address(params.r8(pc+1)), pc + 3 + s8(params.r8(pc+2)));
			return 3 | flags | SUPPORTED;

		case md::BIT:   // bit test
			util::stream_format(stream, "%d, %s", (code >> 1) & 7, address(params.r8(pc+1)));
			return 2 | flags | SUPPORTED;

		case md::REL:   // relative
			util::stream_format(stream, "$%03X", pc + 2 + s8(params.r8(pc+1)));
			return 2 | flags | SUPPORTED;

		case md::IMM:   // immediate
			util::stream_format(stream, "#$%02X", params.r8(pc+1));
			return 2 | flags | SUPPORTED;

		case md::DIR:   // direct (zero page address)
			util::stream_format(stream, "%s", address(params.r8(pc+1)));
			return 2 | flags | SUPPORTED;

		case md::EXT:   // extended (16 bit address)
			util::stream_format(stream, "%s", address(params.r16(pc+1)));
			return 3 | flags | SUPPORTED;

		case md::IDX:   // indexed
			util::stream_format(stream, "(x)");
			return 1 | flags | SUPPORTED;

		case md::IX1:   // indexed + byte (zero page)
			util::stream_format(stream, "(x+$%02X)", params.r8(pc+1));
			return 2 | flags | SUPPORTED;

		case md::IX2:   // indexed + word (16 bit address)
			util::stream_format(stream, "(x+$%04X)", params.r16(pc+1));
			return 3 | flags | SUPPORTED;
		}

		// if we fall off the switch statement something is very wrong
		throw false;
	}
}

m6805_disassembler::m6805_disassembler() : m6805_base_disassembler(lvl::HMOS)
{
}

m6805_disassembler::m6805_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count) : m6805_base_disassembler(lvl::HMOS, symbols, symbol_count)
{
}

m146805_disassembler::m146805_disassembler() : m6805_base_disassembler(lvl::CMOS)
{
}

m68hc05_disassembler::m68hc05_disassembler() : m6805_base_disassembler(lvl::HC)
{
}

m68hc05_disassembler::m68hc05_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count) : m6805_base_disassembler(lvl::HC, symbols, symbol_count)
{
}
