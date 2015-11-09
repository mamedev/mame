// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1863 CMOS 8-Bit Programmable Frequency Generator emulation

**********************************************************************/

/*

    TODO:

    - what happens if you connect both clocks?

*/

#include "cdp1863.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define CDP1863_DEFAULT_LATCH   0x35



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type CDP1863 = &device_creator<cdp1863_device>;



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1863_device - constructor
//-------------------------------------------------

cdp1863_device::cdp1863_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, CDP1863, "CDP1863", tag, owner, clock, "cdp1863", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_clock1(clock),
		m_clock2(0)
{
}


//-------------------------------------------------
//  static_set_clock2 - configuration helper
//-------------------------------------------------

void cdp1863_device::static_set_clock2(device_t &device, int clock2)
{
	cdp1863_device &cdp1863 = downcast<cdp1863_device &>(device);

	cdp1863.m_clock2 = clock2;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1863_device::device_start()
{
	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());

	// register for state saving
	save_item(NAME(m_clock1));
	save_item(NAME(m_clock2));
	save_item(NAME(m_oe));
	save_item(NAME(m_latch));
	save_item(NAME(m_signal));
	save_item(NAME(m_incr));
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void cdp1863_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	INT16 signal = m_signal;
	stream_sample_t *buffer = outputs[0];

	memset( buffer, 0, samples * sizeof(*buffer) );

	if (m_oe)
	{
		double frequency;
		int rate = machine().sample_rate() / 2;

		// get progress through wave
		int incr = m_incr;

		if (m_clock1 > 0)
		{
			// CLK1 is pre-divided by 4
			frequency = m_clock1 / 4 / (m_latch + 1) / 2;
		}
		else
		{
			// CLK2 is pre-divided by 8
			frequency = m_clock2 / 8 / (m_latch + 1) / 2;
		}

		if (signal < 0)
		{
			signal = -0x7fff;
		}
		else
		{
			signal = 0x7fff;
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		// store progress through wave
		m_incr = incr;
		m_signal = signal;
	}
}


//-------------------------------------------------
//  str_w - latch write
//-------------------------------------------------

WRITE8_MEMBER( cdp1863_device::str_w )
{
	m_latch = data;
}


//-------------------------------------------------
//  str_w - latch write
//-------------------------------------------------

void cdp1863_device::str_w(UINT8 data)
{
	m_latch = data;
}


//-------------------------------------------------
//  oe_w - output enable write
//-------------------------------------------------

WRITE_LINE_MEMBER( cdp1863_device::oe_w )
{
	m_oe = state;
}


//-------------------------------------------------
//  set_clk1 - set clock 1
//-------------------------------------------------

void cdp1863_device::set_clk1(int clock)
{
	m_clock1 = clock;
}


//-------------------------------------------------
//  set_clk2 - set clock 2
//-------------------------------------------------

void cdp1863_device::set_clk2(int clock)
{
	m_clock2 = clock;
}
