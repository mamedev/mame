// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_s1985.h"


const device_type MSX_S1985 = &device_creator<msx_s1985_device>;


msx_s1985_device::msx_s1985_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: msx_switched_device(mconfig, MSX_S1985, "MSX-Engine S1985", tag, owner, clock, "msx_s1985", __FILE__)
	, m_6_1(0)
	, m_6_2(0)
	, m_7(0)
{
}


UINT8 msx_s1985_device::get_id()
{
	return 0xFE;
}


READ8_MEMBER(msx_s1985_device::io_read)
{
	switch (offset)
	{
		case 0:
			return ~get_id();

		case 7:
			{
				UINT8 data = (m_7 & 0x80) ? m_6_2 : m_6_1;
				m_7 = ( m_7 << 1 ) | ( m_7 >> 7 );
				return data;
			}

		default:
			printf("msx_s1985: unhandled read from offset %02x\n", offset);
			break;
	}

	return 0xFF;
}


WRITE8_MEMBER(msx_s1985_device::io_write)
{
	switch (offset)
	{
		case 6:
			m_6_2 = m_6_1;
			m_6_1 = data;
			break;

		case 7:
			m_7 = data;
			break;

		default:
			printf("msx_s1985: unhandled write %02x to offset %02x\n", data, offset);
			break;
	}
}
