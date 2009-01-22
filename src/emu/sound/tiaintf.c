#include "sndintrf.h"
#include "streams.h"
#include "tiaintf.h"
#include "tiasound.h"

struct tia_info
{
	sound_stream * channel;
	void *chip;
};


static STREAM_UPDATE( tia_update )
{
	struct tia_info *info = param;
	tia_process(info->chip, outputs[0], samples);
}


static SND_START( tia )
{
	struct tia_info *info = device->token;

	info->channel = stream_create(device, 0, 1, clock, info, tia_update);

	info->chip = tia_sound_init(clock, clock, 16);
	assert_always(info->chip != NULL, "Error creating TIA chip");
}

static SND_STOP( tia )
{
	struct tia_info *info = device->token;
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

static SND_SET_INFO( tia )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( tia )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case SNDINFO_INT_TOKEN_BYTES:					info->i = sizeof(struct tia_info); 			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( tia );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( tia );		break;
		case SNDINFO_PTR_STOP:							info->stop = SND_STOP_NAME( tia );			break;
		case SNDINFO_PTR_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "TIA");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Atari custom");			break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");						break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);					break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

