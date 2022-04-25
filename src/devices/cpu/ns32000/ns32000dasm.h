// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NS32000_NS32000DASM_H
#define MAME_CPU_NS32000_NS32000DASM_H

#pragma once

class ns32000_disassembler : public util::disasm_interface
{
public:
	ns32000_disassembler() = default;
	virtual ~ns32000_disassembler() = default;

	virtual u32 opcode_alignment() const override { return 1; }
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, data_buffer const &opcodes, data_buffer const &params) override;

protected:
	enum size_code : unsigned
	{
		SIZE_B = 0,
		SIZE_W = 1,
		SIZE_D = 3,
		SIZE_Q = 7,
	};

	struct addr_mode
	{
		addr_mode(unsigned gen)
			: gen(gen)
			, fpu(false)
			, mode()
		{};

		void size_i(size_code code)  { size = code; }
		void size_f(size_code code)  { size = code; fpu = true; }

		unsigned gen;
		size_code size;
		bool fpu;

		std::string mode;
	};

	s32 displacement(offs_t pc, data_buffer const &opcodes, unsigned &bytes);
	std::string displacement_string(offs_t pc, data_buffer const &opcodes, unsigned &bytes, std::string const zero = "");
	void decode(addr_mode *mode, offs_t pc, data_buffer const &opcodes, unsigned &bytes);
	std::string reglist(u8 imm);
	std::string config(u8 imm);
};

#endif // MAME_CPU_NS32000_NS32000DASM_H
