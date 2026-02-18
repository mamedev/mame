// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

VT82C586B PCIC IDE section

SFF8038i v1.0 compliant

**************************************************************************************************/

#include "emu.h"
#include "vt82c586b_ide.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(VT82C586B_IDE, vt82c586b_ide_device, "vt82c586b_ide", "VT82C586B \"PIPC\" IDE")

vt82c586b_ide_device::vt82c586b_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, VT82C586B_IDE, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_ide(*this, "ide%u", 1U)
	, m_irq_pri_callback(*this)
	, m_irq_sec_callback(*this)
{
	// pclass programmable
	set_ids(0x11060571, 0, 0x01018a, 0x00000000);
}

void vt82c586b_ide_device::device_start()
{
	pci_device::device_start();

	add_map(8, M_IO, FUNC(vt82c586b_ide_device::ide1_command_map));
	add_map(4, M_IO, FUNC(vt82c586b_ide_device::ide1_control_map));
	add_map(8, M_IO, FUNC(vt82c586b_ide_device::ide2_command_map));
	add_map(4, M_IO, FUNC(vt82c586b_ide_device::ide2_control_map));
	add_map(16, M_IO, FUNC(vt82c586b_ide_device::bus_master_ide_control_map));

}

void vt82c586b_ide_device::device_reset()
{
	pci_device::device_reset();

	// Medium DEVSEL#, Fast Back to Back
	status = 0x0280;
	// Address Stepping, I/O Space and Bus Master capable
	command = 0x80;
	command_mask = 0x85;

	pclass = 0x01018a;

	m_bar[0] = 0x1f0;
	m_bar[1] = 0x3f4;
	m_bar[2] = 0x170;
	m_bar[3] = 0x374;
	m_bar[4] = 0xcc0;
	for (int i = 0; i < 5; i++)
		bank_infos[i].adr = m_bar[i];
	remap_cb();
	flush_ide_mode();
}

inline bool vt82c586b_ide_device::ide1_mode()
{
	return (pclass & 0x3) == 3;
}

inline bool vt82c586b_ide_device::ide2_mode()
{
	return (pclass & 0xc) == 0xc;
}

// In compatible mode BARs with legacy addresses but can values written can still be readout.
// In practice we need to override writes and make sure we flush remapping accordingly
inline void vt82c586b_ide_device::flush_ide_mode()
{
	// Map Primary IDE Channel
	if (ide1_mode())
	{
		// PCI Mode
		pci_device::address_base_w(0, m_bar[0]);
		pci_device::address_base_w(1, m_bar[1]);
	}
	else
	{
		// Legacy Mode
		pci_device::address_base_w(0, 0x1f0);
		pci_device::address_base_w(1, 0x3f4);
	}

	// Map Secondary IDE Channel
	if (ide2_mode())
	{
		// PCI Mode
		pci_device::address_base_w(2, m_bar[2]);
		pci_device::address_base_w(3, m_bar[3]);
	}
	else
	{
		// Legacy Mode
		pci_device::address_base_w(2, 0x170);
		pci_device::address_base_w(3, 0x374);
	}
}

void vt82c586b_ide_device::prog_if_w(u8 data)
{
	uint32_t oldVal = pclass;
	pclass = (pclass & ~(0xff)) | (data & 0xff);
	// Check for switch to/from compatibility (legacy) mode from/to pci mode
	if ((oldVal ^ pclass) & 0xf)
		flush_ide_mode();
}

u32 vt82c586b_ide_device::bar_r(offs_t offset)
{
	if (bank_reg_infos[offset].bank == -1)
		return 0;
	int bid = bank_reg_infos[offset].bank;
	if (bank_reg_infos[offset].hi)
		return bank_infos[bid].adr >> 32;
	int flags = bank_infos[bid].flags;
	return (m_bar[offset] & ~(bank_infos[bid].size - 1)) | (flags & M_IO ? 1 : 0) | (flags & M_64A ? 4 : 0) | (flags & M_PREF ? 8 : 0);
}

