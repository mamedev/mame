// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    STmicro ST6-series microcontroller disassembler

**********************************************************************/

#ifndef MAME_CPU_ST62XX_DASM_H
#define MAME_CPU_ST62XX_DASM_H

#pragma once

class st62xx_disassembler : public util::disasm_interface
{
public:
	st62xx_disassembler() = default;
	virtual ~st62xx_disassembler() = default;

	virtual uint32_t opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	std::string reg_name(const uint8_t reg);
};

#endif // MAME_CPU_ST62XX_DASM_H
