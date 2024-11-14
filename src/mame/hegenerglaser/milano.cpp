// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Berger
/*******************************************************************************

Mephisto Milano

Hardware notes:
- RP65C02G or W65C02P-8 @ 4.91MHz
- 8KB RAM(battery-backed), 64KB ROM
- HD44100H, HD44780, 2*16 chars LCD screen
- 8*8 chessboard buttons, 16 leds, piezo

Nigel Short is basically a Milano 2.00

*******************************************************************************/

#include "emu.h"

#include "mmdisplay2.h"

#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "video/pwm.h"

// internal artwork
#include "mephisto_milano.lh"


namespace {

class milano_state : public driver_device
{
public:
	milano_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_led_pwm(*this, "led_pwm"),
		m_keys(*this, "KEY")
	{ }

	void milano(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<mephisto_display2_device> m_display;
	required_device<pwm_display_device> m_led_pwm;
	required_ioport m_keys;

	u8 m_board_mux = 0;
	u8 m_led_data = 0;

	void milano_mem(address_map &map) ATTR_COLD;

	void update_leds();
	void io_w(u8 data);
	void board_w(u8 data);
	u8 board_r();
	u8 keys_r(offs_t offset);
};

void milano_state::machine_start()
{
	save_item(NAME(m_board_mux));
	save_item(NAME(m_led_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void milano_state::update_leds()
{
	m_led_pwm->matrix(m_board_mux, m_led_data);
}

void milano_state::io_w(u8 data)
{
	// default display module
	m_display->io_w(data & 0x0f);

	// high bits go to board leds
	m_led_data = data >> 4;
	update_leds();
}

void milano_state::board_w(u8 data)
{
	m_board_mux = ~data;
	update_leds();
}

u8 milano_state::board_r()
{
	u8 data = 0;

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_board_mux, i))
			data |= m_board->read_rank(i);

	return data;
}

u8 milano_state::keys_r(offs_t offset)
{
	return ~(BIT(m_keys->read(), offset) << 7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void milano_state::milano_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x1fc0, 0x1fff).unmaprw();
	map(0x1fc0, 0x1fc0).w(m_display, FUNC(mephisto_display2_device::latch_w));
	map(0x1fd0, 0x1fd0).w(FUNC(milano_state::board_w));
	map(0x1fe0, 0x1fe0).r(FUNC(milano_state::board_r));
	map(0x1fe8, 0x1fef).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x1fd8, 0x1fdf).r(FUNC(milano_state::keys_r));
	map(0x1ff0, 0x1ff0).w(FUNC(milano_state::io_w));
	map(0x2000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( milano )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Training / Pawn")   PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Info / Knight")     PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Memory / Bishop")   PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Position / Rook")   PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Level / Queen")     PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Function / King")   PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Enter / New Game")  PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Clear / New Game")  PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void milano_state::milano(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &milano_state::milano_mem);

	const attotime nmi_period = attotime::from_hz(4.9152_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(milano_state::nmi_line_pulse), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	hc259_device &outlatch(HC259(config, "outlatch"));
	outlatch.q_out_cb<0>().set_output("led100");
	outlatch.q_out_cb<1>().set_output("led101");
	outlatch.q_out_cb<2>().set_output("led102");
	outlatch.q_out_cb<3>().set_output("led103");
	outlatch.q_out_cb<4>().set_output("led104");
	outlatch.q_out_cb<5>().set_output("led105");

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(8, 2);

	MEPHISTO_DISPLAY_MODULE2(config, m_display); // internal
	config.set_default_layout(layout_mephisto_milano);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

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
	ROM_LOAD("milano_3c59_nigel_short_21-sep-93", 0x0000, 0x10000, CRC(4bd51e23) SHA1(3f55cc1c55dae8818b7e9384b6b8d43dc4f0a1af) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT   CLASS          INIT       COMPANY, FULLNAME, FLAGS
SYST( 1991, milano,    0,       0,      milano,   milano, milano_state, empty_init, "Hegener + Glaser", "Mephisto Milano (v1.02)", MACHINE_SUPPORTS_SAVE )
SYST( 1991, milanoa,   milano,  0,      milano,   milano, milano_state, empty_init, "Hegener + Glaser", "Mephisto Milano (v1.01)", MACHINE_SUPPORTS_SAVE )

SYST( 1993, nshort,    0,       0,      milano,   milano, milano_state, empty_init, "Hegener + Glaser", "Mephisto Nigel Short", MACHINE_SUPPORTS_SAVE )
