// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

CXG Chess 2001, also sold by Hanimex as Computachess (model HCG 1900),
and by CGL as Computachess Champion.
CXG Chess 3000 is assumed to be on similar hardware as this.

The chess engine is by Richard Lang, based on Cyrus.

CXG Systems S.A. and Newcrest Technology Ltd. are related companies, with
Eric White at the steering wheel. Newcrest(1984-1991) is probably a rename of
"White and Allcock"(1981-1984).

Hardware notes:
- Zilog Z8400APS @ 4 MHz (8MHz XTAL)
- 2KB RAM HM6116, 16KB ROM D27128D
- TTL, piezo, 8*8+9 LEDs, magnetic sensors

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_chess2001.lh"


namespace {

class chess2001_state : public driver_device
{
public:
	chess2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void chess2001(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	u16 m_inp_mux = 0;
	int m_dac_data = 0;

	void main_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void speaker_w(u8 data);
	void leds_w(u8 data);
	u8 input_r();
};

void chess2001_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_dac_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void chess2001_state::speaker_w(u8 data)
{
	// 74ls109 toggle to speaker
	m_dac_data ^= 1;
	m_dac->write(m_dac_data);
}

void chess2001_state::leds_w(u8 data)
{
	// d0-d7: 74ls273 (WR to CLK)
	// 74ls273 Q1-Q4: 74ls145 A-D
	// 74ls145 0-9: input mux/led select
	m_inp_mux = data & 0xf;
	u16 sel = 1 << m_inp_mux & 0x3ff;

	// 74ls273 Q5-Q8: MC14028 A-D
	// MC14028 Q0-Q7: led data, Q8,Q9: N/C
	u8 led_data = 1 << (data >> 4 & 0xf) & 0xff;
	m_display->matrix(sel, led_data);
}

u8 chess2001_state::input_r()
{
	u8 data = 0;

	// d0-d7: multiplexed inputs
	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux, true);

	// read other buttons
	else if (m_inp_mux < 10)
		data = m_inputs[m_inp_mux - 8]->read();

	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chess2001_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).mirror(0x3800).ram();
	map(0x8000, 0x8000).mirror(0x3fff).rw(FUNC(chess2001_state::input_r), FUNC(chess2001_state::leds_w));
	map(0xc000, 0xc000).mirror(0x3fff).w(FUNC(chess2001_state::speaker_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( chess2001 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Queen")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("White")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Set up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Take Back")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Forward")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Hint")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Move")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chess2001_state::chess2001(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &chess2001_state::main_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 568)); // 555 timer (20nF, 100K+33K, 1K2), measured 568Hz
	irq_clock.set_pulse_width(attotime::from_nsec(16600)); // active for 16.6us
	irq_clock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(10, 8);
	config.set_default_layout(layout_cxg_chess2001);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ch2001 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chess2001.bin", 0x0000, 0x4000, CRC(b3485c73) SHA1(f405c6f67fe70edf45dcc383a4049ee6bad387a9) ) // D27128D, no label
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1984, ch2001, 0,      0,      chess2001, chess2001, chess2001_state, empty_init, "CXG Systems / Newcrest Technology / Intelligent Software", "Chess 2001", MACHINE_SUPPORTS_SAVE )
