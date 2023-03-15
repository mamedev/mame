// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
/*

    Atmel Serial DataFlash

    (c) 2001-2007 Tim Schuerewegen

    AT45DB041 -  528 KByte
    AT45DB081 - 1056 KByte
    AT45DB161 - 2112 KByte

*/

#include "emu.h"
#include "at45dbxx.h"

#define LOG_LEVEL  1
#define _logerror(level,x)  do { if (LOG_LEVEL > level) logerror x; } while (0)

#define FLASH_CMD_52  0x52
#define FLASH_CMD_57  0x57
#define FLASH_CMD_60  0x60
#define FLASH_CMD_82  0x82

#define FLASH_MODE_XX  0 // unknown
#define FLASH_MODE_SI  1 // input
#define FLASH_MODE_SO  2 // output


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(AT45DB041, at45db041_device, "at45db041", "AT45DB041")
DEFINE_DEVICE_TYPE(AT45DB081, at45db081_device, "at45db081", "AT45DB081")
DEFINE_DEVICE_TYPE(AT45DB161, at45db161_device, "at45db161", "AT45DB161")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  at45db041_device - constructor
//-------------------------------------------------

at45db041_device::at45db041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at45db041_device(mconfig, AT45DB041, tag, owner, clock)
{
}


at45db041_device::at45db041_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
	, write_so(*this)
{
}


at45db081_device::at45db081_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at45db041_device(mconfig, AT45DB081, tag, owner, clock)
{
}


at45db161_device::at45db161_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: at45db041_device(mconfig, AT45DB161, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void at45db041_device::device_start()
{
	m_size = num_pages() * page_size();
	m_data.resize(m_size);
	m_buffer1.resize(page_size());
	//m_buffer2.resize(page_size());

	// pins
	m_pin.cs    = 0;
	m_pin.sck   = 0;
	m_pin.si    = 0;
	m_pin.wp    = 0;
	m_pin.reset = 0;
	m_pin.busy  = 0;

	// data
	save_item(NAME(m_data));
	// pins
	save_item(NAME(m_pin.cs));
	save_item(NAME(m_pin.sck));
	save_item(NAME(m_pin.si));
	save_item(NAME(m_pin.so));
	save_item(NAME(m_pin.wp));
	save_item(NAME(m_pin.reset));
	save_item(NAME(m_pin.busy));

	write_so.resolve_safe();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void at45db041_device::device_reset()
{
	_logerror( 1, ("at45dbxx_reset\n"));
	// mode
	m_mode = FLASH_MODE_SI;
	m_status = 0;
	// command
	memset(&m_cmd.data[0], 0, sizeof(m_cmd.data));
	m_cmd.size = 0;
	// input/output
	m_io.data = nullptr;
	m_io.size = 0;
	m_io.pos  = 0;
	// pins
	m_pin.so  = 0;
	// output
	m_so_byte = 0;
	m_so_bits = 0;
	// input
	m_si_byte = 0;
	m_si_bits = 0;
}


//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void at45db041_device::nvram_default()
{
	memset(&m_data[0], 0xff, m_data.size());

	memory_region *region = memregion(DEVICE_SELF);
	if (region != nullptr)
	{
		uint32_t bytes = region->bytes();
		if (bytes > m_size)
			bytes = m_size;

		memcpy(&m_data[0], region->base(), bytes);
	}
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool at45db041_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_data[0], m_size, actual) && actual == m_size;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool at45db041_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_data[0], m_size, actual) && actual == m_size;
}

uint8_t at45db041_device::read_byte()
{
	uint8_t data;
	// check mode
	if ((m_mode != FLASH_MODE_SO) || (!m_io.data)) return 0;
	// read byte
	data = m_io.data[m_io.pos++];
	_logerror( 2, ("at45dbxx_read_byte (%02X) (%03d/%03d)\n", data, m_io.pos, m_io.size));
	if (m_io.pos == m_io.size) m_io.pos = 0;
	return data;
}

void at45db041_device::flash_set_io(uint8_t* data, uint32_t size, uint32_t pos)
{
	m_io.data = data;
	m_io.size = size;
	m_io.pos  = pos;
}

uint32_t at45db041_device::flash_get_page_addr()
{
	return ((m_cmd.data[1] & 0x0F) << 7) | ((m_cmd.data[2] & 0xFE) >> 1);
}

uint32_t at45db041_device::flash_get_byte_addr()
{
	return ((m_cmd.data[2] & 0x01) << 8) | ((m_cmd.data[3] & 0xFF) >> 0);
}

uint32_t at45db081_device::flash_get_page_addr()
{
	return ((m_cmd.data[1] & 0x1F) << 7) | ((m_cmd.data[2] & 0xFE) >> 1);
}

uint32_t at45db161_device::flash_get_page_addr()
{
	return ((m_cmd.data[1] & 0x3F) << 6) | ((m_cmd.data[2] & 0xFC) >> 2);
}

uint32_t at45db161_device::flash_get_byte_addr()
{
	return ((m_cmd.data[2] & 0x03) << 8) | ((m_cmd.data[3] & 0xFF) >> 0);
}

