// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/*******************************************************************************

Mephisto Academy

Hardware notes:
- PCB label HGS 10 130 01
- VL65NC02-04PC @ 4.91MHz
- 2*32KB ROM(TC57256AD-12), 1st ROM half-unused (A14 = VCC)
- 8KB battery-backed RAM(TC5564APL-15)
- HD44100H, HD44780, 2*16 chars LCD screen
- 8 tri-color leds (not fully used: always outputs 6 red, 2 green)
- magnets chessboard with leds, piezo

Since the program is on an external module, it appears it was meant to be
a modular chesscomputer. However, no extra modules were sold separately.
Module PCB is the same as Super Mondial II College, label HGS 10 116 05.

The T+T version is (presumably the first version of) Polgar ported over to
Academy hardware. Much unused data remains, and the TRAIN button doesn't work.

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay2.h"

#include "cpu/m6502/m65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "video/pwm.h"

// internal artwork
#include "mephisto_academy.lh"


namespace {

class academy_state : public driver_device
{
public:
	academy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_outlatch(*this, "outlatch"),
		m_keys(*this, "KEY")
	{ }

	void academy(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<hc259_device> m_outlatch;
	required_ioport m_keys;

	void main_map(address_map &map) ATTR_COLD;

	void led_w(u8 data);
	u8 input_r();
};



/*******************************************************************************
    I/O
*******************************************************************************/

void academy_state::led_w(u8 data)
{
	// d0-d3: keypad led select
	// d4-d7: keypad led data
	m_led_pwm->matrix(data & 0xf, ~data >> 4 & 0xf);
}

u8 academy_state::input_r()
{
	// 74259 Q1 selects keypad
	u8 data = m_outlatch->q1_r() ? 0 : m_keys->read();
	return ~m_board->input_r() | data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void academy_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2400, 0x2400).r(FUNC(academy_state::input_r));
	map(0x2800, 0x2800).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c00).w(m_board, FUNC(mephisto_board_device::led_w));
	map(0x3000, 0x3007).w("outlatch", FUNC(hc259_device::write_d7));
	map(0x3400, 0x3400).w(FUNC(academy_state::led_w));
	map(0x3800, 0x3801).rw("display:hd44780", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x4000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( academy )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("TRAIN")  PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO")   PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS")    PORT_CODE(KEYCODE_P)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("FCT")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT")    PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void academy_state::academy(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &academy_state::main_map);

	const attotime nmi_period = attotime::from_hz(4.9152_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(academy_state::nmi_line_pulse), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HC259(config, m_outlatch); // SN74HC259N
	m_outlatch->parallel_out_cb().set("display:dac", FUNC(dac_2bit_ones_complement_device::write)).rshift(2).mask(0x03);

	MEPHISTO_SENSORS_BOARD(config, "board"); // internal
	MEPHISTO_DISPLAY_MODULE2(config, "display"); // internal

	// video hardware
	PWM_DISPLAY(config, m_led_pwm).set_size(4, 4);
	config.set_default_layout(layout_mephisto_academy);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( academy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("academy_engl.16k", 0x0000, 0x8000, CRC(ebda6674) SHA1(2ca3ad697cb9de2873e4ef9d52c5701278456acb) )
	ROM_LOAD("academy_engl.32k", 0x8000, 0x8000, CRC(a967922b) SHA1(1327903ff89bf96d72c930c400f367ae19e3ec68) )
ROM_END

ROM_START( academyg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("academy_16k_6.3.89", 0x0000, 0x8000, CRC(463a2106) SHA1(cc10c1ec78e20063926ed025dab08c3276499141) )
	ROM_LOAD("academy_32k_6.3.89", 0x8000, 0x8000, CRC(e313d084) SHA1(ced5712d34fcc81bedcd741b7ac9e2ba17bf5235) )
ROM_END

ROM_START( academyga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("academy_16k_04.10.88", 0x0000, 0x8000, CRC(85c3b076) SHA1(338b165f051e9142364d344b518ff13732de404e) )
	ROM_LOAD("academy_32k_04.10.88", 0x8000, 0x8000, CRC(478155db) SHA1(d363ab6d5bc0f47a6cdfa5132b77535ef8da8256) )
ROM_END

ROM_START( academygb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("academy_16k_27.8.88", 0x0000, 0x8000, CRC(9530def8) SHA1(376a6263477dd2c36723871acefa2939643747a5) )
	ROM_LOAD("academy_32k_27.8.88", 0x8000, 0x8000, CRC(bb9e3dc8) SHA1(98cd9bf830de1eeef339a3e1a8604d40e0ac2334) )
ROM_END

ROM_START( academyd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("academy_hollandisch_16k_4.10.88", 0x0000, 0x8000, CRC(a8ebdff4) SHA1(a25015bc10c0c3b4dcd726b6ef2495ced188ee2e) )
	ROM_LOAD("academy_hollandisch_32k_4.10.88", 0x8000, 0x8000, CRC(ec92358e) SHA1(27bd542ac39ded5a6c7d3b8547c1a79c7221c5a7) )
ROM_END

ROM_START( academytt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("boek", 0x4000, 0x4000, CRC(5503fbb2) SHA1(4dd584b065207d6db408cf02eae9aca825c21f7d) ) // A14 = GND?
	ROM_CONTINUE(    0x0000, 0x4000 )
	ROM_LOAD("v-11", 0x8000, 0x8000, CRC(6bc144d5) SHA1(c0c9be144ed0d1fe9ef601c57f38ff9103f2bc64) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, academy,   0,       0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy (English, 04-10-88)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, academyg,  academy, 0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy (German, 06-03-89)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, academyga, academy, 0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy (German, 04-10-88)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, academygb, academy, 0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy (German, 27-08-88)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, academyd,  academy, 0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy (Dutch, 04-10-88)", MACHINE_SUPPORTS_SAVE )

SYST( 1989, academytt, academy, 0,      academy,  academy, academy_state, empty_init, "Hegener + Glaser", "Mephisto Academy T+T (Dutch, prototype)", MACHINE_SUPPORTS_SAVE )
