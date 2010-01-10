#include "emu.h"
#include "streams.h"
#include "dac.h"


/* default to 4x oversampling */
#define DEFAULT_SAMPLE_RATE (48000 * 4)


typedef struct _dac_state dac_state;
struct _dac_state
{
	sound_stream	*channel;
	INT16			output;
	INT16			UnsignedVolTable[256];
	INT16			SignedVolTable[256];
};


INLINE dac_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == SOUND);
	assert(sound_get_type(device) == SOUND_DAC);
	return (dac_state *)device->token;
}


static STREAM_UPDATE( DAC_update )
{
	dac_state *info = (dac_state *)param;
	stream_sample_t *buffer = outputs[0];
	INT16 out = info->output;

	while (samples--) *(buffer++) = out;
}


void dac_data_w(const device_config *device, UINT8 data)
{
	dac_state *info = get_safe_token(device);
	INT16 out = info->UnsignedVolTable[data];

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_signed_data_w(const device_config *device, UINT8 data)
{
	dac_state *info = get_safe_token(device);
	INT16 out = info->SignedVolTable[data];

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_data_16_w(const device_config *device, UINT16 data)
{
	dac_state *info = get_safe_token(device);
	INT16 out = data >> 1;		/* range      0..32767 */

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


void dac_signed_data_16_w(const device_config *device, UINT16 data)
{
	dac_state *info = get_safe_token(device);
	INT16 out = (INT32)data - (INT32)0x08000;	/* range -32768..32767 */
						/* casts avoid potential overflow on some ABIs */

	if (info->output != out)
	{
		/* update the output buffer before changing the registers */
		stream_update(info->channel);
		info->output = out;
	}
}


static void DAC_build_voltable(dac_state *info)
{
	int i;

	/* build volume table (linear) */
	for (i = 0;i < 256;i++)
	{
		info->UnsignedVolTable[i] = i * 0x101 / 2;	/* range      0..32767 */
		info->SignedVolTable[i] = i * 0x101 - 0x8000;	/* range -32768..32767 */
	}
}


static DEVICE_START( dac )
{
	dac_state *info = get_safe_token(device);

	DAC_build_voltable(info);

	info->channel = stream_create(device,0,1,device->clock ? device->clock : DEFAULT_SAMPLE_RATE,info,DAC_update);
	info->output = 0;

	state_save_register_device_item(device, 0, info->output);
}



WRITE8_DEVICE_HANDLER( dac_w )
{
	dac_data_w(device, data);
}

WRITE8_DEVICE_HANDLER( dac_signed_w )
{
	dac_signed_data_w(device, data);
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

DEVICE_GET_INFO( dac )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(dac_state);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME( dac );		break;
		case DEVINFO_FCT_STOP:							/* nothing */								break;
		case DEVINFO_FCT_RESET:							/* nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "DAC");						break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "DAC");						break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
