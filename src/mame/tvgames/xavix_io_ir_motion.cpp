// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Paired XaviX infrared motion controllers. Paddles, rackets, kick sensors,
    shakers and swords share an eight-bit player/button/strength report table.
    The configured controls feed the report encoder; the selected profile
    sets original or later receiver timing. The child report generator
    owns timed transmission and its saved state.

    TODO:
    - Measure physical motion thresholds and transmitter repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ir_motion.h"

DEFINE_DEVICE_TYPE(XAVIX_IR_MOTION, xavix_ir_motion_device, "xavix_ir_motion", "XaviX Infrared Motion Controllers")

xavix_ir_motion_device::xavix_ir_motion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_IR_MOTION, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_ir_strength(*this, "IR%u", 1U)
	, m_ir_button(*this, "IR%u_BUTTON", 1U)
	, m_ir_power(*this, "IR_POWER")
	, m_controller_type(controller_type::PADDLES)
{
}

static INPUT_PORTS_START( xavix_ir_motion_swing )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	// Original Ping Pong rackets are swing-only; games with a physical button override these report bits.
	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_paddles )
	PORT_INCLUDE(xavix_ir_motion_swing)

	PORT_MODIFY("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Paddle Button") PORT_PLAYER(1)

	PORT_MODIFY("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Paddle Button") PORT_PLAYER(2)
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_ping_pong_rackets )
	PORT_INCLUDE(xavix_ir_motion_swing)

	PORT_MODIFY("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Red Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)

	PORT_MODIFY("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Blue Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_powered_ping_pong_rackets )
	PORT_INCLUDE(xavix_ir_motion_ping_pong_rackets)

	PORT_MODIFY("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Red Racket Button") PORT_PLAYER(1)

	PORT_MODIFY("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Blue Racket Button") PORT_PLAYER(2)

	PORT_START("IR_POWER")
	// Each later racket has a maintained side switch that suppresses its wireless report while off.
	PORT_CONFNAME( 0x01, 0x01, "P1 Red Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "P2 Blue Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_kick_sensors )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Kick Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Kick Sensor Start / Confirm") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Kick Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Kick Sensor Start / Confirm") PORT_CODE(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR_POWER")
	PORT_CONFNAME( 0x01, 0x01, "P1 Kick Sensor Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "P2 Kick Sensor Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_tennis_rackets )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Racket Button / Confirm") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Racket Button / Confirm") PORT_CODE(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR_POWER")
	PORT_CONFNAME( 0x01, 0x01, "P1 Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "P2 Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_shakers )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Shake Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Confirm Button") PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Shake Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Confirm Button") PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_swords )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Blue Sword Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Blue Sword Trigger") PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Red Sword Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Red Sword Trigger") PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( xavix_ir_motion_xavix_rackets )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Racket Button") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Racket Button") PORT_CODE(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_ir_motion_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->report_callback<0>().set(*this, FUNC(xavix_ir_motion_device::ir_report_r<0>));
	m_ir->report_callback<1>().set(*this, FUNC(xavix_ir_motion_device::ir_report_r<1>));
	m_ir->report_enable_callback<0>().set(*this, FUNC(xavix_ir_motion_device::ir_report_enabled_r<0>));
	m_ir->report_enable_callback<1>().set(*this, FUNC(xavix_ir_motion_device::ir_report_enabled_r<1>));
}

void xavix_ir_motion_device::device_config_complete()
{
	bool const original = (m_controller_type == controller_type::PADDLES)
		|| (m_controller_type == controller_type::PING_PONG_RACKETS)
		|| (m_controller_type == controller_type::SHAKERS);
	// Both receivers sample within the first cell, then at uniform timer intervals.
	m_ir.lookup()->set_bit_ticks(original ? 0x47 * 64 : 0xe5 * 32);
}

void xavix_ir_motion_device::set_input_source(device_t &source)
{
	for (auto &port : m_ir_strength)
		port.set_tag(source, port.finder_tag());
	for (auto &port : m_ir_button)
		port.set_tag(source, port.finder_tag());
	m_ir_power.set_tag(source, m_ir_power.finder_tag());
}

void xavix_ir_motion_device::device_start()
{
}

void xavix_ir_motion_device::vblank_w(int state)
{
	m_ir->vblank_w(state);
}

template <unsigned Player>
u16 xavix_ir_motion_device::ir_report_r()
{
	u8 const level = m_ir_strength[Player]->read() & 3;
	return xavix_ir_report_device::motion_report(Player, BIT(m_ir_button[Player]->read(), 0), level);
}

template <unsigned Player>
int xavix_ir_motion_device::ir_report_enabled_r()
{
	return BIT(m_ir_power.read_safe(3), Player);
}
