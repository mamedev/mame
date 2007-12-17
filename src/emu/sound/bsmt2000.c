/***************************************************************************

    bsmt2000.c

    BSMT2000 sound emulator.

    Copyright (c) Aaron Giles

    Chip is actually a TMS320C15 DSP with embedded mask rom
    Trivia: BSMT stands for "Brian Schmidt's Mouse Trap"

***************************************************************************/

#include <math.h>

#include "sndintrf.h"
#include "streams.h"
#include "bsmt2000.h"


/***************************************************************************
    DEBUGGING/OPTIONS
***************************************************************************/

#define LOG_COMMANDS			0

/* NOTE: the original chip did not support interpolation, but it sounds */
/* nicer if you enable it. For accuracy's sake, we leave it off by default. */
#define ENABLE_INTERPOLATION	0



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_VOICES				(12+1)
#define ADPCM_VOICE				12



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* struct describing a single playing voice */
typedef struct _bsmt2000_voice bsmt2000_voice;
struct _bsmt2000_voice
{
	UINT16		pos;					/* current position */
	UINT16		rate;					/* stepping value */
	UINT16		loopend;				/* loop end value */
	UINT16		loopstart;				/* loop start value */
	UINT16		bank;					/* bank number */
	UINT16		leftvol;				/* left volume */
	UINT16		rightvol;				/* right volume */
	UINT16		fraction;				/* current fractional position */
};

typedef struct _bsmt2000_chip bsmt2000_chip;
struct _bsmt2000_chip
{
	sound_stream *stream;				/* which stream are we using */
	UINT8		last_register;			/* last register address written */

	INT8 *		region_base;			/* pointer to the base of the region */
	int			total_banks;			/* number of total banks in the region */

	bsmt2000_voice voice[MAX_VOICES];	/* the voices */
	UINT16 *	regmap[128];			/* mapping of registers to voice params */

	UINT32		clock;					/* original clock on the chip */
	UINT8		stereo;					/* stereo output? */
	UINT8		voices;					/* number of voices */
	UINT8		adpcm;					/* adpcm enabled? */

	INT32		adpcm_current;			/* current ADPCM sample */
	INT32		adpcm_delta_n;			/* current ADPCM scale factor */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

const struct BSMT2000interface bsmt2000_interface_region_1 = { REGION_SOUND1 };
const struct BSMT2000interface bsmt2000_interface_region_2 = { REGION_SOUND2 };
const struct BSMT2000interface bsmt2000_interface_region_3 = { REGION_SOUND3 };
const struct BSMT2000interface bsmt2000_interface_region_4 = { REGION_SOUND4 };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* core implementation */
static void *bsmt2000_start(int sndindex, int clock, const void *config);
static void bsmt2000_reset(void *_chip);
static void bsmt2000_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length);

/* read/write access */
static void bsmt2000_reg_write(bsmt2000_chip *chip, offs_t offset, UINT16 data);

/* local functions */
static void set_mode(bsmt2000_chip *chip);
static void set_regmap(bsmt2000_chip *chip, UINT8 posbase, UINT8 ratebase, UINT8 endbase, UINT8 loopbase, UINT8 bankbase, UINT8 rvolbase, UINT8 lvolbase);



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    bsmt2000_start - initialization callback
-------------------------------------------------*/

