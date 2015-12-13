// license:BSD-3-Clause
// copyright-holders:smf
/*
 * ds1204.c
 *
 * Electronic Key
 *
 */

#include <stdio.h>
#include "emu.h"
#include "ds1204.h"

#define VERBOSE_LEVEL ( 0 )

inline void ATTR_PRINTF( 3, 4 ) ds1204_device::verboselog( int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: ds1204(%s) %s", machine().describe_context(), tag(), buf );
	}
}

// device type definition
const device_type DS1204 = &device_creator<ds1204_device>;

ds1204_device::ds1204_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t( mconfig, DS1204, "DS1204", tag, owner, clock, "ds1204", __FILE__ ),
	device_nvram_interface(mconfig, *this),
	m_rst( 0 ),
	m_clk( 0 ),
	m_dqw( 0 ), m_dqr(0), m_state(0), m_bit(0)
{
}

void ds1204_device::device_start()
{
	new_state( STATE_STOP );
	m_dqr = DQ_HIGH_IMPEDANCE;

	memset( m_command, 0, sizeof( m_command ) );
	memset( m_compare_register, 0, sizeof( m_compare_register ) );

	save_item( NAME( m_rst ) );
	save_item( NAME( m_clk ) );
	save_item( NAME( m_dqw ) );
	save_item( NAME( m_dqr ) );
	save_item( NAME( m_state ) );
	save_item( NAME( m_bit ) );
	save_item( NAME( m_command ) );
	save_item( NAME( m_compare_register ) );
	save_item( NAME( m_unique_pattern ) );
	save_item( NAME( m_identification ) );
	save_item( NAME( m_security_match ) );
	save_item( NAME( m_secure_memory ) );
}

void ds1204_device::nvram_default()
{
	memset( m_unique_pattern, 0, sizeof( m_unique_pattern ) );
	memset( m_identification, 0, sizeof( m_identification ) );
	memset( m_security_match, 0, sizeof( m_security_match ) );
	memset( m_secure_memory, 0, sizeof( m_secure_memory ) );

	int expected_bytes = sizeof( m_unique_pattern ) + sizeof( m_identification ) + sizeof( m_security_match ) + sizeof( m_secure_memory );

	if( !m_region )
	{
		logerror( "ds1204(%s) region not found\n", tag() );
	}
	else if( m_region->bytes() != expected_bytes )
	{
		logerror( "ds1204(%s) region length 0x%x expected 0x%x\n", tag(), m_region->bytes(), expected_bytes );
	}
	else
	{
		UINT8 *region = m_region->base();

		memcpy( m_unique_pattern, region, sizeof( m_unique_pattern ) ); region += sizeof( m_unique_pattern );
		memcpy( m_identification, region, sizeof( m_identification ) ); region += sizeof( m_identification );
		memcpy( m_security_match, region, sizeof( m_security_match ) ); region += sizeof( m_security_match );
		memcpy( m_secure_memory, region, sizeof( m_secure_memory ) ); region += sizeof( m_secure_memory );
	}
}

void ds1204_device::nvram_read( emu_file &file )
{
	file.read( m_unique_pattern, sizeof( m_unique_pattern ) );
	file.read( m_identification, sizeof( m_identification ) );
	file.read( m_security_match, sizeof( m_security_match ) );
	file.read( m_secure_memory, sizeof( m_secure_memory ) );
}

void ds1204_device::nvram_write( emu_file &file )
{
	file.write( m_unique_pattern, sizeof( m_unique_pattern ) );
	file.write( m_identification, sizeof( m_identification ) );
	file.write( m_security_match, sizeof( m_security_match ) );
	file.write( m_secure_memory, sizeof( m_secure_memory ) );
}

void ds1204_device::new_state( int state )
{
	m_state = state;
	m_bit = 0;
}

void ds1204_device::writebit( UINT8 *buffer )
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

void ds1204_device::readbit( UINT8 *buffer )
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

WRITE_LINE_MEMBER( ds1204_device::write_rst )
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
			case STATE_WRITE_SECURE_MEMORY:
				verboselog( 0, "reset during write secure memory (bit=%d)\n", m_bit );
				break;
			}

			new_state( STATE_STOP );
			m_dqr = DQ_HIGH_IMPEDANCE;
		}
	}
}

