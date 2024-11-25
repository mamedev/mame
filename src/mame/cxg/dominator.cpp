// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, Mr. Lars
/*******************************************************************************

CXG Sphinx Dominator, chess computer.

The chess engine is by Frans Morsch, older versions (before 2.05) were buggy.
Hold Pawn + Knight buttons at boot for test mode, it will tell the version number.
This engine was also used in the newer Mephisto Modena.

Hardware notes:
- R65C02P4 @ 4MHz
- 32KB ROM, 8KB RAM battery-backed
- Sanyo LC7582, 2 LCD panels (each 4-digit, some unused segments)
- TTL, piezo, 8*8+8 LEDs, button sensors

Sphinx Commander also uses the Dominator program, and is on similar hardware,
but with a Chess 3008 housing (wooden chessboard, magnet sensors).
Sphinx Galaxy is on similar hardware too, with less leds.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/lc7580.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cxg_dominator.lh"
#include "cxg_galaxy.lh"
#include "cxg_commander.lh"


namespace {

class dominator_state : public driver_device
{
public:
	dominator_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd(*this, "lcd"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_digit(*this, "digit%u", 0U),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	// machine configs
	void dominator(machine_config &config);
	void galaxy(machine_config &config);
	void commander(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<lc7582_device> m_lcd;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<8> m_out_digit;
	output_finder<2, 52> m_out_lcd;

	// address maps
	void dominator_map(address_map &map) ATTR_COLD;
	void galaxy_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void lcd_output_w(offs_t offset, u64 data);
	void control_w(u8 data);
	void leds_w(offs_t offset, u8 data);
	u8 input_r(offs_t offset);
};

void dominator_state::machine_start()
{
	m_out_digit.resolve();
	m_out_lcd.resolve();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LC7582 LCD

void dominator_state::lcd_output_w(offs_t offset, u64 data)
{
	// output individual segments
	for (int i = 0; i < 52; i++)
		m_out_lcd[offset][i] = BIT(data, i);

	// unscramble digit 7segs
	static const u8 seg2digit[4*7] =
	{
		0x03, 0x04, 0x00, 0x40, 0x41, 0x02, 0x42,
		0x05, 0x06, 0x07, 0x48, 0x44, 0x45, 0x46,
		0x0c, 0x0d, 0x0b, 0x0a, 0x4a, 0x4c, 0x4b,
		0x0e, 0x0f, 0x10, 0x50, 0x4d, 0x4e, 0x4f
	};

	for (int i = 0; i < 8; i++)
	{
		u8 digit = 0;
		for (int seg = 0; seg < 7; seg++)
		{
			u8 bit = seg2digit[7 * (i & 3) + seg] + 26 * (i >> 2);
			digit |= m_out_lcd[BIT(bit, 6)][bit & 0x3f] << seg;
		}
		m_out_digit[i] = digit;
	}
}


// TTL

void dominator_state::control_w(u8 data)
{
	// d0: LC7582 DATA
	// d1: LC7582 CLK
	// d2: LC7582 CE
	m_lcd->data_w(BIT(data, 0));
	m_lcd->clk_w(BIT(data, 1));
	m_lcd->ce_w(BIT(data, 2));

	// d3: speaker out
	m_dac->write(BIT(data, 3));
}

void dominator_state::leds_w(offs_t offset, u8 data)
{
	// led data (sgalaxy only uses d4-d6)
	m_display->matrix(1 << offset, data);
}

u8 dominator_state::input_r(offs_t offset)
{
	u8 data = 0;

	// read chessboard sensors
	if (offset < 8)
		data = m_board->read_rank(offset);

	// read other buttons
	else if (offset < 10)
		data = m_inputs[offset - 8]->read();

	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void dominator_state::dominator_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x400f).rw(FUNC(dominator_state::input_r), FUNC(dominator_state::leds_w));
	map(0x4010, 0x4010).w(FUNC(dominator_state::control_w));
	map(0x7f00, 0x7fff).nopr(); // dummy read on 6502 absolute X page wrap
	map(0x8000, 0xffff).rom();
}

void dominator_state::galaxy_map(address_map &map)
{
	dominator_map(map);

	map(0x4008, 0x4008).mirror(0x0007).w(FUNC(dominator_state::control_w));
	map(0x4010, 0x4010).unmapw();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( dominator )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi Move")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Library/Clear")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Colour")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END

static INPUT_PORTS_START( commander )
	PORT_INCLUDE( dominator )

	PORT_MODIFY("IN.1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Library")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Color")
INPUT_PORTS_END

static INPUT_PORTS_START( galaxy )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Bishop")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Take Back")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Knight")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Enter Position")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_C) PORT_NAME("Sound/Color")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Library/Clearboard")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Multi Move")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Hint")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Replay")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void dominator_state::dominator(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &dominator_state::dominator_map);

	const attotime nmi_period = attotime::from_hz(4_MHz_XTAL / 0x2000); // from 4020
	m_maincpu->set_periodic_int(FUNC(dominator_state::nmi_line_pulse), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	LC7582(config, m_lcd, 0);
	m_lcd->write_segs().set(FUNC(dominator_state::lcd_output_w));

	PWM_DISPLAY(config, m_display).set_size(8+1, 8);
	config.set_default_layout(layout_cxg_dominator);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void dominator_state::galaxy(machine_config &config)
{
	dominator(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &dominator_state::galaxy_map);

	m_display->set_size(8, 8);
	config.set_default_layout(layout_cxg_galaxy);
}

void dominator_state::commander(machine_config &config)
{
	dominator(config);

	// basic machine hardware
	m_board->set_type(sensorboard_device::MAGNETS);
	m_board->set_delay(attotime::from_msec(250));

	config.set_default_layout(layout_cxg_commander);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sdtor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("22p_2.05", 0x8000, 0x8000, CRC(9707119c) SHA1(d7cde835a37bd5d9ff349a871c890ea4cd9b2c26) )
ROM_END


ROM_START( sgalaxy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cxg_230_20_oct_88_ver_2.03", 0x8000, 0x8000, CRC(b93996e3) SHA1(d616620393154373b613060901e233eea4fd4c55) )
ROM_END

ROM_START( sgalaxya )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("sgalaxya.bin", 0x8000, 0x8000, CRC(e373b2ef) SHA1(b72720ea404ae2bd59f1bdba147bb48f424d8c18) ) // label unknown
ROM_END

ROM_START( sgalaxyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("230_1.03", 0x8000, 0x8000, CRC(30d81c4e) SHA1(ab3a9b58db26d87d3e66600b77c9435313b632f2) )
ROM_END


ROM_START( scmder )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("scmder.bin", 0x8000, 0x8000, CRC(7a3f27b4) SHA1(6b3bcca707f034124b8b4bd061b500ce1a75faf4) ) // label unknown
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, sdtor,    0,       0,      dominator, dominator, dominator_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Dominator (v2.05)", MACHINE_SUPPORTS_SAVE )

SYST( 1988, sgalaxy,  0,       0,      galaxy,    galaxy,    dominator_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Galaxy (v2.03)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, sgalaxya, sgalaxy, 0,      galaxy,    galaxy,    dominator_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Galaxy (v2.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1988, sgalaxyb, sgalaxy, 0,      galaxy,    galaxy,    dominator_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Galaxy (v1.03)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, scmder,   0,       0,      commander, commander, dominator_state, empty_init, "CXG Systems / Newcrest Technology", "Sphinx Commander (v2.00)", MACHINE_SUPPORTS_SAVE )
