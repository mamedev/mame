// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78K2D_H
#define MAME_CPU_UPD78K_UPD78K2D_H

#pragma once

#include "upd78k1d.h"

class upd78k2_disassembler : public upd78k1_disassembler
{
protected:
	upd78k2_disassembler(const char *const sfr_names[], const char *const sfrp_names[]);

	// disassembly helper overrides
	virtual offs_t dasm_01xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_05xx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_06(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_0axx(std::ostream &stream, u8 op2, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_16xx(std::ostream &stream, u8 op2, const data_buffer &opcodes) override;
	virtual offs_t dasm_25(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_29(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_38(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_43(std::ostream &stream, offs_t pc, const data_buffer &opcodes) override;
	virtual offs_t dasm_50(std::ostream &stream, u8 op) override;
	virtual offs_t dasm_78(std::ostream &stream, u8 op, offs_t pc, const data_buffer &opcodes) override;
};

class upd78214_disassembler : public upd78k2_disassembler
{
public:
	upd78214_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78218a_disassembler : public upd78k2_disassembler
{
public:
	upd78218a_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78224_disassembler : public upd78k2_disassembler
{
public:
	upd78224_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78234_disassembler : public upd78k2_disassembler
{
public:
	upd78234_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78244_disassembler : public upd78k2_disassembler
{
public:
	upd78244_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

#endif // MAME_CPU_UPD78K_UPD78K2D_H
