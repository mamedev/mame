#include "emu.h"
#include "tms3615.h"

#define VMIN	0x0000
#define VMAX	0x7fff

#define TONES 13

static const int divisor[TONES] = { 478, 451, 426, 402, 379, 358, 338, 319, 301, 284, 268, 253, 239 };

struct tms_state {
	sound_stream *channel;	/* returned by stream_create() */
	int samplerate; 		/* output sample rate */
	int basefreq;			/* chip's base frequency */
	int counter8[TONES];	/* tone frequency counter for 8' */
	int counter16[TONES];	/* tone frequency counter for 16'*/
	int output8;			/* output signal bits for 8' */
	int output16;			/* output signal bits for 16' */
	int enable; 			/* mask which tones to play */
};

INLINE tms_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS3615);
	return (tms_state *)downcast<tms3615_device *>(device)->token();
}


static STREAM_UPDATE( tms3615_sound_update )
{
	tms_state *tms = (tms_state *)param;
	int samplerate = tms->samplerate;
	stream_sample_t *buffer8 = outputs[TMS3615_FOOTAGE_8];
	stream_sample_t *buffer16 = outputs[TMS3615_FOOTAGE_16];

	while( samples-- > 0 )
	{
		int sum8 = 0, sum16 = 0, tone = 0;

		for (tone = 0; tone < TONES; tone++)
		{
			// 8'

			tms->counter8[tone] -= (tms->basefreq / divisor[tone]);

			while( tms->counter8[tone] <= 0 )
			{
				tms->counter8[tone] += samplerate;
				tms->output8 ^= 1 << tone;
			}

			if (tms->output8 & tms->enable & (1 << tone))
			{
				sum8 += VMAX;
			}

			// 16'

			tms->counter16[tone] -= (tms->basefreq / divisor[tone] / 2);

			while( tms->counter16[tone] <= 0 )
			{
				tms->counter16[tone] += samplerate;
				tms->output16 ^= 1 << tone;
			}

			if (tms->output16 & tms->enable & (1 << tone))
			{
				sum16 += VMAX;
			}
		}

        *buffer8++ = sum8 / TONES;
        *buffer16++ = sum16 / TONES;
	}

	tms->enable = 0;
}

void tms3615_enable_w(device_t *device, int enable)
{
	tms_state *tms = get_safe_token(device);
	tms->enable = enable;
}

static DEVICE_START( tms3615 )
{
	tms_state *tms = get_safe_token(device);

	tms->channel = device->machine().sound().stream_alloc(*device, 0, 2, device->clock()/8, tms, tms3615_sound_update);
	tms->samplerate = device->clock()/8;
	tms->basefreq = device->clock();
}

const device_type TMS3615 = &device_creator<tms3615_device>;

tms3615_device::tms3615_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS3615, "TMS3615", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(tms_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tms3615_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms3615_device::device_start()
{
	DEVICE_START_NAME( tms3615 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms3615_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


