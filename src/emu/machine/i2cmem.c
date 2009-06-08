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

#include "driver.h"
#include "machine/i2cmem.h"

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

struct i2cmem_chip
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
	UINT8 *data;
	int data_size;
	UINT8 *page;
	int page_offset;
	int page_size;
};

#define STATE_IDLE ( 0 )
#define STATE_DEVSEL ( 1 )
#define STATE_BYTEADDR ( 2 )
#define STATE_DATAIN ( 3 )
#define STATE_DATAOUT ( 4 )

#define DEVSEL_RW ( 1 )
#define DEVSEL_ADDRESS ( 0xfe )

static struct i2cmem_chip i2cmem[ I2CMEM_MAXCHIP ];

void i2cmem_init( running_machine *machine, int chip, int slave_address, int page_size, int data_size, UINT8 *data )
{
	struct i2cmem_chip *c;
	UINT8 *page = NULL;

	if( chip >= I2CMEM_MAXCHIP )
	{
		verboselog( machine, 0, "i2cmem_init( %d ) invalid chip\n", chip );
		return;
	}

	c = &i2cmem[ chip ];

	if( data == NULL )
	{
		data = auto_alloc_array( machine, UINT8, data_size );
	}

	if( page_size > 0 )
	{
		page = auto_alloc_array( machine, UINT8, page_size );
	}

	c->slave_address = slave_address;
	c->data_size = data_size;
	c->page_size = page_size;

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
	c->data = data;
	c->page = page;

	state_save_register_item( machine, "i2cmem", NULL, chip, c->scl );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->sdaw );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->e0 );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->e1 );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->e2 );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->wc );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->sdar );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->state );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->bits );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->shift );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->devsel );
	state_save_register_item( machine, "i2cmem", NULL, chip, c->byteaddr );
	state_save_register_item_pointer( machine, "i2cmem", NULL, chip, c->data, c->data_size );
}

static int select_device( struct i2cmem_chip *c )
{
	int device = ( c->slave_address & 0xf0 ) | ( c->e2 << 3 ) | ( c->e1 << 2 ) | ( c->e0 << 1 );
	int mask = DEVSEL_ADDRESS & ~( ( c->data_size - 1 ) >> 7 );

	if( ( c->devsel & mask ) == ( device & mask ) )
	{
		return 1;
	}

	return 0;
}

static int data_offset( struct i2cmem_chip *c )
{
	return ( ( ( c->devsel << 7 ) & 0xff00 ) | ( c->byteaddr & 0xff ) ) & ( c->data_size - 1 );
}

