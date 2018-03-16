// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
#ifndef MAME_CPU_DSP16_DSP16DIS_H
#define MAME_CPU_DSP16_DSP16DIS_H

#pragma once

class dsp16a_disassembler : public util::disasm_interface
{
public:
	dsp16a_disassembler() = default;
	virtual ~dsp16a_disassembler() = default;

	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::string disasmF1Field(u8 F1, u8 D, u8 S);
	std::string disasmYField(u8 Y);
	std::string disasmZField(u8 Z);
	std::string disasmF2Field(u8 F2, u8 D, u8 S);
	std::string disasmCONField(u8 CON);
	std::string disasmBField(u8 B, uint32_t &dasmflags);
	std::string disasmRImmediateField(u8 R);
	std::string disasmRField(u8 R);
	std::string disasmIField(u8 I);
	bool disasmSIField(u8 SI);
};

#endif // MAME_CPU_DSP16_DSP16DIS_H
