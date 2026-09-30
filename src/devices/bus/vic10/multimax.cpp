// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MultiMAX 1MB ROM / 2KB RAM cartridge emulation

**********************************************************************/

#include "emu.h"
#include "multimax.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIC10_MULTIMAX, vic10_multimax_device, "vic10_multimax", "VIC-10 MultiMAX Cartridge")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vic10_multimax_device - constructor
//-------------------------------------------------

vic10_multimax_device::vic10_multimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIC10_MULTIMAX, tag, owner, clock), device_vic10_expansion_card_interface(mconfig, *this),
	m_exram(*this, "exram", 0x800, ENDIANNESS_LITTLE),
	m_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vic10_multimax_device::device_start()
{
	// state saving
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_post_load - called after loading a state
//-------------------------------------------------

void vic10_multimax_device::device_post_load()
{
	update_banks();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vic10_multimax_device::device_reset()
{
	m_latch = 0;

	m_slot->exram()[0].install_write_handler(0x000, 0x7ff, write8smo_delegate(*this, FUNC(vic10_multimax_device::latch_w)));
	m_slot->exram()[1].install_ram(0x000, 0x7ff, m_exram.target());

	update_banks();
}


//-------------------------------------------------
//  latch_w - bank latch write
//-------------------------------------------------

void vic10_multimax_device::latch_w(uint8_t data)
{
	m_latch = data;

	update_banks();
}


//-------------------------------------------------
//  update_banks -
//-------------------------------------------------

void vic10_multimax_device::update_banks()
{
	uint8_t *const rom = &m_slot->memregion("lorom")->base()[(m_latch & 0x3f) << 14];

	m_slot->lorom().install_rom(0x0000, 0x1fff, rom);
	m_slot->uprom().install_rom(0x0000, 0x1fff, rom + 0x2000);
	m_slot->exram().select(m_latch ? 1 : 0);
}
