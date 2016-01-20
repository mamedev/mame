// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "ram_mm.h"

const device_type MSX_SLOT_RAM_MM = &device_creator<msx_slot_ram_mm_device>;

msx_slot_ram_mm_device::msx_slot_ram_mm_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MSX_SLOT_RAM_MM, "MSX Internal Memory Mapped RAM", tag, owner, clock, "msx_slot_ram_mm", __FILE__)
	, msx_internal_slot_interface()
	, m_total_size(0)
	, m_bank_mask(0)
	, m_ramio_set_bits(0)
{
}

void msx_slot_ram_mm_device::device_start()
{
	// Valid mapper sizes are 64KB, 128KB, 256KB, 512KB, 1MB, 2MB, and 4MB */
	switch (m_total_size)
	{
		case   64*1024: m_bank_mask = 0x03; break;
		case  128*1024: m_bank_mask = 0x07; break;
		case  256*1024: m_bank_mask = 0x0F; break;
		case  512*1024: m_bank_mask = 0x1F; break;
		case 1024*1024: m_bank_mask = 0x3F; break;
		case 2048*1024: m_bank_mask = 0x7F; break;
		case 4096*1024: m_bank_mask = 0xFF; break;
		default: fatalerror("Invalid memory mapper size specified\n");
	}

	m_ram.resize(m_total_size);

	for ( int i = 0; i < 4; i++ )
	{
		m_bank_selected[i] = 3 -i;
		m_bank_base[i] = &m_ram[0x4000 * m_bank_selected[i]];
	}

	save_item(NAME(m_ram));
	save_item(NAME(m_bank_selected));

	machine().save().register_postload(save_prepost_delegate(FUNC(msx_slot_ram_mm_device::restore_banks), this));

	// Install IO read/write handlers
	address_space &space = machine().device<cpu_device>("maincpu")->space(AS_IO);
	space.install_read_handler(0xFC, 0xFF, read8_delegate(FUNC(msx_slot_ram_mm_device::read_mapper_bank), this));
	space.install_write_handler(0xFC, 0xFF, write8_delegate(FUNC(msx_slot_ram_mm_device::write_mapper_bank), this));
}

void msx_slot_ram_mm_device::restore_banks()
{
	for ( int i = 0; i < 3; i++ )
	{
		m_bank_base[i] = &m_ram[0x4000 * ( m_bank_selected[i] & m_bank_mask )];
	}
}

READ8_MEMBER(msx_slot_ram_mm_device::read)
{
	return m_bank_base[offset >> 14][offset & 0x3fff];
}

WRITE8_MEMBER(msx_slot_ram_mm_device::write)
{
	m_bank_base[offset >> 14][offset & 0x3fff] = data;
}

READ8_MEMBER(msx_slot_ram_mm_device::read_mapper_bank)
{
	return m_bank_selected[offset & 3] | m_ramio_set_bits;
}

WRITE8_MEMBER(msx_slot_ram_mm_device::write_mapper_bank)
{
	offset &= 3;

	m_bank_selected[offset] = data;
	m_bank_base[offset] = &m_ram[0x4000 * ( m_bank_selected[offset] & m_bank_mask )];
}
