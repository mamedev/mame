// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78K3D_H
#define MAME_CPU_UPD78K_UPD78K3D_H

#pragma once

#include "upd78kd.h"

class upd78k3_disassembler : public upd78k_family_disassembler
{
protected:
	upd78k3_disassembler(const char *const sfr_names[], const char *const sfrp_names[], const char *const psw_bits[], bool has_macw, bool has_macsw);
	upd78k3_disassembler(const char *const sfr_names[], const char *const sfrp_names[], const char *const ix_bases[], u16 saddr_ram_base);

	// register and mnemonic tables
	static const char *const s_r_names[16];
	static const char *const s_rp_names[8];
	static const char *const s_ix_bases[5];
	static const char *const s_psw_bits[16];
	static const char *const s_alu_ops[8];
	static const char *const s_16bit_ops[4];
	static const char *const s_bool_ops[4];
	static const char *const s_shift_ops[2][4];
	static const char *const s_bcond[8];
	static const char *const s_bcond_07f8[6];
	static const char *const s_cmpscond[4];

	// disasm_interface overrides
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// disassembly helpers
	offs_t dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_02xx(std::ostream &stream, u8 op1, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_04(std::ostream &stream);
	virtual offs_t dasm_05xx(std::ostream &stream, u8 op2);
	virtual offs_t dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_08xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_09xx_sfrmov(std::ostream &stream, u8 sfr, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_15xx(std::ostream &stream, u8 op2);
	virtual offs_t dasm_16xx(std::ostream &stream, u8 op1, u8 op2);
	virtual offs_t dasm_24xx(std::ostream &stream, u8 op, u8 rr);
	virtual offs_t dasm_2a(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_38(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_3c(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_43(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_50(std::ostream &stream, u8 op);
	virtual offs_t dasm_78(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes);
	virtual offs_t dasm_88xx(std::ostream &stream, u8 op, u8 rr);

	// parameters
	const char *const *const m_ix_bases;
	const char *const *const m_psw_bits;
	const bool m_has_macw;
	const bool m_has_macsw;
};

class upd78312_disassembler : public upd78k3_disassembler
{
public:
	upd78312_disassembler();

protected:
	// upd78k3_disassembler overrides
	virtual offs_t dasm_04(std::ostream &stream) override;
	virtual offs_t dasm_05xx(std::ostream &stream, u8 op2) override;
	virtual offs_t dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_16xx(std::ostream &stream, u8 op1, u8 op2) override;
	virtual offs_t dasm_50(std::ostream &stream, u8 op) override;

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];

	// PSW table
	static const char *const s_psw_bits[16];
};

class upd78322_disassembler : public upd78k3_disassembler
{
public:
	upd78322_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78328_disassembler : public upd78k3_disassembler
{
public:
	upd78328_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78334_disassembler : public upd78k3_disassembler
{
public:
	upd78334_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78352_disassembler : public upd78k3_disassembler
{
public:
	upd78352_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78356_disassembler : public upd78k3_disassembler
{
public:
	upd78356_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78366a_disassembler : public upd78k3_disassembler
{
public:
	upd78366a_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78372_disassembler : public upd78k3_disassembler
{
public:
	upd78372_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

#endif // MAME_CPU_UPD78K_UPD78K3D_H
