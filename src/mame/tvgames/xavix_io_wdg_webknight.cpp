// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Optional Web Knight infrared transmitters for Webdiver Gladion. Their
    mark switches send an identity report for dialogue or weapon attachment;
    the attachment contact remains on Gladion itself.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_wdg_webknight.h"

DEFINE_DEVICE_TYPE(XAVIX_WEBKNIGHT, xavix_webknight_device, "xavix_webknight", "Webdiver Web Knight Infrared Controller")

xavix_webknight_device::xavix_webknight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_pulse_distance_device(mconfig, XAVIX_WEBKNIGHT, tag, owner, clock)
	, m_identity(*this, "WEBKNIGHT_IDENTITY")
	, m_mark_switch(*this, "WEBKNIGHT_MARK")
{
	set_report_count(1);
	set_report_bits<0>(10);
	// Representative intervals lie inside Gladion's receiver windows; no physical capture is available.
	set_pulse_distance_ticks(0x28 * 256, 0x48 * 256, 0x2c * 256);
}

static INPUT_PORTS_START( xavix_webknight )
	PORT_START("WEBKNIGHT_IDENTITY")
	// Names are identified by the ROM's dialogue; only reports 0-6 are accepted for weapon attachment.
	PORT_CONFNAME( 0x1f, 0x1f, "Web Knight" )
	PORT_CONFSETTING(    0x1f, DEF_STR(None) )
	PORT_CONFSETTING(    0x00, "Jagaon" )
	PORT_CONFSETTING(    0x01, "Sharkon" )
	PORT_CONFSETTING(    0x02, "Griffion" )
	PORT_CONFSETTING(    0x03, "Garyun" )
	PORT_CONFSETTING(    0x04, "Phoenixon" )
	PORT_CONFSETTING(    0x05, "Kerberion" )
	PORT_CONFSETTING(    0x06, "Orthrion" )
	// Golemon forms follow the ROM portraits; their physical switch reports are unmeasured.
	PORT_CONFSETTING(    0x07, "Golemon (Kerberion)" )
	PORT_CONFSETTING(    0x08, "Golemon (Orthrion)" )
	PORT_CONFSETTING(    0x09, "Dragwon" )
	PORT_CONFSETTING(    0x0d, "Ligeron" )
	PORT_CONFSETTING(    0x0e, "Wyverion" )
	PORT_CONFSETTING(    0x0f, "Pegasion" )
	PORT_CONFSETTING(    0x10, "Ditalion" )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("WEBKNIGHT_MARK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Web Knight Mark Switch") PORT_CODE(KEYCODE_V) PORT_PLAYER(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_webknight_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_webknight);
}

u32 xavix_webknight_device::read_report(unsigned)
{
	u8 const code = m_identity->read() & 0x1f;
	return complemented_report(code);
}

bool xavix_webknight_device::report_enabled(unsigned)
{
	return (m_identity->read() & 0x1f) != 0x1f && BIT(m_mark_switch->read(), 0);
}
