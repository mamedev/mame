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
		m_type(LOWPASS),
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
	m_stream = stream_alloc(1, 1, machine().sample_rate());
	recalc();

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

void filter_rc_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];
	int memory = m_memory;

	switch (m_type)
	{
		case LOWPASS:
			while (samples--)
			{
				memory += ((*src++ - memory) * m_k) / 0x10000;
				*dst++ = memory;
			}
			break;
		case HIGHPASS:
		case AC:
			while (samples--)
			{
				*dst++ = *src - memory;
				memory += ((*src++ - memory) * m_k) / 0x10000;
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
		case LOWPASS:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 0x10000;
				return;
			}
			Req = (m_R1 * (m_R2 + m_R3)) / (m_R1 + m_R2 + m_R3);
			break;
		case HIGHPASS:
		case AC:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 0x0;
				m_memory = 0x0;
				return;
			}
			Req = m_R1;
			break;
		default:
			fatalerror("filter_rc_setRC: Wrong filter type %d\n", m_type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	m_k = 0x10000 - 0x10000 * (exp(-1 / (Req * m_C) / machine().sample_rate()));
}
