// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT61_VT61DASM_H
#define MAME_CPU_VT61_VT61DASM_H

#pragma once

class vt61_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	vt61_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// tables
	static const char *const s_opr_a[8];
	static const char *const s_opr_b[16];
	static const char *const s_sources[12];
	static const char *const s_conditions[32][2];

	// disassembly helpers
	void dasm_spr(std::ostream &stream, u8 r);
	void dasm_source(std::ostream &stream, u16 inst);
};

#endif // MAME_CPU_VT61_VT61DASM_H
