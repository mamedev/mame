// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle
/*******************************************************************************

SciSys Travel Sensor Chess (aka Travel Sensor)

The chess engine was written by Mark Taylor, employee of Intelligent Software
(formerly known as Philidor Software). The I/O is very similar to CXG Sensor
Computachess (see cxg/computachess.cpp).

Hardware notes:
- PCB label: SCISYS TC-A, 201148
- Hitachi 44801A85 MCU @ ~400kHz (R=91K) or ~350Hz (R=150K)
- piezo, 21 leds, button sensors chessboard

44801A85 MCU is used in:
- SciSys Travel Sensor Chess
- SciSys Travel Mate Chess
- SciSys Chess Partner 5000
- SciSys Chess Partner 6000

TODO:
- add memory switch (it goes to the HLT pin)

*******************************************************************************/

#include "emu.h"

#include "cpu/hmcs40/hmcs40.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_tschess.lh"


namespace {

class tschess_state : public driver_device
{
public:
	tschess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void tschess(machine_config &config);

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
	required_ioport_array<2> m_inputs;

	u8 m_inp_mux = 0;

	template<int N> void mux_w(u8 data);
	void control_w(u16 data);
	u16 input_r();
};

void tschess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

template<int N>
void tschess_state::mux_w(u8 data)
{
	// R2x,R3x: input mux, led data
	m_inp_mux = (m_inp_mux & ~(0xf << (N*4))) | ((data ^ 0xf) << (N*4));
	m_display->write_mx(m_inp_mux);
}

void tschess_state::control_w(u16 data)
{
	// D1-D3: led select
	m_display->write_my(~data >> 1 & 7);

	// D4: speaker out
	m_dac->write(BIT(data, 4));
}

u16 tschess_state::input_r()
{
	u16 data = 0;

	// D6,D7: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 0x40 << i;

	// D8-D15: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_file(i ^ 7) << 8;

	return ~data;
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tschess )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White/Black")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.1")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Compute")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Up")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, tschess_state, reset_button, 0) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void tschess_state::tschess(machine_config &config)
{
	// basic machine hardware
	HD44801(config, m_maincpu, 400'000); // approximation
	m_maincpu->write_r<2>().set(FUNC(tschess_state::mux_w<0>));
	m_maincpu->write_r<3>().set(FUNC(tschess_state::mux_w<1>));
	m_maincpu->write_d().set(FUNC(tschess_state::control_w));
	m_maincpu->read_d().set(FUNC(tschess_state::input_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_saitek_tschess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( tschess )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD("44801a85_scisys_tc-1982.u1", 0x0000, 0x2000, CRC(3ed0253a) SHA1(a3352758285292cfb0ad66e095cc951113332ced) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, tschess, 0,      0,      tschess, tschess, tschess_state, empty_init, "SciSys / Intelligent Software", "Travel Sensor Chess", MACHINE_SUPPORTS_SAVE )
