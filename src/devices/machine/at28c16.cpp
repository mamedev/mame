// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************
    ATMEL AT28C16

    16K ( 2K x 8 ) Parallel EEPROM

***************************************************************************/

#include "emu.h"
#include "machine/at28c16.h"

#define AT28C16_DATA_BYTES ( 0x800 )
#define AT28C16_ID_BYTES ( 0x20 )
#define AT28C16_TOTAL_BYTES ( AT28C16_DATA_BYTES + AT28C16_ID_BYTES )

#define AT28C16_ID_OFFSET ( AT28C16_DATA_BYTES - AT28C16_ID_BYTES )



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

static ADDRESS_MAP_START( at28c16_map8, AS_PROGRAM, 8, at28c16_device )
	AM_RANGE(0x0000, 0x081f) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type AT28C16 = &device_creator<at28c16_device>;

//-------------------------------------------------
//  at28c16_device - constructor
//-------------------------------------------------

at28c16_device::at28c16_device( const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, AT28C16, "AT28C16", tag, owner, clock, "at28c16", __FILE__),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_a9_12v( 0 ),
		m_oe_12v( 0 ),
		m_last_write( -1 )
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void at28c16_device::device_config_complete()
{
	m_space_config = address_space_config( "at28c16", ENDIANNESS_BIG, 8,  12, 0, *ADDRESS_MAP_NAME( at28c16_map8 ) );
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void at28c16_device::device_validity_check(validity_checker &valid) const
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *at28c16_device::memory_space_config( address_spacenum spacenum ) const
{
	return ( spacenum == 0 ) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void at28c16_device::device_start()
{
	m_write_timer = timer_alloc(0);

	save_item( NAME(m_a9_12v) );
	save_item( NAME(m_oe_12v) );
	save_item( NAME(m_last_write) );
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
	if( m_region != nullptr )
	{
		if( m_region->bytes() != AT28C16_DATA_BYTES )
		{
			fatalerror( "at28c16 region '%s' wrong size (expected size = 0x%X)\n", tag(), AT28C16_DATA_BYTES );
		}

		if( m_region->bytewidth() != 1 )
		{
			fatalerror( "at28c16 region '%s' needs to be an 8-bit region\n", tag() );
		}

		UINT8 *default_data = m_region->base();

		for( offs_t offs = 0; offs < AT28C16_DATA_BYTES; offs++ )
			m_addrspace[ 0 ]->write_byte( offs, default_data[offs] );
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void at28c16_device::nvram_read( emu_file &file )
{
	dynamic_buffer buffer( AT28C16_TOTAL_BYTES );

	file.read( &buffer[0], AT28C16_TOTAL_BYTES );

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		m_addrspace[ 0 ]->write_byte( offs, buffer[ offs ] );
	}
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void at28c16_device::nvram_write( emu_file &file )
{
	dynamic_buffer buffer ( AT28C16_TOTAL_BYTES );

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		buffer[ offs ] = m_addrspace[ 0 ]->read_byte( offs );
	}

	file.write( &buffer[0], AT28C16_TOTAL_BYTES );
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

WRITE8_MEMBER( at28c16_device::write )
{
	if( m_last_write >= 0 )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) busy\n", machine.describe_context(), offset, data );
	}
	else if( m_oe_12v )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) erase\n", machine.describe_context(), offset, data );
		if( m_last_write < 0 )
		{
			for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
			{
				m_addrspace[ 0 ]->write_byte( offs, 0xff );
			}

			m_last_write = 0xff;
			m_write_timer->adjust( attotime::from_usec( 200 ) );
		}
	}
	else
	{
		if( m_a9_12v && offset >= AT28C16_ID_OFFSET )
		{
			offset += AT28C16_ID_BYTES;
		}

//      logerror( "%s: AT28C16: write( %04x, %02x )\n", machine.describe_context(), offset, data );
		if( m_last_write < 0 && m_addrspace[ 0 ]->read_byte( offset ) != data )
		{
			m_addrspace[ 0 ]->write_byte( offset, data );
			m_last_write = data;
			m_write_timer->adjust( attotime::from_usec( 200 ) );
		}
	}
}


READ8_MEMBER( at28c16_device::read )
{
	if( m_last_write >= 0 )
	{
		UINT8 data = m_last_write ^ 0x80;
//      logerror( "%s: AT28C16: read( %04x ) write status %02x\n", machine.describe_context(), offset, data );
		return data;
	}
	else
	{
		if( m_a9_12v && offset >= AT28C16_ID_OFFSET )
		{
			offset += AT28C16_ID_BYTES;
		}

		UINT8 data = m_addrspace[ 0 ]->read_byte( offset );
//      logerror( "%s: AT28C16: read( %04x ) data %02x\n", machine.describe_context(), offset, data );
		return data;
	}
}


WRITE_LINE_MEMBER( at28c16_device::set_a9_12v )
{
	state &= 1;
	if( m_a9_12v != state )
	{
//      logerror( "%s: AT28C16: set_a9_12v( %d )\n", machine.describe_context(), state );
		m_a9_12v = state;
	}
}


WRITE_LINE_MEMBER( at28c16_device::set_oe_12v )
{
	state &= 1;
	if( m_oe_12v != state )
	{
//      logerror( "%s: AT28C16: set_oe_12v( %d )\n", machine.describe_context(), state );
		m_oe_12v = state;
	}
}


void at28c16_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch( id )
	{
	case 0:
		m_last_write = -1;
		break;
	}
}
