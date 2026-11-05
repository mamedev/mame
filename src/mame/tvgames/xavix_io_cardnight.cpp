// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Card Night handheld infrared controller. The receiver checks three
    repeated reports. Physical timing and simultaneous-key priority still
    require measurement; the docking contact belongs to the console.

***************************************************************************/

#include "emu.h"
#include "xavix_io_cardnight.h"

DEFINE_DEVICE_TYPE(XAVIX_CARDNIGHT_REMOTE, xavix_cardnight_remote_device, "xavix_cardnight_remote", "Card Night Infrared Remote")

xavix_cardnight_remote_device::xavix_cardnight_remote_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_run_length_device(mconfig, XAVIX_CARDNIGHT_REMOTE, tag, owner, clock)
	, m_dock_cb(*this, 0)
	, m_directions(*this, "IN1")
	, m_buttons(*this, "ROW0")
	, m_remote_reset(*this, "REMOTE_RESET")
	, m_factory_test(*this, "FACTORY_TX")
{
	set_bit_ticks(0x15 * 512);
	set_report_gap_ticks(64 * 512);
	set_report_count(3);
}

static INPUT_PORTS_START( xavix_cardnight_remote )
	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW0")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Enter")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("New Game")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Sound")

	PORT_START("REMOTE_RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Remote Reset") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(xavix_cardnight_remote_device::reset_changed), 0)

	PORT_START("FACTORY_TX")
	// The physical control or fixture sending this additional wireless command is unidentified.
	// PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Factory Test Transmission") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_cardnight_remote_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_cardnight_remote);
}

u32 xavix_cardnight_remote_device::read_report(unsigned report)
{
	if (BIT(m_factory_test->read(), 0))
		return 0x55;

	u8 const buttons = m_buttons->read();
	if (BIT(buttons, 5))
		return 0xa5;
	if (BIT(buttons, 6))
		return 0x95;
	if (BIT(buttons, 7))
		return 0x45;
	return DIRECTION_REPORTS[BIT(m_directions->read(), 2, 4)];
}

bool xavix_cardnight_remote_device::reports_enabled()
{
	return !m_dock_cb() && !BIT(m_remote_reset->read(), 0) && read_report(0);
}

INPUT_CHANGED_MEMBER(xavix_cardnight_remote_device::reset_changed)
{
	if (newval && machine().phase() == machine_phase::RUNNING)
		reset();
}
