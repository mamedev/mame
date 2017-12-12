// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_CPU_ADSP2100_2100DASM_H
#define MAME_CPU_ADSP2100_2100DASM_H

#pragma once

class adsp21xx_disassembler : public util::disasm_interface
{
public:
	adsp21xx_disassembler() = default;
	virtual ~adsp21xx_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	static const char *const flag_change[];
	static const char *const mode_change[];
	static const char *const alu_xop[];
	static const char *const alu_yop[];
	static const char *const alu_dst[];
	static const char *const mac_xop[];
	static const char *const mac_yop[];
	static const char *const mac_dst[];
	static const char *const shift_xop[];;
	static const char *const reg_grp[][16];
	static const char *const dual_xreg[];
	static const char *const dual_yreg[];
	static const char *const condition[];
	static const char *const do_condition[];
	static const char *const alumac_op[][2];
	static const char *const shift_op[];
	static const char *const shift_by_op[];
	static const char *const constants[];

	void alumac(std::ostream &stream, int dest, int op);
	void aluconst(std::ostream &stream, int dest, int op);

};

#endif
