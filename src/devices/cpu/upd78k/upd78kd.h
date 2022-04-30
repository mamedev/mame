// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78KD_H
#define MAME_CPU_UPD78K_UPD78KD_H

#pragma once

class upd78k_family_disassembler : public util::disasm_interface
{
protected:
	upd78k_family_disassembler(const char *const sfr_names[], const char *const sfrp_names[], u16 saddr_ram_base);

	// disasm_interface overrides
	virtual u32 opcode_alignment() const override;

	// formatting helpers
	void format_imm8(std::ostream &stream, u8 d);
	void format_imm16(std::ostream &stream, u16 d);
	void format_ix_disp8(std::ostream &stream, const char *r, u8 d);
	void format_ix_disp16(std::ostream &stream, const char *r, u16 d);
	void format_ix_base16(std::ostream &stream, const char *r, u16 d);
	void format_abs16(std::ostream &stream, u16 addr);
	virtual void format_jdisp8(std::ostream &stream, offs_t pc, u8 disp);
	void format_sfr(std::ostream &stream, u8 addr);
	void format_saddr(std::ostream &stream, u8 addr);
	void format_sfrp(std::ostream &stream, u8 addr);
	void format_saddrp(std::ostream &stream, u8 addr);
	void format_count(std::ostream &stream, u8 n);

	// generic illegal instruction dissasembly
	offs_t dasm_illegal(std::ostream &stream, u8 op);
	offs_t dasm_illegal2(std::ostream &stream, u8 op1, u8 op2);
	offs_t dasm_illegal3(std::ostream &stream, u8 op1, u8 op2, u8 op3);

private:
	const char *const *const m_sfr_names;
	const char *const *const m_sfrp_names;
	const u16 m_saddr_ram_base;
};

class upd78k_8reg_disassembler : public upd78k_family_disassembler
{
protected:
	upd78k_8reg_disassembler(const char *const sfr_names[], const char *const sfrp_names[]);

	// tables
	static const char *const s_r_names[8];
	static const char *const s_rp_names[4];
	static const char *const s_psw_bits[8];
};

#endif // MAME_CPU_UPD78K_UPD78KD_H
