// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**************************************************************************************************

Mephisto Polgar and RISC

The chess engine in Mephisto Risc is also compatible with Tasc's The ChessMachine.

TODO:
- split driver into several files? mrisc for example is completely different hw

**************************************************************************************************/


#include "emu.h"

#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "machine/chessmachine.h"
#include "video/mmdisplay2.h"

#include "speaker.h"

// internal artwork
#include "mephisto_milano.lh"
#include "mephisto_polgar.lh"


class mephisto_polgar_state : public driver_device
{
public:
	mephisto_polgar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keys(*this, "KEY")
	{ }

	uint8_t polgar_keys_r(offs_t offset);

	void polgar10(machine_config &config);
	void polgar(machine_config &config);
	void polgar_mem(address_map &map);
protected:
	required_device<cpu_device> m_maincpu;
	required_ioport m_keys;
};

class mephisto_risc_state : public mephisto_polgar_state
{
public:
	mephisto_risc_state(const machine_config &mconfig, device_type type, const char *tag)
		: mephisto_polgar_state(mconfig, type, tag)
		, m_chessm(*this, "chessm")
		, m_rombank(*this, "rombank")
	{ }

	uint8_t chessm_r();
	void chessm_w(uint8_t data);

	void mrisc(machine_config &config);
	void mrisc_mem(address_map &map);
protected:
	virtual void machine_start() override;

private:
	required_device<chessmachine_device> m_chessm;
	required_memory_bank m_rombank;
	uint8_t m_bank;
};

class mephisto_milano_state : public mephisto_polgar_state
{
public:
	mephisto_milano_state(const machine_config &mconfig, device_type type, const char *tag)
		: mephisto_polgar_state(mconfig, type, tag)
		, m_board(*this, "board")
		, m_display(*this, "display")
		, m_leds(*this, "led%u", 0U)
	{ }

	uint8_t milano_input_r();
	void milano_led_w(uint8_t data);
	void milano_io_w(uint8_t data);

	void milano(machine_config &config);
	void milano_mem(address_map &map);
protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display_module2_device> m_display;
	output_finder<16> m_leds;
	uint8_t m_led_latch;
};

uint8_t mephisto_polgar_state::polgar_keys_r(offs_t offset)
{
	return (BIT(m_keys->read(), offset) << 7) | 0x7f;
}

void mephisto_polgar_state::polgar_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w("display", FUNC(mephisto_display_module2_device::latch_w));
	map(0x2004, 0x2004).w("display", FUNC(mephisto_display_module2_device::io_w));
	map(0x2400, 0x2400).w("board", FUNC(mephisto_board_device::led_w));
	map(0x2800, 0x2800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c07).r(FUNC(mephisto_polgar_state::polgar_keys_r));
	map(0x3000, 0x3000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x4000, 0xffff).rom();
}


uint8_t mephisto_risc_state::chessm_r()
{
	return m_chessm->data_r();
}

void mephisto_risc_state::chessm_w(uint8_t data)
{
	m_chessm->data0_w(data & 1);
	m_chessm->data1_w(data & 0x80);
	m_chessm->reset_w(data & 2);
}

void mephisto_risc_state::mrisc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w("display", FUNC(mephisto_display_module2_device::latch_w));
	map(0x2004, 0x2004).w("display", FUNC(mephisto_display_module2_device::io_w));
	map(0x2c00, 0x2c07).r(FUNC(mephisto_risc_state::polgar_keys_r));
	map(0x2400, 0x2400).w("board", FUNC(mephisto_board_device::led_w));
	map(0x2800, 0x2800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x3000, 0x3000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x3800, 0x3800).w(FUNC(mephisto_risc_state::chessm_w));
	map(0x3c00, 0x3c00).r(FUNC(mephisto_risc_state::chessm_r));
	map(0x4000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("rombank");
}


uint8_t mephisto_milano_state::milano_input_r()
{
	return m_board->input_r() ^ 0xff;
}

