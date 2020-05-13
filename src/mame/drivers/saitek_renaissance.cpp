// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

Saitek Kasparov Renaissance

Saitek's 2nd version modular chesscomputer. It accepts the same modules as
Leonardo/Galileo. "OSA" version for Renaissance is 1.5.

Hardware notes:
- 6301Y0(mode 1) or HD6303YP MCU @ 10MHz
- 8KB RAM, 32KB ROM
- "HELIOS" I/O (NEC gate array)
- Seiko Epson SED1502F, LCD screen
- magnet sensors chessboard with 81 leds

The 6301Y0 seen on one of them, was a SX8A 6301Y0G84P, this is in fact the
MCU(+internal maskrom, disabled here) used in Saitek Conquistador.

The LCD screen is fairly large, it's the same one as in Saitek Simultano,
so a chessboard display + 7seg info.

TODO:
- LCD (need SVG screen)
- not sure about comm/module leds
- make it a subdriver of saitek_leonardo.cpp? or too many differences
- same TODO list as saitek_leonardo.cpp

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"
#include "video/sed1500.h"

#include "speaker.h"

// internal artwork
#include "saitek_renaissance.lh" // clickable


namespace {

class ren_state : public driver_device
{
public:
	ren_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void ren(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	optional_device<dac_bit_interface> m_dac;
	required_ioport_array<8+1> m_inputs;

	void main_map(address_map &map);

	void update_display();
	void mux_w(u8 data);
	void leds_w(u8 data);
	void control_w(u8 data);
	u8 control_r();

	u8 p2_r();
	void p2_w(u8 data);
	u8 p5_r();
	void p5_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { 0, 0 };
};

void ren_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// misc

void ren_state::update_display()
{
	m_display->matrix_partial(0, 9, 1 << (m_inp_mux & 0xf), (m_inp_mux << 4 & 0x100) | m_led_data[0], false);
	m_display->matrix_partial(9, 1, 1, (m_inp_mux >> 2 & 0x30) | m_led_data[1], true);
}

void ren_state::mux_w(u8 data)
{
	// d0-d3 input/chessboard led mux
	// d4: chessboard led data
	// d6,d7: mode led
	m_inp_mux = data;
	update_display();

	// d5: ?
}

void ren_state::leds_w(u8 data)
{
	// chessboard led data
	m_led_data[0] = data;
	update_display();
}

void ren_state::control_w(u8 data)
{
	// d1: speaker out
	m_dac->write(BIT(data, 1));

	// d2,d3: comm/module leds?
	m_led_data[1] = (m_led_data[1] & ~0xc) | (~data & 0xc);
	update_display();

	// d6: power off?

	// other: ?
}

u8 ren_state::control_r()
{
	// d5,d6: freq sel?
	// d7: ?
	return 0;
}


// MCU ports

u8 ren_state::p2_r()
{
	u8 data = 0;

	// d0-d2: multiplexed inputs
	if (~m_inp_mux & 8)
		data = m_inputs[m_inp_mux & 7]->read();

	// d3: ?

	return ~data;
}

void ren_state::p2_w(u8 data)
{
	// d5,d6: b/w leds
	m_led_data[1] = (m_led_data[1] & ~3) | (~data >> 5 & 3);
	update_display();
}

u8 ren_state::p5_r()
{
	// d6: battery status
	u8 b = m_inputs[8]->read() & 0x40;

	// other: ?
	return b | 0xbf;
}

void ren_state::p5_w(u8 data)
{
	// ?
}

u8 ren_state::p6_r()
{
	// read chessboard sensors
	return ~m_board->read_file(m_inp_mux & 0xf);
}

void ren_state::p6_w(u8 data)
{
	// module data
}



/******************************************************************************
    Address Maps
******************************************************************************/

void ren_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0040, 0x013f).ram(); // internal
	map(0x2000, 0x2000).w(FUNC(ren_state::mux_w));
	map(0x2400, 0x2400).w(FUNC(ren_state::leds_w));
	map(0x2600, 0x2600).rw(FUNC(ren_state::control_r), FUNC(ren_state::control_w));
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x607f).w("lcd", FUNC(sed1502_device::write));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( ren )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // king
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // rook
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // knight

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // queen
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // bishop
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // pawn

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // scroll?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // tab
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) // +

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // n
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // function?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // sound

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // n
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // stop?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // library

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // info
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // play?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) // level

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // -
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // normal?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) // analysis

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) // n
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // new game
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) // setup

	PORT_START("IN.8")
	PORT_CONFNAME( 0x40, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x40, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void ren_state::ren(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ren_state::main_map);
	m_maincpu->in_p2_cb().set(FUNC(ren_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(ren_state::p2_w));
	m_maincpu->in_p5_cb().set(FUNC(ren_state::p5_r));
	m_maincpu->out_p5_cb().set(FUNC(ren_state::p5_w));
	m_maincpu->in_p6_cb().set(FUNC(ren_state::p6_r));
	m_maincpu->out_p6_cb().set(FUNC(ren_state::p6_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	SED1502(config, "lcd", 32768);
	PWM_DISPLAY(config, m_display).set_size(9+1, 9);
	config.set_default_layout(layout_saitek_renaissance);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( renaissa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw7_518d_u3.u3", 0x8000, 0x8000, CRC(21d2405f) SHA1(6ddcf9bdd30aa446fcaeab919a8f950dc3428365) ) // HN27C256AG-10
ROM_END

ROM_START( renaissaa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx7_518b.u3", 0x8000, 0x8000, CRC(a0c3ffe8) SHA1(fa170a6d4d54d41de77e0bb72f969219e6f376af) ) // MBM27C256H-10
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP  MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1989, renaissa,  0,        0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1989, renaissaa, renaissa, 0,   ren,    ren,  ren_state, empty_init, "Saitek", "Kasparov Renaissance (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
