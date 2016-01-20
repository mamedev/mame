// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 8 bit Adlib Sound Card

***************************************************************************/

#include "emu.h"
#include "adlib.h"
#include "sound/speaker.h"

#define ym3812_StdClock 3579545

static MACHINE_CONFIG_FRAGMENT( adlib_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym3812", YM3812, ym3812_StdClock)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 3.00)
MACHINE_CONFIG_END

READ8_MEMBER( isa8_adlib_device::ym3812_16_r )
{
	UINT8 retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = m_ym3812->status_port_r( space, offset ); break;
	}
	return retVal;
}

WRITE8_MEMBER( isa8_adlib_device::ym3812_16_w )
{
	switch(offset)
	{
		case 0 : m_ym3812->control_port_w( space, offset, data ); break;
		case 1 : m_ym3812->write_port_w( space, offset, data ); break;
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_ADLIB = &device_creator<isa8_adlib_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_adlib_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adlib_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_adlib_device - constructor
//-------------------------------------------------

isa8_adlib_device::isa8_adlib_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_ADLIB, "Ad Lib Sound Card", tag, owner, clock, "isa_adlib", __FILE__),
		device_isa8_card_interface( mconfig, *this ),
		m_ym3812(*this, "ym3812")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_adlib_device::device_start()
{
	set_isa_device();
	m_isa->install_device(0x0388, 0x0389, 0, 0, read8_delegate( FUNC(isa8_adlib_device::ym3812_16_r), this ), write8_delegate( FUNC(isa8_adlib_device::ym3812_16_w), this ) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_adlib_device::device_reset()
{
}