WRITE_LINE_MEMBER( ds1204_device::write_clk )
{
	if( m_clk != state )
	{
		m_clk = state;
		verboselog( 2, "clk=%d (bit=%d)\n", m_clk, m_bit );

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
				verboselog( 1, "-> command %02x %02x %02x (%02x %02x)\n",
					m_command[ 0 ], m_command[ 1 ], m_command[ 2 ], m_unique_pattern[ 0 ], m_unique_pattern[ 1 ] );

				if( m_command[ 0 ] == COMMAND_READ && m_command[ 1 ] == ( m_unique_pattern[ 0 ] | CYCLE_NORMAL ) && m_command[ 2 ] == m_unique_pattern[ 1 ] )
				{
					new_state( STATE_READ_IDENTIFICATION );
				}
				else if( m_command[ 0 ] == COMMAND_WRITE && m_command[ 1 ] == ( m_unique_pattern[ 0 ] | CYCLE_NORMAL ) && m_command[ 2 ] == m_unique_pattern[ 1 ] )
				{
					new_state( STATE_READ_IDENTIFICATION );
				}
				else if( m_command[ 0 ] == COMMAND_WRITE && m_command[ 1 ] == ( m_unique_pattern[ 0 ] | CYCLE_PROGRAM ) && m_command[ 2 ] == m_unique_pattern[ 1 ] )
				{
					new_state( STATE_WRITE_IDENTIFICATION );
				}
				else
				{
					new_state( STATE_STOP );
				}
			}
			break;

		case STATE_READ_IDENTIFICATION:
			readbit( m_identification );

			if( m_bit == 64 )
			{
				verboselog( 1, "<- identification %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_identification[ 0 ], m_identification[ 1 ], m_identification[ 2 ], m_identification[ 3 ],
					m_identification[ 4 ], m_identification[ 5 ], m_identification[ 6 ], m_identification[ 7 ] );

				new_state( STATE_WRITE_COMPARE_REGISTER );
			}
			break;

		case STATE_WRITE_COMPARE_REGISTER:
			writebit( m_compare_register );

			if( m_bit == 64 )
			{
				verboselog( 1, "-> compare register %02x %02x %02x %02x %02x %02x %02x %02x (%02x %02x %02x %02x %02x %02x %02x %02x)\n",
					m_compare_register[ 0 ], m_compare_register[ 1 ], m_compare_register[ 2 ], m_compare_register[ 3 ],
					m_compare_register[ 4 ], m_compare_register[ 5 ], m_compare_register[ 6 ], m_compare_register[ 7 ],
					m_security_match[ 0 ], m_security_match[ 1 ], m_security_match[ 2 ], m_security_match[ 3 ],
					m_security_match[ 4 ], m_security_match[ 5 ], m_security_match[ 6 ], m_security_match[ 7 ] );

				if( memcmp( m_compare_register, m_security_match, sizeof( m_compare_register ) ) == 0 )
				{
					if( m_command[ 0 ] == COMMAND_READ )
					{
						new_state( STATE_READ_SECURE_MEMORY );
					}
					else
					{
						new_state( STATE_WRITE_SECURE_MEMORY );
					}
				}
				else
				{
					new_state( STATE_OUTPUT_GARBLED_DATA );
				}
			}
			break;

		case STATE_READ_SECURE_MEMORY:
			readbit( m_secure_memory );

			if( m_bit == 128 )
			{
				verboselog( 1, "<- secure memory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_secure_memory[ 0 ], m_secure_memory[ 1 ], m_secure_memory[ 2 ], m_secure_memory[ 3 ],
					m_secure_memory[ 4 ], m_secure_memory[ 5 ], m_secure_memory[ 6 ], m_secure_memory[ 7 ],
					m_secure_memory[ 8 ], m_secure_memory[ 9 ], m_secure_memory[ 10 ], m_secure_memory[ 11 ],
					m_secure_memory[ 12 ], m_secure_memory[ 13 ], m_secure_memory[ 14 ], m_secure_memory[ 15 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_WRITE_SECURE_MEMORY:
			writebit( m_secure_memory );

			if( m_bit == 128 )
			{
				verboselog( 1, "-> secure memory %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_secure_memory[ 0 ], m_secure_memory[ 1 ], m_secure_memory[ 2 ], m_secure_memory[ 3 ],
					m_secure_memory[ 4 ], m_secure_memory[ 5 ], m_secure_memory[ 6 ], m_secure_memory[ 7 ],
					m_secure_memory[ 8 ], m_secure_memory[ 9 ], m_secure_memory[ 10 ], m_secure_memory[ 11 ],
					m_secure_memory[ 12 ], m_secure_memory[ 13 ], m_secure_memory[ 14 ], m_secure_memory[ 15 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_WRITE_IDENTIFICATION:
			writebit( m_identification );

			if( m_bit == 64 )
			{
				verboselog( 1, "-> identification %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_identification[ 0 ], m_identification[ 1 ], m_identification[ 2 ], m_identification[ 3 ],
					m_identification[ 4 ], m_identification[ 5 ], m_identification[ 6 ], m_identification[ 7 ] );

				new_state( STATE_WRITE_SECURITY_MATCH );
			}
			break;

		case STATE_WRITE_SECURITY_MATCH:
			writebit( m_security_match );

			if( m_bit == 64 )
			{
				verboselog( 1, ">- security match %02x %02x %02x %02x %02x %02x %02x %02x\n",
					m_security_match[ 0 ], m_security_match[ 1 ], m_security_match[ 2 ], m_security_match[ 3 ],
					m_security_match[ 4 ], m_security_match[ 5 ], m_security_match[ 6 ], m_security_match[ 7 ] );

				new_state( STATE_STOP );
			}
			break;

		case STATE_OUTPUT_GARBLED_DATA:
			if( !m_clk && m_command[ 0 ] == COMMAND_READ )
			{
				m_dqr = machine().rand() & 1;
				m_bit++;
			}
			else if( m_clk && m_command[ 0 ] == COMMAND_WRITE )
			{
				m_bit++;
			}

			if( m_bit == 64 )
			{
				if( m_command[ 0 ] == COMMAND_READ )
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

WRITE_LINE_MEMBER( ds1204_device::write_dq )
{
	if( m_dqw != state )
	{
		m_dqw = state;

		verboselog( 2, "dqw=%d\n", m_dqw );
	}
}

READ_LINE_MEMBER( ds1204_device::read_dq )
{
	if( m_dqr == DQ_HIGH_IMPEDANCE )
	{
		verboselog( 2, "dqr=high impedance\n" );
		return 0;
	}

	verboselog( 2, "dqr=%d (bit=%d)\n", m_dqr, m_bit );
	return m_dqr;
}
