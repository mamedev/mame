// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_APIC_H
#define MAME_MACHINE_PCI_APIC_H

#pragma once

#include "pci.h"

class apic_device : public pci_device {
public:
	apic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id)
		: apic_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x080010, subdevice_id);
	}
	apic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(APIC, apic_device)

#endif // MAME_MACHINE_PCI_APIC_H
