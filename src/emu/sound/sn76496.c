/***************************************************************************

  sn76496.c
  by Nicola Salmoria

  Routines to emulate the:
  Texas Instruments SN76489, SN76489A, SN76494/SN76496
  ( Also known as, or at least compatible with, the TMS9919.)
  and the Sega 'PSG' used on the Master System, Game Gear, and Megadrive/Genesis
  This chip is known as the Programmable Sound Generator, or PSG, and is a 4
  channel sound generator, with three squarewave channels and a noise/arbitrary
  duty cycle channel.

  Noise emulation for all chips should be accurate:
  SN76489 uses a 15-bit shift register with taps on bits D and E, output on /E
  * It uses a 15-bit ring buffer for periodic noise/arbitrary duty cycle.
  SN76489A uses a 16-bit shift register with taps on bits D and F, output on F
  * It uses a 16-bit ring buffer for periodic noise/arbitrary duty cycle.
  SN76494 and SN76496 are PROBABLY identical in operation to the SN76489A
  * They have an audio input line which is mixed with the 4 channels of output.
  Sega Master System III/MD/Genesis PSG uses a 16-bit shift register with taps
  on bits C and F, output on F
  * It uses a 16-bit ring buffer for periodic noise/arbitrary duty cycle.
  Sega Game Gear PSG is identical to the SMS3/MD/Genesis one except it has an
  extra register for mapping which channels go to which speaker.

  28/03/2005 : Sebastien Chevalier
  Update th SN76496Write func, according to SN76489 doc found on SMSPower.
   - On write with 0x80 set to 0, when LastRegister is other then TONE,
   the function is similar than update with 0x80 set to 1

  23/04/2007 : Lord Nightmare
  Major update, implement all three different noise generation algorithms plus
  the game gear stereo output, and a set_variant call to discern among them.
***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "sn76496.h"


#define MAX_OUTPUT 0x7fff

#define STEP 0x10000

struct SN76496
{
	sound_stream * Channel;
	int SampleRate;
	int VolTable[16];	/* volume table         */
	INT32 Register[8];	/* registers */
	INT32 LastRegister;	/* last register written */
	INT32 Volume[4];	/* volume of voice 0-2 and noise */
	UINT32 RNG;		/* noise generator      */
	INT32 NoiseMode;	/* active noise mode */
	INT32 FeedbackMask;     /* mask for feedback */
	INT32 WhitenoiseTaps;   /* mask for white noise taps */
	INT32 WhitenoiseInvert; /* white noise invert flag */
	INT32 Period[4];
	INT32 Count[4];
	INT32 Output[4];
};



static void SN76496Write(int chip,int data)
{
	struct SN76496 *R = sndti_token(SOUND_SN76496, chip);
	int n, r, c;


	/* update the output buffer before changing the registers */
	stream_update(R->Channel);

	if (data & 0x80)
	{
		r = (data & 0x70) >> 4;
		R->LastRegister = r;
		R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
	}
	else
    {
		r = R->LastRegister;
	}
	c = r/2;
	switch (r)
	{
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
		    if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x0f) | ((data & 0x3f) << 4);
			R->Period[c] = STEP * R->Register[r];
			if (R->Period[c] == 0) R->Period[c] = STEP;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((R->Register[6] & 0x03) == 0x03)
					R->Period[3] = 2 * R->Period[2];
			}
			break;
		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
			R->Volume[c] = R->VolTable[data & 0x0f];
			if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
			break;
		case 6:	/* noise  : frequency, mode */
			{
			        if ((data & 0x80) == 0) R->Register[r] = (R->Register[r] & 0x3f0) | (data & 0x0f);
				n = R->Register[6];
				R->NoiseMode = (n & 4) ? 1 : 0;
				/* N/512,N/1024,N/2048,Tone #3 output */
				R->Period[3] = ((n&3) == 3) ? 2 * R->Period[2] : (STEP << (5+(n&3)));
			        /* Reset noise shifter */
				R->RNG = R->FeedbackMask; /* this is correct according to the smspower document */
				//R->RNG = 0xF35; /* this is not, but sounds better in do run run */
				R->Output[3] = R->RNG & 1;
			}
			break;
	}
}



