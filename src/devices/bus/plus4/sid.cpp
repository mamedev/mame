// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 SID cartridge emulation

    http://solder.dyndns.info/cgi-bin/showdir.pl?dir=files/commodore/plus4/hardware/SID-Card

**********************************************************************/

/*

    TODO:

    - GAL16V8 dump
    - get SID clock from expansion port

*/

#include "emu.h"
#include "sid.h"

#include "speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8580_TAG     "mos8580"
#define CONTROL1_TAG    "joy1"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PLUS4_SID, plus4_sid_cartridge_device, "plus4_sid", "Plus/4 SID cartridge")


//-------------------------------------------------
//  ROM( plus4_sid )
//-------------------------------------------------

ROM_START( plus4_sid )
	ROM_REGION( 0x100, "pld", 0 )
	ROM_LOAD( "gal16v8", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *plus4_sid_cartridge_device::device_rom_region() const
{
	return ROM_NAME( plus4_sid );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void plus4_sid_cartridge_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	MOS8580(config, m_sid, XTAL(17'734'470)/20).add_route(ALL_OUTPUTS, "speaker", 1.0);

	VCS_CONTROL_PORT(config, m_joy, vcs_control_port_devices, nullptr);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  plus4_sid_cartridge_device - constructor
//-------------------------------------------------

plus4_sid_cartridge_device::plus4_sid_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PLUS4_SID, tag, owner, clock),
	device_plus4_expansion_card_interface(mconfig, *this),
	m_sid(*this, MOS8580_TAG),
	m_joy(*this, CONTROL1_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void plus4_sid_cartridge_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void plus4_sid_cartridge_device::device_reset()
{
	m_sid->reset();
}


//-------------------------------------------------
//  plus4_cd_r - cartridge data read
//-------------------------------------------------

uint8_t plus4_sid_cartridge_device::plus4_cd_r(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if ((offset >= 0xfe80 && offset < 0xfea0) || (offset >= 0xfd40 && offset < 0xfd60))
	{
		data = m_sid->read(offset & 0x1f);
	}
	else if (offset >= 0xfd80 && offset < 0xfd90)
	{
		data = m_joy->joy_r(machine().dummy_space(), 0);
	}

	return data;
}


//-------------------------------------------------
//  plus4_cd_w - cartridge data write
//-------------------------------------------------

void plus4_sid_cartridge_device::plus4_cd_w(offs_t offset, uint8_t data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if ((offset >= 0xfe80 && offset < 0xfea0) || (offset >= 0xfd40 && offset < 0xfd60))
	{
		m_sid->write(offset & 0x1f, data);
	}
}


//-------------------------------------------------
//  plus4_breset_w - buffered reset write
//-------------------------------------------------

void plus4_sid_cartridge_device::plus4_breset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		device_reset();
	}
}
