/***************************************************************************

    Videotone TVC 32/64 sound emulation

***************************************************************************/

#include "emu.h"
#include "includes/tvc.h"

// device type definition
const device_type TVC_SOUND = &device_creator<tvc_sound_device>;

//-------------------------------------------------
//  tvc_sound_device - constructor
//-------------------------------------------------

tvc_sound_device::tvc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TVC_SOUND, "TVC 64 Custom Sound", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void tvc_sound_device::device_start()
{
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate(), this );
	m_sndint_timer = timer_alloc(TIMER_SNDINT);

	// resolve callbacks
	m_sndint_func.resolve(m_sndint_cb, *this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void tvc_sound_device::device_reset()
{
	m_enabled = 0;
	m_freq    = 0;
	m_incr    = 0;
	m_signal  = 1;
	m_sndint_timer->reset();
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tvc_sound_device::device_config_complete()
{
	// inherit a copy of the static data
	const tvc_sound_interface *intf = reinterpret_cast<const tvc_sound_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<tvc_sound_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
		memset(&m_sndint_cb, 0, sizeof(m_sndint_cb));
	}
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void tvc_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_sndint_func(1);
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void tvc_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int rate = machine().sample_rate() / 2;

	if (m_enabled && m_freq)
	{
		while( samples-- > 0 )
		{
			*outputs[0]++ = m_signal * (m_volume * 0x0800);
			m_incr -= m_freq;
			while(m_incr < 0)
			{
				m_incr += rate;
				m_signal = -m_signal;
			}
		}
	}
	else
	{
		// fill output with 0 if the sound is disabled
		memset(outputs[0], 0, samples * sizeof(stream_sample_t));
	}
}



//-------------------------------------------------
//  ports write
//-------------------------------------------------

WRITE8_MEMBER(tvc_sound_device::write)
{
	m_stream->update();

	m_ports[offset] = data;

	switch(offset)
	{
		case 1:
			m_enabled = BIT(data, 4);
			// fall through

		case 0:
		{
			UINT16 pitch = (m_ports[0] | (m_ports[1]<<8)) & 0x0fff;
			m_freq = (pitch == 0x0fff) ? 0 : (int)(195312.5 / (4096 - pitch));

			if ((m_ports[1] & 0x20) && m_freq != 0)
				m_sndint_timer->adjust(attotime::from_hz(m_freq), 0, attotime::from_hz(m_freq));
			else
				m_sndint_timer->reset();

			break;
		}

		case 2:
			m_volume = (data>>2) & 0x0f;
			break;
	}
}


//-------------------------------------------------
//  tvc_sound_device::reset_divider
//-------------------------------------------------

void tvc_sound_device::reset_divider()
{
	m_stream->update();

	m_incr = 0;
	m_signal = 1;

	if (m_ports[1] & 0x20 && m_freq != 0)
		m_sndint_timer->adjust(attotime::from_hz(m_freq), 0, attotime::from_hz(m_freq));
	else
		m_sndint_timer->reset();
}
