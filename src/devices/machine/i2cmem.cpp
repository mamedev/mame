// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

I2C Memory

ram/rom/eeprom/flash on an i2c bus.

inputs:
 e0,e1,e2  lower 3 bits of the slave address
 sda       serial data
 scl       serial clock
 wc        write protect

outputs:
 sda       serial data

The memory address is only 8 bits for devices up to 2048 bytes,
devices from 512 to 2048 bytes occupy multiple slave addresses.

The top five address bits are set at manufacture time,
there are two standard values.
***************************************************************************/

#include "emu.h"
#include "machine/i2cmem.h"

constexpr int STATE_IDLE(0);
constexpr int STATE_DEVSEL(1);
constexpr int STATE_ADDRESSHIGH(2);
constexpr int STATE_ADDRESSLOW(3);
constexpr int STATE_DATAIN(4);
constexpr int STATE_DATAOUT(5);
constexpr int STATE_RESET(6);

constexpr int DEVSEL_RW(1);
constexpr int DEVSEL_ADDRESS(0xfe);

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE_LEVEL ( 0 )

static inline void ATTR_PRINTF( 3, 4 ) verboselog( device_t *device, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		device->logerror( "%s: I2CMEM %s", device->machine().describe_context(), buf );
	}
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I2C_X24C01,  i2c_x24c01_device,  "x24c01",  "X24C01 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C01,   i2c_24c01_device,   "24c01",   "24C01 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_PCF8570, i2c_pcf8570_device, "pcf8570", "PCF8570 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_PCD8572, i2c_pcd8572_device, "pcd8572", "PCD8572 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_PCF8582, i2c_pcf8582_device, "pcf8582", "PCF8582 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C02,   i2c_24c02_device,   "24c02",   "24C02 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_M24C02,  i2c_m24c02_device,  "m24c02",  "M24C02 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C04,   i2c_24c04_device,   "24c04",   "24C04 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_X2404P,  i2c_x2404p_device,  "x2404p",  "X2404P I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C08,   i2c_24c08_device,   "24c08",   "24C08 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C16,   i2c_24c16_device,   "24c16",   "24C16 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C64,   i2c_24c64_device,   "24c64",   "24C64 I2C Memory")
DEFINE_DEVICE_TYPE(I2C_24C512,  i2c_24c512_device,  "24c512",  "24C512 I2C Memory")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  i2cmem_device - constructor
//-------------------------------------------------

i2cmem_device::i2cmem_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock,
		int read_page_size,
		int write_page_size,
		int data_size) :
	device_t(mconfig, type, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_region(*this, DEVICE_SELF),
	m_slave_address(I2CMEM_SLAVE_ADDRESS),
	m_read_page_size(read_page_size),
	m_write_page_size(write_page_size),
	m_data_size(data_size),
	m_scl(0),
	m_sdaw(0),
	m_e0(0),
	m_e1(0),
	m_e2(0),
	m_wc(0),
	m_sdar(1),
	m_state(STATE_IDLE),
	m_bits(0),
	m_shift(0),
	m_devsel(0),
	m_addresshigh(0),
	m_byteaddr(0),
	m_page_offset(0),
	m_page_written_size(0),
	m_devsel_address_low(false)
{
	// these memories work off the I2C clock only
	assert(!clock);
}

i2c_x24c01_device::i2c_x24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_X24C01, tag, owner, clock, 0, 8, 0x80)
{
	set_devsel_address_low(true);
}

i2c_24c01_device::i2c_24c01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C01, tag, owner, clock, 0, 8, 0x80)
{
}

i2c_pcf8570_device::i2c_pcf8570_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_PCF8570, tag, owner, clock, 0, 0, 0x100)
{
}

i2c_pcd8572_device::i2c_pcd8572_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_PCD8572, tag, owner, clock, 0, 0, 0x80)
{
}

