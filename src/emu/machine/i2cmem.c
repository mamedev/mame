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

INLINE void ATTR_PRINTF( 3, 4 ) verboselog( running_device *device, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: I2CMEM(%s) %s", cpuexec_describe_context( device->machine ), device->tag(), buf );
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


static ADDRESS_MAP_START( i2cmem_map8, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  i2cmem_device_config - constructor
//-------------------------------------------------

i2cmem_device_config::i2cmem_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock ) :
	device_config( mconfig, static_alloc_device_config, "I2CMEM", tag, owner, clock),
	device_config_memory_interface(mconfig, *this),
	device_config_nvram_interface(mconfig, *this)
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
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *i2cmem_device_config::static_alloc_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock )
{
	return global_alloc( i2cmem_device_config( mconfig, tag, owner, clock ) );
}



//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *i2cmem_device_config::alloc_device( running_machine &machine ) const
{
	return auto_alloc( &machine, i2cmem_device( machine, *this ) );
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void i2cmem_device_config::device_config_complete()
{
	// extract inline configuration from raw data
	const i2cmem_interface *intf = reinterpret_cast<const i2cmem_interface *>( m_inline_data[ INLINE_INTERFACE ] );

	// inherit a copy of the static data
	if( intf != NULL )
	{
		*static_cast<i2cmem_interface *>(this) = *intf;
	}

	m_space_config = address_space_config( "i2cmem", ENDIANNESS_BIG, 8,  m_address_bits, 0, *ADDRESS_MAP_NAME( i2cmem_map8 ) );
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool i2cmem_device_config::device_validity_check( const game_driver &driver ) const
{
	bool error = false;

	if( m_inline_data[ INLINE_INTERFACE ] == 0 )
	{
		mame_printf_error( "%s: %s i2cmem device '%s' did not specify an interface\n", driver.source_file, driver.name, tag() );
		error = true;
	}

	return error;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *i2cmem_device_config::memory_space_config( int spacenum ) const
{
	return ( spacenum == 0 ) ? &m_space_config : NULL;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i2cmem_device - constructor
//-------------------------------------------------

i2cmem_device::i2cmem_device( running_machine &_machine, const i2cmem_device_config &config ) :
	device_t( _machine, config ),
	device_memory_interface( _machine, config, *this ),
	device_nvram_interface( _machine, config, *this ),
	m_config( config ),
	m_scl( 0 ),
	m_sdaw( 0 ),
	m_e0( 0 ),
	m_e1( 0 ),
	m_e2( 0 ),
	m_wc( 0 ),
	m_sdar( 1 ),
	m_state( STATE_IDLE )
{
	if( m_page_size > 0 )
	{
		m_page = auto_alloc_array( machine, UINT8, m_page_size );
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i2cmem_device::device_start()
{
	state_save_register_device_item( this, 0, m_scl );
	state_save_register_device_item( this, 0, m_sdaw );
	state_save_register_device_item( this, 0, m_e0 );
	state_save_register_device_item( this, 0, m_e1 );
	state_save_register_device_item( this, 0, m_e2 );
	state_save_register_device_item( this, 0, m_wc );
	state_save_register_device_item( this, 0, m_sdar );
	state_save_register_device_item( this, 0, m_state );
	state_save_register_device_item( this, 0, m_bits );
	state_save_register_device_item( this, 0, m_shift );
	state_save_register_device_item( this, 0, m_devsel );
	state_save_register_device_item( this, 0, m_byteaddr );
	state_save_register_device_item_pointer( this, 0, m_page, m_page_size );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i2cmem_device::device_reset()
{
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void i2cmem_device::nvram_default()
{
	int i2cmem_bytes = m_config.m_data_size;

	UINT16 default_value = 0xff;
	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		memory_write_byte( m_addrspace[ 0 ], offs, default_value );
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
			memory_write_byte( m_addrspace[ 0 ], offs, m_region->u8( offs ) );
		}
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_read( mame_file &file )
{
	int i2cmem_bytes = m_config.m_data_size;
	UINT8 *buffer = auto_alloc_array( &m_machine, UINT8, i2cmem_bytes );

	mame_fread( &file, buffer, i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		memory_write_byte( m_addrspace[ 0 ], offs, buffer[ offs ] );
	}

	auto_free( &m_machine, buffer );
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void i2cmem_device::nvram_write( mame_file &file )
{
	int i2cmem_bytes = m_config.m_data_size;
	UINT8 *buffer = auto_alloc_array( &m_machine, UINT8, i2cmem_bytes );

	for( offs_t offs = 0; offs < i2cmem_bytes; offs++ )
	{
		buffer[ offs ] = memory_read_byte( m_addrspace[ 0 ], offs );
	}

	mame_fwrite( &file, buffer, i2cmem_bytes );

	auto_free( &m_machine, buffer );
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
				m_byteaddr = 0;
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
									memory_write_byte( m_addrspace[ 0 ], offset + i, m_page[ i ] );
								}

								m_page_offset = 0;
							}
						}
						else
						{
							int offset = data_offset();

							verboselog( this, 1, "data[ %04x ] <- %02x\n", offset, m_shift );
							memory_write_byte( m_addrspace[ 0 ], offset, m_shift );

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

						m_shift = memory_read_byte( m_addrspace[ 0 ], offset );
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
	int res = m_sdar & m_sdaw & 1;

	verboselog( this, 2, "read sda %d\n", res );

	return res;
}


//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

int i2cmem_device::address_mask()
{
	return ( 1 << m_config.m_address_bits ) - 1;
}

int i2cmem_device::select_device()
{
	int device = ( m_config.m_slave_address & 0xf0 ) | ( m_e2 << 3 ) | ( m_e1 << 2 ) | ( m_e0 << 1 );
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

const device_type I2CMEM = i2cmem_device_config::static_alloc_device_config;
