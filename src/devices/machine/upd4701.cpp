// license:BSD-3-Clause
// copyright-holders:smf,AJR
/***************************************************************************

    NEC µPD4701A 2-Axis Incremental Encoder Counter

***************************************************************************/

#include "emu.h"
#include "upd4701.h"

#define MASK_COUNTER ( 0xfff )

DEFINE_DEVICE_TYPE(UPD4701A, upd4701_device, "upd4701a", "uPD4701A Incremental Encoder")

upd4701_device::upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, UPD4701A, tag, owner, clock)
	, m_cs(true)
	, m_xy(false)
	, m_ul(false)
	, m_resetx(false)
	, m_resety(false)
	, m_portx(*this, finder_base::DUMMY_TAG)
	, m_porty(*this, finder_base::DUMMY_TAG)
	, m_latchx(0)
	, m_latchy(0)
	, m_startx(0)
	, m_starty(0)
	, m_x(0)
	, m_y(0)
	, m_switches(0)
	, m_latchswitches(0)
	, m_cf(true)
	, m_cf_cb(*this)
	, m_sf_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd4701_device::device_start()
{
	// resolve callbacks
	m_cf_cb.resolve_safe();
	m_sf_cb.resolve_safe();

	// register state for saving
	save_item(NAME(m_cs));
	save_item(NAME(m_xy));
	save_item(NAME(m_ul));
	save_item(NAME(m_resetx));
	save_item(NAME(m_resety));
	save_item(NAME(m_latchx));
	save_item(NAME(m_latchy));
	save_item(NAME(m_startx));
	save_item(NAME(m_starty));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_switches));
	save_item(NAME(m_latchswitches));
	save_item(NAME(m_cf));

	// register special callback for analog inputs
	if (m_portx.found() || m_porty.found())
		machine().add_notifier(MACHINE_NOTIFY_FRAME, machine_notify_delegate(&upd4701_device::analog_update, this));
}

//-------------------------------------------------
//  ul_w - write to counter select line
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::ul_w)
{
	m_ul = state;
}

//-------------------------------------------------
//  xy_w - write to byte select line
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::xy_w)
{
	m_xy = state;
}

//-------------------------------------------------
//  cs_w - write to chip select line
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::cs_w)
{
	if (m_cs != state)
	{
		m_cs = state;

		if (!m_cs)
		{
			m_latchx = (m_x - m_startx) & MASK_COUNTER;
			m_latchy = (m_y - m_starty) & MASK_COUNTER;

			m_latchswitches = m_switches;
			if (m_switches != 0)
				m_latchswitches |= 8;

			if (!m_cf)
			{
				// CF remains inactive while CS is low
				m_cf = true;
				m_cf_cb(1);
			}
		}
	}
}

//-------------------------------------------------
//  resetx_w - write to X counter reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::resetx_w)
{
	if (m_resetx != state)
	{
		m_resetx = state;

		if (m_resetx)
			m_startx = m_x;
	}
}

//-------------------------------------------------
//  resety_w - write to Y counter reset line
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::resety_w)
{
	if (m_resety != state)
	{
		m_resety = state;

		if (m_resety)
			m_starty = m_y;
	}
}

//-------------------------------------------------
//  reset_x - pulse the X counter reset line
//-------------------------------------------------

READ8_MEMBER(upd4701_device::reset_x_r)
{
	if (!machine().side_effects_disabled())
	{
		resetx_w(1);
		resetx_w(0);
	}
	return space.unmap();
}

WRITE8_MEMBER(upd4701_device::reset_x_w)
{
	resetx_w(1);
	resetx_w(0);
}

//-------------------------------------------------
//  reset_y - pulse the Y counter reset line
//-------------------------------------------------

READ8_MEMBER(upd4701_device::reset_y_r)
{
	if (!machine().side_effects_disabled())
	{
		resety_w(1);
		resety_w(0);
	}
	return space.unmap();
}

WRITE8_MEMBER(upd4701_device::reset_y_w)
{
	resety_w(1);
	resety_w(0);
}

//-------------------------------------------------
//  reset_xy - pulse the counter reset lines
//-------------------------------------------------