i2c_pcf8582_device::i2c_pcf8582_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_PCF8582, tag, owner, clock, 0, 0, 0x100)
{
}

i2c_24c02_device::i2c_24c02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C02, tag, owner, clock, 0, 8, 0x100)
{
}

i2c_m24c02_device::i2c_m24c02_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_M24C02, tag, owner, clock, 0, 16, 0x100)
{
}

i2c_24c04_device::i2c_24c04_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C04, tag, owner, clock, 0, 16, 0x200)
{
}

i2c_x2404p_device::i2c_x2404p_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_X2404P, tag, owner, clock, 0x100, 8, 0x200)
{
}

i2c_24c08_device::i2c_24c08_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C08, tag, owner, clock, 0, 16, 0x400)
{
}

i2c_24c16_device::i2c_24c16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C16, tag, owner, clock, 0, 16, 0x800)
{
}

i2c_24c64_device::i2c_24c64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C64, tag, owner, clock, 0, 32, 0x2000)
{
}

i2c_24c512_device::i2c_24c512_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	i2cmem_device(mconfig, I2C_24C512, tag, owner, clock, 0, 128, 0x10000)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i2cmem_device::device_start()
{
	m_data = std::make_unique<uint8_t []>(m_data_size);
	m_page.resize( m_write_page_size );

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
	if (!skip_addresshigh())
		save_item( NAME(m_addresshigh) );
	save_item( NAME(m_byteaddr) );
	save_item( NAME(m_page_offset) );
	save_item( NAME(m_page_written_size) );
	save_pointer( &m_data[0], "m_data", m_data_size );
	if ( m_write_page_size > 0 )
	{
		save_item( NAME(m_page) );
	}
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void i2cmem_device::nvram_default()
{
	memset(&m_data[0], 0xff, m_data_size);

	/* populate from a memory region if present */
	if (m_region.found())
	{
		if( m_region->bytes() != m_data_size )
		{
			fatalerror( "i2cmem region '%s' wrong size (expected size = 0x%X)\n", tag(), m_data_size );
		}

		if( m_region->bytewidth() != 1 )
		{
			fatalerror( "i2cmem region '%s' needs to be an 8-bit region\n", tag() );
		}

		memcpy(&m_data[0], m_region->base(), m_data_size);
	}
}


//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool i2cmem_device::nvram_read( util::read_stream &file )
{
	size_t actual;
	return !file.read( &m_data[0], m_data_size, actual ) && actual == m_data_size;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool i2cmem_device::nvram_write( util::write_stream &file )
{
	size_t actual;
	return !file.write( &m_data[0], m_data_size, actual ) && actual == m_data_size;
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
				if( m_page_written_size > 0 )
				{
					int base = data_offset();
					int root = base & ~( m_write_page_size - 1 );
					for( int i = 0; i < m_page_written_size; i++ )
						m_data[root | ((base + i) & (m_write_page_size - 1))] = m_page[i];
					verboselog( this, 1, "data[ %04x to %04x ] = %x bytes\n", base, root | ((base + m_page_written_size - 1) & (m_write_page_size - 1)), m_page_written_size );

					m_page_written_size = 0;
				}
				verboselog( this, 1, "stop\n" );
				m_state = STATE_IDLE;
			}
			else
			{
				verboselog( this, 1, "start\n" );
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
		case STATE_ADDRESSHIGH:
		case STATE_ADDRESSLOW:
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

						if( m_devsel == 0 && !m_devsel_address_low )
						{
							// TODO: Atmel datasheets document 2-wire software reset, but doesn't mention it will lower sda only that it will release it.
							// ltv_naru however requires it to be lowered, but we don't currently know the manufacturer of the chip used.
							verboselog( this, 1, "software reset\n" );
							m_state = STATE_RESET;
							m_sdar = 0;
						}
						else if( !select_device() )
						{
							verboselog( this, 1, "devsel %02x: not this device\n", m_devsel );
							m_state = STATE_IDLE;
						}
						else if( ( m_devsel & DEVSEL_RW ) == 0 )
						{
							if (m_devsel_address_low)
							{
								verboselog( this, 1, "devsel %02x: write (Xicor special, address %02x)\n", m_devsel, m_devsel >> 1);
								m_byteaddr = (m_devsel & DEVSEL_ADDRESS) >> 1;
								m_page_offset = 0;
								m_page_written_size = 0;
								m_state = STATE_DATAIN;
							}
							else
							{
								verboselog( this, 1, "devsel %02x: write\n", m_devsel );
								m_state = skip_addresshigh() ? STATE_ADDRESSLOW : STATE_ADDRESSHIGH;
							}
						}
						else
						{
							if (m_devsel_address_low)
							{
								verboselog( this, 1, "devsel %02x: read (Xicor special, address %02x)\n", m_devsel, m_devsel >> 1);
								m_byteaddr = (m_devsel & DEVSEL_ADDRESS) >> 1;
							}
							else
							{
								verboselog( this, 1, "devsel %02x: read\n", m_devsel );
							}
							m_state = STATE_DATAOUT;
						}
						break;

					case STATE_ADDRESSHIGH:
						m_addresshigh = m_shift;

						verboselog(this, 1, "addresshigh %02x\n", m_addresshigh);

						m_state = STATE_ADDRESSLOW;
						break;

					case STATE_ADDRESSLOW:
						m_byteaddr = m_shift | (skip_addresshigh() ? ((m_devsel & DEVSEL_ADDRESS) << 7) & address_mask() : m_addresshigh << 8);
						m_page_offset = 0;
						m_page_written_size = 0;

						verboselog( this, 1, "addresslow %02x (byteaddr %04x)\n", m_shift, m_byteaddr );

						m_state = STATE_DATAIN;
						break;

					case STATE_DATAIN:
						if( m_wc )
						{
							verboselog( this, 0, "write not enabled\n" );
							m_state = STATE_IDLE;
						}
						else if( m_write_page_size > 0 )
						{
							m_page[ m_page_offset ] = m_shift;
							verboselog( this, 1, "page[ %04x ] <- %02x\n", m_page_offset, m_page[ m_page_offset ] );

							m_page_offset++;
							if( m_page_offset == m_write_page_size )
								m_page_offset = 0;
							m_page_written_size++;
							if( m_page_written_size > m_write_page_size)
								m_page_written_size = m_write_page_size;
						}
						else
						{
							int offset = data_offset();

							verboselog( this, 1, "data[ %04x ] <- %02x\n", offset, m_shift );
							m_data[ offset ] = m_shift;

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

						m_shift = m_data[offset];
						verboselog( this, 1, "data[ %04x ] -> %02x\n", offset, m_shift );
						m_byteaddr = (m_byteaddr & ~(m_read_page_size - 1)) | ((m_byteaddr + 1) & (m_read_page_size - 1));
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
						verboselog( this, 1, "nack\n" );
						m_state = STATE_IDLE;
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

		case STATE_RESET:
			if( m_scl )
			{
				verboselog(this, 1, "software reset ack\n");
				m_state = STATE_IDLE;
				m_sdar = 1;
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
	if ( m_devsel_address_low )
	{
		// Due to a full address and read/write flag fitting in one 8-bit packet, the Xicor X24C01 replies on all addresses.
		return 1;
	}

	int device = ( m_slave_address & 0xf0 ) | ( m_e2 << 3 ) | ( m_e1 << 2 ) | ( m_e0 << 1 );
	int mask = DEVSEL_ADDRESS & ~( skip_addresshigh() ? address_mask() >> 7 : 0 );

	if( ( m_devsel & mask ) == ( device & mask ) )
	{
		return 1;
	}

	return 0;
}

int i2cmem_device::data_offset()
{
	return m_byteaddr & address_mask();
}
