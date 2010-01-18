/*
 * Xicor X2212
 *
 * 256 x 4 bit Nonvolatile Static RAM
 *
 */

#include "emu.h"
#include "machine/x2212.h"

#define SIZE_DATA ( 0x100 )

typedef struct
{
	UINT8 *sram;
	UINT8 *e2prom;
	UINT8 *default_data;
	int store;
	int array_recall;
} x2212_state;

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an X2212
-------------------------------------------------*/

INLINE x2212_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == X2212);

	return (x2212_state *)device->token;
}

void x2212_write( running_device *device, int offset, int data )
{
	x2212_state *c = get_safe_token(device);

	c->sram[ offset ] = data;
}


int x2212_read( running_device *device, int offset )
{
	x2212_state *c = get_safe_token(device);

	return c->sram[ offset ];
}

void x2212_store( running_device *device, int store )
{
	x2212_state *c = get_safe_token(device);

	if( !store && c->store )
	{
		memcpy( c->e2prom, c->sram, SIZE_DATA );
	}

	c->store = store;
}

void x2212_array_recall( running_device *device, int array_recall )
{
	x2212_state *c = get_safe_token(device);

	if( !array_recall && c->array_recall )
	{
		memcpy( c->sram, c->e2prom, SIZE_DATA );
	}

	c->array_recall = array_recall;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START(x2212)
{
	x2212_state *c = get_safe_token(device);
	const x2212_config *config;

	/* validate some basic stuff */
	assert(device != NULL);
//  assert(device->baseconfig().static_config != NULL);
	assert(device->baseconfig().inline_config == NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	c->sram = auto_alloc_array( device->machine, UINT8, SIZE_DATA );
	c->e2prom = auto_alloc_array( device->machine, UINT8, SIZE_DATA );
	c->store = 1;
	c->array_recall = 1;

	config = (const x2212_config *)device->baseconfig().static_config;
	if( config != NULL && config->data != NULL )
	{
		c->default_data = memory_region( device->machine, config->data );
	}

	state_save_register_device_item_pointer( device, 0, c->sram, SIZE_DATA );
	state_save_register_device_item_pointer( device, 0, c->e2prom, SIZE_DATA );
	state_save_register_device_item( device, 0, c->store );
	state_save_register_device_item( device, 0, c->array_recall );
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET(x2212)
{
}

static DEVICE_NVRAM(x2212)
{
	x2212_state *c = get_safe_token(device);

	if( read_or_write )
	{
		mame_fwrite( file, c->sram, SIZE_DATA );
	}
	else
	{
		if( file )
		{
			mame_fread( file, c->e2prom, SIZE_DATA );
		}
		else
		{
			if( c->default_data != NULL )
			{
				memcpy( c->e2prom, c->default_data, SIZE_DATA );
			}
			else
			{
				memset( c->e2prom, 0xff, SIZE_DATA );
			}
		}

		memcpy( c->sram, c->e2prom, SIZE_DATA );
	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO(x2212)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(x2212_state); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = 0; break; // sizeof(x2212_config)
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(x2212); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(x2212); break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(x2212); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "X2212"); break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "EEPROM"); break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0"); break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
