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
	: device_t(mconfig, XAVIX_STADIUM_BAT, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_swing(*this, "SWING")
{
}

INPUT_PORTS_START( xavix_stadium_bat )
	PORT_START("SWING")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Bat Swing") PORT_CODE(KEYCODE_SPACE) PORT_IMPULSE(1)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_stadium_bat_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(1);
	m_ir->set_report_bits<0>(9);
	m_ir->set_bit_ticks((0x0a + 1) << (0x07 + 1));
	// The original bat sends an undifferentiated active-low infrared envelope rather than a data word.
	m_ir->report_callback<0>().set_constant(0);
	m_ir->enable_callback().set(*this, FUNC(xavix_stadium_bat_device::bat_active_r));
}

void xavix_stadium_bat_device::set_input_source(device_t &source)
{
	m_swing.set_tag(source, m_swing.finder_tag());
}

void xavix_stadium_bat_device::device_start()
{
}

void xavix_stadium_bat_device::vblank_w(int state)
{
	m_ir->vblank_w(state);
}

int xavix_stadium_bat_device::bat_active_r()
{
	return BIT(m_swing->read(), 0);
}
