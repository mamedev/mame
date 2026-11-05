// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Excite Boxing infrared gloves. Swing and Guard controls select reports
    from the shared paired-controller alphabet, while each glove's power
    switch suppresses its transmitter. The shared infrared report generator
    owns the serial waveform and saved state.

    TODO:
    - Measure physical transmitter timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ebox_gloves.h"

DEFINE_DEVICE_TYPE(XAVIX_EBOX_GLOVES, xavix_ebox_gloves_device, "xavix_ebox_gloves", "Excite Boxing Infrared Gloves")

xavix_ebox_gloves_device::xavix_ebox_gloves_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_motion_base_device(mconfig, XAVIX_EBOX_GLOVES, tag, owner, clock)
	, m_punch(*this, "PUNCH")
	, m_guard(*this, "GUARD")
	, m_glove_power(*this, "GLOVE_POWER")
{
}

static INPUT_PORTS_START( xavix_ebox_gloves )
	PORT_START("PUNCH")
	// Holding a Punch control represents continued glove motion.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left Punch") PORT_PLAYER(1) PORT_CODE(KEYCODE_SPACE) PORT_CODE(MOUSECODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Right Punch") PORT_PLAYER(1) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GUARD")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Left Guard") PORT_PLAYER(1) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Right Guard") PORT_PLAYER(1) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GLOVE_POWER")
	// A switched-off or exhausted glove emits no infrared report.
	PORT_CONFNAME( 0x01, 0x01, "Left Glove Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x01, DEF_STR(On) )
	PORT_CONFNAME( 0x02, 0x02, "Right Glove Power" )
	PORT_CONFSETTING(    0x00, DEF_STR(Off) )
	PORT_CONFSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_ebox_gloves_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_ebox_gloves);
}

u32 xavix_ebox_gloves_device::read_report(unsigned report)
{
	u8 const guard = m_guard->read();
	u8 const motion = BIT(m_punch->read(), report);
	return motion_report(report, BIT(guard, report), motion);
}

bool xavix_ebox_gloves_device::report_enabled(unsigned report)
{
	u8 const mask = 1U << report;
	return bool(m_glove_power->read() & (m_punch->read() | m_guard->read()) & mask);
}

bool xavix_ebox_gloves_device::reports_enabled()
{
	return bool((m_punch->read() | m_guard->read()) & m_glove_power->read());
}
