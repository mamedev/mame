/*
 * zs01.c
 *
 * Secure SerialFlash
 *
 * This is a high level emulation of the PIC used in some of the System 573 security cartridges.
 *
 */

#include "driver.h"
#include "machine/zs01.h"

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

#define SIZE_WRITE_BUFFER ( 12 )
#define SIZE_READ_BUFFER ( 12 )
#define SIZE_DATA_BUFFER ( 8 )

#define SIZE_RESPONSE_TO_RESET ( 4 )
#define SIZE_KEY ( 8 )
#define SIZE_DATA ( 4096 )

struct zs01_chip
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
	UINT8 write_buffer[ SIZE_WRITE_BUFFER ];
	UINT8 read_buffer[ SIZE_READ_BUFFER ];
	UINT8 response_key[ SIZE_KEY ];
	UINT8 *response_to_reset;
	UINT8 *command_key;
	UINT8 *data_key;
	UINT8 *data;
	UINT8 *ds2401;
	zs01_write_handler write;
	zs01_read_handler read;
};

static struct zs01_chip zs01[ ZS01_MAXCHIP ];

#define COMMAND_WRITE ( 0x00 )
#define COMMAND_READ ( 0x01 )

#define STATE_STOP ( 0 )
#define STATE_RESPONSE_TO_RESET ( 1 )
#define STATE_LOAD_COMMAND ( 2 )
#define STATE_READ_DATA ( 3 )

void zs01_init( int chip, UINT8 *data, zs01_write_handler write, zs01_read_handler read, UINT8 *ds2401 )
{
	int offset;
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_init( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( data == NULL )
	{
		data = auto_malloc(
			SIZE_RESPONSE_TO_RESET +
			SIZE_KEY +
			SIZE_KEY +
			SIZE_DATA );
	}

	if( ds2401 == NULL )
	{
		ds2401 = auto_malloc( SIZE_DATA_BUFFER );
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
	memset( c->write_buffer, 0, SIZE_WRITE_BUFFER );
	memset( c->read_buffer, 0, SIZE_READ_BUFFER );
	memset( c->response_key, 0, SIZE_KEY );

	offset = 0;
	c->response_to_reset = &data[ offset ]; offset += SIZE_RESPONSE_TO_RESET;
	c->command_key = &data[ offset ]; offset += SIZE_KEY;
	c->data_key = &data[ offset ]; offset += SIZE_KEY;
	c->data = &data[ offset ]; offset += SIZE_DATA;
	c->ds2401 = ds2401;
	c->write = write;
	c->read = read;

	state_save_register_item( "zs01", chip, c->cs );
	state_save_register_item( "zs01", chip, c->rst );
	state_save_register_item( "zs01", chip, c->scl );
	state_save_register_item( "zs01", chip, c->sdaw );
	state_save_register_item( "zs01", chip, c->sdar );
	state_save_register_item( "zs01", chip, c->state );
	state_save_register_item( "zs01", chip, c->shift );
	state_save_register_item( "zs01", chip, c->bit );
	state_save_register_item( "zs01", chip, c->byte );
	state_save_register_item_array( "zs01", chip, c->write_buffer );
	state_save_register_item_array( "zs01", chip, c->read_buffer );
	state_save_register_item_array( "zs01", chip, c->response_key );
	state_save_register_item_pointer( "zs01", chip, c->response_to_reset, SIZE_RESPONSE_TO_RESET );
	state_save_register_item_pointer( "zs01", chip, c->command_key, SIZE_KEY );
	state_save_register_item_pointer( "zs01", chip, c->data_key, SIZE_DATA );
}

void zs01_rst_write( int chip, int rst )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_rst_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( c->rst != rst )
	{
		verboselog( 2, "zs01(%d) rst=%d\n", chip, rst );
	}
	if( c->rst == 0 && rst != 0 && c->cs == 0 )
	{
		verboselog( 1, "zs01(%d) goto response to reset\n", chip );
		c->state = STATE_RESPONSE_TO_RESET;
		c->bit = 0;
		c->byte = 0;
	}
	c->rst = rst;
}

void zs01_cs_write( int chip, int cs )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_cs_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( c->cs != cs )
	{
		verboselog( 2, "zs01(%d) cs=%d\n", chip, cs );
	}
//  if( c->cs != 0 && cs == 0 )
//  {
//      /* enable chip */
//      c->state = STATE_STOP;
//  }
//  if( c->cs == 0 && cs != 0 )
//  {
//      /* disable chip */
//      c->state = STATE_STOP;
//      /* high impendence? */
//      c->sdar = 0;
//  }
	c->cs = cs;
}

