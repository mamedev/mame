// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

SciSys Chess Companion

The 1st one (this) has a 6504, the 'sequels' II and III have a 6301.
Not to be confused with Kasparov Chess Companion.

Hardware notes:
- PCB label: SciSys Y01 A
- Synertek 6504 @ ~1MHz
- Synertek SY6520/SY6820 PIA
- 4KB ROM (2332N, chip manufacturer unknown)
- 1KB RAM (2*HM472114P-4)
- beeper, button sensors chessboard, 16+12 leds

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6504.h"
#include "machine/6821pia.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_companion.lh"


namespace {

class compan_state : public driver_device
{
public:
	compan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void compan(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_data = 0;
	u8 m_led_direct = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void control_w(u8 data);
	u8 input_r();
	void sled_w(int state);
	void cled_w(int state);
};

void compan_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
	save_item(NAME(m_led_direct));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void compan_state::update_display()
{
	m_display->matrix((1 << m_inp_mux & 0x3ff) | (m_led_direct << 10), m_led_data);
}

void compan_state::sled_w(int state)
{
	// CA2: "sides swapped" led
	m_led_direct = (m_led_direct & ~2) | (state ? 2 : 0);
	update_display();
}

void compan_state::cled_w(int state)
{
	// CB2: "color" led
	m_led_direct = (m_led_direct & ~1) | (state ? 1 : 0);
	update_display();
}

void compan_state::control_w(u8 data)
{
	// PB0-PB3: input mux, led select
	m_inp_mux = data & 0xf;

	// PB4-PB6: led data
	m_led_data = ~data >> 4 & 7;
	update_display();

	// PB7: speaker out
	m_dac->write(BIT(data, 7));
}

u8 compan_state::input_r()
{
	u8 data = 0;

	// PA0-PA7: multiplexed inputs
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux - 8]->read();

	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void compan_state::main_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x0803).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1000, 0x1fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( compan )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Swap Sides")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Color")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Play")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Clear Board")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Change Level")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void compan_state::compan(machine_config &config)
{
	// basic machine hardware
	M6504(config, m_maincpu, 1'000'000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &compan_state::main_map);

	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(compan_state::input_r));
	m_pia->writepb_handler().set(FUNC(compan_state::control_w));
	m_pia->ca2_handler().set(FUNC(compan_state::sled_w));
	m_pia->cb2_handler().set(FUNC(compan_state::cled_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10+2, 3);
	config.set_default_layout(layout_saitek_companion);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( compan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("2332n_yo1a", 0x1000, 0x1000, CRC(a715d51c) SHA1(3e1bd9dc119c914b502f1433ee2d6ce3f477b99a) ) // 2332
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, compan, 0,      0,      compan,  compan, compan_state, empty_init, "SciSys / Heuristic Software", "Chess Companion", MACHINE_SUPPORTS_SAVE )
