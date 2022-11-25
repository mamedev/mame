// license:BSD-3-Clause
// copyright-holders:kmg
/**********************************************************************

    Nintendo Family Computer Sharp Cassette Interface AN-300SL

    An alternative to the Nintendo released Data Recorder. Unlike the
    Data Recorder, which plugs into the Famicom Keyboard via audio jacks,
    this Sharp Cassette Interface plugs directly into the expansion port
    and connects to any cassette deck via two audio jacks. The Sharp and
    Nintendo cassette interfaces are software incompatible.

    This device is used by the built-in Graphics/Note program of the
    Sharp My Computer Terebi C1 for saving/loading. Is there any other
    use for this device?

    TODO: The menu option of "play" when verifying saved data suggests
    that the unit autodetects when the play button is pressed on a
    cassette deck and signals this somehow to the Graphics/Note program.

**********************************************************************/

#include "emu.h"
#include "sharpcass.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_SHARPCASS, nes_sharpcass_device, "nes_sharpcass", "Sharp Cassette Interface AN-300SL")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_sharpcass_device::device_add_mconfig(machine_config &config)
{
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("fc_cass");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_sharpcass_device - constructor
//-------------------------------------------------

nes_sharpcass_device::nes_sharpcass_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_SHARPCASS, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_cassette(*this, "tape")
{
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_sharpcass_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (offset == 1)  // $4017
	{
		if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
		{
			if (m_cassette->input() < 0)
				ret |= 0x04;
		}
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_sharpcass_device::write(u8 data)
{
	if ((m_cassette->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
		m_cassette->output(BIT(data, 2) ? +1.0 : -1.0);
}
