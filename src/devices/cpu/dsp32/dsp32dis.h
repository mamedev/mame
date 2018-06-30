// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dsp32dis.c
    Disassembler for the portable AT&T/Lucent DSP32C emulator.
    Written by Aaron Giles

***************************************************************************/

#ifndef MAME_CPU_DSP32_DSP32DIS_H
#define MAME_CPU_DSP32_DSP32DIS_H

#pragma once

class dsp32c_disassembler : public util::disasm_interface
{
public:
	dsp32c_disassembler() = default;
	virtual ~dsp32c_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const sizesuffix[];
	static const char *const unarysign[];
	static const char *const sign[];
	static const char *const aMvals[];
	static const char *const memsuffix[];
	static const char *const functable[];
	static const char *const condtable[];
	static const char *const regname[];
	static const char *const regnamee[];
	std::string signed_16bit_unary(int16_t val);
	std::string signed_16bit_sep(int16_t val);
	std::string signed_16bit_sep_nospace(int16_t val);
	std::string unsigned_16bit_size(int16_t val, uint8_t size);
	std::string dasm_XYZ(uint8_t bits);
	std::string dasm_PI(uint16_t bits);

	uint8_t lastp;
};

#endif
