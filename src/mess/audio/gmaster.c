/***************************************************************************
 PeT mess@utanet.at march 2002
***************************************************************************/

#include <math.h>
#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "includes/gmaster.h"

typedef struct _gmaster_sound gmaster_sound;
struct _gmaster_sound
{
	/*bool*/int level;
	sound_stream *mixer_channel;
};


static gmaster_sound *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GMASTER);
	return (gmaster_sound *) downcast<legacy_device_base *>(device)->token();
}


int gmaster_io_callback(device_t *device, int ioline, int state)
{	/* comes across with cpu device - need to use sound device */
	gmaster_sound *token = get_token(device->machine().device("custom"));

	switch (ioline)
	{
		case UPD7810_TO:
			token->mixer_channel->update();
			token->level = state;
			break;
		default:
			logerror("io changed %d %.2x\n",ioline, state);
			break;
	}
	return 0;
}


/************************************/
/* Sound handler update             */
/************************************/
static STREAM_UPDATE( gmaster_update )
{
	int i;
	gmaster_sound *token = get_token(device);
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = token->level ? 0x4000 : 0;
	}
}

/************************************/
/* Sound handler start              */
/************************************/

static DEVICE_START( gmaster_sound )
{
	gmaster_sound *token = get_token(device);
	token->mixer_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, gmaster_update);
}


DEVICE_GET_INFO( gmaster_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(gmaster_sound);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(gmaster_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Game Master Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(GMASTER, gmaster_sound);
