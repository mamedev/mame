// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    XaviX and Radica baseball infrared bat input. Turns swing-strength
    selections into timed I/O events for the console's infrared receiver.
    The event counts represent weak, medium and strong swings; each swing
    remains active while the swing-strength input is held.

    TODO:
    - Measure the infrared swing envelope and repetition on an original
      bat.

***************************************************************************/

#include "emu.h"
#include "xavix_io_bb_bat.h"

DEFINE_DEVICE_TYPE(XAVIX_BASEBALL_BAT, xavix_baseball_bat_device, "xavix_baseball_bat", "XaviX / Radica Baseball Infrared Bat")

xavix_baseball_bat_device::xavix_baseball_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_optical_pulse_device_base(mconfig, XAVIX_BASEBALL_BAT, tag, owner, clock, "xavix_baseball_bat_device::event_timer")
	, m_strength(*this, "BAT")
{
}

static INPUT_PORTS_START( xavix_baseball_bat )
	PORT_START("BAT")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL2 ) PORT_NAME("Bat Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(3) PORT_CENTERDELTA(3) PORT_CODE_INC(KEYCODE_SPACE)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_baseball_bat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_baseball_bat);
}

void xavix_baseball_bat_device::vblank_w(int state)
{
	if (!state)
		return;

	u8 const strength = m_strength->read() & 3;
	if (!strength)
		return;

	// The receiver's event rate selects the ROM's weak, medium or strong swing bucket.
	schedule_events(EVENT_COUNTS[strength]);
}
