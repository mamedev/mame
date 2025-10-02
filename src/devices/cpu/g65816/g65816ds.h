// license:BSD-3-Clause
// copyright-holders:Karl Stenerud
#ifndef MAME_CPU_G65816_G65816DS_H
#define MAME_CPU_G65816_G65816DS_H

#pragma once

/* ======================================================================== */
/* =============================== COPYRIGHT ============================== */
/* ======================================================================== */
/*

G65C816 CPU Emulator V0.92

Copyright Karl Stenerud

*/

#ifdef __sun
#undef SEC
#undef op
#endif

class g65816_disassembler : public util::disasm_interface
{
public:
	class config {
	protected:
		~config() = default;
	public:
		virtual bool get_m_flag() const = 0;
		virtual bool get_x_flag() const = 0;
	};

	g65816_disassembler(const config *conf);

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum class op : unsigned;

	enum
	{
		IMP , ACC , RELB, RELW, IMM , A   , AI  , AL  , ALX , AX  , AXI ,
		AY  , D   , DI  , DIY , DLI , DLIY, DX  , DXI , DY  , S   , SIY ,
		SIG , MVN , MVP , PEA , PEI , PER
	};

	enum
	{
		I, /* ignore */
		M, /* check m bit */
		X  /* check x bit */
	};

	class opcode_struct;

	static const char *const s_opnames[];
	static const opcode_struct s_opcodes[256];

	const config *m_config;

	std::string int_8_str(u8 val);
	std::string int_16_str(u16 val);
};

#endif // MAME_CPU_G65816_G65816DS_H
