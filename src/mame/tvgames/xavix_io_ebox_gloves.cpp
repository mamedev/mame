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
	: device_t(mconfig, XAVIX_EBOX_GLOVES, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_punch(*this, "PUNCH")
	, m_guard(*this, "GUARD")
	, m_glove_power(*this, "GLOVE_POWER")
{
}

static INPUT_PORTS_START( xavix_ebox_gloves )
	PORT_START("PUNCH")
	// The receiver filters the first changed report, so each swing occupies two frames.
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Left Punch") PORT_PLAYER(1) PORT_IMPULSE(2) PORT_CODE(KEYCODE_SPACE) PORT_CODE(MOUSECODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Right Punch") PORT_PLAYER(1) PORT_IMPULSE(2) PORT_CODE(KEYCODE_LSHIFT)
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

void xavix_ebox_gloves_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(2);
	m_ir->set_bit_ticks(0xe5 * 32);
	m_ir->report_callback<0>().set(*this, FUNC(xavix_ebox_gloves_device::glove_report_r<0>));
	m_ir->report_callback<1>().set(*this, FUNC(xavix_ebox_gloves_device::glove_report_r<1>));
	m_ir->report_enable_callback<0>().set(*this, FUNC(xavix_ebox_gloves_device::glove_enabled_r<0>));
	m_ir->report_enable_callback<1>().set(*this, FUNC(xavix_ebox_gloves_device::glove_enabled_r<1>));
	m_ir->enable_callback().set(*this, FUNC(xavix_ebox_gloves_device::ir_enable_r));
}

ioport_constructor xavix_ebox_gloves_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_ebox_gloves);
}

void xavix_ebox_gloves_device::device_start()
{
}

void xavix_ebox_gloves_device::vblank_w(int state)
{
	m_ir->vblank_w(state);
}

template <unsigned Glove>
u16 xavix_ebox_gloves_device::glove_report_r()
{
	u8 const guard = m_guard->read();
	// All four strength codes select the same game action; use the strongest valid report.
	return xavix_ir_report_device::motion_report(Glove, BIT(guard, Glove), 3);
}

template <unsigned Glove>
int xavix_ebox_gloves_device::glove_enabled_r()
{
	u8 const mask = 1U << Glove;
	return bool(m_glove_power->read() & (m_punch->read() | m_guard->read()) & mask);
}

int xavix_ebox_gloves_device::ir_enable_r()
{
	return bool((m_punch->read() | m_guard->read()) & m_glove_power->read());
}
