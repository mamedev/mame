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
	: device_t(mconfig, I2CMEM, "I2CMEM", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  device_nvram_interface(mconfig, *this),
	m_scl( 0 ),
	m_sdaw( 0 ),
	m_e0( 0 ),
	m_e1( 0 ),
	m_e2( 0 ),
	m_wc( 0 ),
	m_sdar( 1 ),
	m_state( STATE_IDLE )
{
	m_address_bits = 0;

	int i = m_data_size - 1;
	while( i > 0 )
	{
		m_address_bits++;
		i >>= 1;
	}
}


//-------------------------------------------------
//  static_set_interface - set the device
//  configuration
//-------------------------------------------------

void i2cmem_device::static_set_interface(device_t &device, const i2cmem_interface &interface)
{
	i2cmem_device &i2cmem = downcast<i2cmem_device &>(device);
	static_cast<i2cmem_interface &>(i2cmem) = interface;
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i2cmem_device::device_config_complete()
{
	m_space_config = address_space_config( "i2cmem", ENDIANNESS_BIG, 8,  m_address_bits, 0, *ADDRESS_MAP_NAME( i2cmem_map8 ) );
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void i2cmem_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i2cmem_device::device_start()
{
	if( m_page_size > 0 )
	{
		m_page = auto_alloc_array( machine(), UINT8, m_page_size );
	}

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
	save_pointer( NAME(m_page), m_page_size );
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
			fatalerror( "i2cmem region '%s' wrong size (expected size = 0x%X)", tag(), i2cmem_bytes );
		}

		if( m_region->width() != 1 )
		{
			fatalerror( "i2cmem region '%s' needs to be an 8-bit region", tag() );
		}

		for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
		{
			m_addrspace[ 0 ]->write_byte( offs, m_region->u8( offs ) );
		}
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_read( emu_file &file )
{
	int i2cmem_bytes = m_data_size;
	UINT8 *buffer = auto_alloc_array( machine(), UINT8, i2cmem_bytes );

	file.read( buffer, i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, buffer[ offs ] );
	}

	auto_free( machine(), buffer );
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_write( emu_file &file )
{
	int i2cmem_bytes = m_data_size;
	UINT8 *buffer = auto_alloc_array( machine(), UINT8, i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		buffer[ offs ] = m_addrspace[ 0 ]->read_byte( offs );
	}

	file.write( buffer, i2cmem_bytes );

	auto_free( machine(), buffer );
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE_LINE_DEVICE_HANDLER( i2cmem_e0_write )
{
	downcast<i2cmem_device *>( device )->set_e0_line( state );
}

void i2cmem_device::set_e0_line( int state )
{
	state &= 1;
	if( m_e0 != state )
	{
		verboselog( this, 2, "set e0 %d\n", state );
		m_e0 = state;
	}
}


WRITE_LINE_DEVICE_HANDLER( i2cmem_e1_write )
{
	downcast<i2cmem_device *>( device )->set_e1_line( state );
}

void i2cmem_device::set_e1_line( int state )
{
	state &= 1;
	if( m_e1 != state )
	{
		verboselog( this, 2, "set e1 %d\n", state );
		m_e1 = state;
	}
}


WRITE_LINE_DEVICE_HANDLER( i2cmem_e2_write )
{
	downcast<i2cmem_device *>( device )->set_e2_line( state );
}

void i2cmem_device::set_e2_line( int state )
{
	state &= 1;
	if( m_e2 != state )
	{
		verboselog( this, 2, "set e2 %d\n", state );
		m_e2 = state;
	}
}


WRITE_LINE_DEVICE_HANDLER( i2cmem_sda_write )
{
	downcast<i2cmem_device *>( device )->set_sda_line( state );
}

void i2cmem_device::set_sda_line( int state )
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

WRITE_LINE_DEVICE_HANDLER( i2cmem_scl_write )
{
	downcast<i2cmem_device *>( device )->set_scl_line( state );
}

void i2cmem_device::set_scl_line( int state )
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


WRITE_LINE_DEVICE_HANDLER( i2cmem_wc_write )
{
	downcast<i2cmem_device *>( device )->set_wc_line( state );
}

void i2cmem_device::set_wc_line( int state )
{
	state &= 1;
	if( m_wc != state )
	{
		verboselog( this, 2, "set wc %d\n", state );
		m_wc = state;
	}
}


READ_LINE_DEVICE_HANDLER( i2cmem_sda_read )
{
	return downcast<i2cmem_device *>( device )->read_sda_line();
}

int i2cmem_device::read_sda_line()
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
