// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:yoyo_chessboard
/*******************************************************************************

Mephisto Modena

The chess engine is by Frans Morsch, same one as CXG Sphinx Dominator 2.05.
Hold Pawn + Knight buttons at boot for test mode.

Hardware notes:
- PCB label: MODENA-A-2
- W65C02SP or RP65C02G @ 4.19MHz
- 8KB RAM (battery-backed), 32KB ROM
- 8*8 chessboard buttons, 16+6 leds, 7seg lcd, piezo

*******************************************************************************/

#include "emu.h"

#include "mmdisplay1.h"

#include "cpu/m6502/m65c02.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "mephisto_modena.lh"


namespace {

class modena_state : public driver_device
{
public:
	modena_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_led_pwm(*this, "led_pwm"),
		m_dac(*this, "dac"),
		m_keys(*this, "KEY")
	{ }

	void modena(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<mephisto_display1_device> m_display;
	required_device<pwm_display_device> m_led_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_keys;

	u8 m_board_mux = 0;
	u8 m_io_ctrl = 0;

	void modena_mem(address_map &map) ATTR_COLD;

	u8 input_r();
	void io_w(u8 data);
	void led_w(u8 data);
	void update_display();
};

void modena_state::machine_start()
{
	save_item(NAME(m_board_mux));
	save_item(NAME(m_io_ctrl));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void modena_state::update_display()
{
	m_led_pwm->matrix(m_io_ctrl >> 1 & 7, m_board_mux);
}

u8 modena_state::input_r()
{
	u8 data = 0;

	// read buttons
	if (~m_io_ctrl & 1)
		data |= m_keys->read();

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_board_mux, i))
			data |= m_board->read_rank(i);

	return data;
}

void modena_state::led_w(u8 data)
{
	// d0-d7: chessboard mux, led data
	m_board_mux = ~data;
	update_display();
}

void modena_state::io_w(u8 data)
{
	// d0: button select
	// d1-d3: led select
	m_io_ctrl = data;
	update_display();

	// d4: lcd strobe
	m_display->strobe_w(BIT(data, 4));

	// d6: speaker out
	m_dac->write(BIT(data, 6));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void modena_state::modena_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4000).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0x5000, 0x5000).w(FUNC(modena_state::led_w));
	map(0x6000, 0x6000).w(FUNC(modena_state::io_w));
	map(0x7000, 0x7000).r(FUNC(modena_state::input_r));
	map(0x7f00, 0x7fff).nopr(); // dummy read on 6502 absolute X page wrap
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( modena )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Book / Pawn")       PORT_CODE(KEYCODE_B)
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

void modena_state::modena(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 4.194304_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &modena_state::modena_mem);

	clock_device &nmi_clock(CLOCK(config, "nmi_clock", 4.194304_MHz_XTAL / 0x2000)); // active for 975us
	nmi_clock.signal_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display); // internal
	PWM_DISPLAY(config, m_led_pwm).set_size(3, 8);
	config.set_default_layout(layout_mephisto_modena);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( modena )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("modena_12aug1992_441d.u3", 0x0000, 0x8000, CRC(dd7b4920) SHA1(4606b9d1f8a30180aabedfc0ed3cca0c96618524) )
ROM_END

ROM_START( modenaa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("27c256_457f.u3", 0x0000, 0x8000, CRC(2889082c) SHA1(b63f0d856793b4f87471837e2219ce2a42fe18de) )
ROM_END

ROM_START( modenab )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("modena_4929_270192.u3", 0x0000, 0x8000, CRC(99212677) SHA1(f0565e5441fb38df201176d01793c953886b0303) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1992, modena,   0,      0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1992, modenaa,  modena, 0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1992, modenab,  modena, 0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 3)", MACHINE_SUPPORTS_SAVE )
