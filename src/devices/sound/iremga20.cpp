// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont, Valley Bell
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

09-25-2018 Valley Bell
- rewrote channel update to make data 0 act as sample terminator

*********************************************************/

#include "emu.h"
#include "iremga20.h"

#include <algorithm>

#define MAX_VOL 256


// device type definition
DEFINE_DEVICE_TYPE(IREMGA20, iremga20_device, "iremga20", "Irem GA20")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  iremga20_device - constructor
//-------------------------------------------------

iremga20_device::iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IREMGA20, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this, 20),
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

	std::fill(std::begin(m_regs), std::end(m_regs), 0);

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
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void iremga20_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock()/4);
}

//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void iremga20_device::rom_bank_updated()
{
	m_stream->update();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void iremga20_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *outL, *outR;

	outL = outputs[0];
	outR = outputs[1];

	for (int i = 0; i < samples; i++)
	{
		stream_sample_t sampleout = 0;

		for (auto & ch : m_channel)
		{
			if (ch.play)
			{
				int sample = read_byte(ch.pos);
				if (sample == 0x00) // check for sample end marker
					ch.play = 0;
				else
					sampleout += (sample - 0x80) * (int32_t)ch.volume;
				ch.frac += ch.rate;
				ch.pos += (ch.frac >> 24);
				ch.frac &= ((1 << 24) - 1);
				if (ch.pos >= ch.end)   // for safety (the actual chip probably doesn't check this)
					ch.play = 0;
			}
		}

		sampleout >>= 2;
		outL[i] = sampleout;
		outR[i] = sampleout;
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

		case 2: /* end? address low */
			m_channel[channel].end = ((m_channel[channel].end)&0xff000) | (data<<4);
			break;

		case 3: /* end? address high */
			m_channel[channel].end = ((m_channel[channel].end)&0x00ff0) | (data<<12);
			break;

		case 4:
			m_channel[channel].rate = (1 << 24) / (256 - data);
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
