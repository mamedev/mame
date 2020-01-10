// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT50_VT50DASM_H
#define MAME_CPU_VT50_VT50DASM_H

#pragma once

class vt5x_disassembler : public util::disasm_interface
{
protected:
	vt5x_disassembler(const char *const opcodes_e[8], const char *const opcodes_f[8], const char *const opcodes_g[8], const char *const jumps_h[2][8], const char *const opcodes_w[8]);

	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// tables
	static const char *const s_opcodes_e[8];
	static const char *const s_opcodes_f[8];
	static const char *const s_opcodes_w[8];

private:
	const char *const *m_opcodes_e;
	const char *const *m_opcodes_f;
	const char *const *m_opcodes_g;
	const char *const (*m_jumps_h)[8];
	const char *const *m_opcodes_w;
};

class vt50_disassembler : public vt5x_disassembler
{
public:
	// construction/destruction
	vt50_disassembler();

private:
	// tables
	static const char *const s_opcodes_g[8];
	static const char *const s_jumps_h[2][8];
};

class vt52_disassembler : public vt5x_disassembler
{
public:
	// construction/destruction
	vt52_disassembler();

private:
	// tables
	static const char *const s_opcodes_g[8];
	static const char *const s_jumps_h[2][8];
};

#endif // MAME_CPU_VT50_VT50DASM_H
