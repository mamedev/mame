/***************************************************************************

  audio/special.c

  Functions to emulate sound hardware of Specialist MX
  ( based on code of DAI interface )

****************************************************************************/

#include "includes/special.h"

struct specimx_sound_state
{
	sound_stream *mixer_channel;
	int specimx_input[3];
};

static STREAM_UPDATE( specimx_sh_update );

INLINE specimx_sound_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SPECIMX);
	return (specimx_sound_state *)downcast<specimx_sound_device *>(device)->token();
}

static DEVICE_START(specimx_sound)
{
	specimx_sound_state *state = get_safe_token(device);
	state->specimx_input[0] = state->specimx_input[1] = state->specimx_input[2] = 0;
	state->mixer_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, specimx_sh_update);
}

static STREAM_UPDATE( specimx_sh_update )
{
	specimx_sound_state *state = get_safe_token(device);
	INT16 channel_0_signal;
	INT16 channel_1_signal;
	INT16 channel_2_signal;

	stream_sample_t *sample_left = outputs[0];

	channel_0_signal = state->specimx_input[0] ? 3000 : -3000;
	channel_1_signal = state->specimx_input[1] ? 3000 : -3000;
	channel_2_signal = state->specimx_input[2] ? 3000 : -3000;

	while (samples--)
	{
		*sample_left = 0;

		/* music channel 0 */
		*sample_left += channel_0_signal;

		/* music channel 1 */
		*sample_left += channel_1_signal;

		/* music channel 2 */
		*sample_left += channel_2_signal;

		sample_left++;
	}
}

void specimx_set_input(device_t *device, int index, int state)
{
	specimx_sound_state *sndstate = get_safe_token(device);
	if (sndstate->mixer_channel!=NULL) {
		sndstate->mixer_channel->update();
	}
	sndstate->specimx_input[index] = state;
}


const device_type SPECIMX = &device_creator<specimx_sound_device>;

specimx_sound_device::specimx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SPECIMX, "Specialist MX Custom", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(specimx_sound_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void specimx_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void specimx_sound_device::device_start()
{
	DEVICE_START_NAME( specimx_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void specimx_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
