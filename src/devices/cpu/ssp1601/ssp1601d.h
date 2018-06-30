// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli,Grazvydas Ignotas
/*

 SSP1601 disassembler
 written by Pierpaolo Prazzoli
 updated for SSP1601 by Grazvydas Ignotas

*/

#ifndef MAME_CPU_SSP1601_SSP1601DASM_H
#define MAME_CPU_SSP1601_SSP1601DASM_H

#pragma once

class ssp1601_disassembler : public util::disasm_interface
{
public:
	ssp1601_disassembler() = default;
	virtual ~ssp1601_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const reg[16];
	static const char *const rij[8];
	static const char *const modifier[4];
	static const char *const modifier_sf[4];
	static const char *const cond[16];
	static const char *const acc_op[8];
	static const char *const flag_op[16];
	static const char *const arith_ops[8];
	static std::string get_cond(int op);
};

#endif
