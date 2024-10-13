// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable Z8x180 disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_Z180_Z180DASM_H
#define MAME_CPU_Z180_Z180DASM_H

#pragma once

class z180_disassembler : public util::disasm_interface
{
public:
	z180_disassembler() = default;
	virtual ~z180_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	struct z80dasm {
		uint8_t mnemonic;
		const char *arguments;
	};

	enum e_mnemonics {
		zADC   ,zADD   ,zAND   ,zBIT   ,zCALL  ,zCCF   ,zCP    ,zCPD   ,
		zCPDR  ,zCPI   ,zCPIR  ,zCPL   ,zDAA   ,zDB    ,zDEC   ,zDI    ,
		zDJNZ  ,zEI    ,zEX    ,zEXX   ,zHLT   ,zIM    ,zIN    ,zIN0   ,
		zINC   ,zIND   ,zINDR  ,zINI   ,zINIR  ,zJP    ,zJR    ,zLD    ,
		zLDD   ,zLDDR  ,zLDI   ,zLDIR  ,zMLT   ,zNEG   ,zNOP   ,zOR    ,
		zOTDM  ,zOTDMR ,zOTDR  ,zOTIM  ,zOTIMR ,zOTIR  ,zOUT   ,zOUT0  ,
		zOUTD  ,zOUTI  ,zPOP   ,zPUSH  ,zRES   ,zRET   ,zRETI  ,zRETN  ,
		zRL    ,zRLA   ,zRLC   ,zRLCA  ,zRLD   ,zRR    ,zRRA   ,zRRC   ,
		zRRCA  ,zRRD   ,zRST   ,zSBC   ,zSCF   ,zSET   ,zSLA   ,zSLL   ,
		zSLP   ,zSRA   ,zSRL   ,zSUB   ,zTST   ,zTSTIO ,zXOR
	};

	static const char *const s_mnemonic[];
	static const z80dasm mnemonic_xx_cb[256];
	static const z80dasm mnemonic_cb[256];
	static const z80dasm mnemonic_ed[256];
	static const z80dasm mnemonic_xx[256];
	static const z80dasm mnemonic_main[256];

	static char sign(int8_t offset);
	static int offs(int8_t offset);
};

#endif
