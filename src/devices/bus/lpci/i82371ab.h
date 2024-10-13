// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82371AB PCI IDE ISA Xcelerator (PIIX4)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef MAME_BUS_LPCI_I82371AB_H
#define MAME_BUS_LPCI_I82371AB_H

#pragma once

#include "pci.h"
#include "southbridge.h"

// ======================> i82371ab_device

class i82371ab_device :  public southbridge_extended_device,
							public pci_device_interface
{
public:
	// construction/destruction
	i82371ab_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	uint32_t pci_isa_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_isa_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

	uint32_t pci_ide_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_ide_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

	uint32_t pci_usb_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_usb_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

	uint32_t pci_acpi_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_acpi_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

private:
	uint8_t m_regs[4][0x100];
};

// device type definition
DECLARE_DEVICE_TYPE(I82371AB, i82371ab_device)

#endif // MAME_BUS_LPCI_I82371AB_H
