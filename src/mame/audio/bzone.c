/*

Battlezone sound info, courtesy of Al Kossow:

D7  motor enable            this enables the engine sound
D6  start LED
D5  sound enable            this enables ALL sound outputs
                            including the POKEY output
D4  engine rev en           this controls the engine speed
                            the engine sound is an integrated square
                            wave (saw tooth) that is frequency modulated
                            by engine rev.
D3  shell loud, soft/       explosion volume
D2  shell enable
D1  explosion loud, soft/   explosion volume
D0  explosion enable        gates a noise generator

*/

#include <math.h>
#include "driver.h"
#include "streams.h"
#include "bzone.h"
#include "sound/custom.h"

#define OUTPUT_RATE (6000*4)


/* Statics */
static INT16 *discharge = NULL;
#define EXP(charge,n) (charge ? 0x7fff - discharge[0x7fff-n] : discharge[n])

static sound_stream *channel;
static int latch;
static int poly_counter;
static int poly_shift;

static int explosion_clock;
static int explosion_out;
static int explosion_amp;
static int explosion_amp_counter;

static int shell_clock;
static int shell_out;
static int shell_amp;
static int shell_amp_counter;

static int motor_counter;
static int motor_counter_a;
static int motor_counter_b;
static int motor_rate;
static int motor_rate_new;
static int motor_rate_counter;
static int motor_amp;
static int motor_amp_new;
static int motor_amp_step;
static int motor_amp_counter;

WRITE8_HANDLER( bzone_sounds_w )
{
	if( data == latch )
		return;

	stream_update(channel);
    latch = data;

    sound_global_enable(latch & 0x20);
}