void mephisto_milano_state::milano_led_w(uint8_t data)
{
	m_led_latch = data;
	m_board->mux_w(data);
}

void mephisto_milano_state::milano_io_w(uint8_t data)
{
	if ((data & 0xf0) == 0x90 || (data & 0xf0) == 0x60)
	{
		uint8_t base = (data & 0xf0) == 0x90 ? 0 : 8;
		for(int i=0; i<8; i++)
			m_leds[base + i] = BIT(m_led_latch, i) ? 0 : 1;
	}
	else
	{
		for(int i=0; i<16; i++)
			m_leds[i] = 0;
	}

	m_display->io_w(data & 0x0f);
}

void mephisto_milano_state::milano_mem(address_map &map)
{
	map(0x0000, 0x1fbf).ram().share("nvram");

	map(0x1fc0, 0x1fc0).w(m_display, FUNC(mephisto_display_module2_device::latch_w));
	map(0x1fd0, 0x1fd0).w(FUNC(mephisto_milano_state::milano_led_w));
	map(0x1fe0, 0x1fe0).r(FUNC(mephisto_milano_state::milano_input_r));
	map(0x1fe8, 0x1fef).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x1fd8, 0x1fdf).r(FUNC(mephisto_milano_state::polgar_keys_r));
	map(0x1ff0, 0x1ff0).w(FUNC(mephisto_milano_state::milano_io_w));

	map(0x2000, 0xffff).rom();
}


static INPUT_PORTS_START( polgar )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Trn")    PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Info")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Mem")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("Pos")    PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("LEV")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("FCT")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)     PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END

void mephisto_risc_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);
	save_item(NAME(m_bank));
}

void mephisto_milano_state::machine_start()
{
	m_leds.resolve();
	save_item(NAME(m_led_latch));
}

void mephisto_milano_state::machine_reset()
{
	m_led_latch = 0;
}

void mephisto_polgar_state::polgar(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(4'915'200)); // RP65C02G
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_polgar_state::polgar_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_polgar_state::nmi_line_pulse), attotime::from_hz(XTAL(4'915'200) / (1 << 13)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_polgar);
}

void mephisto_polgar_state::polgar10(machine_config &config)
{
	polgar(config);
	m_maincpu->set_clock(9.8304_MHz_XTAL); // W65C02P-8
	m_maincpu->set_periodic_int(FUNC(mephisto_polgar_state::nmi_line_pulse), attotime::from_hz(9.8304_MHz_XTAL / (1 << 13)));
}

void mephisto_risc_state::mrisc(machine_config &config)
{
	M65SC02(config, m_maincpu, XTAL(10'000'000) / 4); // G65SC02P-4
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_risc_state::mrisc_mem);
	m_maincpu->set_periodic_int(FUNC(mephisto_risc_state::irq0_line_hold), attotime::from_hz(XTAL(10'000'000) / (1 << 14)));

	CHESSMACHINE(config, m_chessm, 14'000'000); // Tasc ChessMachine EC PCB, Mephisto manual says 14MHz (no XTAL)
	config.set_perfect_quantum(m_maincpu);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");
	outlatch.parallel_out_cb().set_membank("rombank").rshift(6).mask(0x03).exor(0x01);

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODULE2(config, "display");
	config.set_default_layout(layout_mephisto_polgar);
}

void mephisto_milano_state::milano(machine_config &config)
{
	polgar(config); // CPU: W65C02P-8, 4.9152MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_milano_state::milano_mem);

	MEPHISTO_BUTTONS_BOARD(config.replace(), m_board);
	m_board->set_disable_leds(true);
	config.set_default_layout(layout_mephisto_milano);
}


