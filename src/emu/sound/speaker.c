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



static void speaker_sound_update(void *param,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
	struct speaker *sp = (struct speaker *) param;
	stream_sample_t *buffer = _buffer[0];
	int volume = sp->levels[sp->level];

    while( length-- > 0 )
		*buffer++ = volume;
}



static void *speaker_start(int sndindex, int clock, const void *config)
{
	struct speaker *sp = auto_malloc(sizeof(*sp));

	sp->channel = stream_create(0, 1, Machine->sample_rate, sp, speaker_sound_update);
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

static void speaker_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void speaker_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = speaker_set_info;		break;
		case SNDINFO_PTR_START:							info->start = speaker_start;			break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Speaker";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Speaker";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright The MESS Team"; break;
	}
}
