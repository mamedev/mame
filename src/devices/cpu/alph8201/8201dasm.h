// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
/*

Notice: The alpha 8201 is now emulated using mame/alpha8201.*

cpu/alph8201/ will be removed when the alpha 8304 has been dumped.




*/

/****************************************************************************
                         Alpha 8201/8301 Disassembler

                      Copyright Tatsuyuki Satoh
                   Originally written for the MAME project.

****************************************************************************/

#ifndef MAME_CPU_ALPH8201_8201DASM_H
#define MAME_CPU_ALPH8201_8201DASM_H

#pragma once

class alpha8201_disassembler : public util::disasm_interface
{
public:
	alpha8201_disassembler();
	virtual ~alpha8201_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct AD8201Opcode {
		u8 mask;
		u8 bits;
		u8 type;
		u8 pmask;
		u8 pdown;
		const char *fmt;
	};

	static const char *const Formats[];
	AD8201Opcode Op[256];
	int op_count;
};

#endif
