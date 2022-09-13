// license:BSD-3-Clause
// copyright-holders:hap
/*

Hitachi HD61603 LCD Driver
64 segment outputs, no duty cycle

TODO:
- OSC pin (input is resistor)
- SB pin, halts internal clock
- SYNC pin for chip cascading
- READY pin

*/

#include "emu.h"
#include "video/hd61603.h"


DEFINE_DEVICE_TYPE(HD61603, hd61603_device, "hd61603", "Hitachi HD61603 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hd61603_device::hd61603_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, HD61603, tag, owner, clock),
	m_write_segs(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hd61603_device::device_start()
{
	// resolve callbacks
	m_write_segs.resolve_safe();

	// zerofill
	m_blank = 0;
	m_count = 0;
	m_data = 0;
	m_ram = 0;

	// register for savestates
	save_item(NAME(m_blank));
	save_item(NAME(m_count));
	save_item(NAME(m_data));
	save_item(NAME(m_ram));
}

void hd61603_device::device_reset()
{
	m_count = 0;
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void hd61603_device::data_w(u8 data)
{
	// 1-byte nop command
	if (m_count == 0 && (data & 0xc) == 0xc)
		return;

	// each command is 4 writes
	m_data = (m_data << 4) | (data & 0xf);
	m_count = (m_count + 1) & 3;

	if (m_count == 0)
	{
		switch (m_data >> 14 & 3)
		{
			// write byte
			case 0:
			{
				u8 shift = (m_data >> 8 & 7) * 8;
				u8 digit = bitswap<8>(m_data & 0xff, 0,1,2,3,4,5,6,7);
				m_ram = (m_ram & ~(u64(0xff) << shift)) | u64(digit) << shift;
				break;
			}

			// write bit
			case 1:
			{
				u64 mask = u64(1) << (m_data & 0x3f);
				m_ram = (m_ram & ~mask) | ((m_data & 0x2000) ? mask : 0);
				break;
			}

			// set mode
			case 2:
				m_blank = BIT(m_data, 2);
				break;

			default:
				break;
		}

		m_write_segs(0, m_blank ? 0 : m_ram);
	}
}
