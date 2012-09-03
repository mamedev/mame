/***************************************************************************

    wave.c

    Code that interfaces
    Functions to handle loading, creation, recording and playback
    of wave samples for IO_CASSETTE

    2010-06-19 - Found that since 0.132, the right channel is badly out of
    sync on a mono system, causing bad sound. Added code to disable
    the second channel on a mono system.


****************************************************************************/

#include "emu.h"
#include "imagedev/cassette.h"
#include "wave.h"

#define ALWAYS_PLAY_SOUND	0

static STREAM_UPDATE( wave_sound_update )
{
	cassette_image_device *cass = (cassette_image_device *)param;
	cassette_image *cassette;
	cassette_state state;
	double time_index;
	double duration;
	stream_sample_t *left_buffer = outputs[0];
	stream_sample_t *right_buffer = NULL;
	int i;

	speaker_device_iterator spkiter(cass->machine().root_device());
	int speakers = spkiter.count();
	if (speakers>1)
		right_buffer = outputs[1];

	state = cass->get_state();

	state = (cassette_state)(state & (CASSETTE_MASK_UISTATE | CASSETTE_MASK_MOTOR | CASSETTE_MASK_SPEAKER));

	if (cass->exists() && (ALWAYS_PLAY_SOUND || (state == (CASSETTE_PLAY | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED))))
	{
		cassette = cass->get_image();
		time_index = cass->get_position();
		duration = ((double) samples) / cass->machine().sample_rate();

		cassette_get_samples(cassette, 0, time_index, duration, samples, 2, left_buffer, CASSETTE_WAVEFORM_16BIT);
		if (speakers > 1)
			cassette_get_samples(cassette, 1, time_index, duration, samples, 2, right_buffer, CASSETTE_WAVEFORM_16BIT);

		for (i = samples - 1; i >= 0; i--)
		{
			left_buffer[i] = ((INT16 *) left_buffer)[i];
			if (speakers > 1)
				right_buffer[i] = ((INT16 *) right_buffer)[i];
		}
	}
	else
	{
		memset(left_buffer, 0, sizeof(*left_buffer) * samples);
		if (speakers > 1)
			memset(right_buffer, 0, sizeof(*right_buffer) * samples);
	}
}



static DEVICE_START( wave )
{
	cassette_image_device *image = NULL;

	assert( device != NULL );
	assert( device->static_config() != NULL );
	speaker_device_iterator spkiter(device->machine().root_device());
	int speakers = spkiter.count();
	image = dynamic_cast<cassette_image_device *>(device->machine().device( (const char *)device->static_config()));
	if (speakers > 1)
		device->machine().sound().stream_alloc(*device, 0, 2, device->machine().sample_rate(), (void *)image, wave_sound_update);
	else
		device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), (void *)image, wave_sound_update);
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


const device_type WAVE = &device_creator<wave_device>;

wave_device::wave_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, WAVE, "Cassette", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void wave_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wave_device::device_start()
{
	DEVICE_START_NAME( wave )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void wave_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


