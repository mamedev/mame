// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_CLIPPER_CLIPPERDASM_H
#define MAME_CPU_CLIPPER_CLIPPERDASM_H

#pragma once

class clipper_disassembler : public util::disasm_interface
{
public:
	clipper_disassembler() = default;
	virtual ~clipper_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// the CLIPPER addressing modes (unshifted)
	enum
	{
		ADDR_MODE_PC32 =  0x10,
		ADDR_MODE_ABS32 = 0x30,
		ADDR_MODE_REL32 = 0x60,
		ADDR_MODE_PC16 =  0x90,
		ADDR_MODE_REL12 = 0xa0,
		ADDR_MODE_ABS16 = 0xb0,
		ADDR_MODE_PCX =   0xd0,
		ADDR_MODE_RELX =  0xe0
	};

	static const char *const cc[];
	std::string address (offs_t pc, const data_buffer &opcodes);


};

#endif
