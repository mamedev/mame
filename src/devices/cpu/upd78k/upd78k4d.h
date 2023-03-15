// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78K4D_H
#define MAME_CPU_UPD78K_UPD78K4D_H

#pragma once

#include "upd78k3d.h"

class upd78k4_disassembler : public upd78k3_disassembler
{
protected:
	upd78k4_disassembler(const char *const sfr_names[], const char *const sfrp_names[]);

	// register and mnemonic tables
	static const char *const s_ix_bases[5];
	static const char *const s_imm24_offsets[4];
	static const char *const s_rg_names[4];

	// upd78k_family_disassembler overrides
	virtual void format_jdisp8(std::ostream &stream, offs_t pc, u8 disp) override;

	// upd78k3_disassembler overrides
	virtual offs_t dasm_05xx(std::ostream &stream, u8 op2) override;
	virtual offs_t dasm_06xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_07xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_09xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_16xx(std::ostream &stream, u8 op1, u8 op2) override;
	virtual offs_t dasm_24xx(std::ostream &stream, u8 op, u8 rr) override;
	virtual offs_t dasm_2a(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_38(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_3c(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_43(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_78(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_88xx(std::ostream &stream, u8 op, u8 rr) override;

private:
	// formatting helpers
	void format_imm24(std::ostream &stream, u32 d);
	void format_abs20(std::ostream &stream, u32 addr);
	void format_abs24(std::ostream &stream, u32 addr);
	void format_saddr1(std::ostream &stream, u8 addr);
	void format_saddrp1(std::ostream &stream, u8 addr);
	void format_saddrg1(std::ostream &stream, u8 addr);
	void format_saddrg2(std::ostream &stream, u8 addr);
	void format_jdisp16(std::ostream &stream, offs_t pc, u16 disp);
};

class upd784026_disassembler : public upd78k4_disassembler
{
public:
	upd784026_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784038_disassembler : public upd78k4_disassembler
{
public:
	upd784038_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784046_disassembler : public upd78k4_disassembler
{
public:
	upd784046_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784054_disassembler : public upd78k4_disassembler
{
public:
	upd784054_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784216_disassembler : public upd78k4_disassembler
{
public:
	upd784216_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784218_disassembler : public upd78k4_disassembler
{
public:
	upd784218_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd784225_disassembler : public upd78k4_disassembler
{
public:
	upd784225_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

#endif // MAME_CPU_UPD78K_UPD78K3D_H
