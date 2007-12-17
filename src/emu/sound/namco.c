/***************************************************************************

    NAMCO sound driver.

    This driver handles the four known types of NAMCO wavetable sounds:

        - 3-voice mono (PROM-based design: Pac-Man, Pengo, Dig Dug, etc)
        - 8-voice quadrophonic (Pole Position 1, Pole Position 2)
        - 8-voice mono (custom 15XX: Mappy, Dig Dug 2, etc)
        - 8-voice stereo (System 1)

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "namco.h"


/* 8 voices max */
#define MAX_VOICES 8

#define MAX_VOLUME 16

/* quality parameter: internal sample rate is 192 KHz, output is 48 KHz */
#define INTERNAL_RATE	192000

/* 16 bits: sample bits of the stream buffer    */
/* 4 bits:  volume                  */
/* 4 bits:  prom sample bits            */
#define MIXLEVEL	(1 << (16 - 4 - 4))

/* stream output level */
#define OUTPUT_LEVEL(n)		((n) * MIXLEVEL / chip->num_voices)

/* a position of waveform sample */
#define WAVEFORM_POSITION(n)	(((n) >> chip->f_fracbits) & 0x1f)


/* this structure defines the parameters for a channel */
typedef struct
{
	UINT32 frequency;
	UINT32 counter;
	INT32 volume[2];
	INT32 noise_sw;
	INT32 noise_state;
	INT32 noise_seed;
	UINT32 noise_counter;
	INT32 noise_hold;
	INT32 waveform_select;
} sound_channel;


/* globals available to everyone */
UINT8 *namco_soundregs;
UINT8 *namco_wavedata;

struct namco_sound
{
	/* data about the sound system */
	sound_channel channel_list[MAX_VOICES];
	sound_channel *last_channel;

	/* global sound parameters */
	int wave_size;
	INT32 num_voices;
	INT32 sound_enable;
	sound_stream * stream;
	int namco_clock;
	int sample_rate;
	int f_fracbits;
	int stereo;

	/* decoded waveform table */
	INT16 *waveform[MAX_VOLUME];
};


/* update the decoded waveform data */
static void update_namco_waveform(struct namco_sound *chip, int offset, UINT8 data)
{
	if (chip->wave_size == 1)
	{
		INT16 wdata;
		int v;

		/* use full byte, first 4 high bits, then low 4 bits */
		for (v = 0; v < MAX_VOLUME; v++)
		{
			wdata = ((data >> 4) & 0x0f) - 8;
			chip->waveform[v][offset * 2] = OUTPUT_LEVEL(wdata * v);
			wdata = (data & 0x0f) - 8;
			chip->waveform[v][offset * 2 + 1] = OUTPUT_LEVEL(wdata * v);
		}
	}
	else
	{
		int v;

		/* use only low 4 bits */
		for (v = 0; v < MAX_VOLUME; v++)
			chip->waveform[v][offset] = OUTPUT_LEVEL(((data & 0x0f) - 8) * v);
	}
}


/* build the decoded waveform table */
static int build_decoded_waveform(struct namco_sound *chip, int region)
{
	INT16 *p;
	int size;
	int offset;
	int v;

	/* 20pacgal has waves in RAM but old sound system */
	if (region == -1 && chip->num_voices != 3)
	{
		chip->wave_size = 1;
		size = 32 * 16;		/* 32 samples, 16 waveforms */
	}
	else
	{
		chip->wave_size = 0;
		size = 32 * 8;		/* 32 samples, 8 waveforms */
	}

	p = auto_malloc(size * MAX_VOLUME * sizeof (INT16));

	for (v = 0; v < MAX_VOLUME; v++)
	{
		chip->waveform[v] = p;
		p += size;
	}

	if (region != -1)
		namco_wavedata = memory_region(region);

	/* We need waveform data. It fails if region is not specified. */
	if (namco_wavedata)
	{
		for (offset = 0; offset < 256; offset++)
			update_namco_waveform(chip, offset, namco_wavedata[offset]);
	}

	return 0;
}


/* generate sound by oversampling */
INLINE UINT32 namco_update_one(struct namco_sound *chip, stream_sample_t *buffer, int length, const INT16 *wave, UINT32 counter, UINT32 freq)
{
	while (length-- > 0)
	{
		*buffer++ += wave[WAVEFORM_POSITION(counter)];
		counter += freq;
	}

	return counter;
}


