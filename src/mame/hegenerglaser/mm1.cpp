// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Mephisto MM I, the first H+G slide-in chesscomputer module

The module was included with either the Modular or Exclusive chessboards.
Initially, the module itself didn't have a name. It was only later in retrospect,
after the release of Modul MM II that it became known as the MM I. The program is
actually more like a prequel of III-S Glasgow, same chess engine authors too.

Hardware notes:
- PCB label: HGS 10 121 01
- CDP1806 @ 8MHz, 6.5V (IRQ from internal timer)
- 32KB ROM (2*D27128, or HN613256P)
- 4KB RAM (2*HM6116LP-3)
- CDP1853CE, CD4011BE, 3*40373BP, 4556BE
- Mephisto modular display module
- Mephisto modular chessboard
- 18-button keypad, beeper

It supports the HG 170 opening book module.
LCD module is assumed to be same as MM II and others.

Mephisto Mirage is on similar hardware, but it's a single module (LCD is included
on the main PCB). Like MM I, the module by itself didn't have a name at first.
The boards that were included with the product were either Mephisto Mirage
(button sensors, no leds), or Mephisto Mobil (no sensors, no leds). The module
also works on the more expensive wooden chessboards like Modular, Exclusive or
Muenchen, as long as it supports the higher voltage.

TODO:
- mmirage unknown_w
- mm1 unknown expansion rom at $c000?
- add mm1 STP/ON buttons? (they're off/on, game continues when ON again)

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/cosmac/cosmac.h"
#include "sound/dac.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "mephisto_mm1.lh"
#include "mephisto_mirage.lh"


namespace {

class mm1_state : public driver_device
{
public:
	mm1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(mirage_switch_sensor_type);

	// machine configs
	void mirage(machine_config &config);
	void mm1(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cdp1806_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;

	bool m_reset = false;
	u8 m_kp_mux = 0;

	// address maps
	void mirage_map(address_map &map);
	void mm1_map(address_map &map);
	void mm1_io(address_map &map);

	// I/O handlers
	void update_display();
	int clear_r();
	void sound_w(u8 data);
	void unknown_w(u8 data);
	void keypad_w(u8 data);
	template<int N> int keypad_r();
};

void mm1_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_reset));
	save_item(NAME(m_kp_mux));
}

void mm1_state::machine_reset()
{
	m_reset = true;
}



/*******************************************************************************
    I/O
*******************************************************************************/

int mm1_state::clear_r()
{
	// CLEAR low + WAIT high resets cpu
	int ret = (m_reset) ? 0 : 1;
	m_reset = false;
	return ret;
}

void mm1_state::sound_w(u8 data)
{
	// d0: speaker out
	m_dac->write(~data & 1);
}

void mm1_state::unknown_w(u8 data)
{
	// mmirage: unused serial device?
}

void mm1_state::keypad_w(u8 data)
{
	// d0-d7: keypad input mux
	m_kp_mux = ~data;
}

template<int N>
int mm1_state::keypad_r()
{
	// EF3,EF4: multiplexed inputs (keypad)
	return (m_inputs[N]->read() & m_kp_mux) ? 1 : 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mm1_state::mirage_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().nopw();
	map(0xd000, 0xdfff).mirror(0x2000).ram();
}

void mm1_state::mm1_map(address_map &map)
{
	mirage_map(map);
	map(0x8000, 0xbfff).r("cartslot", FUNC(generic_slot_device::read_rom)); // opening library
	map(0xc000, 0xc000).nopr(); // looks for $c0, jumps to $c003 if true
}

void mm1_state::mm1_io(address_map &map)
{
	map(0x01, 0x01).w(FUNC(mm1_state::sound_w));
	map(0x02, 0x02).w(FUNC(mm1_state::keypad_w));
	map(0x03, 0x03).r(m_board, FUNC(mephisto_board_device::input_r));
	map(0x04, 0x04).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x05, 0x05).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0x06, 0x06).w("display", FUNC(mephisto_display1_device::data_w));
	map(0x07, 0x07).w(FUNC(mm1_state::unknown_w));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mm1 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INFO")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right / White / 0")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("POS")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / King")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MEM")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left / Black / 9")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Knight")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Pawn")
