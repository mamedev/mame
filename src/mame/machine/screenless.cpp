// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Generic screenless base class

This file contains helpers for strobed display elements.

TODO:
- use pwm_display_device, remove this file

******************************************************************************/

#include "emu.h"
#include "includes/screenless.h"

#include <algorithm>


// machine start

ALLOW_SAVE_TYPE(attotime); // m_ds_acc

void screenless_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// initialize
	std::fill_n(m_display_state, ARRAY_LENGTH(m_display_state), 0);
	std::fill_n(m_ds_prev, ARRAY_LENGTH(m_ds_prev), 0);
	std::fill_n(m_display_segmask, ARRAY_LENGTH(m_display_segmask), 0);
	std::fill_n(*m_ds_bri, ARRAY_LENGTH(m_ds_bri) * ARRAY_LENGTH(m_ds_bri[0]), 0.0);

	m_display_frame_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(screenless_state::display_frame),this));
	display_schedule_frame();
	m_ds_update_time = machine().time();

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_state));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_ds_prev));
	save_item(NAME(m_ds_bri));
	save_item(NAME(m_ds_acc));
	save_item(NAME(m_ds_update_time));
	save_item(NAME(m_ds_frame_time));
	save_item(NAME(m_ds_frame_time_set));
	save_item(NAME(m_ds_frame_factor));
	save_item(NAME(m_ds_level));
}


// public interface

void screenless_state::set_display_duration(attotime duration)
{
	// set frame duration
	m_ds_frame_time_set = duration;
}

void screenless_state::set_display_factor(double factor)
{
	// set frame interpolation (factor * curframe, 1.0-factor * prev frame)
	// factor range is 0.0 to 1.0
	m_ds_frame_factor = factor;
}

void screenless_state::reset_display_levels()
{
	std::fill_n(m_ds_level, ARRAY_LENGTH(m_ds_level), 1.0);
}

void screenless_state::set_display_level(u8 i, double level)
{
	// set a brightness level, range is 0.0 to 1.0
	m_ds_level[i] = level;
}

void screenless_state::set_display_levels(double l0, double l1, double l2, double l3)
{
	// init brightness level(s) (if you need to set more than 4, use set_display_level)
	reset_display_levels();
	m_ds_level[0] = l0;
	m_ds_level[1] = l1;
	m_ds_level[2] = l2;
	m_ds_level[3] = l3;
}

void screenless_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void screenless_state::set_display_segmask(u32 digits, u32 mask)
{
	// set a segment mask per selected digit, but leave unselected ones alone
	for (int i = 0; i < 0x20; i++)
	{
		if (digits & 1)
			m_display_segmask[i] = mask;
		digits >>= 1;
	}
}

void screenless_state::display_matrix(int maxx, int maxy, u64 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u64 mask = (u64(1) << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (u64(1) << maxx)) : 0;

	if (update)
		display_update();
}

void screenless_state::display_update()
{
	// call this every time m_display_state is changed (automatic with display_matrix)
	const attotime now = machine().time();
	const attotime diff = (m_ds_update_time >= now) ? attotime::zero : now - m_ds_update_time;

	// accumulate active time
	for (int y = 0; y < m_display_maxy; y++)
	{
		u64 row = m_ds_prev[y];
		m_ds_prev[y] = m_display_state[y];

		if (diff != attotime::zero)
		{
			for (int x = 0; x <= m_display_maxx; x++)
			{
				if (BIT(row, x))
					m_ds_acc[y][x] += diff;
			}
		}
	}

	m_ds_update_time = now;
}

bool screenless_state::display_element_on(u32 x, u32 y)
{
	// display element active state
	return m_ds_bri[y][x] > m_ds_level[0];
}


// internal handlers

void screenless_state::display_schedule_frame()
{
	std::fill_n(*m_ds_acc, ARRAY_LENGTH(m_ds_acc) * ARRAY_LENGTH(m_ds_acc[0]), attotime::zero);

	m_ds_frame_time = m_ds_frame_time_set;
	m_display_frame_timer->adjust(m_ds_frame_time);
}

TIMER_CALLBACK_MEMBER(screenless_state::display_frame)
{
	display_update();

	const double frame_time = m_ds_frame_time.as_double();
	const double factor0 = m_ds_frame_factor;
	const double factor1 = 1.0 - factor0;

	for (int y = 0; y < m_display_maxy; y++)
	{
		u64 row = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// determine brightness level
			double bri = m_ds_bri[y][x] * factor1 + (m_ds_acc[y][x].as_double() / frame_time) * factor0;
			if (bri > 1.0) bri = 1.0; // shouldn't happen
			m_ds_bri[y][x] = bri;

			u8 level;
			for (level = 0; bri > m_ds_level[level]; level++) { ; }

			if (level > 0)
				row |= (u64(1) << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = level;
			else
				m_out_a[y] = level;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = row & m_display_segmask[y];
	}

	display_schedule_frame();
}
