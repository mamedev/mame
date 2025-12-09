// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*
    Skeleton driver for the uPD78C14-based "Driver Information System" used by
    the circa-1990 Oldsmobile 98 and other models.

    Press 6, then Test/Reset, then enter a distance of 8192 to show a debug display
    with information about the current ADC readings. After that, press any button
    to display the ROM version.

    TODO:
    - identify/hook up the display hardware
    - properly hook up or at least figure out other inputs (port B, SCK/RX, CI, most ADCs, etc)
      The function at 09D5 handles reading the ADC values.
*/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"

#include <algorithm>

namespace {

class omdisv22_state : public driver_device
{
public:
	omdisv22_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_inputs(*this, "IN%u", 0U)
	{ }

	void omdisv22(machine_config &config);

	void inputs_w(int state) { m_input_sel = state; }
	ioport_value inputs_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<upd78c14_device> m_maincpu;
	required_ioport_array<3> m_inputs;

	void pd_w(u8 data) { m_pd = data; }
	void pf_w(u8 data);

	ioport_value m_input_sel;

	u8 m_output_pos;
	u8 m_output[18];

	u8 m_pd, m_pf;
};

/**************************************************************************/
static INPUT_PORTS_START(omdisv22)
	PORT_START("PA")
	PORT_BIT( 0x07, IP_ACTIVE_HIGH, IPT_OUTPUT  ) PORT_WRITE_LINE_MEMBER(FUNC(omdisv22_state::inputs_w))
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_CUSTOM_MEMBER(FUNC(omdisv22_state::inputs_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("AN5")
	PORT_CONFNAME( 0xff, 0xff, "Default Units" )
	PORT_CONFSETTING(    0xff, "English")
	PORT_CONFSETTING(    0x00, "Metric")

	PORT_START("IN0")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0 / E/M")   PORT_CODE(KEYCODE_0_PAD);
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1 / RANGE") PORT_CODE(KEYCODE_1_PAD);
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2 / ECON")  PORT_CODE(KEYCODE_2_PAD);
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3 / GAGES") PORT_CODE(KEYCODE_3_PAD);

	PORT_START("IN1")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4 / OIL")  PORT_CODE(KEYCODE_4_PAD);
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5 / FUEL") PORT_CODE(KEYCODE_5_PAD);
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6 / DEST") PORT_CODE(KEYCODE_6_PAD);
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7 / ETA")  PORT_CODE(KEYCODE_7_PAD);

	PORT_START("IN2")
	PORT_BIT( 0x1, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8 / E/T")   PORT_CODE(KEYCODE_8_PAD);
	PORT_BIT( 0x2, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 / SPEED") PORT_CODE(KEYCODE_9_PAD);
	PORT_BIT( 0x4, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET")     PORT_CODE(KEYCODE_ENTER_PAD);
	PORT_BIT( 0x8, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("DATE/TIME") PORT_CODE(KEYCODE_DEL_PAD);
INPUT_PORTS_END

/**************************************************************************/
void omdisv22_state::omdisv22(machine_config &config)
{
	UPD78C14(config, m_maincpu, 3'145'728); // 128*64*384, generates 1 Hz INTFT1 and 8192 Hz SCK
	m_maincpu->pa_in_cb().set_ioport("PA");
	m_maincpu->pa_out_cb().set_ioport("PA");
	m_maincpu->pb_in_cb().set_constant(0xff); // bit 1 = display ready
	m_maincpu->pc_in_cb().set_constant(0xff);
	m_maincpu->pd_out_cb().set(FUNC(omdisv22_state::pd_w));
	m_maincpu->pf_out_cb().set(FUNC(omdisv22_state::pf_w));
	m_maincpu->an0_func().set_constant(0xff); // fuel level sensor?
	m_maincpu->an1_func().set_constant(0xff); // unknown
	m_maincpu->an2_func().set_constant(0xff); // unknown (only tested whether on/off)
	m_maincpu->an3_func().set_constant(0xff); // reference/divisor for AN0
	m_maincpu->an4_func().set_constant(0xff); // affects scaling of AN0/AN3 (only tested whether on/off)
	m_maincpu->an5_func().set_ioport("AN5");
	m_maincpu->an6_func().set_constant(0xff); // similar to AN4
}

/**************************************************************************/
void omdisv22_state::machine_start()
{
	std::fill(std::begin(m_output), std::end(m_output), 0);

	save_item(NAME(m_input_sel));
	save_item(NAME(m_output_pos));
	save_item(NAME(m_output));
	save_item(NAME(m_pd));
	save_item(NAME(m_pf));
}

/**************************************************************************/
void omdisv22_state::machine_reset()
{
	m_maincpu->set_input_line(UPD7810_INTF2, ASSERT_LINE);

	m_input_sel = 0x7;

	m_output_pos = 0;
	m_pd = m_pf = 0xff;
}

/**************************************************************************/
ioport_value omdisv22_state::inputs_r()
{
	ioport_value val = 0xf;

	for (int i = 0; i < 3; i++)
		if (!BIT(m_input_sel, i))
			val &= m_inputs[i]->read();

	return val;
}

/**************************************************************************/
void omdisv22_state::pf_w(u8 data)
{
	// TODO: what kind of display is this, anyway?
	if (BIT(m_pf, 5) && BIT(m_pf, 4) && !BIT(data, 4))
	{
		if (m_pd == 0xf1)
		{
			m_output_pos = 0;
		}
		else if (m_pd == 0xf2)
		{
			m_output[m_output_pos] = 0;
			logerror("%s\n", (const char*)m_output);
			popmessage("%s", (const char*)m_output);
		}
		else if (m_output_pos < (sizeof(m_output) - 1))
		{
			m_output[m_output_pos++] = 0x30 + (m_pd / 5);
		}
	}

	m_pf = data;
}

/**************************************************************************/
ROM_START( omdisv22 )
	ROM_REGION(0x4000, "maincpu", 0) // date code 9020, upper half of ROM is empty/unused
	ROM_LOAD( "upd78c14g-443.bin", 0x0000, 0x4000, CRC(fab369b2) SHA1(7cbd5be32e475efd58db70c0a94b770c58fd9b8e) )
ROM_END

} // anonymous namespace

SYST( 1990?, omdisv22, 0, 0, omdisv22, omdisv22, omdisv22_state, empty_init, "General Motors", "Oldsmobile Driver Information System (version 2.2 CH)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
