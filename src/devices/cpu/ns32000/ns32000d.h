// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_NS32000_NS32000D_H
#define MAME_CPU_NS32000_NS32000D_H

#pragma once

class ns32000_disassembler : public util::disasm_interface
{
public:
	// CPU model.  ns32032 selects the external-MMU (NS32082) Format-14 register
	// names used by the NS32008/016/032/332; ns32532 selects the NS32532's
	// on-chip MMU register set and enables its CINV instruction.
	enum class model { ns32032, ns32532 };

	ns32000_disassembler(model variant = model::ns32032);
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

private:
	model const m_model;
};

#endif // MAME_CPU_NS32000_NS32000D_H
