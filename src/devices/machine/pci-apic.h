// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef PCI_APIC_H
#define PCI_APIC_H

#include "pci.h"

#define MCFG_APIC_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, APIC, _main_id, _revision, 0x0c0320, _subdevice_id)

class apic_device : public pci_device {
public:
	apic_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

extern const device_type APIC;

#endif
