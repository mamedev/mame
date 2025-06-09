// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Videotone TVC 32/64 sound emulation

***************************************************************************/

#include "emu.h"
#include "tvc_a.h"

// device type definition
DEFINE_DEVICE_TYPE(TVC_SOUND, tvc_sound_device, "tvc_sound", "TVC 64 Custom Sound")

//-------------------------------------------------
//  tvc_sound_device - constructor
//-------------------------------------------------

tvc_sound_device::tvc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TVC_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_write_sndint(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void tvc_sound_device::device_start()
{
	m_stream = stream_alloc(0, 1, machine().sample_rate());
	m_sndint_timer = timer_alloc(FUNC(tvc_sound_device::trigger_int), this);
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
//  trigger_int
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(tvc_sound_device::trigger_int)
{
	m_write_sndint(1);
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void tvc_sound_device::sound_stream_update(sound_stream &stream)
{
	int rate = stream.sample_rate() / 2;
	if (m_enabled && m_freq)
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
		{
			stream.put_int(0, sampindex, m_signal * m_volume, 32768 / 0x0800);
			m_incr -= m_freq;
			while(m_incr < 0)
			{
				m_incr += rate;
				m_signal = -m_signal;
			}
		}
	}
}



//-------------------------------------------------
//  ports write
//-------------------------------------------------

void tvc_sound_device::write(offs_t offset, uint8_t data)
{
	m_stream->update();

	m_ports[offset] = data;

	switch(offset)
	{
		case 1:
			m_enabled = BIT(data, 4);
			[[fallthrough]];

		case 0:
		{
			uint16_t pitch = (m_ports[0] | (m_ports[1]<<8)) & 0x0fff;
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
