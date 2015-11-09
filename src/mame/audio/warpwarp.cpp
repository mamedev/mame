// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/****************************************************************************
 *
 * warpwarp.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "audio/warpwarp.h"

#define CLOCK_16H   (18432000/3/2/16)
#define CLOCK_1V    (18432000/3/2/384)


const device_type WARPWARP = &device_creator<warpwarp_sound_device>;

warpwarp_sound_device::warpwarp_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WARPWARP, "Warp Warp Audio Custom", tag, owner, clock, "warpwarp_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_decay(NULL),
		m_channel(NULL),
		m_sound_latch(0),
		m_music1_latch(0),
		m_music2_latch(0),
		m_sound_signal(0),
		m_sound_volume(0),
		m_sound_volume_timer(NULL),
		m_music_signal(0),
		m_music_volume(0),
		m_music_volume_timer(NULL),
		m_noise(0),
		m_vcarry(0),
		m_vcount(0),
		m_mcarry(0),
		m_mcount(0)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void warpwarp_sound_device::device_start()
{
	m_decay = auto_alloc_array(machine(), INT16, 32768);

	for (int i = 0; i < 0x8000; i++)
		m_decay[0x7fff - i] = (INT16) (0x7fff/exp(1.0*i/4096));

	m_channel = machine().sound().stream_alloc(*this, 0, 1, CLOCK_16H);

	m_sound_volume_timer = timer_alloc(TIMER_SOUND_VOLUME_DECAY);
	m_music_volume_timer = timer_alloc(TIMER_MUSIC_VOLUME_DECAY);

	save_item(NAME(m_sound_latch));
	save_item(NAME(m_music1_latch));
	save_item(NAME(m_music2_latch));
	save_item(NAME(m_sound_signal));
	save_item(NAME(m_sound_volume));
	save_item(NAME(m_music_signal));
	save_item(NAME(m_music_volume));
	save_item(NAME(m_noise));
	save_item(NAME(m_vcarry));
	save_item(NAME(m_vcount));
	save_item(NAME(m_mcarry));
	save_item(NAME(m_mcount));
}

void warpwarp_sound_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_SOUND_VOLUME_DECAY:
			if (--m_sound_volume < 0)
				m_sound_volume = 0;
			break;

		case TIMER_MUSIC_VOLUME_DECAY:
			if (--m_music_volume < 0)
				m_music_volume = 0;
			break;

		default:
			assert_always(FALSE, "Unknown id in warpwarp_sound_device::device_timer");
	}
}

