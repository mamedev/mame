// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina

#include "emu.h"
#include "machine/sda2006.h"

//-------------------------------------------------
//
// Siemens SDA2006 512-bit (32x16) NV EEPROM
//
// TODO:
//  - 8/12 bit controll word selection (currently emulates only 8 bt one)
//  - INV pin
//  - better( and correct) state flow
//  - read mode, with reversed data stream
//
//-------------------------------------------------

enum {
	CMD_WRITE,
	CMD_READ_REVERSED,
	CMD_READ,
	CMD_UNKNOWN
};

#define EEPROM_CAPACITY     0x40
#define EEPROM_ADDRESS_MASK 0x1f

// device type definition
DEFINE_DEVICE_TYPE(SDA2006, sda2006_device, "sda2006", "SDA2006 EEPROM")

//-------------------------------------------------
//  sda2006_device - constructor
//-------------------------------------------------

sda2006_device::sda2006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SDA2006, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, m_latch(0)
	, m_current_address(0)
	, m_eeprom_state()
	, m_read_stream_pos(0)
	, m_is_end_o_stream(false)
	, m_write_stream_length(0)
	, m_write_stream(0)
	, m_write_state(0)
	, m_clock_state(0)
	, m_region(*this, DEVICE_SELF)
{
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void sda2006_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sda2006_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_current_address));
	save_item(NAME(m_eeprom_state));
	save_item(NAME(m_read_stream_pos));
	save_item(NAME(m_is_end_o_stream));
	save_item(NAME(m_write_stream_length));
	save_item(NAME(m_write_stream));
	save_item(NAME(m_write_state));
	save_item(NAME(m_clock_state));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sda2006_device::device_reset()
{
	m_current_address = 0;
	m_is_end_o_stream = false;
	m_write_stream_length = 0;
	m_write_stream = 0;
	m_write_state = 0;
	m_read_stream_pos = 0;
	m_eeprom_state = EEPROM_WRITE;
	m_clock_state = 0;
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void sda2006_device::nvram_default()
{
	// region always wins
	if (m_region.found())
	{
		memcpy(m_eeprom_data, m_region->base(), EEPROM_CAPACITY);
		return;
	}

	for (auto & elem : m_eeprom_data)
		elem = 0xffff;
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool sda2006_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_eeprom_data, EEPROM_CAPACITY, actual) && actual == EEPROM_CAPACITY;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool sda2006_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_eeprom_data, EEPROM_CAPACITY, actual) && actual == EEPROM_CAPACITY;
}

READ_LINE_MEMBER( sda2006_device::read_data )
{
	return m_latch^1;
}

WRITE_LINE_MEMBER( sda2006_device::write_data )
{
	m_latch = state;
}

WRITE_LINE_MEMBER( sda2006_device::write_enable )
{
	if( (m_write_state ^ state) && (!state)){  //falling edge
		m_is_end_o_stream = true;
	}

	m_write_state = state;
}

WRITE_LINE_MEMBER( sda2006_device::write_clock )
{
	if( (m_clock_state ^ state) && (!state)) { // falling edge


		if( m_eeprom_state == EEPROM_READ){
			m_latch = (m_eeprom_data[m_current_address]>>(m_read_stream_pos))&1;
			++m_read_stream_pos;
			if ( m_read_stream_pos == 16) {
				// end of read
				m_eeprom_state = EEPROM_WRITE;
				m_write_stream_length = 0;
				m_write_stream = 0;
			}
		} else {
			if( m_is_end_o_stream ){
				// stream data  = AAAAACCC (read) or DDDDDDDDDDDDDDDDAAAAACCC (write)

				uint32_t reversed_stream = 0;
				uint32_t mask = 1;
				uint32_t counter = m_write_stream_length;
				m_is_end_o_stream = false;
				m_write_stream_length = 0;
				while(counter>0){
					reversed_stream<<=1;

					if (m_write_stream & mask) {
						reversed_stream |= 1;
					}
					mask <<= 1;
					--counter;
				}

				uint32_t command = bitswap<8>(m_write_stream, 7,6,5,4,3,0,1,2);

				switch (command&3){
					case CMD_WRITE:  m_eeprom_data[(reversed_stream>>16) & EEPROM_ADDRESS_MASK] = reversed_stream & 0xffff; break;
					case CMD_READ:
						m_current_address =  reversed_stream & EEPROM_ADDRESS_MASK;
						m_read_stream_pos = 0;
						m_eeprom_state = EEPROM_READ;
						break;
					case CMD_READ_REVERSED:
					case CMD_UNKNOWN: break;
				}
			} else {

				if( m_write_state ) {
					m_write_stream = ( m_write_stream << 1 ) | m_latch;
					++m_write_stream_length;
				}
			}
		}
	}
	m_clock_state = state;
}
