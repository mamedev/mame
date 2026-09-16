// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   z80dasm.h
 *   Portable Z80 disassembler
 *
 *****************************************************************************/

#ifndef MAME_CPU_Z80_Z80DASM_H
#define MAME_CPU_Z80_Z80DASM_H

#pragma once

class z80_disassembler : public util::disasm_interface
{
public:
	z80_disassembler();
	virtual ~z80_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

protected:
	struct z80dasm
	{
		u8 mnemonic;
		const char *arguments;
	};

	enum e_mnemonics
	{
		// z80
		zADC  ,zADD  ,zAND  ,zBIT  ,zCALL ,zCCF  ,zCP   ,zCPD  ,
		zCPDR ,zCPI  ,zCPIR ,zCPL  ,zDAA  ,zDB   ,zDEC  ,zDI   ,
		zDJNZ ,zEI   ,zEX   ,zEXX  ,zHLT  ,zIM   ,zIN   ,zINC  ,
		zIND  ,zINDR ,zINI  ,zINIR ,zJP   ,zJR   ,zLD   ,zLDD  ,
		zLDDR ,zLDI  ,zLDIR ,zNEG  ,zNOP  ,zOR   ,zOTDR ,zOTIR ,
		zOUT  ,zOUTD ,zOUTI ,zPOP  ,zPUSH ,zRES  ,zRET  ,zRETI ,
		zRETN ,zRL   ,zRLA  ,zRLC  ,zRLCA ,zRLD  ,zRR   ,zRRA  ,
		zRRC  ,zRRCA ,zRRD  ,zRST  ,zSBC  ,zSCF  ,zSET  ,zSLA  ,
		zSLL  ,zSRA  ,zSRL  ,zSUB  ,zXOR

		// z80n
										  ,zSWAP ,zMIRR ,zTEST ,
		zBSLA ,zBSRA ,zBSRL ,zBSRF ,zBRLC ,zMUL  ,zOTIB ,zNREG ,
		zPXDN ,zPXAD ,zSTAE ,zLDIX ,zLDWS ,zLDDX ,zLIRX ,zLPRX ,
		zLDRX
	};

	static inline char sign(s8 offset);
	static inline u32 offs(s8 offset);

	static const u32 s_flags[];
	static const z80dasm mnemonic_xx_cb[256];
	static const z80dasm mnemonic_cb[256];
	static const z80dasm mnemonic_ed[256];
	static const z80dasm mnemonic_xx[256];
	static const z80dasm mnemonic_main[256];

	virtual const z80dasm &get_mnemonic_ed(u8 opcode) { return mnemonic_ed[opcode]; }
};

#endif
