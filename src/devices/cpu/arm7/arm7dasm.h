// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff,R. Belmont,Ryan Holtz
/*****************************************************************************
 *
 *   arm7dasm.h
 *   Portable ARM7TDMI Core Emulator - Disassembler
 *
 *   Copyright Steve Ellenoff
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/

#ifndef MAME_CPU_ARM7_ARM7DASM_H
#define MAME_CPU_ARM7_ARM7DASM_H

#pragma once

class arm7_disassembler : public util::disasm_interface
{
public:
	class config {
	public:
		virtual ~config() = default;
		virtual bool get_t_flag() const = 0;
	};

	arm7_disassembler(config *conf);

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	config *m_config;

	void WritePadding(std::ostream &stream, std::streampos start_position);
	void DasmCoProc_RT(std::ostream &stream, u32 opcode, const char *pConditionCode, std::streampos start_position);
	void DasmCoProc_DT(std::ostream &stream, u32 opcode, const char *pConditionCode, std::streampos start_position);
	void DasmCoProc_DO(std::ostream &stream, u32 opcode, const char *pConditionCode, std::streampos start_position);
	static u32 ExtractImmediateOperand( u32 opcode );
	void WriteShiftCount( std::ostream &stream, u32 opcode );
	void WriteDataProcessingOperand( std::ostream &stream, u32 opcode, bool printOp0, bool printOp1 );
	void WriteRegisterOperand1( std::ostream &stream, u32 opcode );
	void WriteBranchAddress( std::ostream &stream, u32 pc, u32 opcode, bool h_bit );
	u32 arm7_disasm( std::ostream &stream, u32 pc, u32 opcode );
	u32 thumb_disasm(std::ostream &stream, u32 pc, u16 opcode);
};


#endif
