/***************************************************************************

    speaker.c

    Sound driver to emulate a simple speaker,
    driven by one or more output bits

****************************************************************************/

#include "sndintrf.h"
#include "streams.h"
#include "speaker.h"

static const INT16 default_levels[2] = {0,32767};

struct speaker
{
	sound_stream *channel;
	const INT16 *levels;
	int num_levels;
	int level;
};



static STREAM_UPDATE( speaker_sound_update )
{
	struct speaker *sp = (struct speaker *) param;
	stream_sample_t *buffer = outputs[0];
	int volume = sp->levels[sp->level];

    while( samples-- > 0 )
		*buffer++ = volume;
}



static SND_START( speaker )
{
	struct speaker *sp = auto_malloc(sizeof(*sp));

	sp->channel = stream_create(device, 0, 1, device->machine->sample_rate, sp, speaker_sound_update);
	sp->num_levels = 2;
	sp->levels = default_levels;
	sp->level = 0;
	return sp;
}



void speaker_level_w(int which, int new_level)
{
	struct speaker *sp = sndti_token(SOUND_SPEAKER, which);

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

static SND_SET_INFO( speaker )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( speaker )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( speaker );	break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( speaker );		break;
		case SNDINFO_PTR_STOP:							/* nothing */									break;
		case SNDINFO_PTR_RESET:							/* nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							strcpy(info->s, "Speaker");						break;
		case SNDINFO_STR_CORE_FAMILY:					strcpy(info->s, "Speaker");						break;
		case SNDINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");							break;
		case SNDINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);						break;
		case SNDINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright The MESS Team"); 	break;
	}
}
