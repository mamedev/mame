// license:BSD-3-Clause
// copyright-holders:Samuele Zannoli
#ifndef MAME_SEGA_315_6154_H
#define MAME_SEGA_315_6154_H

#pragma once

#include "machine/pci.h"

DECLARE_DEVICE_TYPE(SEGA315_6154, sega_315_6154_device)

class sega_315_6154_device : public pci_host_device
{
public:
	// construction/destruction
	sega_315_6154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq1_callback() { return irq1_cb.bind(); }

	u32 registers_r(offs_t offset);
	void registers_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffff);
	template<int Aperture>
	u32 aperture_r(offs_t offset, u32 mem_mask = 0xffffffff);
	template<int Aperture>
	void aperture_w(offs_t offset, u32 data, u32 mem_mask = 0xffffffff);

	void set_mode(u32 mode) { m_mode = mode; };
	enum {
		AS_PCI_MEMORY = 1
	};
	enum {
		MODE_MASTER = 0,
		MODE_SLAVE = 0xffffffff
	};

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual void regenerate_config_mapping() override;

private:
	address_space_config m_configuration_config;
	address_space_config m_memory_config;
	address_space *m_configuration = nullptr;
	devcb_write_line irq1_cb;

	u32 m_registers[0x100 / 4];
	u32 m_bases[4 * 3];
	u32 m_mode;
	bool m_useconfig_14x;
	bool m_useconfig_18x;

	void irq_update();
};

#endif // MAME_SEGA_315_6154_H
