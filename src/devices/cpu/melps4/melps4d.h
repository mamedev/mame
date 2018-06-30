// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family disassembler

  Not counting the extra opcodes for peripherals (eg. timers, A/D),
  each MCU in the series has small differences in the opcode map.

*/

#ifndef MAME_CPU_MELPS4_MELPS4D_H
#define MAME_CPU_MELPS4_MELPS4D_H

#pragma once

class melps4_disassembler : public util::disasm_interface
{
public:
	melps4_disassembler() = default;
	virtual ~melps4_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual u32 interface_flags() const override;
	virtual u32 page_address_bits() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	// opcode mnemonics
	enum e_mnemonics
	{
		em_ILL,
		em_TAB, em_TBA, em_TAY, em_TYA, em_TEAB, em_TABE, em_TEPA, em_TXA, em_TAX,
		em_LXY, em_LZ, em_INY, em_DEY, em_LCPS, em_SADR,
		em_TAM, em_XAM, em_XAMD, em_XAMI,
		em_LA, em_AM, em_AMC, em_AMCS, em_A, em_SC, em_RC, em_SZC, em_CMA, em_RL, em_RR,
		em_SB, em_RB, em_SZB, em_SEAM, em_SEY,
		em_TLA, em_THA, em_TAJ, em_XAL, em_XAH, em_LC7, em_DEC, em_SHL, em_RHL, em_CPA, em_CPAS, em_CPAE, em_SZJ,
		em_T1AB, em_TRAB, em_T2AB, em_TAB1, em_TABR, em_TAB2, em_TVA, em_TWA, em_SNZ1, em_SNZ2,
		em_BA, em_SP, em_B, em_BM, em_RT, em_RTS, em_RTI,
		em_CLD, em_CLS, em_CLDS, em_SD, em_RD, em_SZD, em_OSAB, em_OSPA, em_OSE, em_IAS, em_OFA, em_IAF, em_OGA, em_IAK, em_SZK, em_SU, em_RU,
		em_EI, em_DI, em_INTH, em_INTL, em_NOP
	};

	static const char *const em_name[];
	static const uint8_t em_bits[];
	static const uint32_t em_flags[];
	static const uint8_t m58846_opmap[0xc0];

};

#endif
