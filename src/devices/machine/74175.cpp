// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    5/74174/5 Hex/Quad D Flip-Flops with Clear

*****************************************************************************/

#include "emu.h"
#include "74175.h"

DEFINE_DEVICE_TYPE(TTL74174, ttl74174_device, "ttl74174", "54/74174 Hex D Flip-Flops with Clear")
DEFINE_DEVICE_TYPE(TTL74175, ttl74175_device, "ttl74175", "54/74175 Quad D Flip-Flops with Clear")

ttl741745_device::ttl741745_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_q1_func(*this)
	, m_q2_func(*this)
	, m_q3_func(*this)
	, m_q4_func(*this)
	, m_clock(0)
	, m_clear(0)
	, m_d1(0)
	, m_d2(0)
	, m_d3(0)
	, m_d4(0)
	, m_q1(0)
	, m_q2(0)
	, m_q3(0)
	, m_q4(0)
{
}

ttl74174_device::ttl74174_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl741745_device(mconfig, TTL74174, tag, owner, clock)
	, m_q5_func(*this)
	, m_q6_func(*this)
	, m_d5(0)
	, m_d6(0)
	, m_q5(0)
	, m_q6(0)
{
}

ttl74175_device::ttl74175_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ttl741745_device(mconfig, TTL74175, tag, owner, clock)
	, m_not_q1_func(*this)
	, m_not_q2_func(*this)
	, m_not_q3_func(*this)
	, m_not_q4_func(*this)
{
}

void ttl741745_device::device_start()
{
	save_item(NAME(m_d1));
	save_item(NAME(m_d2));
	save_item(NAME(m_d3));
	save_item(NAME(m_d4));
	save_item(NAME(m_q1));
	save_item(NAME(m_q2));
	save_item(NAME(m_q3));
	save_item(NAME(m_q4));

	m_q1_func.resolve_safe();
	m_q2_func.resolve_safe();
	m_q3_func.resolve_safe();
	m_q4_func.resolve_safe();
}

void ttl741745_device::device_reset()
{
	init();
}

void ttl74174_device::device_start()
{
	ttl741745_device::device_start();

	save_item(NAME(m_d5));
	save_item(NAME(m_d6));
	save_item(NAME(m_q5));
	save_item(NAME(m_q6));

	m_q5_func.resolve_safe();
	m_q6_func.resolve_safe();
}

void ttl74175_device::device_start()
{
	save_item(NAME(m_not_q1));
	save_item(NAME(m_not_q2));
	save_item(NAME(m_not_q3));
	save_item(NAME(m_not_q4));

	m_not_q1_func.resolve_safe();
	m_not_q2_func.resolve_safe();
	m_not_q3_func.resolve_safe();
	m_not_q4_func.resolve_safe();
}

void ttl741745_device::init()
{
	m_clock = 0;
	m_clear = 0;

	m_d1 = 0;
	m_d2 = 0;
	m_d3 = 0;
	m_d4 = 0;

	m_q1 = 0;
	m_q2 = 0;
	m_q3 = 0;
	m_q4 = 0;
}

void ttl741745_device::tick()
{
	uint8_t last_q1 = m_q1;
	uint8_t last_q2 = m_q2;
	uint8_t last_q3 = m_q3;
	uint8_t last_q4 = m_q4;

	int q1 = m_d1;
	int q2 = m_d2;
	int q3 = m_d3;
	int q4 = m_d4;
	if (m_clear)
	{
		q1 = 0;
		q2 = 0;
		q3 = 0;
		q4 = 0;
	}

	m_q1 = q1;
	m_q2 = q2;
	m_q3 = q3;
	m_q4 = q4;

	if (last_q1 != m_q1)
		m_q1_func(m_q1);
	if (last_q2 != m_q2)
		m_q2_func(m_q2);
	if (last_q3 != m_q3)
		m_q3_func(m_q3);
	if (last_q4 != m_q4)
		m_q4_func(m_q4);
}

WRITE_LINE_MEMBER( ttl741745_device::clear_w )
{
	m_clear = state;
}

WRITE_LINE_MEMBER( ttl741745_device::clock_w )
{
	uint8_t last_clock = m_clock;
	m_clock = state;
	if (m_clock != last_clock && m_clock != 0)
	{
		tick();
	}
}

WRITE_LINE_MEMBER( ttl741745_device::d1_w )
{
	m_d1 = state;
}

WRITE_LINE_MEMBER( ttl741745_device::d2_w )
{
	m_d2 = state;
}

WRITE_LINE_MEMBER( ttl741745_device::d3_w )
{
	m_d3 = state;
}

WRITE_LINE_MEMBER( ttl741745_device::d4_w )
{
	m_d4 = state;
}

WRITE_LINE_MEMBER( ttl74174_device::d5_w )
{
	m_d5 = state;
}

WRITE_LINE_MEMBER( ttl74174_device::d6_w )
{
	m_d6 = state;
}

void ttl74174_device::init()
{
	ttl741745_device::init();
	m_d5 = 0;
	m_d6 = 0;

	m_q5 = 0;
	m_q6 = 0;
}

void ttl74174_device::tick()
{
	ttl741745_device::tick();

	uint8_t last_q5 = m_q5;
	uint8_t last_q6 = m_q6;

	int q5 = m_d5;
	int q6 = m_d6;
	if (m_clear)
	{
		q5 = 0;
		q6 = 0;
	}

	m_q5 = q5;
	m_q6 = q6;

	if (last_q5 != m_q5)
		m_q5_func(m_q5);
	if (last_q6 != m_q6)
		m_q6_func(m_q6);
}

void ttl74175_device::tick()
{
	uint8_t last_q1 = m_q1;
	uint8_t last_q2 = m_q2;
	uint8_t last_q3 = m_q3;
	uint8_t last_q4 = m_q4;

	ttl741745_device::tick();

	if (last_q1 != m_q1)
		m_not_q1_func(m_q1 ^ 1);
	if (last_q2 != m_q1)
		m_not_q2_func(m_q2 ^ 1);
	if (last_q3 != m_q1)
		m_not_q3_func(m_q3 ^ 1);
	if (last_q4 != m_q1)
		m_not_q4_func(m_q4 ^ 1);
}
