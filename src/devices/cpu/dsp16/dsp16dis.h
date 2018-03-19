// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    WE|AT&T DSP16 series disassembler

***************************************************************************/
#ifndef MAME_CPU_DSP16_DSP16DIS_H
#define MAME_CPU_DSP16_DSP16DIS_H

#pragma once

#include <iosfwd>
#include <string>


class dsp16_disassembler : public util::disasm_interface
{
public:
	// interface to live CPU
	class cpu
	{
	public:
		enum class predicate : u8 { INDETERMINATE, TAKEN, SKIPPED };
		virtual predicate check_con(offs_t pc, u16 op) const = 0;
		virtual predicate check_branch(offs_t pc) const = 0;
	protected:
		virtual ~cpu() = default;
	};

	// construction/destruction
	dsp16_disassembler();
	dsp16_disassembler(cpu const &host);
	virtual ~dsp16_disassembler() = default;

	// util::disasm_interface implementation
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

private:
	struct result { bool nop; bool ambiguous; std::string text; };

	// sub-instruction helpers
	static result dasm_int(u16 op);
	static result dasm_con(u16 op);
	static result dasm_b(u16 op, u32 &flags);
	static result dasm_ja(u16 op, offs_t pc, u32 &flags);
	static result dasm_f1(u16 op);
	static result dasm_f2(u16 op);
	static result dasm_r(u16 op);
	static result dasm_x(u16 op);
	static result dasm_y(u16 op);
	static result dasm_z(u16 op);

	// common maths
	static constexpr offs_t inc_pc(offs_t pc, offs_t inc);

	// live state checks
	char const *check_branch_predicate(offs_t pc) const;
	char const *check_branch_con(offs_t pc, u16 op) const;
	char const *check_special_con(offs_t pc, u16 op) const;

	// access to live CPU
	cpu const *const m_host;
};

#endif // MAME_CPU_DSP16_DSP16DIS_H
