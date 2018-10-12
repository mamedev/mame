// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    NEC μPD934G

    Percussion Generator

    TODO:
    - Play MUTED and ACCENTED
    - T1 input
    - 8 channels?

***************************************************************************/

#include "emu.h"
#include "upd934g.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(UPD934G, upd934g_device, "upd934g", "NEC uPD934G")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd934g_device - constructor
//-------------------------------------------------

upd934g_device::upd934g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, UPD934G, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_data_cb(*this),
	m_stream(nullptr),
	m_sample(0),
	m_ready(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd934g_device::device_start()
{
	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 4, 20000);

	// resolve callbacks
	m_data_cb.resolve_safe(0);

	// register for save states
	save_pointer(NAME(m_addr), 16);

	for (unsigned i = 0; i < 4; i++)
	{
		save_item(NAME(m_channel[i].pos), i);
		save_item(NAME(m_channel[i].playing), i);
		save_item(NAME(m_channel[i].volume), i);
	}

	save_item(NAME(m_sample));
	save_item(NAME(m_ready));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd934g_device::device_reset()
{
	m_ready = false;

	for (unsigned i = 0; i < 4; i++)
		m_channel[i].playing = -1;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void upd934g_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	for (unsigned ch = 0; ch < 4; ch++)
	{
		std::fill(&outputs[ch][0], &outputs[ch][samples], 0);

		if (m_ready && m_channel[ch].playing != -1)
		{
			uint16_t end = m_addr[m_channel[ch].playing + 1] - 1;

			for (unsigned i = 0; i < samples; i++)
			{
				int8_t raw = static_cast<int8_t>(m_data_cb(m_channel[ch].pos));
				outputs[ch][i] = raw * (m_channel[ch].volume + 1) * 64;

				if (++m_channel[ch].pos >= end)
				{
					m_channel[ch].playing = -1;
					break;
				}
			}
		}
	}
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE8_MEMBER( upd934g_device::write )
{
	switch (offset)
	{
	case 0:
		// format of data written here is:
		// 76------  command
		// --5432--  sample number
		// ------10  volume?
		m_sample = (data >> 2) & 0x0f;
		switch (data >> 6)
		{
		case 0:
			logerror("CMD STORE ADDRESS sample %x\n", m_sample);
			break;
		case 1:
			logerror("CMD PLAY sample %x (channel %d)\n", m_sample, m_sample >> 1);
			if (m_sample < 8)
			{
				m_channel[m_sample >> 1].pos = m_addr[m_sample];
				m_channel[m_sample >> 1].playing = m_sample;
				m_channel[m_sample >> 1].volume = data & 0x03;
			}
			break;
		case 2:
			logerror("CMD PLAY MUTED sample %x (channel %d)\n", m_sample, m_sample >> 1);
			break;
		case 3:
			logerror("CMD PLAY ACCENTED sample %x (channel %d)\n", m_sample, m_sample >> 1);
			break;
		}
		break;
	case 1:
		m_addr[m_sample] = (m_addr[m_sample] & 0xff00) | (data << 0);
		break;
	case 2:
		m_addr[m_sample] = (m_addr[m_sample] & 0x00ff) | (data << 8);
		logerror("  sample %x address = %04x\n", m_sample, m_addr[m_sample]);
		break;
	case 3:
		m_ready = true;
		break;
	}
}
