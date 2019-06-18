// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    am25s55x.cpp
    AMD Am25S557/Am25S558 8x8-bit Combinatorial Multiplier

***************************************************************************/

#include "emu.h"
#include "am25s55x.h"

/*****************************************************************************/

DEFINE_DEVICE_TYPE(AM25S557, am25s557_device, "am25s557", "AMD Am25S557 Combinatorial Multiplier")
DEFINE_DEVICE_TYPE(AM25S558, am25s558_device, "am25s558", "AMD Am25S558 Combinatorial Multiplier")

//-------------------------------------------------
//  am25s55x_device - constructor
//-------------------------------------------------

am25s55x_device::am25s55x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_x_in(0)
	, m_y_in(0)
	, m_xm(false)
	, m_ym(false)
	, m_oe(false)
	, m_round_s(false)
	, m_round_u(false)
	, m_s(*this)
{
}

am25s557_device::am25s557_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am25s55x_device(mconfig, AM25S557, tag, owner, clock)
	, m_r(false)
{
}

am25s558_device::am25s558_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am25s55x_device(mconfig, AM25S558, tag, owner, clock)
{
}

void am25s55x_device::device_start()
{
	save_item(NAME(m_x_in));
	save_item(NAME(m_y_in));
	save_item(NAME(m_xm));
	save_item(NAME(m_ym));
	save_item(NAME(m_oe));
	save_item(NAME(m_round_s));
	save_item(NAME(m_round_u));
	save_item(NAME(m_x.u));
	save_item(NAME(m_y.u));
	save_item(NAME(m_s_out.u));

	m_s.resolve_safe();
}

void am25s55x_device::device_reset()
{
	m_x_in = 0;
	m_y_in = 0;
	m_xm = false;
	m_ym = false;
	m_oe = false;
	m_round_s = false;
	m_round_u = false;

	m_x.u = 0;
	m_y.u = 0;
	m_s_out.u = 0;
}

void am25s55x_device::x_w(uint8_t data)
{
	m_x_in = data;
}

void am25s55x_device::y_w(uint8_t data)
{
	m_y_in = data;
}

void am25s55x_device::xm_w(int state)
{
	m_xm = (bool)state;
}

void am25s55x_device::ym_w(int state)
{
	m_ym = (bool)state;
}

void am25s55x_device::oe_w(int state)
{
	m_oe = (bool)state;
}

void am25s55x_device::multiply()
{
	if (m_xm)
	{
		if (m_ym)
		{
			m_s_out.s = (int16_t)m_x.u * (int16_t)m_y.u;
		}
		else
		{
			m_s_out.s = (int16_t)m_x.u * (uint16_t)m_y.u;
		}
	}
	else
	{
		if (m_ym)
		{
			m_s_out.s = (uint16_t)m_x.u * (int16_t)m_y.u;
		}
		else
		{
			m_s_out.u = (uint16_t)m_x.u * (uint16_t)m_y.u;
		}
	}

	if (m_round_u)
		m_s_out.u += 0x80;
	if (m_round_s)
		m_s_out.u += 0x40;

	if (!m_oe)
		m_s(m_s_out.u);
}

//
// Am25S557, combined signed/unsigned rounding pin
//

void am25s557_device::device_start()
{
	am25s55x_device::device_start();
	save_item(NAME(m_r));
}

void am25s557_device::device_reset()
{
	am25s55x_device::device_reset();
	m_r = false;
}

void am25s557_device::xm_w(int state)
{
	am25s55x_device::xm_w(state);
	update_rounding();
}

void am25s557_device::ym_w(int state)
{
	am25s55x_device::ym_w(state);
	update_rounding();
}

void am25s557_device::r_w(int state)
{
	m_r = (bool)state;
	update_rounding();
}

void am25s557_device::update_rounding()
{
	if (m_r)
	{
		if (m_xm || m_ym)
		{
			m_round_s = true;
			m_round_u = false;
		}
		else
		{
			m_round_s = false;
			m_round_u = true;
		}
	}
	else
	{
		m_round_s = false;
		m_round_u = false;
	}
}

//
// Am25S558, separate signed/unsigned rounding pins
//

void am25s558_device::rs_w(int state)
{
	m_round_s = (bool)state;
}

void am25s558_device::ru_w(int state)
{
	m_round_u = (bool)state;
}
