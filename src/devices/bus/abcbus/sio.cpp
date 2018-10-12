// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*
ABC SIO

PCB Layout
----------

  |-------------------------------------------|
|-|                                           |
|-|                                           |
|-|                         Z80SIO        CN2 |
|-|                                           |
|-|       4.9152MHz                           |
|-|                                           |
|-|            ROM0         Z80CTC            |
|-|                                       CN1 |
|-|            ROM1                           |
  |-------------------------------------------|

Notes:
    Relevant IC's shown.

    ROM0    - Hitachi HN462716 2Kx8 EPROM "SYN 1.6"
    ROM1    - Mitsubishi MB8516 2Kx8 EPROM "T80 1.3"
    Z80SIO  - Zilog Z-80A SIO/0
    Z80CTC  - Zilog Z-80A CTC
    CN1     - DB9 serial connector
    CN2     - DB25 serial connector

*/

#include "emu.h"
#include "sio.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80CTC_TAG  "z80ctc"
#define Z80SIO_TAG  "z80sio"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC_SIO, abc_sio_device, "abcsio", "ABC SIO")


//-------------------------------------------------
//  ROM( abc_sio )
//-------------------------------------------------

ROM_START( abc_sio )
	ROM_REGION( 0x1000, "abc80", 0 )
	ROM_LOAD( "t80 1.3", 0x000, 0x800, CRC(f20ff827) SHA1(a1c4af1c374184a14872d7253d6f9e470603117f) )
	ROM_LOAD( "syn 1.6", 0x800, 0x800, CRC(7bd96b75) SHA1(d1f9b16530be28b03eeddb3f6ee4fa9e1cc9458e) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc_sio_device::device_rom_region() const
{
	return ROM_NAME( abc_sio );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc_sio_device::device_add_mconfig(machine_config &config)
{
	Z80CTC(config, m_ctc, XTAL(4'915'200));
	Z80DART(config, m_sio, 0);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_sio_device - constructor
//-------------------------------------------------

abc_sio_device::abc_sio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ABC_SIO, tag, owner, clock),
		device_abcbus_card_interface(mconfig, *this),
		m_ctc(*this, Z80CTC_TAG),
		m_sio(*this, Z80SIO_TAG),
		m_rom(*this, "abc80")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_sio_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_sio_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_sio_device::abcbus_cs(uint8_t data)
{
}


//-------------------------------------------------
//  abcbus_xmemfl -
//-------------------------------------------------

uint8_t abc_sio_device::abcbus_xmemfl(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x4000 && offset < 0x5000) // TODO where is this mapped?
	{
		data = m_rom->base()[offset & 0xfff];
	}

	return data;
}
