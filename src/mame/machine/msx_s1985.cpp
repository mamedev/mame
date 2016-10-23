// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msx_s1985.h"


const device_type MSX_S1985 = &device_creator<msx_s1985_device>;


msx_s1985_device::msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_switched_device(mconfig, MSX_S1985, "MSX-Engine S1985", tag, owner, clock, "msx_s1985", __FILE__)
	, m_6_1(0)
	, m_6_2(0)
	, m_7(0)
{
}


uint8_t msx_s1985_device::get_id()
{
	return 0xFE;
}


uint8_t msx_s1985_device::io_read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	switch (offset)
	{
		case 0:
			return ~get_id();

		case 7:
			{
				uint8_t data = (m_7 & 0x80) ? m_6_2 : m_6_1;
				m_7 = ( m_7 << 1 ) | ( m_7 >> 7 );
				return data;
			}

		default:
			printf("msx_s1985: unhandled read from offset %02x\n", offset);
			break;
	}

	return 0xFF;
}


void msx_s1985_device::io_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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
