/***************************************************************************

    wave.c

    Code that interfaces
    Functions to handle loading, creation, recording and playback
    of wave samples for IO_CASSETTE

****************************************************************************/

#include "driver.h"
#include "streams.h"
#include "deprecat.h"
#ifdef MESS
#include "messdrv.h"
#include "utils.h"
#include "devices/cassette.h"
#endif
#include "wave.h"

#define ALWAYS_PLAY_SOUND	0
#define WAVE_TOKEN_MASK		0xFFFF0000



static void wave_sound_update(void *param,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
#ifdef MESS
	mess_image *image;
	cassette_image *cassette;
	cassette_state state;
	double time_index;
	double duration;
	int num = ((FPTR)param) & ~WAVE_TOKEN_MASK;
	stream_sample_t *buffer = _buffer[0];
	int i;

	image = image_from_devtype_and_index(IO_CASSETTE, num);
	state = cassette_get_state(image);

	state &= CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER;
	if (image_exists(image) && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette = cassette_get_image(image);
		time_index = cassette_get_position(image);
		duration = ((double) length) / Machine->sample_rate;

		cassette_get_samples(cassette, 0, time_index, duration, length, 2, buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = length-1; i >= 0; i--)
			buffer[i] = ((INT16 *) buffer)[i];
	}
	else
	{
		memset(buffer, 0, sizeof(*buffer) * length);
	}
#endif
}



static void *wave_start(int sndindex, int clock, const void *config)
{
	stream_create(0, 1, Machine->sample_rate, (void *) (FPTR)sndindex, wave_sound_update);
	return (void *) (FPTR)(sndindex | WAVE_TOKEN_MASK);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static void wave_set_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* no parameters to set */
	}
}


void wave_get_info(void *token, UINT32 state, sndinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = wave_set_info;			break;
		case SNDINFO_PTR_START:							info->start = wave_start;				break;
		case SNDINFO_PTR_STOP:							/* nothing */							break;
		case SNDINFO_PTR_RESET:							/* nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case SNDINFO_STR_NAME:							info->s = "Cassette";					break;
		case SNDINFO_STR_CORE_FAMILY:					info->s = "Cassette";					break;
		case SNDINFO_STR_CORE_VERSION:					info->s = "1.0";						break;
		case SNDINFO_STR_CORE_FILE:						info->s = __FILE__;						break;
		case SNDINFO_STR_CORE_CREDITS:					info->s = "Copyright The MESS Team"; break;
	}
}
