#include "emu.h"
#include "streams.h"
#include "tiaintf.h"
#include "tiasound.h"

typedef struct _tia_state tia_state;
struct _tia_state
{
	sound_stream * channel;
	void *chip;
};

INLINE tia_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_TIA);
	return (tia_state *)device->token;
}


static STREAM_UPDATE( tia_update )
{
	tia_state *info = (tia_state *)param;
	tia_process(info->chip, outputs[0], samples);
}


static DEVICE_START( tia )
{
	tia_state *info = get_safe_token(device);

	info->channel = stream_create(device, 0, 1, device->clock, info, tia_update);

	info->chip = tia_sound_init(device->clock, device->clock, 16);
	assert_always(info->chip != NULL, "Error creating TIA chip");
}

static DEVICE_STOP( tia )
{
	tia_state *info = get_safe_token(device);
	tia_sound_free(info->chip);
}

WRITE8_DEVICE_HANDLER( tia_sound_w )
{
	tia_state *info = get_safe_token(device);
	stream_update(info->channel);
	tia_write(info->chip, offset, data);
}




/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( tia )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(tia_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( tia );		break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( tia );			break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "TIA");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Atari custom");			break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

