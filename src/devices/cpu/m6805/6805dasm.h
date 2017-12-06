// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Aaron Giles, Vas Crabb

// 6805 disassembler interface

#ifndef MAME_CPU_M6805_6805DASM_H
#define MAME_CPU_M6805_6805DASM_H

#pragma once

class m6805_base_disassembler : public util::disasm_interface
{
public:
	enum class lvl {
		HMOS,
		CMOS,
		HC
	};

	m6805_base_disassembler(lvl level, std::pair<u16, char const *> const symbols[], std::size_t symbol_count);

	m6805_base_disassembler(lvl level) : m6805_base_disassembler(level, nullptr, 0) {}

	virtual ~m6805_base_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
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

	struct info {
		op_names op;
		char const *name;
		md mode;
		lvl level;
	};

	static const info disasm[0x100];

	lvl m_level;
	std::pair<u16, char const *> const *m_symbols;
	std::size_t m_symbol_count;

	template <typename T> std::string address(T offset) const;
};

class m6805_disassembler : public m6805_base_disassembler
{
public:
	m6805_disassembler();
	m6805_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count);
	template<size_t N> m6805_disassembler(std::pair<u16, char const *> const (&symbols)[N]) : m6805_disassembler(symbols, N) {}

	virtual ~m6805_disassembler() = default;
};

class m146805_disassembler : public m6805_base_disassembler
{
public:
	m146805_disassembler();
	virtual ~m146805_disassembler() = default;
};

class m68hc05_disassembler : public m6805_base_disassembler
{
public:
	m68hc05_disassembler();
	m68hc05_disassembler(std::pair<u16, char const *> const symbols[], std::size_t symbol_count);
	template<size_t N> m68hc05_disassembler(std::pair<u16, char const *> const (&symbols)[N]) : m68hc05_disassembler(symbols, N) {}
	virtual ~m68hc05_disassembler() = default;
};


#endif