INPUT_PORTS_END

static INPUT_PORTS_START( mirage )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A / 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("ENT")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B / 2 / Pawn")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("STA")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C / 3 / Knight")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LEV")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D / 4 / Bishop")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("LIST")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E / 5 / Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("Black / 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F / 6 / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("White / 0")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G / 7 / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("REV")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H / 8")

	PORT_START("FAKE") // module came with buttons sensorboard by default
	PORT_CONFNAME( 0x01, 0x00, "Board Sensors" ) PORT_CHANGED_MEMBER(DEVICE_SELF, mm1_state, mirage_switch_sensor_type, 0)
	PORT_CONFSETTING(    0x00, "Buttons (Mirage)" )
	PORT_CONFSETTING(    0x01, "Magnets (Modular)" )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(mm1_state::mirage_switch_sensor_type)
{
	m_board->get()->set_type(newval ? sensorboard_device::MAGNETS : sensorboard_device::BUTTONS);
}



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mm1_state::mirage(machine_config &config)
{
	// basic machine hardware
	CDP1806(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm1_state::mirage_map);
	m_maincpu->set_addrmap(AS_IO, &mm1_state::mm1_io);
	m_maincpu->clear_cb().set(FUNC(mm1_state::clear_r));
	m_maincpu->q_cb().set("display", FUNC(mephisto_display1_device::strobe_w)).invert();
	m_maincpu->ef3_cb().set(FUNC(mm1_state::keypad_r<0>));
	m_maincpu->ef4_cb().set(FUNC(mm1_state::keypad_r<1>));

	MEPHISTO_BUTTONS_BOARD(config, m_board); // see mirage_switch_sensor_type
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, "display");
	config.set_default_layout(layout_mephisto_mirage);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void mm1_state::mm1(machine_config &config)
{
	mirage(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mm1_state::mm1_map);
	m_maincpu->q_cb().set("display", FUNC(mephisto_display1_device::strobe_w));

	MEPHISTO_SENSORS_BOARD(config.replace(), m_board);
	m_board->set_delay(attotime::from_msec(200));

	config.set_default_layout(layout_mephisto_mm1);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mephisto_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mm1");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mm1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mm1b.bin", 0x0000, 0x8000, CRC(90bf840e) SHA1(cdec6b02c1352b2a00d66964989a17c2b81ec79e) ) // HN613256P
ROM_END

ROM_START( mm1a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("114", 0x0000, 0x4000, CRC(208b4c43) SHA1(48f891d614fa643f47d099f94aff15a44c2efc07) ) // D27128
	ROM_LOAD("214", 0x4000, 0x4000, CRC(93734e49) SHA1(9ad6c191074c4122300f059e2ef9cfeff7b81463) ) // "
ROM_END


ROM_START( mmirage )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("g79", 0x0000, 0x8000, CRC(8cbaff40) SHA1(693086ae179f1ada4ac403b3a6bc7ea718b4e71e) ) // HN613256P, 2nd half empty
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE INPUT   CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1983, mm1,     0,      0,      mm1,    mm1,    mm1_state, empty_init, "Hegener + Glaser", "Mephisto MM I (ver. B)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, mm1a,    mm1,    0,      mm1,    mm1,    mm1_state, empty_init, "Hegener + Glaser", "Mephisto MM I (ver. A)", MACHINE_SUPPORTS_SAVE )

SYST( 1984, mmirage, 0,      0,      mirage, mirage, mm1_state, empty_init, "Hegener + Glaser", "Mephisto Mirage", MACHINE_SUPPORTS_SAVE )
