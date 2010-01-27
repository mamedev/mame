/***************************************************************************

    wave.c

    Code that interfaces
    Functions to handle loading, creation, recording and playback
    of wave samples for IO_CASSETTE

****************************************************************************/

#include "emu.h"
#include "streams.h"
#ifdef MESS
#include "messdrv.h"
#include "utils.h"
#include "devices/cassette.h"
#endif
#include "wave.h"

#define ALWAYS_PLAY_SOUND	0

static STREAM_UPDATE( wave_sound_update )
{
#ifdef MESS
	running_device *image = (running_device *)param;
	cassette_image *cassette;
	cassette_state state;
	double time_index;
	double duration;
	stream_sample_t *left_buffer = outputs[0];
	stream_sample_t *right_buffer = outputs[1];
	int i;

	state = cassette_get_state(image);

	state = (cassette_state)(state & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER));

	if (image_exists(image) && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette = cassette_get_image(image);
		time_index = cassette_get_position(image);
		duration = ((double) samples) / image->machine->sample_rate;

		cassette_get_samples(cassette, 0, time_index, duration, samples, 2, left_buffer, CASSETTE_WAVEFORM_16BIT);
		cassette_get_samples(cassette, 1, time_index, duration, samples, 2, right_buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = samples - 1; i >= 0; i--)
		{
			left_buffer[i] = ((INT16 *) left_buffer)[i];
			right_buffer[i] = ((INT16 *) right_buffer)[i];
		}
	}
	else
	{
		memset(left_buffer, 0, sizeof(*left_buffer) * samples);
		memset(right_buffer, 0, sizeof(*right_buffer) * samples);
	}
#endif
}



static DEVICE_START( wave )
{
	running_device *image = NULL;

	assert( device != NULL );
	assert( device->baseconfig().static_config != NULL );

#ifdef MESS
	image = device->machine->device( (const char *)device->baseconfig().static_config );
#endif
	stream_create(device, 0, 2, device->machine->sample_rate, (void *)image, wave_sound_update);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( wave )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( wave );	break;
		case DEVINFO_FCT_STOP:							/* nothing */								break;
		case DEVINFO_FCT_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Cassette");				break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Cassette");				break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						strcpy(info->s, "Copyright The MESS Team"); break;
	}
}
