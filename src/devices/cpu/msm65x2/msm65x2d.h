// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Oki MSM65X2 generic disassembler

#ifndef MAME_CPU_MSM65X2_MSM65X2D_H
#define MAME_CPU_MSM65X2_MSM65X2D_H

#pragma once

class msm65x2_disassembler : public util::disasm_interface
{
public:
	msm65x2_disassembler() = default;
	virtual ~msm65x2_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u8 value;
		u8 mask;
		u32 (*cb)(std::ostream &, const data_buffer &, u8, u16);
	};

	static const instruction instructions[];
};

#endif // MAME_CPU_MSM65X2_MSM65X2D_H
