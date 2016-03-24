// license:BSD-3-Clause
// copyright-holders:smf
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

#include "emu.h"
#include "machine/x76f100.h"

#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF( 3, 4 ) x76f100_device::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: x76f100(%s) %s", machine().describe_context(), tag(), buf );
	}
}

// device type definition
const device_type X76F100 = &device_creator<x76f100_device>;

x76f100_device::x76f100_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t( mconfig, X76F100, "X76F100 Flash", tag, owner, clock, "x76f100", __FILE__ ),
	device_nvram_interface(mconfig, *this),
	m_cs( 0 ),
	m_rst( 0 ),
	m_scl( 0 ),
	m_sdaw( 0 ),
	m_sdar( 0 ),
	m_state( STATE_STOP ),
	m_shift( 0 ),
	m_bit( 0 ),
	m_byte( 0 ),
	m_command( 0 )
{
}

void x76f100_device::device_start()
{
	memset( m_write_buffer, 0, sizeof( m_write_buffer ) );

	save_item( NAME( m_cs ) );
	save_item( NAME( m_rst ) );
	save_item( NAME( m_scl ) );
	save_item( NAME( m_sdaw ) );
	save_item( NAME( m_sdar ) );
	save_item( NAME( m_state ) );
	save_item( NAME( m_shift ) );
	save_item( NAME( m_bit ) );
	save_item( NAME( m_byte ) );
	save_item( NAME( m_command ) );
	save_item( NAME( m_write_buffer ) );
	save_item( NAME( m_response_to_reset ) );
	save_item( NAME( m_write_password ) );
	save_item( NAME( m_read_password ) );
	save_item( NAME( m_data ) );
}

WRITE_LINE_MEMBER( x76f100_device::write_cs )
{
	if( m_cs != state )
	{
		verboselog( 2, "cs=%d\n", state );
	}

	if( m_cs != 0 && state == 0 )
	{
		/* enable chip */
		m_state = STATE_STOP;
	}

	if( m_cs == 0 && state != 0 )
	{
		/* disable chip */
		m_state = STATE_STOP;
		/* high impendence? */
		m_sdar = 0;
	}

	m_cs = state;
}

WRITE_LINE_MEMBER( x76f100_device::write_rst )
{
	if( m_rst != state )
	{
		verboselog( 2, "rst=%d\n", state );
	}

	if( m_rst == 0 && state != 0 && m_cs == 0 )
	{
		verboselog( 1, "goto response to reset\n" );
		m_state = STATE_RESPONSE_TO_RESET;
		m_bit = 0;
		m_byte = 0;
	}

	m_rst = state;
}

UINT8 *x76f100_device::password()
{
	if( ( m_command & 0xe1 ) == COMMAND_READ )
	{
		return m_read_password;
	}

	return m_write_password;
}

void x76f100_device::password_ok()
{
	if( ( m_command & 0xe1 ) == COMMAND_READ )
	{
		m_state = STATE_READ_DATA;
	}
	else if( ( m_command & 0xe1 ) == COMMAND_WRITE )
	{
		m_state = STATE_WRITE_DATA;
	}
	else
	{
		/* TODO: */
	}
}

int x76f100_device::data_offset()
{
	int block_offset = ( m_command >> 1 ) & 0x0f;

	return ( block_offset * sizeof( m_write_buffer ) ) + m_byte;
}

WRITE_LINE_MEMBER( x76f100_device::write_scl )
{
	if( m_scl != state )
	{
		verboselog( 2, "scl=%d\n", state );
	}

	if( m_cs == 0 )
	{
		switch( m_state )
		{
		case STATE_STOP:
			break;

		case STATE_RESPONSE_TO_RESET:
			if( m_scl != 0 && state == 0 )
			{
				if( m_bit == 0 )
				{
					m_shift = m_response_to_reset[ m_byte ];
					verboselog( 1, "<- response_to_reset[%d]: %02x\n", m_byte, m_shift );
				}

				m_sdar = m_shift & 1;
				m_shift >>= 1;
				m_bit++;

				if( m_bit == 8 )
				{
					m_bit = 0;
					m_byte++;

					if( m_byte == sizeof( m_response_to_reset ) )
					{
						m_byte = 0;
					}
				}
			}
			break;

		case STATE_LOAD_COMMAND:
		case STATE_LOAD_PASSWORD:
		case STATE_VERIFY_PASSWORD:
		case STATE_WRITE_DATA:
			if( m_scl == 0 && state != 0 )
			{
				if( m_bit < 8 )
				{
					verboselog( 2, "clock\n" );
					m_shift <<= 1;

					if( m_sdaw != 0 )
					{
						m_shift |= 1;
					}

					m_bit++;
				}
				else
				{
					m_sdar = 0;

					switch( m_state )
					{
					case STATE_LOAD_COMMAND:
						m_command = m_shift;
						verboselog( 1, "-> command: %02x\n", m_command );
						/* todo: verify command is valid? */
						m_state = STATE_LOAD_PASSWORD;
						break;

					case STATE_LOAD_PASSWORD:
						verboselog( 1, "-> password: %02x\n", m_shift );
						m_write_buffer[ m_byte++ ] = m_shift;

						if( m_byte == sizeof( m_write_buffer ) )
						{
							m_state = STATE_VERIFY_PASSWORD;
						}
						break;

					case STATE_VERIFY_PASSWORD:
						verboselog( 1, "-> verify password: %02x\n", m_shift );

						/* todo: this should probably be handled as a command */
						if( m_shift == COMMAND_ACK_PASSWORD )
						{
							/* todo: this should take 10ms before it returns ok. */
							if( memcmp( password(), m_write_buffer, sizeof( m_write_buffer ) ) == 0 )
							{
								password_ok();
							}
							else
							{
								m_sdar = 1;
							}
						}
						break;

					case STATE_WRITE_DATA:
						verboselog( 2, "-> data: %02x\n", m_shift );
						m_write_buffer[ m_byte++ ] = m_shift;

						if( m_byte == sizeof( m_write_buffer ) )
						{
							for( m_byte = 0; m_byte < sizeof( m_write_buffer ); m_byte++ )
							{
								int offset = data_offset();
								verboselog( 1, "-> data[ %03x ]: %02x\n", offset, m_write_buffer[ m_byte ] );
								m_data[ offset ] = m_write_buffer[ m_byte ];
							}

							m_byte = 0;

							verboselog( 1, "data flushed\n" );
						}
						break;
					}

					m_bit = 0;
					m_shift = 0;
				}
			}
			break;

		case STATE_READ_DATA:
			if( m_scl == 0 && state != 0 )
			{
				if( m_bit < 8 )
				{
					if( m_bit == 0 )
					{
						int offset;

						switch( m_state )
						{
						case STATE_READ_DATA:
							offset = data_offset();
							m_shift = m_data[ offset ];
							verboselog( 1, "<- data[ %02x ]: %02x\n", offset, m_shift );
							break;
						}
					}

					m_sdar = ( m_shift >> 7 ) & 1;
					m_shift <<= 1;
					m_bit++;
				}
				else
				{
					m_bit = 0;
					m_sdar = 0;

					if( m_sdaw == 0 )
					{
						verboselog( 2, "ack <-\n" );
						m_byte++;
					}
					else
					{
						verboselog( 2, "nak <-\n" );
					}
				}
			}
			break;
		}
	}

	m_scl = state;
}

