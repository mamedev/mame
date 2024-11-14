// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_PCI_NCR53C825_PCI_H
#define MAME_BUS_PCI_NCR53C825_PCI_H

#pragma once

#include "pci_slot.h"

class ncr53c825_pci_device : public pci_card_device
{
public:
	ncr53c825_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::MEDIA; }

protected:
	ncr53c825_pci_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

	void io_map(address_map &map) ATTR_COLD;
	void scripts_map(address_map &map) ATTR_COLD;

	required_memory_region m_scsi_rom;
private:
	// ...
};

DECLARE_DEVICE_TYPE(NCR53C825_PCI, ncr53c825_pci_device)

#endif // MAME_BUS_PCI_NCR53C825_PCI_H
