// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    BMKAX25/AMTOR for G6WHK

    (c) 1990 Grosvenor Software

     Left: AX25  EXEC 49152
    Right: AMTOR EXEC 49152

    Cartridge contains a single 32K ROM and a switch to select either AX25
    or AMTOR, each taking 16K.

***************************************************************************/

#include "emu.h"
#include "dragon_amtor.h"


//-------------------------------------------------
//  ROM( amtor )
//-------------------------------------------------

ROM_START( amtor )
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	// this region is filled by cococart_slot_device::call_load()
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(DRAGON_AMTOR, dragon_amtor_device, "dragon_amtor", "Dragon Amtor Cartridge")

//-------------------------------------------------
//  INPUT_PORTS( amtor )
//-------------------------------------------------

static INPUT_PORTS_START( amtor )
	PORT_START("SWITCH")
	PORT_CONFNAME( 0x01, 0x00, "Switch" )
	PORT_CONFSETTING(    0x00, "AMTOR")
	PORT_CONFSETTING(    0x01, "AX25")
INPUT_PORTS_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dragon_amtor_device - constructor
//-------------------------------------------------

dragon_amtor_device::dragon_amtor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DRAGON_AMTOR, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_eprom(*this, "eprom")
	, m_switch(*this, "SWITCH")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_amtor_device::device_start()
{
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor dragon_amtor_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( amtor );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dragon_amtor_device::device_rom_region() const
{
	return ROM_NAME( amtor );
}


//-------------------------------------------------
//  dragon_amtor_device::get_cart_base
//-------------------------------------------------

uint8_t* dragon_amtor_device::get_cart_base()
{
	return m_eprom->base();
}

//-------------------------------------------------
//  dragon_amtor_device::get_cart_memregion
//-------------------------------------------------

memory_region* dragon_amtor_device::get_cart_memregion()
{
	return m_eprom;
}

//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(dragon_amtor_device::cts_read)
{
	offset = bitswap<16>(offset, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 0, 1);

	return bitswap<8>(m_eprom->base()[(m_switch->read() << 14) | offset], 3, 2, 1, 0, 7, 6, 5, 4 );
}
