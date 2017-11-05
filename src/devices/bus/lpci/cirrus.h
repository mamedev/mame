// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    video/cirrus.h

    Cirrus SVGA card emulation (preliminary)

***************************************************************************/

#ifndef MAME_BUS_LPCI_CIRRUS_H
#define MAME_BUS_LPCI_CIRRUS_H

#pragma once

#include "bus/lpci/pci.h"

// ======================> pci_cirrus_svga_device

class pci_cirrus_svga_device : public device_t, public pci_device_interface
{
public:
		// construction/destruction
	pci_cirrus_svga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	DECLARE_WRITE8_MEMBER( cirrus_42E8_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(PCI_CIRRUS_SVGA, pci_cirrus_svga_device)

#endif // MAME_BUS_LPCI_CIRRUS_H
