/*************************************************************************

    Atari Red Baron hardware

*************************************************************************/
/*

    Red Baron sound notes:
    bit function
    7-4 explosion volume
    3   start led
    2   shot (machine gun sound)
    1   squeal (nosedive sound)
    0   POTSEL (rb_input_select)
*/

#include "emu.h"
#include "streams.h"
#include "includes/bzone.h"
#include "sound/pokey.h"

#define OUTPUT_RATE		(48000)

typedef struct _redbaron_sound_state redbaron_sound_state;
struct _redbaron_sound_state
{
	INT16 *vol_lookup;

	INT16 vol_crash[16];

	sound_stream *channel;
	int latch;
	int poly_counter;
	int poly_shift;

	int filter_counter;

	int crash_amp;
	int shot_amp;
	int shot_amp_counter;

	int squeal_amp;
	int squeal_amp_counter;
	int squeal_off_counter;
	int squeal_on_counter;
	int squeal_out;
};

INLINE redbaron_sound_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == REDBARON);

	return (redbaron_sound_state *)downcast<legacy_device_base *>(device)->token();
}


WRITE8_DEVICE_HANDLER( redbaron_sounds_w )
{
	redbaron_sound_state *state = get_safe_token(device);

	/* If sound is off, don't bother playing samples */
	if( data == state->latch )
		return;

	stream_update(state->channel);
	state->latch = data;
}

#ifdef UNUSED_FUNCTION
WRITE8_DEVICE_HANDLER( redbaron_pokey_w )
{
	redbaron_sound_state *state = get_safe_token(device);

	if( state->latch & 0x20 )
		pokey_w(device, offset, data);
}
#endif

static STREAM_UPDATE( redbaron_sound_update )
{
	redbaron_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];
	while( samples-- )
	{
		int sum = 0;

		/* polynome shifter E5 and F4 (LS164) clocked with 12kHz */
		state->poly_counter -= 12000;
		while( state->poly_counter <= 0 )
		{
			state->poly_counter += OUTPUT_RATE;
			if( ((state->poly_shift & 0x0001) == 0) == ((state->poly_shift & 0x4000) == 0) )
				state->poly_shift = (state->poly_shift << 1) | 1;
			else
				state->poly_shift <<= 1;
		}

		/* What is the exact low pass filter frequency? */
		state->filter_counter -= 330;
		while( state->filter_counter <= 0 )
		{
			state->filter_counter += OUTPUT_RATE;
			state->crash_amp = (state->poly_shift & 1) ? state->latch >> 4 : 0;
		}
		/* mix crash sound at 35% */
		sum += state->vol_crash[state->crash_amp] * 35 / 100;

		/* shot not active: charge C32 (0.1u) */
		if( (state->latch & 0x04) == 0 )
			state->shot_amp = 32767;
		else
		if( (state->poly_shift & 0x8000) == 0 )
		{
			if( state->shot_amp > 0 )
			{
                /* discharge C32 (0.1u) through R26 (33k) + R27 (15k)
                 * 0.68 * C32 * (R26 + R27) = 3264us
                 */
//              #define C32_DISCHARGE_TIME (int)(32767 / 0.003264);
				/* I think this is to short. Is C32 really 1u? */
				#define C32_DISCHARGE_TIME (int)(32767 / 0.03264);
				state->shot_amp_counter -= C32_DISCHARGE_TIME;
				while( state->shot_amp_counter <= 0 )
				{
					state->shot_amp_counter += OUTPUT_RATE;
					if( --state->shot_amp == 0 )
						break;
				}
				/* mix shot sound at 35% */
				sum += state->vol_lookup[state->shot_amp] * 35 / 100;
			}
		}


		if( (state->latch & 0x02) == 0 )
			state->squeal_amp = 0;
		else
		{
			if( state->squeal_amp < 32767 )
			{
				/* charge C5 (22u) over R3 (68k) and CR1 (1N914)
                 * time = 0.68 * C5 * R3 = 1017280us
                 */
				#define C5_CHARGE_TIME (int)(32767 / 1.01728);
				state->squeal_amp_counter -= C5_CHARGE_TIME;
				while( state->squeal_amp_counter <= 0 )
				{
					state->squeal_amp_counter += OUTPUT_RATE;
					if( ++state->squeal_amp == 32767 )
						break;
				}
			}

			if( state->squeal_out )
			{
				/* NE555 setup as pulse position modulator
                 * C = 0.01u, Ra = 33k, Rb = 47k
                 * frequency = 1.44 / ((33k + 2*47k) * 0.01u) = 1134Hz
                 * modulated by squeal_amp
                 */
				state->squeal_off_counter -= (1134 + 1134 * state->squeal_amp / 32767) / 3;
				while( state->squeal_off_counter <= 0 )
				{
					state->squeal_off_counter += OUTPUT_RATE;
					state->squeal_out = 0;
				}
			}
			else
			{
				state->squeal_on_counter -= 1134;
				while( state->squeal_on_counter <= 0 )
				{
					state->squeal_on_counter += OUTPUT_RATE;
					state->squeal_out = 1;
				}
			}
		}

		/* mix sequal sound at 40% */
		if( state->squeal_out )
			sum += 32767 * 40 / 100;

		*buffer++ = sum;
	}
}

static DEVICE_START( redbaron_sound )
{
	redbaron_sound_state *state = get_safe_token(device);
	int i;

	state->vol_lookup = auto_alloc_array(device->machine, INT16, 32768);
	for( i = 0; i < 0x8000; i++ )
		state->vol_lookup[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	for( i = 0; i < 16; i++ )
	{
		/* r0 = R18 and R24, r1 = open */
		double r0 = 1.0/(5600 + 680), r1 = 1/6e12;

		/* R14 */
		if( i & 1 )
			r1 += 1.0/8200;
		else
			r0 += 1.0/8200;
		/* R15 */
		if( i & 2 )
			r1 += 1.0/3900;
		else
			r0 += 1.0/3900;
		/* R16 */
		if( i & 4 )
			r1 += 1.0/2200;
		else
			r0 += 1.0/2200;
		/* R17 */
		if( i & 8 )
			r1 += 1.0/1000;
		else
			r0 += 1.0/1000;
		r0 = 1.0/r0;
		r1 = 1.0/r1;
		state->vol_crash[i] = 32767 * r0 / (r0 + r1);
	}

	state->channel = stream_create(device, 0, 1, OUTPUT_RATE, 0, redbaron_sound_update);
}


DEVICE_GET_INFO( redbaron_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(redbaron_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(redbaron_sound);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Red Baron Custom");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


DEFINE_LEGACY_SOUND_DEVICE(REDBARON, redbaron_sound);
