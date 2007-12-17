/***************************************************************************

An Hitachi HD637A01X0 MCU programmed to act as a sample player.
Used by some Namco System 86 games.

The MCU has internal ROM which hasn't been dumped, so here we simulate its
simple functions.

The chip can address ROM space up to 8 block of 0x10000 bytes. At the beginning
of each block there's a table listing the start offset of each sample.
Samples are 8 bit unsigned, 0xff marks the end of the sample. 0x00 is used for
silence compression: '00 nn' must be replaced by nn+1 times '80'.

***************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "n63701x.h"


typedef struct
{
	int select;
	int playing;
	int base_addr;
	int position;
	int volume;
	int silence_counter;
} voice;

struct namco_63701x
{
	voice voices[2];
	sound_stream * stream;		/* channel assigned by the mixer */
	const struct namco_63701x_interface *intf;	/* pointer to our config data */
	UINT8 *rom;		/* pointer to sample ROM */
};


/* volume control has three resistors: 22000, 10000 and 3300 Ohm.
   22000 is always enabled, the other two can be turned off.
   Since 0x00 and 0xff samples have special meaning, the available range is
   0x01 to 0xfe, therefore 258 * (0x01 - 0x80) = 0x8002 just keeps us
   inside 16 bits without overflowing.
 */
static const int vol_table[4] = { 26, 84, 200, 258 };


static void namco_63701x_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct namco_63701x *chip = param;
	int ch;

	for (ch = 0;ch < 2;ch++)
	{
		stream_sample_t *buf = buffer[ch];
		voice *v = &chip->voices[ch];

		if (v->playing)
		{
			UINT8 *base = chip->rom + v->base_addr;
			int pos = v->position;
			int vol = vol_table[v->volume];
			int p;

			for (p = 0;p < length;p++)
			{
				if (v->silence_counter)
				{
					v->silence_counter--;
					*(buf++) = 0;
				}
				else
				{
					int data = base[(pos++) & 0xffff];

					if (data == 0xff)	/* end of sample */
					{
						v->playing = 0;
						break;
					}
					else if (data == 0x00)	/* silence compression */
					{
						data = base[(pos++) & 0xffff];
						v->silence_counter = data;
						*(buf++) = 0;
					}
					else
					{
						*(buf++) = vol * (data - 0x80);
					}
				}
			}

			v->position = pos;
		}
		else
			memset(buf, 0, length * sizeof(*buf));
	}
}


static void *namco_63701x_start(int sndindex, int clock, const void *config)
{
	struct namco_63701x *chip;

	chip = auto_malloc(sizeof(*chip));
	memset(chip, 0, sizeof(*chip));

	chip->intf = config;
	chip->rom = memory_region(chip->intf->region);

	chip->stream = stream_create(0, 2, clock/1000, chip, namco_63701x_update);

	return chip;
}



void namco_63701x_write(int offset, int data)
{
	struct namco_63701x *chip = sndti_token(SOUND_NAMCO_63701X, 0);
	int ch = offset / 2;

	if (offset & 1)
		chip->voices[ch].select = data;
	else
	{
		/*
          should we stop the playing sample if voice_select[ch] == 0 ?
          originally we were, but this makes us lose a sample in genpeitd,
          after the continue counter reaches 0. Either we shouldn't stop
          the sample, or genpeitd is returning to the title screen too soon.
         */
		if (chip->voices[ch].select & 0x1f)
		{
			int rom_offs;

			/* update the streams */
			stream_update(chip->stream);

			chip->voices[ch].playing = 1;
			chip->voices[ch].base_addr = 0x10000 * ((chip->voices[ch].select & 0xe0) >> 5);
			rom_offs = chip->voices[ch].base_addr + 2 * ((chip->voices[ch].select & 0x1f) - 1);
			chip->voices[ch].position = (chip->rom[rom_offs] << 8) + chip->rom[rom_offs+1];
			/* bits 6-7 = volume */
			chip->voices[ch].volume = data >> 6;
			/* bits 0-5 = counter to indicate new sample start? we don't use them */

			chip->voices[ch].silence_counter = 0;
		}
	}
}






/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void namco_63701x_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void namco_63701x_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = namco_63701x_set_info;	break;
		case SNDINFO_PTR_START:							info->start = namco_63701x_start;		break;
		case SNDINFO_PTR_STOP:							/* Nothing */							break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Namco 63701X";				break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Namco custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

