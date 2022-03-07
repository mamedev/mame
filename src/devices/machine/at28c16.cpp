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

void at28c16_device::at28c16_map8(address_map &map)
{
	map(0x0000, 0x081f).ram();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AT28C16, at28c16_device, "at28c16", "AT28C16 2Kx8 EEPROM")

//-------------------------------------------------
//  at28c16_device - constructor
//-------------------------------------------------

at28c16_device::at28c16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AT28C16, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		device_nvram_interface(mconfig, *this),
		m_space_config("at28c16", ENDIANNESS_BIG, 8,  12, 0, address_map_constructor(FUNC(at28c16_device::at28c16_map8), this)),
		m_a9_12v(0),
		m_oe_12v(0),
		m_last_write(-1),
		m_default_data(*this, DEVICE_SELF)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector at28c16_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
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
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void at28c16_device::nvram_default()
{
	uint16_t default_value = 0xff;
	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		space(AS_PROGRAM).write_byte( offs, default_value );
	}

	/* populate from a memory region if present */
	if (m_default_data.found())
	{
		for( offs_t offs = 0; offs < AT28C16_DATA_BYTES; offs++ )
			space(AS_PROGRAM).write_byte(offs, m_default_data[offs]);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool at28c16_device::nvram_read( util::read_stream &file )
{
	std::vector<uint8_t> buffer( AT28C16_TOTAL_BYTES );
	size_t actual;

	if (file.read( &buffer[0], AT28C16_TOTAL_BYTES, actual ) || actual != AT28C16_TOTAL_BYTES)
		return false;

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		space(AS_PROGRAM).write_byte( offs, buffer[ offs ] );
	}

	return true;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool at28c16_device::nvram_write( util::write_stream &file )
{
	std::vector<uint8_t> buffer ( AT28C16_TOTAL_BYTES );
	size_t actual;

	for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
	{
		buffer[ offs ] = space(AS_PROGRAM).read_byte( offs );
	}

	return !file.write( &buffer[0], AT28C16_TOTAL_BYTES, actual ) && actual == AT28C16_TOTAL_BYTES;
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

void at28c16_device::write(offs_t offset, uint8_t data)
{
	if( m_last_write >= 0 )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) busy\n", machine().describe_context(), offset, data );
	}
	else if( m_oe_12v )
	{
//      logerror( "%s: AT28C16: write( %04x, %02x ) erase\n", machine().describe_context(), offset, data );
		if( m_last_write < 0 )
		{
			for( offs_t offs = 0; offs < AT28C16_TOTAL_BYTES; offs++ )
			{
				this->space(AS_PROGRAM).write_byte( offs, 0xff );
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

//      logerror( "%s: AT28C16: write( %04x, %02x )\n", machine().describe_context(), offset, data );
		if( m_last_write < 0 && this->space(AS_PROGRAM).read_byte( offset ) != data )
		{
			this->space(AS_PROGRAM).write_byte( offset, data );
			m_last_write = data;
			m_write_timer->adjust( attotime::from_usec( 200 ) );
		}
	}
}


uint8_t at28c16_device::read(offs_t offset)
{
	if( m_last_write >= 0 )
	{
		uint8_t data = m_last_write ^ 0x80;
//      logerror( "%s: AT28C16: read( %04x ) write status %02x\n", machine().describe_context(), offset, data );
		return data;
	}
	else
	{
		if( m_a9_12v && offset >= AT28C16_ID_OFFSET )
		{
			offset += AT28C16_ID_BYTES;
		}

		uint8_t data = this->space(AS_PROGRAM).read_byte( offset );
//      logerror( "%s: AT28C16: read( %04x ) data %02x\n", machine().describe_context(), offset, data );
		return data;
	}
}


WRITE_LINE_MEMBER( at28c16_device::set_a9_12v )
{
	state &= 1;
	if( m_a9_12v != state )
	{
//      logerror( "%s: AT28C16: set_a9_12v( %d )\n", machine().describe_context(), state );
		m_a9_12v = state;
	}
}


WRITE_LINE_MEMBER( at28c16_device::set_oe_12v )
{
	state &= 1;
	if( m_oe_12v != state )
	{
//      logerror( "%s: AT28C16: set_oe_12v( %d )\n", machine().describe_context(), state );
		m_oe_12v = state;
	}
}


void at28c16_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch( id )
	{
	case 0:
		m_last_write = -1;
		break;
	}
}