WRITE8_MEMBER( warpwarp_sound_device::sound_w )
{
	m_channel->update();
	m_sound_latch = data & 0x0f;
	m_sound_volume = 0x7fff; /* set sound_volume */
	m_noise = 0x0000;  /* reset noise shifter */

	/* faster decay enabled? */
	if( m_sound_latch & 8 )
	{
		/*
		 * R85(?) is 10k, Rb is 0, C92 is 1uF
		 * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
		 * discharge time t2 = 0.693 * (Rb) * C57 -> 0
		 * C90(?) is only charged via D17 (1N914), no discharge!
		 * Decay:
		 * discharge C90(?) (1uF) through R13||R14 (22k||47k)
		 * 0.639 * 15k * 1uF -> 0.9585s
		 */
		attotime period = attotime::from_hz(32768) * 95850 / 100000;
		m_sound_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
		 * discharge only after R93 (100k) and through the 10k
		 * potentiometerin the amplifier section.
		 * 0.639 * 110k * 1uF -> 7.0290s
		 * ...but this is not very realistic for the game sound :(
		 * maybe there _is_ a discharge through the diode D17?
		 */
		//attotime period = attotime::from_hz(32768) * 702900 / 100000;
		attotime period = attotime::from_hz(32768) * 191700 / 100000;
		m_sound_volume_timer->adjust(period, 0, period);
	}
}

WRITE8_MEMBER( warpwarp_sound_device::music1_w )
{
	m_channel->update();
	m_music1_latch = data & 0x3f;
}

WRITE8_MEMBER( warpwarp_sound_device::music2_w )
{
	m_channel->update();
	m_music2_latch = data & 0x3f;
	m_music_volume = 0x7fff;
	/* fast decay enabled? */
	if( m_music2_latch & 0x10 )
	{
		/*
		 * Ra (R83?) is 10k, Rb is 0, C92 is 1uF
		 * charge time t1 = 0.693 * (Ra + Rb) * C -> 0.22176s
		 * discharge time is (nearly) zero, because Rb is zero
		 * C95(?) is only charged via D17, not discharged!
		 * Decay:
		 * discharge C95(?) (10uF) through R13||R14 (22k||47k)
		 * 0.639 * 15k * 10uF -> 9.585s
		 * ...I'm sure this is off by one number of magnitude :/
		 */
		attotime period = attotime::from_hz(32768) * 95850 / 100000;
		m_music_volume_timer->adjust(period, 0, period);
	}
	else
	{
		/*
		 * discharge through R14 (47k),
		 * discharge C95(?) (10uF) through R14 (47k)
		 * 0.639 * 47k * 10uF -> 30.033s
		 */
		//attotime period = attotime::from_hz(32768) * 3003300 / 100000;
		attotime period = attotime::from_hz(32768) * 300330 / 100000;
		m_music_volume_timer->adjust(period, 0, period);
	}

}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void warpwarp_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
	{
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = (m_sound_signal + m_music_signal) / 2;

		/*
		 * The music signal is selected at a rate of 2H (1.536MHz) from the
		 * four bits of a 4 bit binary counter which is clocked with 16H,
		 * which is 192kHz, and is divided by 4 times (64 - music1_latch).
		 *  0 = 256 steps -> 750 Hz
		 *  1 = 252 steps -> 761.9 Hz
		 * ...
		 * 32 = 128 steps -> 1500 Hz
		 * ...
		 * 48 =  64 steps -> 3000 Hz
		 * ...
		 * 63 =   4 steps -> 48 kHz
		 */

		m_mcarry -= CLOCK_16H / (4 * (64 - m_music1_latch));
		while( m_mcarry < 0 )
		{
			m_mcarry += CLOCK_16H;
			m_mcount++;
			m_music_signal = (m_mcount & ~m_music2_latch & 15) ? m_decay[m_music_volume] : 0;
			/* override by noise gate? */
			if( (m_music2_latch & 32) && (m_noise & 0x8000) )
				m_music_signal = m_decay[m_music_volume];
		}

		/* clock 1V = 8kHz */
		m_vcarry -= CLOCK_1V;
		while (m_vcarry < 0)
		{
			m_vcarry += CLOCK_16H;
			m_vcount++;

			/* noise is clocked with raising edge of 2V */
			if ((m_vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((m_noise & 1) == ((m_noise >> 10) & 1))
					m_noise = (m_noise << 1) | 1;
				else
					m_noise = m_noise << 1;
			}

			switch (m_sound_latch & 7)
			{
			case 0: /* 4V */
				m_sound_signal = (m_vcount & 0x04) ? m_decay[m_sound_volume] : 0;
				break;
			case 1: /* 8V */
				m_sound_signal = (m_vcount & 0x08) ? m_decay[m_sound_volume] : 0;
				break;
			case 2: /* 16V */
				m_sound_signal = (m_vcount & 0x10) ? m_decay[m_sound_volume] : 0;
				break;
			case 3: /* 32V */
				m_sound_signal = (m_vcount & 0x20) ? m_decay[m_sound_volume] : 0;
				break;
			case 4: /* TONE1 */
				m_sound_signal = !(m_vcount & 0x01) && !(m_vcount & 0x10) ? m_decay[m_sound_volume] : 0;
				break;
			case 5: /* TONE2 */
				m_sound_signal = !(m_vcount & 0x02) && !(m_vcount & 0x20) ? m_decay[m_sound_volume] : 0;
				break;
			case 6: /* TONE3 */
				m_sound_signal = !(m_vcount & 0x04) && !(m_vcount & 0x40) ? m_decay[m_sound_volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				m_sound_signal = (m_noise & 0x8000) ? m_decay[m_sound_volume] : 0;
			}

		}
	}
}
