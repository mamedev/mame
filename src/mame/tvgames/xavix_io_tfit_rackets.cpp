// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Tennis x Fitness pink and blue infrared rackets. Reads each racket's
    swing strength, buttons and power setting, encoding the existing
    parity-protected reports through the shared infrared report generator.

    TODO:
    - Measure physical swing thresholds and transmitter repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_tfit_rackets.h"

DEFINE_DEVICE_TYPE(XAVIX_TFIT_RACKETS, xavix_tfit_rackets_device, "xavix_tfit_rackets", "Tennis x Fitness Infrared Rackets")

xavix_tfit_rackets_device::xavix_tfit_rackets_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_TFIT_RACKETS, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_racket_strength(*this, "RACKET%u_STRENGTH", 1U)
	, m_racket_buttons(*this, "RACKET%u_BUTTONS", 1U)
	, m_racket_power(*this, "RACKET_POWER")
{
}

INPUT_PORTS_START( xavix_tfit_rackets )
	PORT_START("RACKET1_STRENGTH")
	// The four decoded levels are exact; their physical acceleration thresholds are unmeasured.
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Pink Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET2_STRENGTH")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Blue Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET1_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pink Racket Circle / Serve / Confirm") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pink Racket Square / Slice / Cancel") PORT_PLAYER(1)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET2_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Blue Racket Circle / Serve / Confirm") PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Blue Racket Square / Slice / Cancel") PORT_PLAYER(2)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET_POWER")
	PORT_CONFNAME( 0x01, 0x01, "P1 Pink Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "P2 Blue Racket Power" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x02, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void xavix_tfit_rackets_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(2);
	m_ir->set_mode(xavix_ir_report_device::mode::SAMPLE_SEQUENCE);
	m_ir->set_report_bits<0>(13);
	m_ir->set_report_bits<1>(13);
	m_ir->set_bit_ticks(0x8d * 64);
	m_ir->sequence_callback<0>().set(*this, FUNC(xavix_tfit_rackets_device::racket_report_r<0>));
	m_ir->sequence_callback<1>().set(*this, FUNC(xavix_tfit_rackets_device::racket_report_r<1>));
}

void xavix_tfit_rackets_device::set_input_source(device_t &source)
{
	for (auto &port : m_racket_strength)
		port.set_tag(source, port.finder_tag());
	for (auto &port : m_racket_buttons)
		port.set_tag(source, port.finder_tag());
	m_racket_power.set_tag(source, m_racket_power.finder_tag());
}

void xavix_tfit_rackets_device::device_start()
{
}

void xavix_tfit_rackets_device::vblank_w(int state)
{
	// Once-per-frame report pairs satisfy the receiver; physical transmitter repetition is unmeasured.
	m_ir->vblank_w(state);
}

template <unsigned Player>
u32 xavix_tfit_rackets_device::racket_report_r()
{
	if (!BIT(m_racket_power->read(), Player))
		return xavix_ir_report_device::NO_SEQUENCE;

	u8 data = 0x80 | Player | ((m_racket_strength[Player]->read() & 3) << 1);
	u8 const buttons = m_racket_buttons[Player]->read();
	if (BIT(buttons, 0))
		data |= 0x40;
	if (BIT(buttons, 1))
		data |= 0x20;

	bool const parity = 1 ^ BIT(data, 0) ^ BIT(data, 1) ^ BIT(data, 2) ^ BIT(data, 5) ^ BIT(data, 6);
	data |= parity ? 0x10 : 0x08;

	return (u32(data ^ 0xff) << 5) | (BIT(data, 0) << 4) | 0x0f;
}
