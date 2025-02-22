// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#include "emu.h"
#include "zr36060.h"

#define VERBOSE       (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ZR36060, zr36060_device, "zr36060", "Zoran ZR36060 Integrated JPEG codec")

zr36060_device::zr36060_device(const machine_config &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ZR36060, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
{
	m_space_config = address_space_config("regs", ENDIANNESS_BIG, 8, 10, 0, address_map_constructor(FUNC(zr36060_device::regs_map), this));

}

device_memory_interface::space_config_vector zr36060_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void zr36060_device::device_start()
{
}

void zr36060_device::device_reset()
{
}

void zr36060_device::regs_map(address_map &map)
{
//  map(0x000, 0x000) LOAD Parameters
//  map(0x001, 0x001) Code FIFO Status (r/o)
//  map(0x002, 0x002) Code Interface
//  map(0x003, 0x003) Codec Mode

//  map(0x005, 0x005) Maximum Block Code
//  map(0x006, 0x006) Markers Enable
//  map(0x007, 0x007) Interrupt Mask
//  map(0x008, 0x008) Interrupt Status (r/o)
//  map(0x009, 0x00c) Target Net Code Volume
//  map(0x00d, 0x010) Target Data Code Volume
//  map(0x011, 0x012) Scale Factor
//  map(0x013, 0x015) Allocation Factor
//  map(0x016, 0x019) Accumulated Code Volume
//  map(0x01a, 0x01d) Accumulated Total Activity
//  map(0x01e, 0x021) Accumulated Truncated Bits
	map(0x022, 0x022).lr8(NAME([this] (offs_t offset) { LOG("Read Device ID\n"); return 0x33; }));
	map(0x023, 0x023).lr8(NAME([this] (offs_t offset) { LOG("Read Revision ID\n"); return 0x01; }));
//  map(0x024, 0x025) Test Control

//  map(0x030, 0x030) Video Control
//  map(0x031, 0x031) Video Polarity
//  map(0x032, 0x032) Scaling
//  map(0x033, 0x035) Background Color
//  map(0x036, 0x041) Sync Generator
//  map(0x042, 0x049) Active Area
//  map(0x04a, 0x051) SUBIMG Window

	map(0x060, 0x3ff).ram(); // JPEG Markers Array
}

/**************************************
 *
 * Host I/F
 *
 *************************************/

u8 zr36060_device::read(offs_t offset)
{
	switch(offset & 3)
	{
		case 0:
			LOG("CODE FIFO read\n");
			return 0;
		case 1:
			return (m_address >> 8) & 3;
		case 2:
			return m_address & 0xff;
		case 3:
			return space(0).read_byte(m_address);
	}
	return 0;
}

void zr36060_device::write(offs_t offset, u8 data)
{
	switch(offset & 3)
	{
		case 0:
			LOG("CODE FIFO write %02x\n", data);
			break;
		case 1:
			m_address = (data << 8) | (m_address & 0xff);
			break;
		case 2:
			m_address = (data & 0xff) | (m_address & 0x300);
			break;
		case 3:
			space(0).write_byte(m_address, data);
			break;
	}
}
