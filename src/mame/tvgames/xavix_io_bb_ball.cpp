// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX and Radica baseball pitching-ball inputs. Converts directional
    controls and face buttons to shared analogue input levels, and represents
    pitch gestures with representative two-field motion pulses.

    TODO:
    - Measure motion voltage levels, waveform and pulse shape on an
      original ball.

***************************************************************************/

#include "emu.h"
#include "xavix_io_bb_ball.h"

DEFINE_DEVICE_TYPE(XAVIX_BASEBALL_BALL, xavix_baseball_ball_device, "xavix_baseball_ball", "XaviX / Radica Baseball Pitching Ball")

xavix_baseball_ball_device::xavix_baseball_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_BASEBALL_BALL, tag, owner, clock)
	, m_controls(*this, "PITCH")
	, m_motion(*this, "PITCH_MOTION")
{
}

void xavix_baseball_ball_device::set_input_source(device_t &source)
{
	m_controls.set_tag(source, m_controls.finder_tag());
	m_motion.set_tag(source, m_motion.finder_tag());
}

void xavix_baseball_ball_device::device_start()
{
	save_item(NAME(m_motion_level));
	save_item(NAME(m_motion_fields_remaining));
	save_item(NAME(m_motion_armed));
}

void xavix_baseball_ball_device::device_reset()
{
	m_motion_level = 0;
	m_motion_fields_remaining = 0;
	m_motion_armed = !(m_motion->read() & 3);
}

u8 xavix_baseball_ball_device::vertical_buttons_r()
{
	u8 const controls = m_controls->read();

	// Each direction axis shares an ADC resistor ladder with its adjacent face button.
	if (BIT(controls, 4))
		return 0xff;
	if (BIT(controls, 1))
		return 0x40;
	if (BIT(controls, 0))
		return 0xc0;
	return 0x00;
}

u8 xavix_baseball_ball_device::horizontal_buttons_r()
{
	u8 const controls = m_controls->read();

	if (BIT(controls, 5))
		return 0xff;
	if (BIT(controls, 3))
		return 0x40;
	if (BIT(controls, 2))
		return 0xc0;
	return 0x00;
}

u8 xavix_baseball_ball_device::motion_r()
{
	// These representative voltages preserve all four ROM states; the real motion waveform is not yet measured.
	return MOTION_LEVELS[m_motion_level];
}

void xavix_baseball_ball_device::vblank_w(int state)
{
	if (!state)
		return;

	if (m_motion_fields_remaining && !--m_motion_fields_remaining)
		m_motion_level = 0;

	u8 const motion = m_motion->read() & 3;
	if (!motion)
	{
		m_motion_armed = true;
	}
	else if (m_motion_armed)
	{
		// Approximate each throw with one two-field pulse; returning to rest rearms the next gesture.
		m_motion_level = motion;
		m_motion_fields_remaining = 2;
		m_motion_armed = false;
	}
}
