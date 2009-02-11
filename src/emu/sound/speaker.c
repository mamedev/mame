/***************************************************************************

    speaker.c

    Sound driver to emulate a simple speaker,
    driven by one or more output bits

****************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "speaker.h"

static const INT16 default_levels[2] = {0,32767};

typedef struct _speaker_state speaker_state;
struct _speaker_state
{
	sound_stream *channel;
	const INT16 *levels;
	int num_levels;
	int level;
};


INLINE speaker_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_SPEAKER);
	return (speaker_state *)device->token;
}


static STREAM_UPDATE( speaker_sound_update )
{
	speaker_state *sp = (speaker_state *) param;
	stream_sample_t *buffer = outputs[0];
	int volume = sp->levels[sp->level];

    while( samples-- > 0 )
		*buffer++ = volume;
}



static DEVICE_START( speaker )
{
	speaker_state *sp = get_safe_token(device);

	sp->channel = stream_create(device, 0, 1, device->machine->sample_rate, sp, speaker_sound_update);
	sp->num_levels = 2;
	sp->levels = default_levels;
	sp->level = 0;
}



void speaker_level_w(const device_config *device, int new_level)
{
	speaker_state *sp = get_safe_token(device);

    if( new_level < 0 )
		new_level = 0;
	else
	if( new_level >= sp->num_levels )
		new_level = sp->num_levels - 1;

	if( new_level == sp->level )
		return;

    /* force streams.c to update sound until this point in time now */
	stream_update(sp->channel);

	sp->level = new_level;
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( speaker )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(speaker_state); 				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( speaker );		break;
		case DEVINFO_FCT_STOP:							/* nothing */									break;
		case DEVINFO_FCT_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Speaker");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Speaker");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright The MESS Team"); 	break;
	}
}
