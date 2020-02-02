// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_MACHINE_315_6154_H
#define MAME_MACHINE_315_6154_H

#pragma once

DECLARE_DEVICE_TYPE(SEGA315_6154, sega_315_6154_device)

class sega_315_6154_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	sega_315_6154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	address_space *memory() { return m_memory; }

	DECLARE_READ32_MEMBER(registers_r);
	DECLARE_WRITE32_MEMBER(registers_w);
	template<int Aperture>
	u32 aperture_r(address_space &space, offs_t offset, u32 mem_mask = 0xffffffff);
	template<int Aperture>
	void aperture_w(address_space &space, offs_t offset, u32 data, u32 mem_mask = 0xffffffff);

	enum {
		AS_PCI_MEMORY = 0,
		AS_PCI_CONFIGURATION = 1
	};

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_memory_config;
	address_space_config m_configuration_config;
	address_space *m_memory;
	address_space *m_configuration;

	u32 m_registers[0x100 / 4];
	u32 m_bases[4 * 3];
	bool m_useconfig_14x;
	bool m_useconfig_18x;

};

#endif // MAME_MACHINE_315_6154_H