WRITE_LINE_MEMBER( x76f100_device::write_sda )
{
	if( m_sdaw != state )
	{
		verboselog( 2, "sdaw=%d\n", state );
	}

	if( m_cs == 0 && m_scl != 0 )
	{
		if( m_sdaw == 0 && state != 0 )
		{
			verboselog( 1, "goto stop\n" );
			m_state = STATE_STOP;
			m_sdar = 0;
		}

		if( m_sdaw != 0 && state == 0 )
		{
			switch( m_state )
			{
			case STATE_STOP:
				verboselog( 1, "goto start\n" );
				m_state = STATE_LOAD_COMMAND;
				break;

			case STATE_LOAD_PASSWORD:
				/* todo: this will be the 0xc0 command, but it's not handled as a command yet. */
				verboselog( 1, "goto start\n" );
				break;

			case STATE_READ_DATA:
				verboselog( 1, "continue reading??\n" );
//              verboselog( 1, "goto load address\n" );
//              m_state = STATE_LOAD_ADDRESS;
				break;

			default:
				verboselog( 1, "skipped start (default)\n" );
				break;
			}

			m_bit = 0;
			m_byte = 0;
			m_shift = 0;
			m_sdar = 0;
		}
	}

	m_sdaw = state;
}

READ_LINE_MEMBER( x76f100_device::read_sda )
{
	if( m_cs != 0 )
	{
		verboselog( 2, "not selected\n" );
		return 1;
	}

	verboselog( 2, "sdar=%d\n", m_sdar );
	return m_sdar;
}

void x76f100_device::nvram_default()
{
	m_response_to_reset[ 0 ] = 0x19;
	m_response_to_reset[ 1 ] = 0x00;
	m_response_to_reset[ 2 ] = 0xaa;
	m_response_to_reset[ 3 ] = 0x55,

	memset( m_write_password, 0, sizeof( m_write_password ) );
	memset( m_read_password, 0, sizeof( m_read_password ) );
	memset( m_data, 0, sizeof( m_data ) );

	int expected_size = sizeof( m_response_to_reset ) + sizeof( m_write_password ) + sizeof( m_read_password ) + sizeof( m_data );

	if( !m_region )
	{
		logerror( "x76f100(%s) region not found\n", tag() );
	}
	else if( m_region->bytes() != expected_size )
	{
		logerror("x76f100(%s) region length 0x%x expected 0x%x\n", tag(), m_region->bytes(), expected_size );
	}
	else
	{
		UINT8 *region = m_region->base();

		memcpy( m_response_to_reset, region, sizeof( m_response_to_reset )); region += sizeof( m_response_to_reset );
		memcpy( m_write_password, region, sizeof( m_write_password )); region += sizeof( m_write_password );
		memcpy( m_read_password, region, sizeof( m_read_password )); region += sizeof( m_read_password );
		memcpy( m_data, region, sizeof( m_data )); region += sizeof( m_data );
	}
}

void x76f100_device::nvram_read( emu_file &file )
{
	file.read( m_response_to_reset, sizeof( m_response_to_reset ) );
	file.read( m_write_password, sizeof( m_write_password ) );
	file.read( m_read_password, sizeof( m_read_password ) );
	file.read( m_data, sizeof( m_data ) );
}

void x76f100_device::nvram_write( emu_file &file )
{
	file.write( m_response_to_reset, sizeof( m_response_to_reset ) );
	file.write( m_write_password, sizeof( m_write_password ) );
	file.write( m_read_password, sizeof( m_read_password ) );
	file.write( m_data, sizeof( m_data ) );
}
