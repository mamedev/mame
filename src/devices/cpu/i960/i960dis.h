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
		signed char     flags;
	};

	static const mnemonic_t mnemonic[256];
	static const mnemonic_t mnem_reg[197];
	static const char *const regnames[32];
	static const char *const fprnames[32];

	offs_t dis_decode_invalid(std::ostream &stream, u32 iCode);
	offs_t dis_decode_ctrl(std::ostream &stream, u32 iCode, u32 ip, signed char cnt);
	offs_t dis_decode_cobr(std::ostream &stream, u32 iCode, u32 ip, signed char cnt);
	offs_t dis_decode_mema(std::ostream &stream, u32 iCode, signed char cnt);
	offs_t dis_decode_memb(std::ostream &stream, u32 iCode, u32 ip, u32 disp, signed char cnt);
	offs_t dis_decode_reg(std::ostream &stream, u32 iCode);

};

#endif