/* generate sound to the mix buffer in mono */
static void namco_update_mono(void *param, stream_sample_t **inputs, stream_sample_t **_buffer, int length)
{
	struct namco_sound *chip = param;
	stream_sample_t *buffer = _buffer[0];
	sound_channel *voice;

	/* zap the contents of the buffer */
	memset(buffer, 0, length * sizeof(*buffer));

	/* if no sound, we're done */
	if (chip->sound_enable == 0)
		return;

	/* loop over each voice and add its contribution */
	for (voice = chip->channel_list; voice < chip->last_channel; voice++)
	{
		stream_sample_t *mix = buffer;
		int v = voice->volume[0];

		if (voice->noise_sw)
		{
			int f = voice->frequency & 0xff;

			/* only update if we have non-zero volume and frequency */
			if (v && f)
			{
				int hold_time = 1 << (chip->f_fracbits - 16);
				int hold = voice->noise_hold;
				UINT32 delta = f << 4;
				UINT32 c = voice->noise_counter;
				INT16 noise_data = OUTPUT_LEVEL(0x07 * (v >> 1));
				int i;

				/* add our contribution */
				for (i = 0; i < length; i++)
				{
					int cnt;

					if (voice->noise_state)
						*mix++ += noise_data;
					else
						*mix++ -= noise_data;

					if (hold)
					{
						hold--;
						continue;
					}

					hold = 	hold_time;

					c += delta;
					cnt = (c >> 12);
					c &= (1 << 12) - 1;
					for( ;cnt > 0; cnt--)
					{
						if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
						if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
						voice->noise_seed >>= 1;
					}
				}

				/* update the counter and hold time for this voice */
				voice->noise_counter = c;
				voice->noise_hold = hold;
			}
		}
		else
		{
			/* only update if we have non-zero volume and frequency */
			if (v && voice->frequency)
			{
				const INT16 *w = &chip->waveform[v][voice->waveform_select * 32];

				/* generate sound into buffer and update the counter for this voice */
				voice->counter = namco_update_one(chip, mix, length, w, voice->counter, voice->frequency);
			}
		}
	}
}


/* generate sound to the mix buffer in stereo */
static void namco_update_stereo(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct namco_sound *chip = param;
	sound_channel *voice;

	/* zap the contents of the buffers */
	memset(buffer[0], 0, length * sizeof(*buffer[0]));
	memset(buffer[1], 0, length * sizeof(*buffer[1]));

	/* if no sound, we're done */
	if (chip->sound_enable == 0)
		return;

	/* loop over each voice and add its contribution */
	for (voice = chip->channel_list; voice < chip->last_channel; voice++)
	{
		stream_sample_t *lmix = buffer[0];
		stream_sample_t *rmix = buffer[1];
		int lv = voice->volume[0];
		int rv = voice->volume[1];

		if (voice->noise_sw)
		{
			int f = voice->frequency & 0xff;

			/* only update if we have non-zero volume and frequency */
			if ((lv || rv) && f)
			{
				int hold_time = 1 << (chip->f_fracbits - 16);
				int hold = voice->noise_hold;
				UINT32 delta = f << 4;
				UINT32 c = voice->noise_counter;
				INT16 l_noise_data = OUTPUT_LEVEL(0x07 * (lv >> 1));
				INT16 r_noise_data = OUTPUT_LEVEL(0x07 * (rv >> 1));
				int i;

				/* add our contribution */
				for (i = 0; i < length; i++)
				{
					int cnt;

					if (voice->noise_state)
					{
						*lmix++ += l_noise_data;
						*rmix++ += r_noise_data;
					}
					else
					{
						*lmix++ -= l_noise_data;
						*rmix++ -= r_noise_data;
					}

					if (hold)
					{
						hold--;
						continue;
					}

					hold = 	hold_time;

					c += delta;
					cnt = (c >> 12);
					c &= (1 << 12) - 1;
					for( ;cnt > 0; cnt--)
					{
						if ((voice->noise_seed + 1) & 2) voice->noise_state ^= 1;
						if (voice->noise_seed & 1) voice->noise_seed ^= 0x28000;
						voice->noise_seed >>= 1;
					}
				}

				/* update the counter and hold time for this voice */
				voice->noise_counter = c;
				voice->noise_hold = hold;
			}
		}
		else
		{
			/* only update if we have non-zero frequency */
			if (voice->frequency)
			{
				/* save the counter for this voice */
				UINT32 c = voice->counter;

				/* only update if we have non-zero left volume */
				if (lv)
				{
					const INT16 *lw = &chip->waveform[lv][voice->waveform_select * 32];

					/* generate sound into the buffer */
					c = namco_update_one(chip, lmix, length, lw, voice->counter, voice->frequency);
				}

				/* only update if we have non-zero right volume */
				if (rv)
				{
					const INT16 *rw = &chip->waveform[rv][voice->waveform_select * 32];

					/* generate sound into the buffer */
					c = namco_update_one(chip, rmix, length, rw, voice->counter, voice->frequency);
				}

				/* update the counter for this voice */
				voice->counter = c;
			}
		}
	}
}