static void *bsmt2000_start(int sndindex, int clock, const void *config)
{
	const struct BSMT2000interface *intf = config;
	bsmt2000_chip *chip;
	int voicenum;

	/* allocate the chip */
	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* create a stream at a nominal sample rate (real one specified later) */
	chip->stream = stream_create(0, 2, clock / 1000, chip, bsmt2000_update);
	chip->clock = clock;

	/* initialize the regions */
	chip->region_base = (INT8 *)memory_region(intf->region);
	chip->total_banks = memory_region_length(intf->region) / 0x10000;

	/* register chip-wide data for save states */
	state_save_register_item("bsmt2000", sndindex * 16, chip->last_register);
	state_save_register_item("bsmt2000", sndindex * 16, chip->stereo);
	state_save_register_item("bsmt2000", sndindex * 16, chip->voices);
	state_save_register_item("bsmt2000", sndindex * 16, chip->adpcm);
	state_save_register_item("bsmt2000", sndindex * 16, chip->adpcm_current);
	state_save_register_item("bsmt2000", sndindex * 16, chip->adpcm_delta_n);

	/* register voice-specific data for save states */
	for (voicenum = 0; voicenum < MAX_VOICES; voicenum++)
	{
		bsmt2000_voice *voice = &chip->voice[voicenum];
		int index = sndindex * 16 + voicenum;

		state_save_register_item("bsmt2000", index, voice->pos);
		state_save_register_item("bsmt2000", index, voice->rate);
		state_save_register_item("bsmt2000", index, voice->loopend);
		state_save_register_item("bsmt2000", index, voice->loopstart);
		state_save_register_item("bsmt2000", index, voice->bank);
		state_save_register_item("bsmt2000", index, voice->leftvol);
		state_save_register_item("bsmt2000", index, voice->rightvol);
		state_save_register_item("bsmt2000", index, voice->fraction);
	}

	/* reset the chip -- this also configures the default mode */
	bsmt2000_reset(chip);
	return chip;
}


/*-------------------------------------------------
    bsmt2000_reset - chip reset callback
-------------------------------------------------*/

static void bsmt2000_reset(void *_chip)
{
	bsmt2000_chip *chip = _chip;
	int voicenum;

	/* reset all the voice data */
	for (voicenum = 0; voicenum < MAX_VOICES; voicenum++)
	{
		bsmt2000_voice *voice = &chip->voice[voicenum];
		memset(voice, 0, sizeof(*voice));
		voice->leftvol = 0x7fff;
		voice->rightvol = 0x7fff;
	}

	/* recompute the mode */
	set_mode(chip);
}


/*-------------------------------------------------
    bsmt2000_update - update callback for
    sample generation
-------------------------------------------------*/

