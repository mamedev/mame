/**************************************************************************

    Votrax SC-01 Emulator

    Mike@Dissfulfils.co.uk

**************************************************************************

sh_votrax_start  - Start emulation, load samples from Votrax subdirectory
sh_votrax_stop   - End emulation, free memory used for samples
votrax_w         - Write data to votrax port
votrax_status    - Return busy status (-1 = busy)

If you need to alter the base frequency (i.e. Qbert) then just alter
the variable VotraxBaseFrequency, this is defaulted to 8000

**************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "samples.h"


struct votrax_info
{
	int		stream;
	int		frequency;		/* Some games (Qbert) change this */
	int 	volume;
	sound_stream * 	channel;

	struct loaded_sample *sample;
	UINT32		pos;
	UINT32		frac;
	UINT32		step;

	struct  loaded_samples *samples;
};

#define FRAC_BITS		24
#define FRAC_ONE		(1 << FRAC_BITS)
#define FRAC_MASK		(FRAC_ONE - 1)


/****************************************************************************
 * 64 Phonemes - currently 1 sample per phoneme, will be combined sometime!
 ****************************************************************************/

static const char *VotraxTable[65] =
{
 "EH3","EH2","EH1","PA0","DT" ,"A1" ,"A2" ,"ZH",
 "AH2","I3" ,"I2" ,"I1" ,"M"  ,"N"  ,"B"  ,"V",
 "CH" ,"SH" ,"Z"  ,"AW1","NG" ,"AH1","OO1","OO",
 "L"  ,"K"  ,"J"  ,"H"  ,"G"  ,"F"  ,"D"  ,"S",
 "A"  ,"AY" ,"Y1" ,"UH3","AH" ,"P"  ,"O"  ,"I",
 "U"  ,"Y"  ,"T"  ,"R"  ,"E"  ,"W"  ,"AE" ,"AE1",
 "AW2","UH2","UH1","UH" ,"O2" ,"O1" ,"IU" ,"U1",
 "THV","TH" ,"ER" ,"EH" ,"E1" ,"AW" ,"PA1","STOP",
 0
};

static void votrax_update_sound(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct votrax_info *info = param;
	stream_sample_t *buffer = _buffer[0];

	if (info->sample)
	{
		/* load some info locally */
		UINT32 pos = info->pos;
		UINT32 frac = info->frac;
		UINT32 step = info->step;
		UINT32 length = info->sample->length;
		INT16 *sample = info->sample->data;

		while (length--)
		{
			/* do a linear interp on the sample */
			INT32 sample1 = sample[pos];
			INT32 sample2 = sample[(pos + 1) % length];
			INT32 fracmult = frac >> (FRAC_BITS - 14);
			*buffer++ = ((0x4000 - fracmult) * sample1 + fracmult * sample2) >> 14;

			/* advance */
			frac += step;
			pos += frac >> FRAC_BITS;
			frac = frac & ((1 << FRAC_BITS) - 1);

			/* handle looping/ending */
			if (pos >= length)
			{
				info->sample = NULL;
				break;
			}
		}

		/* push position back out */
		info->pos = pos;
		info->frac = frac;
	}
}


static void *votrax_start(int sndindex, int clock, const void *config)
{
	struct votrax_info *votrax;

	votrax = auto_malloc(sizeof(*votrax));
	memset(votrax, 0, sizeof(*votrax));

	votrax->samples = readsamples(VotraxTable,"votrax");
    votrax->frequency = 8000;
    votrax->volume = 230;

    votrax->channel = stream_create(0, 1, Machine->sample_rate, votrax, votrax_update_sound);

	votrax->sample = NULL;
	votrax->step = 0;

	return votrax;
}


void votrax_w(int data)
{
	struct votrax_info *info = sndti_token(SOUND_VOTRAX, 0);
	int Phoneme,Intonation;

	stream_update(info->channel);

    Phoneme = data & 0x3F;
    Intonation = data >> 6;

  	logerror("Speech : %s at intonation %d\n",VotraxTable[Phoneme],Intonation);

    if(Phoneme==63)
    	info->sample = NULL;

    if(info->samples->sample[Phoneme].data)
	{
		info->sample = &info->samples->sample[Phoneme];
		info->pos = 0;
		info->frac = 0;
		info->step = ((INT64)(info->sample->frequency + (256*Intonation)) << FRAC_BITS) / Machine->sample_rate;
		stream_set_output_gain(info->channel, 0, (info->volume + (8*Intonation)*100/255) / 100.0);
	}
}

int votrax_status_r(void)
{
	struct votrax_info *info = sndti_token(SOUND_VOTRAX, 0);
	stream_update(info->channel);
    return (info->sample != NULL);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void votrax_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void votrax_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = votrax_set_info;		break;
		case SNDINFO_PTR_START:							info->start = votrax_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Votrax SC-01";				break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Votrax speech";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

