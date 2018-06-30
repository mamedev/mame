// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef MAME_CPU_SUPERFX_SFX_DASM_H
#define MAME_CPU_SUPERFX_SFX_DASM_H

#pragma once

class superfx_disassembler : public util::disasm_interface
{
public:
	enum {
		SUPERFX_SFR_ALT  =   0x0300,  // ALT Mode, both bits
		SUPERFX_SFR_ALT0 =   0x0000,  // ALT Mode, no bits
		SUPERFX_SFR_ALT1 =   0x0100,  // ALT Mode, bit 0
		SUPERFX_SFR_ALT2 =   0x0200,  // ALT Mode, bit 1
		SUPERFX_SFR_ALT3 =   0x0300   // ALT Mode, both bits (convenience dupe)
	};

	struct config {
		virtual ~config() = default;

		virtual u16 get_alt() const = 0;
	};

	superfx_disassembler(config *conf);
	virtual ~superfx_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	config *m_config;
};

#endif
