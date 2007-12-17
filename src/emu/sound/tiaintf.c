#include "sndintrf.h"
#include "streams.h"
#include "tiaintf.h"
#include "tiasound.h"

struct tia_info
{
	sound_stream * channel;
	void *chip;
};


static void tia_update(void *param, stream_sample_t **inputs, stream_sample_t **buffer, int length)
{
	struct tia_info *info = param;
	tia_process(info->chip, buffer[0], length);
}


static void *tia_start(int sndindex, int clock, const void *config)
{
	struct tia_info *info;

	info = auto_malloc(sizeof(*info));
	memset(info, 0, sizeof(*info));

	info->channel = stream_create(0, 1, clock, info, tia_update);

	info->chip = tia_sound_init(clock, clock, 16);
	if (!info->chip)
		return NULL;

    return info;
}

static void tia_stop(void *token)
{
	struct tia_info *info = (struct tia_info*)token;
	tia_sound_free(info->chip);
}

WRITE8_HANDLER( tia_sound_w )
{
	struct tia_info *info = sndti_token(SOUND_TIA, 0);
	stream_update(info->channel);
	tia_write(info->chip, offset, data);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void tia_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void tia_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = tia_set_info;			break;
		case SNDINFO_PTR_START:							info->start = tia_start;				break;
		case SNDINFO_PTR_STOP:							info->stop = tia_stop;					break;
		case SNDINFO_PTR_RESET:							/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "TIA";						break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Atari custom";				break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright (c) 2004, The MAME Team"; break;
	}
}