static void bsmt2000_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	stream_sample_t *left = buffer[0];
	stream_sample_t *right = buffer[1];
	bsmt2000_chip *chip = param;
	bsmt2000_voice *voice;
	int samp, voicenum;

	/* clear out the accumulator */
	memset(left, 0, length * sizeof(left[0]));
	memset(right, 0, length * sizeof(right[0]));

	/* loop over voices */
	for (voicenum = 0; voicenum < chip->voices; voicenum++)
	{
		voice = &chip->voice[voicenum];

		/* compute the region base */
		if (voice->bank < chip->total_banks)
		{
			INT8 *base = &chip->region_base[voice->bank * 0x10000];
			UINT32 rate = voice->rate;
			INT32 rvol = voice->rightvol;
			INT32 lvol = chip->stereo ? voice->leftvol : rvol;
			UINT16 pos = voice->pos;
			UINT16 frac = voice->fraction;

			/* loop while we still have samples to generate */
			for (samp = 0; samp < length; samp++)
			{
#if ENABLE_INTERPOLATION
				INT32 sample = (base[pos] * (0x800 - frac) + (base[pos + 1] * frac)) >> 11;
#else
				INT32 sample = base[pos];
#endif
				/* apply volumes and add */
				left[samp] += sample * lvol;
				right[samp] += sample * rvol;

				/* update position */
				frac += rate;
				pos += frac >> 11;
				frac &= 0x7ff;

				/* check for loop end */
				if (pos >= voice->loopend)
					pos += voice->loopstart - voice->loopend;
			}

			/* update the position */
			voice->pos = pos;
			voice->fraction = frac;
		}
	}

	/* compressed voice (11-voice model only) */
	voice = &chip->voice[ADPCM_VOICE];
	if (chip->adpcm && voice->bank < chip->total_banks && voice->rate)
	{
		INT8 *base = &chip->region_base[voice->bank * 0x10000];
		INT32 rvol = voice->rightvol;
		INT32 lvol = chip->stereo ? voice->leftvol : rvol;
		UINT32 pos = voice->pos;
		UINT32 frac = voice->fraction;

		/* loop while we still have samples to generate */
		for (samp = 0; samp < length && pos < voice->loopend; samp++)
		{
			/* apply volumes and add */
			left[samp] += (chip->adpcm_current * lvol) >> 8;
			right[samp] += (chip->adpcm_current * rvol) >> 8;

			/* update position */
			frac++;
			if (frac == 6)
			{
				pos++;
				frac = 0;
			}

			/* every 3 samples, we update the ADPCM state */
			if (frac == 1 || frac == 4)
			{
				static const UINT8 delta_tab[] = { 58,58,58,58,77,102,128,154 };
				int nibble = base[pos] >> ((frac == 1) ? 4 : 0);
				int value = (INT8)(nibble << 4) >> 4;
				int delta;

				/* compute the delta for this sample */
				delta = chip->adpcm_delta_n * value;
				if (value > 0)
					delta += chip->adpcm_delta_n >> 1;
				else
					delta -= chip->adpcm_delta_n >> 1;

				/* add and clamp against the sample */
				chip->adpcm_current += delta;
				if (chip->adpcm_current >= 32767)
					chip->adpcm_current = 32767;
				else if (chip->adpcm_current <= -32768)
					chip->adpcm_current = -32768;

				/* adjust the delta multiplier */
				chip->adpcm_delta_n = (chip->adpcm_delta_n * delta_tab[abs(value)]) >> 6;
				if (chip->adpcm_delta_n > 2000)
					chip->adpcm_delta_n = 2000;
				else if (chip->adpcm_delta_n < 1)
					chip->adpcm_delta_n = 1;
			}
		}

		/* update the position */
		voice->pos = pos;
		voice->fraction = frac;

		/* "rate" is a control register; clear it to 0 when done */
		if (pos >= voice->loopend)
			voice->rate = 0;
	}

	/* reduce the overall gain */
	for (samp = 0; samp < length; samp++)
	{
		left[samp] >>= 9;
		right[samp] >>= 9;
	}
}



/***************************************************************************
    READ/WRITE ACCESS
***************************************************************************/

/*-------------------------------------------------
    bsmt2000_reg_write - handle a register write
-------------------------------------------------*/

static void bsmt2000_reg_write(bsmt2000_chip *chip, offs_t offset, UINT16 data)
{
#if LOG_COMMANDS
	mame_printf_debug("BSMT write: reg %02X = %04X\n", offset, data);
#endif

	/* remember the last write */
	chip->last_register = offset;

	/* update the register */
	if (offset < 0x80 && chip->regmap[offset] != NULL)
	{
		UINT16 *dest = chip->regmap[offset];

		/* force an update, then write the data */
		stream_update(chip->stream);
		*dest = data;

		/* special case: reset ADPCM parameters when writing to the ADPCM position */
		if (dest == &chip->voice[ADPCM_VOICE].rate)
		{
			chip->adpcm_current = 0;
			chip->adpcm_delta_n = 10;
		}
	}
}



/***************************************************************************
    PER-CHIP READ/WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    BSMT2000_data_0_w - write to chip 0
-------------------------------------------------*/

WRITE16_HANDLER( BSMT2000_data_0_w )
{
	bsmt2000_reg_write(sndti_token(SOUND_BSMT2000, 0), offset, data);
}



/***************************************************************************
    LOCAL FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    set_mode - set the mode after reset
-------------------------------------------------*/

