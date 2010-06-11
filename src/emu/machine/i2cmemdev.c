/*
   I2C Memory

Generic ram/rom/eeprom/flash on an i2c bus. Supports specifying the slave address,
the data size & the page size for writing.

inputs:
 e0,e1,e2  lower 3 bits of the slave address
 sda       serial data
 scl       serial clock
 wc        write protect

outputs:
 sda       serial data

The memory address is only 8 bits, devices larger than this have multiple slave addresses.
The top five address bits are set at manufacture time, two values are standard.
Up to 4096 bytes can be addressed.

*/

#include "emu.h"
#include "machine/i2cmemdev.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

#define STATE_IDLE ( 0 )
#define STATE_DEVSEL ( 1 )
#define STATE_BYTEADDR ( 2 )
#define STATE_DATAIN ( 3 )
#define STATE_DATAOUT ( 4 )

#define DEVSEL_RW ( 1 )
#define DEVSEL_ADDRESS ( 0xfe )

typedef struct _i2cmem_state i2cmem_state;
struct _i2cmem_state
{
	int slave_address;
	int scl;
	int sdaw;
	int e0;
	int e1;
	int e2;
	int wc;
	int sdar;
	int state;
	int bits;
	int shift;
	int devsel;
	int byteaddr;
	unsigned char *data;
	int data_size;
	unsigned char *page;
	int page_offset;
	int page_size;
	int readmode;
};

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an I2C memory
-------------------------------------------------*/

INLINE i2cmem_state *get_safe_token( running_device *device )
{
	assert( device != NULL );
	assert( device->type() == I2CMEM );

	return (i2cmem_state *)downcast<legacy_device_base *>(device)->token();
}

static int select_device( i2cmem_state *c )
{
	int device = ( c->slave_address & 0xf0 ) | ( c->e2 << 3 ) | ( c->e1 << 2 ) | ( c->e0 << 1 );
	int mask = DEVSEL_ADDRESS & ~( ( c->data_size - 1 ) >> 7 );

	if( ( c->devsel & mask ) == ( device & mask ) )
	{
		return 1;
	}

	return 0;
}

static int data_offset( i2cmem_state *c )
{
	return ( ( ( c->devsel << 7 ) & 0xff00 ) | ( c->byteaddr & 0xff ) ) & ( c->data_size - 1 );
}

