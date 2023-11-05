// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Chess Companion II family

The chess engine is LogiChess (ported from Z80 to 6800), by Kaare Danielsen.
CXG Enterprise "S" / Star Chess is probably on similar hardware.

TODO:
- standby/battery save mode

********************************************************************************

Hardware notes:

Chess Companion II:
- PCB label: Y01B-01 REV.B
- Hitachi HD6301V1 (0609V171) @ ~3MHz (LC oscillator)
- chessboard buttons, 16+5 leds, piezo

Explorer Chess:
- slightly different UI (12 buttons instead of 14, but same functionality)
- portable, peg board instead of button board
- rest is same as ccompan2

Concord II:
- PCB label: SCISYS ST3 REV.E
- MCU clock frequency is twice higher than Concord
- rest is same as ccompan2, it just has the buttons/status leds at the bottom
  instead of at the right

Explorer Chess and Chess Companion II / Concord have the same MCU ROM, pin P23
is either VCC or GND to distinguish between the two.

0609V171 MCU is used in:
- SciSys Chess Companion II (2 revisions)
- SciSys Olympiade (French)
- SciSys Explorer Chess
- SciSys Concord
- SciSys Concord II
- SciSys Electronic Chess Mark 8
- Tandy 1650 Portable Sensory Chess (Tandy brand Explorer Chess)
- Tandy 1650 Fast Response Time: Computerized Chess (Tandy brand Concord II)

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_ccompan2.lh"
#include "saitek_expchess.lh"


namespace {

class ccompan2_state : public driver_device
{
public:
	ccompan2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void ccompan2(machine_config &config);
	void expchess(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq) { set_cpu_freq(); }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { set_cpu_freq(); }
	DECLARE_MACHINE_RESET(expchess) { ; }

private:
	// devices/pointers
	required_device<hd6301v1_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8+1> m_inputs;

	// address maps
	void main_map(address_map &map);

	// I/O handlers
	u8 input1_r();
	u8 input2_r();
	void speaker_w(u8 data);
	void mux_w(u8 data);
	u8 battery_r();
	void led_w(u8 data);

	void set_cpu_freq();

	u8 m_inp_mux = 0;
};

void ccompan2_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
}

void ccompan2_state::set_cpu_freq()
{
	// Concord II MCU speed is twice higher
	m_maincpu->set_unscaled_clock((ioport("FAKE")->read() & 1) ? 6000000 : 3000000);
}



/*******************************************************************************
    I/O
*******************************************************************************/

u8 ccompan2_state::input1_r()
{
	u8 data = 0;

	// P10-P17: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i ^ 7);

	return ~data;
}

u8 ccompan2_state::input2_r()
{
	u8 data = 0;

	// P21,P22: read buttons
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read() << 1 & 6;

	// P23: button configuration
	data |= m_inputs[8]->read() << 3 & 8;

	return ~data;
}

void ccompan2_state::speaker_w(u8 data)
{
	// P20: speaker out
	m_dac->write(data & 1);
}

void ccompan2_state::mux_w(u8 data)
{
	// P30-P37: input mux, led data
	m_inp_mux = data ^ 0xff;
	m_display->write_mx(m_inp_mux);
}

u8 ccompan2_state::battery_r()
{
	// P40: battery status (only reads it at boot)
	return 0;
}

void ccompan2_state::led_w(u8 data)
{
	// P41-P45: direct leds
	// P46,P47: board leds
	m_display->write_my(~data >> 1 & 0x7f);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void ccompan2_state::main_map(address_map &map)
{
	map(0x0000, 0x0014).m(m_maincpu, FUNC(hd6301v1_cpu_device::m6801_io));
	map(0x0080, 0x00ff).ram(); // internal
	map(0xf000, 0xffff).rom(); // internal
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( ccompan2 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Play")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // button config

	PORT_START("FAKE")
	PORT_CONFNAME( 0x01, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, ccompan2_state, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "3MHz (original)" )
	PORT_CONFSETTING(    0x01, "6MHz (Concord II)" )
INPUT_PORTS_END

static INPUT_PORTS_START( expchess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White/Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game / Clear Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / Sound")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("2nd F")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Play / PVP")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // button config
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void ccompan2_state::ccompan2(machine_config &config)
{
	// basic machine hardware
	HD6301V1(config, m_maincpu, 3000000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &ccompan2_state::main_map);
	m_maincpu->in_p1_cb().set(FUNC(ccompan2_state::input1_r));
	m_maincpu->in_p2_cb().set(FUNC(ccompan2_state::input2_r));
	m_maincpu->out_p2_cb().set(FUNC(ccompan2_state::speaker_w));
	m_maincpu->out_p3_cb().set(FUNC(ccompan2_state::mux_w));
	m_maincpu->in_p4_cb().set(FUNC(ccompan2_state::battery_r));
	m_maincpu->out_p4_cb().set(FUNC(ccompan2_state::led_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(5+2, 8);
	m_display->set_interpolation(0.25);
	config.set_default_layout(layout_saitek_ccompan2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void ccompan2_state::expchess(machine_config &config)
{
	ccompan2(config);

	MCFG_MACHINE_RESET_OVERRIDE(ccompan2_state, expchess)
	config.set_default_layout(layout_saitek_expchess);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ccompan2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("1983_te-1_scisys-w_0609v171.u1", 0xf000, 0x1000, CRC(a26632fd) SHA1(fb83dc2476500acaabd949d749e58adca01012ea) )
ROM_END

ROM_START( expchess )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("1983_te-1_scisys-w_0609v171.u1", 0xf000, 0x1000, CRC(a26632fd) SHA1(fb83dc2476500acaabd949d749e58adca01012ea) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, ccompan2, 0,        0,      ccompan2, ccompan2, ccompan2_state, empty_init, "SciSys", "Chess Companion II", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
SYST( 1983, expchess, ccompan2, 0,      expchess, expchess, ccompan2_state, empty_init, "SciSys", "Explorer Chess", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