WRITE8_HANDLER( SN76496_0_w ) {	SN76496Write(0,data); }
WRITE8_HANDLER( SN76496_1_w ) {	SN76496Write(1,data); }
WRITE8_HANDLER( SN76496_2_w ) {	SN76496Write(2,data); }
WRITE8_HANDLER( SN76496_3_w ) {	SN76496Write(3,data); }
WRITE8_HANDLER( SN76496_4_w ) {	SN76496Write(4,data); }


static void SN76496Update(void *param,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
	int i;
	struct SN76496 *R = param;
	stream_sample_t *buffer = _buffer[0];


	/* If the volume is 0, increase the counter */
	for (i = 0;i < 4;i++)
	{
		if (R->Volume[i] == 0)
		{
			/* note that I do count += length, NOT count = length + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (R->Count[i] <= length*STEP) R->Count[i] += length*STEP;
		}
	}

	while (length > 0)
	{
		int vol[4];
		unsigned int out;
		int left;


		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = 0;

		for (i = 0;i < 3;i++)
		{
			if (R->Output[i]) vol[i] += R->Count[i];
			R->Count[i] -= STEP;
			/* Period[i] is the half period of the square wave. Here, in each */
			/* loop I add Period[i] twice, so that at the end of the loop the */
			/* square wave is in the same status (0 or 1) it was at the start. */
			/* vol[i] is also incremented by Period[i], since the wave has been 1 */
			/* exactly half of the time, regardless of the initial position. */
			/* If we exit the loop in the middle, Output[i] has to be inverted */
			/* and vol[i] incremented only if the exit status of the square */
			/* wave is 1. */
			while (R->Count[i] <= 0)
			{
				R->Count[i] += R->Period[i];
				if (R->Count[i] > 0)
				{
					R->Output[i] ^= 1;
					if (R->Output[i]) vol[i] += R->Period[i];
					break;
				}
				R->Count[i] += R->Period[i];
				vol[i] += R->Period[i];
			}
			if (R->Output[i]) vol[i] -= R->Count[i];
		}

		left = STEP;
		do
		{
			int nextevent;


			if (R->Count[3] < left) nextevent = R->Count[3];
			else nextevent = left;

			if (R->Output[3]) vol[3] += R->Count[3];
			R->Count[3] -= nextevent;
			if (R->Count[3] <= 0)
			{
		        if (R->NoiseMode == 1) /* White Noise Mode */
		        {
			        if (((R->RNG & R->WhitenoiseTaps) != R->WhitenoiseTaps) && ((R->RNG & R->WhitenoiseTaps) != 0)) /* crappy xor! */
					{
				        R->RNG >>= 1;
				        R->RNG |= R->FeedbackMask;
					}
					else
					{
				        R->RNG >>= 1;
					}
					R->Output[3] = R->WhitenoiseInvert ? !(R->RNG & 1) : R->RNG & 1;
				}
				else /* Periodic noise mode */
				{
			        if (R->RNG & 1)
					{
				        R->RNG >>= 1;
				        R->RNG |= R->FeedbackMask;
					}
					else
					{
				        R->RNG >>= 1;
					}
					R->Output[3] = R->RNG & 1;
				}
				R->Count[3] += R->Period[3];
				if (R->Output[3]) vol[3] += R->Period[3];
			}
			if (R->Output[3]) vol[3] -= R->Count[3];

			left -= nextevent;
		} while (left > 0);

		out = vol[0] * R->Volume[0] + vol[1] * R->Volume[1] +
				vol[2] * R->Volume[2] + vol[3] * R->Volume[3];

		if (out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;

		*(buffer++) = out / STEP;

		length--;
	}
}



static void SN76496_set_gain(struct SN76496 *R,int gain)
{
	int i;
	double out;


	gain &= 0xff;

	/* increase max output basing on gain (0.2 dB per step) */
	out = MAX_OUTPUT / 3;
	while (gain-- > 0)
		out *= 1.023292992;	/* = (10 ^ (0.2/20)) */

	/* build volume table (2dB per step) */
	for (i = 0;i < 15;i++)
	{
		/* limit volume to avoid clipping */
		if (out > MAX_OUTPUT / 3) R->VolTable[i] = MAX_OUTPUT / 3;
		else R->VolTable[i] = out;

		out /= 1.258925412;	/* = 10 ^ (2/20) = 2dB */
	}
	R->VolTable[15] = 0;
}



static int SN76496_init(struct SN76496 *R,int sndindex,int clock)
{
	int sample_rate = clock/16;
	int i;

	R->Channel = stream_create(0,1, sample_rate,R,SN76496Update);

	R->SampleRate = sample_rate;

	for (i = 0;i < 4;i++) R->Volume[i] = 0;

	R->LastRegister = 0;
	for (i = 0;i < 8;i+=2)
	{
		R->Register[i] = 0;
		R->Register[i + 1] = 0x0f;	/* volume = 0 */
	}

	for (i = 0;i < 4;i++)
	{
		R->Output[i] = 0;
		R->Period[i] = R->Count[i] = STEP;
	}

	/* Default is SN76489 non-A */
	R->FeedbackMask = 0x4000;     /* mask for feedback */
	R->WhitenoiseTaps = 0x03;   /* mask for white noise taps */
	R->WhitenoiseInvert = 1; /* white noise invert flag */

	R->RNG = R->FeedbackMask;
	R->Output[3] = R->RNG & 1;

	return 0;
}


static void *generic_start(int sndindex, int clock, int feedbackmask, int noisetaps, int noiseinvert)
{
	struct SN76496 *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	if (SN76496_init(chip,sndindex,clock) != 0)
		return NULL;
	SN76496_set_gain(chip, 0);

	chip->FeedbackMask = feedbackmask;
	chip->WhitenoiseTaps = noisetaps;
	chip->WhitenoiseInvert = noiseinvert;

	state_save_register_item_array("sn76496", sndindex, chip->Register);
	state_save_register_item("sn76496", sndindex, chip->LastRegister);
	state_save_register_item_array("sn76496", sndindex, chip->Volume);
	state_save_register_item("sn76496", sndindex, chip->RNG);
	state_save_register_item("sn76496", sndindex, chip->NoiseMode);
	state_save_register_item_array("sn76496", sndindex, chip->Period);
	state_save_register_item_array("sn76496", sndindex, chip->Count);
	state_save_register_item_array("sn76496", sndindex, chip->Output);

	return chip;

}


static void *sn76489_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x4000, 0x03, TRUE);
}

