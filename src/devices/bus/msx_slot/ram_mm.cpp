// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "ram_mm.h"

DEFINE_DEVICE_TYPE(MSX_SLOT_RAM_MM, msx_slot_ram_mm_device, "msx_slot_ram_mm", "MSX Internal Memory Mapped RAM")

msx_slot_ram_mm_device::msx_slot_ram_mm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MSX_SLOT_RAM_MM, tag, owner, clock)
	, msx_internal_slot_interface(mconfig, *this)
	, m_total_size(0)
	, m_bank_mask(0)
	, m_ramio_set_bits(0)
	, m_rambank(*this, "mmbank%u", 0U)
{
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

	save_item(NAME(m_ram));

	// Install IO read/write handlers
	io_space().install_read_handler(0xfc, 0xff, read8sm_delegate(*this, FUNC(msx_slot_ram_mm_device::read_mapper_bank)));
	io_space().install_write_handler(0xfc, 0xff, write8sm_delegate(*this, FUNC(msx_slot_ram_mm_device::write_mapper_bank)));
	for (int i = 0; i < 4; i++)
	{
		m_rambank[i]->configure_entries(0, u32(m_bank_mask) + 1, m_ram.data(), 0x4000);
		page(i)->install_readwrite_bank(0x4000 * i, (0x4000 * i) + 0x3fff, m_rambank[i]);
	}
}

u8 msx_slot_ram_mm_device::read_mapper_bank(offs_t offset)
{
	return m_rambank[offset & 3]->entry() | m_ramio_set_bits;
}

void msx_slot_ram_mm_device::write_mapper_bank(offs_t offset, u8 data)
{
	m_rambank[offset & 0x03]->set_entry(data & m_bank_mask);
}
