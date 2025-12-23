// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Gomoku Computer

Five in a row electronic board game. The board size is 13x13 instead of the more
common 15x15. No additional Renju rules, except for no overlines (eg. 6 in a row
doesn't count as a win).

For faster interface control in MAME, map the piece spawn inputs to your keyboard.

NOTE: If internal artwork Japanese text is not visible (either fontprovider or OS
doesn't support character substitution), manually set the -artfont option to a CJK
font.

Hardware notes:
- PCB label: 100015
- Zilog Z8400B PS @ ~2.25MHz
- 4KB ROM (HN462732G), 1KB RAM (2*TMM314APL-1)
- 13*13 sensorboard buttons, 26+8 leds, piezo

The Z80 CLK is from a weird circuit with a 7404 and a capacitor. A Z80B is rated
6MHz, the unexpectedly low frequency here was confirmed on two PCBs. One of them
was measured ~2.1MHz, and the other was measured ~2.3MHz.

BTANB:
- Occasionally, it doesn't register a button press, including sensorboard buttons.
  Simply press it again if it didn't beep. It's not keybounce related, increasing
  sensorboard delay doesn't resolve it either.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_gomoku.lh"


namespace {

class gomoku_state : public driver_device
{
public:
	gomoku_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void gomoku(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<3> m_inputs;

	u8 m_select = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	// I/O handlers
	u16 read_inputs();
	u8 input1_r();
	u8 input2_r();
	void select_w(u8 data);
	void control_w(u8 data);
};

void gomoku_state::machine_start()
{
	save_item(NAME(m_select));
}



/*******************************************************************************
    I/O
*******************************************************************************/

u16 gomoku_state::read_inputs()
{
	// read board
	if (m_select < 13)
		return m_board->read_file(m_select);

	// read buttons
	else if (m_select < 15)
		return m_inputs[m_select - 13]->read();

	return 0;
}

u8 gomoku_state::input1_r()
{
	// d2: battery status
	u8 data = m_inputs[2]->read() << 2 & 4;

	// d3-d7: inputs high
	data |= read_inputs() >> 5 & 0xf8;
	return ~data;
}

u8 gomoku_state::input2_r()
{
	// d0-d7: inputs low
	return ~read_inputs() & 0xff;
}

void gomoku_state::select_w(u8 data)
{
	// d0-d3: 74154 to input/led select
	m_select = data & 0xf;
	m_display->write_my(1 << m_select);
}

void gomoku_state::control_w(u8 data)
{
	// d0-d5: led data
	m_display->write_mx(data & 0x3f);

	// d6,d7: speaker out
	m_dac->write(data >> 6 & 3);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void gomoku_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0fff).mirror(0x1000).rom();
	map(0x4000, 0x43ff).mirror(0x1c00).ram();
}

void gomoku_state::io_map(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x00).r(FUNC(gomoku_state::input1_r));
	map(0x01, 0x01).r(FUNC(gomoku_state::input2_r));
	map(0x02, 0x02).w(FUNC(gomoku_state::select_w));
	map(0x03, 0x03).w(FUNC(gomoku_state::control_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( gomoku )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Trace Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Trace Forward")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Warning")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Change Color")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Verify / Set Up")

	PORT_START("IN.2")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x01, "Low" ) // leds will intentionally flicker
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void gomoku_state::gomoku(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 2'250'000); // approximation, no XTAL
	m_maincpu->set_addrmap(AS_PROGRAM, &gomoku_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gomoku_state::io_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 1463)); // from 555 timer (100nF, 8.2K, 820)
	irq_clock.set_pulse_width(attotime::from_usec(57)); // active for 57us
	irq_clock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->set_size(13, 13);
	m_board->set_spawnpoints(2);
	m_board->set_delay(attotime::from_msec(250));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(6, 6);
	config.set_default_layout(layout_novag_gomoku);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( ngomoku )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("hn462732g.u2", 0x0000, 0x1000, CRC(24efaab7) SHA1(5ecf53718c31fed410b347af5c931b7bbd7d747c) ) // no custom label
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, ngomoku, 0,      0,      gomoku,  gomoku, gomoku_state, empty_init, "Novag Industries", "Gomoku Computer", MACHINE_SUPPORTS_SAVE )
