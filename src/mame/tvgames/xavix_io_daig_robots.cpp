// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Daigunder infrared robots and accessories. Reads both players' controls,
    robot identities and accessory reports. The shared infrared report
    generator transmits the existing five-report sequence; identity reports
    alternate players with the configured screen's frame count.

    TODO:
    - Measure physical transmitter timing and identity repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_daig_robots.h"

DEFINE_DEVICE_TYPE(XAVIX_DAIGUNDER_ROBOTS, xavix_daigunder_robots_device, "xavix_daigunder_robots", "Daigunder Robot Infrared Controllers")

xavix_daigunder_robots_device::xavix_daigunder_robots_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_DAIGUNDER_ROBOTS, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_screen(*this, finder_base::DUMMY_TAG)
	, m_robot_controls(*this, "P%u_ROBOT", 1U)
	, m_robot_id(*this, "P%u_ID", 1U)
	, m_accessory_controls(*this, "ACCESSORY")
{
}

INPUT_PORTS_START( xavix_daigunder_robots )
	PORT_START("P1_ROBOT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Robot Attack") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P1 Robot Right") PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P1 Robot Left") PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_ROBOT")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Robot Attack") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_NAME("P2 Robot Right") PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_NAME("P2 Robot Left") PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1_ID")
	PORT_CONFNAME( 0x0f, 0x00, "P1 Robot" )
	PORT_CONFSETTING(    0x00, "Ryugu (Included)" )
	PORT_CONFSETTING(    0x01, "Eagle Arrow" )
	PORT_CONFSETTING(    0x02, "Drimog" )
	PORT_CONFSETTING(    0x03, "Trihorn" )
	PORT_CONFSETTING(    0x04, "Despector" )
	PORT_CONFSETTING(    0x05, "Ginzan" )
	PORT_CONFSETTING(    0x06, "Tigamaru" )
	PORT_CONFSETTING(    0x07, "Rougamaru" )
	PORT_CONFSETTING(    0x08, "Dragon Freezer" )
	PORT_CONFSETTING(    0x09, "Dragon Flame" )
	PORT_CONFSETTING(    0x0a, "Captain Werny" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2_ID")
	PORT_CONFNAME( 0x0f, 0x00, "P2 Robot" )
	PORT_CONFSETTING(    0x00, "Ryugu (Included)" )
	PORT_CONFSETTING(    0x01, "Eagle Arrow" )
	PORT_CONFSETTING(    0x02, "Drimog" )
	PORT_CONFSETTING(    0x03, "Trihorn" )
	PORT_CONFSETTING(    0x04, "Despector" )
	PORT_CONFSETTING(    0x05, "Ginzan" )
	PORT_CONFSETTING(    0x06, "Tigamaru" )
	PORT_CONFSETTING(    0x07, "Rougamaru" )
	PORT_CONFSETTING(    0x08, "Dragon Freezer" )
	PORT_CONFSETTING(    0x09, "Dragon Flame" )
	PORT_CONFSETTING(    0x0a, "Captain Werny" )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ACCESSORY")
	// Bulion and Bonerex transmit sensed claw/sword motions through these three report bits.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Accessory Swing / Attack") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Accessory Left Motion") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Accessory Right Motion") PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_daigunder_robots_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(5);
	m_ir->set_mode(xavix_ir_report_device::mode::FRAMED_LSB);
	m_ir->set_bit_ticks(0x50 * 64);
	m_ir->report_callback<0>().set(*this, FUNC(xavix_daigunder_robots_device::control_report_r<0>));
	m_ir->report_callback<1>().set(*this, FUNC(xavix_daigunder_robots_device::control_report_r<1>));
	m_ir->report_callback<2>().set(*this, FUNC(xavix_daigunder_robots_device::accessory_report_r));
	m_ir->report_callback<3>().set(*this, FUNC(xavix_daigunder_robots_device::identity_report_r<0>));
	m_ir->report_callback<4>().set(*this, FUNC(xavix_daigunder_robots_device::identity_report_r<1>));
}

void xavix_daigunder_robots_device::set_input_source(device_t &source)
{
	for (auto &port : m_robot_controls)
		port.set_tag(source, port.finder_tag());
	for (auto &port : m_robot_id)
		port.set_tag(source, port.finder_tag());
	m_accessory_controls.set_tag(source, m_accessory_controls.finder_tag());
}

void xavix_daigunder_robots_device::device_start()
{
}

void xavix_daigunder_robots_device::vblank_w(int state)
{
	m_ir->vblank_w(state);
}

u16 xavix_daigunder_robots_device::control_report_r(unsigned player)
{
	return CONTROL_REPORTS[player][m_robot_controls[player]->read() & 7];
}

u16 xavix_daigunder_robots_device::accessory_report_r()
{
	return ACCESSORY_REPORTS[m_accessory_controls->read() & 7];
}

template <unsigned Part>
u16 xavix_daigunder_robots_device::identity_report_r()
{
	unsigned const player = BIT(m_screen->frame_number(), 0);
	u8 const identity = m_robot_id[player]->read() & 0x0f;
	return identity <= 10 ? IDENTITY_REPORTS[player][identity][Part] : control_report_r(player);
}
