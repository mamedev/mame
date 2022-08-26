// license:BSD-3-Clause
// copyright-holders:hap
/*

Sanyo LC7582 LCD Driver

53 outputs (static), or 106 outputs (1/2 duty)

TODO:
- OSC pin (input is R/C)
- AD/DSP function

*/

#include "emu.h"
#include "video/lc7582.h"


DEFINE_DEVICE_TYPE(LC7582, lc7582_device, "lc7582", "Sanyo LC7582 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

lc7582_device::lc7582_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, LC7582, tag, owner, clock),
	m_write_segs(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void lc7582_device::device_start()
{
	// resolve callbacks
	m_write_segs.resolve_safe();

	// zerofill
	m_data = 0;
	m_ce = 0;
	m_clk = 0;
	m_blank = false;
	m_duty = 0;
	m_addsp = 0;
	m_shift = 0;
	std::fill_n(m_latch, std::size(m_latch), 0);

	// register for savestates
	save_item(NAME(m_data));
	save_item(NAME(m_ce));
	save_item(NAME(m_clk));
	save_item(NAME(m_blank));
	save_item(NAME(m_duty));
	save_item(NAME(m_addsp));
	save_item(NAME(m_shift));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void lc7582_device::refresh_output()
{
	m_write_segs(0, m_blank ? 0 : m_latch[0]);
	m_write_segs(1, (m_blank || !m_duty) ? 0 : m_latch[1]);
}

WRITE_LINE_MEMBER(lc7582_device::clk_w)
{
	state = (state) ? 1 : 0;

	// clock shift register
	if (state && !m_clk)
		m_shift = m_shift >> 1 | u64(m_data) << 55;

	m_clk = state;
}

WRITE_LINE_MEMBER(lc7582_device::ce_w)
{
	state = (state) ? 1 : 0;

	// latch at falling edge
	if (!state && m_ce)
	{
		// d53: DP (drive mode select, aka duty)
		// d54: DQ (AD/DSP function)
		// d55: commons
		if (!BIT(m_shift, 55))
		{
			m_duty = BIT(m_shift, 53);
			m_addsp = BIT(m_shift, 54);
		}

		m_latch[BIT(m_shift, 55)] = m_shift & u64(0x1f'ffff'ffff'ffff);
		refresh_output();
	}

	m_ce = state;
}
