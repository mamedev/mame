/*
 * x76f100.c
 *
 * Secure SerialFlash
 *
 * The X76F100 is a Password Access Security Supervisor, containing one 896-bit Secure SerialFlash array.
 * Access to the memory array can be controlled by two 64-bit passwords. These passwords protect read and
 * write operations of the memory array.
 *
 */

#include "driver.h"
#include "machine/x76f100.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		if( cpu_getactivecpu() != -1 )
		{
			logerror( "%08x: %s", activecpu_get_pc(), buf );
		}
		else
		{
			logerror( "(timer) : %s", buf );
		}
	}
}

#define SIZE_WRITE_BUFFER ( 8 )

struct x76f100_chip
{
	int cs;
	int rst;
	int scl;
	int sdaw;
	int sdar;
	int state;
	int shift;
	int bit;
	int byte;
	int command;
	UINT8 write_buffer[ SIZE_WRITE_BUFFER ];
	UINT8 *response_to_reset;
	UINT8 *write_password;
	UINT8 *read_password;
	UINT8 *data;
};

#define SIZE_RESPONSE_TO_RESET ( 4 )
#define SIZE_WRITE_PASSWORD ( SIZE_WRITE_BUFFER )
#define SIZE_READ_PASSWORD ( SIZE_WRITE_BUFFER )
#define SIZE_DATA ( 112 )

struct x76f100_chip x76f100[ X76F100_MAXCHIP ];

#define COMMAND_WRITE ( 0x80 )
#define COMMAND_READ ( 0x81 )
#define COMMAND_CHANGE_WRITE_PASSWORD ( 0xfc )
#define COMMAND_CHANGE_READ_PASSWORD ( 0xfe )
#define COMMAND_ACK_PASSWORD ( 0x55 )

#define STATE_STOP ( 0 )
#define STATE_RESPONSE_TO_RESET ( 1 )
#define STATE_LOAD_COMMAND ( 2 )
#define STATE_LOAD_PASSWORD ( 4 )
#define STATE_VERIFY_PASSWORD ( 5 )
#define STATE_READ_DATA ( 6 )
#define STATE_WRITE_DATA ( 7 )

void x76f100_init( int chip, UINT8 *data )
{
	int offset;
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_init( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( data == NULL )
	{
		data = auto_malloc(
			SIZE_RESPONSE_TO_RESET +
			SIZE_READ_PASSWORD +
			SIZE_WRITE_PASSWORD +
			SIZE_DATA );
	}

	c->cs = 0;
	c->rst = 0;
	c->scl = 0;
	c->sdaw = 0;
	c->sdar = 0;
	c->state = STATE_STOP;
	c->shift = 0;
	c->bit = 0;
	c->byte = 0;
	c->command = 0;
	memset( c->write_buffer, 0, SIZE_WRITE_BUFFER );

	offset = 0;
	c->response_to_reset = &data[ offset ]; offset += SIZE_RESPONSE_TO_RESET;
	c->write_password = &data[ offset ]; offset += SIZE_WRITE_PASSWORD;
	c->read_password = &data[ offset ]; offset += SIZE_READ_PASSWORD;
	c->data = &data[ offset ]; offset += SIZE_DATA;

	state_save_register_item( "x76f100", chip, c->cs );
	state_save_register_item( "x76f100", chip, c->rst );
	state_save_register_item( "x76f100", chip, c->scl );
	state_save_register_item( "x76f100", chip, c->sdaw );
	state_save_register_item( "x76f100", chip, c->sdar );
	state_save_register_item( "x76f100", chip, c->state );
	state_save_register_item( "x76f100", chip, c->shift );
	state_save_register_item( "x76f100", chip, c->bit );
	state_save_register_item( "x76f100", chip, c->byte );
	state_save_register_item( "x76f100", chip, c->command );
	state_save_register_item_array( "x76f100", chip, c->write_buffer );
	state_save_register_item_pointer( "x76f100", chip, c->response_to_reset, SIZE_RESPONSE_TO_RESET );
	state_save_register_item_pointer( "x76f100", chip, c->write_password, SIZE_WRITE_PASSWORD );
	state_save_register_item_pointer( "x76f100", chip, c->read_password, SIZE_READ_PASSWORD );
	state_save_register_item_pointer( "x76f100", chip, c->data, SIZE_DATA );
}

void x76f100_cs_write( int chip, int cs )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_cs_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( c->cs != cs )
	{
		verboselog( 2, "x76f100(%d) cs=%d\n", chip, cs );
	}
	if( c->cs != 0 && cs == 0 )
	{
		/* enable chip */
		c->state = STATE_STOP;
	}
	if( c->cs == 0 && cs != 0 )
	{
		/* disable chip */
		c->state = STATE_STOP;
		/* high impendence? */
		c->sdar = 0;
	}
	c->cs = cs;
}

