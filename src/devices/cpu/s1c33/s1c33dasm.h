// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_S1C33_S1C33D_H
#define MAME_CPU_S1C33_S1C33D_H

#pragma once

class s1c33_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	s1c33_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;

	offs_t handle_000x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_0000_000x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_0000_001x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_0000_010x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_0000_011x_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_001x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_001x_xx00_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_001x_xx01_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_001x_xx10_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_001x_xx11_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_010x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_011x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_1000_00xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_1000_01xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100y_yyxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100y_00xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100y_01xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100y_10xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_100y_11xx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_101x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_101x_xx00_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_101x_xx01_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_101x_xx10_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_101x_xx11_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_110x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);
	offs_t handle_111x_xxxx_xxxx_xxxx(std::ostream &stream, offs_t pc, const s1c33_disassembler::data_buffer &opcodes, const s1c33_disassembler::data_buffer &params, u16 op);

	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;
};

#endif // MAME_CPU_S1C33_S1C33D_H
