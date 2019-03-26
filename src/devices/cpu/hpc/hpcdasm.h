// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_HPC_HPCDASM_H
#define MAME_CPU_HPC_HPCDASM_H

#pragma once

class hpc_disassembler : public util::disasm_interface
{
protected:
	enum : u16
	{
		REGISTER_A = 0x00c8,
		REGISTER_K = 0x00ca,
		REGISTER_B = 0x00cc,
		REGISTER_X = 0x00ce
	};

	// construction/destruction
	hpc_disassembler(const char *const regs[]);

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	void format_register(std::ostream &stream, u16 reg) const;
	void format_immediate_byte(std::ostream &stream, u8 data) const;
	void format_immediate_word(std::ostream &stream, u16 data) const;
	void disassemble_op(std::ostream &stream, const char *op, u16 reg, u16 src, bool imm, bool indir, bool idx, bool w) const;
	void disassemble_unary_op(std::ostream &stream, const char *op, u16 offset, u16 src, bool indir, bool idx, bool w) const;
	void disassemble_bit_op(std::ostream &stream, const char *op, u8 bit, u16 offset, u16 src, bool indir, bool idx) const;

	// register names
	const char *const *m_regs;
};

class hpc16083_disassembler : public hpc_disassembler
{
public:
	hpc16083_disassembler() : hpc_disassembler(s_regs) { }

private:
	static const char *const s_regs[128];
};

class hpc16164_disassembler : public hpc_disassembler
{
public:
	hpc16164_disassembler() : hpc_disassembler(s_regs) { }

private:
	static const char *const s_regs[128];
};

#endif // MAME_CPU_HPC_HPCDASM_H
