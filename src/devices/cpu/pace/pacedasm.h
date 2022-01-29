// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor PACE (IPC-16, INS8900) disassembler

***************************************************************************/

#ifndef MAME_CPU_PACE_PACEDASM_H
#define MAME_CPU_PACE_PACEDASM_H

#pragma once

class pace_disassembler : public util::disasm_interface
{
public:
	// construction/destruction
	pace_disassembler();

protected:
	// disassembler overrides
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// internal helpers
	void format_addr(std::ostream &stream, u16 addr);
	void format_disp(std::ostream &stream, u8 disp);
	void format_ea(std::ostream &stream, u16 pc, u16 inst);

	// tables
	static const char *const s_cc[16];
	static const char *const s_flags[16];
};

#endif // MAME_CPU_PACE_PACEDASM_H
