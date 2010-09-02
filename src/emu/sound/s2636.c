/***************************************************************************

  PeT mess@utanet.at

***************************************************************************/

#include "emu.h"
#include "streams.h"
#include "sound/s2636.h"


typedef struct _s2636_sound s2636_sound;
struct _s2636_sound
{
    sound_stream *channel;
    UINT8 reg[1];
    int size, pos;
    unsigned level;
};


static s2636_sound *get_token(running_device *device)
{
	assert(device != NULL);
	assert(device->type() == S2636_SOUND);
	return (s2636_sound *) downcast<legacy_device_base *>(device)->token();
}


void s2636_soundport_w (running_device *device, int offset, int data)
{
	s2636_sound *token = get_token(device);

	stream_update(token->channel);
	token->reg[offset] = data;
	switch (offset)
	{
		case 0:
			token->pos = 0;
			token->level = TRUE;
			// frequency 7874/(data+1)
			token->size = device->machine->sample_rate * (data + 1) /7874;
			break;
	}
}



/************************************/
/* Sound handler update             */
/************************************/

static STREAM_UPDATE( s2636_update )
{
	int i;
	s2636_sound *token = get_token(device);
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

static DEVICE_START(s2636_sound)
{
	s2636_sound *token = get_token(device);
	memset(token, 0, sizeof(*token));
    token->channel = stream_create(device, 0, 1, device->machine->sample_rate, 0, s2636_update);
}


DEVICE_GET_INFO( s2636_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(s2636_sound);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(s2636_sound);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "S2636");				break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
	}
}

DEFINE_LEGACY_SOUND_DEVICE(S2636_SOUND, s2636_sound);
