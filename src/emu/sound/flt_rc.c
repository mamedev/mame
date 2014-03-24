#include "emu.h"
#include "flt_rc.h"


const flt_rc_config flt_rc_ac_default = {FLT_RC_AC, 10000, 0, 0, CAP_U(1)};


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
		m_type(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_rc_device::device_start()
{
	const flt_rc_config *conf = (const flt_rc_config *)static_config();

	m_stream = stream_alloc(1, 1, machine().sample_rate());
	if (conf)
		set_RC_info(conf->type, conf->R1, conf->R2, conf->R3, conf->C);
	else
		set_RC_info(FLT_RC_LOWPASS, 1, 1, 1, 0);
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


void filter_rc_device::set_RC_info(int type, double R1, double R2, double R3, double C)
{
	double Req;

	m_type = type;

	switch (m_type)
	{
		case FLT_RC_LOWPASS:
			if (C == 0.0)
			{
				/* filter disabled */
				m_k = 0x10000;
				return;
			}
			Req = (R1 * (R2 + R3)) / (R1 + R2 + R3);
			break;
		case FLT_RC_HIGHPASS:
		case FLT_RC_AC:
			if (C == 0.0)
			{
				/* filter disabled */
				m_k = 0x0;
				m_memory = 0x0;
				return;
			}
			Req = R1;
			break;
		default:
			fatalerror("filter_rc_setRC: Wrong filter type %d\n", m_type);
	}

	/* Cut Frequency = 1/(2*Pi*Req*C) */
	/* k = (1-(EXP(-TIMEDELTA/RC)))    */
	m_k = 0x10000 - 0x10000 * (exp(-1 / (Req * C) / machine().sample_rate()));
}


void filter_rc_device::filter_rc_set_RC(int type, double R1, double R2, double R3, double C)
{
	m_stream->update();
	set_RC_info(type, R1, R2, R3, C);
}
