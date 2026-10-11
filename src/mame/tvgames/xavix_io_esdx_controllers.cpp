// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Excite Stadium DX and Hanshin Tigers infrared ball and bat. Reads the
    throw strength, ball buttons, bunt/swing controls and transmitter power
    settings. Ball state and its complement form two seven-bit halves;
    transmission uses the shared infrared report generator.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_esdx_controllers.h"

DEFINE_DEVICE_TYPE(XAVIX_ESDX_CONTROLLERS, xavix_esdx_controllers_device, "xavix_esdx_controllers", "Excite Stadium DX Infrared Controllers")

xavix_esdx_controllers_device::xavix_esdx_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_ESDX_CONTROLLERS, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_ball_throw(*this, "BALL_THROW")
	, m_ball_buttons(*this, "BALL_BUTTONS")
	, m_bat(*this, "BAT")
	, m_controller_power(*this, "CONTROLLER_POWER")
{
}

INPUT_PORTS_START( xavix_esdx_controllers )
	PORT_START("BALL_THROW")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("Ball Throw Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BALL_BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Ball Leftmost") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ball Inner Left") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ball Inner Right") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Ball Rightmost") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0xc3, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BAT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bat Bunt") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Bat Swing") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONTROLLER_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Wireless Ball Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "Wireless Bat Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_esdx_controllers_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(2);
	m_ir->set_mode(xavix_ir_report_device::mode::FRAMED_LSB);
	m_ir->set_bit_ticks(0x57 * 64);
	// The receiver samples after 1.53 data cells, requiring the transmitter's extended start pulse.
	m_ir->set_start_ticks(0x83 * 64);
	m_ir->set_report_bits<0>(15);
	m_ir->set_report_bits<1>(8);
	m_ir->report_callback<0>().set(*this, FUNC(xavix_esdx_controllers_device::ball_report_r));
	m_ir->report_callback<1>().set(*this, FUNC(xavix_esdx_controllers_device::bat_report_r));
	m_ir->report_enable_callback<0>().set(*this, FUNC(xavix_esdx_controllers_device::controller_enabled_r<0>));
	m_ir->report_enable_callback<1>().set(*this, FUNC(xavix_esdx_controllers_device::controller_enabled_r<1>));
}

void xavix_esdx_controllers_device::set_input_source(device_t &source)
{
	m_ball_throw.set_tag(source, m_ball_throw.finder_tag());
	m_ball_buttons.set_tag(source, m_ball_buttons.finder_tag());
	m_bat.set_tag(source, m_bat.finder_tag());
	m_controller_power.set_tag(source, m_controller_power.finder_tag());
}

void xavix_esdx_controllers_device::device_start()
{
}

void xavix_esdx_controllers_device::vblank_w(int state)
{
	// Physical transmitter cadence is unknown; one Ball-then-Bat pair per frame satisfies both receiver paths.
	m_ir->vblank_w(state);
}

u16 xavix_esdx_controllers_device::ball_report(u8 strength, u8 buttons)
{
	u8 const value = (strength & 0x03) | (buttons & 0x3c);
	u8 const complement = value ^ 0x7f;

	// The receiver validates two seven-bit halves as a state followed by its complement.
	return (u16(value) << 8) | (u16(complement) << 1);
}

u16 xavix_esdx_controllers_device::ball_report_r()
{
	return ball_report(m_ball_throw->read(), m_ball_buttons->read());
}

u16 xavix_esdx_controllers_device::bat_report_r()
{
	u8 const controls = m_bat->read();
	return xavix_ir_report_device::baseball_bat_report(BIT(controls, 0), BIT(controls, 1));
}

template <unsigned Controller>
int xavix_esdx_controllers_device::controller_enabled_r()
{
	return BIT(m_controller_power->read(), Controller);
}
