// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Intel 8089 I/O Processor

    Disassembler

***************************************************************************/

#ifndef MAME_CPU_I8089_I8089_DASM_H
#define MAME_CPU_I8089_I8089_DASM_H

#pragma once

class i8089_disassembler : public util::disasm_interface
{
public:
	i8089_disassembler() = default;
	virtual ~i8089_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// register index
	enum
	{
		GA,  // 20-bit general purpose address a
		GB,  // 20-bit general purpose address b
		GC,  // 20-bit general purpose address c
		BC,  // byte count
		TP,  // 20-bit task pointer
		IX,  // index
		CC,  // mask compare
		MC   // channel control
	};

	static const char *const m_reg[];

	uint8_t m_brp, m_wb, m_aa, m_w, m_opc, m_mm;
	offs_t m_pc, m_flags;

	const data_buffer *m_opcodes;

	uint8_t fetch_value8();
	uint16_t fetch_value16();
	uint16_t fetch_immediate();
	std::string offset();
	std::string invalid();

	std::string from_i(const std::string &instr8, const std::string &instr16, const std::string &target);
	std::string inst_ri(const std::string &instr8, const std::string &instr16);
	std::string inst_r(const std::string &instr);
	std::string inst_jr(const std::string &instr8, const std::string &instr16);
	std::string inst_mi(const std::string &instr8, const std::string &instr16);
	std::string inst_rm(const std::string &instr8, const std::string &instr16);
	std::string inst_jm(const std::string &jump8short, const std::string &jump8long);
	std::string inst_jmb(const std::string &jump8short, const std::string &jump8long);
	std::string inst_mr(const std::string &instr8, const std::string &instr16);
	std::string inst_pm(const std::string &instr16);
	std::string inst_mp(const std::string &instr16);
	std::string inst_j16(const std::string &jump8short, const std::string &jump16short, const std::string &jump8long, const std::string &jump16long);
	std::string inst_m(const std::string &instr8, const std::string &instr16);
	std::string inst_b(const std::string &instr);

	std::string do_disassemble();
	void load_instruction();
};

#endif // MAME_CPU_I8089_I8089_DASM_H
