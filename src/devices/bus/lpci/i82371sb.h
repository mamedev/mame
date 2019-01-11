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

	auto smi() { return m_smi_callback.bind(); }
	auto boot_state_hook() { return m_boot_state_hook.bind(); }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

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
	DECLARE_READ8_MEMBER(read_apmcapms);
	DECLARE_WRITE8_MEMBER(write_apmcapms);

	void map_busmaster_dma();
	void update_smireq_line();

	uint32_t m_regs[3][0x400/4];
	devcb_write_line m_smi_callback;
	devcb_write8 m_boot_state_hook;
	int m_csmigate;
	int m_smien;
	int m_apmc;
	int m_apms;
	uint32_t m_base;
};

// device type definition
DECLARE_DEVICE_TYPE(I82371SB, i82371sb_device)

#endif // MAME_BUS_LPCI_I82371SB_H
