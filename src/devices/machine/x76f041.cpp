// license:BSD-3-Clause
// copyright-holders:smf
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

inline void ATTR_PRINTF( 3, 4 ) x76f041_device::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: x76f041(%s) %s", machine().describe_context(), tag().c_str(), buf );
	}
}

// device type definition
const device_type X76F041 = &device_creator<x76f041_device>;

x76f041_device::x76f041_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock )
	: device_t( mconfig, X76F041, "X76F041 Flash", tag, owner, clock, "x76f041", __FILE__ ),
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
	m_command( 0 ),
	m_address( 0 )
{
}

void x76f041_device::device_start()
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
	save_item( NAME( m_address ) );
	save_item( NAME( m_write_buffer ) );
	save_item( NAME( m_response_to_reset ) );
	save_item( NAME( m_write_password ) );
	save_item( NAME( m_read_password ) );
	save_item( NAME( m_configuration_password ) );
	save_item( NAME( m_configuration_registers ) );
	save_item( NAME( m_data ) );
}

WRITE_LINE_MEMBER( x76f041_device::write_cs )
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

WRITE_LINE_MEMBER( x76f041_device::write_rst )
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

UINT8 *x76f041_device::password()
{
	switch( m_command & 0xe0 )
	{
	case COMMAND_WRITE:
		return m_write_password;

	case COMMAND_READ:
		return m_read_password;

	default:
		return m_configuration_password;
	}
}

void x76f041_device::password_ok()
{
	switch( m_command & 0xe0 )
	{
	case COMMAND_WRITE:
		m_state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ:
		m_state = STATE_READ_DATA;
		break;
	case COMMAND_WRITE_USE_CONFIGURATION_PASSWORD:
		m_state = STATE_WRITE_DATA;
		break;
	case COMMAND_READ_USE_CONFIGURATION_PASSWORD:
		m_state = STATE_READ_DATA;
		break;
	case COMMAND_CONFIGURATION:
		switch( m_address )
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
			m_state = STATE_WRITE_CONFIGURATION_REGISTERS;
			m_byte = 0;
			break;
		case CONFIGURATION_READ_CONFIGURATION_REGISTERS:
			m_state = STATE_READ_CONFIGURATION_REGISTERS;
			m_byte = 0;
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

void x76f041_device::load_address()
{
	/* todo: handle other bcr bits */
	int bcr;

	m_address = m_shift;

	verboselog( 1, "-> address: %02x\n", m_address );

	if( ( m_command & 1 ) == 0 )
	{
		bcr = m_configuration_registers[ CONFIG_BCR1 ];
	}
	else
	{
		bcr = m_configuration_registers[ CONFIG_BCR2 ];
	}
	if( ( m_address & 0x80 ) != 0 )
	{
		bcr >>= 4;
	}

	if( ( ( m_command & 0xe0 ) == COMMAND_READ && ( bcr & BCR_Z ) != 0 && ( bcr & BCR_T ) != 0 ) ||
		( ( m_command & 0xe0 ) == COMMAND_WRITE && ( bcr & BCR_Z ) != 0 ) )
	{
		/* todo: find out when this is really checked. */
		verboselog( 1, "command not allowed\n" );
		m_state = STATE_STOP;
		m_sdar = 0;
	}
	else if( ( ( m_command & 0xe0 ) == COMMAND_WRITE && ( bcr & BCR_X ) == 0 ) ||
		( ( m_command & 0xe0 ) == COMMAND_READ && ( bcr & BCR_Y ) == 0 ) )
	{
		verboselog( 1, "password not required\n" );
		password_ok();
	}
	else
	{
		verboselog( 1, "send password\n" );
		m_state = STATE_LOAD_PASSWORD;
		m_byte = 0;
	}
}

int x76f041_device::data_offset()
{
	int block_offset = ( ( m_command & 1 ) << 8 ) + m_address;

	// TODO: confirm block_start doesn't wrap.

	return ( block_offset & 0x180 ) | ( ( block_offset + m_byte ) & 0x7f );
}

WRITE_LINE_MEMBER( x76f041_device::write_scl )
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
				m_sdar = ( m_response_to_reset[ m_byte ] >> m_bit ) & 1;
				verboselog( 2, "in response to reset %d (%d/%d)\n", m_sdar, m_byte, m_bit );
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
		case STATE_LOAD_ADDRESS:
		case STATE_LOAD_PASSWORD:
		case STATE_VERIFY_PASSWORD:
		case STATE_WRITE_DATA:
		case STATE_WRITE_CONFIGURATION_REGISTERS:
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
						m_state = STATE_LOAD_ADDRESS;
						break;

					case STATE_LOAD_ADDRESS:
						load_address();
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
						if( m_shift == 0xc0 )
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

					case STATE_WRITE_CONFIGURATION_REGISTERS:
						verboselog( 1, "-> configuration register[ %d ]: %02x\n", m_byte, m_shift );
						/* todo: write after all bytes received? */
						m_configuration_registers[ m_byte++ ] = m_shift;

						if( m_byte == sizeof( m_configuration_registers ) )
						{
							m_byte = 0;
						}
						break;
					}

					m_bit = 0;
					m_shift = 0;
				}
			}
			break;

		case STATE_READ_DATA:
		case STATE_READ_CONFIGURATION_REGISTERS:
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
							verboselog( 1, "<- data[ %03x ]: %02x\n", offset, m_shift );
							break;

						case STATE_READ_CONFIGURATION_REGISTERS:
							offset = m_byte & 7;
							m_shift = m_configuration_registers[ offset ];
							verboselog( 1, "<- configuration register[ %d ]: %02x\n", offset, m_shift );
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

