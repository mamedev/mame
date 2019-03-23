// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*****************************************************************************
*
*   ns32000dasm.cpp
*
*   NS32000 CPU Disassembly
*
*****************************************************************************/

#ifndef MAME_CPU_NS32000_NS32000DASM_H
#define MAME_CPU_NS32000_NS32000DASM_H

#pragma once

class ns32000_disassembler : public util::disasm_interface
{
public:

	ns32000_disassembler() = default;
	virtual ~ns32000_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	/* implied operand attributes */
	enum
	{
		REG = 16,
		QUICK,
		SHORT,
		IMM,
		DISP,
		GEN,
		OPTIONS,
	};
	/* access classes */
	enum
	{
		READ = 8,
		WRITE,
		RMW,
		ADDR,
		REGADDR
	};
	/* length attributes */
	enum
	{
		B = 0,
		W = 1,
		D = 3,
		I,
		I2,
		F,
		L
	};
	struct NS32000_OPCODE {
		const char *mnemonic;
		u32 operand1;
		u32 operand;
		u32 operand3;
		u32 operand4;
		offs_t dasm_flags;
	};

	static const NS32000_OPCODE format0_op[1];
	static const NS32000_OPCODE format1_op[16];
	static const NS32000_OPCODE format2_op[8];
	static const NS32000_OPCODE format3_op[16];
	static const NS32000_OPCODE format4_op[16];
	static const NS32000_OPCODE format5_op[16];
	static const NS32000_OPCODE format6_op[16];
	static const NS32000_OPCODE format7_op[16];
	static const NS32000_OPCODE format8_op[16];
	static const NS32000_OPCODE format9_op[16];
	static const NS32000_OPCODE format11_op[16];
	static const NS32000_OPCODE format14_op[16];


	static char const *const Format0[];
	static char const *const Format1[];
	static char const *const Format2[];
	static char const *const Format3[];
	static char const *const Format4[];
	static char const *const Format5[];
	static char const *const Format6[];
	static char const *const Format7[];
	static char const *const Format8[];
	static char const *const Format9[];
	static char const *const Format11[];
	static char const *const Format14[];
	static char const *const iType[];
	static char const *const fType[];
	static char const *const cType[];
	static char const *const indexSize[];
	static char const *const cond[];
	static char const *const areg[];
	static char const *const mreg[];
	static char const *const R[];
	static char const *const M[];
	static char const *const PR[];

	std::string mnemonic_index(std::string form, std::string itype, std::string ftype);
	uint8_t opcode_format(uint8_t byte);
	int8_t short2int(uint8_t val);
	static inline int32_t get_disp(offs_t &pc, const data_buffer &opcodes);
	static inline std::string get_option_list(uint8_t cfg);
	static inline std::string get_options(uint8_t opts);
	static inline std::string get_reg_list(offs_t &pc, const data_buffer &opcodes, bool reverse);

	void stream_gen(std::ostream &stream, u8 gen_addr, u8 op_len, offs_t &pc, const data_buffer &opcodes);

	u32 m_base_pc;
};

#endif
