// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_CMI8738_H
#define MAME_BUS_PCI_CMI8738_H

#pragma once

#include "pci_slot.h"

class cmi8738_device : public pci_card_device
{
public:
	cmi8738_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	cmi8738_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;
	virtual u8 capptr_r() override;

	virtual uint8_t latency_timer_r() override;
private:
	void map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(CMI8738, cmi8738_device)

#endif // MAME_BUS_PCI_CMI8738_H
