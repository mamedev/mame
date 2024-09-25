// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Mephisto Roma II

It's a cost-reduced version of Roma 68000, not really a 'sequel'.

Hardware notes:
- MC68HC000FN10 @ 9.83MHz
- 64KB ROM (2*27C256), 16KB RAM (2*HY6264LP-10), piezo
- compatible with magnets chessboard or Mephisto Mobil, and 7seg LCD module

Mephisto Montreal 68000 from 1993 is on similar hardware, but it is a standalone
chess computer (Roma II is a module). The chess engine is still the old 1987 Roma.

TODO:
- verify montreal XTAL (currently guessed from beeper pitch on video)
- verify irq source and level
- does it have DTACK waitstates?

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "cpu/m68000/m68000.h"
#include "machine/74259.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "mephisto_montreal.lh"
#include "mephisto_roma2.lh"


namespace {

class roma2_state : public driver_device
{
public:
	roma2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_reset(*this, "RESET")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void roma2(machine_config &config);
	void montreal(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<hc259_device> m_outlatch;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display1_device> m_display;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<4> m_inputs;
	optional_ioport m_reset;

	void main_map(address_map &map) ATTR_COLD;

	u8 input_r();
};

INPUT_CHANGED_MEMBER(roma2_state::reset_button)
{
	// RES buttons in serial tied to CPU RESET
	if (m_reset->read() == 3)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_display->reset();
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 roma2_state::input_r()
{
	u8 data = 0;

	// read keypad
	for (int i = 0; i < 4; i++)
		if (!BIT(m_outlatch->output_state(), i))
			data |= m_inputs[i]->read();

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void roma2_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x800000, 0x803fff).ram();
	map(0x900000, 0x90000f).w(m_outlatch, FUNC(hc259_device::write_d0)).umask16(0xff00);
	map(0xb00000, 0xb00000).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0xc00000, 0xc00000).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0xd00000, 0xd00000).r(m_board, FUNC(mephisto_board_device::input_r));
	map(0xd00001, 0xd00001).r(FUNC(roma2_state::input_r));
	map(0xe00000, 0xe00000).w(m_display, FUNC(mephisto_display1_device::data_w));

	map(0x900000, 0x900009).nopr(); // st
	map(0xc00000, 0xc00001).nopr(); // st
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( montreal )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // combine CL/ENT for NEW GAME
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // "
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
INPUT_PORTS_END

static INPUT_PORTS_START( roma2 )
	PORT_INCLUDE( montreal )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 1") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, roma2_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 2") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, roma2_state, reset_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void roma2_state::roma2(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 9.8304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roma2_state::main_map);

	const attotime irq_period = attotime::from_hz(9.8304_MHz_XTAL / 0x14000); // 120Hz
	m_maincpu->set_periodic_int(FUNC(roma2_state::irq5_line_hold), irq_period);

	HC259(config, m_outlatch);
	// Q0-Q3: input mux; Q4: strobe; Q6-Q7: DAC
	m_outlatch->q_out_cb<4>().set(m_display, FUNC(mephisto_display1_device::strobe_w));
	m_outlatch->parallel_out_cb().set(m_dac, FUNC(dac_2bit_ones_complement_device::write)).rshift(6).mask(3);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display);
	config.set_default_layout(layout_mephisto_roma2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}

void roma2_state::montreal(machine_config &config)
{
	roma2(config);

	// basic machine hardware
	m_maincpu->set_clock(12.288_MHz_XTAL);

	const attotime irq_period = attotime::from_hz(12.288_MHz_XTAL / 0x14000); // 150Hz
	m_maincpu->set_periodic_int(FUNC(roma2_state::irq5_line_hold), irq_period);

	config.set_default_layout(layout_mephisto_montreal);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( roma2 )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("roma_ii_u_ver.4.02", 0x00000, 0x08000, CRC(89e95f5f) SHA1(6c3992f35fba2c1cc08a93f50aa991d5ffda5bc3) )
	ROM_LOAD16_BYTE("roma_ii_l_ver.4.02", 0x00001, 0x08000, CRC(74b03889) SHA1(9d2a09b93f3b2dc483b4f30db134f83deb3aa951) )
ROM_END

ROM_START( montreal )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("montreal_even_5.01.u3", 0x00000, 0x08000, CRC(b32a5db4) SHA1(02a41732d80d91a984814acfae7fec0a8522c13d) )
	ROM_LOAD16_BYTE("montreal_odd_5.01.u2",  0x00001, 0x08000, CRC(a60e2808) SHA1(8a2d404548a934573be125b0cdb91285569b8f72) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, roma2,     0,      0,      roma2,    roma2,    roma2_state, empty_init, "Hegener + Glaser", "Mephisto Roma II", MACHINE_SUPPORTS_SAVE )
SYST( 1993, montreal,  0,      0,      montreal, montreal, roma2_state, empty_init, "Hegener + Glaser", "Mephisto Montreal 68000", MACHINE_SUPPORTS_SAVE )
