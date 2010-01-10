/*
 * x76f041.c
 *
 * Secure SerialFlash
 *
 * The X76F041 is a Password Access Security Supervisor, containing four 128 x 8 bit SecureFlash arrays.
 * Access can be controlled by three 64-bit programmable passwords, one for read operations, one for write
 * operations and one for device configuration.
 *
 * The data sheet has an incorrect diagrams for sequential read with password, there shouldn't be an extra address after the 0xc0 command.
 *
 */

#include "emu.h"
#include "machine/x76f041.h"

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

#define SIZE_WRITE_BUFFER ( 8 )

struct x76f041_chip
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
	int address;
	UINT8 write_buffer[ SIZE_WRITE_BUFFER ];
	UINT8 *response_to_reset;
	UINT8 *write_password;
	UINT8 *read_password;
	UINT8 *configuration_password;
	UINT8 *configuration_registers;
	UINT8 *data;
};

#define SIZE_RESPONSE_TO_RESET ( 4 )
#define SIZE_WRITE_PASSWORD ( SIZE_WRITE_BUFFER )
#define SIZE_READ_PASSWORD ( SIZE_WRITE_BUFFER )
#define SIZE_CONFIGURATION_PASSWORD ( SIZE_WRITE_BUFFER )
#define SIZE_CONFIGURATION_REGISTERS ( SIZE_WRITE_BUFFER )
#define SIZE_DATA ( 512 )

#define CONFIG_BCR1 ( 0 )
#define CONFIG_BCR2 ( 1 )
#define CONFIG_CR ( 2 )
#define CONFIG_RR ( 3 )
#define CONFIG_RC ( 4 )

#define BCR_X ( 8 )
#define BCR_Y ( 4 )
#define BCR_Z ( 2 )
#define BCR_T ( 1 )

static struct x76f041_chip x76f041[ X76F041_MAXCHIP ];

#define COMMAND_WRITE ( 0x00 )
#define COMMAND_READ ( 0x20 )
#define COMMAND_WRITE_USE_CONFIGURATION_PASSWORD ( 0x40 )
#define COMMAND_READ_USE_CONFIGURATION_PASSWORD ( 0x60 )
#define COMMAND_CONFIGURATION ( 0x80 )

#define CONFIGURATION_PROGRAM_WRITE_PASSWORD ( 0x00 )
#define CONFIGURATION_PROGRAM_READ_PASSWORD ( 0x10 )
#define CONFIGURATION_PROGRAM_CONFIGURATION_PASSWORD ( 0x20 )
#define CONFIGURATION_RESET_WRITE_PASSWORD ( 0x30 )
#define CONFIGURATION_RESET_READ_PASSWORD ( 0x40 )
#define CONFIGURATION_PROGRAM_CONFIGURATION_REGISTERS ( 0x50 )
#define CONFIGURATION_READ_CONFIGURATION_REGISTERS ( 0x60 )
#define CONFIGURATION_MASS_PROGRAM ( 0x70 )
#define CONFIGURATION_MASS_ERASE ( 0x80 )

#define STATE_STOP ( 0 )
#define STATE_RESPONSE_TO_RESET ( 1 )
#define STATE_LOAD_COMMAND ( 2 )
#define STATE_LOAD_ADDRESS ( 3 )
#define STATE_LOAD_PASSWORD ( 4 )
#define STATE_VERIFY_PASSWORD ( 5 )
#define STATE_READ_DATA ( 6 )
#define STATE_WRITE_DATA ( 7 )
#define STATE_READ_CONFIGURATION_REGISTERS ( 8 )
#define STATE_WRITE_CONFIGURATION_REGISTERS ( 9 )

