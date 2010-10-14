/****************************************************************************
 *
 * warpwarp.c
 *
 * sound driver
 * juergen buchmueller <pullmoll@t-online.de>, jan 2000
 *
 ****************************************************************************/

#include "emu.h"
#include "streams.h"
#include "includes/warpwarp.h"

#define CLOCK_16H	(18432000/3/2/16)
#define CLOCK_1V    (18432000/3/2/384)

typedef struct _warpwarp_sound_state warpwarp_sound_state;
struct _warpwarp_sound_state
{
	INT16 *decay;
	sound_stream *channel;
	int sound_latch;
	int music1_latch;
	int music2_latch;
	int sound_signal;
	int sound_volume;
	emu_timer *sound_volume_timer;
	int music_signal;
	int music_volume;
	emu_timer *music_volume_timer;
	int noise;

	int vcarry;
	int vcount;
	int mcarry;
	int mcount;
};

INLINE warpwarp_sound_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == WARPWARP);

	return (warpwarp_sound_state *)downcast<legacy_device_base *>(device)->token();
}

static TIMER_CALLBACK( sound_volume_decay )
{
	warpwarp_sound_state *state = (warpwarp_sound_state *)ptr;

	if( --state->sound_volume < 0 )
		state->sound_volume = 0;
}

WRITE8_DEVICE_HANDLER( warpwarp_sound_w )
{
	warpwarp_sound_state *state = get_safe_token(device);

	stream_update(state->channel);
	state->sound_latch = data & 0x0f;
	state->sound_volume = 0x7fff; /* set sound_volume */
	state->noise = 0x0000;  /* reset noise shifter */

	/* faster decay enabled? */
	if( state->sound_latch & 8 )
	{
		/*
         * R85(?) is 10k, Rb is 0, C92 is 1uF
         * charge time t1 = 0.693 * (R24 + Rb) * C57 -> 0.22176s
         * discharge time t2 = 0.693 * (Rb) * C57 -> 0
         * C90(?) is only charged via D17 (1N914), no discharge!
         * Decay:
         * discharge C90(?) (1uF) through R13||R14 (22k||47k)
         * 0.639 * 15k * 1uF -> 0.9585s
         */
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 95850), 100000);
		timer_adjust_periodic(state->sound_volume_timer, period, 0, period);
	}
	else
	{
		/*
         * discharge only after R93 (100k) and through the 10k
         * potentiometerin the amplifier section.
         * 0.639 * 110k * 1uF -> 7.0290s
         * ...but this is not very realistic for the game sound :(
         * maybe there _is_ a discharge through the diode D17?
         */
		//attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 702900), 100000);
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 191700), 100000);
		timer_adjust_periodic(state->sound_volume_timer, period, 0, period);
	}
}

WRITE8_DEVICE_HANDLER( warpwarp_music1_w )
{
	warpwarp_sound_state *state = get_safe_token(device);

	stream_update(state->channel);
	state->music1_latch = data & 0x3f;
}

static TIMER_CALLBACK( music_volume_decay )
{
	warpwarp_sound_state *state = (warpwarp_sound_state *)ptr;
	if( --state->music_volume < 0 )
		state->music_volume = 0;
}

WRITE8_DEVICE_HANDLER( warpwarp_music2_w )
{
	warpwarp_sound_state *state = get_safe_token(device);
	stream_update(state->channel);
	state->music2_latch = data & 0x3f;
	state->music_volume = 0x7fff;
	/* fast decay enabled? */
	if( state->music2_latch & 0x10 )
	{
		/*
         * Ra (R83?) is 10k, Rb is 0, C92 is 1uF
         * charge time t1 = 0.693 * (Ra + Rb) * C -> 0.22176s
         * discharge time is (nearly) zero, because Rb is zero
         * C95(?) is only charged via D17, not discharged!
         * Decay:
         * discharge C95(?) (10uF) through R13||R14 (22k||47k)
         * 0.639 * 15k * 10uF -> 9.585s
         * ...I'm sure this is off by one number of magnitude :/
         */
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 95850), 100000);
		timer_adjust_periodic(state->music_volume_timer, period, 0, period);
	}
	else
	{
		/*
         * discharge through R14 (47k),
         * discharge C95(?) (10uF) through R14 (47k)
         * 0.639 * 47k * 10uF -> 30.033s
         */
		//attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768), 3003300), 100000);
		attotime period = attotime_div(attotime_mul(ATTOTIME_IN_HZ(32768),  300330), 100000);
		timer_adjust_periodic(state->music_volume_timer, period, 0, period);
	}

}

