// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_MACHINE_PCI_APIC_H
#define MAME_MACHINE_PCI_APIC_H

#pragma once

#include "pci.h"

#define MCFG_APIC_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, APIC, _main_id, _revision, 0x0c0320, _subdevice_id)

class apic_device : public pci_device {
public:
	apic_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(APIC, apic_device)

#endif // MAME_MACHINE_PCI_APIC_H
