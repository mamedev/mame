// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    JCB Sound Extension Module

    http://archive.worldofdragon.org/index.php?title=Dragon_32_Sound_Extension_Module

    The Dragon 32 Sound Extension Module is a cartridge by J.C.B. (Microsystems),
    that contains a General Instruments AY-3-8910 sound chip. This allows the
    Dragon to play interesting sound effects and complex chiptunes without
    taking all processor time.
    The cartridge also adds two 8-bit I/O ports (provided also by the AY-3-8910).
    The sound chip can be operated via the new commands in BASIC provided in
    the cartridge ROM, or via the 0xFEFE and 0xFEFF addresses.

***************************************************************************/

#include "emu.h"
#include "dragon_jcbsnd.h"
#include "speaker.h"


ROM_START( dragon_jcbsnd )
	ROM_REGION(0x1000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("d32sem.rom", 0x0000, 0x1000, CRC(4cd0f30b) SHA1(d07bb9272e3d3928059853730ff656905a80b68e))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_JCBSND, dragon_jcbsnd_device, "dragon_jcbsnd", "Dragon Sound Extension Module")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_jcbsnd_device - constructor
//-------------------------------------------------

dragon_jcbsnd_device::dragon_jcbsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DRAGON_JCBSND, tag, owner, clock)
	, device_cococart_interface(mconfig, *this )
	, m_eprom(*this, "eprom")
	, m_ay8910(*this, "ay8910")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_jcbsnd_device::device_start()
{
	set_line_value(line::CART, line_value::Q);
}

//-------------------------------------------------
//  dragon_jcbsnd_device::get_cart_base
//-------------------------------------------------

uint8_t* dragon_jcbsnd_device::get_cart_base()
{
	return m_eprom->base();
}

//-------------------------------------------------
//  dragon_jcbsnd_device::get_cart_memregion
//-------------------------------------------------

memory_region* dragon_jcbsnd_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void dragon_jcbsnd_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay8910, DERIVED_CLOCK(1, 1)); /* AY-3-8910 - clock not verified */
	m_ay8910->add_route(ALL_OUTPUTS, "mono", 1.00);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dragon_jcbsnd_device::device_rom_region() const
{
	return ROM_NAME( dragon_jcbsnd );
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(dragon_jcbsnd_device::cts_read)
{
	if (offset == 0x3eff)
		return m_ay8910->data_r();
	else
		return m_eprom->base()[offset & 0x1fff];
}

//-------------------------------------------------
//  cts_write
//-------------------------------------------------

WRITE8_MEMBER(dragon_jcbsnd_device::cts_write)
{
	if ((offset & ~1) == 0x3efe)
		m_ay8910->address_data_w(offset & 1, data);
}