READ8_MEMBER(upd4701_device::reset_xy_r)
{
	if (!machine().side_effects_disabled())
	{
		resetx_w(1);
		resety_w(1);
		resetx_w(0);
		resety_w(0);
	}
	return space.unmap();
}

WRITE8_MEMBER(upd4701_device::reset_xy_w)
{
	resetx_w(1);
	resety_w(1);
	resetx_w(0);
	resety_w(0);
}

//-------------------------------------------------
//  analog_update - per-frame input update
//-------------------------------------------------

void upd4701_device::analog_update()
{
	if (m_portx.found())
		x_add(m_portx->read() & MASK_COUNTER);
	if (m_porty.found())
		y_add(m_porty->read() & MASK_COUNTER);
}

//-------------------------------------------------
//  x_add - count X-axis input
//-------------------------------------------------

void upd4701_device::x_add(s16 data)
{
	if (!m_resetx && data != 0)
	{
		m_x += data;

		if (m_cs && m_cf)
		{
			m_cf = false;
			m_cf_cb(0);
		}
	}
}

//-------------------------------------------------
//  y_add - count Y-axis input
//-------------------------------------------------

void upd4701_device::y_add(s16 data)
{
	if (!m_resety && data != 0)
	{
		m_y += data;

		if (m_cs && m_cf)
		{
			m_cf = false;
			m_cf_cb(0);
		}
	}
}

//-------------------------------------------------
//  switch_update - update one of three switches
//-------------------------------------------------

void upd4701_device::switch_update(u8 mask, bool state)
{
	if (!state && (m_switches & mask) == 0)
	{
		// active low
		m_switches |= mask;

		// update SF output if other switches were not active
		if ((m_switches & ~mask) == 0)
			m_sf_cb(0);
	}
	else if (state && (m_switches & mask) == mask)
	{
		// inactive high
		m_switches &= ~mask;

		// update SF output if other switches are also inactive
		if ((m_switches & ~mask) == 0)
			m_sf_cb(1);
	}
}

//-------------------------------------------------
//  left_w - update left switch state
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::left_w)
{
	switch_update(4, state);
}

//-------------------------------------------------
//  right_w - update right switch state
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::right_w)
{
	switch_update(2, state);
}

//-------------------------------------------------
//  middle_w - update middle switch state
//-------------------------------------------------

WRITE_LINE_MEMBER(upd4701_device::middle_w)
{
	switch_update(1, state);
}

//-------------------------------------------------
//  d_r - read data lines directly
//-------------------------------------------------

READ8_MEMBER(upd4701_device::d_r)
{
	if (m_cs)
	{
		logerror("Read while CS inactive\n");
		return space.unmap();
	}

	u16 data = m_xy ? m_latchy : m_latchx;
	data |= m_latchswitches << 12;

	if (m_ul)
		return data >> 8;
	else
		return data & 0xff;
}

//-------------------------------------------------
//  read_x - read X axis through data/address bus
//-------------------------------------------------

READ8_MEMBER(upd4701_device::read_x)
{
	return read_xy(space, (offset & 1) | 0);
}

//-------------------------------------------------
//  read_y - read Y axis through data/address bus
//-------------------------------------------------

READ8_MEMBER(upd4701_device::read_y)
{
	return read_xy(space, (offset & 1) | 2);
}

//-------------------------------------------------
//  read_xy - read either axis through bus
//-------------------------------------------------

READ8_MEMBER(upd4701_device::read_xy)
{
	bool old_cs = m_cs;
	cs_w(0);
	xy_w(BIT(offset, 1));
	ul_w(BIT(offset, 0));
	u8 result = d_r(space, 0);
	cs_w(old_cs);
	return result;
}

//-------------------------------------------------
//  sf_r - read switch flag
//-------------------------------------------------

READ_LINE_MEMBER(upd4701_device::sf_r)
{
	if (m_switches != 0)
		return 0;

	return 1;
}

//-------------------------------------------------
//  cf_r - read counter flag
//-------------------------------------------------

READ_LINE_MEMBER(upd4701_device::cf_r)
{
	return m_cf;
}
