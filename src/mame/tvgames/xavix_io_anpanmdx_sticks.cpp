// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Anpanman Mat DX wireless sticks. Each activation button wakes one
    transmitter; exhausted or removed batteries stop its reports. Awake
    sticks send the selected motion sequence through the shared IR generator.

    TODO:
    - Verify motion thresholds, auxiliary bits and physical sleep cadence.

***************************************************************************/

#include "emu.h"
#include "xavix_io_anpanmdx_sticks.h"

DEFINE_DEVICE_TYPE(XAVIX_ANPANMAN_STICKS, xavix_anpanman_sticks_device, "xavix_anpanman_sticks", "Anpanman Mat DX Infrared Sticks")

xavix_anpanman_sticks_device::xavix_anpanman_sticks_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_ANPANMAN_STICKS, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_motion_level(*this, "STICK%u", 1U)
	, m_stick_batteries(*this, "STICK_BATTERIES")
{
}

INPUT_PORTS_START( xavix_anpanman_sticks )
	PORT_START("STICK1")
	// The receiver proves four ordered levels, but whether they represent acceleration or orientation remains unknown.
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Stick Motion Level") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Stick Motion Level") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK_ACTIVATE")
	// Each momentary handle button wakes its transmitter; the firmware-compatible report leaves unresolved auxiliary bits clear.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Stick Activate") PORT_PLAYER(1)
		PORT_CHANGED_MEMBER("sticks", FUNC(xavix_anpanman_sticks_device::stick_activate), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Stick Activate") PORT_PLAYER(2)
		PORT_CHANGED_MEMBER("sticks", FUNC(xavix_anpanman_sticks_device::stick_activate), 1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STICK_BATTERIES")
	PORT_CONFNAME( 0x01, 0x01, "P1 Stick Batteries" )
	PORT_CONFSETTING(    0x00, "Exhausted / Removed" )
	PORT_CONFSETTING(    0x01, "Normal" )
	PORT_CONFNAME( 0x02, 0x02, "P2 Stick Batteries" )
	PORT_CONFSETTING(    0x00, "Exhausted / Removed" )
	PORT_CONFSETTING(    0x02, "Normal" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_anpanman_sticks_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(2);
	m_ir->set_mode(xavix_ir_report_device::mode::SAMPLE_SEQUENCE);
	m_ir->set_report_bits<0>(17);
	m_ir->set_report_bits<1>(17);
	// These transitions target the ROM's 17 sampling windows; transmitter edge timing remains unmeasured.
	m_ir->set_sample_ticks(std::array<u32, 17>
	{
		10'414, 7'164, 5'564, 3'804, 4'380, 8'060, 9'084, 9'084, 9'084,
		9'084, 9'084, 9'084, 9'084, 9'084, 9'084, 9'084, 9'084
	});
	m_ir->set_bit_ticks(9'084);
	m_ir->sequence_callback<0>().set(*this, FUNC(xavix_anpanman_sticks_device::stick_sequence_r<0>));
	m_ir->sequence_callback<1>().set(*this, FUNC(xavix_anpanman_sticks_device::stick_sequence_r<1>));
	m_ir->enable_callback().set(*this, FUNC(xavix_anpanman_sticks_device::ir_enable_r));
}

void xavix_anpanman_sticks_device::set_input_source(device_t &source)
{
	for (auto &port : m_motion_level)
		port.set_tag(source, port.finder_tag());
	m_stick_batteries.set_tag(source, m_stick_batteries.finder_tag());
}

void xavix_anpanman_sticks_device::device_start()
{
	save_item(NAME(m_stick_awake));
}

void xavix_anpanman_sticks_device::device_reset()
{
	m_stick_awake.fill(false);
}

void xavix_anpanman_sticks_device::vblank_w(int state)
{
	// Continuous once-per-frame reports reproduce the observed post-wake operation; the physical sleep cadence is unknown.
	m_ir->vblank_w(state);
}

INPUT_CHANGED_MEMBER(xavix_anpanman_sticks_device::stick_activate)
{
	unsigned const player = unsigned(param);
	assert(player < m_stick_awake.size());
	if (newval && !oldval && BIT(m_stick_batteries->read(), player))
		m_stick_awake[player] = true;
}

int xavix_anpanman_sticks_device::ir_enable_r()
{
	u8 const batteries = m_stick_batteries->read();
	for (unsigned player = 0; player != m_stick_awake.size(); ++player)
	{
		if (!BIT(batteries, player))
			m_stick_awake[player] = false;
	}

	return m_stick_awake[0] || m_stick_awake[1];
}

template <unsigned Player>
u32 xavix_anpanman_sticks_device::stick_sequence_r()
{
	if (!m_stick_awake[Player] || !BIT(m_stick_batteries->read(), Player))
		return xavix_ir_report_device::NO_SEQUENCE;

	// Stick-number polarity follows the decoder's two report destinations; a physical IR capture is still needed to verify it.
	return STICK_SEQUENCES[Player][m_motion_level[Player]->read() & 3];
}
