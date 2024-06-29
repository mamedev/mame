// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_CPU_XA_XA_H
#define MAME_CPU_XA_XA_H

#pragma once

class xa_cpu_device : public cpu_device
{
public:
	xa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	xa_cpu_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock, address_map_constructor prg_map);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void internal_map(address_map &map);

	address_space_config m_program_config;

	uint32_t m_pc;

	address_space *m_program;
	int m_icount;
};

class mx10exa_cpu_device : public xa_cpu_device
{
public:
	mx10exa_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	void mx10exa_internal_map(address_map &map);
};


DECLARE_DEVICE_TYPE(XA, xa_cpu_device)
DECLARE_DEVICE_TYPE(MX10EXA, mx10exa_cpu_device)

#endif // MAME_CPU_XA_XA_H
