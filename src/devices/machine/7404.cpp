// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    5/7404 Hex Inverters

*****************************************************************************/

#include "emu.h"
#include "7404.h"

DEFINE_DEVICE_TYPE(TTL7404, ttl7404_device, "7404", "5/7404 Hex Inverters")

ttl7404_device::ttl7404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TTL7404, tag, owner, clock)
	, m_y1_func(*this)
	, m_y2_func(*this)
	, m_y3_func(*this)
	, m_y4_func(*this)
	, m_y5_func(*this)
	, m_y6_func(*this)
	, m_a(0)
	, m_y(0x3f)
{
}

void ttl7404_device::device_start()
{
	init();

	save_item(NAME(m_a));
	save_item(NAME(m_y));

	m_y1_func.resolve_safe();
	m_y2_func.resolve_safe();
	m_y3_func.resolve_safe();
	m_y4_func.resolve_safe();
	m_y5_func.resolve_safe();
	m_y6_func.resolve_safe();
}

void ttl7404_device::device_reset()
{
	init();
}

void ttl7404_device::init()
{
	m_a = 0;
	m_y = 0x3f;
}

void ttl7404_device::update()
{
	uint8_t last_y = m_y;

	m_y = (~m_a) & 0x3f;

	if (m_y != last_y)
	{
		for (int bit = 0; bit < 6; bit++)
		{
			if (BIT(m_y, bit) == BIT(last_y, bit))
				continue;

			switch(bit)
			{
				case 0: m_y1_func(BIT(m_y, bit)); break;
				case 1: m_y2_func(BIT(m_y, bit)); break;
				case 2: m_y3_func(BIT(m_y, bit)); break;
				case 3: m_y4_func(BIT(m_y, bit)); break;
				case 4: m_y5_func(BIT(m_y, bit)); break;
				case 5: m_y6_func(BIT(m_y, bit)); break;
			}
		}
	}
}

void ttl7404_device::a_w(uint8_t line, uint8_t state)
{
	uint8_t old_a = m_a;
	m_a &= ~(1 << line);
	m_a |= (state << line);
	if (old_a != m_a)
		update();
}

uint8_t ttl7404_device::y_r(uint8_t line)
{
	return (m_y >> line) & 1;
}


WRITE_LINE_MEMBER( ttl7404_device::a1_w ) { a_w(0, state); }
WRITE_LINE_MEMBER( ttl7404_device::a2_w ) { a_w(1, state); }
WRITE_LINE_MEMBER( ttl7404_device::a3_w ) { a_w(2, state); }
WRITE_LINE_MEMBER( ttl7404_device::a4_w ) { a_w(3, state); }
WRITE_LINE_MEMBER( ttl7404_device::a5_w ) { a_w(4, state); }
WRITE_LINE_MEMBER( ttl7404_device::a6_w ) { a_w(5, state); }

READ_LINE_MEMBER( ttl7404_device::y1_r ) { return y_r(0); }
READ_LINE_MEMBER( ttl7404_device::y2_r ) { return y_r(1); }
READ_LINE_MEMBER( ttl7404_device::y3_r ) { return y_r(2); }
READ_LINE_MEMBER( ttl7404_device::y4_r ) { return y_r(3); }
READ_LINE_MEMBER( ttl7404_device::y5_r ) { return y_r(4); }
READ_LINE_MEMBER( ttl7404_device::y6_r ) { return y_r(5); }
