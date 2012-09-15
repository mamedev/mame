/***************************************************************************
 PeT mess@utanet.at march 2002
***************************************************************************/

#include <math.h>
#include "emu.h"
#include "cpu/upd7810/upd7810.h"
#include "includes/gmaster.h"

struct gmaster_sound
{
	/*bool*/int level;
	sound_stream *mixer_channel;
};


static gmaster_sound *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == GMASTER);
	return (gmaster_sound *) downcast<gmaster_sound_device *>(device)->token();
}


int gmaster_io_callback(device_t *device, int ioline, int state)
{	/* comes across with cpu device - need to use sound device */
	gmaster_sound *token = get_token(device->machine().device("custom"));

	switch (ioline)
	{
		case UPD7810_TO:
			token->mixer_channel->update();
			token->level = state;
			break;
		default:
			logerror("io changed %d %.2x\n",ioline, state);
			break;
	}
	return 0;
}


/************************************/
/* Sound handler update             */
/************************************/
static STREAM_UPDATE( gmaster_update )
{
	int i;
	gmaster_sound *token = get_token(device);
	stream_sample_t *buffer = outputs[0];

	for (i = 0; i < samples; i++, buffer++)
	{
		*buffer = token->level ? 0x4000 : 0;
	}
}

/************************************/
/* Sound handler start              */
/************************************/

static DEVICE_START( gmaster_sound )
{
	gmaster_sound *token = get_token(device);
	token->mixer_channel = device->machine().sound().stream_alloc(*device, 0, 1, device->machine().sample_rate(), 0, gmaster_update);
}

const device_type GMASTER = &device_creator<gmaster_sound_device>;

gmaster_sound_device::gmaster_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GMASTER, "Game Master Custom", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(gmaster_sound));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gmaster_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gmaster_sound_device::device_start()
{
	DEVICE_START_NAME( gmaster_sound )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gmaster_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


