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

#define MCFG_IDE_PCI_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, IDE_PCI, _main_id, _revision, 0x01018a, _subdevice_id)

// Setting this to a value other than -1 will cause the ide_pci_device to assert/clear the cpu interrupt directly.
#define MCFG_IDE_PCI_IRQ_ADD(_cpu_tag, _irq_num) \
	downcast<ide_pci_device *>(device)->set_irq_info(_cpu_tag, _irq_num);

#define MCFG_IDE_PCI_IRQ_HANDLER(_devcb) \
	devcb = &ide_pci_device::set_irq_handler(*device, DEVCB_##_devcb);

// This will set the top 12 bits for address decoding in legacy mode. Needed for seattle driver.
#define MCFG_IDE_PCI_SET_LEGACY_TOP(_val) \
	downcast<ide_pci_device *>(device)->set_legacy_top(_val);

// Sets the default Programming Interface (PIF) register
#define MCFG_IDE_PCI_SET_PIF(_val) \
	downcast<ide_pci_device *>(device)->set_pif(_val);

class ide_pci_device : public pci_device {
public:
	ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_irq_info(const char *tag, const int irq_num);
	template <class Object> static devcb_base &set_irq_handler(device_t &device, Object &&cb) { return downcast<ide_pci_device &>(device).m_irq_handler.set_callback(std::forward<Object>(cb)); }
	void set_legacy_top(int val) { m_legacy_top = val & 0xfff; };
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

	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	devcb_write_line m_irq_handler;
	uint32_t pci_bar[6];
	// Bits 31-20 for legacy mode hack
	uint32_t m_legacy_top;
	uint32_t m_pif;

	uint32_t m_config_data[0x10];
	void chan1_data_command_map(address_map &map);
	void chan1_control_map(address_map &map);
	void chan2_data_command_map(address_map &map);
	void chan2_control_map(address_map &map);
	void bus_master_map(address_map &map);
};

DECLARE_DEVICE_TYPE(IDE_PCI, ide_pci_device)

#endif // MAME_MACHINE_PCI_IDE_H
