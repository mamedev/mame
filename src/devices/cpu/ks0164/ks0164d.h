// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 disassembler

#ifndef MAME_CPU_KS0164_KS0164DASM_H
#define MAME_CPU_KS0164_KS0164DASM_H

#pragma once

class ks0164_disassembler : public util::disasm_interface
{
public:
	ks0164_disassembler() = default;
	virtual ~ks0164_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct instruction {
		u16 value;
		u16 mask;
		std::function<u32 (std::ostream &, u32, const data_buffer &, u32)> cb;
	};

	static const instruction instructions[];
	static const char *const regs[8];

	static s32 off10(u32 opcode);
	static std::string off16(s16 dt);
};

#endif

