/***************************************************************************

  ISA 8 bit Adlib Sound Card

***************************************************************************/

#include "emu.h"
#include "adlib.h"
#include "sound/speaker.h"
#include "sound/3812intf.h"

#define ym3812_StdClock 3579545

static MACHINE_CONFIG_FRAGMENT( adlib_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ym3812", YM3812, ym3812_StdClock)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 3.00)
MACHINE_CONFIG_END

static READ8_DEVICE_HANDLER( ym3812_16_r )
{
	ym3812_device *ym3812 = (ym3812_device *) device;

	UINT8 retVal = 0xff;
	switch(offset)
	{
		case 0 : retVal = ym3812->status_port_r( space, offset ); break;
	}
	return retVal;
}

static WRITE8_DEVICE_HANDLER( ym3812_16_w )
{
	ym3812_device *ym3812 = (ym3812_device *) device;

	switch(offset)
	{
		case 0 : ym3812->control_port_w( space, offset, data ); break;
		case 1 : ym3812->write_port_w( space, offset, data ); break;
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

isa8_adlib_device::isa8_adlib_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, ISA8_ADLIB, "Ad Lib Sound Card", tag, owner, clock, "isa_adlib", __FILE__),
		device_isa8_card_interface( mconfig, *this )
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_adlib_device::device_start()
{
	set_isa_device();
	m_isa->install_device(subdevice("ym3812"), 0x0388, 0x0389, 0, 0, FUNC(ym3812_16_r), FUNC(ym3812_16_w) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_adlib_device::device_reset()
{
}
