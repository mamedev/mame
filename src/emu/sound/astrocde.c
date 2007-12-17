/***********************************************************

    Astrocade custom 'IO' chip sound chip driver
    Aaron Giles
    based on original work by Frank Palazzolo

************************************************************

    Register Map
    ============

    Register 0:
        D7..D0: Master oscillator frequency

    Register 1:
        D7..D0: Tone generator A frequency

    Register 2:
        D7..D0: Tone generator B frequency

    Register 3:
        D7..D0: Tone generator C frequency

    Register 4:
        D7..D6: Vibrato speed
        D5..D0: Vibrato depth

    Register 5:
            D5: Noise AM enable
            D4: Mux source (0=vibrato, 1=noise)
        D3..D0: Tone generator C volume

    Register 6:
        D7..D4: Tone generator B volume
        D3..D0: Tone generator A volume

    Register 7:
        D7..D0: Noise volume

***********************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "astrocde.h"


struct astrocade_info
{
	sound_stream *stream;		/* sound stream */

	UINT8		reg[8];			/* 8 control registers */

	UINT8		master_count;	/* current master oscillator count */
	UINT16		vibrato_clock;	/* current vibrato clock */

	UINT8		noise_clock;	/* current noise generator clock */
	UINT16		noise_state;	/* current noise LFSR state */

	UINT8		a_count;		/* current tone generator A count */
	UINT8		a_state;		/* current tone generator A state */

	UINT8		b_count;		/* current tone generator B count */
	UINT8		b_state;		/* current tone generator B state */

	UINT8		c_count;		/* current tone generator C count */
	UINT8		c_state;		/* current tone generator C state */

	UINT8		bitswap[256];	/* bitswap table */
};



/*************************************
 *
 *  Core sound update
 *
 *************************************/

static void astrocade_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int samples)
{
	struct astrocade_info *chip = param;
	stream_sample_t *dest = buffer[0];
	UINT16 noise_state;
	UINT8 master_count;
	UINT8 noise_clock;

	/* load some locals */
	master_count = chip->master_count;
	noise_clock = chip->noise_clock;
	noise_state = chip->noise_state;

	/* loop over samples */
	while (samples > 0)
	{
		stream_sample_t cursample = 0;
		int samples_this_time;
		int samp;

		/* compute the number of cycles until the next master oscillator reset */
		/* or until the next noise boundary */
		samples_this_time = MIN(samples, 256 - master_count);
		samples_this_time = MIN(samples_this_time, 64 - noise_clock);
		samples -= samples_this_time;

		/* sum the output of the tone generators */
		if (chip->a_state)
			cursample += chip->reg[6] & 0x0f;
		if (chip->b_state)
			cursample += chip->reg[6] >> 4;
		if (chip->c_state)
			cursample += chip->reg[5] & 0x0f;

		/* add in the noise if it is enabled, based on the top bit of the LFSR */
		if ((chip->reg[5] & 0x20) && (noise_state & 0x4000))
			cursample += chip->reg[7] >> 4;

		/* scale to max and output */
		cursample = cursample * 32767 / 60;
		for (samp = 0; samp < samples_this_time; samp++)
			*dest++ = cursample;

		/* clock the noise; a 2-bit counter clocks a 4-bit counter which clocks the LFSR */
		noise_clock += samples_this_time;
		if (noise_clock >= 64)
		{
			/* update the noise state; this is a 15-bit LFSR with feedback from */
			/* the XOR of the top two bits */
			noise_state = (noise_state << 1) | (~((noise_state >> 14) ^ (noise_state >> 13)) & 1);
			noise_clock -= 64;

			/* the same clock also controls the vibrato clock, which is a 13-bit counter */
			chip->vibrato_clock++;
		}

		/* clock the master oscillator; this is an 8-bit up counter */
		master_count += samples_this_time;
		if (master_count == 0)
		{
			/* reload based on mux value -- the value from the register is negative logic */
			master_count = ~chip->reg[0];

			/* mux value 0 means reload based on the vibrato control */
			if ((chip->reg[5] & 0x10) == 0)
			{
				/* vibrato speed (register 4 bits 6-7) selects one of the top 4 bits */
				/* of the 13-bit vibrato clock to use (0=highest freq, 3=lowest) */
				if (!((chip->vibrato_clock >> (chip->reg[4] >> 6)) & 0x0200))
				{
					/* if the bit is clear, we add the vibrato volume to the counter */
					master_count += chip->reg[4] & 0x3f;
				}
			}

			/* mux value 1 means reload based on the noise control */
			else
			{
				/* the top 8 bits of the noise LFSR are ANDed with the noise volume */
				/* register and added to the count */
				master_count += chip->bitswap[(noise_state >> 7) & 0xff] & chip->reg[7];
			}

			/* clock tone A */
			if (++chip->a_count == 0)
			{
				chip->a_state ^= 1;
				chip->a_count = ~chip->reg[1];
			}

			/* clock tone B */
			if (++chip->b_count == 0)
			{
				chip->b_state ^= 1;
				chip->b_count = ~chip->reg[2];
			}

			/* clock tone C */
			if (++chip->c_count == 0)
			{
				chip->c_state ^= 1;
				chip->c_count = ~chip->reg[3];
			}
		}
	}

	/* put back the locals */
	chip->master_count = master_count;
	chip->noise_clock = noise_clock;
	chip->noise_state = noise_state;
}



