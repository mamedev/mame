// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/****************************************************************************
 *
 * geebee.cpp
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "geebee.h"


DEFINE_DEVICE_TYPE(GEEBEE_SOUND, geebee_sound_device, "geebee_sound", "Gee Bee Custom Sound")

geebee_sound_device::geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GEEBEE_SOUND, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_sound_latch(0),
	m_sound_signal(0),
	m_volume(0),
	m_noise(0),
	m_vcount(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void geebee_sound_device::device_start()
{
	m_decay = std::make_unique<uint16_t[]>(32768);

	for (int i = 0; i < 0x8000; i++)
		m_decay[0x7fff - i] = (int16_t) (0x7fff/exp(1.0*i/4096));

	// 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz
	m_channel = stream_alloc(0, 1, clock() / 3 / 2 / 384);

	m_volume_timer = timer_alloc(FUNC(geebee_sound_device::volume_decay_tick), this);

	save_item(NAME(m_sound_latch));
	save_item(NAME(m_sound_signal));
	save_item(NAME(m_volume));
	save_item(NAME(m_noise));
	save_item(NAME(m_vcount));
}

TIMER_CALLBACK_MEMBER(geebee_sound_device::volume_decay_tick)
{
	if (m_volume > 0)
	{
		m_channel->update();
		m_volume--;
	}
}

void geebee_sound_device::sound_w(u8 data)
{
	m_channel->update();
	m_sound_latch = data;
	m_volume = 0x7fff; // set volume
	m_noise = 0x0000; // reset noise shifter

	// faster decay enabled?
	if (m_sound_latch & 8)
	{
		/*
		 * R24 is 10k, Rb is 0, C57 is 1uF
		 * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
		 * discharge time t2 = 0.693 * (Rb) * C57 -> 0
		 * Then C33 is only charged via D6 (1N914), not discharged!
		 * Decay:
		 * discharge C33 (1uF) through R50 (22k) -> 0.14058s
		 */
		attotime period = attotime::from_hz(32768) * 14058 / 100000;
		m_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
		 * discharge only after R49 (100k) in the amplifier section,
		 * so the volume shouldn't very fast and only when the signal
		 * is gated through 6N (4066).
		 * I can only guess here that the decay should be slower,
		 * maybe half as fast?
		 */
		attotime period = attotime::from_hz(32768) * 29060 / 100000;
		m_volume_timer->adjust(period, 0, period);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void geebee_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		stream.put_int(0, sampindex, m_sound_signal, 32768);

		// 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz
		m_vcount++;

		// noise clocked with rising edge of 2V
		if ((m_vcount & 3) == 2)
		{
			// bit0 = bit0 ^ !bit10
			if ((m_noise & 1) == ((m_noise >> 10) & 1))
				m_noise = ((m_noise << 1) & 0xfffe) | 1;
			else
				m_noise = (m_noise << 1) & 0xfffe;
		}

		switch (m_sound_latch & 7)
		{
		case 0: // 4V
			m_sound_signal = (m_vcount & 0x04) ? m_decay[m_volume] : 0;
			break;
		case 1: // 8V
			m_sound_signal = (m_vcount & 0x08) ? m_decay[m_volume] : 0;
			break;
		case 2: // 16V
			m_sound_signal = (m_vcount & 0x10) ? m_decay[m_volume] : 0;
			break;
		case 3: // 32V
			m_sound_signal = (m_vcount & 0x20) ? m_decay[m_volume] : 0;
			break;
		case 4: // TONE1
			m_sound_signal = !(m_vcount & 0x01) && !(m_vcount & 0x10) ? m_decay[m_volume] : 0;
			break;
		case 5: // TONE2
			m_sound_signal = !(m_vcount & 0x02) && !(m_vcount & 0x20) ? m_decay[m_volume] : 0;
			break;
		case 6: // TONE3
			m_sound_signal = !(m_vcount & 0x04) && !(m_vcount & 0x40) ? m_decay[m_volume] : 0;
			break;
		default: // NOISE
			// QH of 74164 #4V
			m_sound_signal = (m_noise & 0x8000) ? m_decay[m_volume] : 0;
			break;
		}
	}
}
