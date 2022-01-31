// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

SciSys Kasparov Leonardo, Saitek Kasparov Galileo.

This is SciSys's answer to H+G Mephisto modular chesscomputers, but unlike the
Mephistos, these boards are actual chesscomputers and not an accessory.

They called the expansion capability "OSA", for "Open Systems Architecture".
A serial port for linking to a PC, and a parallel port for expansion modules.
The expansion modules are basically entire chesscomputers, making the whole
thing combined a 'dual brain' chesscomputer. The embedded chess engine is by
Julio Kaplan and Craig Barnes, same as the one in SciSys Turbo S-24K.

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

Known expansion modules:
- Maestro (65C02, Kaplan/Barnes)
- Analyst (65C02, Kaplan/Barnes)
- Brute Force (prototype) (65C02?, Ulf Rathsman)
- Brute Force (H8, Frans Morsch)
- Sparc (SPARClite, Spracklen's)

The H8 Brute Force module doesn't work with the 1st program version of Leonardo,
this is mentioned in the repair manual and it says it requires an EPROM upgrade.
The Sparc module doesn't appear to work with it either. Moreover, the Sparc module
manual mentions that for it to work properly on Leonardo, the chesscomputer needs
to be upgraded with an EMI PCB (power supply related).

TODO:
- OSA module comms is not completely understood
- OSA PC link, uses MCU serial interface
- add nvram (MCU port $14?)
- add power-off, not useful with missing nvram support

******************************************************************************/

#include "emu.h"

#include "bus/saitek_osa/expansion.h"
#include "cpu/m6800/m6801.h"
#include "machine/input_merger.h"
#include "machine/sensorboard.h"
#include "sound/spkrdev.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "saitek_galileo.lh" // clickable
#include "saitek_leonardo.lh" // clickable


namespace {

class leo_state : public driver_device
{
public:
	leo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_expansion(*this, "exp"),
		m_stb(*this, "stb"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void leonardo(machine_config &config);
	void leonardoa(machine_config &config);
	void galileo(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<hd6303y_cpu_device> m_maincpu;
	required_device<saitekosa_expansion_device> m_expansion;
	required_device<input_merger_device> m_stb;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	optional_device<speaker_sound_device> m_dac;
	required_ioport_array<9> m_inputs;

	void main_map(address_map &map);

	void update_display();
	void mux_w(u8 data);
	void leds_w(u8 data);
	u8 unk_r();
	void unk_w(u8 data);
	void exp_rts_w(int state);

	u8 p2_r();
	void p2_w(u8 data);
	u8 p6_r();
	void p5_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);

	u8 m_inp_mux = 0;
	u8 m_led_data[2] = { 0, 0 };
};

void leo_state::machine_start()
{
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}

void leo_state::machine_reset()
{
	m_stb->in_clear<0>();
}



/******************************************************************************
    I/O
******************************************************************************/

// misc

void leo_state::update_display()
{
	m_display->matrix_partial(0, 8, 1 << (m_inp_mux & 0xf), m_led_data[0]);
	m_display->matrix_partial(8, 2, 1 << BIT(m_inp_mux, 5), (~m_inp_mux << 2 & 0x300) | m_led_data[1]);
}

void leo_state::mux_w(u8 data)
{
	// d0-d3: input/chessboard led mux
	// d5: button led select
	// d6,d7: button led data
	m_inp_mux = data;
	update_display();

	// d4: speaker out
	m_dac->level_w(BIT(data, 4));
}

void leo_state::leds_w(u8 data)
{
	// button led data
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

void leo_state::exp_rts_w(int state)
{
	// NAND with ACK-P (not used by chesscomputer?)
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
	// d1: N/C, d4: IS strobe (handled with inputline)
	return 0xff ^ 0x10;
}

void leo_state::p5_w(u8 data)
{
	// d2: expansion NMI-P
	m_expansion->nmi_w(BIT(data, 2));

	// d3: NAND with STB-P
	m_stb->in_w<1>(BIT(data, 3));

	// d5: expansion ACK-P
	m_expansion->ack_w(BIT(data, 5));

	// d6,d7: chessboard led row data
	m_led_data[0] = (m_led_data[0] & 3) | (~data >> 4 & 0xc);
	update_display();

	// d0: power-off on falling edge
	m_expansion->pw_w(data & 1);
}

u8 leo_state::p6_r()
{
	// read chessboard sensors and module data
	return ~m_board->read_file(m_inp_mux & 0xf) & m_expansion->data_r();
}

void leo_state::p6_w(u8 data)
{
	// module data
	m_expansion->data_w(data);
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

static INPUT_PORTS_START( leonardo )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Library")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Info")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Normal")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Analysis")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) // freq sel
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Set Up")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_CONFNAME( 0x04, 0x04, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x04, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F1) PORT_NAME("ACL")
INPUT_PORTS_END

static INPUT_PORTS_START( galileo ) // same buttons, but different locations
	PORT_INCLUDE( leonardo )

	PORT_MODIFY("IN.2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Sound")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Library")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("Info")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Play")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_L) PORT_NAME("Level")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Normal")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Analysis")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("Set Up")

	PORT_MODIFY("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Go")
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void leo_state::leonardo(machine_config &config)
{
	// basic machine hardware
	HD6303Y(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &leo_state::main_map);
	m_maincpu->in_p2_cb().set(FUNC(leo_state::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(leo_state::p2_w));
	m_maincpu->in_p5_cb().set(FUNC(leo_state::p5_r));
	m_maincpu->out_p5_cb().set(FUNC(leo_state::p5_w));
	m_maincpu->in_p6_cb().set(FUNC(leo_state::p6_r));
	m_maincpu->out_p6_cb().set(FUNC(leo_state::p6_w));

	INPUT_MERGER_ANY_LOW(config, m_stb);
	m_stb->output_handler().set_inputline(m_maincpu, M6801_IS_LINE);

	config.set_maximum_quantum(attotime::from_hz(6000));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(8+2, 8+2);
	config.set_default_layout(layout_saitek_leonardo);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	SPEAKER_SOUND(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// expansion module
	SAITEKOSA_EXPANSION(config, m_expansion, saitekosa_expansion_modules);
	m_expansion->stb_handler().set(m_stb, FUNC(input_merger_device::in_w<0>));
	m_expansion->rts_handler().set(FUNC(leo_state::exp_rts_w));
}

void leo_state::leonardoa(machine_config &config)
{
	leonardo(config);
	m_board->set_delay(attotime::from_msec(250)); // slower chessboard response?
}

void leo_state::galileo(machine_config &config)
{
	leonardo(config);
	config.set_default_layout(layout_saitek_galileo);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( leonardo ) // OSA version string: Version 1.4
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // D27C256AD-12
ROM_END

ROM_START( leonardoa ) // OSA version string: Leonardo Chess System - Version 1.2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sx6_617l_osa1.2.u9", 0x8000, 0x8000, CRC(4620f827) SHA1(4ae566646d032dd5bcca48316dd90a11e06772f1) ) // D27C256AD-12
ROM_END

ROM_START( leonardob ) // OSA version string: Leonardo Chess System - Version 1.0
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6-830b.u9", 0x8000, 0x8000, CRC(dc892c1b) SHA1(5f7a92080a4062e1de61c7273a2fd0cfd9ede9f3) ) // D27C256AD-15
ROM_END

ROM_START( galileo ) // OSA version string: Version 1.4
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sw6.1_osa1.4_510.u9", 0x8000, 0x8000, CRC(e39676b2) SHA1(288c5f2608277cb4c3ca71cb2e642a6a62c01dca) ) // MBM27C256H-10
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP  MACHINE    INPUT      CLASS      INIT        COMPANY, FULLNAME, FLAGS
CONS( 1986, leonardo,  0,        0,   leonardo,  leonardo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (v1.4)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, leonardoa, leonardo, 0,   leonardoa, leonardo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (v1.2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, leonardob, leonardo, 0,   leonardoa, leonardo,  leo_state, empty_init, "SciSys", "Kasparov Leonardo (v1.0)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1988, galileo,   leonardo, 0,   galileo,   galileo,   leo_state, empty_init, "Saitek", "Kasparov Galileo (v1.4)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
