// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont
/*********************************************************

Irem GA20 PCM Sound Chip

It's not currently known whether this chip is stereo.


Revisions:

04-15-2002 Acho A. Tang
- rewrote channel mixing
- added prelimenary volume and sample rate emulation

05-30-2002 Acho A. Tang
- applied hyperbolic gain control to volume and used
  a musical-note style progression in sample rate
  calculation(still very inaccurate)

02-18-2004 R. Belmont
- sample rate calculation reverse-engineered.
  Thanks to Fujix, Yasuhiro Ogawa, the Guru, and Tormod
  for real PCB samples that made this possible.

02-03-2007 R. Belmont
- Cleaned up faux x86 assembly.

*********************************************************/

#include "emu.h"
#include "iremga20.h"

#define MAX_VOL 256


// device type definition
const device_type IREMGA20 = &device_creator<iremga20_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iremga20_device - constructor
//-------------------------------------------------

iremga20_device::iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IREMGA20, "Irem GA20", tag, owner, clock, "iremga20", __FILE__),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void iremga20_device::device_start()
{
	int i;

	iremga20_reset();

	for ( i = 0; i < 0x40; i++ )
		m_regs[i] = 0;

	m_stream = stream_alloc(0, 2, clock()/4);

	save_item(NAME(m_regs));
	for (i = 0; i < 4; i++)
	{
		save_item(NAME(m_channel[i].rate), i);
		save_item(NAME(m_channel[i].size), i);
		save_item(NAME(m_channel[i].start), i);
		save_item(NAME(m_channel[i].pos), i);
		save_item(NAME(m_channel[i].frac), i);
		save_item(NAME(m_channel[i].end), i);
		save_item(NAME(m_channel[i].volume), i);
		save_item(NAME(m_channel[i].pan), i);
		save_item(NAME(m_channel[i].effect), i);
		save_item(NAME(m_channel[i].play), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void iremga20_device::device_reset()
{
	iremga20_reset();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void iremga20_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT32 rate[4], pos[4], frac[4], end[4], vol[4], play[4];
	UINT8 *pSamples;
	stream_sample_t *outL, *outR;
	int i, sampleout;

	/* precache some values */
	for (i=0; i < 4; i++)
	{
		rate[i] = m_channel[i].rate;
		pos[i] = m_channel[i].pos;
		frac[i] = m_channel[i].frac;
		end[i] = m_channel[i].end - 0x20;
		vol[i] = m_channel[i].volume;
		play[i] = m_channel[i].play;
	}

	i = samples;
	pSamples = &m_rom[0];
	outL = outputs[0];
	outR = outputs[1];

	for (i = 0; i < samples; i++)
	{
		sampleout = 0;

		// update the 4 channels inline
		if (play[0])
		{
			sampleout += (pSamples[pos[0]] - 0x80) * vol[0];
			frac[0] += rate[0];
			pos[0] += frac[0] >> 24;
			frac[0] &= 0xffffff;
			play[0] = (pos[0] < end[0]);
		}
		if (play[1])
		{
			sampleout += (pSamples[pos[1]] - 0x80) * vol[1];
			frac[1] += rate[1];
			pos[1] += frac[1] >> 24;
			frac[1] &= 0xffffff;
			play[1] = (pos[1] < end[1]);
		}
		if (play[2])
		{
			sampleout += (pSamples[pos[2]] - 0x80) * vol[2];
			frac[2] += rate[2];
			pos[2] += frac[2] >> 24;
			frac[2] &= 0xffffff;
			play[2] = (pos[2] < end[2]);
		}
		if (play[3])
		{
			sampleout += (pSamples[pos[3]] - 0x80) * vol[3];
			frac[3] += rate[3];
			pos[3] += frac[3] >> 24;
			frac[3] &= 0xffffff;
			play[3] = (pos[3] < end[3]);
		}

		sampleout >>= 2;
		outL[i] = sampleout;
		outR[i] = sampleout;
	}

	/* update the regs now */
	for (i=0; i < 4; i++)
	{
		m_channel[i].pos = pos[i];
		m_channel[i].frac = frac[i];
		m_channel[i].play = play[i];
	}
}

WRITE8_MEMBER( iremga20_device::irem_ga20_w )
{
	int channel;

	//logerror("GA20:  Offset %02x, data %04x\n",offset,data);

	m_stream->update();

	channel = offset >> 3;

	m_regs[offset] = data;

	switch (offset & 0x7)
	{
		case 0: /* start address low */
			m_channel[channel].start = ((m_channel[channel].start)&0xff000) | (data<<4);
			break;

		case 1: /* start address high */
			m_channel[channel].start = ((m_channel[channel].start)&0x00ff0) | (data<<12);
			break;

		case 2: /* end address low */
			m_channel[channel].end = ((m_channel[channel].end)&0xff000) | (data<<4);
			break;

		case 3: /* end address high */
			m_channel[channel].end = ((m_channel[channel].end)&0x00ff0) | (data<<12);
			break;

		case 4:
			m_channel[channel].rate = 0x1000000 / (256 - data);
			break;

		case 5: //AT: gain control
			m_channel[channel].volume = (data * MAX_VOL) / (data + 10);
			break;

		case 6: //AT: this is always written 2(enabling both channels?)
			m_channel[channel].play = data;
			m_channel[channel].pos = m_channel[channel].start;
			m_channel[channel].frac = 0;
			break;
	}
}

READ8_MEMBER( iremga20_device::irem_ga20_r )
{
	int channel;

	m_stream->update();

	channel = offset >> 3;

	switch (offset & 0x7)
	{
		case 7: // voice status.  bit 0 is 1 if active. (routine around 0xccc in rtypeleo)
			return m_channel[channel].play ? 1 : 0;

		default:
			logerror("GA20: read unk. register %d, channel %d\n", offset & 0xf, channel);
			break;
	}

	return 0;
}


void iremga20_device::iremga20_reset()
{
	int i;

	for( i = 0; i < 4; i++ ) {
	m_channel[i].rate = 0;
	m_channel[i].size = 0;
	m_channel[i].start = 0;
	m_channel[i].pos = 0;
	m_channel[i].frac = 0;
	m_channel[i].end = 0;
	m_channel[i].volume = 0;
	m_channel[i].pan = 0;
	m_channel[i].effect = 0;
	m_channel[i].play = 0;
	}
}
