// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

[Cyrix/National Semiconductor/AMD] [MediaGX/Geode] [Cx/CS]5530 IDE implementation

TODO:
- Derive from common pci-ide.cpp interface

**************************************************************************************************/

#include "emu.h"
#include "mediagx_cs5530_ide.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

DEFINE_DEVICE_TYPE(MEDIAGX_CS5530_IDE, mediagx_cs5530_ide_device, "mediagx_cs5530_ide", "MediaGX CS5530 IDE Controller")

mediagx_cs5530_ide_device::mediagx_cs5530_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MEDIAGX_CS5530_IDE, tag, owner, clock)
	, m_ide1(*this, "ide1")
	, m_ide2(*this, "ide2")
	, m_irq_pri_callback(*this)
	, m_irq_sec_callback(*this)
	, m_bus_master_space(*this, finder_base::DUMMY_TAG, AS_PROGRAM)
{
}

void mediagx_cs5530_ide_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide1).options(ata_devices, "hdd", nullptr, false);
	m_ide1->irq_handler().set([this](int state) { m_irq_pri_callback(state); });
	m_ide1->set_bus_master_space(m_bus_master_space);

	BUS_MASTER_IDE_CONTROLLER(config, m_ide2).options(ata_devices, "cdrom", nullptr, false);
	m_ide2->irq_handler().set([this](int state) { m_irq_sec_callback(state); });
	m_ide2->set_bus_master_space(m_bus_master_space);
}

void mediagx_cs5530_ide_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//  index 0x24-0xff reserved
}

//
void mediagx_cs5530_ide_device::primary_ide_map(address_map &map)
{
	map(0x01f0, 0x01f7).rw(FUNC(mediagx_cs5530_ide_device::ide1_read32_cs0_r), FUNC(mediagx_cs5530_ide_device::ide1_write32_cs0_w));
	map(0x03f6, 0x03f6).rw(FUNC(mediagx_cs5530_ide_device::ide1_read_cs1_r), FUNC(mediagx_cs5530_ide_device::ide1_write_cs1_w));
}

void mediagx_cs5530_ide_device::secondary_ide_map(address_map &map)
{
	map(0x0170, 0x0177).rw(FUNC(mediagx_cs5530_ide_device::ide2_read32_cs0_r), FUNC(mediagx_cs5530_ide_device::ide2_write32_cs0_w));
	map(0x0376, 0x0376).rw(FUNC(mediagx_cs5530_ide_device::ide2_read_cs1_r), FUNC(mediagx_cs5530_ide_device::ide2_write_cs1_w));
}

uint32_t mediagx_cs5530_ide_device::ide1_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide1->read_cs0(offset, mem_mask);
}

void mediagx_cs5530_ide_device::ide1_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs0(offset, data, mem_mask);
}

uint32_t mediagx_cs5530_ide_device::ide2_read32_cs0_r(offs_t offset, uint32_t mem_mask)
{
	if (!(command & 1))
		return 0xffffffff;
	return m_ide2->read_cs0(offset, mem_mask);
}

void mediagx_cs5530_ide_device::ide2_write32_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs0(offset, data, mem_mask);
}

uint8_t mediagx_cs5530_ide_device::ide1_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide1->read_cs1(1, 0xff0000) >> 16;
}

void mediagx_cs5530_ide_device::ide1_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide1->write_cs1(1, data << 16, 0xff0000);
}

uint8_t mediagx_cs5530_ide_device::ide2_read_cs1_r()
{
	if (!(command & 1))
		return 0xff;
	return m_ide2->read_cs1(1, 0xff0000) >> 16;
}

void mediagx_cs5530_ide_device::ide2_write_cs1_w(uint8_t data)
{
	if (!(command & 1))
		return;
	m_ide2->write_cs1(1, data << 16, 0xff0000);
}


// F2BAR
void mediagx_cs5530_ide_device::io_map(address_map &map)
{
	map(0x00, 0x07).rw(m_ide1, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
	map(0x08, 0x0f).rw(m_ide2, FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
//  map(0x20, 0x23).select(0x18) Channel # (bit 5) Drive # (bit 3) PIO
//  map(0x24, 0x27).select(0x18) Channel # (bit 5) Drive # (bit 3) DMA Control
}

void mediagx_cs5530_ide_device::device_start()
{
	pci_device::device_start();

	skip_map_regs(4);
	add_map(128, M_IO, FUNC(mediagx_cs5530_ide_device::io_map));
}

void mediagx_cs5530_ide_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	status = 0x0280;
}
