// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    NEC μPD934G

    Percussion Generator

    TODO:
    - Correct MUTED and ACCENTED (currently just changes volume)
    - T1 input

***************************************************************************/

#include "emu.h"
#include "upd934g.h"

#define VERBOSE (0)
#include "logmacro.h"

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
	device_rom_interface(mconfig, *this),
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
	m_stream = stream_alloc(0, 4, 20000);

	// register for save states
	save_item(NAME(m_addr));
	save_item(NAME(m_valid));

	save_item(STRUCT_MEMBER(m_channel, pos));
	save_item(STRUCT_MEMBER(m_channel, playing));
	save_item(STRUCT_MEMBER(m_channel, effect));

	save_item(NAME(m_sample));
	save_item(NAME(m_ready));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd934g_device::device_reset()
{
	m_ready = false;

	for (unsigned i = 0; i < 16; i++)
		m_addr[i] = m_valid[i] = 0;

	for (unsigned i = 0; i < 4; i++)
	{
		m_channel[i].pos = 0;
		m_channel[i].playing = -1;
		m_channel[i].effect = 0;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void upd934g_device::sound_stream_update(sound_stream &stream)
{
	for (unsigned ch = 0; ch < 4; ch++)
	{
		if (m_ready && m_channel[ch].playing != -1)
		{
			uint16_t end = m_addr[(m_channel[ch].playing + 1) & 0xf] - 1;

			for (unsigned i = 0; i < stream.samples(); i++)
			{
				int16_t raw = static_cast<int8_t>(read_byte(m_channel[ch].pos)) * 4;

				// normal, muted, accented
				// TODO: cps2000 regularly sets effect bits to 0 instead of 2 - are these the same?
				const double adjust[] = { 0.4, 0.7, 0.4, 1.0 };
				raw *= adjust[m_channel[ch].effect];

				stream.put_int(ch, i, raw, 32768 / 64);

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

void upd934g_device::write(offs_t offset, uint8_t data)
{
	switch (offset & 3)
	{
	case 0:
		// format of data written here is:
		// 76------  effect
		// --5432--  sample number
		// ------10  output channel
		m_sample = (data >> 2) & 0x0f;

		// don't play a sample unless we're sure it's actually supposed to play:
		// - all models write this with effect=0 to select which sample to set the address of
		// - cps2000 writes this with effect=0 to play muted(?) samples
		// - cps2000 also writes this with effect=1 and sample=f on boot even though there are only 12 valid samples
		if (m_valid[m_sample])
		{
			const u8 ch = (data & 3) ^ 2; // effective order seems to be "2, 3, 0, 1"
			LOG("CMD PLAY sample %x (channel %d, effect %d)\n", m_sample, ch, data >> 6);
			m_channel[ch].pos = m_addr[m_sample];
			m_channel[ch].playing = m_sample;
			m_channel[ch].effect = data >> 6;
		}
		break;
	case 1:
		m_addr[m_sample] = (m_addr[m_sample] & 0xff00) | (data << 0);
		break;
	case 2:
		m_addr[m_sample] = (m_addr[m_sample] & 0x00ff) | (data << 8);
		m_valid[m_sample] = 1;
		LOG("  sample %x address = %04x\n", m_sample, m_addr[m_sample]);
		break;
	case 3:
		m_ready = true;
		break;
	}
}
