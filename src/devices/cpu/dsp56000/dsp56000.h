// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_CPU_DSP56000_DSP56000_H
#define MAME_CPU_DSP56000_DSP56000_H

#pragma once

class dsp56000_device_base : public cpu_device
{
protected:
	dsp56000_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 16; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// emulation state
	address_space_config m_p_config;
	address_space_config m_x_config;
	address_space_config m_y_config;

	int m_icount;

	// program-visible cpu state
	u16 m_pc;
};

class dsp56000_device : public dsp56000_device_base
{
public:
	dsp56000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

class dsp56001_device : public dsp56000_device_base
{
public:
	dsp56001_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
};

DECLARE_DEVICE_TYPE(DSP56000, dsp56000_device)
DECLARE_DEVICE_TYPE(DSP56001, dsp56001_device)

#endif // MAME_CPU_DSP56000_DSP56000_H
