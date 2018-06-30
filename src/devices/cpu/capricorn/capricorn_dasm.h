// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// ********************************************************************************
// * HP Capricorn processor disassembler
// ********************************************************************************

#ifndef MAME_CPU_CAPRICORN_CAPRICORN_DASM_H
#define MAME_CPU_CAPRICORN_CAPRICORN_DASM_H

#pragma once

class capricorn_disassembler : public util::disasm_interface
{
public:
	capricorn_disassembler() = default;
	virtual ~capricorn_disassembler() = default;

	virtual u32 opcode_alignment() const override;
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

private:
	typedef offs_t (capricorn_disassembler::*fn_dis_param)(std::ostream &stream, offs_t pc, const data_buffer &opcodes);

	struct dis_entry_t {
		uint8_t m_op_mask;
		uint8_t m_opcode;
		const char *m_mnemonic;
		bool m_has_mb;
		char m_addr_mode;
		fn_dis_param m_param_fn;
		uint32_t m_dasm_flags;
	};

	static const dis_entry_t dis_table[];

	void direct_addr(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_arp_drp(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_dr(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_dr_ar(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_dr_lit(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_dr_lit_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes);
	offs_t param_dr_idx_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes);
	offs_t param_xr_lit(std::ostream &stream, offs_t pc,const data_buffer &opcodes);
	offs_t param_lit_dir(std::ostream &stream, offs_t pc,const data_buffer &opcodes);
	offs_t param_dr_id_ar(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t param_jmp_off(std::ostream &stream, offs_t pc, const data_buffer &opcodes);

};

#endif