void i2cmemdev_write( running_device *device, int line, int data )
{
	i2cmem_state *c = get_safe_token( device );

	switch( line )
	{
	case I2CMEM_E0:
		if( c->e0 != data )
		{
			c->e0 = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_E0, %d )\n", c->e0 );
		}
		break;

	case I2CMEM_E1:
		if( c->e1 != data )
		{
			c->e1 = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_E1, %d )\n", c->e1 );
		}
		break;

	case I2CMEM_E2:
		if( c->e2 != data )
		{
			c->e2 = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_E2, %d )\n", c->e2 );
		}
		break;

	case I2CMEM_SDA:
		if( c->sdaw != data )
		{
			c->sdaw = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_SDA, %d )\n", c->sdaw );

			if( c->scl )
			{
				if( c->sdaw )
				{
					verboselog( device->machine, 1, "i2cmem stop\n");
					c->state = STATE_IDLE;
					c->byteaddr = 0;
				}
				else
				{
					verboselog( device->machine, 2, "i2cmem start\n");
					c->state = STATE_DEVSEL;
					c->bits = 0;
				}

				c->sdar = 1;
			}
		}
		break;

	case I2CMEM_SCL:
		if( c->scl != data )
		{
			c->scl = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_SCL, %d )\n", c->scl );

			switch( c->state )
			{
			case STATE_DEVSEL:
			case STATE_BYTEADDR:
			case STATE_DATAIN:
				if( c->bits < 8 )
				{
					if( c->scl )
					{
						c->shift = ( ( c->shift << 1 ) | c->sdaw ) & 0xff;
						c->bits++;
					}
				}
				else
				{
					if( c->scl )
					{
						switch( c->state )
						{
						case STATE_DEVSEL:
							c->devsel = c->shift;

							if( !select_device( c ) )
							{
								verboselog( device->machine, 1, "i2cmem devsel %02x: not this device\n", c->devsel );
								c->state = STATE_IDLE;
							}
							else if( ( c->devsel & DEVSEL_RW ) == 0 )
							{
								verboselog( device->machine, 1, "i2cmem devsel %02x: write\n", c->devsel );
								c->state = STATE_BYTEADDR;
							}
							else
							{
								verboselog( device->machine, 1, "i2cmem devsel %02x: read\n", c->devsel );
								c->state = STATE_DATAOUT;
							}
							break;

						case STATE_BYTEADDR:
							c->byteaddr = c->shift;
							c->page_offset = 0;

							verboselog( device->machine, 1, "i2cmem byteaddr %02x\n", c->byteaddr );

							c->state = STATE_DATAIN;
							break;

						case STATE_DATAIN:
							if( c->wc )
							{
								verboselog( device->machine, 0, "i2cmem write not enabled\n");
								c->state = STATE_IDLE;
							}
							else if( c->page_size > 0 )
							{
								c->page[ c->page_offset ] = c->shift;
								verboselog( device->machine, 1, "i2cmem page[ %04x ] <- %02x\n", c->page_offset, c->page[ c->page_offset ] );

								c->page_offset++;
								if( c->page_offset == c->page_size )
								{
									int offset = data_offset( c ) & ~( c->page_size - 1 );

									memcpy( &c->data[ offset ], c->page, c->page_size );
									verboselog( device->machine, 1, "i2cmem data[ %04x to %04x ] = page\n", offset, offset + c->page_size - 1 );

									c->page_offset = 0;
								}
							}
							else
							{
								int offset = data_offset( c );

								c->data[ offset ] = c->shift;
								verboselog( device->machine, 1, "i2cmem data[ %04x ] <- %02x\n", offset, c->data[ offset ] );

								c->byteaddr++;
							}
							break;
						}

						c->bits++;
					}
					else
					{
						if( c->bits == 8 )
						{
							c->sdar = 0;
						}
						else
						{
							c->bits = 0;
							c->sdar = 1;
						}
					}
				}
				break;

			case STATE_DATAOUT:
				if( c->bits < 8 )
				{
					if( c->scl )
					{
						if( c->bits == 0 )
						{
							int offset = data_offset( c );

							c->shift = c->data[ offset ];
							verboselog( device->machine, 1, "i2cmem data[ %04x ] -> %02x\n", offset, c->data[ offset ] );
							c->byteaddr++;
						}

						c->sdar = ( c->shift >> 7 ) & 1;

						c->shift = ( c->shift << 1 ) & 0xff;
						c->bits++;
					}
				}
				else
				{
					if( c->scl )
					{
						if( c->sdaw )
						{
							verboselog( device->machine, 1, "i2cmem sleep\n");
							c->state = STATE_IDLE;
						}

						c->bits++;
					}
					else
					{
						if( c->bits == 8 )
						{
							c->sdar = 1;
						}
						else
						{
							c->bits = 0;
						}
					}
				}
				break;
			}
		}
		break;

	case I2CMEM_WC:
		if( c->wc != data )
		{
			c->wc = data;
			verboselog( device->machine, 2, "i2cmem_write( I2CMEM_WC, %d )\n", c->wc );
		}
		break;

	default:
		verboselog( device->machine, 0, "i2cmem_write( %d, %d ) invalid line\n", line, data );
		break;
	}
}

