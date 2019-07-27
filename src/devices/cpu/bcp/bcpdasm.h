// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor DP8344 BCP disassembler

***************************************************************************/

#ifndef MAME_CPU_BCP_BCPDASM_H
#define MAME_CPU_BCP_BCPDASM_H 1

#pragma once

class dp8344_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	dp8344_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	const char *cc_to_string(u8 f, bool s) const;
	const char *aop_to_string(u8 o) const;
	void format_number(std::ostream &stream, u8 n) const;
	void format_address(std::ostream &stream, u16 nn) const;
	void format_register(std::ostream &stream, u8 r) const;
	void format_modified_index_register(std::ostream &stream, u8 ir, u8 m) const;
	void format_interrupt_vector(std::ostream &stream, u8 v) const;
};

#endif // MAME_CPU_BCP_BCPDASM_H
