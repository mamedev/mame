/***************************************************************************

    wave.c

    Code that interfaces
    Functions to handle loading, creation, recording and playback
    of wave samples for IO_CASSETTE

****************************************************************************/

#include "driver.h"
#include "streams.h"
#ifdef MESS
#include "messdrv.h"
#include "utils.h"
#include "devices/cassette.h"
#endif
#include "wave.h"

#define ALWAYS_PLAY_SOUND	0
#define WAVE_TOKEN_MASK		0xFFFF0000



static STREAM_UPDATE( wave_sound_update )
{
#ifdef MESS
	const device_config *image = param;
	cassette_image *cassette;
	cassette_state state;
	double time_index;
	double duration;
	stream_sample_t *buffer = outputs[0];
	int i;

	state = cassette_get_state(image);

	state &= CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER;
	if (image_exists(image) && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette = cassette_get_image(image);
		time_index = cassette_get_position(image);
		duration = ((double) samples) / image->machine->sample_rate;

		cassette_get_samples(cassette, 0, time_index, duration, samples, 2, buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = samples-1; i >= 0; i--)
			buffer[i] = ((INT16 *) buffer)[i];
	}
	else
	{
		memset(buffer, 0, sizeof(*buffer) * samples);
	}
#endif
}



static SND_START( wave )
{
	const device_config *image = NULL;

#ifdef MESS
	image = device_list_find_by_tag( device->machine->config->devicelist, CASSETTE, device->tag );
#endif
	stream_create(device, 0, 1, device->machine->sample_rate, (void *)image, wave_sound_update);
	return (void *) (FPTR)(sndindex | WAVE_TOKEN_MASK);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

static SND_SET_INFO( wave )
{
	switch (state)
	{
		/* no parameters to set */
	}
}


SND_GET_INFO( wave )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case SNDINFO_PTR_SET_INFO:						info->set_info = SND_SET_INFO_NAME( wave );			break;
		case SNDINFO_PTR_START:							info->start = SND_START_NAME( wave );				break;
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
