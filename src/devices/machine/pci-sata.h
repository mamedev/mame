// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_SATA_H
#define MAME_MACHINE_PCI_SATA_H

#pragma once

#include "pci.h"

class sata_device : public pci_device {
public:
	sata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: sata_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x01018a, subdevice_id);
	}
	sata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void primary_command_map(address_map &map) ATTR_COLD;
	void primary_control_map(address_map &map) ATTR_COLD;
	void secondary_command_map(address_map &map) ATTR_COLD;
	void secondary_control_map(address_map &map) ATTR_COLD;
	void bus_master_map(address_map &map) ATTR_COLD;
	void ide_command_posting_map(address_map &map) ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SATA, sata_device)

#endif // MAME_MACHINE_PCI_SATA_H
