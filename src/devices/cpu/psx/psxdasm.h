// license:BSD-3-Clause
// copyright-holders:smf
/*
 * PSXCPU disassembler for the MAME project written by smf
 *
 */

#ifndef MAME_CPU_PSX_PSXDASM_H
#define MAME_CPU_PSX_PSXDASM_H

#pragma once

class psxcpu_disassembler : public util::disasm_interface
{
public:
	struct config {
		virtual ~config() = default;

		virtual uint32_t pc() = 0;
		virtual uint32_t delayr() = 0;
		virtual uint32_t delayv() = 0;
		virtual uint32_t r(int i) = 0;
	};

	psxcpu_disassembler(config *conf = nullptr);
	virtual ~psxcpu_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const s_cpugenreg[];
	static const char *const s_cp0genreg[];
	static const char *const s_cp0ctlreg[];
	static const char *const s_cp1genreg[];
	static const char *const s_cp1ctlreg[];
	static const char *const s_cp2genreg[];
	static const char *const s_cp2ctlreg[];
	static const char *const s_cp3genreg[];
	static const char *const s_cp3ctlreg[];
	static const char *const s_gtesf[];
	static const char *const s_gtemx[];
	static const char *const s_gtev[];
	static const char *const s_gtecv[];
	static const char *const s_gtelm[];

	std::string make_signed_hex_str_16( uint32_t value );
	std::string effective_address( uint32_t pc, uint32_t op );
	uint32_t relative_address( uint32_t pc, uint32_t op );
	uint32_t jump_address( uint32_t pc, uint32_t op );
	std::string upper_address( uint32_t op, offs_t pos, const data_buffer &opcodes );

	config *m_config;
};

#endif
