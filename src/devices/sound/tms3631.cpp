// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "tms3631.h"

#define LIVE_AUDIO_VIEW 0

//#define VERBOSE 1
#include "logmacro.h"
#include <cstring>

DEFINE_DEVICE_TYPE(TMS3631, tms3631_device, "tms3631", "TMS3631")


tms3631_device::tms3631_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TMS3631, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_samplerate(0)
	, m_basefreq(0)
	, m_enable(0)
{
//	std::fill(std::begin(m_counter8), std::end(m_counter8), 0);
	std::fill(std::begin(m_channel_data), std::end(m_channel_data), 0);
}


void tms3631_device::device_start()
{
	m_stream = stream_alloc(0, 8, clock() / 8);
	m_samplerate = clock() / 8;
	m_basefreq = clock();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms3631_device::sound_stream_update(sound_stream &stream)
{
//	int samplerate = stream.sample_rate();

	for (int i = 0; i < 8; i++)
		stream.fill(i, 0);
}

// TODO: external to the chip(s)?
void tms3631_device::enable_w(offs_t offset, u8 data)
{
	if (offset)
		popmessage("TMS3631: select with offset (RI105?)");
	m_enable = data;
}

void tms3631_device::data_w(u8 data)
{
	const int ce_state = BIT(data, 7);

	if (!m_ce && ce_state)
	{
		m_program_enable = true;
		m_channel_select = 0;
	}
	else if (m_ce && !ce_state)
	{
		m_program_enable = false;
	}

	if (m_program_enable)
	{
		const int wclk_state = BIT(data, 6);

		if (!m_wclk && wclk_state)
		{
			m_channel_data[m_channel_select] = data & 0x3f;
			m_stream->update();
			if (LIVE_AUDIO_VIEW)
				popmessage(print_audio_state());
		}
		else if (m_wclk && !wclk_state)
		{
			m_channel_select ++;
			m_channel_select &= 7;
		}

		m_wclk = wclk_state;
	}
	m_ce = ce_state;
}

std::string tms3631_device::print_audio_state()
{
	std::ostringstream outbuffer;

	for (int i = 0; i < 8; i++)
	{
		util::stream_format(outbuffer, "%02x ", m_channel_data[i]);
	}

	return outbuffer.str();
}

