// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82371SB PCI IDE ISA Xcelerator (PIIX3)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef MAME_BUS_LPCI_I82371SB_H
#define MAME_BUS_LPCI_I82371SB_H

#pragma once

#include "pci.h"
#include "southbridge.h"

// ======================> i82371sb_device

class i82371sb_device : public southbridge_device, public pci_device_interface
{
public:
	// construction/destruction
	i82371sb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_boot_state_hook(device_t &device, Object &&cb) { return downcast<i82371sb_device &>(device).m_boot_state_hook.set_callback(std::forward<Object>(cb)); }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void port80_debug_write(uint8_t value) override;

	uint32_t pci_isa_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_isa_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

	uint32_t pci_ide_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_ide_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);

	uint32_t pci_usb_r(device_t *busdevice, int offset, uint32_t mem_mask);
	void pci_usb_w(device_t *busdevice, int offset, uint32_t data, uint32_t mem_mask);
private:
	uint32_t m_regs[3][0x400/4];
	devcb_write8 m_boot_state_hook;
};

// device type definition
DECLARE_DEVICE_TYPE(I82371SB, i82371sb_device)

#define MCFG_I82371SB_BOOT_STATE_HOOK(_devcb) \
	devcb = &i82371sb_device::set_boot_state_hook(*device, DEVCB_##_devcb);

#endif // MAME_BUS_LPCI_I82371SB_H