static void *sn76489a_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x8000, 0x06, FALSE);
}

static void *sn76494_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x8000, 0x06, FALSE);
}

static void *sn76496_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x8000, 0x06, FALSE);
}

static void *gamegear_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x8000, 0x09, FALSE);
}

static void *smsiii_start(int sndindex, int clock, const void *config)
{
	return generic_start(sndindex, clock, 0x8000, 0x09, FALSE);
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void sn76496_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void sn76496_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_ALIAS:							info->i = SOUND_SN76496;				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = sn76496_set_info;		break;
		case SNDINFO_PTR_START:							info->start = sn76496_start;			break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "SN76496";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "TI PSG";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.1";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, 2007 The MAME Team"; break;
	}
}

void sn76489_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = sn76489_start;			break;
		case SNDINFO_STR_NAME:							info->s = "SN76489";					break;
		default: 										sn76496_get_info(token, state, info);	break;
	}
}

void sn76489a_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = sn76489a_start;			break;
		case SNDINFO_STR_NAME:							info->s = "SN76489A";					break;
		default: 										sn76496_get_info(token, state, info);	break;
	}
}

void sn76494_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = sn76494_start;			break;
		case SNDINFO_STR_NAME:							info->s = "SN76494";					break;
		default: 										sn76496_get_info(token, state, info);	break;
	}
}

void gamegear_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = gamegear_start;			break;
		case SNDINFO_STR_NAME:							info->s = "Game Gear PSG";				break;
		default: 										sn76496_get_info(token, state, info);	break;
	}
}

void smsiii_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		case SNDINFO_PTR_START:							info->start = smsiii_start;				break;
		case SNDINFO_STR_NAME:							info->s = "SMSIII PSG";					break;
		default: 										sn76496_get_info(token, state, info);	break;
	}
}
