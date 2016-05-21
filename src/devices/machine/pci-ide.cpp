// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "pci-ide.h"

const device_type IDE_PCI = &device_creator<ide_pci_device>;

ide_pci_device::ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, IDE_PCI, "IDE PCI interface", tag, owner, clock, "ide_pci", __FILE__),
	m_ide(*this, "ide"),
	m_ide2(*this, "ide2"),
	m_irq_num(-1)
{
}

DEVICE_ADDRESS_MAP_START(config_map, 32, ide_pci_device)
	AM_RANGE(0x10, 0x1f) AM_WRITE(address_base_w)
	AM_RANGE(0x40, 0x5f) AM_READWRITE(pcictrl_r, pcictrl_w)
	AM_INHERIT_FROM(pci_device::config_map)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(chan1_data_command_map, 32, ide_pci_device)
	AM_RANGE(0x0, 0x7) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, read_cs0, write_cs0)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(chan1_control_map, 32, ide_pci_device)
	AM_RANGE(0x0, 0x3) AM_READWRITE(ide_read_cs1, ide_write_cs1)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(chan2_data_command_map, 32, ide_pci_device)
	AM_RANGE(0x0, 0x7) AM_DEVREADWRITE("ide2", bus_master_ide_controller_device, read_cs0, write_cs0)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(chan2_control_map, 32, ide_pci_device)
	AM_RANGE(0x0, 0x3) AM_READWRITE(ide2_read_cs1, ide2_write_cs1)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(bus_master_map, 32, ide_pci_device)
	AM_RANGE(0x0, 0x7) AM_DEVREADWRITE("ide", bus_master_ide_controller_device, bmdma_r, bmdma_w)
	AM_RANGE(0x8, 0xf) AM_DEVREADWRITE("ide2", bus_master_ide_controller_device, bmdma_r, bmdma_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT(pci_ide)
	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(ide_pci_device, ide_interrupt))
	//MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":pci:00.0", AS_DATA)
	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide2", ata_devices, nullptr, "cdrom", true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(ide_pci_device, ide_interrupt))
	//MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":pci:00.0", AS_DATA)
MACHINE_CONFIG_END

machine_config_constructor ide_pci_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(pci_ide);
}

void ide_pci_device::set_irq_info(const char *tag, const int irq_num)
{
	m_cpu_tag = tag;
	m_irq_num = irq_num;
}

void ide_pci_device::device_start()
{
	m_cpu = machine().device<cpu_device>(m_cpu_tag);

	pci_device::device_start();

	add_map(8,    M_IO,  FUNC(ide_pci_device::chan1_data_command_map));
	bank_infos[0].adr = 0x1f0;
	add_map(4,    M_IO,  FUNC(ide_pci_device::chan1_control_map));
	bank_infos[1].adr = 0x3f4;
	add_map(8,    M_IO,  FUNC(ide_pci_device::chan2_data_command_map));
	bank_infos[2].adr = 0x170;
	add_map(4,    M_IO,  FUNC(ide_pci_device::chan2_control_map));
	bank_infos[3].adr = 0x374;
	add_map(16,   M_IO,  FUNC(ide_pci_device::bus_master_map));
	bank_infos[4].adr = 0xf00;
}

void ide_pci_device::device_reset()
{
	pci_device::device_reset();
}

READ32_MEMBER(ide_pci_device::ide_read_cs1)
{
	// PCI offset starts at 0x3f4, idectrl expects 0x3f0
	UINT32 data = 0;
	data = m_ide->read_cs1(space, ++offset, mem_mask);
	return data;
}

WRITE32_MEMBER(ide_pci_device::ide_write_cs1)
{
	// PCI offset starts at 0x3f4, idectrl expects 0x3f0
	m_ide->write_cs1(space, ++offset, data, mem_mask);
}

READ32_MEMBER(ide_pci_device::ide2_read_cs1)
{
	// PCI offset starts at 0x374, idectrl expects 0x370
	UINT32 data = 0;
	data = m_ide2->read_cs1(space, ++offset, mem_mask);
	return data;
}

WRITE32_MEMBER(ide_pci_device::ide2_write_cs1)
{
	// PCI offset starts at 0x374, idectrl expects 0x370
	m_ide2->write_cs1(space, ++offset, data, mem_mask);
}

WRITE_LINE_MEMBER(ide_pci_device::ide_interrupt)
{
	if (m_irq_num != -1) {
		m_cpu->set_input_line(m_irq_num, state);
	}
	if (0)
		logerror("%s:ide_interrupt %i set to %i\n", machine().describe_context(), m_irq_num, state);
}

READ32_MEMBER(ide_pci_device::pcictrl_r)
{
	return m_config_data[offset];
}

WRITE32_MEMBER(ide_pci_device::pcictrl_w)
{
	COMBINE_DATA(&m_config_data[offset]);
}

WRITE32_MEMBER(ide_pci_device::address_base_w)
{
	if (1) {
		// Bits 0 (ide) and 2 (ide2) control if the mapping is legacy or BAR
		switch (offset) {
		case 0:
			if ((pclass & 0x1) == 0)
				data = (data & 0xfffff000) | 0x1f0;
			break;
		case 1:
			if ((pclass & 0x1) == 0)
				data = (data & 0xfffff000) | 0x3f4;
			break;
		case 2:
			if ((pclass & 0x4) == 0)
				data = (data & 0xfffff000) | 0x170;
			break;
		default:
			if ((pclass & 0x4) == 0)
				data = (data & 0xfffff000) | 0x374;
		}
	}
	pci_device::address_base_w(space, offset, data);
}
