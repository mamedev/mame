// license:GPL-2.0+
// copyright-holders:Peter Trauner
/***************************************************************************
 gamate sound hardware

 PeT mess@utanet.at 2015
***************************************************************************/

#include "emu.h"
#include "includes/gamate.h"
#include "ui/ui.h"


enum { ClockDelay=32 };

// device type definition
const device_type GAMATE_SND = &device_creator<gamate_sound_device>;

const int gamate_sound_device::DAConverter[]={ 0, 3, 7, 13,  23, 41, 75, 137,  249, 453, 825, 1499,  2726, 4956, 9011, 16383 }; // (*.55) on the real below index 8 bareless measureable
const UINT8 Mask[]={ 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x0f, 0x1f, 0x3f,  0x1f, 0x1f, 0x1f, 0xff, 0xff, 0xf };
const int EnvelopeVolumes[]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0  };
//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gamate_sound_device - constructor
//-------------------------------------------------

gamate_sound_device::gamate_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GAMATE_SND, "Gamate Audio Custom", tag, owner, clock, "gamate_sound", __FILE__)
	, device_sound_interface(mconfig, *this)
	, m_mixer_channel(nullptr)
	{}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gamate_sound_device::device_start()
{
	// bind callbacks
//  m_irq_cb.bind_relative_to(*owner());

	memset(m_channels, 0, sizeof(m_channels));
	memset(reg, 0, sizeof(reg));

	m_mixer_channel = stream_alloc(0, 2, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gamate_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *left=outputs[0], *right=outputs[1];
	int i, j;
	Tone *channel;

	for (i = 0; i < samples; i++, left++, right++)
	{
		noise.pos += noise.step;
		while (noise.pos >= 1.0)
		{
			// guess (white noise register taken from supervision)
			noise.level = noise.state & 0x40 ? 1 : 0;
			bool b1 = (noise.state & 0x40) != 0, b2 = (noise.state & 0x20) != 0;
			noise.state=(noise.state<<1)+(b1!=b2?1:0);
			noise.pos -= 1;
		}

		envelope.pos += envelope.step;
		while (envelope.pos >= 1.0) {
			envelope.pos -= 1;
			envelope.index++;
			switch (envelope.control)
			{
				case 0: case 1: case 2: case 3:
				case 4: case 5: case 6: case 7:
				case 8: case 9: case 0xb:
				case 0xd: case 0xf:
					if (envelope.index>=ARRAY_LENGTH(EnvelopeVolumes)/2)
					{
						envelope.index=0;
						envelope.first=false;
					}
					break;
				default:
					if (envelope.index>=ARRAY_LENGTH(EnvelopeVolumes))
					{
						envelope.index=0;
						envelope.first=false;
					}
					break;
			}
		}

		*left = 0;
		*right = 0;
		for (channel=m_channels, j=0; j<ARRAY_LENGTH(m_channels); j++, channel++)
		{
			if (channel->size != 0)
			{
				channel->level= channel->pos <= channel->size / 2;
				bool l= channel->full_cycle? true: channel->level;
				if (!channel->tone)
					l= l && noise.level;
				int volume=0;
				if (l)
				{
					if (channel->envelope_on)
					{
						switch (envelope.control)
						{
							case 0: case 1: case 2: case 3:
							case 0x9: // one time falling, low
								if (envelope.first && channel->level)
									volume=0xf-EnvelopeVolumes[envelope.index];
								break;
							case 4: case 5: case 6: case 7:
							case 0xf: // one time rising, low
								if (envelope.first && channel->level)
									volume=EnvelopeVolumes[envelope.index];
								break;
							case 8: // falling
								if (channel->level)
									volume=0xf-EnvelopeVolumes[envelope.index];
								break;
							case 0xa: // rising, falling
								if (channel->level)
									volume=0xf-EnvelopeVolumes[envelope.index]; // cube up
								break;
							case 0xb: // one time falling, high
								if (channel->level)
									volume=envelope.first ? 0xf-EnvelopeVolumes[envelope.index] : 0xf;
								break;
							case 0xc: // rising, low
								if (channel->level)
									volume=envelope.index<ARRAY_LENGTH(EnvelopeVolumes)/2 ? EnvelopeVolumes[envelope.index] : 0;
								break;
							case 0xd: // one time rising, high
								if (channel->level)
									volume=envelope.first ? EnvelopeVolumes[envelope.index] : 0xf;
								break;
							case 0xe: // falling, rising
								if (channel->level)
									volume=0xf-EnvelopeVolumes[envelope.index];
								break;
						}
					}
					else
					{
						volume=channel->volume;
					}
				}
				if (j == Right)
					*right += Value2Volume(volume);
				else
				if (j==Left)
					*left += Value2Volume(volume);
				else
				{
					*right += Value2Volume(volume);
					*left += Value2Volume(volume);
				}
				channel->pos++;
				if (channel->pos >= channel->size)
					channel->pos = 0;
			}
		}
	}
}

WRITE8_MEMBER( gamate_sound_device::device_w )
{
	UINT16 size;

	m_mixer_channel->update();
	reg[offset] = data;
	int chan=-1;

	switch (offset&0xf)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			chan=offset/2;
			size = reg[chan*2] | ((reg[chan*2+1] & 0xf) << 8);
			if (size)
			{
				m_channels[chan].size= (int) (machine().sample_rate() * size*ClockDelay / m_clock);
			}
			else
			{
				m_channels[chan].size = 0;
			}
			m_channels[chan].pos = 0;
			break;
		case 6:
			size=data&0x1f;
			if (size==0)
				size=1;
			noise.step= m_clock / (1.0*ClockDelay*machine().sample_rate()*size);
			break;
		case 7:
			m_channels[Right].full_cycle=data&1;
			m_channels[Right].tone=data&8;
			m_channels[Left].full_cycle=data&2;
			m_channels[Left].tone=data&0x10;
			m_channels[Both].full_cycle=data&4;
			m_channels[Both].tone=data&0x20;
			noise.state=1;
			noise.pos=0.0;
			noise.level=false;
			break;
		case 8:
		case 9:
		case 0xa:
			chan=offset-8;
			m_channels[chan].envelope_on = data & 0x10; // buggy aussetzer cube up
			m_channels[chan].volume = data & 0xf;
			break;
		case 0xb: case 0xc:
			size = reg[0xb] | ((reg[0xc]) << 8);
			if (size==0)
				size=1;
			envelope.step= m_clock / (1.0*ClockDelay*machine().sample_rate()*size);
			break;
		case 0xd:
			envelope.control=data&0xf;
			envelope.pos=0;
			envelope.index=0;
			envelope.first=true;
			break;

	}
	envelope.pos=0; // guess
	envelope.index=0;
	envelope.first=true;
}

READ8_MEMBER( gamate_sound_device::device_r )
{
	UINT8 data=0;
	if ((offset&0xf)<ARRAY_LENGTH(Mask))
		data=reg[offset&0xf]&Mask[offset&0xf]; // unused bits set to last write value? in this area
	return data;
}