void x76f100_rst_write( int chip, int rst )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_rst_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( c->rst != rst )
	{
		verboselog( 2, "x76f100(%d) rst=%d\n", chip, rst );
	}
	if( c->rst == 0 && rst != 0 && c->cs == 0 )
	{
		verboselog( 1, "x76f100(%d) goto response to reset\n", chip );
		c->state = STATE_RESPONSE_TO_RESET;
		c->bit = 0;
		c->byte = 0;
	}
	c->rst = rst;
}

static UINT8 *x76f100_password( struct x76f100_chip *c )
{
	if( ( c->command & 0xe1 ) == COMMAND_READ )
	{
		return c->read_password;
	}

	return c->write_password;
}

static void x76f100_password_ok( struct x76f100_chip *c )
{
	if( ( c->command & 0xe1 ) == COMMAND_READ )
	{
		c->state = STATE_READ_DATA;
	}
	else if( ( c->command & 0xe1 ) == COMMAND_WRITE )
	{
		c->state = STATE_WRITE_DATA;
	}
	else
	{
		/* TODO: */
	}
}

static int x76f100_data_offset( struct x76f100_chip *c )
{
	int block_offset = ( c->command >> 1 ) & 0x0f;

	return ( block_offset * SIZE_WRITE_BUFFER ) + c->byte;
}

void x76f100_scl_write( int chip, int scl )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_scl_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( c->scl != scl )
	{
		verboselog( 2, "x76f100(%d) scl=%d\n", chip, scl );
	}
	if( c->cs == 0 )
	{
		switch( c->state )
		{
		case STATE_STOP:
			break;

		case STATE_RESPONSE_TO_RESET:
			if( c->scl != 0 && scl == 0 )
			{
				if( c->bit == 0 )
				{
					c->shift = c->response_to_reset[ c->byte ];
					verboselog( 1, "x76f100(%d) <- response_to_reset[%d]: %02x\n", chip, c->byte, c->shift );
				}

				c->sdar = c->shift & 1;
				c->shift >>= 1;
				c->bit++;

				if( c->bit == 8 )
				{
					c->bit = 0;
					c->byte++;
					if( c->byte == 4 )
					{
						c->byte = 0;
					}
				}
			}
			break;

		case STATE_LOAD_COMMAND:
		case STATE_LOAD_PASSWORD:
		case STATE_VERIFY_PASSWORD:
		case STATE_WRITE_DATA:
			if( c->scl == 0 && scl != 0 )
			{
				if( c->bit < 8 )
				{
					verboselog( 2, "x76f100(%d) clock\n", chip );
					c->shift <<= 1;
					if( c->sdaw != 0 )
					{
						c->shift |= 1;
					}
					c->bit++;
				}
				else
				{
					c->sdar = 0;

					switch( c->state )
					{
					case STATE_LOAD_COMMAND:
						c->command = c->shift;
						verboselog( 1, "x76f100(%d) -> command: %02x\n", chip, c->command );
						/* todo: verify command is valid? */
						c->state = STATE_LOAD_PASSWORD;
						break;

					case STATE_LOAD_PASSWORD:
						verboselog( 1, "x76f100(%d) -> password: %02x\n", chip, c->shift );
						c->write_buffer[ c->byte++ ] = c->shift;
						if( c->byte == SIZE_WRITE_BUFFER )
						{
							c->state = STATE_VERIFY_PASSWORD;
						}
						break;

					case STATE_VERIFY_PASSWORD:
						verboselog( 1, "x76f100(%d) -> verify password: %02x\n", chip, c->shift );
						/* todo: this should probably be handled as a command */
						if( c->shift == COMMAND_ACK_PASSWORD )
						{
							/* todo: this should take 10ms before it returns ok. */
							if( memcmp( x76f100_password( c ), c->write_buffer, SIZE_WRITE_BUFFER ) == 0 )
							{
								x76f100_password_ok( c );
							}
							else
							{
								c->sdar = 1;
							}
						}
						break;

					case STATE_WRITE_DATA:
						verboselog( 1, "x76f100(%d) -> data: %02x\n", chip, c->shift );
						c->write_buffer[ c->byte++ ] = c->shift;
						if( c->byte == SIZE_WRITE_BUFFER )
						{
							for( c->byte = 0; c->byte < SIZE_WRITE_BUFFER; c->byte++ )
							{
								c->data[ x76f100_data_offset( c ) ] = c->write_buffer[ c->byte ];
							}
							c->byte = 0;

							verboselog( 1, "x76f100(%d) data flushed\n", chip );
						}
						break;
					}

					c->bit = 0;
					c->shift = 0;
				}
			}
			break;

		case STATE_READ_DATA:
			if( c->scl == 0 && scl != 0 )
			{
				if( c->bit < 8 )
				{
					if( c->bit == 0 )
					{
						switch( c->state )
						{
						case STATE_READ_DATA:
							c->shift = c->data[ x76f100_data_offset( c ) ];
							verboselog( 1, "x76f100(%d) <- data: %02x\n", chip, c->shift );
							break;
						}
					}
					c->sdar = ( c->shift >> 7 ) & 1;
					c->shift <<= 1;
					c->bit++;
				}
				else
				{
					c->bit = 0;
					c->sdar = 0;
					if( c->sdaw == 0 )
					{
						verboselog( 2, "x76f100(%d) ack <-\n", chip );
						c->byte++;
					}
					else
					{
						verboselog( 2, "x76f100(%d) nak <-\n", chip );
					}
				}
			}
			break;
		}
	}
	c->scl = scl;
}

