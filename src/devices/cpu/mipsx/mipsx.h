// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_MIPSX_MIPSX_H
#define MAME_CPU_MIPSX_MIPSX_H

#pragma once

class mipsx_cpu_device : public cpu_device
{
public:
	mipsx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	uint32_t m_pc;

	address_space *m_program;
	int m_icount;
};

DECLARE_DEVICE_TYPE(MIPSX, mipsx_cpu_device)

#endif // MAME_CPU_MIPSX_MIPSX_H