static void *namco_start(int sndindex, int clock, const void *config)
{
	sound_channel *voice;
	const struct namco_interface *intf = config;
	int clock_multiple;
	struct namco_sound *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* extract globals from the interface */
	chip->num_voices = intf->voices;
	chip->last_channel = chip->channel_list + chip->num_voices;
	chip->stereo = intf->stereo;

	/* adjust internal clock */
	chip->namco_clock = clock;
	for (clock_multiple = 0; chip->namco_clock < INTERNAL_RATE; clock_multiple++)
		chip->namco_clock *= 2;

	chip->f_fracbits = clock_multiple + 15;

	/* adjust output clock */
	chip->sample_rate = chip->namco_clock;

	logerror("Namco: freq fractional bits = %d: internal freq = %d, output freq = %d\n", chip->f_fracbits, chip->namco_clock, chip->sample_rate);

	/* build the waveform table */
	if (build_decoded_waveform(chip, intf->region))
		return NULL;

	/* get stream channels */
	if (intf->stereo)
		chip->stream = stream_create(0, 2, chip->sample_rate, chip, namco_update_stereo);
	else
		chip->stream = stream_create(0, 1, chip->sample_rate, chip, namco_update_mono);

	/* start with sound enabled, many games don't have a sound enable register */
	chip->sound_enable = 1;

	/* register with the save state system */
	state_save_register_item("namco", sndindex, chip->num_voices);
	state_save_register_item("namco", sndindex, chip->sound_enable);
	state_save_register_item_pointer("namco", sndindex, chip->waveform[0],
										 MAX_VOLUME * 32 * 8 * (1+chip->wave_size));

	/* reset all the voices */
	for (voice = chip->channel_list; voice < chip->last_channel; voice++)
	{
		int state_index = sndindex * MAX_VOICES + (voice - chip->channel_list);

		voice->frequency = 0;
		voice->volume[0] = voice->volume[1] = 0;
		voice->waveform_select = 0;
		voice->counter = 0;
		voice->noise_sw = 0;
		voice->noise_state = 0;
		voice->noise_seed = 1;
		voice->noise_counter = 0;
		voice->noise_hold = 0;

		/* register with the save state system */
		state_save_register_item("namco", state_index, voice->frequency);
		state_save_register_item("namco", state_index, voice->counter);
		state_save_register_item_array("namco", state_index, voice->volume);
		state_save_register_item("namco", state_index, voice->noise_sw);
		state_save_register_item("namco", state_index, voice->noise_state);
		state_save_register_item("namco", state_index, voice->noise_seed);
		state_save_register_item("namco", state_index, voice->noise_hold);
		state_save_register_item("namco", state_index, voice->noise_counter);
		state_save_register_item("namco", state_index, voice->waveform_select);
	}

	return chip;
}


/********************************************************************************/

/* pacman register map
    0x05:       ch 0    waveform select
    0x0a:       ch 1    waveform select
    0x0f:       ch 2    waveform select

    0x10:       ch 0    the first voice has extra frequency bits
    0x11-0x14:  ch 0    frequency
    0x15:       ch 0    volume

    0x16-0x19:  ch 1    frequency
    0x1a:       ch 1    volume

    0x1b-0x1e:  ch 2    frequency
    0x1f:       ch 2    volume
*/

WRITE8_HANDLER( pacman_sound_enable_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);
	chip->sound_enable = data;
}

WRITE8_HANDLER( pacman_sound_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);
	sound_channel *voice;
	int ch;

	data &= 0x0f;
	if (namco_soundregs[offset] == data)
		return;

	/* update the streams */
	stream_update(chip->stream);

	/* set the register */
	namco_soundregs[offset] = data;

	if (offset < 0x10)
		ch = (offset - 5) / 5;
	else if (offset == 0x10)
		ch = 0;
	else
		ch = (offset - 0x11) / 5;

	if (ch >= chip->num_voices)
		return;

	/* recompute the voice parameters */
	voice = chip->channel_list + ch;
	switch (offset - ch * 5)
	{
	case 0x05:
		voice->waveform_select = data & 7;
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
		/* the frequency has 20 bits */
		/* the first voice has extra frequency bits */
		voice->frequency = (ch == 0) ? namco_soundregs[0x10] : 0;
		voice->frequency += (namco_soundregs[ch * 5 + 0x11] << 4);
		voice->frequency += (namco_soundregs[ch * 5 + 0x12] << 8);
		voice->frequency += (namco_soundregs[ch * 5 + 0x13] << 12);
		voice->frequency += (namco_soundregs[ch * 5 + 0x14] << 16);	/* always 0 */
		break;

	case 0x15:
		voice->volume[0] = data;
		break;
	}
}


