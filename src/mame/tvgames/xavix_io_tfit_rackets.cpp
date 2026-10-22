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
	: xavix_ir_msb_device(mconfig, XAVIX_TFIT_RACKETS, tag, owner, clock)
	, m_racket_strength(*this, "RACKET%u_STRENGTH", 1U)
	, m_racket_buttons(*this, "RACKET%u_BUTTONS", 1U)
	, m_racket_power(*this, "RACKET_POWER")
{
	set_report_count(2);
	set_report_bits<0>(13);
	set_report_bits<1>(13);
	set_bit_ticks(0x8d * 64);
}

static INPUT_PORTS_START( xavix_tfit_rackets )
	PORT_START("RACKET1_STRENGTH")
	// The four decoded levels are exact; their physical acceleration thresholds are unmeasured.
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P1 Pink Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(1) PORT_CODE_INC(KEYCODE_LCONTROL)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET2_STRENGTH")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_NAME("P2 Blue Racket Swing Strength") PORT_MINMAX(0, 3) PORT_SENSITIVITY(100) PORT_KEYDELTA(1) PORT_CENTERDELTA(1) PORT_PLAYER(2) PORT_CODE_INC(KEYCODE_A)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET1_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P1 Pink Racket Circle / Serve / Confirm") PORT_PLAYER(1) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(0))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 Pink Racket Square / Slice / Cancel") PORT_PLAYER(1) PORT_CODE(KEYCODE_LALT) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(0)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(0))
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("RACKET2_BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("P2 Blue Racket Circle / Serve / Confirm") PORT_PLAYER(2) PORT_CODE(KEYCODE_A) PORT_CODE(MOUSECODE_BUTTON1_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON1_INDEXED(1))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 Blue Racket Square / Slice / Cancel") PORT_PLAYER(2) PORT_CODE(KEYCODE_S) PORT_CODE(MOUSECODE_BUTTON3_INDEXED(1)) PORT_CODE(GUNCODE_BUTTON2_INDEXED(1))
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

ioport_constructor xavix_tfit_rackets_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_tfit_rackets);
}

u32 xavix_tfit_rackets_device::read_report(unsigned report)
{
	if (!BIT(m_racket_power->read(), report))
		return xavix_ir_report_device::NO_SEQUENCE;

	u8 data = 0x80 | report | ((m_racket_strength[report]->read() & 3) << 1);
	u8 const buttons = m_racket_buttons[report]->read();
	if (BIT(buttons, 0))
		data |= 0x40;
	if (BIT(buttons, 1))
		data |= 0x20;

	bool const parity = 1 ^ BIT(data, 0) ^ BIT(data, 1) ^ BIT(data, 2) ^ BIT(data, 5) ^ BIT(data, 6);
	data |= parity ? 0x10 : 0x08;

	return (u32(data ^ 0xff) << 5) | (BIT(data, 0) << 4) | 0x0f;
}