static void zs01_decrypt( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte )
{
	UINT32 a0;
	UINT32 v1;
	UINT32 v0;
	UINT32 a1;
	UINT32 t1;
	UINT32 t0;

	length--;
	if( length >= 0 )
	{
		do
		{
			t1 = source[ length ];
			a1 = 7;
			t0 = t1;

			do
			{
				v1 = key[ a1 ];
				a1--;
				v0 = v1 & 0x1f;
				v0 = t0 - v0;
				v1 >>= 5;
				v0 &= 0xff;
				a0 = (signed)v0 >> v1;
				v1 = 8 - v1;
				v1 &= 7;
				v0 = (signed)v0 << v1;
				t0 = a0 | v0;
			} while( a1 > 0 );

			v1 = key[ 0 ];
			a0 = previous_byte;
			v0 = t0 & 0xff;
			previous_byte = t1;
			v0 = v0 - v1;
			v0 = v0 ^ a0;

			destination[ length ] = v0;
			length--;
		}
		while( length >= 0 );
	}
}

static void zs01_decrypt2( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT8 previous_byte )
{
	UINT32 a0;
	UINT32 v1;
	UINT32 v0;
	UINT32 a1;
	UINT32 t2;
	UINT32 t1;
	UINT32 t0;

	t2 = 0;
	if( length >= 0 )
	{
		do
		{
			t1 = source[ t2 ];
			a1 = 7;
			t0 = t1;

			do
			{
				v1 = key[ a1 ];
				a1--;
				v0 = v1 & 0x1f;
				v0 = t0 - v0;
				v1 >>= 5;
				v0 &= 0xff;
				a0 = (signed)v0 >> v1;
				v1 = 8 - v1;
				v1 &= 7;
				v0 = (signed)v0 << v1;
				t0 = a0 | v0;
			} while( a1 > 0 );

			v1 = key[ 0 ];
			a0 = previous_byte;
			v0 = t0 & 0xff;
			previous_byte = t1;
			v0 = v0 - v1;
			v0 = v0 ^ a0;

			destination[ t2 ] = v0;
			t2++;

		} while( t2 < length );
	}
}

static void zs01_encrypt( UINT8 *destination, UINT8 *source, int length, UINT8 *key, UINT32 previous_byte )
{
	UINT32 t0;
	UINT32 v0;
	UINT32 v1;
	UINT32 a0;
	UINT32 a1;

	length--;
	if( length >= 0 )
	{
		do
		{
			t0 = 1;
			v0 = source[ length ];
			v1 = previous_byte;
			a0 = key[ 0 ];
			v0 ^= v1;
			a0 += v0;

			do
			{
				a1 = key[ t0 ];
				t0++;
				a0 &= 0xff;
				v0 = a1 >> 5;
				v1 = a0 << v0;
				v0 = 8 - v0;
				v0 &= 7;
				a0 = (signed) a0 >> v0;
				v1 |= a0;
				v1 &= 0xff;
				a1 &= 0x1f;
				v1 += a1;
				v0 = (signed) t0 < 8;
				a0 = v1;

			} while( v0 != 0 );

			previous_byte = v1;

			destination[ length ] = a0;
			length--;

		} while( length >= 0 );
	}
}

static UINT16 zs01_crc( UINT8 *buffer, UINT32 length )
{
	UINT32 v1;
	UINT32 a3;
	UINT32 v0;
	UINT32 a2;

	v1 = 0xffff;
	a3 = 0;

	if( length > 0 )
	{
		do
		{
			v0 = buffer[ a3 ];
			a2 = 7;
			v0 = v0 << 8;
			v1 = v1 ^ v0;
			v0 = v1 & 0x8000;

			do
			{
				if( v0 != 0 )
				{
					v0 = v1 << 1;
					v1 = v0 ^ 0x1021;
				}
				else
				{
					v0 = v1 << 1;
					v1 = v1 << 1;
				}

				a2--;
				v0 = v1 & 0x8000;
			} while( (signed) a2 >= 0 );

			a3++;
			v0 = (signed) a3 < (signed) length;
		} while ( v0 != 0 );
	}

	v0 = ~v1 ;
	v0 = v0 & 0xffff;

	return v0;
}

