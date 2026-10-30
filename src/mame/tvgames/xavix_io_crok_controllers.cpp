// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Croket wireless hammer and wristband. Reads motion, guard and power
    controls, suppressing idle or unpowered reports and selecting the
    strongest accepted motion code. The shared infrared report generator
    transmits the existing paired reports.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_crok_controllers.h"

DEFINE_DEVICE_TYPE(XAVIX_CROKET_CONTROLLERS, xavix_croket_controllers_device, "xavix_croket_controllers", "Croket Infrared Hammer and Wristband")

xavix_croket_controllers_device::xavix_croket_controllers_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_motion_base_device(mconfig, XAVIX_CROKET_CONTROLLERS, tag, owner, clock)
	, m_motion(*this, "MOTION")
	, m_motion_strength(*this, "STRENGTH%u", 1U)
	, m_guard(*this, "GUARD")
	, m_controller_power(*this, "CONTROLLER_POWER")
{
}

static INPUT_PORTS_START( xavix_croket_controllers )
	PORT_START("MOTION")
	// The first changed report is filtered, so each physical action occupies two frames.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Hammer Swing") PORT_PLAYER(1) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Wristband Punch") PORT_PLAYER(2) PORT_IMPULSE(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STRENGTH1")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Hammer Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_Z)

	PORT_START("STRENGTH2")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Wristband Punch Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_X)

	PORT_START("GUARD")
	// The controller button selects a report family and repeats while held.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Hammer Guard") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Wristband Guard") PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("CONTROLLER_POWER")
	// A switched-off or exhausted controller emits no wireless report.
	PORT_CONFNAME( 0x01, 0x01, "P1 Hammer Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "P2 Wristband Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_croket_controllers_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_croket_controllers);
}

u32 xavix_croket_controllers_device::read_report(unsigned report)
{
	u8 const mask = 1U << report;
	u8 const motion = m_motion->read();
	u8 const guard = m_guard->read();
	u8 const strength = (motion | guard) & mask ? 3 : m_motion_strength[report]->read();
	if (!(m_controller_power->read() & mask) || !strength)
		return 0xff;

	return motion_report(report, BIT(guard, report), strength);
}

bool xavix_croket_controllers_device::reports_enabled()
{
	u8 const buttons = m_motion->read() | m_guard->read();
	u8 const power = m_controller_power->read();
	for (unsigned player = 0; player < 2; player++)
		if (BIT(power, player) && (BIT(buttons, player) || m_motion_strength[player]->read()))
			return 1;
	return 0;
}
