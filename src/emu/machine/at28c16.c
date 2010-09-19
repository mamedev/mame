/*
 * ATMEL AT28C16
 *
 * 16K ( 2K x 8 ) Parallel EEPROM
 *
 */

#include "emu.h"
#include "machine/at28c16.h"

#define AT28C16_DATA_BYTES ( 0x800 )
#define AT28C16_ID_BYTES ( 0x20 )
#define AT28C16_TOTAL_BYTES ( AT28C16_DATA_BYTES + AT28C16_ID_BYTES )

#define AT28C16_ID_OFFSET ( AT28C16_DATA_BYTES - AT28C16_ID_BYTES )



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type AT28C16 = at28c16_device_config::static_alloc_device_config;

static ADDRESS_MAP_START( at28c16_map8, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x081f) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  at28c16_device_config - constructor
//-------------------------------------------------

at28c16_device_config::at28c16_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock ) :
	device_config( mconfig, static_alloc_device_config, "AT28C16", tag, owner, clock),
	device_config_memory_interface(mconfig, *this),
	device_config_nvram_interface(mconfig, *this)
{
}



//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *at28c16_device_config::static_alloc_device_config( const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock )
{
	return global_alloc( at28c16_device_config( mconfig, tag, owner, clock ) );
}



//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *at28c16_device_config::alloc_device( running_machine &machine ) const
{
	return auto_alloc( &machine, at28c16_device( machine, *this ) );
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void at28c16_device_config::device_config_complete()
{
	m_space_config = address_space_config( "at28c16", ENDIANNESS_BIG, 8,  12, 0, *ADDRESS_MAP_NAME( at28c16_map8 ) );
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool at28c16_device_config::device_validity_check( const game_driver &driver ) const
{
	return false;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *at28c16_device_config::memory_space_config( int spacenum ) const
{
	return ( spacenum == 0 ) ? &m_space_config : NULL;
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  at28c16_device - constructor
//-------------------------------------------------

at28c16_device::at28c16_device( running_machine &_machine, const at28c16_device_config &config ) :
	device_t( _machine, config ),
	device_memory_interface( _machine, config, *this ),
	device_nvram_interface( _machine, config, *this ),
	m_config( config ),
	m_a9_12v( 0 ),
	m_oe_12v( 0 ),
	m_last_write( -1 )
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void at28c16_device::device_start()
{
	m_write_timer = timer_alloc( &m_machine,  write_finished, this );

	state_save_register_device_item( this, 0, m_a9_12v );
	state_save_register_device_item( this, 0, m_oe_12v );
	state_save_register_device_item( this, 0, m_last_write );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void at28c16_device::device_reset()
{
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void at28c16_device::nvram_default()
{
	UINT16 default_value = 0xff;
	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, default_value );
	}

	/* populate from a memory region if present */
	if( m_region != NULL )
	{
		if( m_region->bytes() != AT28C16_DATA_BYTES )
		{
			fatalerror( "at28c16 region '%s' wrong size (expected size = 0x%X)", tag(), AT28C16_DATA_BYTES );
		}

		if( m_region->width() != 1 )
		{
			fatalerror( "at28c16 region '%s' needs to be an 8-bit region", tag() );
		}

		for( offs_t offs = 0; offs < AT28C16_DATA_BYTES; offs++ )
		{
			m_addrspace[ 0 ]->write_byte( offs, m_region->u8( offs ) );
		}
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void at28c16_device::nvram_read( mame_file &file )
{
	UINT8 *buffer = auto_alloc_array( &m_machine, UINT8, AT28C16_TOTAL_BYTES );

	mame_fread( &file, buffer, AT28C16_TOTAL_BYTES );

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, buffer[ offs ] );
	}

	auto_free( &m_machine, buffer );
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void at28c16_device::nvram_write( mame_file &file )
{
	UINT8 *buffer = auto_alloc_array( &m_machine, UINT8, AT28C16_TOTAL_BYTES );

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		buffer[ offs ] = m_addrspace[ 0 ]->read_byte( offs );
	}

	mame_fwrite( &file, buffer, AT28C16_TOTAL_BYTES );

	auto_free( &m_machine, buffer );
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_DEVICE_HANDLER( at28c16_w )
{
	downcast<at28c16_device *>( device )->write( offset, data );
}

void at28c16_device::write( offs_t offset, UINT8 data )
{
	if( m_last_write >= 0 )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) busy\n", cpuexec_describe_context(machine), offset, data );
	}
	else if( m_oe_12v )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) erase\n", cpuexec_describe_context(machine), offset, data );
		if( m_last_write < 0 )
		{
			for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
			{
				m_addrspace[ 0 ]->write_byte( offs, 0xff );
			}

			m_last_write = 0xff;
			timer_adjust_oneshot( m_write_timer, ATTOTIME_IN_USEC( 200 ), 0 );
		}
	}
	else
	{
		if( m_a9_12v && offset >= AT28C16_ID_OFFSET )
		{
			offset += AT28C16_ID_BYTES;
		}

//      logerror( "%s: AT28C16: write( %04x, %02x )\n", cpuexec_describe_context(machine), offset, data );
		if( m_last_write < 0 && m_addrspace[ 0 ]->read_byte( offset ) != data )
		{
			m_addrspace[ 0 ]->write_byte( offset, data );
			m_last_write = data;
			timer_adjust_oneshot( m_write_timer, ATTOTIME_IN_USEC( 200 ), 0 );
		}
	}
}


READ8_DEVICE_HANDLER( at28c16_r )
{
	return downcast<at28c16_device *>( device )->read( offset );
}

UINT8 at28c16_device::read( offs_t offset )
{
	if( m_last_write >= 0 )
	{
		UINT8 data = m_last_write ^ 0x80;
//      logerror( "%s: AT28C16: read( %04x ) write status %02x\n", cpuexec_describe_context(machine), offset, data );
		return data;
	}
	else
	{
		if( m_a9_12v && offset >= AT28C16_ID_OFFSET )
		{
			offset += AT28C16_ID_BYTES;
		}

		UINT8 data = m_addrspace[ 0 ]->read_byte( offset );
//      logerror( "%s: AT28C16: read( %04x ) data %02x\n", cpuexec_describe_context(machine), offset, data );
		return data;
	}
}


WRITE_LINE_DEVICE_HANDLER( at28c16_a9_12v )
{
	downcast<at28c16_device *>( device )->set_a9_12v( state );
}

void at28c16_device::set_a9_12v( int state )
{
	state &= 1;
	if( m_a9_12v != state )
	{
//      logerror( "%s: AT28C16: set_a9_12v( %d )\n", cpuexec_describe_context(machine), state );
		m_a9_12v = state;
	}
}


WRITE_LINE_DEVICE_HANDLER( at28c16_oe_12v )
{
	downcast<at28c16_device *>( device )->set_oe_12v( state );
}

void at28c16_device::set_oe_12v( int state )
{
	state &= 1;
	if( m_oe_12v != state )
	{
//      logerror( "%s: AT28C16: set_oe_12v( %d )\n", cpuexec_describe_context(machine), state );
		m_oe_12v = state;
	}
}


//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

TIMER_CALLBACK( at28c16_device::write_finished )
{
	reinterpret_cast<at28c16_device *>(ptr)->m_last_write = -1;
}
