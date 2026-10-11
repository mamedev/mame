// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Kamen Rider Hibiki wireless drumsticks. Each powered stick latches its
    swing strength and sends two matching reports on successive frames.
    Transmission uses the shared infrared report generator.

    TODO:
    - Measure physical repetition cadence and report-family polarity.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ordj_sticks.h"

DEFINE_DEVICE_TYPE(XAVIX_HIBIKI_STICKS, xavix_hibiki_sticks_device, "xavix_hibiki_sticks", "Kamen Rider Hibiki Infrared Sticks")

xavix_hibiki_sticks_device::xavix_hibiki_sticks_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_HIBIKI_STICKS, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_swing_strength(*this, "STICK%u", 1U)
	, m_stick_power(*this, "STICK_POWER")
{
}

INPUT_PORTS_START( xavix_hibiki_sticks )
	PORT_START("SWING")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Left (Red Strap) Stick Swing") PORT_PLAYER(1)
		PORT_CHANGED_MEMBER("sticks", FUNC(xavix_hibiki_sticks_device::stick_swing), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Right (Black Strap) Stick Swing") PORT_PLAYER(2)
		PORT_CHANGED_MEMBER("sticks", FUNC(xavix_hibiki_sticks_device::stick_swing), 1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("Left (Red Strap) Stick Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("Right (Black Strap) Stick Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK_POWER")
	// The report contains no battery field, so an exhausted or switched-off stick simply stops reporting.
	PORT_CONFNAME( 0x01, 0x01, "Left (Red Strap) Stick Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "Right (Black Strap) Stick Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_hibiki_sticks_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(2);
	m_ir->set_bit_ticks(0xe5 * 32);
	m_ir->set_first_data_ticks(0x17d * 32);
	m_ir->report_callback<0>().set(*this, FUNC(xavix_hibiki_sticks_device::stick_report_r<0>));
	m_ir->report_callback<1>().set(*this, FUNC(xavix_hibiki_sticks_device::stick_report_r<1>));
	m_ir->enable_callback().set(*this, FUNC(xavix_hibiki_sticks_device::ir_enable_r));
}

void xavix_hibiki_sticks_device::set_input_source(device_t &source)
{
	for (auto &port : m_swing_strength)
		port.set_tag(source, port.finder_tag());
	m_stick_power.set_tag(source, m_stick_power.finder_tag());
}

void xavix_hibiki_sticks_device::device_start()
{
	save_item(NAME(m_latched_strength));
	save_item(NAME(m_reports_remaining));
}

void xavix_hibiki_sticks_device::device_reset()
{
	m_latched_strength.fill(0);
	m_reports_remaining.fill(0);
}

void xavix_hibiki_sticks_device::vblank_w(int state)
{
	// Physical repetition cadence is unknown; two identical reports per stick across successive frames satisfy the receiver's duplicate filter.
	m_ir->vblank_w(state);
}

INPUT_CHANGED_MEMBER(xavix_hibiki_sticks_device::stick_swing)
{
	unsigned const player = unsigned(param);
	assert(player < m_reports_remaining.size());
	if (newval && !oldval && BIT(m_stick_power->read(), player))
		m_reports_remaining[player] = 2;
}

int xavix_hibiki_sticks_device::ir_enable_r()
{
	u8 const power = m_stick_power->read();
	int active = 0;
	for (unsigned player = 0; player != m_reports_remaining.size(); ++player)
	{
		if (!BIT(power, player))
			m_reports_remaining[player] = 0;
		else if (m_reports_remaining[player])
		{
			active = 1;
			if (m_reports_remaining[player] == 2)
				m_latched_strength[player] = m_swing_strength[player]->read() & 3;
		}
	}
	return active;
}

template <unsigned Player>
u16 xavix_hibiki_sticks_device::stick_report_r()
{
	if (!m_reports_remaining[Player] || !BIT(m_stick_power->read(), Player))
		return 0xff;

	--m_reports_remaining[Player];
	// The sticks have no gameplay switch; the transmitter's alternate report-family polarity is unknown.
	return xavix_ir_report_device::motion_report(Player, false, m_latched_strength[Player]);
}
