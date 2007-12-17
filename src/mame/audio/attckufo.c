/***************************************************************************

  Attack ufo sound emulation
  based on MOS 6560 emulator by
  PeT mess@utanet.at

***************************************************************************/
#include <math.h>
#include "driver.h"
#include "streams.h"
#include "sound/custom.h"
#include "includes/attckufo.h"

/*
 * assumed model:
 * each write to a ton/noise generated starts it new
 * each generator behaves like an timer
 * when it reaches 0, the next samplevalue is given out
 */

/*
 * noise channel
 * based on a document by diku0748@diku.dk (Asger Alstrup Nielsen)
 *
 * 23 bit shift register
 * initial value (0x7ffff8)
 * after shift bit 0 is set to bit 22 xor bit 17
 * dac sample bit22 bit20 bit16 bit13 bit11 bit7 bit4 bit2(lsb)
 *
 * emulation:
 * allocate buffer for 5 sec sampledata (fastest played frequency)
 * and fill this buffer in init with the required sample
 * fast turning off channel, immediate change of frequency
 */

#define NOISE_BUFFER_SIZE_SEC 5


#define TONE1_ON (attckufo_regs[0xa]&0x80)
#define TONE2_ON (attckufo_regs[0xb]&0x80)
#define TONE3_ON (attckufo_regs[0xc]&0x80)
#define NOISE_ON (attckufo_regs[0xd]&0x80)
#define VOLUME (attckufo_regs[0xe]&0x0f)

#define OUTPUT_RATE		(14318181/14/32)

#define TONE_FREQUENCY_MIN  ((14318181/14)/256/128)
#define TONE1_VALUE (8*(128-((attckufo_regs[0xa]+1)&0x7f)))
#define TONE1_FREQUENCY ((14318181/14)/32/TONE1_VALUE)
#define TONE2_VALUE (4*(128-((attckufo_regs[0xb]+1)&0x7f)))
#define TONE2_FREQUENCY ((14318181/14)/32/TONE2_VALUE)
#define TONE3_VALUE (2*(128-((attckufo_regs[0xc]+1)&0x7f)))
#define TONE3_FREQUENCY ((14318181/14)/32/TONE3_VALUE)
#define NOISE_VALUE (32*(128-((attckufo_regs[0xd]+1)&0x7f)))
#define NOISE_FREQUENCY ((14318181/14)/NOISE_VALUE)
#define NOISE_FREQUENCY_MAX ((14318181/14)/32/1)

static int tone1pos = 0, tone2pos = 0, tone3pos = 0,
	tonesize, tone1samples = 1, tone2samples = 1, tone3samples = 1,
	noisesize,		  /* number of samples */
	noisepos = 0,         /* pos of tone */
	noisesamples = 1;	  /* count of samples to give out per tone */

static sound_stream *channel;
static INT16 *tone;

static INT8 *noise;

void attckufo_soundport_w (int offset, int data)
{
  int old = attckufo_regs[offset];
	stream_update(channel);
	switch (offset)
	{
	case 0xa:
		attckufo_regs[offset] = data;
		if (!(old & 0x80) && TONE1_ON)
		{
			tone1pos = 0;
			tone1samples = OUTPUT_RATE / TONE1_FREQUENCY;
			if (tone1samples == 0)
				tone1samples = 1;
		}

		break;
	case 0xb:
		attckufo_regs[offset] = data;
		if (!(old & 0x80) && TONE2_ON)
		{
			tone2pos = 0;
			tone2samples = OUTPUT_RATE / TONE2_FREQUENCY;
			if (tone2samples == 0)
				tone2samples = 1;
		}

		break;
	case 0xc:
		attckufo_regs[offset] = data;
		if (!(old & 0x80) && TONE3_ON)
		{
			tone3pos = 0;
			tone3samples = OUTPUT_RATE / TONE3_FREQUENCY;
			if (tone2samples == 0)
				tone2samples = 1;
		}

		break;
	case 0xd:
		attckufo_regs[offset] = data;
		if (NOISE_ON)
		{
			noisesamples = (int) ((double) NOISE_FREQUENCY_MAX * OUTPUT_RATE
								  * NOISE_BUFFER_SIZE_SEC / NOISE_FREQUENCY);

			if ((double) noisepos / noisesamples >= 1.0)
			{
				noisepos = 0;
			}
		}
		else
		{
			noisepos = 0;
		}
		break;
	case 0xe:
		attckufo_regs[offset] = (old & ~0xf) | (data & 0xf);

		break;
	}
}

