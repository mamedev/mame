/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 * Todo:
 *  Emulate write timing.
 *
 */

#include "driver.h"
#include "machine/at28c16.h"

#define SIZE_DATA ( 0x800 )
#define SIZE_ID ( 0x020 )
#define OFFSET_ID ( SIZE_DATA - SIZE_ID )

struct at28c16_chip
{
	UINT8 *data;
	UINT8 *id;
	UINT8 a9_12v;
};

static struct at28c16_chip at28c16[ MAX_AT28C16_CHIPS ];

void at28c16_a9_12v( int chip, int a9_12v )
{
	struct at28c16_chip *c;
	if( chip >= MAX_AT28C16_CHIPS )
	{
		logerror( "at28c16_a9_12v: invalid chip %d\n", chip );
		return;
	}
	c = &at28c16[ chip ];

	c->a9_12v = a9_12v;
}

/* nvram handlers */

void at28c16_init( int chip, UINT8 *data, UINT8 *id )
{
	struct at28c16_chip *c;
	if( chip >= MAX_AT28C16_CHIPS )
	{
		logerror( "at28c16_init: invalid chip %d\n", chip );
		return;
	}
	c = &at28c16[ chip ];

	c->a9_12v = 0;

	if( data == NULL )
	{
		data = auto_malloc( SIZE_DATA );
	}

	if( id == NULL )
	{
		id = auto_malloc( SIZE_ID );
	}

	c->data = data;
	c->id = id;

	state_save_register_item_pointer( "at28c16", chip, c->data, SIZE_DATA );
	state_save_register_item_pointer( "at28c16", chip, c->id, SIZE_ID );
	state_save_register_item( "at28c16", chip, c->a9_12v );
}

static void nvram_handler_at28c16( running_machine *machine, int chip, mame_file *file, int read_or_write )
{
	struct at28c16_chip *c;
	if( chip >= MAX_AT28C16_CHIPS )
	{
		logerror( "at28c16_nvram_handler: invalid chip %d\n", chip );
		return;
	}
	c = &at28c16[ chip ];

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
	}
}

NVRAM_HANDLER( at28c16_0 ) { nvram_handler_at28c16( machine, 0, file, read_or_write ); }
NVRAM_HANDLER( at28c16_1 ) { nvram_handler_at28c16( machine, 1, file, read_or_write ); }
NVRAM_HANDLER( at28c16_2 ) { nvram_handler_at28c16( machine, 2, file, read_or_write ); }
NVRAM_HANDLER( at28c16_3 ) { nvram_handler_at28c16( machine, 3, file, read_or_write ); }

/* read / write */

static UINT8 at28c16_read( UINT32 chip, offs_t offset )
{
	struct at28c16_chip *c;
	if( chip >= MAX_AT28C16_CHIPS )
	{
		logerror( "at28c16_read( %d, %04x ) chip out of range\n", chip, offset );
		return 0;
	}
	c = &at28c16[ chip ];

	if( offset >= OFFSET_ID && c->a9_12v )
	{
		logerror( "at28c16_read( %04x ) id\n", offset );
		return c->id[ offset - OFFSET_ID ];
	}
	else
	{
//      logerror( "%08x: at28c16_read( %d, %04x ) %02x data\n", activecpu_get_pc(), chip, offset, c->data[ offset ] );
		return c->data[ offset ];
	}
}

static void at28c16_write( UINT32 chip, offs_t offset, UINT8 data )
{
	struct at28c16_chip *c;
	if( chip >= MAX_AT28C16_CHIPS )
	{
		logerror( "at28c16_write( %d, %04x, %02x ) chip out of range\n", chip, offset, data );
		return;
	}
	c = &at28c16[ chip ];

	if( offset >= OFFSET_ID && c->a9_12v )
	{
		logerror( "at28c16_write( %d, %04x, %02x ) id\n", chip, offset, data );
		c->id[ offset - OFFSET_ID ] = data;
	}
	else
	{
//      logerror( "%08x: at28c16_write( %d, %04x, %02x ) data\n", activecpu_get_pc(), chip, offset, data );
		c->data[ offset ] = data;
	}
}

READ8_HANDLER( at28c16_0_r ) { return at28c16_read( 0, offset ); }
READ8_HANDLER( at28c16_1_r ) { return at28c16_read( 1, offset ); }
READ8_HANDLER( at28c16_2_r ) { return at28c16_read( 2, offset ); }
READ8_HANDLER( at28c16_3_r ) { return at28c16_read( 3, offset ); }
WRITE8_HANDLER( at28c16_0_w ) { at28c16_write( 0, offset, data ); }
WRITE8_HANDLER( at28c16_1_w ) { at28c16_write( 1, offset, data ); }
WRITE8_HANDLER( at28c16_2_w ) { at28c16_write( 2, offset, data ); }
WRITE8_HANDLER( at28c16_3_w ) { at28c16_write( 3, offset, data ); }
