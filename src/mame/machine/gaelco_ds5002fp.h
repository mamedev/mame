// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GAELCO_DS5002FP_H
#define MAME_MACHINE_GAELCO_DS5002FP_H

#pragma once


DECLARE_DEVICE_TYPE(GAELCO_DS5002FP, gaelco_ds5002fp_device)


class gaelco_ds5002fp_device : public device_t, public device_memory_interface
{
public:
	gaelco_ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// not really public, but address map needs to get them
	DECLARE_READ8_MEMBER(hostmem_r);
	DECLARE_WRITE8_MEMBER(hostmem_w);

	void dallas_ram(address_map &map);
	void dallas_rom(address_map &map);
protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config const m_hostmem_config;
	address_space *m_hostmem;
};

#endif // MAME_MACHINE_GAELCO_DS5002FP_H
