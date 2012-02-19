/*************************************************************************

    Basic Gridlee sound driver

*************************************************************************/

#include "emu.h"
#include "includes/gridlee.h"
#include "sound/samples.h"


/*************************************
 *
 *  Structures
 *
 *************************************/

struct gridlee_sound_state
{
	/* tone variables */
	UINT32 m_tone_step;
	UINT32 m_tone_fraction;
	UINT8 m_tone_volume;

	/* sound streaming variables */
	sound_stream *m_stream;
	samples_device *m_samples;
	double m_freq_to_step;
	UINT8 m_sound_data[24];
};


/*************************************
 *
 *  Core sound generation
 *
 *************************************/

INLINE gridlee_sound_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == GRIDLEE);

	return (gridlee_sound_state *)downcast<legacy_device_base *>(device)->token();
}

static STREAM_UPDATE( gridlee_stream_update )
{
	gridlee_sound_state *state = get_safe_token(device);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
	{
		/* tone channel */
		state->m_tone_fraction += state->m_tone_step;
		*buffer++ = (state->m_tone_fraction & 0x0800000) ? (state->m_tone_volume << 6) : 0;
	}
}



/*************************************
 *
 *  Sound startup routines
 *
 *************************************/

static DEVICE_START( gridlee_sound )
{
	gridlee_sound_state *state = get_safe_token(device);
	running_machine &machine = device->machine();

	/* allocate the stream */
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, machine.sample_rate(), NULL, gridlee_stream_update);

	state->m_samples = device->machine().device<samples_device>("samples");

	state->m_freq_to_step = (double)(1 << 24) / (double)machine.sample_rate();
}


DEVICE_GET_INFO( gridlee_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(gridlee_sound_state);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(gridlee_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Gridlee Custom");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}



WRITE8_DEVICE_HANDLER( gridlee_sound_w )
{
	gridlee_sound_state *state = get_safe_token(device);
	UINT8 *sound_data = state->m_sound_data;
	samples_device *samples = state->m_samples;

	state->m_stream->update();

	switch (offset)
	{
		case 0x04:
			if (data == 0xef && sound_data[offset] != 0xef)
				samples->start(4, 1);
			else if (data != 0xef && sound_data[offset] == 0xef)
				samples->stop(4);
//          if (!(data & 0x01) && (sound_data[offset] & 0x01))
//              samples->start(5, 1);
//          else if ((data & 0x01) && !(sound_data[offset] & 0x01))
//              samples->stop(5);
			break;

		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
			if ((data & 1) && !(sound_data[offset] & 1))
				samples->start(offset - 0x0c, 1 - sound_data[offset - 4]);
			else if (!(data & 1) && (sound_data[offset] & 1))
				samples->stop(offset - 0x0c);
			break;

		case 0x08+0x08:
			if (data)
				state->m_tone_step = state->m_freq_to_step * (double)(data * 5);
			else
				state->m_tone_step = 0;
			break;

		case 0x09+0x08:
			state->m_tone_volume = data;
			break;

		case 0x0b+0x08:
//          tone_volume = (data | sound_data[0x0c+0x08]) ? 0xff : 0x00;
			break;

		case 0x0c+0x08:
//          tone_volume = (data | sound_data[0x0b+0x08]) ? 0xff : 0x00;
			break;

		case 0x0d+0x08:
//          if (data)
//              tone_step = freq_to_step * (double)(data * 11);
//          else
//              tone_step = 0;
			break;
	}
	sound_data[offset] = data;



#if 0
{
static int first = 1;
FILE *f;
f = fopen("sound.log", first ? "w" : "a");
first = 0;
fprintf(f, "[%02x=%02x] %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\n",
	offset,data,
	sound_data[0],
	sound_data[1],
	sound_data[2],
	sound_data[3],
	sound_data[4],
	sound_data[5],
	sound_data[6],
	sound_data[7],
	sound_data[8],
	sound_data[9],
	sound_data[10],
	sound_data[11],
	sound_data[12],
	sound_data[13],
	sound_data[14],
	sound_data[15],
	sound_data[16],
	sound_data[17],
	sound_data[18],
	sound_data[19],
	sound_data[20],
	sound_data[21],
	sound_data[22],
	sound_data[23]);
fclose(f);
}
#endif
}


DEFINE_LEGACY_SOUND_DEVICE(GRIDLEE, gridlee_sound);
