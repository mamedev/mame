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

#include "emu.h"
#include "lmc1992.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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
DEFINE_DEVICE_TYPE(LMC1992, lmc1992_device, "lmc1992", "LMC1992 Stereo Tone and Volume")



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
			LOG("LMC1992 Input Select : OPEN\n");
		}
		else
		{
			LOG("LMC1992 Input Select : INPUT%u\n", data);
		}
		m_input = data;
		break;

	case FUNCTION_BASS:
		LOG("LMC1992 Bass : %i dB\n", -40 + (data * 2));
		m_bass = data;
		break;

	case FUNCTION_TREBLE:
		LOG("LMC1992 Treble : %i dB\n", -40 + (data * 2));
		m_treble = data;
		break;

	case FUNCTION_VOLUME:
		LOG("LMC1992 Volume : %i dB\n", -80 + (data * 2));
		m_volume = data;
		break;

	case FUNCTION_RIGHT_FRONT_FADER:
		LOG("LMC1992 Right Front Fader : %i dB\n", -40 + (data * 2));
		m_fader_rf = data;
		break;

	case FUNCTION_LEFT_FRONT_FADER:
		LOG("LMC1992 Left Front Fader : %i dB\n", -40 + (data * 2));
		m_fader_lf = data;
		break;

	case FUNCTION_RIGHT_REAR_FADER:
		LOG("LMC1992 Right Rear Fader : %i dB\n", -40 + (data * 2));
		m_fader_rr = data;
		break;

	case FUNCTION_LEFT_REAR_FADER:
		LOG("LMC1992 Left Rear Fader : %i dB\n", -40 + (data * 2));
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

lmc1992_device::lmc1992_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LMC1992, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
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

void lmc1992_device::sound_stream_update(sound_stream &stream)
{
}


//-------------------------------------------------
//  clock_w -
//-------------------------------------------------

void lmc1992_device::clock_w(int state)
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

void lmc1992_device::data_w(int state)
{
	m_data = state;
}


//-------------------------------------------------
//  enable_w -
//-------------------------------------------------

void lmc1992_device::enable_w(int state)
{
	if ((m_enable == 0) && (state == 1))
	{
		uint8_t device_addr = (m_si & 0xc000) >> 14;
		uint8_t addr = (m_si & 0x3800) >> 11;
		uint8_t data = (m_si & 0x07e0) >> 5;

		if (device_addr == MICROWIRE_DEVICE_ADDRESS)
		{
			execute_command(addr, data);
		}
	}

	m_enable = state;
}
