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
	template <typename T>
	void set_vga(T &&tag) { m_vga.set_tag(std::forward<T>(tag)); }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	void cirrus_42E8_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<vga_device> m_vga;
};


// device type definition
DECLARE_DEVICE_TYPE(PCI_CIRRUS_SVGA, pci_cirrus_svga_device)

#endif // MAME_BUS_LPCI_CIRRUS_H