void vt82c586b_ide_device::bar_w(offs_t offset, u32 data)
{
	m_bar[offset] = data;
	// Bits 0 (primary) and 2 (secondary) control if the mapping is legacy or BAR
	switch (offset) {
	case 0:
	case 1:
		if (ide1_mode())
			pci_device::address_base_w(offset, data);
		break;
	case 2:
	case 3:
		if (ide2_mode())
			pci_device::address_base_w(offset, data);
		break;
	default:
		// Only the first 4 bars are controlled by pif
		pci_device::address_base_w(offset, data);
	}
	LOG("Mapping bar[%i] = %08x\n", offset, data);
}


// $1f0
void vt82c586b_ide_device::ide1_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(vt82c586b_ide_device::ide1_read32_cs0_r), FUNC(vt82c586b_ide_device::ide1_write32_cs0_w));
}

// $3f4
void vt82c586b_ide_device::ide1_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(vt82c586b_ide_device::ide1_read_cs1_r), FUNC(vt82c586b_ide_device::ide1_write_cs1_w));
}

// $170
void vt82c586b_ide_device::ide2_command_map(address_map &map)
{
	map(0, 7).rw(FUNC(vt82c586b_ide_device::ide2_read32_cs0_r), FUNC(vt82c586b_ide_device::ide2_write32_cs0_w));
}

// $374
void vt82c586b_ide_device::ide2_control_map(address_map &map)
{
	map(2, 2).rw(FUNC(vt82c586b_ide_device::ide2_read_cs1_r), FUNC(vt82c586b_ide_device::ide2_write_cs1_w));
}

void vt82c586b_ide_device::bus_master_ide_control_map(address_map &map)
{
	map(0x0, 0x7).rw(m_ide[0], FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x8, 0xf).rw(m_ide[1], FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

void vt82c586b_ide_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide[0]).options(ata_devices, "hdd", nullptr, false);
	m_ide[0]->irq_handler().set([this] (int state) { m_irq_pri_callback(state); });

	BUS_MASTER_IDE_CONTROLLER(config, m_ide[1]).options(ata_devices, "cdrom", nullptr, false);
	m_ide[1]->irq_handler().set([this] (int state) { m_irq_sec_callback(state); });
}

void vt82c586b_ide_device::device_config_complete()
{
	auto ide1 = m_ide[0].finder_target();
	auto ide2 = m_ide[1].finder_target();
	ide1.first.subdevice<bus_master_ide_controller_device>(ide1.second)->set_bus_master_space(m_maincpu, AS_PROGRAM);
	ide2.first.subdevice<bus_master_ide_controller_device>(ide2.second)->set_bus_master_space(m_maincpu, AS_PROGRAM);

	pci_device::device_config_complete();
}


void vt82c586b_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x09, 0x09).w(FUNC(vt82c586b_ide_device::prog_if_w));
	map(0x10, 0x23).rw(FUNC(vt82c586b_ide_device::bar_r), FUNC(vt82c586b_ide_device::bar_w));
	map(0x24, 0x27).unmaprw();
}


/*
 *
 * Legacy/Compatibility interface
 *
 */

uint32_t vt82c586b_ide_device::ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide[0]->read_cs0(offset, mem_mask);
}

void vt82c586b_ide_device::ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide[0]->write_cs0(offset, data, mem_mask);
}

uint32_t vt82c586b_ide_device::ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide[1]->read_cs0(offset, mem_mask);
}

void vt82c586b_ide_device::ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide[1]->write_cs0(offset, data, mem_mask);
}

uint8_t vt82c586b_ide_device::ide1_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide[0]->read_cs1(1, 0xff0000) >> 16;
}

void vt82c586b_ide_device::ide1_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide[0]->write_cs1(1, data << 16, 0xff0000);
}

uint8_t vt82c586b_ide_device::ide2_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide[1]->read_cs1(1, 0xff0000) >> 16;
}

void vt82c586b_ide_device::ide2_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide[1]->write_cs1(1, data << 16, 0xff0000);
}

