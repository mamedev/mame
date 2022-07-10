// license:BSD-3-Clause
// copyright-holders:smf, Carl
/*
 * ds1205.c
 *
 * MultiKey
 *
 */

#include "emu.h"
#include "ds1205.h"

#include <cstdio>


#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF( 3, 4 ) ds1205_device::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: ds1205(%s) %s", machine().describe_context(), tag(), buf );
	}
}

// device type definition
DEFINE_DEVICE_TYPE(DS1205, ds1205_device, "ds1205", "DS1205")

ds1205_device::ds1205_device( const machine_config &mconfig, const char *tag, device_t *owner, u32 clock )
	: device_t(mconfig, DS1205, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_rst( 0 ),
	m_clk( 0 ),
	m_dqw( 0 ), m_dqr(0), m_state(0), m_bit(0)
{
}

void ds1205_device::device_start()
{
	new_state( STATE_STOP );
	m_dqr = DQ_HIGH_IMPEDANCE;

	memset( m_command, 0, sizeof( m_command ) );
	memset( m_compare_register, 0, sizeof( m_compare_register ) );
	memset( m_scratchpad, 0, sizeof( m_scratchpad ) );

	save_item( NAME( m_rst ) );
	save_item( NAME( m_clk ) );
	save_item( NAME( m_dqw ) );
	save_item( NAME( m_dqr ) );
	save_item( NAME( m_state ) );
	save_item( NAME( m_bit ) );
	save_item( NAME( m_command ) );
	save_item( NAME( m_scratchpad ) );
	save_item( NAME( m_compare_register ) );
	save_item( NAME( m_identification ) );
	save_item( NAME( m_security_match ) );
	save_item( NAME( m_secure_memory ) );
}

void ds1205_device::nvram_default()
{
	memset( m_identification, 0, sizeof( m_identification ) );
	memset( m_security_match, 0, sizeof( m_security_match ) );
	memset( m_secure_memory, 0, sizeof( m_secure_memory ) );

	int expected_bytes = sizeof( m_identification ) + sizeof( m_security_match ) + sizeof( m_secure_memory );

	if (!m_region.found())
	{
		logerror( "ds1205(%s) region not found\n", tag() );
	}
	else if( m_region->bytes() != expected_bytes )
	{
		logerror( "ds1205(%s) region length 0x%x expected 0x%x\n", tag(), m_region->bytes(), expected_bytes );
	}
	else
	{
		u8 *region = m_region->base();

		for(int i = 0; i < 3; i++)
		{
			memcpy( m_identification[i], region, sizeof( m_identification[i] ) ); region += sizeof( m_identification[i] );
			memcpy( m_security_match[i], region, sizeof( m_security_match[i] ) ); region += sizeof( m_security_match[i] );
			memcpy( m_secure_memory[i], region, sizeof( m_secure_memory[i] ) ); region += sizeof( m_secure_memory[i] );
		}
	}
}

bool ds1205_device::nvram_read( util::read_stream &file )
{
	for(int i = 0; i < 3; i++)
	{
		size_t actual;
		if( file.read( m_identification[i], sizeof( m_identification[i] ), actual ) || actual != sizeof( m_identification[i] ) )
			return false;
		if( file.read( m_security_match[i], sizeof( m_security_match[i] ), actual ) || actual != sizeof( m_security_match[i] ) )
			return false;
		if( file.read( m_secure_memory[i], sizeof( m_secure_memory[i] ), actual ) || actual != sizeof( m_secure_memory[i] ) )
			return false;
	}

	return true;
}

bool ds1205_device::nvram_write( util::write_stream &file )
{
	for(int i = 0; i < 3; i++)
	{
		size_t actual;
		if( file.write( m_identification[i], sizeof( m_identification[i] ), actual ) || actual != sizeof( m_identification[i] ) )
			return false;
		if( file.write( m_security_match[i], sizeof( m_security_match[i] ), actual ) || actual != sizeof( m_security_match[i] ) )
			return false;
		if( file.write( m_secure_memory[i], sizeof( m_secure_memory[i] ), actual ) || actual != sizeof( m_secure_memory[i] ) )
			return false;
	}

	return true;
}

void ds1205_device::new_state( int state )
{
	m_state = state;
	m_bit = 0;
}

void ds1205_device::writebit( u8 *buffer )
{
	if( m_clk )
	{
		int index = m_bit / 8;
		int mask = 1 << ( m_bit % 8 );

		if( m_dqw )
		{
			buffer[ index ] |= mask;
		}
		else
		{
			buffer[ index ] &= ~mask;
		}

		m_bit++;
	}
}

void ds1205_device::readbit( u8 *buffer )
{
	if( !m_clk )
	{
		int index = m_bit / 8;
		int mask = 1 << ( m_bit % 8 );

		if( buffer[ index ] & mask )
		{
			m_dqr = 1;
		}
		else
		{
			m_dqr = 0;
		}
	}
	else
	{
		m_bit++;
	}
}

WRITE_LINE_MEMBER( ds1205_device::write_rst )
{
	if( m_rst != state )
	{
		m_rst = state;
		verboselog( 2, "rst=%d\n", m_rst );

		if( m_rst )
		{
			new_state( STATE_PROTOCOL );
		}
		else
		{
			switch( m_state )
			{
			case STATE_WRITE_IDENTIFICATION:
				verboselog( 0, "reset during write identification (bit=%d)\n", m_bit );
				break;
			case STATE_WRITE_SECURITY_MATCH:
				verboselog( 0, "reset during write security match (bit=%d)\n", m_bit );
				break;
			case STATE_WRITE_DATA:
				verboselog( 0, "reset during write secure memory (bit=%d)\n", m_bit );
				break;
			}

			new_state( STATE_STOP );
			m_dqr = DQ_HIGH_IMPEDANCE;
		}
	}
}

WRITE_LINE_MEMBER( ds1205_device::write_clk )
{
	if( m_clk != state )
	{
		m_clk = state;
		verboselog( 2, "clk=%d state=%d (bit=%d)\n", m_clk, m_state, m_bit );

		if( m_clk )
		{
			m_dqr = DQ_HIGH_IMPEDANCE;
		}

		switch( m_state )
		{
		case STATE_PROTOCOL:
			writebit( m_command );

			if( m_bit == 24 )
			{
				verboselog( 1, "-> command %02x %02x %02x\n",
					m_command[ 0 ], m_command[ 1 ], m_command[ 2 ] );

				if( m_command[ 0 ] == COMMAND_GET_SCRATCHPAD && (m_command[ 1 ] & 0xc0) == 0xc0 && m_command[ 1 ] == ( ~m_command[ 2 ] & 0xff ) )
				{
					new_state( STATE_READ_SCRATCH );
				}
				else if( m_command[ 0 ] == COMMAND_GET_DATA && (m_command[ 1 ] & 0xc0) != 0xc0 && (m_command[ 1 ] & 0x3f) >= 0x10 && m_command[ 1 ] == ( ~m_command[ 2 ] & 0xff ) )
				{
					new_state( STATE_READ_IDENTIFICATION );
				}
				else if( m_command[ 0 ] == COMMAND_SET_SECURITY && (m_command[ 1 ] & 0xc0) != 0xc0 && !(m_command [ 1 ] & 0x3f) && m_command[ 1 ] == ( ~m_command[ 2 ] & 0xff ) )
				{
					new_state( STATE_READ_IDENTIFICATION );
				}
				else
				{
					new_state( STATE_STOP );
				}
			}
			break;

		case STATE_READ_IDENTIFICATION:
			readbit( m_identification[ m_command[ 1 ] >> 6 ] );

			if( m_bit == 64 )
			{
				int page = m_command [ 1 ] >> 6;
				verboselog( 1, "<- identification %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_identification[ page ][ 0 ], m_identification[ page ][ 1 ], m_identification[ page ][ 2 ], m_identification[ page ][ 3 ],
					m_identification[ page ][ 4 ], m_identification[ page ][ 5 ], m_identification[ page ][ 6 ], m_identification[ page ][ 7 ] );

				new_state( STATE_WRITE_COMPARE_REGISTER );
			}
			break;

		case STATE_WRITE_COMPARE_REGISTER:
			writebit( m_compare_register );

			if( m_bit == 64 )
			{
				int page = m_command[ 1 ] >> 6;
				verboselog( 1, "-> compare register %02x %02x %02x %02x %02x %02x %02x %02x (%02x %02x %02x %02x %02x %02x %02x %02x)\n",
					m_compare_register[ 0 ], m_compare_register[ 1 ], m_compare_register[ 2 ], m_compare_register[ 3 ],
					m_compare_register[ 4 ], m_compare_register[ 5 ], m_compare_register[ 6 ], m_compare_register[ 7 ],
					m_security_match[ page ][ 0 ], m_security_match[ page ][ 1 ], m_security_match[ page ][ 2 ], m_security_match[ page ][ 3 ],
					m_security_match[ page ][ 4 ], m_security_match[ page ][ 5 ], m_security_match[ page ][ 6 ], m_security_match[ page ][ 7 ] );

				if( memcmp( m_compare_register, m_security_match[ page ], sizeof( m_compare_register ) ) == 0 )
				{
					if( m_command[ 0 ] == COMMAND_GET_DATA )
					{
						new_state( STATE_READ_DATA );
					}
					else if( m_command[ 0 ] == COMMAND_GET_SCRATCHPAD)
					{
						new_state( STATE_READ_SCRATCH );
					}
					else if( m_command[ 0 ] == COMMAND_SET_SECURITY)
					{
						new_state( STATE_WRITE_IDENTIFICATION );
					}
				}
				else
				{
					new_state( STATE_OUTPUT_GARBLED_DATA );
				}
			}
			break;

		case STATE_READ_DATA:
			readbit( m_secure_memory[ m_command[ 1 ] >> 6 ] );

			if( m_bit == 384 )
			{
				int page = m_command [ 1 ] >> 6;
				verboselog( 1, "<- secure memory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_secure_memory[ page ][ 0 ], m_secure_memory[ page ][ 1 ], m_secure_memory[ page ][ 2 ], m_secure_memory[ page ][ 3 ],
					m_secure_memory[ page ][ 4 ], m_secure_memory[ page ][ 5 ], m_secure_memory[ page ][ 6 ], m_secure_memory[ page ][ 7 ],
					m_secure_memory[ page ][ 8 ], m_secure_memory[ page ][ 9 ], m_secure_memory[ page ][ 10 ], m_secure_memory[ page ][ 11 ],
					m_secure_memory[ page ][ 12 ], m_secure_memory[ page ][ 13 ], m_secure_memory[ page ][ 14 ], m_secure_memory[ page ][ 15 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_READ_SCRATCH:
			readbit( m_scratchpad );

			if( m_bit == 512 )
			{
				verboselog( 1, "<- scratchpad %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_scratchpad[ 0 ], m_scratchpad[ 1 ], m_scratchpad[ 2 ], m_scratchpad[ 3 ],
					m_scratchpad[ 4 ], m_scratchpad[ 5 ], m_scratchpad[ 6 ], m_scratchpad[ 7 ],
					m_scratchpad[ 8 ], m_scratchpad[ 9 ], m_scratchpad[ 10 ], m_scratchpad[ 11 ],
					m_scratchpad[ 12 ], m_scratchpad[ 13 ], m_scratchpad[ 14 ], m_scratchpad[ 15 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_WRITE_IDENTIFICATION:
			writebit( m_identification[ m_command[ 1 ] >> 6 ] );

			if( m_bit == 64 )
			{
				int page = m_command[ 1 ] >> 6;
				verboselog( 1, "-> identification %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_identification[ page ][ 0 ], m_identification[ page ][ 1 ], m_identification[ page ][ 2 ], m_identification[ page ][ 3 ],
					m_identification[ page ][ 4 ], m_identification[ page ][ 5 ], m_identification[ page ][ 6 ], m_identification[ page ][ 7 ] );

				new_state( STATE_WRITE_SECURITY_MATCH );
			}
			break;

		case STATE_WRITE_SECURITY_MATCH:
			writebit( m_security_match[ m_command[ 1 ] >> 6 ] );

			if( m_bit == 64 )
			{
				int page = m_command[ 1 ] >> 6;
				verboselog( 1, ">- security match %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_security_match[ page ][ 0 ], m_security_match[ page ][ 1 ], m_security_match[ page ][ 2 ], m_security_match[ page ][ 3 ],
					m_security_match[ page ][ 4 ], m_security_match[ page ][ 5 ], m_security_match[ page ][ 6 ], m_security_match[ page ][ 7 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_OUTPUT_GARBLED_DATA:
			if( !m_clk && m_command[ 0 ] == COMMAND_GET_DATA )
			{
				m_dqr = machine().rand() & 1;
				m_bit++;
			}
			else if( m_clk && m_command[ 0 ] == COMMAND_SET_DATA )
			{
				m_bit++;
			}

			if( m_bit == 64 )
			{
				if( m_command[ 0 ] == COMMAND_GET_DATA )
				{
					verboselog( 1, "<- random\n" );
				}
				else
				{
					verboselog( 1, "-> ignore\n" );
				}

				new_state( STATE_STOP );
			}
			break;
		}
	}
}

WRITE_LINE_MEMBER( ds1205_device::write_dq )
{
	if( m_dqw != state )
	{
		m_dqw = state;

		verboselog( 2, "dqw=%d\n", m_dqw );
	}
}

READ_LINE_MEMBER( ds1205_device::read_dq )
{
	if( m_dqr == DQ_HIGH_IMPEDANCE )
	{
		verboselog( 2, "dqr=high impedance\n" );
		return 0;
	}

	verboselog( 2, "dqr=%d (bit=%d)\n", m_dqr, m_bit );
	return m_dqr;
}
