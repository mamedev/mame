/***************************************************************************

  t6w28.c (based on sn74696.c)

  The t6w28 sound core is used in the SNK NeoGeo Pocket. It is a stereo
  sound chip based on 2 partial sn76498a cores.

  The block diagram for this chip is as follows:

Offset 0:
        Tone 0          /---------->   Att0  ---\
                        |                       |
        Tone 1          |  /------->   Att1  ---+
                        |  |                    |    Right
        Tone 2          |  |  /---->   Att2  ---+-------->
         |              |  |  |                 |
        Noise   -----+------------->   Att3  ---/
                     |  |  |  |
                     |  |  |  |
 Offset 1:           |  |  |  |
        Tone 0  --------+---------->   Att0  ---\
                     |     |  |                 |
        Tone 1  -----------+------->   Att1  ---+
                     |        |                 |     Left
        Tone 2  --------------+---->   Att2  ---+-------->
                     |                          |
        Noise        \------------->   Att3  ---/


***************************************************************************/

#include "emu.h"
#include "t6w28.h"


#define MAX_OUTPUT 0x7fff

#define STEP 0x10000

struct t6w28_state
{
	sound_stream * Channel;
	int SampleRate;
	int VolTable[16];	/* volume table         */
	INT32 Register[16];	/* registers */
	INT32 LastRegister[2];	/* last register written */
	INT32 Volume[8];	/* volume of voice 0-2 and noise */
	UINT32 RNG[2];		/* noise generator      */
	INT32 NoiseMode[2];	/* active noise mode */
	INT32 FeedbackMask;     /* mask for feedback */
	INT32 WhitenoiseTaps;   /* mask for white noise taps */
	INT32 WhitenoiseInvert; /* white noise invert flag */
	INT32 Period[8];
	INT32 Count[8];
	INT32 Output[8];
};


INLINE t6w28_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == T6W28);
	return (t6w28_state *)downcast<t6w28_device *>(device)->token();
}


WRITE8_DEVICE_HANDLER( t6w28_w )
{
	t6w28_state *R = get_safe_token(device);
	int n, r, c;


	/* update the output buffer before changing the registers */
	R->Channel->update();

	offset &= 1;

	if (data & 0x80)
	{
		r = (data & 0x70) >> 4;
		R->LastRegister[offset] = r;
		R->Register[offset * 8 + r] = (R->Register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
	}
	else
    {
		r = R->LastRegister[offset];
	}
	c = r/2;
	switch (r)
	{
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
		    if ((data & 0x80) == 0) R->Register[offset * 8 + r] = (R->Register[offset * 8 + r] & 0x0f) | ((data & 0x3f) << 4);
			R->Period[offset * 4 + c] = STEP * R->Register[offset * 8 + r];
			if (R->Period[offset * 4 + c] == 0) R->Period[offset * 4 + c] = STEP;
			if (r == 4)
			{
				/* update noise shift frequency */
				if ((R->Register[offset * 8 + 6] & 0x03) == 0x03)
					R->Period[offset * 4 + 3] = 2 * R->Period[offset * 4 + 2];
			}
			break;
		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
			R->Volume[offset * 4 + c] = R->VolTable[data & 0x0f];
			if ((data & 0x80) == 0) R->Register[offset * 8 + r] = (R->Register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
			break;
		case 6:	/* noise  : frequency, mode */
			{
			        if ((data & 0x80) == 0) R->Register[offset * 8 + r] = (R->Register[offset * 8 + r] & 0x3f0) | (data & 0x0f);
				n = R->Register[offset * 8 + 6];
				R->NoiseMode[offset] = (n & 4) ? 1 : 0;
				/* N/512,N/1024,N/2048,Tone #3 output */
				R->Period[offset * 4 + 3] = ((n&3) == 3) ? 2 * R->Period[offset * 4 + 2] : (STEP << (5+(n&3)));
			        /* Reset noise shifter */
				R->RNG[offset] = R->FeedbackMask; /* this is correct according to the smspower document */
				//R->RNG = 0xF35; /* this is not, but sounds better in do run run */
				R->Output[offset * 4 + 3] = R->RNG[offset] & 1;
			}
			break;
	}
}



static STREAM_UPDATE( t6w28_update )
{
	int i;
	t6w28_state *R = (t6w28_state *)param;
	stream_sample_t *buffer0 = outputs[0];
	stream_sample_t *buffer1 = outputs[1];


	/* If the volume is 0, increase the counter */
	for (i = 0;i < 8;i++)
	{
		if (R->Volume[i] == 0)
		{
			/* note that I do count += samples, NOT count = samples + 1. You might think */
			/* it's the same since the volume is 0, but doing the latter could cause */
			/* interferencies when the program is rapidly modulating the volume. */
			if (R->Count[i] <= samples*STEP) R->Count[i] += samples*STEP;
		}
	}

	while (samples > 0)
	{
		int vol[8];
		unsigned int out0, out1;
		int left;


		/* vol[] keeps track of how long each square wave stays */
		/* in the 1 position during the sample period. */
		vol[0] = vol[1] = vol[2] = vol[3] = vol[4] = vol[5] = vol[6] = vol[7] = 0;

		for (i = 2;i < 3;i++)
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

		for (i = 4;i < 7;i++)
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
		        if (R->NoiseMode[0] == 1) /* White Noise Mode */
		        {
			        if (((R->RNG[0] & R->WhitenoiseTaps) != R->WhitenoiseTaps) && ((R->RNG[0] & R->WhitenoiseTaps) != 0)) /* crappy xor! */
					{
				        R->RNG[0] >>= 1;
				        R->RNG[0] |= R->FeedbackMask;
					}
					else
					{
				        R->RNG[0] >>= 1;
					}
					R->Output[3] = R->WhitenoiseInvert ? !(R->RNG[0] & 1) : R->RNG[0] & 1;
				}
				else /* Periodic noise mode */
				{
			        if (R->RNG[0] & 1)
					{
				        R->RNG[0] >>= 1;
				        R->RNG[0] |= R->FeedbackMask;
					}
					else
					{
				        R->RNG[0] >>= 1;
					}
					R->Output[3] = R->RNG[0] & 1;
				}
				R->Count[3] += R->Period[3];
				if (R->Output[3]) vol[3] += R->Period[3];
			}
			if (R->Output[3]) vol[3] -= R->Count[3];

			left -= nextevent;
		} while (left > 0);

		out0 = vol[4] * R->Volume[4] + vol[5] * R->Volume[5] +
				vol[6] * R->Volume[6] + vol[3] * R->Volume[7];

		out1 = vol[4] * R->Volume[0] + vol[5] * R->Volume[1] +
				vol[6] * R->Volume[2] + vol[3] * R->Volume[3];

		if (out0 > MAX_OUTPUT * STEP) out0 = MAX_OUTPUT * STEP;
		if (out1 > MAX_OUTPUT * STEP) out1 = MAX_OUTPUT * STEP;

		*(buffer0++) = out0 / STEP;
		*(buffer1++) = out1 / STEP;

		samples--;
	}
}



