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

#define MPC105_MEMORYBANK_COUNT     8

#define MCFG_MPC105_CPU( _tag ) \
	downcast<mpc105_device &>(*device).set_cpu(_tag);

#define MCFG_MPC105_BANK_BASE_DEFAULT( bank_base_default ) \
	downcast<mpc105_device &>(*device).set_bank_base_default(bank_base_default);

// ======================> mpc105_device

class mpc105_device : public device_t,
	public pci_device_interface
{
public:
	// construction/destruction
	mpc105_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu(const char *tag) { m_cpu_tag = tag; }
	void set_bank_base_default(int bank_base_default) { m_bank_base_default = bank_base_default; }

	virtual uint32_t pci_read(pci_bus_device *pcibus, int function, int offset, uint32_t mem_mask) override;
	virtual void pci_write(pci_bus_device *pcibus, int function, int offset, uint32_t data, uint32_t mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void update_memory();

private:
	const char *m_cpu_tag;
	int m_bank_base_default;
	int m_bank_base;
	uint8_t m_bank_enable;
	uint32_t m_bank_registers[8];

	cpu_device*   m_maincpu;
};


// device type definition
DECLARE_DEVICE_TYPE(MPC105, mpc105_device)

#endif // MAME_BUS_LPCI_MPC105_H
