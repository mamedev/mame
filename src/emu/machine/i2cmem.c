// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

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

***************************************************************************/

#include "emu.h"
#include "machine/i2cmem.h"

#define STATE_IDLE ( 0 )
#define STATE_DEVSEL ( 1 )
#define STATE_BYTEADDR ( 2 )
#define STATE_DATAIN ( 3 )
#define STATE_DATAOUT ( 4 )

#define DEVSEL_RW ( 1 )
#define DEVSEL_ADDRESS ( 0xfe )

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF( 3, 4 ) verboselog( device_t *device, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: I2CMEM(%s) %s", device->machine().describe_context( ), device->tag(), buf );
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type I2CMEM = &device_creator<i2cmem_device>;

static ADDRESS_MAP_START( i2cmem_map8, AS_PROGRAM, 8, i2cmem_device )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i2cmem_device - constructor
//-------------------------------------------------

i2cmem_device::i2cmem_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, I2CMEM, "I2C Memory", tag, owner, clock, "i2cmem", __FILE__),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
	m_slave_address( I2CMEM_SLAVE_ADDRESS ),
	m_page_size( 0 ),
	m_data_size( 0 ),
	m_scl( 0 ),
	m_sdaw( 0 ),
	m_e0( 0 ),
	m_e1( 0 ),
	m_e2( 0 ),
	m_wc( 0 ),
	m_sdar( 1 ),
	m_state( STATE_IDLE ),
	m_shift( 0 ),
	m_byteaddr( 0 )
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i2cmem_device::device_config_complete()
{
	int address_bits = 0;

	int i = m_data_size - 1;
	while( i > 0 )
	{
		address_bits++;
		i >>= 1;
	}

	m_space_config = address_space_config( "i2cmem", ENDIANNESS_BIG, 8,  address_bits, 0, *ADDRESS_MAP_NAME( i2cmem_map8 ) );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i2cmem_device::device_start()
{
	m_page.resize( m_page_size );

	save_item( NAME(m_scl) );
	save_item( NAME(m_sdaw) );
	save_item( NAME(m_e0) );
	save_item( NAME(m_e1) );
	save_item( NAME(m_e2) );
	save_item( NAME(m_wc) );
	save_item( NAME(m_sdar) );
	save_item( NAME(m_state) );
	save_item( NAME(m_bits) );
	save_item( NAME(m_shift) );
	save_item( NAME(m_devsel) );
	save_item( NAME(m_byteaddr) );
	if ( m_page_size > 0 )
	{
		save_item( NAME(m_page) );
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i2cmem_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *i2cmem_device::memory_space_config( address_spacenum spacenum ) const
{
	return ( spacenum == 0 ) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void i2cmem_device::nvram_default()
{
	int i2cmem_bytes = m_data_size;

	UINT16 default_value = 0xff;
	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, default_value );
	}

	/* populate from a memory region if present */
	if( m_region != NULL )
	{
		if( m_region->bytes() != i2cmem_bytes )
		{
			fatalerror( "i2cmem region '%s' wrong size (expected size = 0x%X)\n", tag(), i2cmem_bytes );
		}

		if( m_region->bytewidth() != 1 )
		{
			fatalerror( "i2cmem region '%s' needs to be an 8-bit region\n", tag() );
		}

		UINT8 *default_data = m_region->base();
		for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
			m_addrspace[ 0 ]->write_byte( offs, default_data[offs] );
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_read( emu_file &file )
{
	int i2cmem_bytes = m_data_size;
	dynamic_buffer buffer ( i2cmem_bytes );

	file.read( &buffer[0], i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, buffer[ offs ] );
	}
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_write( emu_file &file )
{
	int i2cmem_bytes = m_data_size;
	dynamic_buffer buffer ( i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		buffer[ offs ] = m_addrspace[ 0 ]->read_byte( offs );
	}

	file.write( &buffer[0], i2cmem_bytes );
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_MEMBER( i2cmem_device::write_e0 )
{
	state &= 1;
	if( m_e0 != state )
	{
		verboselog( this, 2, "set e0 %d\n", state );
		m_e0 = state;
	}
}


WRITE_LINE_MEMBER( i2cmem_device::write_e1 )
{
	state &= 1;
	if( m_e1 != state )
	{
		verboselog( this, 2, "set e1 %d\n", state );
		m_e1 = state;
	}
}


WRITE_LINE_MEMBER( i2cmem_device::write_e2 )
{
	state &= 1;
	if( m_e2 != state )
	{
		verboselog( this, 2, "set e2 %d\n", state );
		m_e2 = state;
	}
}


WRITE_LINE_MEMBER( i2cmem_device::write_sda )
{
	state &= 1;
	if( m_sdaw != state )
	{
		verboselog( this, 2, "set sda %d\n", state );
		m_sdaw = state;

		if( m_scl )
		{
			if( m_sdaw )
			{
				verboselog( this, 1, "stop\n" );
				m_state = STATE_IDLE;
			}
			else
			{
				verboselog( this, 2, "start\n" );
				m_state = STATE_DEVSEL;
				m_bits = 0;
			}

			m_sdar = 1;
		}
	}
}

WRITE_LINE_MEMBER( i2cmem_device::write_scl )
{
	if( m_scl != state )
	{
		m_scl = state;
		verboselog( this, 2, "set_scl_line %d\n", m_scl );

		switch( m_state )
		{
		case STATE_DEVSEL:
		case STATE_BYTEADDR:
		case STATE_DATAIN:
			if( m_bits < 8 )
			{
				if( m_scl )
				{
					m_shift = ( ( m_shift << 1 ) | m_sdaw ) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if( m_scl )
				{
					switch( m_state )
					{
					case STATE_DEVSEL:
						m_devsel = m_shift;

						if( !select_device() )
						{
							verboselog( this, 1, "devsel %02x: not this device\n", m_devsel );
							m_state = STATE_IDLE;
						}
						else if( ( m_devsel & DEVSEL_RW ) == 0 )
						{
							verboselog( this, 1, "devsel %02x: write\n", m_devsel );
							m_state = STATE_BYTEADDR;
						}
						else
						{
							verboselog( this, 1, "devsel %02x: read\n", m_devsel );
							m_state = STATE_DATAOUT;
						}
						break;

					case STATE_BYTEADDR:
						m_byteaddr = m_shift;
						m_page_offset = 0;

						verboselog( this, 1, "byteaddr %02x\n", m_byteaddr );

						m_state = STATE_DATAIN;
						break;

					case STATE_DATAIN:
						if( m_wc )
						{
							verboselog( this, 0, "write not enabled\n" );
							m_state = STATE_IDLE;
						}
						else if( m_page_size > 0 )
						{
							m_page[ m_page_offset ] = m_shift;
							verboselog( this, 1, "page[ %04x ] <- %02x\n", m_page_offset, m_page[ m_page_offset ] );

							m_page_offset++;
							if( m_page_offset == m_page_size )
							{
								int offset = data_offset() & ~( m_page_size - 1 );

								verboselog( this, 1, "data[ %04x to %04x ] = page\n", offset, offset + m_page_size - 1 );

								for( int i = 0; i < m_page_size; i++ )
								{
									m_addrspace[ 0 ]->write_byte( offset + i, m_page[ i ] );
								}

								m_page_offset = 0;
							}
						}
						else
						{
							int offset = data_offset();

							verboselog( this, 1, "data[ %04x ] <- %02x\n", offset, m_shift );
							m_addrspace[ 0 ]->write_byte( offset, m_shift );

							m_byteaddr++;
						}
						break;
					}

					m_bits++;
				}
				else
				{
					if( m_bits == 8 )
					{
						m_sdar = 0;
					}
					else
					{
						m_bits = 0;
						m_sdar = 1;
					}
				}
			}
			break;

		case STATE_DATAOUT:
			if( m_bits < 8 )
			{
				if( m_scl )
				{
					if( m_bits == 0 )
					{
						int offset = data_offset();

						m_shift = m_addrspace[ 0 ]->read_byte( offset );
						verboselog( this, 1, "data[ %04x ] -> %02x\n", offset, m_shift );
						m_byteaddr++;
					}

					m_sdar = ( m_shift >> 7 ) & 1;

					m_shift = ( m_shift << 1 ) & 0xff;
					m_bits++;
				}
			}
			else
			{
				if( m_scl )
				{
					if( m_sdaw )
					{
						verboselog( this, 1, "sleep\n" );
						m_state = STATE_IDLE;
						m_sdar = 0;
					}

					m_bits++;
				}
				else
				{
					if( m_bits == 8 )
					{
						m_sdar = 1;
					}
					else
					{
						m_bits = 0;
					}
				}
			}
			break;
		}
	}
}


WRITE_LINE_MEMBER( i2cmem_device::write_wc )
{
	state &= 1;
	if( m_wc != state )
	{
		verboselog( this, 2, "set wc %d\n", state );
		m_wc = state;
	}
}


READ_LINE_MEMBER( i2cmem_device::read_sda )
{
	int res = m_sdar & 1;

	verboselog( this, 2, "read sda %d\n", res );

	return res;
}


//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

int i2cmem_device::address_mask()
{
	return (m_data_size - 1);
}

int i2cmem_device::select_device()
{
	int device = ( m_slave_address & 0xf0 ) | ( m_e2 << 3 ) | ( m_e1 << 2 ) | ( m_e0 << 1 );
	int mask = DEVSEL_ADDRESS & ~( address_mask() >> 7 );

	if( ( m_devsel & mask ) == ( device & mask ) )
	{
		return 1;
	}

	return 0;
}

int i2cmem_device::data_offset()
{
	return ( ( ( m_devsel << 7 ) & 0xff00 ) | ( m_byteaddr & 0xff ) ) & address_mask();
}
