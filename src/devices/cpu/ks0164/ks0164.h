// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, David Carne

// KS0164 core

#ifndef MAME_CPU_KS0164_KS0164_H
#define MAME_CPU_KS0164_KS0164_H

#pragma once

class ks0164_device : public cpu_device
{
public:
	ks0164_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	int m_icount;
	u32 m_pc;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual uint32_t execute_input_lines() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	const address_space_config m_program_config;
};

DECLARE_DEVICE_TYPE(KS0164, ks0164_device)

#endif
