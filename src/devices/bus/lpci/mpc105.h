// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    mpc105.h

    Motorola MPC105 PCI bridge

***************************************************************************/

#ifndef MAME_BUS_LPCI_MPC105_H
#define MAME_BUS_LPCI_MPC105_H

#pragma once

#include "pci.h"
#include "machine/ram.h"

#define MPC105_MEMORYBANK_COUNT     8


// ======================> mpc105_device

class mpc105_device : public device_t,
	public pci_device_interface
{
public:
	// construction/destruction
	mpc105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	void set_bank_base_default(int bank_base_default) { m_bank_base_default = bank_base_default; }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void update_memory();

private:
	int m_bank_base_default;
	int m_bank_base;
	uint8_t m_bank_enable;
	uint32_t m_bank_registers[8];

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(MPC105, mpc105_device)

#endif // MAME_BUS_LPCI_MPC105_H
