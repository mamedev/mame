// license:BSD-3-Clause
// copyright-holders:Ted Green
/***************************************************************************

pci-ide.h

Generic PCI IDE controller implementation.
Based on datasheet for National Semiconductor PC87415

TODO:
    Add pci configuration write to PIF byte
***************************************************************************/

#ifndef MAME_MACHINE_PCI_IDE_H
#define MAME_MACHINE_PCI_IDE_H

#pragma once

#include "pci.h"
#include "idectrl.h"

class ide_pci_device : public pci_device {
public:
	ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, uint32_t main_id, uint32_t revision, uint32_t subdevice_id
		, const char *bmtag = ":pci:00.0", uint32_t bmspace = AS_DATA)
		: ide_pci_device(mconfig, tag, owner, clock)
	{
		set_ids(main_id, revision, 0x01018a, subdevice_id);
		m_bus_master_tag = bmtag;
		m_bus_master_space = bmspace;
	}
	ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_handler() { return m_irq_handler.bind(); }

	// This will set the top 12 bits for address decoding in legacy mode. Needed for seattle driver.
	void set_legacy_top(int val) { m_legacy_top = val & 0xfff; };

	// Sets the default Programming Interface (PIF) register
	void set_pif(int val) { m_pif = val & 0xff; };

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void config_map(address_map &map) override;

private:
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_WRITE8_MEMBER(prog_if_w);
	DECLARE_READ32_MEMBER(pcictrl_r);
	DECLARE_WRITE32_MEMBER(pcictrl_w);
	DECLARE_READ32_MEMBER(address_base_r);
	DECLARE_WRITE32_MEMBER(address_base_w);
	DECLARE_WRITE32_MEMBER(subsystem_id_w);
	DECLARE_READ32_MEMBER(ide_read_cs1);
	DECLARE_WRITE32_MEMBER(ide_write_cs1);
	DECLARE_READ32_MEMBER(ide2_read_cs1);
	DECLARE_WRITE32_MEMBER(ide2_write_cs1);

	required_device<bus_master_ide_controller_device> m_ide;
	required_device<bus_master_ide_controller_device> m_ide2;

	devcb_write_line m_irq_handler;
	uint32_t pci_bar[6];
	// Bits 31-20 for legacy mode hack
	uint32_t m_legacy_top;
	uint32_t m_pif;
	const char* m_bus_master_tag;
	uint32_t m_bus_master_space;

	uint32_t m_config_data[0x10];
	void chan1_data_command_map(address_map &map);
	void chan1_control_map(address_map &map);
	void chan2_data_command_map(address_map &map);
	void chan2_control_map(address_map &map);
	void bus_master_map(address_map &map);
};

DECLARE_DEVICE_TYPE(IDE_PCI, ide_pci_device)

#endif // MAME_MACHINE_PCI_IDE_H
