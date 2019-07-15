// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "pci-ide.h"

DEFINE_DEVICE_TYPE(IDE_PCI, ide_pci_device, "ide_pci", "PCI IDE interface")

ide_pci_device::ide_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, IDE_PCI, tag, owner, clock),
	m_ide(*this, "ide"),
	m_ide2(*this, "ide2"),
	m_irq_handler(*this),
	m_legacy_top(0x000),
	m_pif(0x8a),
	m_bus_master_tag(":pci:00.0"),
	m_bus_master_space(AS_DATA)
{
}

void ide_pci_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x09, 0x09).w(FUNC(ide_pci_device::prog_if_w));
	map(0x10, 0x1f).rw(FUNC(ide_pci_device::address_base_r), FUNC(ide_pci_device::address_base_w));
	map(0x2c, 0x2f).w(FUNC(ide_pci_device::subsystem_id_w));
	map(0x40, 0x5f).rw(FUNC(ide_pci_device::pcictrl_r), FUNC(ide_pci_device::pcictrl_w));
	map(0x70, 0x77).rw("ide", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w)); // PCI646
	map(0x78, 0x7f).rw("ide2", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w)); // PCI646
}

void ide_pci_device::chan1_data_command_map(address_map &map)
{
	map(0x0, 0x7).rw("ide", FUNC(bus_master_ide_controller_device::cs0_r), FUNC(bus_master_ide_controller_device::cs0_w));
}

void ide_pci_device::chan1_control_map(address_map &map)
{
	map(0x0, 0x3).rw(FUNC(ide_pci_device::ide_read_cs1), FUNC(ide_pci_device::ide_write_cs1));
}

void ide_pci_device::chan2_data_command_map(address_map &map)
{
	map(0x0, 0x7).rw("ide2", FUNC(bus_master_ide_controller_device::cs0_r), FUNC(bus_master_ide_controller_device::cs0_w));
}

void ide_pci_device::chan2_control_map(address_map &map)
{
	map(0x0, 0x3).rw(FUNC(ide_pci_device::ide2_read_cs1), FUNC(ide_pci_device::ide2_write_cs1));
}

void ide_pci_device::bus_master_map(address_map &map)
{
	map(0x0, 0x7).rw("ide", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x8, 0xf).rw("ide2", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

void ide_pci_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide).options(ata_devices, "hdd", "cdrom", true);
	m_ide->irq_handler().set(FUNC(ide_pci_device::ide_interrupt));
	m_ide->set_bus_master_space(m_bus_master_tag, m_bus_master_space);

	BUS_MASTER_IDE_CONTROLLER(config, m_ide2).options(ata_devices, "hdd", "cdrom", true);
	m_ide2->irq_handler().set(FUNC(ide_pci_device::ide_interrupt));
	m_ide2->set_bus_master_space(m_bus_master_tag, m_bus_master_space);
}

void ide_pci_device::device_start()
{
	pci_device::device_start();

	// always keep this device memory ranges active
	command |= 3;
	command_mask &= ~3;

	add_map(8,    M_IO,  FUNC(ide_pci_device::chan1_data_command_map));
	add_map(4,    M_IO,  FUNC(ide_pci_device::chan1_control_map));
	add_map(8,    M_IO,  FUNC(ide_pci_device::chan2_data_command_map));
	add_map(4,    M_IO,  FUNC(ide_pci_device::chan2_control_map));
	add_map(16,   M_IO,  FUNC(ide_pci_device::bus_master_map));

	// Setup stored BARs
	pci_bar[0] = 0x1f0;
	pci_bar[1] = 0x3f4;
	pci_bar[2] = 0x170;
	pci_bar[3] = 0x374;
	pci_bar[4] = 0xf00;

	m_irq_handler.resolve_safe();

	intr_pin = 0x1;
	intr_line = 0xe;

	// Save states
	save_item(NAME(pci_bar));
	save_item(NAME(m_config_data));

}

void ide_pci_device::device_reset()
{
	pci_device::device_reset();
	bank_infos[0].adr = (m_legacy_top << 20) | 0x1f0;
	bank_infos[1].adr = (m_legacy_top << 20) | 0x3f4;
	bank_infos[2].adr = (m_legacy_top << 20) | 0x170;
	bank_infos[3].adr = (m_legacy_top << 20) | 0x374;
	bank_infos[4].adr = 0xf00;
	pclass = 0x010100 | m_pif;
	remap_cb();

	// PCI0646U allow BAR
	if (main_id == 0x10950646)
		m_config_data[0x10 / 4] |= 0x0C40;
	m_ide->reset();
	m_ide2->reset();
}

READ32_MEMBER(ide_pci_device::ide_read_cs1)
{
	// PCI offset starts at 0x3f4, idectrl expects 0x3f0
	uint32_t data;
	data = m_ide->read_cs1(1, mem_mask);
	if (0)
		logerror("%s:ide_read_cs1 offset=%08X data=%08X mask=%08X\n", machine().describe_context(), offset, data, mem_mask);
	return data;
}

