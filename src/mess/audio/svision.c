/***************************************************************************
 supervision sound hardware

 PeT mess@utanet.at
***************************************************************************/

#include "emu.h"
#include "includes/svision.h"

typedef enum
{
	SVISION_NOISE_Type7Bit,
	SVISION_NOISE_Type14Bit
} SVISION_NOISE_Type;

struct SVISION_NOISE 
{
	UINT8 reg[3];
	int on, right, left, play;
	SVISION_NOISE_Type type;
	int state;
	int volume;
	int count;
	double step, pos;
	int value; // currently simple random function
};

struct SVISION_DMA 
{
	UINT8 reg[5];
	int on, right, left;
	int ca14to16;
	int start,size;
	double pos, step;
	int finished;
};

struct SVISION_CHANNEL 
{
	UINT8 reg[4];
	int on;
	int waveform, volume;
	int pos;
	int size;
	int count;
};

struct svision_sound_state
{
	sound_stream *mixer_channel;
	SVISION_DMA dma;
	SVISION_NOISE noise;
	SVISION_CHANNEL channel[2];
};


INLINE svision_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SVISION);
	return (svision_sound_state *)downcast<svision_sound_device *>(device)->token();
}

int *svision_dma_finished(device_t *device)
{
	svision_sound_state *state = get_safe_token(device);
	return &state->dma.finished;
}

void svision_sound_decrement(device_t *device)
{
	svision_sound_state *state = get_safe_token(device);

	if (state->channel[0].count > 0)
		state->channel[0].count--;
	if (state->channel[1].count > 0)
		state->channel[1].count--;
	if (state->noise.count > 0)
		state->noise.count--;
}

WRITE8_DEVICE_HANDLER( svision_sounddma_w )
{
	svision_sound_state *state = get_safe_token(device);
	logerror("%.6f svision snddma write %04x %02x\n", device->machine().time().as_double(),offset+0x18,data);
	state->dma.reg[offset] = data;
	switch (offset)
	{
		case 0:
		case 1:
			state->dma.start = (state->dma.reg[0] | (state->dma.reg[1] << 8));
			break;
		case 2:
			state->dma.size = (data ? data : 0x100) * 32;
			break;
		case 3:
			state->dma.step = device->machine().device("maincpu")->unscaled_clock() / (256.0 * device->machine().sample_rate() * (1 + (data & 3)));
			state->dma.right = data & 4;
			state->dma.left = data & 8;
			state->dma.ca14to16 = ((data & 0x70) >> 4) << 14;
			break;
		case 4:
			state->dma.on = data & 0x80;
			if (state->dma.on)
			{
				state->dma.pos = 0.0;
			}
			break;
	}
}

WRITE8_DEVICE_HANDLER( svision_noise_w )
{
	svision_sound_state *state = get_safe_token(device);
	//  logerror("%.6f svision noise write %04x %02x\n",machine.time(),offset+0x28,data);
	state->noise.reg[offset]=data;
	switch (offset)
	{
		case 0:
			state->noise.volume=data&0xf;
			state->noise.step= device->machine().device("maincpu")->unscaled_clock() / (256.0*device->machine().sample_rate()*(1+(data>>4)));
			break;
		case 1:
			state->noise.count = data + 1;
			break;
		case 2:
			state->noise.type = (SVISION_NOISE_Type) (data & 1);
			state->noise.play = data & 2;
			state->noise.right = data & 4;
			state->noise.left = data & 8;
			state->noise.on = data & 0x10; /* honey bee start */
			state->noise.state = 1;
			break;
	}
	state->noise.pos=0.0;
}

void svision_soundport_w(device_t *device, int which, int offset, int data)
{
	svision_sound_state *state = get_safe_token(device);
	SVISION_CHANNEL *channel = &state->channel[which];
	UINT16 size;

	state->mixer_channel->update();
	channel->reg[offset] = data;

	switch (offset)
	{
		case 0:
		case 1:
			size = channel->reg[0] | ((channel->reg[1] & 7) << 8);
			if (size)
			{
				//  channel->size=(int)(device->machine().sample_rate()*(size<<5)/4e6);
				channel->size= (int) (device->machine().sample_rate() * (size << 5) / device->machine().device("maincpu")->unscaled_clock());
			}
			else
			{
				channel->size = 0;
			}
			channel->pos = 0;
			break;
		case 2:
			channel->on = data & 0x40;
			channel->waveform = (data & 0x30) >> 4;
			channel->volume = data & 0xf;
			break;
		case 3:
			channel->count = data + 1;
			break;
	}
}

