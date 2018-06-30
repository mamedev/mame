// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Mariusz Wojcieszek

#ifndef MAME_CPU_SCUDSP_SCUDSPDASM_H
#define MAME_CPU_SCUDSP_SCUDSPDASM_H

#pragma once

class scudsp_disassembler : public util::disasm_interface
{
public:
	scudsp_disassembler() = default;
	virtual ~scudsp_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const ALU_Commands[];
	static const char *const X_Commands[];
	static const char *const Y_Commands[];
	static const char *const D1_Commands[];
	static const char *const SourceMemory[];
	static const char *const SourceMemory2[];
	static const char *const DestMemory[];
	static const char *const DestDMAMemory[];
	static const char *const MVI_Command[];
	static const char *const JMP_Command[];
	static const char *const DMA_Command[];
	std::string scudsp_dasm_prefix( const char* format, uint32_t *data );
};

#endif