void x76f100_sda_write( int chip, int sda )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_sda_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( c->sdaw != sda )
	{
		verboselog( 2, "x76f100(%d) sdaw=%d\n", chip, sda );
	}
	if( c->cs == 0 && c->scl != 0 )
	{
		if( c->sdaw == 0 && sda != 0 )
		{
			verboselog( 1, "x76f100(%d) goto stop\n", chip );
			c->state = STATE_STOP;
			c->sdar = 0;
		}
		if( c->sdaw != 0 && sda == 0 )
		{
			switch( c->state )
			{
			case STATE_STOP:
				verboselog( 1, "x76f100(%d) goto start\n", chip );
				c->state = STATE_LOAD_COMMAND;
				break;

			case STATE_LOAD_PASSWORD:
				/* todo: this will be the 0xc0 command, but it's not handled as a command yet. */
				verboselog( 1, "x76f100(%d) goto start\n", chip );
				break;

			case STATE_READ_DATA:
				verboselog( 1, "x76f100(%d) continue reading??\n", chip );
//              verboselog( 1, "x76f100(%d) goto load address\n", chip );
//              c->state = STATE_LOAD_ADDRESS;
				break;

			default:
				verboselog( 1, "x76f100(%d) skipped start (default)\n", chip );
				break;
			}

			c->bit = 0;
			c->byte = 0;
			c->shift = 0;
			c->sdar = 0;
		}
	}
	c->sdaw = sda;
}

int x76f100_sda_read( int chip )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "x76f100_sda_read( %d ) chip out of range\n", chip );
		return 1;
	}

	c = &x76f100[ chip ];

	if( c->cs != 0 )
	{
		verboselog( 2, "x76f100(%d) not selected\n", chip );
		return 1;
	}
	verboselog( 2, "x76f100(%d) sdar=%d\n", chip, c->sdar );
	return c->sdar;
}

static void nvram_handler_x76f100( int chip, running_machine *machine, mame_file *file, int read_or_write )
{
	struct x76f100_chip *c;

	if( chip >= X76F100_MAXCHIP )
	{
		verboselog( 0, "nvram_handler_x76f100( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f100[ chip ];

	if( read_or_write )
	{
		mame_fwrite( file, c->write_password, SIZE_WRITE_PASSWORD );
		mame_fwrite( file, c->read_password, SIZE_READ_PASSWORD );
		mame_fwrite( file, c->data, SIZE_DATA );
	}
	else if( file )
	{
		mame_fread( file, c->write_password, SIZE_WRITE_PASSWORD );
		mame_fread( file, c->read_password, SIZE_READ_PASSWORD );
		mame_fread( file, c->data, SIZE_DATA );
	}
}

NVRAM_HANDLER( x76f100_0 ) { nvram_handler_x76f100( 0, machine, file, read_or_write ); }
NVRAM_HANDLER( x76f100_1 ) { nvram_handler_x76f100( 1, machine, file, read_or_write ); }