static void t6w28_set_gain(t6w28_state *R,int gain)
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



static int t6w28_init(device_t *device, t6w28_state *R)
{
	int sample_rate = device->clock()/16;
	int i;

	R->Channel = device->machine().sound().stream_alloc(*device,0,2,sample_rate,R,t6w28_update);

	R->SampleRate = sample_rate;

	for (i = 0;i < 8;i++) R->Volume[i] = 0;

	R->LastRegister[0] = 0;
	R->LastRegister[1] = 0;
	for (i = 0;i < 8;i+=2)
	{
		R->Register[i] = 0;
		R->Register[i + 1] = 0x0f;	/* volume = 0 */
	}

	for (i = 0;i < 8;i++)
	{
		R->Output[i] = 0;
		R->Period[i] = R->Count[i] = STEP;
	}

	/* Default is SN76489 non-A */
	R->FeedbackMask = 0x4000;     /* mask for feedback */
	R->WhitenoiseTaps = 0x03;   /* mask for white noise taps */
	R->WhitenoiseInvert = 1; /* white noise invert flag */

	R->RNG[0] = R->FeedbackMask;
	R->RNG[1] = R->FeedbackMask;
	R->Output[3] = R->RNG[0] & 1;

	return 0;
}


static DEVICE_START( t6w28 )
{
	t6w28_state *chip = get_safe_token(device);

	if (t6w28_init(device,chip) != 0)
		fatalerror("Error creating t6w28 chip\n");
	t6w28_set_gain(chip, 0);

	/* values from sn76489a */
	chip->FeedbackMask = 0x8000;
	chip->WhitenoiseTaps = 0x06;
	chip->WhitenoiseInvert = FALSE;

	device->save_item(NAME(chip->Register));
	device->save_item(NAME(chip->LastRegister));
	device->save_item(NAME(chip->Volume));
	device->save_item(NAME(chip->RNG));
	device->save_item(NAME(chip->NoiseMode));
	device->save_item(NAME(chip->Period));
	device->save_item(NAME(chip->Count));
	device->save_item(NAME(chip->Output));
}

const device_type T6W28 = &device_creator<t6w28_device>;

t6w28_device::t6w28_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, T6W28, "T6W28", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(t6w28_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void t6w28_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void t6w28_device::device_start()
{
	DEVICE_START_NAME( t6w28 )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void t6w28_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


