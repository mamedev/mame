// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    (DM)9334 8-Bit Addressable Latch

*****************************************************************************/

#include "emu.h"
#include "dm9334.h"

DEFINE_DEVICE_TYPE(DM9334, dm9334_device, "dm9934", "NS [DM]9934 8-bit Addressable Latch")

dm9334_device::dm9334_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DM9334, tag, owner, clock)
	, m_out_func(*this)
	, m_q0_func(*this)
	, m_q1_func(*this)
	, m_q2_func(*this)
	, m_q3_func(*this)
	, m_q4_func(*this)
	, m_q5_func(*this)
	, m_q6_func(*this)
	, m_q7_func(*this)
	, m_e(0)
	, m_c(0)
	, m_d(0)
	, m_a(0)
	, m_out(0)
{
}

void dm9334_device::device_start()
{
	init();

	save_item(NAME(m_e));
	save_item(NAME(m_c));
	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_out));

	m_out_func.resolve_safe();
	m_q0_func.resolve_safe();
	m_q1_func.resolve_safe();
	m_q2_func.resolve_safe();
	m_q3_func.resolve_safe();
	m_q4_func.resolve_safe();
	m_q5_func.resolve_safe();
	m_q6_func.resolve_safe();
	m_q7_func.resolve_safe();
}

void dm9334_device::device_reset()
{
	init();
}

void dm9334_device::init()
{
	m_e = 0;
	m_c = 0;
	m_d = 0;
	m_a = 0;
	m_out = 0;
}

void dm9334_device::tick()
{
	uint8_t last_out = m_out;

	mode_t mode = static_cast<mode_t>((m_e << 1) | m_c);
	switch(mode)
	{
		case mode_t::CLEAR:
			m_out = 0;
			break;

		case mode_t::DEMUX:
			m_out = (m_d << m_a);
			break;

		case mode_t::MEMORY:
			// Preserve previous state
			break;

		case mode_t::LATCH:
			m_out &= ~(1 << m_a);
			m_out |= (m_d << m_a);
			break;
	}

	if (m_out != last_out)
	{
		m_out_func(m_out);

		for (int bit = 0; bit < 8; bit++)
		{
			if (BIT(m_out, bit) == BIT(last_out, bit))
				continue;

			switch(bit)
			{
				case 0: m_q0_func(BIT(m_out, bit)); break;
				case 1: m_q1_func(BIT(m_out, bit)); break;
				case 2: m_q2_func(BIT(m_out, bit)); break;
				case 3: m_q3_func(BIT(m_out, bit)); break;
				case 4: m_q4_func(BIT(m_out, bit)); break;
				case 5: m_q5_func(BIT(m_out, bit)); break;
				case 6: m_q6_func(BIT(m_out, bit)); break;
				case 7: m_q7_func(BIT(m_out, bit)); break;
			}
		}
	}
}

WRITE_LINE_MEMBER( dm9334_device::e_w )
{
	uint8_t last_e = m_e;
	m_e = state & 1;
	if (last_e != m_e)
		tick();
}

WRITE_LINE_MEMBER( dm9334_device::c_w )
{
	uint8_t last_c = m_c;
	m_c = state & 1;
	if (last_c != m_c)
		tick();
}

WRITE_LINE_MEMBER( dm9334_device::d_w )
{
	uint8_t last_d = m_d;
	m_d = state & 1;
	if (last_d != m_d)
		tick();
}

WRITE_LINE_MEMBER( dm9334_device::a0_w )
{
	uint8_t last_a = m_a;
	m_a &= ~(1 << 0);
	m_a |= (state & 1) << 0;
	if (last_a != m_a)
		tick();
}

WRITE_LINE_MEMBER( dm9334_device::a1_w )
{
	uint8_t last_a = m_a;
	m_a &= ~(1 << 1);
	m_a |= (state & 1) << 1;
	if (last_a != m_a)
		tick();
}

WRITE_LINE_MEMBER( dm9334_device::a2_w )
{
	uint8_t last_a = m_a;
	m_a &= ~(1 << 2);
	m_a |= (state & 1) << 2;
	if (last_a != m_a)
		tick();
}

WRITE8_MEMBER( dm9334_device::a_w )
{
	uint8_t last_a = m_a;
	m_a = data & 0x7;
	if (last_a != m_a)
		tick();
}

READ8_MEMBER( dm9334_device::output_r )
{
	return m_out;
}

READ_LINE_MEMBER( dm9334_device::q0_r ) { return BIT(m_out, 0); }
READ_LINE_MEMBER( dm9334_device::q1_r ) { return BIT(m_out, 1); }
READ_LINE_MEMBER( dm9334_device::q2_r ) { return BIT(m_out, 2); }
READ_LINE_MEMBER( dm9334_device::q3_r ) { return BIT(m_out, 3); }
READ_LINE_MEMBER( dm9334_device::q4_r ) { return BIT(m_out, 4); }
READ_LINE_MEMBER( dm9334_device::q5_r ) { return BIT(m_out, 5); }
READ_LINE_MEMBER( dm9334_device::q6_r ) { return BIT(m_out, 6); }
READ_LINE_MEMBER( dm9334_device::q7_r ) { return BIT(m_out, 7); }
