// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Intel 82439TX System Controller (MTXC)

    Part of the Intel 430TX chipset

***************************************************************************/

#ifndef MAME_BUS_LPCI_I82439TX_H
#define MAME_BUS_LPCI_I82439TX_H

#pragma once

#include "pci.h"
#include "northbridge.h"

// ======================> i82439tx_device

class i82439tx_device : public northbridge_device, public pci_device_interface
{
public:
	// construction/destruction
	i82439tx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_region(const char *tag) { m_region_tag = tag; }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

	void smi_act_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void i82439tx_configure_memory(uint8_t val, offs_t begin, offs_t end);
	void update_smram_mappings();

private:
	const char *m_region_tag;

	address_space *m_space;
	uint8_t *m_rom;

	uint32_t m_regs[8*256];
	uint32_t m_bios_ram[0x40000 / 4];

	// system management ram setup
	struct {
		int tseg_en;
		int tseg_sz;
		int e_smerr;
		int h_smrame;
		int c_base_seg;
		int g_smrame;
		int d_lck;
		int d_cls;
		int d_open;
		int smiact_n;
		int tseg_size;
		int mapping;
	} m_smram;
};

// device type definition
DECLARE_DEVICE_TYPE(I82439TX_LEGACY, i82439tx_device)

#endif // MAME_BUS_LPCI_I82439TX_H