void x76f041_init( running_machine *machine, int chip, UINT8 *data )
{
	int offset;
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_init( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( data == NULL )
	{
		data = auto_alloc_array( machine, UINT8,
			SIZE_RESPONSE_TO_RESET +
			SIZE_READ_PASSWORD +
			SIZE_WRITE_PASSWORD +
			SIZE_CONFIGURATION_PASSWORD +
			SIZE_CONFIGURATION_REGISTERS +
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
	c->address = 0;
	memset( c->write_buffer, 0, SIZE_WRITE_BUFFER );

	offset = 0;
	c->response_to_reset = &data[ offset ]; offset += SIZE_RESPONSE_TO_RESET;
	c->write_password = &data[ offset ]; offset += SIZE_WRITE_PASSWORD;
	c->read_password = &data[ offset ]; offset += SIZE_READ_PASSWORD;
	c->configuration_password = &data[ offset ]; offset += SIZE_CONFIGURATION_PASSWORD;
	c->configuration_registers = &data[ offset ]; offset += SIZE_CONFIGURATION_REGISTERS;
	c->data = &data[ offset ]; offset += SIZE_DATA;

	state_save_register_item( machine, "x76f041", NULL, chip, c->cs );
	state_save_register_item( machine, "x76f041", NULL, chip, c->rst );
	state_save_register_item( machine, "x76f041", NULL, chip, c->scl );
	state_save_register_item( machine, "x76f041", NULL, chip, c->sdaw );
	state_save_register_item( machine, "x76f041", NULL, chip, c->sdar );
	state_save_register_item( machine, "x76f041", NULL, chip, c->state );
	state_save_register_item( machine, "x76f041", NULL, chip, c->shift );
	state_save_register_item( machine, "x76f041", NULL, chip, c->bit );
	state_save_register_item( machine, "x76f041", NULL, chip, c->byte );
	state_save_register_item( machine, "x76f041", NULL, chip, c->command );
	state_save_register_item( machine, "x76f041", NULL, chip, c->address );
	state_save_register_item_array( machine, "x76f041", NULL, chip, c->write_buffer );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->response_to_reset, SIZE_RESPONSE_TO_RESET );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->write_password, SIZE_WRITE_PASSWORD );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->read_password, SIZE_READ_PASSWORD );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->configuration_password, SIZE_CONFIGURATION_PASSWORD );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->configuration_registers, SIZE_CONFIGURATION_REGISTERS );
	state_save_register_item_pointer( machine, "x76f041", NULL, chip, c->data, SIZE_DATA );
}

void x76f041_cs_write( running_machine *machine, int chip, int cs )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_cs_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( c->cs != cs )
	{
		verboselog( machine, 2, "x76f041(%d) cs=%d\n", chip, cs );
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

void x76f041_rst_write( running_machine *machine, int chip, int rst )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_rst_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( c->rst != rst )
	{
		verboselog( machine, 2, "x76f041(%d) rst=%d\n", chip, rst );
	}
	if( c->rst == 0 && rst != 0 && c->cs == 0 )
	{
		verboselog( machine, 1, "x76f041(%d) goto response to reset\n", chip );
		c->state = STATE_RESPONSE_TO_RESET;
		c->bit = 0;
		c->byte = 0;
	}
	c->rst = rst;
}

static UINT8 *x76f041_password( struct x76f041_chip *c )
{
	switch( c->command & 0xe0 )
	{
	case COMMAND_WRITE:
		return c->write_password;
	case COMMAND_READ:
		return c->read_password;
	default:
		return c->configuration_password;
	}
}

static void x76f041_password_ok( struct x76f041_chip *c )
{
	switch( c->command & 0xe0 )
	{
	case COMMAND_WRITE:
		c->state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ:
		c->state = STATE_READ_DATA;
		break;
	case COMMAND_WRITE_USE_CONFIGURATION_PASSWORD:
		c->state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ_USE_CONFIGURATION_PASSWORD:
		c->state = STATE_READ_DATA;
		break;
	case COMMAND_CONFIGURATION:
		switch( c->address )
		{
		case CONFIGURATION_PROGRAM_WRITE_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_READ_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_CONFIGURATION_PASSWORD:
			break;
		case CONFIGURATION_RESET_WRITE_PASSWORD:
			break;
		case CONFIGURATION_RESET_READ_PASSWORD:
			break;
		case CONFIGURATION_PROGRAM_CONFIGURATION_REGISTERS:
			c->state = STATE_WRITE_CONFIGURATION_REGISTERS;
			c->byte = 0;
			break;
		case CONFIGURATION_READ_CONFIGURATION_REGISTERS:
			c->state = STATE_READ_CONFIGURATION_REGISTERS;
			c->byte = 0;
			break;
		case CONFIGURATION_MASS_PROGRAM:
			break;
		case CONFIGURATION_MASS_ERASE:
			break;
		default:
			break;
		}
	}
}

