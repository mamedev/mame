// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 *   lr35902d.c
 *   Portable Sharp LR35902 disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_LR35902_LR35902DASM_H
#define MAME_CPU_LR35902_LR35902DASM_H

#pragma once

class lr35902_disassembler : public util::disasm_interface
{
public:
	lr35902_disassembler() = default;
	virtual ~lr35902_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	enum e_mnemonics
	{
		zADC,  zADD,  zAND,  zBIT,  zCALL, zCCF,  zCP,
		zCPL,  zDAA,  zDB,   zDEC,  zDI,   zEI,   zHLT,
		zIN,   zINC,  zJP,   zJR,   zLD,   zNOP,  zOR,
		zPOP,  zPUSH, zRES,  zRET,  zRETI, zRL,   zRLA,
		zRLC,  zRLCA, zRR,   zRRA,  zRRC,  zRRCA, zRST,
		zSBC,  zSCF,  zSET,  zSLA,  zSLL,  zSRA,  zSRL,
		zSTOP, zSUB,  zXOR,  zSWAP
	};

	struct lr35902dasm
	{
		uint8_t   mnemonic;
		const char *arguments;
	};

	static const char *const s_mnemonic[];
	static const uint32_t s_flags[];
	static const lr35902dasm mnemonic_cb[256];
	static const lr35902dasm mnemonic_main[256];
};

#endif
