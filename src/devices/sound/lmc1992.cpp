// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    LMC1992 Digitally-Controlled Stereo Tone and Volume Circuit with
    Four-Channel Input-Selector emulation

**********************************************************************/

/*

    TODO:

    - inputs
    - outputs
    - bass
    - treble
    - volume
    - balance

*/

#include "lmc1992.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define MICROWIRE_DEVICE_ADDRESS    2


enum
{
	FUNCTION_INPUT_SELECT = 0,
	FUNCTION_BASS,
	FUNCTION_TREBLE,
	FUNCTION_VOLUME,
	FUNCTION_RIGHT_FRONT_FADER,
	FUNCTION_LEFT_FRONT_FADER,
	FUNCTION_RIGHT_REAR_FADER,
	FUNCTION_LEFT_REAR_FADER
};


enum
{
	INPUT_SELECT_OPEN = 0,
	INPUT_SELECT_INPUT1,
	INPUT_SELECT_INPUT2,
	INPUT_SELECT_INPUT3,
	INPUT_SELECT_INPUT4
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type LMC1992 = &device_creator<lmc1992_device>;



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  execute_command -
//-------------------------------------------------

inline void lmc1992_device::execute_command(int addr, int data)
{
	switch (addr)
	{
	case FUNCTION_INPUT_SELECT:
		if (data == INPUT_SELECT_OPEN)
		{
			if (LOG) logerror("LMC1992 '%s' Input Select : OPEN\n", tag().c_str());
		}
		else
		{
			if (LOG) logerror("LMC1992 '%s' Input Select : INPUT%u\n", tag().c_str(), data);
		}
		m_input = data;
		break;

	case FUNCTION_BASS:
		if (LOG) logerror("LMC1992 '%s' Bass : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_bass = data;
		break;

	case FUNCTION_TREBLE:
		if (LOG) logerror("LMC1992 '%s' Treble : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_treble = data;
		break;

	case FUNCTION_VOLUME:
		if (LOG) logerror("LMC1992 '%s' Volume : %i dB\n", tag().c_str(), -80 + (data * 2));
		m_volume = data;
		break;

	case FUNCTION_RIGHT_FRONT_FADER:
		if (LOG) logerror("LMC1992 '%s' Right Front Fader : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_fader_rf = data;
		break;

	case FUNCTION_LEFT_FRONT_FADER:
		if (LOG) logerror("LMC1992 '%s' Left Front Fader : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_fader_lf = data;
		break;

	case FUNCTION_RIGHT_REAR_FADER:
		if (LOG) logerror("LMC1992 '%s' Right Rear Fader : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_fader_rr = data;
		break;

	case FUNCTION_LEFT_REAR_FADER:
		if (LOG) logerror("LMC1992 '%s' Left Rear Fader : %i dB\n", tag().c_str(), -40 + (data * 2));
		m_fader_lr = data;
		break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  lmc1992_device - constructor
//-------------------------------------------------

lmc1992_device::lmc1992_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LMC1992, "LMC1992", tag, owner, clock, "lmc1992", __FILE__),
		device_sound_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lmc1992_device::device_start()
{
	// create sound streams

	// register for state saving
	save_item(NAME(m_enable));
	save_item(NAME(m_data));
	save_item(NAME(m_clk));
	save_item(NAME(m_si));
	save_item(NAME(m_input));
	save_item(NAME(m_bass));
	save_item(NAME(m_treble));
	save_item(NAME(m_volume));
	save_item(NAME(m_fader_rf));
	save_item(NAME(m_fader_lf));
	save_item(NAME(m_fader_rr));
	save_item(NAME(m_fader_lr));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void lmc1992_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}


//-------------------------------------------------
//  clock_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( lmc1992_device::clock_w )
{
	if ((m_enable == 0) && ((m_clk == 0) && (state == 1)))
	{
		m_si >>= 1;
		m_si = m_si & 0x7fff;

		if (m_data)
		{
			m_si &= 0x8000;
		}
	}

	m_clk = state;
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( lmc1992_device::data_w )
{
	m_data = state;
}


//-------------------------------------------------
//  enable_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( lmc1992_device::enable_w )
{
	if ((m_enable == 0) && (state == 1))
	{
		UINT8 device_addr = (m_si & 0xc000) >> 14;
		UINT8 addr = (m_si & 0x3800) >> 11;
		UINT8 data = (m_si & 0x07e0) >> 5;

		if (device_addr == MICROWIRE_DEVICE_ADDRESS)
		{
			execute_command(addr, data);
		}
	}

	m_enable = state;
}
