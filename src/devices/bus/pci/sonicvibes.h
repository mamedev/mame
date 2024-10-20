// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PCI_SONICVIBES_H
#define MAME_BUS_PCI_SONICVIBES_H

#pragma once

#include "pci_slot.h"

#include "bus/pc_joy/pc_joy.h"
#include "sound/ymopl.h"


class sonicvibes_device : public pci_card_device
{
public:
	sonicvibes_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::SOUND; }

protected:
	sonicvibes_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<ymf262_device> m_opl3;
	required_device<pc_joy_device> m_joy;

	void games_legacy_map(address_map &map) ATTR_COLD;
	void enhanced_map(address_map &map) ATTR_COLD;
	void fm_map(address_map &map) ATTR_COLD;
	void midi_map(address_map &map) ATTR_COLD;
	void gameport_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SONICVIBES, sonicvibes_device)

#endif // MAME_BUS_PCI_SONICVIBES_H