/************************************/
/* Sound handler update             */
/************************************/
static STREAM_UPDATE( svision_update )
{
	svision_sound_state *state = get_safe_token(device);
	stream_sample_t *left=outputs[0], *right=outputs[1];
	int i, j;
	SVISION_CHANNEL *channel;

	for (i = 0; i < samples; i++, left++, right++)
	{
		*left = 0;
		*right = 0;
		for (channel=state->channel, j=0; j<ARRAY_LENGTH(state->channel); j++, channel++)
		{
			if (channel->size != 0)
			{
				if (channel->on||channel->count)
				{
					int on = FALSE;
					switch (channel->waveform)
					{
						case 0:
							on = channel->pos <= (28 * channel->size) >> 5;
							break;
						case 1:
							on = channel->pos <= (24 * channel->size) >> 5;
							break;
						default:
						case 2:
							on = channel->pos <= channel->size / 2;
							break;
						case 3:
							on = channel->pos <= (9 * channel->size) >> 5;
							break;
					}
					{
						INT16 s = on ? channel->volume << 8 : 0;
						if (j == 0)
							*right += s;
						else
							*left += s;
					}
				}
				channel->pos++;
				if (channel->pos >= channel->size)
					channel->pos = 0;
			}
		}
		if (state->noise.on && (state->noise.play || state->noise.count))
		{
			INT16 s = (state->noise.value ? 1 << 8: 0) * state->noise.volume;
			int b1, b2;
			if (state->noise.left)
				*left += s;
			if (state->noise.right)
				*right += s;
			state->noise.pos += state->noise.step;
			if (state->noise.pos >= 1.0)
			{
				switch (state->noise.type)
				{
					case SVISION_NOISE_Type7Bit:
						state->noise.value = state->noise.state & 0x40 ? 1 : 0;
						b1 = (state->noise.state & 0x40) != 0;
						b2 = (state->noise.state & 0x20) != 0;
						state->noise.state=(state->noise.state<<1)+(b1!=b2?1:0);
						break;
					case SVISION_NOISE_Type14Bit:
					default:
						state->noise.value = state->noise.state & 0x2000 ? 1 : 0;
						b1 = (state->noise.state & 0x2000) != 0;
						b2 = (state->noise.state & 0x1000) != 0;
						state->noise.state = (state->noise.state << 1) + (b1 != b2 ? 1 : 0);
				}
				state->noise.pos -= 1;
			}
		}
		if (state->dma.on)
		{
			UINT8 sample;
			INT16 s;
			UINT16 addr = state->dma.start + (unsigned) state->dma.pos / 2;
			if (addr >= 0x8000 && addr < 0xc000)
			{
				sample = device->machine().root_device().memregion("user1")->base()[(addr & 0x3fff) | state->dma.ca14to16];
			}
			else
			{
				sample = device->machine().device("maincpu")->memory().space(AS_PROGRAM)->read_byte(addr);
			}
			if (((unsigned)state->dma.pos) & 1)
				s = (sample & 0xf);
			else
				s = (sample & 0xf0) >> 4;
			s <<= 8;
			if (state->dma.left)
				*left += s;
			if (state->dma.right)
				*right += s;
			state->dma.pos += state->dma.step;
			if (state->dma.pos >= state->dma.size)
			{
				state->dma.finished = TRUE;
				state->dma.on = FALSE;
				svision_irq(device->machine());
			}
		}
	}
}

/************************************/
/* Sound handler start              */
/************************************/

static DEVICE_START( svision_sound )
{
	svision_sound_state *state = get_safe_token(device);
	memset(&state->dma, 0, sizeof(state->dma));
	memset(&state->noise, 0, sizeof(state->noise));
	memset(state->channel, 0, sizeof(state->channel));

	state->mixer_channel = device->machine().sound().stream_alloc(*device, 0, 2, device->machine().sample_rate(), 0, svision_update);
}

const device_type SVISION = &device_creator<svision_sound_device>;

svision_sound_device::svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SVISION, "Super Vision Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(svision_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void svision_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void svision_sound_device::device_start()
{
	DEVICE_START_NAME( svision_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void svision_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


