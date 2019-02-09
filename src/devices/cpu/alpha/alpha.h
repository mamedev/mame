// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_ALPHA_ALPHA_H
#define MAME_CPU_ALPHA_ALPHA_H

#pragma once

#include "alphad.h"

class alpha_device : public cpu_device
{
public:
	void set_dasm_type(alpha_disassembler::dasm_type type) { m_dasm_type = type; }

protected:
	alpha_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 1; }
	virtual u32 execute_max_cycles() const override { return 1; }
	virtual u32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// configuration
	address_space_config m_main_config;
	alpha_disassembler::dasm_type m_dasm_type;

	// emulation state
	int m_icount;

	u64 m_pc;
	u64 m_r[32]; // R31 is zero
	u64 m_f[32]; // F31 is zero
};

class dec_21064_device : public alpha_device
{
public:
	dec_21064_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(DEC_21064, dec_21064_device)

#endif // MAME_CPU_ALPHA_ALPHA_H
