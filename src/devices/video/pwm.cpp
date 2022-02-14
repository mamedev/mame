// license:BSD-3-Clause
// copyright-holders:hap
/*

This thing is a generic helper for PWM(strobed) display elements, to prevent
flickering and optionally handle perceived brightness levels.

Common usecase is to call matrix(selmask, datamask), a collision between the
2 masks implies a powered-on display element (eg. a LED, or VFD sprite).
The maximum matrix size is 64 by 64, simply due to uint64_t constraints.
If a larger size is needed, create an array of pwm_display_device.

If display elements are directly addressable, you can also use write_element
or write_row to set them.

Display element states are sent to output tags "y.x" where y is the matrix row
number, x is the row bit. It is also sent to "y.a" for all rows. The output state
is 0 for off, and >0 for on, depending on brightness level. If segmask is defined,
it is also sent to "digity", for use with multi-state elements, eg. 7seg leds.

If you use this device in a slot, or use multiple of them (or just don't want
to use the default output tags), set a callback.

Brightness tresholds (0.0 to 1.0) indicate how long an element was powered on
in the last frame, eg. 0.01 means a minimum on-time for 1%. Some games use two
levels of brightness by strobing elements longer.


TODO:
- SVG screens and rendlay digit elements don't support multiple brightness levels

*/

#include "emu.h"
#include "video/pwm.h"

#include <algorithm>


DEFINE_DEVICE_TYPE(PWM_DISPLAY, pwm_display_device, "pwm_display", "PWM Display")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

pwm_display_device::pwm_display_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PWM_DISPLAY, tag, owner, clock),
	m_out_x(*this, "%u.%u", 0U, 0U),
	m_out_a(*this, "%u.a", 0U),
	m_out_digit(*this, "digit%u", 0U),
	m_output_x_cb(*this),
	m_output_a_cb(*this),
	m_output_digit_cb(*this)
{
	// set defaults
	set_refresh(attotime::from_hz(60));
	set_interpolation(0.5);
	set_bri_levels(0.01);
	set_bri_minimum(0);
	set_bri_maximum(0.0);
	set_size(0, 0);
	reset_segmask();
}


//-------------------------------------------------
//  device_start/reset
//-------------------------------------------------

void pwm_display_device::device_start()
{
	// resolve handlers
	m_external_output = !m_output_x_cb.isnull() || !m_output_a_cb.isnull() || !m_output_digit_cb.isnull();
	m_output_x_cb.resolve_safe();
	m_output_a_cb.resolve_safe();
	m_output_digit_cb.resolve_safe();

	if (!m_external_output)
	{
		m_out_x.resolve();
		m_out_a.resolve();
		m_out_digit.resolve();
	}

	// initialize
	m_rowsel = 0;
	m_rowdata_last = 0;
	std::fill(std::begin(m_rowdata), std::end(m_rowdata), 0);

	for (auto &bri : m_bri)
		std::fill(std::begin(bri), std::end(bri), 0.0);

	m_frame_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pwm_display_device::frame_tick),this));
	m_sync_time = machine().time();

	// register for savestates
	save_item(NAME(m_width));
	save_item(NAME(m_height));
	save_item(NAME(m_framerate_set));
	save_item(NAME(m_framerate));
	save_item(NAME(m_interpolation));
	save_item(NAME(m_levels));
	save_item(NAME(m_level_min));
	save_item(NAME(m_level_max));

	save_item(NAME(m_segmask));
	save_item(NAME(m_rowsel));
	save_item(NAME(m_rowdata));
	save_item(NAME(m_rowdata_last));

	save_item(NAME(m_bri));
	save_item(NAME(m_sync_time));
	save_item(NAME(m_acc));
}

void pwm_display_device::device_reset()
{
	if (m_height > 64 || m_width > 64)
		fatalerror("%s: Invalid size %d*%d, maximum is 64*64!\n", tag(), m_height, m_width);

	schedule_frame();
	m_sync_time = machine().time();
}