static STREAM_UPDATE( warpwarp_sound_update )
{
	warpwarp_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];

	while (samples--)
	{
		*buffer++ = (state->sound_signal + state->music_signal) / 2;

		/*
         * The music signal is selected at a rate of 2H (1.536MHz) from the
         * four bits of a 4 bit binary counter which is clocked with 16H,
         * which is 192kHz, and is divided by 4 times (64 - music1_latch).
         *  0 = 256 steps -> 750 Hz
         *  1 = 252 steps -> 761.9 Hz
         * ...
         * 32 = 128 steps -> 1500 Hz
         * ...
         * 48 =  64 steps -> 3000 Hz
         * ...
         * 63 =   4 steps -> 48 kHz
         */
		state->mcarry -= CLOCK_16H / (4 * (64 - state->music1_latch));
		while( state->mcarry < 0 )
		{
			state->mcarry += CLOCK_16H;
			state->mcount++;
			state->music_signal = (state->mcount & ~state->music2_latch & 15) ? state->decay[state->music_volume] : 0;
			/* override by noise gate? */
			if( (state->music2_latch & 32) && (state->noise & 0x8000) )
				state->music_signal = state->decay[state->music_volume];
		}

		/* clock 1V = 8kHz */
		state->vcarry -= CLOCK_1V;
		while (state->vcarry < 0)
		{
			state->vcarry += CLOCK_16H;
			state->vcount++;

			/* noise is clocked with raising edge of 2V */
			if ((state->vcount & 3) == 2)
			{
				/* bit0 = bit0 ^ !bit10 */
				if ((state->noise & 1) == ((state->noise >> 10) & 1))
					state->noise = (state->noise << 1) | 1;
				else
					state->noise = state->noise << 1;
			}

			switch (state->sound_latch & 7)
			{
			case 0: /* 4V */
				state->sound_signal = (state->vcount & 0x04) ? state->decay[state->sound_volume] : 0;
				break;
			case 1: /* 8V */
				state->sound_signal = (state->vcount & 0x08) ? state->decay[state->sound_volume] : 0;
				break;
			case 2: /* 16V */
				state->sound_signal = (state->vcount & 0x10) ? state->decay[state->sound_volume] : 0;
				break;
			case 3: /* 32V */
				state->sound_signal = (state->vcount & 0x20) ? state->decay[state->sound_volume] : 0;
				break;
			case 4: /* TONE1 */
				state->sound_signal = !(state->vcount & 0x01) && !(state->vcount & 0x10) ? state->decay[state->sound_volume] : 0;
				break;
			case 5: /* TONE2 */
				state->sound_signal = !(state->vcount & 0x02) && !(state->vcount & 0x20) ? state->decay[state->sound_volume] : 0;
				break;
			case 6: /* TONE3 */
				state->sound_signal = !(state->vcount & 0x04) && !(state->vcount & 0x40) ? state->decay[state->sound_volume] : 0;
				break;
			default: /* NOISE */
				/* QH of 74164 #4V */
				state->sound_signal = (state->noise & 0x8000) ? state->decay[state->sound_volume] : 0;
			}

		}
	}
}

static DEVICE_START( warpwarp_sound )
{
	warpwarp_sound_state *state = get_safe_token(device);
	running_machine *machine = device->machine;
	int i;

	state->decay = auto_alloc_array(machine, INT16, 32768);

	for( i = 0; i < 0x8000; i++ )
		state->decay[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	state->channel = stream_create(device, 0, 1, CLOCK_16H, NULL, warpwarp_sound_update);

	state->sound_volume_timer = timer_alloc(machine, sound_volume_decay, state);
	state->music_volume_timer = timer_alloc(machine, music_volume_decay, state);
}


DEVICE_GET_INFO( warpwarp_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(warpwarp_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(warpwarp_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Warp Warp Custom");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(WARPWARP, warpwarp_sound);
