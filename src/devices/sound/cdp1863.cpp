// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1863 CMOS 8-Bit Programmable Frequency Generator emulation

**********************************************************************/

/*

    TODO:

    - what happens if you connect both clocks?

*/

#include "emu.h"
#include "cdp1863.h"

//#define VERBOSE 1
#include "logmacro.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define CDP1863_DEFAULT_LATCH   0x35



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(CDP1863, cdp1863_device, "cdp1863", "RCA CDP1863")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1863_device - constructor
//-------------------------------------------------

cdp1863_device::cdp1863_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDP1863, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_clock1(clock)
	, m_clock2(0)
	, m_oe(0)
	, m_latch(0)
	, m_signal(0)
	, m_incr(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1863_device::device_start()
{
	// create sound stream
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);

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

void cdp1863_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t signal = m_signal;

	if (m_oe)
	{
		double frequency;
		int rate = stream.sample_rate() / 2;

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
			signal = -1.0;
		}
		else
		{
			signal = 1.0;
		}

		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			stream.put(0, sampindex, signal);
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
//  oe_w - output enable write
//-------------------------------------------------

void cdp1863_device::oe_w(int state)
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