/********************************************************************************/

/* polepos register map
Note: even if there are 8 voices, the game doesn't use the first 2 because
it select the 54XX/52XX outputs on those channels

    0x00-0x01   ch 0    frequency
    0x02        ch 0    xxxx---- GAIN 2 volume
    0x03        ch 0    xxxx---- GAIN 3 volume
                        ----xxxx GAIN 4 volume

    0x04-0x07   ch 1

    .
    .
    .

    0x1c-0x1f   ch 7

    0x23        ch 0    xxxx---- GAIN 1 volume
                        -----xxx waveform select
                        ----x-xx channel output select
                                 0-7 (all the same, shared with waveform select) = wave
                                 8 = CHANL1 (54XX pins 17-20)
                                 9 = CHANL2 (54XX pins 8-11)
                                 A = CHANL3 (54XX pins 4-7)
                                 B = CHANL4 (52XX)
    0x27        ch 1
    0x2b        ch 2
    0x2f        ch 3
    0x33        ch 4
    0x37        ch 5
    0x3b        ch 6
    0x3f        ch 7
*/

void polepos_sound_enable(int enable)
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);
	chip->sound_enable = enable;
}

WRITE8_HANDLER( polepos_sound_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);
	sound_channel *voice;
	int ch;

	if (namco_soundregs[offset] == data)
		return;

	/* update the streams */
	stream_update(chip->stream);

	/* set the register */
	namco_soundregs[offset] = data;

	ch = (offset & 0x1f) / 4;

	/* recompute the voice parameters */
	voice = chip->channel_list + ch;
	switch (offset & 0x23)
	{
	case 0x00:
	case 0x01:
		/* the frequency has 16 bits */
		voice->frequency = namco_soundregs[ch * 4 + 0x00];
		voice->frequency += namco_soundregs[ch * 4 + 0x01] << 8;
		break;

	case 0x23:
		voice->waveform_select = data & 7;
		/* fall through */
	case 0x02:
	case 0x03:
		voice->volume[0] = voice->volume[1] = 0;
		// front speakers ?
		voice->volume[0] += namco_soundregs[ch * 4 + 0x03] >> 4;
		voice->volume[1] += namco_soundregs[ch * 4 + 0x03] & 0x0f;
		// rear speakers ?
		voice->volume[0] += namco_soundregs[ch * 4 + 0x23] >> 4;
		voice->volume[1] += namco_soundregs[ch * 4 + 0x02] >> 4;

		voice->volume[0] /= 2;
		voice->volume[1] /= 2;

		/* if 54XX or 52XX selected, silence this voice */
		if (namco_soundregs[ch * 4 + 0x23] & 8)
			voice->volume[0] = voice->volume[1] = 0;
		break;
	}
}


/********************************************************************************/

/* 15XX register map
    0x03        ch 0    volume
    0x04-0x05   ch 0    frequency
    0x06        ch 0    waveform select & frequency

    0x0b        ch 1    volume
    0x0c-0x0d   ch 1    frequency
    0x0e        ch 1    waveform select & frequency

    .
    .
    .

    0x3b        ch 7    volume
    0x3c-0x3d   ch 7    frequency
    0x3e        ch 7    waveform select & frequency
*/

void mappy_sound_enable(int enable)
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO_15XX, 0);
	chip->sound_enable = enable;
}

WRITE8_HANDLER( namco_15xx_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO_15XX, 0);
	sound_channel *voice;
	int ch;

	if (namco_soundregs[offset] == data)
		return;

	/* update the streams */
	stream_update(chip->stream);

	/* set the register */
	namco_soundregs[offset] = data;

	ch = offset / 8;
	if (ch >= chip->num_voices)
		return;

	/* recompute the voice parameters */
	voice = chip->channel_list + ch;
	switch (offset - ch * 8)
	{
	case 0x03:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x06:
		voice->waveform_select = (data >> 4) & 7;
	case 0x04:
	case 0x05:
		/* the frequency has 20 bits */
		voice->frequency = namco_soundregs[ch * 8 + 0x04];
		voice->frequency += namco_soundregs[ch * 8 + 0x05] << 8;
		voice->frequency += (namco_soundregs[ch * 8 + 0x06] & 15) << 16;	/* high bits are from here */
		break;
	}
}


