// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sanyo LC6500 series disassembler

***************************************************************************/

#ifndef MAME_CPU_LC6500_LC6500_DASM_H
#define MAME_CPU_LC6500_LC6500_DASM_H

#pragma once

class lc6500_disassembler : public util::disasm_interface
{
public:
	lc6500_disassembler() = default;
	virtual ~lc6500_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		uint8_t value;
		uint8_t mask;
		uint32_t (*cb)(std::ostream &, uint32_t, const data_buffer &, uint32_t);
	};
	static const instruction instructions[];
};

#endif // MAME_CPU_LC6500_LC6500_DASM_H