//-------------------------------------------------
//  public handlers (most of the interface is in the .h file)
//-------------------------------------------------

pwm_display_device &pwm_display_device::set_bri_levels(double l0, double l1, double l2, double l3)
{
	// init brightness level(s) (if you need to set more than 4, use set_bri_one)
	reset_bri_levels();
	set_bri_one(0, l0);
	set_bri_one(1, l1);
	set_bri_one(2, l2);
	set_bri_one(3, l3);

	return *this;
}

pwm_display_device &pwm_display_device::set_segmask(u64 digits, u64 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int y = 0; y < m_height; y++)
	{
		if (digits & 1)
			m_segmask[y] = mask;
		digits >>= 1;
	}

	return *this;
}

void pwm_display_device::matrix_partial(u8 start, u8 height, u64 rowsel, u64 rowdata)
{
	sync();

	u64 selmask = (u64(1) << height) - 1;
	rowsel &= selmask;
	selmask <<= start;
	m_rowsel = (m_rowsel & ~selmask) | (rowsel << start);

	// update selected rows
	u64 rowmask = (u64(1) << m_width) - 1;
	m_rowdata_last = rowdata & rowmask;
	for (int y = start; y < (start + height) && y < m_height; y++)
	{
		m_rowdata[y] = (rowsel & 1) ? (rowdata & rowmask) : 0;
		rowsel >>= 1;
	}
}


//-------------------------------------------------
//  internal handlers
//-------------------------------------------------

void pwm_display_device::schedule_frame()
{
	std::fill_n(*m_acc, m_height * std::size(m_acc[0]), attotime::zero);

	m_framerate = m_framerate_set;
	m_frame_timer->adjust(m_framerate);
}

TIMER_CALLBACK_MEMBER(pwm_display_device::frame_tick)
{
	const double frame_time = m_framerate.as_double();
	const double factor0 = m_interpolation;
	const double factor1 = 1.0 - factor0;

	// determine brightness cutoff
	double cutoff = m_level_max;
	if (cutoff == 0.0)
	{
		u8 level;
		for (level = 1; m_levels[level] < 1.0; level++) { ; }
		cutoff = 4 * m_levels[level - 1];
	}
	if (cutoff > 1.0)
		cutoff = 1.0;

	sync(); // final timeslice

	for (int y = 0; y < m_height; y++)
	{
		u64 row = 0;

		for (int x = 0; x <= m_width; x++)
		{
			// determine brightness level
			double bri = m_bri[y][x] * factor1 + (m_acc[y][x].as_double() / frame_time) * factor0;
			if (bri > cutoff)
				bri = cutoff;
			m_bri[y][x] = bri;

			u8 level;
			for (level = 0; bri > m_levels[level]; level++) { ; }

			// output to y.x, or y.a when always-on
			if (x != m_width)
			{
				if (level > m_level_min)
					row |= (u64(1) << x);

				if (m_external_output)
					m_output_x_cb(x << 6 | y, level);
				else
					m_out_x[y][x] = level;
			}
			else
			{
				if (m_external_output)
					m_output_a_cb(y, level);
				else
					m_out_a[y] = level;
			}
		}

		// output to digity (does not support multiple brightness levels)
		if (m_segmask[y] != 0)
		{
			row &= m_segmask[y];

			if (m_external_output)
				m_output_digit_cb(y, row);
			else
				m_out_digit[y] = row;
		}
	}

	schedule_frame();
}

void pwm_display_device::sync()
{
	const attotime now = machine().time();
	const attotime last = m_sync_time;
	m_sync_time = now;

	if (last >= now)
		return;

	const attotime diff = now - last;
	u64 sel = m_rowsel;

	// accumulate active time
	for (int y = 0; y < m_height; y++)
	{
		u64 row = m_rowdata[y];

		if (sel & 1)
			m_acc[y][m_width] += diff;

		for (int x = 0; x < m_width; x++)
		{
			if (row & 1)
				m_acc[y][x] += diff;
			row >>= 1;
		}
		sel >>= 1;
	}
}
