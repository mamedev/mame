// license:BSD-3-Clause
// copyright-holders:Ted Green
/***************************************************************************

pci-ide.h

Generic PCI IDE controller implementation.
Based on datasheet for National Semiconductor PC87415

TODO:
	Add pci configuration write to PIF byte
***************************************************************************/

#ifndef PCI_IDE_H
#define PCI_IDE_H

#include "pci.h"
#include "idectrl.h"

#define MCFG_IDE_PCI_ADD(_tag, _main_id, _revision, _subdevice_id) \
	MCFG_PCI_DEVICE_ADD(_tag, IDE_PCI, _main_id, _revision, 0x01018a, _subdevice_id)

#define MCFG_IDE_PCI_IRQ_ADD(_cpu_tag, _irq_num) \
	downcast<ide_pci_device *>(device)->set_irq_info(_cpu_tag, _irq_num);

class ide_pci_device : public pci_device {
public:
	ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	required_device<bus_master_ide_controller_device> m_ide;
	required_device<bus_master_ide_controller_device> m_ide2;
	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_READ32_MEMBER(ide_read_cs1);
	DECLARE_WRITE32_MEMBER(ide_write_cs1);
	DECLARE_READ32_MEMBER(ide2_read_cs1);
	DECLARE_WRITE32_MEMBER(ide2_write_cs1);
	void set_irq_info(const char *tag, const int irq_num);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	int m_irq_status;

	UINT32 m_config_data[0x10];
	DECLARE_ADDRESS_MAP(chan1_data_command_map, 32);
	DECLARE_ADDRESS_MAP(chan1_control_map, 32);
	DECLARE_ADDRESS_MAP(chan2_data_command_map, 32);
	DECLARE_ADDRESS_MAP(chan2_control_map, 32);
	DECLARE_ADDRESS_MAP(bus_master_map, 32);
	DECLARE_READ32_MEMBER(pcictrl_r);
	DECLARE_WRITE32_MEMBER(pcictrl_w);
	DECLARE_WRITE32_MEMBER(address_base_w);
};

extern const device_type IDE_PCI;

#endif
