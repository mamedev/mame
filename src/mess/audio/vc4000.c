/***************************************************************************

  PeT mess@utanet.at
  main part in video/

***************************************************************************/

#include "includes/vc4000.h"


struct vc4000_sound
{
	sound_stream *channel;
	UINT8 reg[1];
	int size, pos;
	unsigned level;
};


static vc4000_sound *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == VC4000);
	return (vc4000_sound *) downcast<vc4000_sound_device *>(device)->token();
}


void vc4000_soundport_w (device_t *device, int offset, int data)
{
	vc4000_sound *token = get_token(device);

	token->channel->update();
	token->reg[offset] = data;
	switch (offset)
	{
		case 0:
			token->pos = 0;
			token->level = TRUE;
			// frequency 7874/(data+1)
			token->size = device->machine().sample_rate() * (data + 1) /7874;
			break;
	}
}



/************************************/
/* Sound handler update             */
/************************************/

static STREAM_UPDATE( vc4000_update )
{
	int i;
	vc4000_sound *token = get_token(device);
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = 0;
		if (token->reg[0] && token->pos <= token->size / 2)
		{
			*buffer = 0x7fff;
		}
		if (token->pos <= token->size)
			token->pos++;
		if (token->pos > token->size)
			token->pos = 0;
	}
}



/************************************/
/* Sound handler start              */
/************************************/

static DEVICE_START(vc4000_sound)
{
	vc4000_sound *token = get_token(device);
	memset(token, 0, sizeof(*token));
	token->channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, vc4000_update);
}

const device_type VC4000 = &device_creator<vc4000_sound_device>;

vc4000_sound_device::vc4000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VC4000, "VC 4000 Custom", tag, owner, clock),
		device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_clear(vc4000_sound);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vc4000_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vc4000_sound_device::device_start()
{
	DEVICE_START_NAME( vc4000_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void vc4000_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}
