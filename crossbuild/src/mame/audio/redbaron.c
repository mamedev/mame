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

#include <math.h>
#include "driver.h"
#include "streams.h"
#include "bzone.h"
#include "sound/custom.h"
#include "sound/pokey.h"

#define OUTPUT_RATE		(48000)

/* Statics */
static INT16 *vol_lookup = NULL;

static INT16 vol_crash[16];

static sound_stream *channel;
static int latch;
static int poly_counter;
static int poly_shift;

static int filter_counter;

static int crash_amp;
static int shot_amp;
static int shot_amp_counter;

static int squeal_amp;
static int squeal_amp_counter;
static int squeal_off_counter;
static int squeal_on_counter;
static int squeal_out;

WRITE8_HANDLER( redbaron_sounds_w )
{
	/* If sound is off, don't bother playing samples */
	if( data == latch )
		return;

	stream_update(channel);
    latch = data;
    rb_input_select = data & 1;
}

WRITE8_HANDLER( redbaron_pokey_w )
{
    if( latch & 0x20 )
        pokey1_w (offset, data);
}

static void redbaron_sound_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	stream_sample_t *buffer = outputs[0];
	while( length-- )
	{
		int sum = 0;

		/* polynome shifter E5 and F4 (LS164) clocked with 12kHz */
		poly_counter -= 12000;
		while( poly_counter <= 0 )
		{
			poly_counter += OUTPUT_RATE;
			if( ((poly_shift & 0x0001) == 0) == ((poly_shift & 0x4000) == 0) )
				poly_shift = (poly_shift << 1) | 1;
			else
				poly_shift <<= 1;
		}

		/* What is the exact low pass filter frequency? */
		filter_counter -= 330;
		while( filter_counter <= 0 )
		{
			filter_counter += OUTPUT_RATE;
			crash_amp = (poly_shift & 1) ? latch >> 4 : 0;
		}
		/* mix crash sound at 35% */
		sum += vol_crash[crash_amp] * 35 / 100;

		/* shot not active: charge C32 (0.1u) */
		if( (latch & 0x04) == 0 )
            shot_amp = 32767;
        else
		if( (poly_shift & 0x8000) == 0 )
        {
			if( shot_amp > 0 )
			{
                /* discharge C32 (0.1u) through R26 (33k) + R27 (15k)
                 * 0.68 * C32 * (R26 + R27) = 3264us
                 */
//              #define C32_DISCHARGE_TIME (int)(32767 / 0.003264);
				/* I think this is to short. Is C32 really 1u? */
				#define C32_DISCHARGE_TIME (int)(32767 / 0.03264);
				shot_amp_counter -= C32_DISCHARGE_TIME;
				while( shot_amp_counter <= 0 )
				{
					shot_amp_counter += OUTPUT_RATE;
					if( --shot_amp == 0 )
						break;
				}
				/* mix shot sound at 35% */
				sum += vol_lookup[shot_amp] * 35 / 100;
            }
        }


		if( (latch & 0x02) == 0 )
			squeal_amp = 0;
		else
		{
			if( squeal_amp < 32767 )
			{
				/* charge C5 (22u) over R3 (68k) and CR1 (1N914)
                 * time = 0.68 * C5 * R3 = 1017280us
                 */
				#define C5_CHARGE_TIME (int)(32767 / 1.01728);
				squeal_amp_counter -= C5_CHARGE_TIME;
				while( squeal_amp_counter <= 0 )
				{
					squeal_amp_counter += OUTPUT_RATE;
					if( ++squeal_amp == 32767 )
						break;
				}
			}

			if( squeal_out )
			{
				/* NE555 setup as pulse position modulator
                 * C = 0.01u, Ra = 33k, Rb = 47k
                 * frequency = 1.44 / ((33k + 2*47k) * 0.01u) = 1134Hz
                 * modulated by squeal_amp
                 */
				squeal_off_counter -= (1134 + 1134 * squeal_amp / 32767) / 3;
				while( squeal_off_counter <= 0 )
				{
					squeal_off_counter += OUTPUT_RATE;
					squeal_out = 0;
				}
			}
			else
			{
				squeal_on_counter -= 1134;
				while( squeal_on_counter <= 0 )
				{
					squeal_on_counter += OUTPUT_RATE;
					squeal_out = 1;
                }
            }
		}

		/* mix sequal sound at 40% */
        if( squeal_out )
			sum += 32767 * 40 / 100;

		*buffer++ = sum;
	}
}

void *redbaron_sh_start(int clock, const struct CustomSound_interface *config)
{
    int i;

	vol_lookup = (INT16 *)auto_malloc(32768 * sizeof(INT16));
    for( i = 0; i < 0x8000; i++ )
		vol_lookup[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

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
		vol_crash[i] = 32767 * r0 / (r0 + r1);
    }

	channel = stream_create(0, 1, OUTPUT_RATE, 0, redbaron_sound_update);

    return auto_malloc(1);
}
