// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\
 ARCompact disassembler

\*********************************/

#ifndef MAME_CPU_ARCOMPACT_ARCOMPACTDASM_H
#define MAME_CPU_ARCOMPACT_ARCOMPACTDASM_H

#pragma once

#include "arcompact_common.h"


class arcompact_disassembler : public util::disasm_interface, protected arcompact_common
{
public:
	arcompact_disassembler() = default;
	virtual ~arcompact_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	static const char *const conditions[0x20];
	static const char *const auxregnames[0x420];
	static const char *const datasize[0x4];
	static const char *const dataextend[0x2];
	static const char *const addressmode[0x4];
	static const char *const cachebit[0x2];
	static const char *const flagbit[0x2];
	static const char *const delaybit[0x2];
	static const char *const regnames[0x40];
	static const char *const opcodes_04[0x40];

private:
	class handle;

	static const int DASM_REG_LIMM = 62;

	static uint32_t dasm_get_limm_32bit_opcode(uint32_t pc, const data_buffer &opcodes)
	{
		return (opcodes.r16(pc + 4) << 16) | opcodes.r16(pc + 6);
	}

	static uint32_t dasm_get_limm_16bit_opcode(uint32_t pc, const data_buffer &opcodes)
	{
		return (opcodes.r16(pc + 2) << 16) | opcodes.r16(pc + 4);
	}

	static uint8_t dasm_group_0e_get_h(uint16_t &op)
	{
		uint8_t h = ((op & 0x0007) << 3);
		h |= ((op & 0x00e0) >> 5);
		return h;
	}

	static void output_aux_regname(std::ostream& stream, uint32_t auxreg);

	static int handle04_MOV_f_a_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_f_a_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_f_b_b_s12_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_cc_f_b_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_cc_f_b_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_p11_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);
	static int handle04_MOV_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes);

	static int handle01_01_00_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext);
	static int handle01_01_01_helper(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext);
	static int handle04_f_a_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved);
	static int handle04_f_a_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved);
	static int handle04_f_b_b_s12_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved);
	static int handle04_cc_f_b_b_c_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved);
	static int handle04_cc_f_b_b_u6_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved);
	static int handle04_p11_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int b_reserved);
	static int handle04_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext, int ignore_dst, int b_reserved);
	static int handle04_2f_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext);
	static int handle04_3x_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, int dsize, int extend);
	static int handle05_2f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint32_t op, const data_buffer &opcodes, const char* optext);
	static int handle0c_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int format);
	static int handle0d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);
	static int handle0e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int revop);
	static int handle0f_00_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);
	static int handle0f_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int nodst);
	static int handle_ld_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int swap);
	static int handle_l7_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);
	static int handle18_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int st, int format);
	static int handle19_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext, int shift, int format);
	static int handle1d_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);
	static int handle1e_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);
	static int handle1e_03_0x_helper_dasm(std::ostream &stream, offs_t pc, uint16_t op, const data_buffer &opcodes, const char* optext);

	static uint32_t get_01_01_01_address_offset(uint32_t op);
};

#endif // MAME_CPU_ARCOMPACT_ARCOMPACTDASM_H
