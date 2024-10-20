// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

SciSys President Chess (model 231)
(not to be confused with Saitek Kasparov President)

The ROMs are inside a module, at the top-right. No known upgrades were released.
Apparently the chessboard was not that reliable. The manual even says to flip the
computer and shake it when one of the sensors is malfunctioning.

Hardware notes:
- 6502A @ 2MHz
- 16KB ROM(2*HN482764G), 2KB RAM(HM6116P-4)
- buzzer, 64+12 leds, magnet sensors chessboard

TODO:
- verify CPU speed / XTAL
- measure interrupt frequency
- the manual claims that it does a self-test at boot, but on MAME that only
  happens if you hold down one of the buttons

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"
#include "speaker.h"

// internal artwork
#include "saitek_prschess.lh"


namespace {

class prschess_state : public driver_device
{
public:
	prschess_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void prschess(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { };

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void leds_w(offs_t offset, u8 data);
	void control_w(u8 data);
	u8 input_r();
};

void prschess_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void prschess_state::update_display()
{
	u16 led_data = m_led_data[1] << 8 | m_led_data[0];
	led_data = bitswap<16>(led_data,15,14,5,4,3,2,1,0, 7,6,13,12,11,10,9,8);
	m_display->matrix(1 << m_inp_mux, led_data);
}

void prschess_state::leds_w(offs_t offset, u8 data)
{
	m_led_data[offset >> 8] = ~data;
	update_display();
}

void prschess_state::control_w(u8 data)
{
	// d0-d3: input mux, led select
	m_inp_mux = data & 0xf;
	update_display();

	// d5: speaker out
	m_dac->write(BIT(data, 5));

	// other: ?
}

u8 prschess_state::input_r()
{
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 11)
		data = m_inputs[m_inp_mux - 8]->read();

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void prschess_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x4000, 0x4000).select(0x0100).w(FUNC(prschess_state::leds_w));
	map(0x4200, 0x4200).w(FUNC(prschess_state::control_w));
	map(0x4300, 0x4300).r(FUNC(prschess_state::input_r));
	map(0xc000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( prschess )
	PORT_START("IN.0")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Interrupt")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Legal")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("Player V Computer")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Player V Player")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Change Sides")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Clear Board")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("White")

	PORT_START("IN.2")
	PORT_BIT(0x03, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void prschess_state::prschess(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 2'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &prschess_state::main_map);
	m_maincpu->set_periodic_int(FUNC(prschess_state::nmi_line_pulse), attotime::from_hz(100)); // guessed

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8+1, 16);
	config.set_default_layout(layout_saitek_prschess);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( prschess )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("y03_rl", 0xc000, 0x2000, CRC(862c3f42) SHA1(e2d2f1d7a0382b0774e86ca83e270dab1df700c2) ) // HN482764G
	ROM_LOAD("y03_rh", 0xe000, 0x2000, CRC(ef95cb9f) SHA1(02f763cf9cab1b4be8964ddb5d93efb05a898123) ) // "
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, prschess, 0,      0,      prschess, prschess, prschess_state, empty_init, "SciSys / Heuristic Software", "President Chess", MACHINE_SUPPORTS_SAVE )
