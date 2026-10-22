// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX and Radica baseball pitching-ball inputs. Converts directional
    controls and face buttons to shared analogue input levels, and represents
    pitch strength with representative motion voltages.

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
	, m_model(model::RAD_BB)
{
}

static INPUT_PORTS_START( xavix_baseball_ball )
	PORT_START("PITCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_NAME("Pitch Controller Up / Fastball") PORT_8WAY PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_NAME("Pitch Controller Down / Change-up") PORT_8WAY PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("Pitch Controller Left / Knuckleball") PORT_8WAY PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("Pitch Controller Right / Curveball") PORT_8WAY PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pitch Controller New") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Pitch Controller Enter") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("PITCH_MOTION")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("Pitch Motion Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(3) PORT_CENTERDELTA(3) PORT_CODE_INC(KEYCODE_G)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_baseball_ball2 )
	PORT_INCLUDE(xavix_baseball_ball)

	PORT_MODIFY("PITCH")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Pitch Controller Back / X") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Pitch Controller Next / O") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END

ioport_constructor xavix_baseball_ball_device::device_input_ports() const
{
	return m_model == model::RAD_BB ? INPUT_PORTS_NAME(xavix_baseball_ball) : INPUT_PORTS_NAME(xavix_baseball_ball2);
}

void xavix_baseball_ball_device::device_start()
{
	save_item(NAME(m_motion_level));
}

void xavix_baseball_ball_device::device_reset()
{
	m_motion_level = 0;
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

	m_motion_level = m_motion->read() & 3;
}