static void set_mode(bsmt2000_chip *chip)
{
	int sample_rate;

	/* force an update */
	stream_update(chip->stream);

	/* the mode comes from the address of the last register accessed */
	switch (chip->last_register)
	{
		/* mode 0: 24kHz, 12 channel PCM, 1 channel ADPCM, mono */
		default:
		case 0:
			sample_rate = chip->clock / 1000;
			chip->stereo = FALSE;
			chip->voices = 12;
			chip->adpcm = TRUE;
			set_regmap(chip, 0x00, 0x18, 0x24, 0x30, 0x3c, 0x48, 0);
			break;

		/* mode 1: 24kHz, 11 channel PCM, 1 channel ADPCM, stereo */
		case 1:
			sample_rate = chip->clock / 1000;
			chip->stereo = TRUE;
			chip->voices = 11;
			chip->adpcm = TRUE;
			set_regmap(chip, 0x00, 0x16, 0x21, 0x2c, 0x37, 0x42, 0x4d);
			break;

		/* mode 5: 24kHz, 12 channel PCM, stereo */
		case 5:
			sample_rate = chip->clock / 1000;
			chip->stereo = TRUE;
			chip->voices = 12;
			chip->adpcm = FALSE;
			set_regmap(chip, 0x00, 0x18, 0x24, 0x30, 0x3c, 0x54, 0x60);
			break;

		/* mode 6: 34kHz, 8 channel PCM, stereo */
		case 6:
			sample_rate = chip->clock / 706;
			chip->stereo = TRUE;
			chip->voices = 8;
			chip->adpcm = FALSE;
			set_regmap(chip, 0x00, 0x10, 0x18, 0x20, 0x28, 0x38, 0x40);
			break;

		/* mode 7: 32kHz, 9 channel PCM, stereo */
		case 7:
			sample_rate = chip->clock / 750;
			chip->stereo = TRUE;
			chip->voices = 9;
			chip->adpcm = FALSE;
			set_regmap(chip, 0x00, 0x12, 0x1b, 0x24, 0x2d, 0x3f, 0x48);
			break;
	}

	/* update the sample rate */
	stream_set_sample_rate(chip->stream, sample_rate);
}


/*-------------------------------------------------
    set_regmap - initialize the register mapping
-------------------------------------------------*/

static void set_regmap(bsmt2000_chip *chip, UINT8 posbase, UINT8 ratebase, UINT8 endbase, UINT8 loopbase, UINT8 bankbase, UINT8 rvolbase, UINT8 lvolbase)
{
	int voice;

	/* reset the map */
	memset(chip->regmap, 0, sizeof(chip->regmap));

	/* iterate over voices */
	for (voice = 0; voice < chip->voices; voice++)
	{
		chip->regmap[posbase + voice] = &chip->voice[voice].pos;
		chip->regmap[ratebase + voice] = &chip->voice[voice].rate;
		chip->regmap[endbase + voice] = &chip->voice[voice].loopend;
		chip->regmap[loopbase + voice] = &chip->voice[voice].loopstart;
		chip->regmap[bankbase + voice] = &chip->voice[voice].bank;
		chip->regmap[rvolbase + voice] = &chip->voice[voice].rightvol;
		if (chip->stereo)
			chip->regmap[lvolbase + voice] = &chip->voice[voice].leftvol;
	}

	/* set the ADPCM register */
	if (chip->adpcm)
	{
		chip->regmap[0x6d] = &chip->voice[ADPCM_VOICE].loopend;
		chip->regmap[0x6f] = &chip->voice[ADPCM_VOICE].bank;
		chip->regmap[0x73] = &chip->voice[ADPCM_VOICE].rate;
		chip->regmap[0x74] = &chip->voice[ADPCM_VOICE].rightvol;
		chip->regmap[0x75] = &chip->voice[ADPCM_VOICE].pos;
		if (chip->stereo)
			chip->regmap[0x76] = &chip->voice[ADPCM_VOICE].leftvol;
	}
}



/***************************************************************************
    GET/SET INFO CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    bsmt2000_set_info - callback for setting chip
    information
-------------------------------------------------*/

static void bsmt2000_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


/*-------------------------------------------------
    bsmt2000_get_info - callback for retrieving
    chip information
-------------------------------------------------*/

void bsmt2000_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = bsmt2000_set_info;		break;
		case SNDINFO_PTR_START:							info->start = bsmt2000_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = bsmt2000_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "BSMT2000";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Data East Wavetable";		break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}
