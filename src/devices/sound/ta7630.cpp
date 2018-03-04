// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    TA7630P

    Toshiba Dual. Volume / Balance / Tone (Bass/Treble)

    A set of discrete filters that applies to sound chip outputs.
    According to the datasheet, two channels are outputted from here after it applies
    all of the filters

    TODO:
    - mostly a placeholder, needs a way to read from sound chips and output
      back with filters enabled;
    - filters balance/bass/treble;

***************************************************************************/

#include "emu.h"
#include "ta7630.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(TA7630, ta7630_device, "ta7630", "Toshiba TA7630P")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ta7630_device - constructor
//-------------------------------------------------

ta7630_device::ta7630_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TA7630, tag, owner, clock)
//        ,device_sound_interface(mconfig, *this)
{
}




//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ta7630_device::device_start()
{
	int i;

	double db           = 0.0;
	double db_step      = 1.50; /* 1.50 dB step (at least, maybe more) */
	double db_step_inc  = 0.125;
	for (i = 0; i < 16; i++)
	{
		double max = 100.0 / pow(10.0, db/20.0 );
		m_vol_ctrl[15 - i] = max / 100.0;
		//m_vol_ctrl[i] = max;
		/*logerror("vol_ctrl[%x] = %i (%f dB)\n", 15 - i, m_vol_ctrl[15 - i], db);*/
		db += db_step;
		db_step += db_step_inc;
	}

	save_item(NAME(m_vol_ctrl));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ta7630_device::device_reset()
{

}


//**************************************************************************
//  filter setters
//**************************************************************************

void ta7630_device::set_device_volume(device_sound_interface *device,uint8_t value)
{
	device->set_output_gain(ALL_OUTPUTS,m_vol_ctrl[value & 0xf]);
}

//  TODO: Most Taito implementations uses this, is it correct?
void ta7630_device::set_channel_volume(device_sound_interface *device, uint8_t ch,uint8_t value)
{
	device->set_output_gain(ch,m_vol_ctrl[value & 0xf]);
}


