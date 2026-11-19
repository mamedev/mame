// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    VideoLogic PMX1 primary core disassembler
*/

#ifndef MAME_CPU_PMX1_PMX1DASM_H
#define MAME_CPU_PMX1_PMX1DASM_H

#pragma once

#include <string>


class pmx1core_disassembler : public util::disasm_interface
{
public:
	// what has to be known of the thread whose code is shown
	class config
	{
	public:
		// true for thread 0, which counts the addresses of its jumps from four instructions further on
		virtual bool jumps_delayed() const = 0;

	protected:
		~config() = default;
	};

	pmx1core_disassembler(const config *conf = nullptr);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static u32 operand16(u32 op);
	static std::string reg(u32 unit, u32 number);
	static std::string target(u32 unit, u32 number);
	static std::string pointer(u32 selector);
	static std::string mnemonic(const char *name, u32 condition);

	static bool arithmetic(std::ostream &stream, u32 op);
	static bool compare(std::ostream &stream, u32 op);
	static bool memory(std::ostream &stream, u32 op);
	static bool port(std::ostream &stream, u32 op);
	bool move(std::ostream &stream, u32 op, u32 &flags) const;
	bool branch(std::ostream &stream, offs_t pc, u32 op, u32 &flags) const;

	const config *const m_config;
};

#endif // MAME_CPU_PMX1_PMX1DASM_H
