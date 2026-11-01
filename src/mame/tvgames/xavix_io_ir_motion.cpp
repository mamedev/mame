// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Paired XaviX infrared motion controllers. Paddles, rackets, kick sensors,
    shakers and swords share an eight-bit player/button/strength report table.
    The configured controls feed the report encoder; the selected profile
    sets original or later receiver timing. The common transmitter base
    owns timed transmission and its saved state.

    TODO:
    - Measure physical motion thresholds and transmitter repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ir_motion.h"

DEFINE_DEVICE_TYPE(XAVIX_IR_MOTION, xavix_ir_motion_device, "xavix_ir_motion", "XaviX Infrared Motion Controllers")

xavix_ir_motion_base_device::xavix_ir_motion_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_msb_device(mconfig, type, tag, owner, clock)
{
	set_report_count(2);
	set_bit_ticks(0xe5 * 32);
}

u8 xavix_ir_motion_base_device::motion_report(unsigned player, bool button, u8 strength)
{
	assert(player < 2);
	return MOTION_REPORTS[player][button][strength & 3];
}

xavix_ir_motion_device::xavix_ir_motion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_motion_base_device(mconfig, XAVIX_IR_MOTION, tag, owner, clock)
	, m_ir_strength(*this, "IR%u", 1U)
	, m_ir_button(*this, "IR%u_BUTTON", 1U)
	, m_ir_power(*this, "IR_POWER")
	, m_controller_type(controller_type::PADDLES)
{
}

// Keep host bindings independent of MAME's automatic per-device player numbering.
static INPUT_PORTS_START( xavix_ir_motion_swing )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	// Original Ping Pong rackets are swing-only; games with a physical button override these report bits.
	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir_motion_paddles )
	PORT_INCLUDE(xavix_ir_motion_swing)

	PORT_MODIFY("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Paddle Button") PORT_PLAYER(1) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))

	PORT_MODIFY("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Paddle Button") PORT_PLAYER(2) PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir_motion_ping_pong_rackets )
	PORT_INCLUDE(xavix_ir_motion_swing)

	PORT_MODIFY("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Red Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)

	PORT_MODIFY("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Blue Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir_motion_powered_ping_pong_rackets )
	PORT_INCLUDE(xavix_ir_motion_ping_pong_rackets)

	PORT_MODIFY("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Red Racket Button") PORT_PLAYER(1) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))

	PORT_MODIFY("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Blue Racket Button") PORT_PLAYER(2) PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))

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

static INPUT_PORTS_START( xavix_ir_motion_kick_sensors )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Kick Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Kick Sensor Start / Confirm") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Kick Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
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

static INPUT_PORTS_START( xavix_ir_motion_tennis_rackets )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Racket Button / Confirm") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
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

static INPUT_PORTS_START( xavix_ir_motion_shakers )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Shake Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Confirm Button") PORT_PLAYER(1) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Shake Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Confirm Button") PORT_PLAYER(2) PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir_motion_swords )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Blue Sword Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CONDITION("IR_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Blue Sword Trigger") PORT_PLAYER(1) PORT_CONDITION("IR_POWER", 0x01, EQUALS, 0x01)
	// The station reserves this report for factory entry; its physical control/fixture is unidentified.
	// PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("P1 Blue Sword Factory Test Transmission") PORT_CODE(KEYCODE_F1) PORT_CONDITION("IR_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Red Sword Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CONDITION("IR_POWER", 0x02, EQUALS, 0x02)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Red Sword Trigger") PORT_PLAYER(2) PORT_CONDITION("IR_POWER", 0x02, EQUALS, 0x02)
	// PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("P2 Red Sword Factory Test Transmission") PORT_CODE(KEYCODE_F1) PORT_CONDITION("IR_POWER", 0x02, EQUALS, 0x02)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR_POWER")
	PORT_CONFNAME( 0x01, 0x01, "P1 Blue Sword Batteries" )
	PORT_CONFSETTING(    0x00, "Removed" )
	PORT_CONFSETTING(    0x01, "Installed" )
	PORT_CONFNAME( 0x02, 0x02, "P2 Red Sword Batteries" )
	PORT_CONFSETTING(    0x00, "Removed" )
	PORT_CONFSETTING(    0x02, "Installed" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_ir_motion_xavix_rackets )
	PORT_START("IR1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR1_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Racket Button") PORT_CODE(KEYCODE_Z) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR2_BUTTON")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Racket Button") PORT_CODE(KEYCODE_M) PORT_PLAYER(2)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IR_POWER")
	// Removing a racket's batteries silences its transmitter during the EEPROM test.
	PORT_CONFNAME( 0x01, 0x01, "P1 Racket Batteries" )
	PORT_CONFSETTING(    0x00, "Removed" )
	PORT_CONFSETTING(    0x01, "Installed" )
	PORT_CONFNAME( 0x02, 0x02, "P2 Racket Batteries" )
	PORT_CONFSETTING(    0x00, "Removed" )
	PORT_CONFSETTING(    0x02, "Installed" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_ir_motion_device::device_config_complete()
{
	bool const original = (m_controller_type == controller_type::PADDLES)
		|| (m_controller_type == controller_type::PING_PONG_RACKETS)
		|| (m_controller_type == controller_type::SHAKERS);
	// Both receivers sample within the first cell, then at uniform timer intervals.
	set_bit_ticks(original ? 0x47 * 64 : 0xe5 * 32);
}

ioport_constructor xavix_ir_motion_device::device_input_ports() const
{
	switch (m_controller_type)
	{
	case controller_type::PADDLES:
		return INPUT_PORTS_NAME(xavix_ir_motion_paddles);
	case controller_type::PING_PONG_RACKETS:
		return INPUT_PORTS_NAME(xavix_ir_motion_ping_pong_rackets);
	case controller_type::POWERED_PING_PONG_RACKETS:
		return INPUT_PORTS_NAME(xavix_ir_motion_powered_ping_pong_rackets);
	case controller_type::KICK_SENSORS:
		return INPUT_PORTS_NAME(xavix_ir_motion_kick_sensors);
	case controller_type::TENNIS_RACKETS:
		return INPUT_PORTS_NAME(xavix_ir_motion_tennis_rackets);
	case controller_type::SHAKERS:
		return INPUT_PORTS_NAME(xavix_ir_motion_shakers);
	case controller_type::SWORDS:
		return INPUT_PORTS_NAME(xavix_ir_motion_swords);
	case controller_type::XAVIX_RACKETS:
		return INPUT_PORTS_NAME(xavix_ir_motion_xavix_rackets);
	}
	throw emu_fatalerror("Unsupported infrared controller type");
}

u32 xavix_ir_motion_device::read_report(unsigned report)
{
	u8 const level = m_ir_strength[report]->read() & 3;
	u8 const buttons = m_ir_button[report]->read();
	if (m_controller_type == controller_type::SWORDS && BIT(buttons, 1))
		return SWORD_FACTORY_REPORTS[report];
	return motion_report(report, BIT(buttons, 0), level);
}

bool xavix_ir_motion_device::report_enabled(unsigned report)
{
	return BIT(m_ir_power.read_safe(3), report);
}
