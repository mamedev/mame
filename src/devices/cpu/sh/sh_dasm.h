// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, R. Belmont

#ifndef MAME_CPU_SH_SHDASM_H
#define MAME_CPU_SH_SHDASM_H

#pragma once

class sh_disassembler : public util::disasm_interface
{
public:
	sh_disassembler(bool is_sh34);
	virtual ~sh_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t dasm_one(std::ostream &stream, offs_t pc, u16 opcode);

private:
	static const char *const regname[16];
	uint32_t op0000(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0001(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0010(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0011(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0100(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0101(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0110(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0111(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1000(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1001(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1010(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1011(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1100(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1101(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1110(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1111(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0000_sh34(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op0100_sh34(std::ostream &stream, uint32_t pc, uint16_t opcode);
	uint32_t op1111_sh34(std::ostream &stream, uint32_t pc, uint16_t opcode);

	bool m_is_sh34;
};

#endif
