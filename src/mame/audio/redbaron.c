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
#include "includes/bzone.h"
#include "sound/pokey.h"

#define OUTPUT_RATE		(48000)

struct redbaron_sound_state
{
	INT16 *m_vol_lookup;

	INT16 m_vol_crash[16];

	sound_stream *m_channel;
	int m_latch;
	int m_poly_counter;
	int m_poly_shift;

	int m_filter_counter;

	int m_crash_amp;
	int m_shot_amp;
	int m_shot_amp_counter;

	int m_squeal_amp;
	int m_squeal_amp_counter;
	int m_squeal_off_counter;
	int m_squeal_on_counter;
	int m_squeal_out;
};

INLINE redbaron_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == REDBARON);

	return (redbaron_sound_state *)downcast<redbaron_sound_device *>(device)->token();
}


WRITE8_DEVICE_HANDLER( redbaron_sounds_w )
{
	redbaron_sound_state *state = get_safe_token(device);

	/* If sound is off, don't bother playing samples */
	if( data == state->m_latch )
		return;

	state->m_channel->update();
	state->m_latch = data;
}

#ifdef UNUSED_FUNCTION
WRITE8_DEVICE_HANDLER( redbaron_pokey_w )
{
	redbaron_sound_state *state = get_safe_token(device);

	if( state->m_latch & 0x20 )
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
		state->m_poly_counter -= 12000;
		while( state->m_poly_counter <= 0 )
		{
			state->m_poly_counter += OUTPUT_RATE;
			if( ((state->m_poly_shift & 0x0001) == 0) == ((state->m_poly_shift & 0x4000) == 0) )
				state->m_poly_shift = (state->m_poly_shift << 1) | 1;
			else
				state->m_poly_shift <<= 1;
		}

		/* What is the exact low pass filter frequency? */
		state->m_filter_counter -= 330;
		while( state->m_filter_counter <= 0 )
		{
			state->m_filter_counter += OUTPUT_RATE;
			state->m_crash_amp = (state->m_poly_shift & 1) ? state->m_latch >> 4 : 0;
		}
		/* mix crash sound at 35% */
		sum += state->m_vol_crash[state->m_crash_amp] * 35 / 100;

		/* shot not active: charge C32 (0.1u) */
		if( (state->m_latch & 0x04) == 0 )
			state->m_shot_amp = 32767;
		else
		if( (state->m_poly_shift & 0x8000) == 0 )
		{
			if( state->m_shot_amp > 0 )
			{
                /* discharge C32 (0.1u) through R26 (33k) + R27 (15k)
                 * 0.68 * C32 * (R26 + R27) = 3264us
                 */
//              #define C32_DISCHARGE_TIME (int)(32767 / 0.003264);
				/* I think this is to short. Is C32 really 1u? */
				#define C32_DISCHARGE_TIME (int)(32767 / 0.03264);
				state->m_shot_amp_counter -= C32_DISCHARGE_TIME;
				while( state->m_shot_amp_counter <= 0 )
				{
					state->m_shot_amp_counter += OUTPUT_RATE;
					if( --state->m_shot_amp == 0 )
						break;
				}
				/* mix shot sound at 35% */
				sum += state->m_vol_lookup[state->m_shot_amp] * 35 / 100;
			}
		}


		if( (state->m_latch & 0x02) == 0 )
			state->m_squeal_amp = 0;
		else
		{
			if( state->m_squeal_amp < 32767 )
			{
				/* charge C5 (22u) over R3 (68k) and CR1 (1N914)
                 * time = 0.68 * C5 * R3 = 1017280us
                 */
				#define C5_CHARGE_TIME (int)(32767 / 1.01728);
				state->m_squeal_amp_counter -= C5_CHARGE_TIME;
				while( state->m_squeal_amp_counter <= 0 )
				{
					state->m_squeal_amp_counter += OUTPUT_RATE;
					if( ++state->m_squeal_amp == 32767 )
						break;
				}
			}

			if( state->m_squeal_out )
			{
				/* NE555 setup as pulse position modulator
                 * C = 0.01u, Ra = 33k, Rb = 47k
                 * frequency = 1.44 / ((33k + 2*47k) * 0.01u) = 1134Hz
                 * modulated by squeal_amp
                 */
				state->m_squeal_off_counter -= (1134 + 1134 * state->m_squeal_amp / 32767) / 3;
				while( state->m_squeal_off_counter <= 0 )
				{
					state->m_squeal_off_counter += OUTPUT_RATE;
					state->m_squeal_out = 0;
				}
			}
			else
			{
				state->m_squeal_on_counter -= 1134;
				while( state->m_squeal_on_counter <= 0 )
				{
					state->m_squeal_on_counter += OUTPUT_RATE;
					state->m_squeal_out = 1;
				}
			}
		}

		/* mix sequal sound at 40% */
		if( state->m_squeal_out )
			sum += 32767 * 40 / 100;

		*buffer++ = sum;
	}
}

static DEVICE_START( redbaron_sound )
{
	redbaron_sound_state *state = get_safe_token(device);
	int i;

	state->m_vol_lookup = auto_alloc_array(device->machine(), INT16, 32768);
	for( i = 0; i < 0x8000; i++ )
		state->m_vol_lookup[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

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
		state->m_vol_crash[i] = 32767 * r0 / (r0 + r1);
	}

	state->m_channel = device->machine().sound().stream_alloc(*device, 0, 1, OUTPUT_RATE, 0, redbaron_sound_update);
}

const device_type REDBARON = &device_creator<redbaron_sound_device>;

redbaron_sound_device::redbaron_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, REDBARON, "Red Baron Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(redbaron_sound_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void redbaron_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void redbaron_sound_device::device_start()
{
	DEVICE_START_NAME( redbaron_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void redbaron_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


