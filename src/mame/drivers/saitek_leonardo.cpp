// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys Kasparov Leonardo, Saitek Kasparov Galileo.

This is SciSys's answer to H+G Mephisto modular chesscomputers, but unlike the
Mephistos, these boards are actual chesscomputers and not an accessory.

They called the expansion capability "OSA", for "Open Systems Architecture".
One for a link port to a PC, and one for a module slot. The expansion modules
are basically entire chesscomputers, making the whole thing combined a
'dual brain' chesscomputer. The embedded chess engine is by Julio Kaplan,
same as the one in SciSys Turbo S-24K.

Hardware notes:

Leonardo (1986):
- 6301Y0 MCU @ 12MHz
- 32KB ROM(27C256)
- 8KB RAM(M5M5165P-15 or compatible)
- magnet sensors chessboard with 16 leds

The 6301Y0 was seen with internal maskrom serial A96 and B40. It appears to be
running in mode 1 (expanded mode): the internal ROM is disabled and the MCU can
be emulated as if it's a HD6303Y. It's not known what's on the internal ROM,
it could even be from another SciSys chesscomputer.

Galileo (1988):
- HD6303YP MCU @ 12MHz
- almost same as Leonardo

Galileo PCB is different, but essentially it's the same hardware as Leonardo.
The 1.4 ROM is identical to it too, even though it's a different MCU type.
And on the outside, the button panel was redesigned a bit.

Expansion modules released:
- Maestro (65C02, Julio Kaplan)
- Analyst (65C02, Julio Kaplan)
- Brute Force (H8, Frans Morsch)
- Sparc (SPARClite, Spracklen's)

TODO:
- It locks up a short time after you make an input error (eg. on computer's
  turn, enter the wrong move so it will give a low pitch error beep, then hold
  INS to fast-forward and it will lock up) - happens with leonardoa too, but
  after a longer delay. At first glance, it looks like it's caused by inaccurate
  6801 timer emulation. It also locks up when you get checkmated, seems to be
  the same problem as above.
- OSA module support (softwarelist, devices/bus)
- OSA PC link (probably uses MCU serial interface)
- unsure about white/black/check/end/module/comm leds
- add nvram
- finish internal artwork

******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_leonardo.lh" // clickable


namespace {

class leo_state : public driver_device
{
public:
	leo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void leo(machine_config &config);
	void leoa(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	optional_device<dac_bit_interface> m_dac;
	required_ioport_array<9> m_inputs;

	void main_map(address_map &map);

	void update_display();
	void mux_w(u8 data);
	void leds_w(u8 data);
	u8 unk_r();
	void unk_w(u8 data);

	u8 p2_r();
	u8 p5_r();
	u8 p6_r();
	void p2_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { 0, 0 };
};

void leo_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}



/******************************************************************************
    I/O
******************************************************************************/

// misc

void leo_state::update_display()
{
	m_display->matrix_partial(0, 8, 1 << (m_inp_mux & 0xf), m_led_data[0], false);
	m_display->matrix_partial(8, 3, ~m_inp_mux >> 5 & 7, (~m_inp_mux << 3 & 0x700) | m_led_data[1], true);
}

void leo_state::mux_w(u8 data)
{
	// d0-d3: input/chessboard leds mux
	// d5-d7: button leds mux
	m_inp_mux = data;
	update_display();

	// d4: speaker out
	m_dac->write(data >> 4 & 1);
}

void leo_state::leds_w(u8 data)
{
	// button leds data
	m_led_data[1] = ~data;
	update_display();
}

u8 leo_state::unk_r()
{
	// ?
	return 0xff;
}

void leo_state::unk_w(u8 data)
{
	// ?
}


// MCU ports

u8 leo_state::p2_r()
{
	u8 data = 0;

	// d0-d2: multiplexed inputs
	u8 mux = (m_inp_mux & 8) ? 8 : (m_inp_mux & 7);
	data = m_inputs[mux]->read();

	// d3: ?

	return ~data;
}

void leo_state::p2_w(u8 data)
{
	// d5,d6: chessboard led column data
	m_led_data[0] = (m_led_data[0] & ~3) | (~data >> 5 & 3);
	update_display();

	// other: ?
}

u8 leo_state::p5_r()
{
	// ?
	return 0xff ^ 0x10;
}

void leo_state::p5_w(u8 data)
{
	// d6,d7: chessboard led row data
	m_led_data[0] = (m_led_data[0] & 3) | (~data >> 4 & 0xc);
	update_display();

	// d0: power-off
	// other: ?
}

u8 leo_state::p6_r()
{
	// read chessboard sensors
	return ~m_board->read_file(m_inp_mux & 0xf);
}

void leo_state::p6_w(u8 data)
{
	// module data
}



/******************************************************************************
    Address Maps
******************************************************************************/

void leo_state::main_map(address_map &map)
{
	map(0x0000, 0x0027).m(m_maincpu, FUNC(hd6303y_cpu_device::hd6301y_io));
	map(0x0002, 0x0002).rw(FUNC(leo_state::unk_r), FUNC(leo_state::unk_w)); // external
	map(0x0040, 0x013f).ram(); // internal
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x6000).w(FUNC(leo_state::mux_w));
	map(0x7000, 0x7000).w(FUNC(leo_state::leds_w));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( leo )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // king
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // rook
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) // knight

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // queen
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // bishop
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // pawn

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // n
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // tab/color
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) // +

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // freq
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // function?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // sound

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // freq
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // stop?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // library?

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // info
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // play?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) // level

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // -
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // normal?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) // analysis?

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) // n
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // new game
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) // setup?

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) // low battery
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void leo_state::leo(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &leo_state::main_map);
	m_maincpu->in_p2_cb().set(FUNC(leo_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(leo_state::p2_w));
	m_maincpu->in_p5_cb().set(FUNC(leo_state::p5_r));
	m_maincpu->out_p5_cb().set(FUNC(leo_state::p5_w));
	m_maincpu->in_p6_cb().set(FUNC(leo_state::p6_r));
	m_maincpu->out_p6_cb().set(FUNC(leo_state::p6_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(8+3, 8+3);
	config.set_default_layout(layout_saitek_leonardo);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void leo_state::leoa(machine_config &config)
{
	leo(config);

	// slower chessboard response?
	m_board->set_delay(attotime::from_msec(250));
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( leonardo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // D27C256AD-12
ROM_END

ROM_START( leonardoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx6_617l_osa1.2.u9", 0x8000, 0x8000, CRC(4620f827) SHA1(4ae566646d032dd5bcca48316dd90a11e06772f1) ) // D27C256AD-12
ROM_END

ROM_START( galileo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // MBM27C256H-10
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP MACHINE INPUT CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, leonardo,  0,        0,  leo,    leo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
CONS( 1986, leonardoa, leonardo, 0,  leoa,   leo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )

CONS( 1988, galileo,   0,        0,  leo,    leo,  leo_state, empty_init, "Saitek", "Kasparov Galileo", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NOT_WORKING )
