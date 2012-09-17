/***************************************************************************

  ISA 8 bit Creative Labs Game Blaster Sound Card

***************************************************************************/

#include "emu.h"
#include "isa_gblaster.h"
#include "sound/speaker.h"
#include "sound/saa1099.h"

/*
  creative labs game blaster (CMS creative music system)
  2 x saa1099 chips
  also on sound blaster 1.0
  option on sound blaster 1.5

  jumperable? normally 0x220
*/
static MACHINE_CONFIG_FRAGMENT( game_blaster_config )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("saa1099.1", SAA1099, 4772720)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("saa1099.2", SAA1099, 4772720)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static READ8_DEVICE_HANDLER( saa1099_16_r )
{
	return 0xff;
}

static WRITE8_DEVICE_HANDLER( saa1099_16_w )
{
	switch(offset)
	{
		case 0 : saa1099_control_w( device, space, offset, data ); break;
		case 1 : saa1099_data_w( device, space, offset, data ); break;
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_GAME_BLASTER = &device_creator<isa8_gblaster_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_gblaster_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( game_blaster_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_gblaster_device - constructor
//-------------------------------------------------

isa8_gblaster_device::isa8_gblaster_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_GAME_BLASTER, "Game Blaster Sound Card", tag, owner, clock),
		device_isa8_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_gblaster_device::device_start()
{
	set_isa_device();
	m_isa->install_device(subdevice("saa1099.1"), 0x0220, 0x0221, 0, 0, FUNC(saa1099_16_r), FUNC(saa1099_16_w) );
	m_isa->install_device(subdevice("saa1099.2"), 0x0222, 0x0223, 0, 0, FUNC(saa1099_16_r), FUNC(saa1099_16_w) );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_gblaster_device::device_reset()
{
}
