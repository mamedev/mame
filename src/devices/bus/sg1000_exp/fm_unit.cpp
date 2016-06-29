// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega FM Sound Unit emulation


Release data from the Sega Retro project:

  Year: 1987    Country/region: JP    Model code: FM-70

**********************************************************************/

#include "fm_unit.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SEGA_FM_UNIT = &device_creator<sega_fm_unit_device>;


static MACHINE_CONFIG_FRAGMENT( fm_config )
	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_10_738635MHz/3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, ":mono", 1.00)
MACHINE_CONFIG_END


machine_config_constructor sega_fm_unit_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fm_config );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_fm_unit_device - constructor
//-------------------------------------------------

sega_fm_unit_device::sega_fm_unit_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SEGA_FM_UNIT, "Sega FM Sound Unit", tag, owner, clock, "sega_fm_unit", __FILE__),
	device_sg1000_expansion_slot_interface(mconfig, *this),
	m_ym(*this, "ym2413"),
	m_audio_control(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_fm_unit_device::device_start()
{
	/* register for state saving */
	save_item(NAME(m_audio_control));
}


//-------------------------------------------------
//  peripheral_r - fm unit read
//-------------------------------------------------

READ8_MEMBER(sega_fm_unit_device::peripheral_r)
{
	if (offset == 2)
	{
		return m_audio_control & 0x01;
	}
	// will not be called for other offsets.
	return 0xff;
}

//-------------------------------------------------
//  peripheral_w - fm unit write
//-------------------------------------------------

WRITE8_MEMBER(sega_fm_unit_device::peripheral_w)
{
	switch (offset)
	{
		case 0: // register port
			if (m_audio_control == 0x01)
			{
				m_ym->write(space, 0, data & 0x3f);
			}
			break;
		case 1: // data port
			if (m_audio_control == 0x01)
			{
				m_ym->write(space, 1, data);
			}
			break;
		case 2: // control port
			m_audio_control = data & 0x01;
			break;
		default:
			break;
	}
}


bool sega_fm_unit_device::is_readable(UINT8 offset)
{
	return (offset == 2) ? true : false;
}


bool sega_fm_unit_device::is_writeable(UINT8 offset)
{
	return (offset <= 2) ? true : false;
}
