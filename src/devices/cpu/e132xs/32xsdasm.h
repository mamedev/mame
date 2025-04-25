// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

 Hyperstone disassembler
 written by Pierpaolo Prazzoli

*/

#ifndef MAME_CPU_E132XS_32XSDASM_H
#define MAME_CPU_E132XS_32XSDASM_H

#pragma once

#include <string_view>


class hyperstone_disassembler : public util::disasm_interface
{
public:
	struct config
	{
		virtual ~config() = default;

		virtual bool get_h() const = 0;
	};

	hyperstone_disassembler(config *conf);
	virtual ~hyperstone_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	config *m_config;

	int size;

	uint32_t LRconst_format(std::string_view &source, std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes);
	uint32_t RRconst_format(std::string_view &source, std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes);
	int32_t Rimm_format(std::string_view &dest, uint16_t op, offs_t pc, const data_buffer &opcodes, unsigned h_flag);
	int32_t PCrel_format(uint16_t op, offs_t pc, const data_buffer &opcodes);
	uint32_t RRdis_format(std::string_view &source, std::string_view &dest, uint16_t op, uint16_t next_op, offs_t pc, const data_buffer &opcodes);
};

#endif
