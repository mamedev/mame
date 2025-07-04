// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#include "emu.h"
#include "tms3615.h"

const int tms3615_device::divisor[TMS3615_TONES] = { 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239 };


// device type definition
DEFINE_DEVICE_TYPE(TMS3615, tms3615_device, "tms3615", "TMS3615")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms3615_device - constructor
//-------------------------------------------------

tms3615_device::tms3615_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TMS3615, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_channel(nullptr)
	, m_samplerate(0)
	, m_basefreq(0)
	, m_output8(0)
	, m_output16(0)
	, m_enable(0)
{
	std::fill(std::begin(m_counter8), std::end(m_counter8), 0);
	std::fill(std::begin(m_counter16), std::end(m_counter16), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms3615_device::device_start()
{
	m_channel = stream_alloc(0, 2, clock()/8);
	m_samplerate = clock()/8;
	m_basefreq = clock();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms3615_device::sound_stream_update(sound_stream &stream)
{
	int samplerate = stream.sample_rate();

	constexpr sound_stream::sample_t VMAX = 1.0f / sound_stream::sample_t(TMS3615_TONES);
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		sound_stream::sample_t sum8 = 0, sum16 = 0;

		for (int tone = 0; tone < TMS3615_TONES; tone++)
		{
			// 8'

			m_counter8[tone] -= (m_basefreq / divisor[tone]);

			while( m_counter8[tone] <= 0 )
			{
				m_counter8[tone] += samplerate;
				m_output8 ^= 1 << tone;
			}

			if (m_output8 & m_enable & (1 << tone))
			{
				sum8 += VMAX;
			}

			// 16'

			m_counter16[tone] -= (m_basefreq / divisor[tone] / 2);

			while( m_counter16[tone] <= 0 )
			{
				m_counter16[tone] += samplerate;
				m_output16 ^= 1 << tone;
			}

			if (m_output16 & m_enable & (1 << tone))
			{
				sum16 += VMAX;
			}
		}

		stream.put(FOOTAGE_8, sampindex, sum8);
		stream.put(FOOTAGE_16, sampindex, sum16);
	}
}


void tms3615_device::enable_w(int enable)
{
	m_enable = enable;
}
