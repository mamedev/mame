// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    NCR 53C810 PCI interface
    The original part has no SCRIPTS RAM BAR or expansion ROM.
 */

#include "emu.h"
#include "ncr53c810.h"

DEFINE_DEVICE_TYPE(NCR53C810_PCI, ncr53c810_pci_device, "ncr53c810_pci", "NCR 53C810 PCI")

ncr53c810_pci_device::ncr53c810_pci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: pci_card_device(mconfig, NCR53C810_PCI, tag, owner, clock)
	, m_scripts(*this, "scripts")
	, m_io_space(nullptr)
	, m_io_offset(0)
	, m_latency_timer(0)
{
	set_ids(0x1000'0001, 0x01, 0x000000, 0);
	set_irq_map(0);
}

void ncr53c810_pci_device::device_add_mconfig(machine_config &config)
{
	NCR53C810(config, m_scripts, DERIVED_CLOCK(1, 1));
	m_scripts->memory_read().set(FUNC(ncr53c810_pci_device::memory_r));
	m_scripts->memory_write().set(FUNC(ncr53c810_pci_device::memory_w));
	m_scripts->io_read().set(FUNC(ncr53c810_pci_device::io_r));
	m_scripts->io_write().set(FUNC(ncr53c810_pci_device::io_w));
	m_scripts->irq_handler().set(FUNC(ncr53c810_pci_device::irq_w));
}

void ncr53c810_pci_device::device_start()
{
	pci_card_device::device_start();
	add_map(0x100, M_IO, FUNC(ncr53c810_pci_device::registers_map));
	add_map(0x100, M_MEM, FUNC(ncr53c810_pci_device::registers_map));
	intr_pin = 1;
	command_mask = 0x0147;
	save_item(NAME(m_latency_timer));
}

void ncr53c810_pci_device::device_reset()
{
	pci_card_device::device_reset();
	command = 0;
	status = 0x0200; // medium DEVSEL timing
	intr_line = 0;
	m_latency_timer = 0;
	set_map_address(0, 0);
	set_map_address(1, 0);
	command_changed();
	remap_cb();
}

void ncr53c810_pci_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	map(0x04, 0x05).w(FUNC(ncr53c810_pci_device::command_w));
	map(0x06, 0x07).w(FUNC(ncr53c810_pci_device::status_w));
	map(0x0d, 0x0d).w(FUNC(ncr53c810_pci_device::latency_timer_w));
	map(0x10, 0x17).w(FUNC(ncr53c810_pci_device::bar_w));
	map(0x3d, 0x3d).lw8(NAME([] (u8 data) { })); // INTA is fixed
	map(0x80, 0xff).rw(m_scripts, FUNC(ncr53c8xx_device_base::reg_r), FUNC(ncr53c8xx_device_base::reg_w));
}

void ncr53c810_pci_device::registers_map(address_map &map)
{
	map(0x00, 0x7f).mirror(0x80).rw(m_scripts, FUNC(ncr53c8xx_device_base::reg_r), FUNC(ncr53c8xx_device_base::reg_w));
}

void ncr53c810_pci_device::command_w(offs_t offset, u16 data, u16 mem_mask)
{
	pci_card_device::command_w(offset, data, mem_mask);
	command_changed();
}

void ncr53c810_pci_device::command_changed()
{
	m_scripts->cio_w(BIT(command, 0));
	m_scripts->cm_w(BIT(command, 1));
	m_scripts->bus_master_w(BIT(command, 2));
}

void ncr53c810_pci_device::status_w(offs_t offset, u16 data, u16 mem_mask)
{
	status &= ~(data & mem_mask & 0xf900);
}

u8 ncr53c810_pci_device::latency_timer_r()
{
	return m_latency_timer;
}

void ncr53c810_pci_device::latency_timer_w(u8 data)
{
	m_latency_timer = data;
}

void ncr53c810_pci_device::bar_w(offs_t offset, u32 data, u32 mem_mask)
{
	address_base_w(offset, (address_base_r(offset) & ~mem_mask) | (data & mem_mask));
}

void ncr53c810_pci_device::map_extra(u64 memory_window_start, u64 memory_window_end, u64 memory_offset, address_space *memory_space,
	u64 io_window_start, u64 io_window_end, u64 io_offset, address_space *io_space)
{
	// The root supplies the memory master space; map_device supplies PCI I/O.
	m_io_space = io_space;
	m_io_offset = io_offset;
}

u32 ncr53c810_pci_device::memory_r(offs_t address, u32 mem_mask)
{
	return get_pci_busmaster_space()->read_dword(address, mem_mask);
}

void ncr53c810_pci_device::memory_w(offs_t address, u32 data, u32 mem_mask)
{
	get_pci_busmaster_space()->write_dword(address, data, mem_mask);
}

u32 ncr53c810_pci_device::io_r(offs_t address, u32 mem_mask)
{
	return m_io_space->read_dword(address + m_io_offset, mem_mask);
}

void ncr53c810_pci_device::io_w(offs_t address, u32 data, u32 mem_mask)
{
	m_io_space->write_dword(address + m_io_offset, data, mem_mask);
}

void ncr53c810_pci_device::irq_w(int state)
{
	irq_pin_w(0, state);
}
