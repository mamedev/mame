// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "ram_mm.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_RAM_MM, msx_slot_ram_mm_device, "msx_slot_ram_mm", "MSX Internal Memory Mapped RAM")

msx_slot_ram_mm_device::msx_slot_ram_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MSX_SLOT_RAM_MM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_total_size(0)
	, m_bank_mask(0)
	, m_ramio_set_bits(0)
	, m_rambank(*this, "mmbank%u", 0U)
{
	for (int i = 0; i < 4; i++)
		m_page[i] = nullptr;
}

void msx_slot_ram_mm_device::device_start()
{
	// Valid mapper sizes are 64KB, 128KB, 256KB, 512KB, 1MB, 2MB, and 4MB */
	switch (m_total_size)
	{
		case   64*1024: m_bank_mask = 0x03; break;
		case  128*1024: m_bank_mask = 0x07; break;
		case  256*1024: m_bank_mask = 0x0f; break;
		case  512*1024: m_bank_mask = 0x1f; break;
		case 1024*1024: m_bank_mask = 0x3f; break;
		case 2048*1024: m_bank_mask = 0x7f; break;
		case 4096*1024: m_bank_mask = 0xff; break;
		default: fatalerror("Invalid memory mapper size specified\n");
	}

	m_ram.resize(m_total_size);

	for (int i = 0; i < 4; i++)
	{
		m_bank_selected[i] = 3 - i;
		m_bank_base[i] = &m_ram[0x4000 * m_bank_selected[i]];
	}

	save_item(NAME(m_ram));
	save_item(NAME(m_bank_selected));

	// Install IO read/write handlers
	io_space().install_read_handler(0xfc, 0xff, read8sm_delegate(*this, FUNC(msx_slot_ram_mm_device::read_mapper_bank)));
	io_space().install_write_handler(0xfc, 0xff, write8sm_delegate(*this, FUNC(msx_slot_ram_mm_device::write_mapper_bank)));
	for (int i = 0; i < 4; i++)
	{
		m_rambank[i]->configure_entries(0, u32(m_bank_mask) + 1, m_ram.data(), 0x4000);
		m_page[i]->install_readwrite_bank(0x4000 * i, (0x4000 * i) + 0x3fff, m_rambank[i]);
	}
}

void msx_slot_ram_mm_device::device_post_load()
{
	restore_banks();
}

void msx_slot_ram_mm_device::restore_banks()
{
	for (int i = 0; i < 3; i++)
	{
		m_bank_base[i] = &m_ram[0x4000 * ( m_bank_selected[i] & m_bank_mask )];
	}
}

uint8_t msx_slot_ram_mm_device::read(offs_t offset)
{
	return m_bank_base[offset >> 14][offset & 0x3fff];
}

void msx_slot_ram_mm_device::write(offs_t offset, uint8_t data)
{
	m_bank_base[offset >> 14][offset & 0x3fff] = data;
}

uint8_t msx_slot_ram_mm_device::read_mapper_bank(offs_t offset)
{
	return m_rambank[offset & 3]->entry() | m_ramio_set_bits;
//	return m_bank_selected[offset & 3] | m_ramio_set_bits;
}

void msx_slot_ram_mm_device::write_mapper_bank(offs_t offset, uint8_t data)
{
	offset &= 3;

	m_bank_selected[offset] = data;
	m_bank_base[offset] = &m_ram[0x4000 * (m_bank_selected[offset] & m_bank_mask)];
	m_rambank[offset]->set_entry(data & m_bank_mask);
}

void msx_slot_ram_mm_device::install(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}
