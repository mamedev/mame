// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Original Excite Stadium infrared bat. A swing emits an undifferentiated
    active-low infrared envelope, represented by nine zero bits through the
    shared infrared report generator.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_stad_bat.h"

DEFINE_DEVICE_TYPE(XAVIX_STADIUM_BAT, xavix_stadium_bat_device, "xavix_stadium_bat", "Excite Stadium Infrared Bat")

xavix_stadium_bat_device::xavix_stadium_bat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_msb_device(mconfig, XAVIX_STADIUM_BAT, tag, owner, clock)
	, m_swing(*this, "SWING")
{
	set_report_count(1);
	set_report_bits<0>(9);
	set_bit_ticks((0x0a + 1) << (0x07 + 1));
}

static INPUT_PORTS_START( xavix_stadium_bat )
	PORT_START("SWING")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Bat Swing") PORT_CODE(KEYCODE_SPACE) PORT_IMPULSE(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_stadium_bat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_stadium_bat);
}

bool xavix_stadium_bat_device::reports_enabled()
{
	return BIT(m_swing->read(), 0);
}
