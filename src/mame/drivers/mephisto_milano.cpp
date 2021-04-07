// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/******************************************************************************

Mephisto Milano

Hardware notes:
- RP65C02G or W65C02P-8 @ 4.91MHz
- 8KB RAM(battery-backed), 64KB ROM
- HD44100H, HD44780, 2*16 chars LCD screen
- 8*8 chessboard buttons, 16 leds, piezo

Nigel Short is basically a Milano 2.00

******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "video/mmdisplay2.h"

// internal artwork
#include "mephisto_milano.lh"


namespace {

class mephisto_milano_state : public driver_device
{
public:
	mephisto_milano_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_display(*this, "display")
		, m_keys(*this, "KEY")
	{ }

	void milano(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display_module2_device> m_display;
	required_ioport m_keys;

	void milano_mem(address_map &map);

	u8 keys_r(offs_t offset);
	u8 board_r();
	void io_w(u8 data);
};



/******************************************************************************
    I/O
******************************************************************************/

u8 mephisto_milano_state::keys_r(offs_t offset)
{
	return (BIT(m_keys->read(), offset) << 7) | 0x7f;
}

u8 mephisto_milano_state::board_r()
{
	return m_board->input_r() ^ 0xff;
}

void mephisto_milano_state::io_w(u8 data)
{
	// default display module
	m_display->io_w(data & 0x0f);

	// high bits go to board leds
	m_board->led_w(data >> 4);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void mephisto_milano_state::milano_mem(address_map &map)
{
	map(0x0000, 0x1fbf).ram().share("nvram");
	map(0x1fc0, 0x1fc0).w(m_display, FUNC(mephisto_display_module2_device::latch_w));
	map(0x1fd0, 0x1fd0).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x1fe0, 0x1fe0).r(FUNC(mephisto_milano_state::board_r));
	map(0x1fe8, 0x1fef).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x1fd8, 0x1fdf).r(FUNC(mephisto_milano_state::keys_r));
	map(0x1ff0, 0x1ff0).w(FUNC(mephisto_milano_state::io_w));
	map(0x2000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( milano )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Training / Pawn")   PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Info / Knight")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Memory / Bishop")   PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Position / Rook")   PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Level / Queen")     PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Function / King")   PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Enter / New Game")  PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Clear / New Game")  PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void mephisto_milano_state::milano(machine_config &config)
{
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_milano_state::milano_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_milano_state::nmi_line_pulse), attotime::from_hz(4.9152_MHz_XTAL / (1 << 13)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");

	MEPHISTO_BUTTONS_BOARD(config, m_board); // internal
	MEPHISTO_DISPLAY_MODULE2(config, m_display); // internal
	config.set_default_layout(layout_mephisto_milano);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( milano ) // 1.02
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("milano_b958", 0x0000, 0x10000, CRC(0e9c8fe1) SHA1(e9176f42d86fe57e382185c703c7eff7e63ca711) )
ROM_END

ROM_START( milanoa ) // 1.01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("milano_4af8", 0x0000, 0x10000, CRC(22efc0be) SHA1(921607d6dacf72c0686b8970261c43e2e244dc9f) )
ROM_END

ROM_START( nshort ) // 2.00
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("nshort.bin", 0x00000, 0x10000, CRC(4bd51e23) SHA1(3f55cc1c55dae8818b7e9384b6b8d43dc4f0a1af) )
ROM_END

} // anonymous namespace



/***************************************************************************
    Game Drivers
***************************************************************************/

/*    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT   CLASS                   INIT        COMPANY             FULLNAME                   FLAGS */
CONS( 1991, milano,    0,       0,      milano,   milano, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Milano (v1.02)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, milanoa,   milano,  0,      milano,   milano, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Milano (v1.01)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1993, nshort,    0,       0,      milano,   milano, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Nigel Short", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