/********************************************************************************/

/* namcos1 register map
    0x00        ch 0    left volume
    0x01        ch 0    waveform select & frequency
    0x02-0x03   ch 0    frequency
    0x04        ch 0    right volume AND
    0x04        ch 1    noise sw

    0x08        ch 1    left volume
    0x09        ch 1    waveform select & frequency
    0x0a-0x0b   ch 1    frequency
    0x0c        ch 1    right volume AND
    0x0c        ch 2    noise sw

    .
    .
    .

    0x38        ch 7    left volume
    0x39        ch 7    waveform select & frequency
    0x3a-0x3b   ch 7    frequency
    0x3c        ch 7    right volume AND
    0x3c        ch 0    noise sw
*/

static WRITE8_HANDLER( namcos1_sound_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO_CUS30, 0);
	sound_channel *voice;
	int ch;
	int nssw;


	/* verify the offset */
	if (offset > 63)
	{
		logerror("NAMCOS1 sound: Attempting to write past the 64 registers segment\n");
		return;
	}

	namco_soundregs = namco_wavedata + 0x100;

	if (namco_soundregs[offset] == data)
		return;

	/* update the streams */
	stream_update(chip->stream);

	/* set the register */
	namco_soundregs[offset] = data;

	ch = offset / 8;
	if (ch >= chip->num_voices)
		return;

	/* recompute the voice parameters */
	voice = chip->channel_list + ch;
	switch (offset - ch * 8)
	{
	case 0x00:
		voice->volume[0] = data & 0x0f;
		break;

	case 0x01:
		voice->waveform_select = (data >> 4) & 15;
	case 0x02:
	case 0x03:
		/* the frequency has 20 bits */
		voice->frequency = (namco_soundregs[ch * 8 + 0x01] & 15) << 16;	/* high bits are from here */
		voice->frequency += namco_soundregs[ch * 8 + 0x02] << 8;
		voice->frequency += namco_soundregs[ch * 8 + 0x03];
		break;

	case 0x04:
		voice->volume[1] = data & 0x0f;

		nssw = ((data & 0x80) >> 7);
		if (++voice == chip->last_channel)
			voice = chip->channel_list;
		voice->noise_sw = nssw;
		break;
	}
}

WRITE8_HANDLER( namcos1_cus30_w )
{
	if (offset < 0x100)
	{
		if (namco_wavedata[offset] != data)
		{
			struct namco_sound *chip = sndti_token(SOUND_NAMCO_CUS30, 0);
			/* update the streams */
			stream_update(chip->stream);

			namco_wavedata[offset] = data;

			/* update the decoded waveform table */
			update_namco_waveform(chip, offset, data);
		}
	}
	else if (offset < 0x140)
		namcos1_sound_w(offset - 0x100,data);
	else
		namco_wavedata[offset] = data;
}

READ8_HANDLER( namcos1_cus30_r )
{
	return namco_wavedata[offset];
}

WRITE8_HANDLER( _20pacgal_wavedata_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);

	if (namco_wavedata[offset] != data)
	{
		/* update the streams */
		stream_update(chip->stream);

		namco_wavedata[offset] = data;

		/* update the decoded waveform table */
		update_namco_waveform(chip, offset, data);
	}
}

/********************************************************************************/

WRITE8_HANDLER( snkwave_w )
{
	struct namco_sound *chip = sndti_token(SOUND_NAMCO, 0);
	static int freq0 = 0xff;
	sound_channel *voice = chip->channel_list;
	if( offset==0 ) freq0 = data;
	if( offset==1 )
	{
		stream_update(chip->stream);
		if( data==0xff || freq0==0 )
		{
			voice->volume[0] = 0x0;
		}
		else
		{
			voice->volume[0] = 0x8;
			voice->frequency = (data<<16)/freq0;
		}
	}
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void namco_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void namco_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = namco_set_info;		break;
		case SNDINFO_PTR_START:							info->start = namco_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Namco";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void namco_15xx_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void namco_15xx_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = namco_15xx_set_info;	break;
		case SNDINFO_PTR_START:							info->start = namco_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Namco 15XX";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void namco_cus30_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void namco_cus30_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = namco_cus30_set_info;	break;
		case SNDINFO_PTR_START:							info->start = namco_start;				break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Namco CUS30";				break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

