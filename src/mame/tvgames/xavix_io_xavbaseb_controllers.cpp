// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Excite Stadium DX and XaviX Baseball infrared ball and bat. Both
    controllers send the ball state and its complement as two seven-bit
    halves, followed by the bat report. XaviX Baseball interleaves the ball
    bits; Excite Stadium DX also has independent transmitter power switches.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_xavbaseb_controllers.h"

DEFINE_DEVICE_TYPE(XAVIX_BASEBALL_CONTROLLERS, xavix_baseball_controllers_device, "xavix_baseball_controllers", "XaviX Infrared Baseball Controllers")

xavix_baseball_controllers_device::xavix_baseball_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_framed_device(mconfig, XAVIX_BASEBALL_CONTROLLERS, tag, owner, clock)
	, m_ball_throw(*this, "BALL_THROW")
	, m_ball_buttons(*this, "BALL_BUTTONS")
	, m_bat(*this, "BAT")
	, m_controller_power(*this, "CONTROLLER_POWER")
	, m_controller_type(controller_type::BASEBALL)
{
	set_report_count(2);
	set_bit_ticks(0x57 * 64);
	// The receiver samples after 1.53 data cells, requiring the transmitter's extended start pulse.
	set_start_ticks(0x83 * 64);
	set_report_bits<0>(15);
	set_report_bits<1>(8);
}

static INPUT_PORTS_START( xavix_baseball_controllers )
	PORT_START("BALL_THROW")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("Ball Throw Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BALL_BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Ball Leftmost / Cancel") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ball Inner Left / Menu Left") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ball Inner Right / Menu Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Ball Rightmost / Enter") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xc3, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BAT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bat Bunt") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bat Swing") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( xavix_esdx_controllers )
	PORT_INCLUDE(xavix_baseball_controllers)

	PORT_MODIFY("BALL_BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Ball Leftmost") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ball Inner Left") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ball Inner Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Ball Rightmost") PORT_CODE(KEYCODE_F)

	PORT_START("CONTROLLER_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Wireless Ball Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "Wireless Bat Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_baseball_controllers_device::device_input_ports() const
{
	return m_controller_type == controller_type::STADIUM_DX ? INPUT_PORTS_NAME(xavix_esdx_controllers) : INPUT_PORTS_NAME(xavix_baseball_controllers);
}

u32 xavix_baseball_controllers_device::read_report(unsigned report)
{
	if (report)
	{
		u8 const controls = m_bat->read();
		return baseball_bat_report(BIT(controls, 0), BIT(controls, 1));
	}

	u8 const value = (m_ball_throw->read() & 0x03) | (m_ball_buttons->read() & 0x3c);
	u8 const complement = value ^ 0x7f;
	if (m_controller_type == controller_type::STADIUM_DX)
		return (u16(value) << 8) | (u16(complement) << 1);

	// XaviX Baseball interleaves the six-bit state with its complement across two seven-bit halves.
	u8 const first = ((value & 0x07) << 3) | (complement & 0x47);
	u8 const second = (value & 0x38) | ((complement >> 3) & 0x07);
	return (u16(second) << 8) | (u16(first) << 1);
}

bool xavix_baseball_controllers_device::report_enabled(unsigned report)
{
	return BIT(m_controller_power.read_safe(3), report);
}

u16 xavix_baseball_controllers_device::baseball_bat_report(bool bunt, bool swing)
{
	// Accepted state 3 has no known physical meaning; a swing temporarily takes priority over a held bunt button.
	u8 const state = swing ? 2 : bunt;
	return 1 | (u16(BAT_PAYLOADS[state]) << 1);
}
