// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_HP2100_HP2100D_H
#define MAME_CPU_HP2100_HP2100D_H

#pragma once

class hp2100_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	hp2100_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	static char cab(u16 inst) { return char('A' + BIT(inst, 11)); }

	void format_address(std::ostream &stream, u16 addr) const;
	virtual offs_t dasm_mac(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const;

private:
	// internal helpers
	void format_sc(std::ostream &stream, u8 sc) const;
	offs_t dasm_mrg(std::ostream &stream, u16 inst, offs_t pc) const;
	offs_t dasm_asg(std::ostream &stream, u16 inst) const;
	offs_t dasm_srg(std::ostream &stream, u16 inst) const;
	offs_t dasm_iog(std::ostream &stream, u16 inst) const;

	// internal tables
	static const char *const s_shift_ops[2][8];
};

class hp21mx_disassembler : public hp2100_disassembler
{
public:
	// construction/destruction
	using hp2100_disassembler::hp2100_disassembler;

protected:
	virtual offs_t dasm_mac(std::ostream &stream, u16 inst, offs_t pc, const data_buffer &opcodes) const override;

private:
	static char xy(u16 inst) { return char('X' + BIT(inst, 3)); }
};

#endif // MAME_CPU_HP2100_HP2100D_H
