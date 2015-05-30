// license:???
// copyright-holders:Derrick Renaud, Couriersud
#include "emu.h"
#include "flt_rc.h"


// device type definition
const device_type FILTER_RC = &device_creator<filter_rc_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  filter_rc_device - constructor
//-------------------------------------------------

filter_rc_device::filter_rc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, FILTER_RC, "RC Filter", tag, owner, clock, "filter_rc", __FILE__),
		device_sound_interface(mconfig, *this),
		m_stream(NULL),
		m_k(0),
		m_memory(0),
		m_type(FLT_RC_LOWPASS),
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
		case FLT_RC_LOWPASS:
			while (samples--)
			{
				memory += ((*src++ - memory) * m_k) / 0x10000;
				*dst++ = memory;
			}
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
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
		case FLT_RC_LOWPASS:
			if (m_C == 0.0)
			{
				/* filter disabled */
				m_k = 0x10000;
				return;
			}
			Req = (m_R1 * (m_R2 + m_R3)) / (m_R1 + m_R2 + m_R3);
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
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


void filter_rc_device::filter_rc_set_RC(int type, double R1, double R2, double R3, double C)
{
	m_stream->update();
	m_type = type;
	m_R1 = R1;
	m_R2 = R2;
	m_R3 = R3;
	m_C = C;
	recalc();
}

void filter_rc_device::static_set_rc(device_t &device, int type, double R1, double R2, double R3, double C)
{
	downcast<filter_rc_device &>(device).m_type = type;
	downcast<filter_rc_device &>(device).m_R1 = R1;
	downcast<filter_rc_device &>(device).m_R2 = R2;
	downcast<filter_rc_device &>(device).m_R3 = R3;
	downcast<filter_rc_device &>(device).m_C = C;
}
