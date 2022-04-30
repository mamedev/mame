// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_UPD78K_UPD78K0D_H
#define MAME_CPU_UPD78K_UPD78K0D_H

#pragma once

#include "upd78kd.h"

class upd78k0_disassembler : public upd78k_8reg_disassembler
{
protected:
	upd78k0_disassembler(const char *const sfr_names[], const char *const sfrp_names[]);

	// disasm_interface overrides
	virtual offs_t disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params) override;

	// SFR tables
	static const char *const s_common_sfrp_names[128];

private:
	// mnemonic tables
	static const char *const s_alu_ops[8];
	static const char *const s_bool_ops[4];

	// disassembly helpers
	offs_t dasm_31(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_61(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
	offs_t dasm_71(std::ostream &stream, offs_t pc, const data_buffer &opcodes);
};

class upd78014_disassembler : public upd78k0_disassembler
{
public:
	upd78014_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
};

class upd78024_disassembler : public upd78k0_disassembler
{
public:
	upd78024_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
};

class upd78044a_disassembler : public upd78k0_disassembler
{
public:
	upd78044a_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
};

class upd78054_disassembler : public upd78k0_disassembler
{
public:
	upd78054_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
};

class upd78064_disassembler : public upd78k0_disassembler
{
public:
	upd78064_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
};

class upd78078_disassembler : public upd78k0_disassembler
{
public:
	upd78078_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78083_disassembler : public upd78k0_disassembler
{
public:
	upd78083_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd780024a_disassembler : public upd78k0_disassembler
{
public:
	upd780024a_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd780065_disassembler : public upd78k0_disassembler
{
public:
	upd780065_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd780988_disassembler : public upd78k0_disassembler
{
public:
	upd780988_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78k0kx1_disassembler : public upd78k0_disassembler
{
public:
	upd78k0kx1_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

class upd78k0kx2_disassembler : public upd78k0_disassembler
{
public:
	upd78k0kx2_disassembler();

private:
	// SFR tables
	static const char *const s_sfr_names[256];
	static const char *const s_sfrp_names[128];
};

#endif // MAME_CPU_UPD78K_UPD78K0D_H
