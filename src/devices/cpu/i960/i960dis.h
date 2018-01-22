// license:BSD-3-Clause
// copyright-holders:Farfetch'd, R. Belmont
#ifndef MAME_CPU_I960_I960DIS_H
#define MAME_CPU_I960_I960DIS_H

#pragma once

class i960_disassembler : public util::disasm_interface
{
public:
	i960_disassembler() = default;
	virtual ~i960_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct mnemonic_t
	{
		const char      *mnem;
		unsigned short  type;
	};

	static const mnemonic_t mnemonic[256];
	static const mnemonic_t mnem_reg[100];
	static const char *const constnames[32];
	static const char *const regnames[32];

	std::string dis_decode_reg(u32 iCode, unsigned char cnt);

};

#endif
