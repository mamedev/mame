// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
// ********************************************************************************
// * MIT CADR processor microcode disassembler
// ********************************************************************************

#ifndef MAME_CPU_CADR_CADR_DASM_H
#define MAME_CPU_CADR_CADR_DASM_H

#pragma once

class cadr_disassembler : public util::disasm_interface
{
public:
	cadr_disassembler() = default;
	virtual ~cadr_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	static constexpr u64 NOP_MASK = u64(0x7fffffffeffff); // With POPJ bit
//	static constexpr u64 NOP_MASK = u64(0x7fbfffffeffff); // Excluding POPJ bit

private:
	static const char *const bool_op[0x10];
	static const char *const arith_op[0x10];
	static const char *const arith_op_c[0x10];
	static const char *const mult_div_op[0x20];
	static const char *const output_bus_control[0x04];
	static const char *const q_control[0x04];
	static const char *const rp[0x04];

	void a_source(std::ostream &stream, u64 op);
	void m_source(std::ostream &stream, u64 op);
	void disassemble_alu_op(std::ostream &stream, u64 op);
	void disassemble_destination(std::ostream &stream, u64 op);
	void disassemble_condition(std::ostream &stream, u64 op);
};

#endif // MAME_CPU_CADR_CADR_DASM_H
