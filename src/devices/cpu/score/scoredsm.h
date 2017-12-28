// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

    Sunplus Technology S+core disassembler

******************************************************************************/

#ifndef MAME_CPU_SCORE_SCOREDSM_H
#define MAME_CPU_SCORE_SCOREDSM_H

#pragma once

class score7_disassembler : public util::disasm_interface
{
public:
	score7_disassembler() = default;
	virtual ~score7_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// mnemonics
	static const char *const m_cond[16];
	static const char *const m_tcs[4];
	static const char *const m_rix1_op[8];
	static const char *const m_rix2_op[8];
	static const char *const m_r2_op[16];
	static const char *const m_i1_op[8];
	static const char *const m_i2_op[8];
	static const char *const m_ls_op[8];
	static const char *const m_i1a_op[8];
	static const char *const m_i1b_op[8];
	static const char *const m_cr_op[2];

	int32_t sign_extend(uint32_t data, uint8_t len);
	offs_t disasm(std::ostream &stream, offs_t pc, uint32_t opcode);
	void disasm32(std::ostream &stream, offs_t pc, uint32_t opcode);
	void disasm16(std::ostream &stream, offs_t pc, uint16_t opcode);
};

#endif
