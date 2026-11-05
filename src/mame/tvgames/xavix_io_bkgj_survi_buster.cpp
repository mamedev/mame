// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Boukenger Survi Buster infrared weapon. Reads the trigger and controller
    power setting, producing the existing 28-sample trigger sequence through
    the shared infrared report generator.

    TODO:
    - Identify the red side control and measure transmitter edge timing.

***************************************************************************/

#include "emu.h"
#include "xavix_io_bkgj_survi_buster.h"

DEFINE_DEVICE_TYPE(XAVIX_SURVI_BUSTER, xavix_survi_buster_device, "xavix_survi_buster", "Boukenger Survi Buster Infrared Controller")

xavix_survi_buster_device::xavix_survi_buster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: xavix_ir_msb_device(mconfig, XAVIX_SURVI_BUSTER, tag, owner, clock)
	, m_survi_buster(*this, "SURVI_BUSTER")
	, m_survi_buster_power(*this, "SURVI_BUSTER_POWER")
{
	set_report_count(1);
	set_report_bits<0>(28);
	// These transitions target the ROM's 28 sampling windows; physical transmitter edge timing remains unmeasured.
	set_sample_ticks(SURVI_BUSTER_SAMPLE_TICKS);
	set_bit_ticks(SURVI_BUSTER_SAMPLE_TICKS.back());
}

static INPUT_PORTS_START( xavix_survi_buster )
	PORT_START("SURVI_BUSTER")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Survi-Buster Trigger / Shoot / Confirm") PORT_CODE(KEYCODE_LALT) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(0))
	// The red side circle seen in photos has no verified firmware function.
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SURVI_BUSTER_POWER")
	PORT_CONFNAME( 0x01, 0x01, "Survi-Buster Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

ioport_constructor xavix_survi_buster_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_survi_buster);
}

u32 xavix_survi_buster_device::read_report(unsigned)
{
	return BIT(m_survi_buster_power->read(), 0) && BIT(m_survi_buster->read(), 0)
		? SURVI_BUSTER_TRIGGER_REPORT
		: xavix_ir_report_device::NO_SEQUENCE;
}
