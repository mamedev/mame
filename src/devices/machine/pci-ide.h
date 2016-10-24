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

// Setting this to a value other than -1 will cause the ide_pci_device to assert/clear the cpu interrupt directly.
#define MCFG_IDE_PCI_IRQ_ADD(_cpu_tag, _irq_num) \
	downcast<ide_pci_device *>(device)->set_irq_info(_cpu_tag, _irq_num);

#define MCFG_IDE_PCI_IRQ_HANDLER(_devcb) \
	devcb = &ide_pci_device::set_irq_handler(*device, DEVCB_##_devcb);

class ide_pci_device : public pci_device {
public:
	ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	required_device<bus_master_ide_controller_device> m_ide;
	required_device<bus_master_ide_controller_device> m_ide2;
	virtual DECLARE_ADDRESS_MAP(config_map, 32) override;
	void ide_interrupt(int state);
	uint32_t ide_read_cs1(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ide_write_cs1(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t ide2_read_cs1(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ide2_write_cs1(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void set_irq_info(const char *tag, const int irq_num);
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ide_pci_device &>(device).m_irq_handler.set_callback(object); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	const char *m_cpu_tag;
	cpu_device *m_cpu;
	int m_irq_num;
	devcb_write_line m_irq_handler;

	uint32_t m_config_data[0x10];
	DECLARE_ADDRESS_MAP(chan1_data_command_map, 32);
	DECLARE_ADDRESS_MAP(chan1_control_map, 32);
	DECLARE_ADDRESS_MAP(chan2_data_command_map, 32);
	DECLARE_ADDRESS_MAP(chan2_control_map, 32);
	DECLARE_ADDRESS_MAP(bus_master_map, 32);
	uint32_t pcictrl_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void pcictrl_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void address_base_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
};

extern const device_type IDE_PCI;

#endif