void i2cmem_write( running_machine *machine, int chip, int line, int data )
{
	struct i2cmem_chip *c;

	if( chip >= I2CMEM_MAXCHIP )
	{
		verboselog( machine, 0, "i2cmem_write( %d, %d, %d ) invalid chip\n", chip, line, data );
		return;
	}

	c = &i2cmem[ chip ];

	data &= 1;

	switch( line )
	{
	case I2CMEM_E0:
		if( c->e0 != data )
		{
			c->e0 = data;
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_E0, %d )\n", chip, c->e0 );
		}
		break;

	case I2CMEM_E1:
		if( c->e1 != data )
		{
			c->e1 = data;
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_E1, %d )\n", chip, c->e1 );
		}
		break;

	case I2CMEM_E2:
		if( c->e2 != data )
		{
			c->e2 = data;
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_E2, %d )\n", chip, c->e2 );
		}
		break;

	case I2CMEM_SDA:
		if( c->sdaw != data )
		{
			c->sdaw = data;
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_SDA, %d )\n", chip, c->sdaw );

			if( c->scl )
			{
				if( c->sdaw )
				{
					verboselog( machine, 1, "i2cmem(%d) stop\n", chip );
					c->state = STATE_IDLE;
					c->byteaddr = 0;
				}
				else
				{
					verboselog( machine, 2, "i2cmem(%d) start\n", chip );
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
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_SCL, %d )\n", chip, c->scl );

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
								verboselog( machine, 1, "i2cmem(%d) devsel %02x: not this device\n", chip, c->devsel );
								c->state = STATE_IDLE;
							}
							else if( ( c->devsel & DEVSEL_RW ) == 0 )
							{
								verboselog( machine, 1, "i2cmem(%d) devsel %02x: write\n", chip, c->devsel );
								c->state = STATE_BYTEADDR;
							}
							else
							{
								verboselog( machine, 1, "i2cmem(%d) devsel %02x: read\n", chip, c->devsel );
								c->state = STATE_DATAOUT;
							}
							break;

						case STATE_BYTEADDR:
							c->byteaddr = c->shift;
							c->page_offset = 0;

							verboselog( machine, 1, "i2cmem(%d) byteaddr %02x\n", chip, c->byteaddr );

							c->state = STATE_DATAIN;
							break;

						case STATE_DATAIN:
							if( c->wc )
							{
								verboselog( machine, 0, "i2cmem(%d) write not enabled\n", chip );
								c->state = STATE_IDLE;
							}
							else if( c->page_size > 0 )
							{
								c->page[ c->page_offset ] = c->shift;
								verboselog( machine, 1, "i2cmem(%d) page[ %04x ] <- %02x\n", chip, c->page_offset, c->page[ c->page_offset ] );

								c->page_offset++;
								if( c->page_offset == c->page_size )
								{
									int offset = data_offset( c ) & ~( c->page_size - 1 );

									memcpy( &c->data[ offset ], c->page, c->page_size );
									verboselog( machine, 1, "i2cmem(%d) data[ %04x to %04x ] = page\n", chip, offset, offset + c->page_size - 1 );

									c->page_offset = 0;
								}
							}
							else
							{
								int offset = data_offset( c );

								c->data[ offset ] = c->shift;
								verboselog( machine, 1, "i2cmem(%d) data[ %04x ] <- %02x\n", chip, offset, c->data[ offset ] );

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
							verboselog( machine, 1, "i2cmem(%d) data[ %04x ] -> %02x\n", chip, offset, c->data[ offset ] );
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
							verboselog( machine, 1, "i2cmem(%d) sleep\n", chip );
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
			verboselog( machine, 2, "i2cmem_write( %d, I2CMEM_WC, %d )\n", chip, c->wc );
		}
		break;

	default:
		verboselog( machine, 0, "i2cmem_write( %d, %d, %d ) invalid line\n", chip, line, data );
		break;
	}
}

int i2cmem_read( running_machine *machine, int chip, int line )
{
	struct i2cmem_chip *c;

	if( chip >= I2CMEM_MAXCHIP )
	{
		verboselog( machine, 0, "i2cmem_read( %d, %d ) invalid chip\n", chip, line );
		return 0;
	}

	c = &i2cmem[ chip ];

	switch( line )
	{
	case I2CMEM_SDA:
		verboselog( machine, 2, "i2cmem_read( %d, I2CMEM_SDA ) %d\n", chip, c->sdar & c->sdaw );
		return c->sdar & c->sdaw;

	default:
		verboselog( machine, 0, "i2cmem_read( %d, %d ) invalid line\n", chip, line );
		break;
	}

	return 0;
}

static void nvram_handler_i2cmem( running_machine *machine, mame_file *file, int read_or_write, int chip )
{
	struct i2cmem_chip *c;

	if( chip >= I2CMEM_MAXCHIP )
	{
		verboselog( machine, 0, "nvram_handler_i2cmem( %d ) invalid chip\n", chip );
		return;
	}

	c = &i2cmem[ chip ];

	if( read_or_write )
	{
		mame_fwrite( file, c->data, c->data_size );
	}
	else if( file )
	{
		mame_fread( file, c->data, c->data_size );
	}
}

NVRAM_HANDLER( i2cmem_0 ) { nvram_handler_i2cmem( machine, file, read_or_write, 0 ); }