static void bzone_sound_update(void *param, stream_sample_t **inputs, stream_sample_t **outputs, int length)
{
	stream_sample_t *buffer = outputs[0];
	while( length-- )
	{
		static int last_val = 0;
		int sum = 0;

		/* polynome shifter H5 and H4 (LS164) clocked with 6kHz */
		poly_counter -= 6000;
		while( poly_counter <= 0 )
		{
			int clock;

			poly_counter += OUTPUT_RATE;
			if( ((poly_shift & 0x0008) == 0) == ((poly_shift & 0x4000) == 0) )
				poly_shift = (poly_shift << 1) | 1;
			else
				poly_shift <<= 1;

			/* NAND gate J4 */
			clock = ((poly_shift & 0x7000) == 0x7000) ? 0 : 1;

			/* raising edge on pin 3 of J5 (LS74)? */
			if( clock && !explosion_clock )
				explosion_out ^= 1;

			/* save explo clock level */
			explosion_clock = clock;

			/* input 11 of J5 (LS74) */
			clock = (poly_shift >> 15) & 1;

			/* raising edge on pin 11 of J5 (LS74)? */
			if( clock && !shell_clock )
				shell_out ^= 1;

			/* save shell clock level */
			shell_clock = clock;
		}

		/* explosion enable: charge C14 */
		if( latch & 0x01 )
			explosion_amp = 32767;

		/* explosion output? */
		if( explosion_out )
		{
			if( explosion_amp > 0 )
			{
				/*
                 * discharge C14 through R17 + R16
                 * time constant is 10e-6 * 23000 = 0.23 seconds
                 * (samples were decaying much slower: 1/4th rate? )
                 */
				explosion_amp_counter -= (int)(32767 / (0.23*4));
				if( explosion_amp_counter < 0 )
				{
					int n = (-explosion_amp_counter / OUTPUT_RATE) + 1;
					explosion_amp_counter += n * OUTPUT_RATE;
					if( (explosion_amp -= n) < 0 )
						explosion_amp = 0;
				}
			}
			/*
             * I don't know the amplification of the op-amp
             * and feedback, so the loud/soft values are arbitrary
             */
			if( latch & 0x02 )	/* explosion loud ? */
				sum += EXP(0,explosion_amp)/3;
			else
				sum += EXP(0,explosion_amp)/4;
		}

		/* shell enable: charge C9 */
		if( latch & 0x04 )
			shell_amp = 32767;

		/* shell output? */
		if( shell_out )
		{
			if( shell_amp > 0 )
			{
				/*
                 * discharge C9 through R14 + R15
                 * time constant is 4.7e-6 * 23000 = 0.1081 seconds
                 * (samples were decaying much slower: 1/4th rate? )
                 */
				shell_amp_counter -= (int)(32767 / (0.1081*4));
				if( shell_amp_counter < 0 )
				{
					int n = (-shell_amp_counter / OUTPUT_RATE) + 1;
					shell_amp_counter += n * OUTPUT_RATE;
					if( (shell_amp -= n) < 0 )
						shell_amp = 0;
				}
			}
			/*
             * I don't know the amplification of the op-amp
             * and feedback, so the loud/soft values are arbitrary
             */
			if( latch & 0x08 )	/* shell loud ? */
				sum += EXP(0,shell_amp)/3;
			else
				sum += EXP(0,shell_amp)/4;
		}

		if( latch & 0x80 )
		{
			static double r0 = 1.0/1e12, r1 = 1.0/1e12;

			/* NE5555 timer
             * C = 0.018u, Ra = 100k, Rb = 125k
             * charge time = 0.693 * (Ra + Rb) * C = 3870us
             * discharge time = 0.693 * Rb * C = 1559.25us
             * freq approx. 184 Hz
             * I have no idea what frequencies are coming from the NE555
             * with "MOTOR REV EN" being high or low. I took 240Hz as
             * higher rate and sweep up or down to the new rate in 0.25s
             */
			motor_rate_new = (latch & 0x10) ? 240 : 184;
			if( motor_rate != motor_rate_new )
			{
				/* sweep rate to new rate */
				motor_rate_counter -= (int)((240 - 184) / 0.25);
				while( motor_rate_counter <= 0 )
				{
					motor_rate_counter += OUTPUT_RATE;
					motor_rate += (motor_rate < motor_rate_new) ? +1 : -1;
				}
			}
			motor_counter -= motor_rate;
			while( motor_counter <= 0 )
			{
				motor_counter += OUTPUT_RATE;

				r0 = 1.0/1e12;
				r1 = 1.0/1e12;

				if( ++motor_counter_a == 16 )
					motor_counter_a = 6;
				if( ++motor_counter_b == 16 )
					motor_counter_b = 4;

				if( motor_counter_a & 8 )	/* bit 3 */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;
				if( motor_counter_a == 15 ) /* ripple carry */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;

				if( motor_counter_b & 8 )	/* bit 3 */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;
				if( motor_counter_b == 15 ) /* ripple carry */
					r1 += 1.0/33000;
				else
					r0 += 1.0/33000;

				/* new voltage at C29 */
				r0 = 1.0/r0;
				r1 = 1.0/r1;
				motor_amp_new = (int)(32767 * r0 / (r0 + r1));

				/* charge/discharge C29 (0.47uF) */
				if( motor_amp_new > motor_amp )
					motor_amp_step = (int)((motor_amp_new - motor_amp) / (r1*0.47e-6));
				else
					motor_amp_step = (int)((motor_amp - motor_amp_new) / (r0*0.47e-6));
			}
			if( motor_amp != motor_amp_new )
			{
				motor_amp_counter -= motor_amp_step;
				if( motor_amp_counter < 0 )
				{
					int n = (-motor_amp_counter / OUTPUT_RATE) + 1;
					motor_amp_counter += n * OUTPUT_RATE;
					if( motor_amp > motor_amp_new )
					{
						motor_amp -= n;
						if( motor_amp < motor_amp_new )
							motor_amp = motor_amp_new;
					}
					else
					{
						motor_amp += n;
						if( motor_amp > motor_amp_new )
							motor_amp = motor_amp_new;
					}
				}
			}
			sum += EXP((motor_amp<motor_amp_new),motor_amp)/3;
		}

		*buffer++ = (sum + last_val) / 2;

		/* crude 75% low pass filter */
		last_val = (sum + last_val * 3) / 4;
	}
}

void *bzone_sh_start(int clock, const struct CustomSound_interface *config)
{
    int i;

	discharge = (INT16 *)auto_malloc(32768 * sizeof(INT16));
    for( i = 0; i < 0x8000; i++ )
		discharge[0x7fff-i] = (INT16) (0x7fff/exp(1.0*i/4096));

	channel = stream_create(0, 1, OUTPUT_RATE, 0, bzone_sound_update);

    return auto_malloc(1);
}