ROM_START( polgar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polgar_1.5_01.02.1990", 0x0000, 0x10000, CRC(88d55c0f) SHA1(e86d088ec3ac68deaf90f6b3b97e3e31b1515913) )
ROM_END

ROM_START( polgara )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polgar_1.10_04.08.1989", 0x0000, 0x10000, CRC(a4519c55) SHA1(35463a4cbcf20ebbd5ac5bc7664a862b1557c65f) ) // TC57512AD-15
ROM_END

ROM_START( polgar10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polg.10mhz_v_10.0", 0x00000, 0x10000, CRC(7c1960d4) SHA1(4d15b51f9e6f7943815945cd56078ca512a964d4) )
ROM_END

ROM_START( polgar101 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("polg_101.bin", 0x00000, 0x10000, CRC(8fb6afa4) SHA1(d1cf868302a665ff351686b26a149ced0045fc81) )
ROM_END

ROM_START( mrisc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	// contains ChessMachine engine at 0x0-0x03fff + 0x10000-0x1c74f, concatenate those sections and make a .bin file,
	// then it will work on ChessMachine software. It identifies as R E B E L ver. HG-021 03-04-92
	ROM_LOAD("meph-risci-v1-2.bin", 0x00000, 0x20000, CRC(19c6ab83) SHA1(0baab84e5aa6999c24250938d207145144945fd5) )
ROM_END

ROM_START( mrisc2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	// contains ChessMachine engine at 0x0-0x03fff + 0x10000-0x1cb7f, concatenate those sections and make a .bin file,
	// then it will work on ChessMachine software. It identifies as R E B E L ver. 2.31 22-07-93, world champion Madrid 1992
	ROM_LOAD("risc_2.31", 0x00000, 0x20000, CRC(9ecf9cd3) SHA1(7bfc628183037a172242c9589f15aca218d8fb12) )
ROM_END

ROM_START( milano )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("milano102.bin", 0x0000, 0x10000, CRC(0e9c8fe1) SHA1(e9176f42d86fe57e382185c703c7eff7e63ca711) )
ROM_END

ROM_START( milanoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("milano101.bin", 0x0000, 0x10000, CRC(22efc0be) SHA1(921607d6dacf72c0686b8970261c43e2e244dc9f) )
ROM_END

ROM_START( nshort )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("nshort.bin", 0x00000, 0x10000, CRC(4bd51e23) SHA1(3f55cc1c55dae8818b7e9384b6b8d43dc4f0a1af) )
ROM_END


/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT   CLASS                   INIT        COMPANY             FULLNAME                  FLAGS */
CONS( 1990, polgar,    0,       0,      polgar,   polgar, mephisto_polgar_state,  empty_init, "Hegener + Glaser", "Mephisto Polgar (v1.50)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1989, polgara,   polgar,  0,      polgar,   polgar, mephisto_polgar_state,  empty_init, "Hegener + Glaser", "Mephisto Polgar (v1.10)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, polgar10,  polgar,  0,      polgar10, polgar, mephisto_polgar_state,  empty_init, "Hegener + Glaser", "Mephisto Polgar 10 MHz (v10.0)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1990, polgar101, polgar,  0,      polgar10, polgar, mephisto_polgar_state,  empty_init, "Hegener + Glaser", "Mephisto Polgar 10 MHz (v10.1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

CONS( 1992, mrisc,     0,       0,      mrisc,    polgar, mephisto_risc_state,    empty_init, "Hegener + Glaser / Tasc", "Mephisto Risc 1MB", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1994, mrisc2,    mrisc,   0,      mrisc,    polgar, mephisto_risc_state,    empty_init, "Hegener + Glaser / Tasc", "Mephisto Risc II",  MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )

// not modular boards
CONS( 1991, milano,    0,       0,      milano,   polgar, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Milano (v1.02)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1991, milanoa,   milano,  0,      milano,   polgar, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Milano (v1.01)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1993, nshort,    milano,  0,      milano,   polgar, mephisto_milano_state,  empty_init, "Hegener + Glaser", "Mephisto Nigel Short", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