/*************************************
 *
 *  Chip reset
 *
 *************************************/

static void astrocade_reset(void *_chip)
{
	struct astrocade_info *chip = _chip;

	memset(chip->reg, 0, sizeof(chip->reg));

	chip->master_count = 0;
	chip->vibrato_clock = 0;

	chip->noise_clock = 0;
	chip->noise_state = 0;

	chip->a_count = 0;
	chip->a_state = 0;

	chip->b_count = 0;
	chip->b_state = 0;

	chip->c_count = 0;
	chip->c_state = 0;
}



/*************************************
 *
 *  Save state registration
 *
 *************************************/

static void astrocade_state_save_register(struct astrocade_info *chip, int sndindex)
{
	state_save_register_item_array("globals", sndindex, chip->reg);

	state_save_register_item_array("astrocade", sndindex, chip->reg);

	state_save_register_item("astrocade", sndindex, chip->master_count);
	state_save_register_item("astrocade", sndindex, chip->vibrato_clock);

	state_save_register_item("astrocade", sndindex, chip->noise_clock);
	state_save_register_item("astrocade", sndindex, chip->noise_state);

	state_save_register_item("astrocade", sndindex, chip->a_count);
	state_save_register_item("astrocade", sndindex, chip->a_state);

	state_save_register_item("astrocade", sndindex, chip->b_count);
	state_save_register_item("astrocade", sndindex, chip->b_state);

	state_save_register_item("astrocade", sndindex, chip->c_count);
	state_save_register_item("astrocade", sndindex, chip->c_state);
}



/*************************************
 *
 *  Chip initialization
 *
 *************************************/

static void *astrocade_start(int sndindex, int clock, const void *config)
{
	struct astrocade_info *chip;
	int i;

	/* allocate the chip memory */
	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	/* generate a bitswap table for the noise */
	for (i = 0; i < 256; i++)
		chip->bitswap[i] = BITSWAP8(i, 0,1,2,3,4,5,6,7);

	/* allocate a stream for output */
	chip->stream = stream_create(0, 1, clock, chip, astrocade_update);

	/* reset state */
	astrocade_reset(chip);
	astrocade_state_save_register(chip, sndindex);

	return chip;
}



/*************************************
 *
 *  Sound write accessors
 *
 *************************************/

void astrocade_sound_w(UINT8 num, offs_t offset, UINT8 data)
{
	struct astrocade_info *chip = sndti_token(SOUND_ASTROCADE, num);

	/* update */
	stream_update(chip->stream);

	/* stash the new register value */
	chip->reg[offset & 7] = data;
}


WRITE8_HANDLER( astrocade_sound1_w )
{
	if ((offset & 8) != 0)
		astrocade_sound_w(0, (offset >> 8) & 7, data);
	else
		astrocade_sound_w(0, offset & 7, data);
}


WRITE8_HANDLER( astrocade_sound2_w )
{
	if ((offset & 8) != 0)
		astrocade_sound_w(1, (offset >> 8) & 7, data);
	else
		astrocade_sound_w(1, offset & 7, data);
}



/*************************************
 *
 *  Get/set info callbacks
 *
 *************************************/

static void astrocade_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void astrocade_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = astrocade_set_info;	break;
		case SNDINFO_PTR_START:							info->start = astrocade_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							info->reset = astrocade_reset;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Astrocade";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Bally";						break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "2.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004-2007, The MAME Team"; break;
	}
}