static int zs01_data_offset( struct zs01_chip *c )
{
	int block = ( ( c->write_buffer[ 0 ] & 2 ) << 7 ) | c->write_buffer[ 1 ];

	return block * SIZE_DATA_BUFFER;
}

void zs01_scl_write( int chip, int scl )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_scl_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( c->scl != scl )
	{
		verboselog( 2, "zs01(%d) scl=%d\n", chip, scl );
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
					verboselog( 1, "zs01(%d) <- response_to_reset[%d]: %02x\n", chip, c->byte, c->shift );
				}

				c->sdar = ( c->shift >> 7 ) & 1;
				c->shift <<= 1;
				c->bit++;

				if( c->bit == 8 )
				{
					c->bit = 0;
					c->byte++;
					if( c->byte == 4 )
					{
						c->sdar = 1;
						verboselog( 1, "zs01(%d) goto stop\n", chip );
						c->state = STATE_STOP;
					}
				}
			}
			break;

		case STATE_LOAD_COMMAND:
			if( c->scl == 0 && scl != 0 )
			{
				if( c->bit < 8 )
				{
					verboselog( 2, "zs01(%d) clock\n", chip );
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
						c->write_buffer[ c->byte ] = c->shift;
						verboselog( 2, "zs01(%d) -> write_buffer[%d]: %02x\n", chip, c->byte, c->write_buffer[ c->byte ] );

						c->byte++;
						if( c->byte == SIZE_WRITE_BUFFER )
						{
							UINT16 crc;

							zs01_decrypt( c->write_buffer, c->write_buffer, SIZE_WRITE_BUFFER, c->command_key, 0xff );

							if( ( c->write_buffer[ 0 ] & 4 ) != 0 )
							{
								zs01_decrypt2( &c->write_buffer[ 2 ], &c->write_buffer[ 2 ], SIZE_DATA_BUFFER, c->data_key, 0x00 );
							}

							crc = zs01_crc( c->write_buffer, 10 );

							if( crc == ( ( c->write_buffer[ 10 ] << 8 ) | c->write_buffer[ 11 ] ) )
							{
								verboselog( 1, "zs01(%d) -> command: %02x\n", chip, c->write_buffer[ 0 ] );
								verboselog( 1, "zs01(%d) -> address: %02x\n", chip, c->write_buffer[ 1 ] );
								verboselog( 1, "zs01(%d) -> data: %02x%02x%02x%02x%02x%02x%02x%02x\n", chip,
									c->write_buffer[ 2 ], c->write_buffer[ 3 ], c->write_buffer[ 4 ], c->write_buffer[ 5 ],
									c->write_buffer[ 6 ], c->write_buffer[ 7 ], c->write_buffer[ 8 ], c->write_buffer[ 9 ] );
								verboselog( 1, "zs01(%d) -> crc: %02x%02x\n", chip, c->write_buffer[ 10 ], c->write_buffer[ 11 ] );

								switch( c->write_buffer[ 0 ] & 1 )
								{
								case COMMAND_WRITE:
									memcpy( &c->data[ zs01_data_offset( c ) ], &c->write_buffer[ 2 ], SIZE_DATA_BUFFER );

									/* todo: find out what should be returned. */
									memset( &c->read_buffer[ 0 ], 0, SIZE_WRITE_BUFFER );
									break;

								case COMMAND_READ:
									/* todo: find out what should be returned. */
									memset( &c->read_buffer[ 0 ], 0, 2 );

									switch( c->write_buffer[ 1 ] )
									{
									case 0xfd:
										{
											/* TODO: use read/write to talk to the ds2401, which will require a timer. */
											int i;
											for( i = 0; i < SIZE_DATA_BUFFER; i++ )
											{
												c->read_buffer[ 2 + i ] = c->ds2401[ SIZE_DATA_BUFFER - i - 1 ];
											}
										}
										break;
									default:
										memcpy( &c->read_buffer[ 2 ], &c->data[ zs01_data_offset( c ) ], SIZE_DATA_BUFFER );
										break;
									}

									memcpy( c->response_key, &c->write_buffer[ 2 ], SIZE_KEY );
									break;
								}
							}
							else
							{
								verboselog( 0, "zs01(%d) bad crc\n", chip );

								/* todo: find out what should be returned. */
								memset( &c->read_buffer[ 0 ], 0xff, 2 );
							}

							verboselog( 1, "zs01(%d) <- status: %02x%02\n", chip,
								c->read_buffer[ 0 ], c->read_buffer[ 1 ] );

							verboselog( 1, "zs01(%d) <- data: %02x%02x%02x%02x%02x%02x%02x%02x\n", chip,
								c->read_buffer[ 2 ], c->read_buffer[ 3 ], c->read_buffer[ 4 ], c->read_buffer[ 5 ],
								c->read_buffer[ 6 ], c->read_buffer[ 7 ], c->read_buffer[ 8 ], c->read_buffer[ 9 ] );

							crc = zs01_crc( c->read_buffer, 10 );
							c->read_buffer[ 10 ] = crc >> 8;
							c->read_buffer[ 11 ] = crc & 255;

							zs01_encrypt( c->read_buffer, c->read_buffer, SIZE_READ_BUFFER, c->response_key, 0xff );

							c->byte = 0;
							c->state = STATE_READ_DATA;
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
							c->shift = c->read_buffer[ c->byte ];
							verboselog( 2, "zs01(%d) <- read_buffer[%d]: %02x\n", chip, c->byte, c->shift );
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
						verboselog( 2, "zs01(%d) ack <-\n", chip );
						c->byte++;
						if( c->byte == SIZE_READ_BUFFER )
						{
							c->byte = 0;
							c->sdar = 1;
							c->state = STATE_LOAD_COMMAND;
						}
					}
					else
					{
						verboselog( 2, "zs01(%d) nak <-\n", chip );
					}
				}
			}
			break;
		}
	}
	c->scl = scl;
}