WRITE32_MEMBER(ide_pci_device::ide_write_cs1)
{
	// PCI offset starts at 0x3f4, idectrl expects 0x3f0
	m_ide->write_cs1(1, data, mem_mask);
}

READ32_MEMBER(ide_pci_device::ide2_read_cs1)
{
	// PCI offset starts at 0x374, idectrl expects 0x370
	uint32_t data;
	data = m_ide2->read_cs1(1, mem_mask);
	return data;
}

WRITE32_MEMBER(ide_pci_device::ide2_write_cs1)
{
	// PCI offset starts at 0x374, idectrl expects 0x370
	m_ide2->write_cs1(1, data, mem_mask);
}

WRITE_LINE_MEMBER(ide_pci_device::ide_interrupt)
{
	// Call the callback
	m_irq_handler(state);

	// PCI646U2 Offset 0x50 is interrupt status
	if (main_id == 0x10950646) {
		if (state)
			m_config_data[0x10 / 4] |= 0x4;
		else
			m_config_data[0x10 / 4] &= ~0x4;
	}
	if (0)
		logerror("%s:ide_interrupt set to %i\n", machine().describe_context(), state);
}

WRITE8_MEMBER(ide_pci_device::prog_if_w)
{
	uint32_t oldVal = pclass;
	pclass = (pclass & ~(0xff)) | (data & 0xff);
	// Check for switch to/from compatibility (legacy) mode from/to pci mode
	if ((oldVal ^ pclass) & 0x5) {
		// Map Primary IDE Channel
		if (pclass & 0x1) {
			// PCI Mode
			pci_device::address_base_w(space, 0, pci_bar[0]);
			pci_device::address_base_w(space, 1, pci_bar[1]);
		} else {
			// Legacy Mode
			pci_device::address_base_w(space, 0, (m_legacy_top << 20) | 0x1f0);
			pci_device::address_base_w(space, 1, (m_legacy_top << 20) | 0x3f4);
		}
		// Map Primary IDE Channel
		if (pclass & 0x4) {
			// PCI Mode
			pci_device::address_base_w(space, 2, pci_bar[2]);
			pci_device::address_base_w(space, 3, pci_bar[3]);
		}
		else {
			// Legacy Mode
			pci_device::address_base_w(space, 2, (m_legacy_top << 20) | 0x170);
			pci_device::address_base_w(space, 3, (m_legacy_top << 20) | 0x374);
		}
	}
	if (1)
		logerror("%s:prog_if_w pclass = %06X\n", machine().describe_context(), pclass);
}

READ32_MEMBER(ide_pci_device::pcictrl_r)
{
	return m_config_data[offset];
}

WRITE32_MEMBER(ide_pci_device::pcictrl_w)
{
	COMBINE_DATA(&m_config_data[offset]);
	// PCI646U2 Offset 0x50 is interrupt status
	if (main_id == 0x10950646 && offset == (0x10 / 4) && (data & 0x4)) {
		m_config_data[0x10 / 4] &= ~0x4;
		if (0)
			logerror("%s:ide_pci_device::pcictrl_w Clearing interrupt status\n", machine().describe_context());
	}
}

READ32_MEMBER(ide_pci_device::address_base_r)
{
	if (bank_reg_infos[offset].bank == -1)
		return 0;
	int bid = bank_reg_infos[offset].bank;
	if (bank_reg_infos[offset].hi)
		return bank_infos[bid].adr >> 32;
	int flags = bank_infos[bid].flags;
	return (pci_bar[offset] & ~(bank_infos[bid].size - 1)) | (flags & M_IO ? 1 : 0) | (flags & M_64A ? 4 : 0) | (flags & M_PREF ? 8 : 0);

}

WRITE32_MEMBER(ide_pci_device::address_base_w)
{
	// Save local copy of BAR
	pci_bar[offset] = data;
	// Bits 0 (primary) and 2 (secondary) control if the mapping is legacy or BAR
	switch (offset) {
	case 0: case 1:
		if (pclass & 0x1)
			pci_device::address_base_w(space, offset, data);
		break;
	case 2: case 3:
		if (pclass & 0x4)
			pci_device::address_base_w(space, offset, data);
		break;
	default:
		// Only the first 4 bars are controlled by pif
		pci_device::address_base_w(space, offset, data);
	}
	logerror("Mapping bar[%i] = %08x\n", offset, data);
}

WRITE32_MEMBER(ide_pci_device::subsystem_id_w)
{
	// Config register 0x4f enables subsystem id writing for CMD646
	if (m_config_data[0xc / 4] & 0x01000000)
		subsystem_id = (data << 16) | (data >> 16);
}
