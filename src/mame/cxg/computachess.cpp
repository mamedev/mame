// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

CXG Sensor Computachess (CXG-001 or WA-001)
CXG Portachess, Portachess II, Computachess IV, Sphinx Chess Voyager

Sensor Computachess is White and Allcock's first original chess computer.
Cassia's Chess Mate (aka Computachess) doesn't really count since it was a
bootleg of Fidelity Chess Challenger 10.

It was programmed by Intelligent Software (formerly known as Philidor Software).
After loosening ties with SciSys, Intelligent Software provided the software for
various chess computer companies. The chess engine is by Mark Taylor, it's the
same one as in Mini Chess released by SciSys earlier that year.

Initially, it had a "Sound" button for turning the beeps off. This was later
changed to the more useful "New Game". With Portachess, they added a "Save"
switch which puts the MCU in halt state.

Hardware notes:

Sensor Computachess:
- PCB label: WA 001 600 002
- Hitachi 44801A50 MCU @ ~400kHz
- buzzer, 16 leds, button sensors chessboard

Portachess II:
- PCB label: CXG223-600-001 (main pcb), CXG 211 600 101 (led pcb taken from
  Advanced Star Chess, extra led row unused here)
- Hitachi HD44801C89 MCU @ ~400kHz (serial 202: from Portachess 1985 version)
- rest same as above

HD44801A50 MCU is used in:
- CXG Sensor Computachess (1981 version) - 1st use
- CXG Portachess (1983 version, has "Sound" button)
- CGL GrandMaster Travel Sensory
- Hanimex HCG 1500
- Schneider Sensor Chesspartner MK 3
- Systema Computachess
- Tandy Computerized 8-Level Beginner Sensory Chess

HD44801C89 MCU is used in:
- CXG Portachess (1985 version, "NEW 16 LEVELS") - 1st use
- CXG Sensor Computachess (1985 rerelease, "NEW 16 LEVELS")
- CXG Portachess II (1986)
- CXG Computachess IV (1986)
- CXG Sphinx Chess Voyager? (1992)
- Fidelity Computachess IV
- Fidelity Mini Chess Challenger (same housing as Portachess II)
- Schneider MK 7 (same housing as Portachess II)
- Schneider Sensor Chessmaster MK 6
- Schneider Sensor Chesspartner MK 4

Computachess II has a HD44840 MCU (see computachess2.cpp). Computachess III has
a HD6301V1 MCU and should be the same as Enterprise "S" (see saitek/companion2.cpp).

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_scpchess.lh"
#include "cxg_scpchessa.lh"


namespace {

class cpchess_state : public driver_device
{
public:
	cpchess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	void scpchess(machine_config &config);
	void scpchessa(machine_config &config);

	// New Game button is directly tied to MCU reset
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<hmcs40_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u8 m_inp_mux = 0;

	template<int N> void mux_w(u8 data);
	void control_w(u16 data);
	u16 input_r();
};

void cpchess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

template<int N>
void cpchess_state::mux_w(u8 data)
{
	// R2x,R3x: input mux, led data
	const u8 shift = N * 4;
	m_inp_mux = (m_inp_mux & ~(0xf << shift)) | (data << shift);
	m_display->write_mx(m_inp_mux);
}

void cpchess_state::control_w(u16 data)
{
	// D0: speaker out
	m_dac->write(~data & 1);

	// D2,D3: led select
	m_display->write_my(~data >> 2 & 3);
}

u16 cpchess_state::input_r()
{
	u16 data = 0;

	// D7: read buttons
	if (m_inp_mux & m_inputs->read())
		data |= 0x80;

	// D8-D15: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i ^ 7) << 8;

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( scpchess )
	PORT_START("IN.0")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Reverse Play")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(cpchess_state::reset_button), 0) PORT_NAME("New Game")
INPUT_PORTS_END

static INPUT_PORTS_START( scpchessa )
	PORT_INCLUDE( scpchess )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound") // only hooked up on 1st version

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cpchess_state::scpchess(machine_config &config)
{
	// basic machine hardware
	HD44801(config, m_maincpu, 400'000); // approximation
	m_maincpu->write_r<2>().set(FUNC(cpchess_state::mux_w<0>));
	m_maincpu->write_r<3>().set(FUNC(cpchess_state::mux_w<1>));
	m_maincpu->write_d().set(FUNC(cpchess_state::control_w));
	m_maincpu->read_d().set(FUNC(cpchess_state::input_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 8);
	config.set_default_layout(layout_cxg_scpchess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void cpchess_state::scpchessa(machine_config &config)
{
	scpchess(config);

	m_maincpu->write_d().set(FUNC(cpchess_state::control_w)).exor(1);
	config.set_default_layout(layout_cxg_scpchessa);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( scpchess )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("202_newcrest_16_hd44801c89", 0x0000, 0x2000, CRC(56b48f70) SHA1(84ec62323c6d3314e0515bccfde2f65f6d753e99) )
ROM_END

ROM_START( scpchessa )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("white_allcock_44801a50", 0x0000, 0x2000, CRC(c5c53e05) SHA1(8fa9b8e48ca54f08585afd83ae78fb1970fbd382) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, scpchess,  0,        0,      scpchess,  scpchess,  cpchess_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Chess Software", "Sensor Computachess (1985 version)", MACHINE_SUPPORTS_SAVE )
SYST( 1981, scpchessa, scpchess, 0,      scpchessa, scpchessa, cpchess_state, empty_init, "CXG Systems / White and Allcock / Intelligent Software", "Sensor Computachess (1981 version)", MACHINE_SUPPORTS_SAVE )
