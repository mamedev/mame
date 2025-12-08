// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_GAELCO_GAELCO_DS5002FP_H
#define MAME_GAELCO_GAELCO_DS5002FP_H

#pragma once


DECLARE_DEVICE_TYPE(GAELCO_DS5002FP, gaelco_ds5002fp_device)


class gaelco_ds5002fp_device : public device_t, public device_memory_interface
{
public:
	gaelco_ds5002fp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override ATTR_COLD;

private:
	void dallas_rom(address_map &map) ATTR_COLD;
	void dallas_ram(address_map &map) ATTR_COLD;

	uint8_t hostmem_r(offs_t offset);
	void hostmem_w(offs_t offset, uint8_t data);

	address_space_config const m_hostmem_config;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_hostmem;
};

#endif // MAME_GAELCO_GAELCO_DS5002FP_H
