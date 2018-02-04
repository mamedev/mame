// license:BSD-3-Clause
// copyright-holders:Andrew Gardner

#ifndef MAME_CPU_DSP16_DSP16DIS_H
#define MAME_CPU_DSP16_DSP16DIS_H

#pragma once

class dsp16a_disassembler : public util::disasm_interface
{
public:
	dsp16a_disassembler() = default;
	virtual ~dsp16a_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	std::string disasmF1Field(const uint8_t& F1, const uint8_t& D, const uint8_t& S);
	std::string disasmYField(const uint8_t& Y);
	std::string disasmZField(const uint8_t& Z);
	std::string disasmF2Field(const uint8_t& F2, const uint8_t& D, const uint8_t& S);
	std::string disasmCONField(const uint8_t& CON);
	std::string disasmBField(const uint8_t& B);
	std::string disasmRImmediateField(const uint8_t& R);
	std::string disasmRField(const uint8_t& R);
	std::string disasmIField(const uint8_t& I);
	bool disasmSIField(const uint8_t& SI);

};

#endif