/************************************/
/* Sound handler update             */
/************************************/
static void attckufo_update (void *param,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
	int i, v;
	stream_sample_t *buffer = _buffer[0];

	for (i = 0; i < length; i++)
	{
		v = 0;
		if (TONE1_ON /*||(tone1pos!=0) */ )
		{
			v += tone[tone1pos * tonesize / tone1samples];
			tone1pos++;

			if (tone1pos >= tone1samples)
			{
				tone1pos = 0;
				tone1samples = OUTPUT_RATE / TONE1_FREQUENCY;
				if (tone1samples == 0)
					tone1samples = 1;
			}

		}
		if (TONE2_ON  )
		{
			v += tone[tone2pos * tonesize / tone2samples];
			tone2pos++;
			if (tone2pos >= tone2samples)
			{
				tone2pos = 0;
				tone2samples = OUTPUT_RATE / TONE2_FREQUENCY;
				if (tone2samples == 0)
					tone2samples = 1;
			}

		}
		if (TONE3_ON  )
		{
			v += tone[tone3pos * tonesize / tone3samples];
			tone3pos++;

			if (tone3pos >= tone3samples)
			{
				tone3pos = 0;
				tone3samples = OUTPUT_RATE / TONE3_FREQUENCY;
				if (tone3samples == 0)
					tone3samples = 1;
			}

		}
		if (NOISE_ON)
		{
			v += noise[(int) ((double) noisepos * noisesize / noisesamples)];
			noisepos++;
			if ((double) noisepos / noisesamples >= 1.0)
			{
				noisepos = 0;
			}
		}
		v = (v * VOLUME) << 2;
		if (v > 32767)
			buffer[i] = 32767;
		else if (v < -32767)
			buffer[i] = -32767;
		else
			buffer[i] = v;



	}
}

/************************************/
/* Sound handler start          */
/************************************/


void *attckufo_custom_start(int clock, const struct CustomSound_interface *config)
{
	int i;

	channel = stream_create(0, 1, OUTPUT_RATE, 0, attckufo_update);


	/* buffer for fastest played sample for 5 second
     * so we have enough data for min 5 second */
	noisesize = NOISE_FREQUENCY_MAX * NOISE_BUFFER_SIZE_SEC;
	noise = (INT8*) auto_malloc (noisesize * sizeof (noise[0]));
	{
		int noiseshift = 0x7ffff8;
		char data;

		for (i = 0; i < noisesize; i++)
		{
			data = 0;
			if (noiseshift & 0x400000)
				data |= 0x80;
			if (noiseshift & 0x100000)
				data |= 0x40;
			if (noiseshift & 0x010000)
				data |= 0x20;
			if (noiseshift & 0x002000)
				data |= 0x10;
			if (noiseshift & 0x000800)
				data |= 0x08;
			if (noiseshift & 0x000080)
				data |= 0x04;
			if (noiseshift & 0x000010)
				data |= 0x02;
			if (noiseshift & 0x000004)
				data |= 0x01;
			noise[i] = data;
			if (((noiseshift & 0x400000) == 0) != ((noiseshift & 0x002000) == 0))
				noiseshift = (noiseshift << 1) | 1;
			else
				noiseshift <<= 1;
		}
	}
	tonesize = OUTPUT_RATE / TONE_FREQUENCY_MIN;

	tone = (INT16*) auto_malloc (tonesize * sizeof (tone[0]));

	for (i = 0; i < tonesize; i++)
	{
		tone[i] = (INT16)(sin (2 * M_PI * i / tonesize) * 127 + 0.5);
	}
	return (void *) ~0;
}

