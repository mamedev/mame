/****************************************************************

    MAME / MESS functions

****************************************************************/

#include "emu.h"
#include "streams.h"
#include "ym2413.h"
#include "2413intf.h"


/* for stream system */
typedef struct _ym2413_state ym2413_state;
struct _ym2413_state
{
	sound_stream *	stream;
	void *			chip;
};


INLINE ym2413_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_YM2413);
	return (ym2413_state *)device->token;
}


#ifdef UNUSED_FUNCTION
void YM2413DAC_update(int chip,stream_sample_t **inputs, stream_sample_t **_buffer,int length)
{
    INT16 *buffer = _buffer[0];
    static int out = 0;

    if ( ym2413[chip].reg[0x0F] & 0x01 )
    {
        out = ((ym2413[chip].reg[0x10] & 0xF0) << 7);
    }
    while (length--) *(buffer++) = out;
}
#endif

static STREAM_UPDATE( ym2413_stream_update )
{
	ym2413_state *info = (ym2413_state *)param;
	ym2413_update_one(info->chip, outputs, samples);
}

static void _stream_update(void *param, int interval)
{
	ym2413_state *info = (ym2413_state *)param;
	stream_update(info->stream);
}

static DEVICE_START( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	int rate = device->clock/72;

	/* emulator create */
	info->chip = ym2413_init(device, device->clock, rate);
	assert_always(info->chip != NULL, "Error creating YM2413 chip");

	/* stream system initialize */
	info->stream = stream_create(device,0,2,rate,info,ym2413_stream_update);

	ym2413_set_update_handler(info->chip, _stream_update, info);




#if 0
	int i, tst;
	char name[40];

	num = intf->num;

	tst = YM3812_sh_start (msound);
	if (tst)
		return 1;

	for (i=0;i<num;i++)
	{
		ym2413_reset (i);

		ym2413[i].DAC_stream = stream_create(device, 0, 1, device->clock/72, i, YM2413DAC_update);

		if (ym2413[i].DAC_stream == -1)
			return 1;
	}
	return 0;
#endif

}

static DEVICE_STOP( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_shutdown(info->chip);
}

static DEVICE_RESET( ym2413 )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_reset_chip(info->chip);
}


WRITE8_DEVICE_HANDLER( ym2413_w )
{
	ym2413_state *info = get_safe_token(device);
	ym2413_write(info->chip, offset & 1, data);
}

WRITE8_DEVICE_HANDLER( ym2413_register_port_w ) { ym2413_w(device, 0, data); }
WRITE8_DEVICE_HANDLER( ym2413_data_port_w ) { ym2413_w(device, 1, data); }


/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( ym2413 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(ym2413_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( ym2413 );				break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME( ym2413 );				break;
		case DEVINFO_FCT_RESET:							info->reset = DEVICE_RESET_NAME( ym2413 );				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "YM2413");							break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Yamaha FM");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
