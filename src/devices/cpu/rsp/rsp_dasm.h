// license:BSD-3-Clause
// copyright-holders:Ville Linde, Ryan Holtz
/*
    Nintendo/SGI RSP Disassembler

    Written by Ville Linde
*/

#ifndef MAME_CPU_RSP_RSP_DASM_H
#define MAME_CPU_RSP_RSP_DASM_H

#pragma once

class rsp_disassembler : public util::disasm_interface
{
public:
	rsp_disassembler() = default;
	virtual ~rsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	offs_t dasm_one(std::ostream &stream, offs_t pc, u32 op);

private:
	static const char *const reg[32];
	static const char *const vreg[32];
	static const char *const cop0_regs[32];
	static const char *const element[16];
	static const char *const element2[16];
	inline std::string signed_imm16(uint32_t op);
	void disasm_cop0(std::ostream &stream, uint32_t op);
	void disasm_cop2(std::ostream &stream, uint32_t op);
	void disasm_lwc2(std::ostream &stream, uint32_t op);
	void disasm_swc2(std::ostream &stream, uint32_t op);
};

#endif
