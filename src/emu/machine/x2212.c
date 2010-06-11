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
	assert(device->type() == X2212);

	return (x2212_state *)downcast<legacy_device_base *>(device)->token();
}

WRITE8_DEVICE_HANDLER( x2212_write )
{
	x2212_state *c = get_safe_token(device);

	c->sram[ offset ] = data;
}


READ8_DEVICE_HANDLER( x2212_read )
{
	x2212_state *c = get_safe_token(device);

	return c->sram[ offset ];
}

WRITE_LINE_DEVICE_HANDLER( x2212_store )
{
	x2212_state *c = get_safe_token(device);

	state &= 1;
	if( !state && c->store )
	{
		memcpy( c->e2prom, c->sram, SIZE_DATA );
	}

	c->store = state;
}

WRITE_LINE_DEVICE_HANDLER( x2212_array_recall )
{
	x2212_state *c = get_safe_token(device);

	state &= 1;
	if( !state && c->array_recall )
	{
		memcpy( c->sram, c->e2prom, SIZE_DATA );
	}

	c->array_recall = state;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START(x2212)
{
	x2212_state *c = get_safe_token(device);

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->baseconfig().static_config() == NULL);
	assert(downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config() == NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	c->sram = auto_alloc_array( device->machine, UINT8, SIZE_DATA );
	c->e2prom = auto_alloc_array( device->machine, UINT8, SIZE_DATA );
	c->store = 1;
	c->array_recall = 1;

	c->default_data = *device->region();

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
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##x2212##s
#define DEVTEMPLATE_FEATURES	DT_HAS_START | DT_HAS_RESET | DT_HAS_NVRAM
#define DEVTEMPLATE_NAME		"X2212"
#define DEVTEMPLATE_FAMILY		"EEPROM"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"


DEFINE_LEGACY_NVRAM_DEVICE(X2212, x2212);
