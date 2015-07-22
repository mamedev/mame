// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/****************************************************************************
 *
 * geebee.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "audio/warpwarp.h"


const device_type GEEBEE = &device_creator<geebee_sound_device>;

geebee_sound_device::geebee_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GEEBEE, "Gee Bee Audio Custom", tag, owner, clock, "geebee_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_decay(NULL),
		m_channel(NULL),
		m_sound_latch(0),
		m_sound_signal(0),
		m_volume(0),
		m_volume_timer(NULL),
		m_noise(0),
		m_vcount(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void geebee_sound_device::device_start()
{
	m_decay = auto_alloc_array(machine(), UINT16, 32768);

	for (int i = 0; i < 0x8000; i++)
		m_decay[0x7fff - i] = (INT16) (0x7fff/exp(1.0*i/4096));

	/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
	m_channel = machine().sound().stream_alloc(*this, 0, 1, 18432000 / 3 / 2 / 384);
	m_vcount = 0;

	m_volume_timer = timer_alloc(TIMER_VOLUME_DECAY);

	save_item(NAME(m_sound_latch));
	save_item(NAME(m_sound_signal));
	save_item(NAME(m_volume));
	save_item(NAME(m_noise));
	save_item(NAME(m_vcount));
}

void geebee_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_VOLUME_DECAY:
			if (--m_volume < 0)
				m_volume = 0;
			break;

		default:
			assert_always(FALSE, "Unknown id in geebee_device::device_timer");
	}
}

WRITE8_MEMBER( geebee_sound_device::sound_w )
{
	m_channel->update();
	m_sound_latch = data;
	m_volume = 0x7fff; /* set volume */
	m_noise = 0x0000;  /* reset noise shifter */
	/* faster decay enabled? */
	if( m_sound_latch & 8 )
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

void geebee_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
	{
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = m_sound_signal;
		/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
		{
			m_vcount++;
			/* noise clocked with raising edge of 2V */
			if ((m_vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((m_noise & 1) == ((m_noise >> 10) & 1))
					m_noise = ((m_noise << 1) & 0xfffe) | 1;
				else
					m_noise = (m_noise << 1) & 0xfffe;
			}
			switch (m_sound_latch & 7)
			{
			case 0: /* 4V */
				m_sound_signal = (m_vcount & 0x04) ? m_decay[m_volume] : 0;
				break;
			case 1: /* 8V */
				m_sound_signal = (m_vcount & 0x08) ? m_decay[m_volume] : 0;
				break;
			case 2: /* 16V */
				m_sound_signal = (m_vcount & 0x10) ? m_decay[m_volume] : 0;
				break;
			case 3: /* 32V */
				m_sound_signal = (m_vcount & 0x20) ? m_decay[m_volume] : 0;
				break;
			case 4: /* TONE1 */
				m_sound_signal = !(m_vcount & 0x01) && !(m_vcount & 0x10) ? m_decay[m_volume] : 0;
				break;
			case 5: /* TONE2 */
				m_sound_signal = !(m_vcount & 0x02) && !(m_vcount & 0x20) ? m_decay[m_volume] : 0;
				break;
			case 6: /* TONE3 */
				m_sound_signal = !(m_vcount & 0x04) && !(m_vcount & 0x40) ? m_decay[m_volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				m_sound_signal = (m_noise & 0x8000) ? m_decay[m_volume] : 0;
			}

		}
	}
}
