// license:GPL-2.0+
// copyright-holders:smf
#ifndef MAME_BUS_PCCARD_KONAMI_DUAL_PCCARD_H
#define MAME_BUS_PCCARD_KONAMI_DUAL_PCCARD_H

#pragma once

#include "pccard.h"

class konami_dual_pccard_device :
	public device_t,
	public device_pccard_interface
{
public:
	konami_dual_pccard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint16_t read_memory(offs_t offset, uint16_t mem_mask = ~0) override;
	virtual void write_memory(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) override;

protected:
	// device_t
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	required_device_array<pccard_slot_device, 2> m_slot;
};

DECLARE_DEVICE_TYPE(KONAMI_DUAL_PCCARD, konami_dual_pccard_device)

#endif // MAME_BUS_PCCARD_KONAMI_DUAL_PCCARD_H