int i2cmemdev_read( running_device *device, int line )
{
	i2cmem_state *c = get_safe_token( device );

	switch( line )
	{
	case I2CMEM_SDA:
		if (c->readmode == 0)
		{
			verboselog( device->machine, 2, "i2cmem_read( I2CMEM_SDA ) %d\n", c->sdar & c->sdaw );
			return c->sdar & c->sdaw;
		}
		else
		{
			verboselog( device->machine, 2, "i2cmem_read( I2CMEM_SDA ) %d\n", c->sdar );
			return c->sdar;
		}

	default:
		verboselog( device->machine, 0, "i2cmem_read( %d ) invalid line\n", line );
		break;
	}

	return 0;
}

void i2cmemdev_set_read_mode( running_device *device, int mode )
{
	i2cmem_state *c = get_safe_token( device );

	c->readmode = mode;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( i2cmem )
{
	i2cmem_state *c = get_safe_token( device );
	const i2cmem_config *config;
	unsigned char *page = NULL;

	/* validate some basic stuff */
	assert( device != NULL );
	assert( downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config() != NULL );
	assert( device->machine != NULL );
	assert( device->machine->config != NULL );

	config = (const i2cmem_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();

	c->scl = 0;
	c->sdaw = 0;
	c->e0 = 0;
	c->e1 = 0;
	c->e2 = 0;
	c->wc = 0;
	c->sdar = 1;
	c->state = STATE_IDLE;
	c->bits = 0;
	c->shift = 0;
	c->devsel = 0;
	c->byteaddr = 0;
	c->readmode = 0;

	if( config != NULL )
	{
		c->data = auto_alloc_array( device->machine, UINT8, config->data_size );
		memcpy(c->data, config->data, config->data_size);

		if( config->page_size > 0 )
			page = auto_alloc_array( device->machine, UINT8, config->page_size );

		c->slave_address = config->slave_address;
		c->data_size = config->data_size;
		c->page_size = config->page_size;
		c->page = page;
	}

	state_save_register_device_item( device, 0, c->scl );
	state_save_register_device_item( device, 0, c->sdaw );
	state_save_register_device_item( device, 0, c->e0 );
	state_save_register_device_item( device, 0, c->e1 );
	state_save_register_device_item( device, 0, c->e2 );
	state_save_register_device_item( device, 0, c->wc );
	state_save_register_device_item( device, 0, c->sdar );
	state_save_register_device_item( device, 0, c->state );
	state_save_register_device_item( device, 0, c->bits );
	state_save_register_device_item( device, 0, c->shift );
	state_save_register_device_item( device, 0, c->devsel );
	state_save_register_device_item( device, 0, c->byteaddr );
	state_save_register_device_item_pointer( device, 0, c->data, c->data_size );
	state_save_register_device_item( device, 0, c->readmode );
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( i2cmem )
{
}

static DEVICE_NVRAM( i2cmem )
{
	const i2cmem_config *config = (const i2cmem_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
	i2cmem_state *c = get_safe_token( device );

	if( read_or_write )
	{
		mame_fwrite( file, c->data, c->data_size );
	}
	else if( file )
	{
		mame_fread( file, c->data, c->data_size );
	}
	else
	{
		if( config->data != NULL )
			memcpy( c->data, config->data, config->data_size );
	}
}

/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( i2cmem )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof( i2cmem_state ); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof( i2cmem_config ); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME( i2cmem ); break;
		case DEVINFO_FCT_STOP:					/* nothing */ break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME( i2cmem ); break;
		case DEVINFO_FCT_NVRAM:					info->nvram = DEVICE_NVRAM_NAME( i2cmem ); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy( info->s, "I2CMEM" ); break;
		case DEVINFO_STR_FAMILY:				strcpy( info->s, "EEPROM" ); break;
		case DEVINFO_STR_VERSION:				strcpy( info->s, "1.0" ); break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy( info->s, __FILE__ ); break;
		case DEVINFO_STR_CREDITS:				strcpy( info->s, "Copyright Nicola Salmoria and the MAME Team" ); break;
	}
}


DEFINE_LEGACY_NVRAM_DEVICE(I2CMEM, i2cmem);