static void x76f041_load_address( running_machine *machine, int chip )
{
	/* todo: handle other bcr bits */
	struct x76f041_chip *c = &x76f041[ chip ];
	int bcr;

	c->address = c->shift;

	verboselog( machine, 1, "x76f041(%d) -> address: %02x\n", chip, c->address );

	if( ( c->command & 1 ) == 0 )
	{
		bcr = c->configuration_registers[ CONFIG_BCR1 ];
	}
	else
	{
		bcr = c->configuration_registers[ CONFIG_BCR2 ];
	}
	if( ( c->address & 0x80 ) != 0 )
	{
		bcr >>= 4;
	}

	if( ( ( c->command & 0xe0 ) == COMMAND_READ && ( bcr & BCR_Z ) != 0 && ( bcr & BCR_T ) != 0 ) ||
		( ( c->command & 0xe0 ) == COMMAND_WRITE && ( bcr & BCR_Z ) != 0 ) )
	{
		/* todo: find out when this is really checked. */
		verboselog( machine, 1, "x76f041(%d) command not allowed\n", chip );
		c->state = STATE_STOP;
		c->sdar = 0;
	}
	else if( ( ( c->command & 0xe0 ) == COMMAND_WRITE && ( bcr & BCR_X ) == 0 ) ||
		( ( c->command & 0xe0 ) == COMMAND_READ && ( bcr & BCR_Y ) == 0 ) )
	{
		verboselog( machine, 1, "x76f041(%d) password not required\n", chip );
		x76f041_password_ok( c );
	}
	else
	{
		verboselog( machine, 1, "x76f041(%d) send password\n", chip );
		c->state = STATE_LOAD_PASSWORD;
		c->byte = 0;
	}
}

static int x76f041_data_offset( struct x76f041_chip *c )
{
	int block_offset = ( ( c->command & 1 ) << 8 ) + c->address;

	// TODO: confirm block_start doesn't wrap.

	return ( block_offset & 0x180 ) | ( ( block_offset + c->byte ) & 0x7f );
}

void x76f041_scl_write( running_machine *machine, int chip, int scl )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_scl_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( c->scl != scl )
	{
		verboselog( machine, 2, "x76f041(%d) scl=%d\n", chip, scl );
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
				c->sdar = ( c->response_to_reset[ c->byte ] >> c->bit ) & 1;
				verboselog( machine, 2, "x76f041(%d) in response to reset %d (%d/%d)\n", chip, c->sdar, c->byte, c->bit );
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
		case STATE_LOAD_ADDRESS:
		case STATE_LOAD_PASSWORD:
		case STATE_VERIFY_PASSWORD:
		case STATE_WRITE_DATA:
		case STATE_WRITE_CONFIGURATION_REGISTERS:
			if( c->scl == 0 && scl != 0 )
			{
				if( c->bit < 8 )
				{
					verboselog( machine, 2, "x76f041(%d) clock\n", chip );
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
						verboselog( machine, 1, "x76f041(%d) -> command: %02x\n", chip, c->command );
						/* todo: verify command is valid? */
						c->state = STATE_LOAD_ADDRESS;
						break;
					case STATE_LOAD_ADDRESS:
						x76f041_load_address( machine, chip );
						break;
					case STATE_LOAD_PASSWORD:
						verboselog( machine, 1, "x76f041(%d) -> password: %02x\n", chip, c->shift );
						c->write_buffer[ c->byte++ ] = c->shift;
						if( c->byte == SIZE_WRITE_BUFFER )
						{
							c->state = STATE_VERIFY_PASSWORD;
						}
						break;
					case STATE_VERIFY_PASSWORD:
						verboselog( machine, 1, "x76f041(%d) -> verify password: %02x\n", chip, c->shift );
						/* todo: this should probably be handled as a command */
						if( c->shift == 0xc0 )
						{
							/* todo: this should take 10ms before it returns ok. */
							if( memcmp( x76f041_password( c ), c->write_buffer, SIZE_WRITE_BUFFER ) == 0 )
							{
								x76f041_password_ok( c );
							}
							else
							{
								c->sdar = 1;
							}
						}
						break;
					case STATE_WRITE_DATA:
						verboselog( machine, 1, "x76f041(%d) -> data: %02x\n", chip, c->shift );
						c->write_buffer[ c->byte++ ] = c->shift;
						if( c->byte == SIZE_WRITE_BUFFER )
						{
							for( c->byte = 0; c->byte < SIZE_WRITE_BUFFER; c->byte++ )
							{
								c->data[ x76f041_data_offset( c ) ] = c->write_buffer[ c->byte ];
							}
							c->byte = 0;

							verboselog( machine, 1, "x76f041(%d) data flushed\n", chip );
						}
						break;
					case STATE_WRITE_CONFIGURATION_REGISTERS:
						verboselog( machine, 1, "x76f041(%d) -> configuration register: %02x\n", chip, c->shift );
						/* todo: write after all bytes received? */
						c->configuration_registers[ c->byte++ ] = c->shift;
						if( c->byte == SIZE_CONFIGURATION_REGISTERS )
						{
							c->byte = 0;
						}
						break;
					}

					c->bit = 0;
					c->shift = 0;
				}
			}
			break;
		case STATE_READ_DATA:
		case STATE_READ_CONFIGURATION_REGISTERS:
			if( c->scl == 0 && scl != 0 )
			{
				if( c->bit < 8 )
				{
					if( c->bit == 0 )
					{
						switch( c->state )
						{
						case STATE_READ_DATA:
							c->shift = c->data[ x76f041_data_offset( c ) ];
							verboselog( machine, 1, "x76f041(%d) <- data: %02x\n", chip, c->shift );
							break;
						case STATE_READ_CONFIGURATION_REGISTERS:
							c->shift = c->configuration_registers[ c->byte & 7 ];
							verboselog( machine, 1, "x76f041(%d) <- configuration register: %02x\n", chip, c->shift );
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
						verboselog( machine, 2, "x76f041(%d) ack <-\n", chip );
						c->byte++;
					}
					else
					{
						verboselog( machine, 2, "x76f041(%d) nak <-\n", chip );
					}
				}
			}
			break;
		}
	}
	c->scl = scl;
}

