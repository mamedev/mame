/****************************************************************************
 *
 * geebee.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/warpwarp.h"


typedef struct _geebee_sound_state geebee_sound_state;
struct _geebee_sound_state
{
	emu_timer *volume_timer;
	UINT16 *decay;
	sound_stream *channel;
	int sound_latch;
	int sound_signal;
	int volume;
	int noise;
	int vcount;
};

INLINE geebee_sound_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == GEEBEE);

	return (geebee_sound_state *)downcast<legacy_device_base *>(device)->token();
}

static TIMER_CALLBACK( volume_decay )
{
	geebee_sound_state *state = (geebee_sound_state *)ptr;
	if( --state->volume < 0 )
		state->volume = 0;
}

WRITE8_DEVICE_HANDLER( geebee_sound_w )
{
	geebee_sound_state *state = get_safe_token(device);

	stream_update(state->channel);
	state->sound_latch = data;
	state->volume = 0x7fff; /* set volume */
	state->noise = 0x0000;  /* reset noise shifter */
	/* faster decay enabled? */
	if( state->sound_latch & 8 )
	{
		/*
         * R24 is 10k, Rb is 0, C57 is 1uF
         * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
         * discharge time t2 = 0.693 * (Rb) * C57 -> 0
         * Then C33 is only charged via D6 (1N914), not discharged!
         * Decay:
         * discharge C33 (1uF) through R50 (22k) -> 0.14058s
         */
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 14058), 100000);
		timer_adjust_periodic(state->volume_timer, period, 0, period);
	}
	else
	{
		/*
         * discharge only after R49 (100k) in the amplifier section,
         * so the volume shouldn't very fast and only when the signal
         * is gated through 6N (4066).
         * I can only guess here that the decay should be slower,
         * maybe half as fast?
         */
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 29060), 100000);
		timer_adjust_periodic(state->volume_timer, period, 0, period);
    }
}

static STREAM_UPDATE( geebee_sound_update )
{
	geebee_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = state->sound_signal;
		/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
		{
			state->vcount++;
			/* noise clocked with raising edge of 2V */
			if ((state->vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((state->noise & 1) == ((state->noise >> 10) & 1))
					state->noise = ((state->noise << 1) & 0xfffe) | 1;
				else
					state->noise = (state->noise << 1) & 0xfffe;
			}
			switch (state->sound_latch & 7)
			{
			case 0: /* 4V */
				state->sound_signal = (state->vcount & 0x04) ? state->decay[state->volume] : 0;
				break;
			case 1: /* 8V */
				state->sound_signal = (state->vcount & 0x08) ? state->decay[state->volume] : 0;
				break;
			case 2: /* 16V */
				state->sound_signal = (state->vcount & 0x10) ? state->decay[state->volume] : 0;
				break;
			case 3: /* 32V */
				state->sound_signal = (state->vcount & 0x20) ? state->decay[state->volume] : 0;
				break;
			case 4: /* TONE1 */
				state->sound_signal = !(state->vcount & 0x01) && !(state->vcount & 0x10) ? state->decay[state->volume] : 0;
				break;
			case 5: /* TONE2 */
				state->sound_signal = !(state->vcount & 0x02) && !(state->vcount & 0x20) ? state->decay[state->volume] : 0;
				break;
			case 6: /* TONE3 */
				state->sound_signal = !(state->vcount & 0x04) && !(state->vcount & 0x40) ? state->decay[state->volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				state->sound_signal = (state->noise & 0x8000) ? state->decay[state->volume] : 0;
			}
		}
	}
}

static DEVICE_START( geebee_sound )
{
	geebee_sound_state *state = get_safe_token(device);
	running_machine *machine = device->machine;
	int i;

	state->decay = auto_alloc_array(machine, UINT16, 32768);

	for( i = 0; i < 0x8000; i++ )
		state->decay[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	/* 1V = HSYNC = 18.432MHz / 3 / 2 / 384 = 8000Hz */
	state->channel = stream_create(device, 0, 1, 18432000 / 3 / 2 / 384, NULL, geebee_sound_update);
	state->vcount = 0;

	state->volume_timer = timer_alloc(machine, volume_decay, state);
}

DEVICE_GET_INFO( geebee_sound )
{
	switch (state)
	{
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(geebee_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(geebee_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Gee Bee Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(GEEBEE, geebee_sound);
