/***************************************************************************
 gamate sound hardware

 PeT mess@utanet.at
***************************************************************************/

#include "emu.h"
#include "includes/gamate.h"


// device type definition
const device_type GAMATE_SND = &device_creator<gamate_sound_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gamate_sound_device - constructor
//-------------------------------------------------

gamate_sound_device::gamate_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GAMATE_SND, "Gamate Audio Custom", tag, owner, clock, "gamate_sound", __FILE__),
		device_sound_interface(mconfig, *this),
		m_mixer_channel(NULL)
{
}


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
	GAMATE_CHANNEL *channel;

	for (i = 0; i < samples; i++, left++, right++)
	{
		*left = 0;
		*right = 0;
		for (channel=m_channels, j=0; j<ARRAY_LENGTH(m_channels); j++, channel++)
		{
			if (channel->size != 0)
			{
				if (channel->on)//||channel->count)
				{
					int on = FALSE;
					on = channel->pos <= channel->size / 2;
					{
						INT16 s = on ? channel->volume << 8 : 0;
						if (j == 0)
							*right += s;
						else if (j==1)
							*left += s;
						else {
							*right += s;
							*left += s;
						}
					}
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

	switch (offset)
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
				m_channels[chan].size= (int) (machine().sample_rate() * (size << 5) / machine().device("maincpu")->unscaled_clock());
			}
			else
			{
				m_channels[chan].size = 0;
			}
			m_channels[chan].pos = 0;
			break;
		case 6:
		case 7:
		case 8:
			chan=offset-6;
//          m_channels[chan]->on = data & 0x40;
//          channel->waveform = (data & 0x30) >> 4;
			m_channels[chan].volume = data & 0xf;
			break;
	}
	if (chan!=-1) m_channels[chan].on=m_channels[chan].volume!=0 && m_channels[chan].size>3/* avoid speed loss for unhearable >=23khz*/;
}