void at45db041_device::write_byte(uint8_t data)
{
	// check mode
	if (m_mode != FLASH_MODE_SI) return;
	// process byte
	if (m_cmd.size < 8)
	{
		uint8_t opcode;
		_logerror( 2, ("at45dbxx_write_byte (%02X)\n", data));
		// add to command buffer
		m_cmd.data[m_cmd.size++] = data;
		// check opcode
		opcode = m_cmd.data[0];
		switch (opcode)
		{
			// status register read
			case FLASH_CMD_57 :
			{
				// 8 bits command
				if (m_cmd.size == 1)
				{
					_logerror( 1, ("at45dbxx opcode %02X - status register read\n", opcode));
					m_status = (m_status & 0xC7) | device_id(); // 80 = busy / 40 = compare fail
					flash_set_io(&m_status, 1, 0);
					m_mode = FLASH_MODE_SO;
					m_cmd.size = 8;
				}
			}
			break;
			// main memory page to buffer 1 compare
			case FLASH_CMD_60 :
			{
				// 8 bits command + 4 bits reserved + 11 bits page address + 9 bits don't care
				if (m_cmd.size == 4)
				{
					uint32_t page;
					uint8_t comp;
					page = flash_get_page_addr();
					_logerror( 1, ("at45dbxx opcode %02X - main memory page to buffer 1 compare [%04X]\n", opcode, page));
					comp = memcmp( &m_data[page * page_size()], &m_buffer1[0], page_size()) == 0 ? 0 : 1;
					if (comp) m_status |= 0x40; else m_status &= ~0x40;
					_logerror( 1, ("at45dbxx page compare %s\n", comp ? "failure" : "success"));
					m_mode = FLASH_MODE_SI;
					m_cmd.size = 8;
				}
			}
			break;
			// main memory page read
			case FLASH_CMD_52 :
			{
				// 8 bits command + 4 bits reserved + 11 bits page address + 9 bits buffer address + 32 bits don't care
				if (m_cmd.size == 8)
				{
					uint32_t page, byte;
					page = flash_get_page_addr();
					byte = flash_get_byte_addr();
					_logerror( 1, ("at45dbxx opcode %02X - main memory page read [%04X/%04X]\n", opcode, page, byte));
					flash_set_io(&m_data[page * page_size()], page_size(), byte);
					m_mode = FLASH_MODE_SO;
					m_cmd.size = 8;
				}
			}
			break;
			// main memory page program through buffer 1
			case FLASH_CMD_82 :
			{
				// 8 bits command + 4 bits reserved + 11 bits page address + 9 bits buffer address
				if (m_cmd.size == 4)
				{
					uint32_t page, byte;
					page = flash_get_page_addr();
					byte = flash_get_byte_addr();
					_logerror( 1, ("at45dbxx opcode %02X - main memory page program through buffer 1 [%04X/%04X]\n",opcode, page, byte));
					flash_set_io(&m_buffer1[0], page_size(), byte);
					memset(&m_buffer1[0], 0xff, m_buffer1.size());
					m_mode = FLASH_MODE_SI;
					m_cmd.size = 8;
				}
			}
			break;
			// other
			default :
			{
				_logerror( 1, ("at45dbxx opcode %02X - unknown\n", opcode));
				m_cmd.data[0] = 0;
				m_cmd.size = 0;
			}
			break;
		}
	}
	else
	{
		_logerror( 2, ("at45dbxx_write_byte (%02X) (%03d/%03d)\n", data, m_io.pos + 1, m_io.size));
		// store byte
		m_io.data[m_io.pos] = data;
		m_io.pos++;
		if (m_io.pos == m_io.size) m_io.pos = 0;
	}
}

READ_LINE_MEMBER(at45db041_device::so_r)
{
	if (m_pin.cs == 0) return 0;
	return m_pin.so;
}

WRITE_LINE_MEMBER(at45db041_device::si_w)
{
	if (m_pin.cs == 0) return;
	m_pin.si = state;
}

WRITE_LINE_MEMBER(at45db041_device::cs_w)
{
	// check if changed
	if (m_pin.cs == state) return;
	// cs low-to-high
	if (state != 0)
	{
		// complete program command
		if ((m_cmd.size >= 4) && (m_cmd.data[0] == FLASH_CMD_82))
		{
			uint32_t page, byte;
			page = flash_get_page_addr();
			byte = flash_get_byte_addr();
			_logerror( 1, ("at45dbxx - program data stored in buffer 1 into selected page in main memory [%04X/%04X]\n", page, byte));
			memcpy( &m_data[page * page_size()], &m_buffer1[0], page_size());
		}
		// reset
		at45db041_device::device_reset();
	}
	// save cs
	m_pin.cs = state;
}

WRITE_LINE_MEMBER(at45db041_device::sck_w)
{
	// check if changed
	if (m_pin.sck == state) return;
	// sck high-to-low
	if (state == 0)
	{
		// output (part 1)
		if (m_so_bits == 8)
		{
			m_so_bits = 0;
			m_so_byte = read_byte();
		}
		// output (part 2)
		m_pin.so = (m_so_byte >> m_so_bits) & 1;
		write_so(m_pin.so);
		m_so_bits++;
	}
	else
	{
		// input
		if (m_pin.si) m_si_byte = m_si_byte | (1 << m_si_bits);
		m_si_bits++;
		if (m_si_bits == 8)
		{
			m_si_bits = 0;
			write_byte(m_si_byte);
			m_si_byte = 0;
		}
	}
	// save sck
	m_pin.sck = state;
}