void zs01_sda_write( int chip, int sda )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_sda_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( c->sdaw != sda )
	{
		verboselog( 2, "zs01(%d) sdaw=%d\n", chip, sda );
	}
	if( c->cs == 0 && c->scl != 0 )
	{
//      if( c->sdaw == 0 && sda != 0 )
//      {
//          verboselog( 1, "zs01(%d) goto stop\n", chip );
//          c->state = STATE_STOP;
//          c->sdar = 0;
//      }
		if( c->sdaw != 0 && sda == 0 )
		{
			switch( c->state )
			{
			case STATE_STOP:
				verboselog( 1, "zs01(%d) goto start\n", chip );
				c->state = STATE_LOAD_COMMAND;
				break;

//          default:
//              verboselog( 1, "zs01(%d) skipped start (default)\n", chip );
//              break;
			}

			c->bit = 0;
			c->byte = 0;
			c->shift = 0;
			c->sdar = 0;
		}
	}
	c->sdaw = sda;
}

int zs01_sda_read( int chip )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "zs01_sda_read( %d ) chip out of range\n", chip );
		return 1;
	}

	c = &zs01[ chip ];
	if( c->cs != 0 )
	{
		verboselog( 2, "zs01(%d) not selected\n", chip );
		return 1;
	}
	verboselog( 2, "zs01(%d) sdar=%d\n", chip, c->sdar );

	return c->sdar;
}

static void nvram_handler_zs01( int chip, running_machine *machine, mame_file *file, int read_or_write )
{
	struct zs01_chip *c;

	if( chip >= ZS01_MAXCHIP )
	{
		verboselog( 0, "nvram_handler_zs01( %d ) chip out of range\n", chip );
		return;
	}

	c = &zs01[ chip ];

	if( read_or_write )
	{
		mame_fwrite( file, c->data, SIZE_DATA );
	}
	else if( file )
	{
		mame_fread( file, c->data, SIZE_DATA );
	}
}

NVRAM_HANDLER( zs01_0 ) { nvram_handler_zs01( 0, machine, file, read_or_write ); }
NVRAM_HANDLER( zs01_1 ) { nvram_handler_zs01( 1, machine, file, read_or_write ); }