WRITE_LINE_MEMBER( x76f041_device::write_sda )
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
				verboselog( 1, "goto load address\n" );
				m_state = STATE_LOAD_ADDRESS;
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

READ_LINE_MEMBER( x76f041_device::read_sda )
{
	if( m_cs != 0 )
	{
		verboselog( 2, "not selected\n" );
		return 1;
	}

	verboselog( 2, "sdar=%d\n", m_sdar );
	return m_sdar;
}

void x76f041_device::nvram_default()
{
	m_response_to_reset[0] = 0x19;
	m_response_to_reset[1] = 0x55;
	m_response_to_reset[2] = 0xaa;
	m_response_to_reset[3] = 0x55,

	memset( m_write_password, 0, sizeof( m_write_password ) );
	memset( m_read_password, 0, sizeof( m_read_password ) );
	memset( m_configuration_password, 0, sizeof( m_configuration_password ) );
	memset( m_configuration_registers, 0, sizeof( m_configuration_registers ) );
	memset( m_data, 0, sizeof( m_data ) );

	int expected_bytes = sizeof( m_response_to_reset ) + sizeof( m_write_password ) + sizeof( m_read_password ) +
		sizeof( m_configuration_password ) + sizeof( m_configuration_registers ) + sizeof( m_data );

	if( !m_region )
	{
		logerror( "x76f041(%s) region not found\n", tag().c_str() );
	}
	else if( m_region->bytes() != expected_bytes )
	{
		logerror( "x76f041(%s) region length 0x%x expected 0x%x\n", tag().c_str(), m_region->bytes(), expected_bytes );
	}
	else
	{
		UINT8 *region = m_region->base();

		memcpy( m_response_to_reset, region, sizeof( m_response_to_reset ) ); region += sizeof( m_response_to_reset );
		memcpy( m_write_password, region, sizeof( m_write_password ) ); region += sizeof( m_write_password );
		memcpy( m_read_password, region, sizeof( m_read_password ) ); region += sizeof( m_read_password );
		memcpy( m_configuration_password, region, sizeof( m_configuration_password ) ); region += sizeof( m_configuration_password );
		memcpy( m_configuration_registers, region, sizeof( m_configuration_registers ) ); region += sizeof( m_configuration_registers );
		memcpy( m_data, region, sizeof( m_data ) ); region += sizeof( m_data );
	}
}

void x76f041_device::nvram_read( emu_file &file )
{
	file.read( m_response_to_reset, sizeof( m_response_to_reset ) );
	file.read( m_write_password, sizeof( m_write_password ) );
	file.read( m_read_password, sizeof( m_read_password ) );
	file.read( m_configuration_password, sizeof( m_configuration_password ) );
	file.read( m_configuration_registers, sizeof( m_configuration_registers ) );
	file.read( m_data, sizeof( m_data ) );
}

void x76f041_device::nvram_write( emu_file &file )
{
	file.write( m_response_to_reset, sizeof( m_response_to_reset ) );
	file.write( m_write_password, sizeof( m_write_password ) );
	file.write( m_read_password, sizeof( m_read_password ) );
	file.write( m_configuration_password, sizeof( m_configuration_password ) );
	file.write( m_configuration_registers, sizeof( m_configuration_registers ) );
	file.write( m_data, sizeof( m_data ) );
}
