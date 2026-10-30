// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Four-zone XaviX wireless step mats. Each accessory owns its contacts and
    local power switch, supplying a decoded report to the console interface.
    The physical mat-to-receiver signalling is not modelled here.

    The four-zone input definition is also shared by xavjmat/xavaero, whose
    separate mat peripheral generates timed infrared reports.

***************************************************************************/

#include "emu.h"
#include "xavix_io_step_mat.h"

DEFINE_DEVICE_TYPE(XAVIX_STEP_MAT, xavix_step_mat_device, "xavix_step_mat", "XaviX Step Mat")

xavix_step_mat_device::xavix_step_mat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_STEP_MAT, tag, owner, clock)
	, m_contacts(*this, "STEP_SENSOR")
	, m_power(*this, "STEP_POWER")
{
}

INPUT_PORTS_START( xavix_step_sensor )
	PORT_START("STEP_SENSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Step Sensor Enter") PORT_CODE(KEYCODE_F) PORT_CONDITION("STEP_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Step Sensor Forward / Right") PORT_CODE(KEYCODE_D) PORT_CONDITION("STEP_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Step Sensor Back / Left") PORT_CODE(KEYCODE_S) PORT_CONDITION("STEP_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Step Sensor Cancel") PORT_CODE(KEYCODE_A) PORT_CONDITION("STEP_POWER", 0x01, EQUALS, 0x01)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STEP_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Step Sensor Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END

ioport_constructor xavix_step_mat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_step_sensor);
}

void xavix_step_mat_device::device_start()
{
}

u16 xavix_step_mat_device::report_r()
{
	return BIT(m_power->read(), 0) ? (0x10 | (m_contacts->read() & 0x0f)) : 0;
}
