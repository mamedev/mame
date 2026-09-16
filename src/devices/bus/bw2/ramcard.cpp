// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 RAMCARD emulation

**********************************************************************/

#include "emu.h"
#include "ramcard.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BW2_RAMCARD, bw2_ramcard_device, "bw2_ramcard", "Bondwell 2 RAMCARD")


//-------------------------------------------------
//  ROM( bw2_ramcard )
//-------------------------------------------------

ROM_START( bw2_ramcard )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_LOAD( "ramcard-10.ic10", 0x0000, 0x4000, CRC(68cde1ba) SHA1(a776a27d64f7b857565594beb63aa2cd692dcf04) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bw2_ramcard_device::device_rom_region() const
{
	return ROM_NAME( bw2_ramcard );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bw2_ramcard_device - constructor
//-------------------------------------------------

bw2_ramcard_device::bw2_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BW2_RAMCARD, tag, owner, clock),
	device_bw2_expansion_slot_interface(mconfig, *this),
	m_rom(*this, "rom"),
	m_ram(*this, "ram", 512*1024, ENDIANNESS_LITTLE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bw2_ramcard_device::device_start()
{
	// state saving
	save_item(NAME(m_en));
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bw2_ramcard_device::device_reset()
{
	m_en = 0;
	m_bank = 0;
	ram_select();
}

void bw2_ramcard_device::slot_w(offs_t offset, uint8_t data)
{
	m_en = 1;
	m_bank = data & 0xf;
	ram_select();
}

void bw2_ramcard_device::ram_select()
{
	if (m_slot->ram2()) {
		m_rom_tap = m_slot->memspace()->install_read_tap(0x0000, 0x3fff, 0x4000, "rom",
			[this] (offs_t offset, u8 &data, u8) { data &= m_rom->base()[offset]; }, &m_rom_tap);
	} else {
		m_rom_tap.remove();
	}

	if (m_en && m_slot->ram5()) {
		m_ram_tap = m_slot->memspace()->install_readwrite_tap(0x0000, 0x7fff, "ram",
			[this] (offs_t offset, u8 &data, u8) { data &= m_ram[m_bank << 15 | offset]; },
			[this] (offs_t offset, u8 &data, u8) { m_ram[m_bank << 15 | offset] = data; }, &m_ram_tap);
	} else {
		m_ram_tap.remove();
	}
}
