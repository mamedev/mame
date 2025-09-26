// license:BSD-3-Clause
// copyright-holders:Derrick Renaud, Couriersud
#include "emu.h"
#include "flt_rc.h"


// device type definition
DEFINE_DEVICE_TYPE(FILTER_RC, filter_rc_device, "filter_rc", "RC Filter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  filter_rc_device - constructor
//-------------------------------------------------

filter_rc_device::filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FILTER_RC, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_k(0),
		m_memory(0),
		m_type(LOWPASS_3R),
		m_last_sample_rate(0),
		m_R1(1),
		m_R2(1),
		m_R3(1),
		m_C(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_rc_device::device_start()
{
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_last_sample_rate = 0;

	save_item(NAME(m_k));
	save_item(NAME(m_memory));
	save_item(NAME(m_type));
	save_item(NAME(m_R1));
	save_item(NAME(m_R2));
	save_item(NAME(m_R3));
	save_item(NAME(m_C));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_rc_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t memory = m_memory;

	if (m_last_sample_rate != stream.sample_rate())
	{
		recalc();
		m_last_sample_rate = stream.sample_rate();
	}

	switch (m_type)
	{
		case LOWPASS_3R:
		case LOWPASS:
			for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			{
				memory += (stream.get(0, sampindex) - memory) * m_k;
				stream.put(0, sampindex, memory);
			}
			break;
		case HIGHPASS:
		case AC:
			for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			{
				stream.put(0, sampindex, stream.get(0, sampindex) - memory);
				memory += (stream.get(0, sampindex) - memory) * m_k;
			}
			break;
	}
	m_memory = memory;
}


void filter_rc_device::recalc()
{
	double Req;

	switch (m_type)
	{
		case LOWPASS_3R:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 1.0;
				m_memory = 0;
				return;
			}
			Req = (m_R1 * (m_R2 + m_R3)) / (m_R1 + m_R2 + m_R3);
			break;
		case LOWPASS:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 1.0;
				m_memory = 0;
				return;
			}
			Req = m_R1;
			break;
		case HIGHPASS:
		case AC:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 0;
				m_memory = 0;
				return;
			}
			Req = m_R1;
			break;
		default:
			fatalerror("filter_rc_setRC: Wrong filter type %d\n", m_type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	m_k = 1.0 - exp(-1 / (Req * m_C) / m_stream->sample_rate());
}