void x76f041_sda_write( running_machine *machine, int chip, int sda )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_sda_write( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( c->sdaw != sda )
	{
		verboselog( machine, 2, "x76f041(%d) sdaw=%d\n", chip, sda );
	}
	if( c->cs == 0 && c->scl != 0 )
	{
		if( c->sdaw == 0 && sda != 0 )
		{
			verboselog( machine, 1, "x76f041(%d) goto stop\n", chip );
			c->state = STATE_STOP;
			c->sdar = 0;
		}
		if( c->sdaw != 0 && sda == 0 )
		{
			switch( c->state )
			{
			case STATE_STOP:
				verboselog( machine, 1, "x76f041(%d) goto start\n", chip );
				c->state = STATE_LOAD_COMMAND;
				break;
			case STATE_LOAD_PASSWORD:
				/* todo: this will be the 0xc0 command, but it's not handled as a command yet. */
				verboselog( machine, 1, "x76f041(%d) goto start\n", chip );
				break;
			case STATE_READ_DATA:
				verboselog( machine, 1, "x76f041(%d) goto load address\n", chip );
				c->state = STATE_LOAD_ADDRESS;
				break;
			default:
				verboselog( machine, 1, "x76f041(%d) skipped start (default)\n", chip );
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

int x76f041_sda_read( running_machine *machine, int chip )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "x76f041_sda_read( %d ) chip out of range\n", chip );
		return 1;
	}

	c = &x76f041[ chip ];

	if( c->cs != 0 )
	{
		verboselog( machine, 2, "x76f041(%d) not selected\n", chip );
		return 1;
	}
	verboselog( machine, 2, "x76f041(%d) sdar=%d\n", chip, c->sdar );
	return c->sdar;
}

static void nvram_handler_x76f041( running_machine *machine, mame_file *file, int read_or_write, int chip )
{
	struct x76f041_chip *c;

	if( chip >= X76F041_MAXCHIP )
	{
		verboselog( machine, 0, "nvram_handler_x76f041( %d ) chip out of range\n", chip );
		return;
	}

	c = &x76f041[ chip ];

	if( read_or_write )
	{
		mame_fwrite( file, c->write_password, SIZE_WRITE_PASSWORD );
		mame_fwrite( file, c->read_password, SIZE_READ_PASSWORD );
		mame_fwrite( file, c->configuration_password, SIZE_CONFIGURATION_PASSWORD );
		mame_fwrite( file, c->configuration_registers, SIZE_CONFIGURATION_REGISTERS );
		mame_fwrite( file, c->data, SIZE_DATA );
	}
	else if( file )
	{
		mame_fread( file, c->write_password, SIZE_WRITE_PASSWORD );
		mame_fread( file, c->read_password, SIZE_READ_PASSWORD );
		mame_fread( file, c->configuration_password, SIZE_CONFIGURATION_PASSWORD );
		mame_fread( file, c->configuration_registers, SIZE_CONFIGURATION_REGISTERS );
		mame_fread( file, c->data, SIZE_DATA );
	}
}

NVRAM_HANDLER( x76f041_0 ) { nvram_handler_x76f041( machine, file, read_or_write, 0 ); }
NVRAM_HANDLER( x76f041_1 ) { nvram_handler_x76f041( machine, file, read_or_write, 1 ); }
