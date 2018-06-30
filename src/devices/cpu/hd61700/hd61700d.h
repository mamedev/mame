// license:BSD-3-Clause
// copyright-holders:Sandro Ronco


#ifndef MAME_CPU_HD61700_HD61700D_H
#define MAME_CPU_HD61700_HD61700D_H

#pragma once

class hd61700_disassembler : public util::disasm_interface
{
public:
	hd61700_disassembler() = default;
	virtual ~hd61700_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct dasm
	{
		const char *str;
		uint8_t       arg1;
		uint8_t       arg2;
		bool        optjr;
	};

	enum
	{
		OP_NULL=0,
		OP_IM16,
		OP_IM16A,
		OP_IM3,
		OP_IM5,
		OP_IM7,
		OP_IM8,
		OP_IM8I,
		OP_IM8_,
		OP_IR_IM3,
		OP_IR_IM8,
		OP_IR_IM8_,
		OP_JX_COND,
		OP_MREG,
		OP_MREG2,
		OP_MR_SIR,
		OP_MR_SIRI,
		OP_REG16,
		OP_REG16_,
		OP_REG8,
		OP_REG8_,
		OP_REGIM8,
		OP_RMSIM3,
		OP_RSIR
	};

	static const char *const reg_5b[4];
	static const char *const reg_8b[8];
	static const char *const reg_16b[8];
	static const char *const jp_cond[8];

	static const dasm ops[256];

	u8 opread(offs_t pc, offs_t pos, const data_buffer &opcodes);
	void dasm_im8(std::ostream &stream, offs_t pc, int arg, offs_t &pos, const data_buffer &opcodes);
	void dasm_im8(std::ostream &stream, offs_t pc, int arg, int arg1, offs_t &pos, const data_buffer &opcodes);
	void dasm_arg(std::ostream &stream, uint8_t op, offs_t pc, int arg, offs_t &pos, const data_buffer &opcodes);
	uint32_t get_dasmflags(uint8_t op);
};

#endif
