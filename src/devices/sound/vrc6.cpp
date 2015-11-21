// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    vrc6.c
    Konami VRC6 additional sound channels

    Emulation by R. Belmont

    References:
    http://wiki.nesdev.com/w/index.php/VRC6_audio
    http://nesdev.com/vrcvi.txt

***************************************************************************/

#include "emu.h"
#include "vrc6.h"

#define DISABLE_VRC6_SOUND      // not ready yet

// device type definition
const device_type VRC6 = &device_creator<vrc6snd_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vrc6snd_device - constructor
//-------------------------------------------------

vrc6snd_device::vrc6snd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VRC6, "VRC6 sound", tag, owner, clock, "vrc6snd", __FILE__),
		device_sound_interface(mconfig, *this), m_freqctrl(0), m_sawrate(0), m_sawfrql(0), m_sawfrqh(0), m_sawclock(0), m_sawaccum(0), m_stream(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vrc6snd_device::device_start()
{
	m_stream = machine().sound().stream_alloc(*this, 0, 1, clock());

	m_freqctrl = m_pulsectrl[0] = m_pulsectrl[1] = 0;
	m_pulsefrql[0] = m_pulsefrql[1] = m_pulsefrqh[0] = m_pulsefrqh[1] = 0;
	m_sawaccum = m_sawfrql = m_sawfrqh = m_sawclock = m_sawrate = 0;
	m_ticks[0] = m_ticks[1] = m_ticks[2] = 0;
	m_output[0] = m_output[1] = m_output[2] = 0;
	m_pulseduty[0] = m_pulseduty[1] = 15;

	save_item(NAME(m_freqctrl));
	save_item(NAME(m_pulsectrl));
	save_item(NAME(m_sawrate));
	save_item(NAME(m_sawaccum));
	save_item(NAME(m_pulsefrql));
	save_item(NAME(m_pulsefrqh));
	save_item(NAME(m_sawfrql));
	save_item(NAME(m_sawfrqh));
	save_item(NAME(m_ticks));
	save_item(NAME(m_output));
	save_item(NAME(m_pulseduty));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void vrc6snd_device::device_reset()
{
	m_stream->update();

	m_freqctrl = m_pulsectrl[0] = m_pulsectrl[1] = 0;
	m_pulsefrql[0] = m_pulsefrql[1] = 0;
	m_sawaccum = m_sawfrql = m_sawclock = m_sawrate = 0;
	m_ticks[0] = m_ticks[1] = m_ticks[2] = 0;
	m_output[0] = m_output[1] = m_output[2] = 0;
	m_pulseduty[0] = m_pulseduty[1] = 15;
	m_pulsefrqh[0] = m_pulsefrqh[1] = m_sawfrqh = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void vrc6snd_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *out = outputs[0];
	INT16 tmp;
	int i;

	// check global halt bit
	if (m_freqctrl & 1)
	{
		return;
	}

	for (i = 0; i < samples; i++)
	{
		// update pulse1
		if (m_pulsefrqh[0] & 0x80)
		{
			m_ticks[0]--;
			if (m_ticks[0] == 0)
			{
				m_ticks[0] = m_pulsefrql[0] | (m_pulsefrqh[0] & 0xf)<<4;

				m_pulseduty[0]--;
				if (m_pulsectrl[0] & 0x80)
				{
					m_output[0] = m_pulsectrl[0] & 0xf;
				}
				else
				{
					if (m_pulseduty[0] <= ((m_pulsectrl[0]>>4) & 0x7))
					{
						m_output[0] = m_pulsectrl[0] & 0xf;
					}
					else
					{
						m_output[0] = 0;
					}
				}

				if (m_pulseduty[0] == 0)
				{
					m_pulseduty[0] = 15;
				}
			}
		}
		else
		{
			m_output[0] = 0;
		}

		// update pulse2
		if (m_pulsefrqh[1] & 0x80)
		{
			m_ticks[1]--;
			if (m_ticks[1] == 0)
			{
				m_ticks[1] = m_pulsefrql[1] | (m_pulsefrqh[1] & 0xf)<<4;

				m_pulseduty[1]--;
				if (m_pulsectrl[1] & 0x80)
				{
					m_output[1] = m_pulsectrl[1] & 0xf;
				}
				else
				{
					if (m_pulseduty[1] <= ((m_pulsectrl[1]>>4) & 0x7))
					{
						m_output[1] = m_pulsectrl[1] & 0xf;
					}
					else
					{
						m_output[1] = 0;
					}
				}

				if (m_pulseduty[1] == 0)
				{
					m_pulseduty[1] = 15;
				}
			}
		}
		else
		{
			m_output[1] = 0;
		}

		// update saw
		if (m_sawfrqh & 0x80)
		{
			m_ticks[2]--;
			if (m_ticks[2] == 0)
			{
				m_ticks[2] = m_sawfrql | (m_sawfrqh & 0xf)<<4;

				// only update on even steps
				if ((m_sawclock > 0) && (!(m_sawclock & 1)))
				{
					m_sawaccum += (m_sawrate & 0x3f);
					m_output[2] = (m_sawaccum>>3);
				}
				m_sawclock++;

				if (m_sawclock >= 14)
				{
					m_sawclock = m_sawaccum = 0;
					m_output[2] = 0;
				}
			}
		}
		else
		{
			m_output[2] = 0;
		}

		// sum 2 4-bit pulses, 1 5-bit saw = unsigned 6 bit output
		tmp = (INT16)(UINT8)(m_output[0] + m_output[1] + m_output[2]);
		tmp <<= 8;

		out[i] = tmp;
	}
}

//---------------------------------------
//  write - write to the chip's registers
//---------------------------------------

WRITE8_MEMBER( vrc6snd_device::write )
{
	switch (offset >> 8)
	{
		case 0:
			m_stream->update();
			switch (offset & 3)
			{
				case 0:
					m_pulsectrl[0] = data;
					break;

				case 1:
					m_pulsefrql[0] = data;
					if (!(m_pulsefrqh[1] & 0x80))
					{
						m_ticks[0] &= ~0xff;
						m_ticks[0] |= m_pulsefrql[0];
					}
					break;

				case 2:
					#ifndef DISABLE_VRC6_SOUND
					m_pulsefrqh[0] = data;
					// if disabling channel, reset phase
					if (!(data & 0x80))
					{
						m_pulseduty[0] = 15;
						m_ticks[0] &= 0xff;
						m_ticks[0] |= (m_pulsefrqh[0] & 0xf)<<4;
					}
					#endif
					break;

				case 3:
					m_freqctrl = data;
					break;
			}
			break;

		case 1:
			m_stream->update();
			switch (offset & 3)
			{
				case 0:
					m_pulsectrl[1] = data;
					break;

				case 1:
					m_pulsefrql[1] = data;
					if (!(m_pulsefrqh[1] & 0x80))
					{
						m_ticks[1] &= ~0xff;
						m_ticks[1] |= m_pulsefrql[1];
					}
					break;

				case 2:
					#ifndef DISABLE_VRC6_SOUND
					m_pulsefrqh[1] = data;
					// if disabling channel, reset phase
					if (!(data & 0x80))
					{
						m_pulseduty[1] = 15;
						m_ticks[1] &= 0xff;
						m_ticks[1] |= (m_pulsefrqh[1] & 0xf)<<4;
					}
					#endif
					break;
			}
			break;

		case 2:
			m_stream->update();
			switch (offset & 3)
			{
				case 0:
					m_sawrate = data;
					break;

				case 1:
					m_sawfrql = data;
					if (!(m_sawfrqh & 0x80))
					{
						m_ticks[2] &= ~0xff;
						m_ticks[2] |= m_sawfrql;
					}
					break;

				case 2:
					#ifndef DISABLE_VRC6_SOUND
					m_sawfrqh = data;
					// if disabling channel, reset phase
					if (!(data & 0x80))
					{
						m_sawaccum = 0;
						m_ticks[2] &= 0xff;
						m_ticks[2] |= (m_sawfrqh & 0xf)<<4;
					}
					#endif
					break;
			}
			break;
	}

}
