/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 */

#include "emu.h"
#include "machine/at28c16.h"

#define SIZE_DATA ( 0x800 )
#define SIZE_ID ( 0x020 )
#define OFFSET_ID ( SIZE_DATA - SIZE_ID )

typedef struct
{
	UINT8 *data;
	UINT8 *id;
	UINT8 *default_data;
	UINT8 *default_id;
	int last_write;
	int a9_12v;
	int oe_12v;
	emu_timer *write_timer;
} at28c16_state;

static TIMER_CALLBACK( write_finished )
{
	at28c16_state *c = (at28c16_state *) ptr;
	c->last_write = -1;
}

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an AT28C16
-------------------------------------------------*/

INLINE at28c16_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == AT28C16);

	return (at28c16_state *)device->token;
}

WRITE8_DEVICE_HANDLER( at28c16_w )
{
	at28c16_state *c = get_safe_token(device);

	if( c->last_write >= 0 )
	{
//      logerror( "%s: at28c16_write( %d, %04x, %02x ) busy\n", cpuexec_describe_context(machine), chip, offset, data );
	}
	else if( c->oe_12v )
	{
//      logerror( "%s: at28c16_write( %d, %04x, %02x ) erase\n", cpuexec_describe_context(machine), chip, offset, data );
		if( c->last_write < 0 )
		{
			memset( c->data, 0xff, SIZE_DATA );
			memset( c->id, 0xff, SIZE_ID );
			c->last_write = 0xff;
			timer_adjust_oneshot( c->write_timer, ATTOTIME_IN_USEC( 200 ), 0 );
		}
	}
	else if( offset >= OFFSET_ID && c->a9_12v )
	{
//      logerror( "%s: at28c16_write( %d, %04x, %02x ) id\n", cpuexec_describe_context(machine), chip, offset, data );
		if( c->last_write < 0 && c->id[ offset - OFFSET_ID ] != data )
		{
			c->id[ offset - OFFSET_ID ] = data;
			c->last_write = data;
			timer_adjust_oneshot( c->write_timer, ATTOTIME_IN_USEC( 200 ), 0 );
		}
	}
	else
	{
//      logerror( "%s: at28c16_write( %d, %04x, %02x ) data\n", cpuexec_describe_context(machine), chip, offset, data );
		if( c->last_write < 0 && c->data[ offset ] != data )
		{
			c->data[ offset ] = data;
			c->last_write = data;
			timer_adjust_oneshot( c->write_timer, ATTOTIME_IN_USEC( 200 ), 0 );
		}
	}
}


READ8_DEVICE_HANDLER( at28c16_r )
{
	at28c16_state *c = get_safe_token(device);

	if( c->last_write >= 0 )
	{
//      logerror( "at28c16_read( %04x ) write status\n", offset );
		return c->last_write ^ 0x80;
	}
	else if( offset >= OFFSET_ID && c->a9_12v )
	{
//      logerror( "at28c16_read( %04x ) id\n", offset );
		return c->id[ offset - OFFSET_ID ];
	}
	else
	{
//      logerror( "%s: at28c16_read( %d, %04x ) %02x data\n", cpuexec_describe_context(machine), chip, offset, c->data[ offset ] );
		return c->data[ offset ];
	}
}

void at28c16_a9_12v( const device_config *device, int a9_12v )
{
	at28c16_state *c = get_safe_token(device);

	c->a9_12v = a9_12v;
}

void at28c16_oe_12v( const device_config *device, int oe_12v )
{
	at28c16_state *c = get_safe_token(device);

	c->oe_12v = oe_12v;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START(at28c16)
{
	at28c16_state *c = get_safe_token(device);
	const at28c16_config *config;

	/* validate some basic stuff */
	assert(device != NULL);
//  assert(device->static_config != NULL);
//  assert(device->inline_config == NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	c->data = auto_alloc_array( device->machine, UINT8, SIZE_DATA );
	c->id = auto_alloc_array( device->machine, UINT8, SIZE_ID );
	c->a9_12v = 0;
	c->oe_12v = 0;
	c->last_write = -1;
	c->write_timer = timer_alloc(device->machine,  write_finished, c );
	c->default_data = *device->region;

	config = (const at28c16_config *)device->inline_config;
	if (config->id != NULL)
		c->default_id = memory_region( device->machine, config->id );

	/* create the name for save states */
	state_save_register_device_item_pointer( device, 0, c->data, SIZE_DATA );
	state_save_register_device_item_pointer( device, 0, c->id, SIZE_ID );
	state_save_register_device_item( device, 0, c->a9_12v );
	state_save_register_device_item( device, 0, c->oe_12v );
	state_save_register_device_item( device, 0, c->last_write );
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET(at28c16)
{
}

static DEVICE_NVRAM(at28c16)
{
	at28c16_state *c = get_safe_token(device);

	if( read_or_write )
	{
		mame_fwrite( file, c->data, SIZE_DATA );
		mame_fwrite( file, c->id, SIZE_ID );
	}
	else
	{
		if( file )
		{
			mame_fread( file, c->data, SIZE_DATA );
			mame_fread( file, c->id, SIZE_ID );
		}
		else
		{
			if( c->default_data != NULL )
			{
				memcpy( c->data, c->default_data, SIZE_DATA );
			}
			else
			{
				memset( c->data, 0xff, SIZE_DATA );
			}

			if( c->default_id != NULL )
			{
				memcpy( c->id, c->default_id, SIZE_ID );
			}
			else
			{
				memset( c->id, 0xff, SIZE_ID );
			}
		}
	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO(at28c16)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(at28c16_state); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(at28c16_config); break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_PERIPHERAL; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(at28c16); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(at28c16); break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME(at28c16); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "AT28C16"); break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "EEPROM"); break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0"); break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
