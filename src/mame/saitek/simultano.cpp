// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Achim, bataais, Berger
/*******************************************************************************

Saitek Simultano, it is related to Saitek Stratos, see stratos.cpp
But it's not similar enough to be a subdriver of it.

Two versions are known: "B" from 1988/1989, and "C" from 1989.

Hardware notes:
- WDC W65C02P @ 5MHz
- 64KB ROM (2*AMI 27256), socket for 32KB Endgame ROM (ver. 2)
- 8KB RAM (SRM2264LC) battery-backed
- "HELIOS" NEC gate array
- Epson SED1502F, LCD screen
- piezo, 16+3 leds, button sensors chessboard

It also appeared in Tandy's Chess Champion 2150, still manufactured and
programmed by Saitek. Not as a simple rebrand, but with hardware differences:
3MHz R65C02, 1 64KB ROM and no EGR socket.

TODO:
- IRQ is from HELIOS pin 2, how is timing determined? same problem as with stratos
- verify that egr(1) does not work on real chesscomputer
- is cc2150 the same rom contents as the first simultano?

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"
#include "video/sed1500.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "saitek_simultano.lh"


namespace {

class simultano_state : public driver_device
{
public:
	simultano_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_lcd(*this, "lcd"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(go_button);

	void simultano(machine_config &config);
	void cc2150(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	memory_view m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<sed1502_device> m_lcd;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<8+1> m_inputs;
	output_finder<16, 34> m_out_lcd;

	bool m_power = false;
	u8 m_select = 0;
	u8 m_control = 0;

	void simultano_map(address_map &map) ATTR_COLD;
	void cc2150_map(address_map &map) ATTR_COLD;

	void power_off();
	void lcd_pwm_w(offs_t offset, u8 data);
	void lcd_output_w(offs_t offset, u64 data);

	void select_w(u8 data);
	u8 chessboard_r();
	void sound_w(u8 data);
	u8 control_r();
	void control_w(u8 data);
};

void simultano_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_select));
	save_item(NAME(m_control));
}

void simultano_state::machine_reset()
{
	m_power = true;
	m_control = 0;
	m_rombank.select(0);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// soft power on/off

INPUT_CHANGED_MEMBER(simultano_state::go_button)
{
	if (newval && !m_power)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		machine_reset();
	}
}

void simultano_state::power_off()
{
	m_power = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	// clear display
	for (int i = 0; i < 0x80; i++)
		m_lcd->write(i, 0);

	m_led_pwm->clear();
	m_lcd_pwm->clear();
}


// LCD

void simultano_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void simultano_state::lcd_output_w(offs_t offset, u64 data)
{
	if (m_power)
		m_lcd_pwm->write_row(offset, data);
}


// HELIOS

void simultano_state::sound_w(u8 data)
{
	m_dac->write(1);
}

void simultano_state::select_w(u8 data)
{
	m_dac->write(0); // guessed

	// d0-d3: input/chessboard mux
	// d6,d7: side panel led mux
	// d4,d5: led data
	m_led_pwm->matrix_partial(0, 2, data >> 4 & 3, 1 << (data & 0xf));
	m_led_pwm->matrix_partial(2, 2, data >> 6 & 3, ~data >> 4 & 3);
	m_select = data;
}

u8 simultano_state::chessboard_r()
{
	// d0-d7: chessboard sensors
	return ~m_board->read_file(m_select & 0xf);
}

u8 simultano_state::control_r()
{
	u8 data = 0;
	u8 sel = m_select & 0xf;

	// read button panel
	if (sel < 9)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

void simultano_state::control_w(u8 data)
{
	u8 prev = m_control;
	m_control = data;

	// d0,d1: rombank
	m_rombank.select(bitswap<2>(data,0,1));

	// d6 falling edge: power-off request
	if (~data & prev & 0x40)
		power_off();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void simultano_state::cc2150_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2000).w(FUNC(simultano_state::select_w));
	map(0x2200, 0x2200).w(FUNC(simultano_state::sound_w));
	map(0x2400, 0x2400).r(FUNC(simultano_state::chessboard_r));
	map(0x2600, 0x2600).rw(FUNC(simultano_state::control_r), FUNC(simultano_state::control_w));
	//map(0x4000, 0x5fff).noprw(); // tries to access RAM, unpopulated on PCB
	map(0x6000, 0x607f).rw("lcd", FUNC(sed1502_device::read), FUNC(sed1502_device::write));

	map(0x8000, 0xffff).view(m_rombank);
	m_rombank[0](0x8000, 0xffff).rom().region("maincpu", 0x0000);
	m_rombank[1](0x8000, 0xffff).rom().region("maincpu", 0x8000);
	m_rombank[2](0x8000, 0xffff).lr8(NAME([]() { return 0xff; }));
	m_rombank[3](0x8000, 0xffff).lr8(NAME([]() { return 0xff; }));
}

void simultano_state::simultano_map(address_map &map)
{
	cc2150_map(map);
	m_rombank[2](0x8000, 0xffff).r("extrom", FUNC(generic_slot_device::read_rom));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( simultano )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Set Up")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_L) PORT_NAME("Level")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Sound")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Stop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("New Game")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Rook")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bishop")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("King")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Tab / Color")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("-")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_EQUALS) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("+")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Function")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) // freq sel

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Library")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Info")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Analysis")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Normal")

	PORT_START("IN.8")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x01, DEF_STR( Normal ) )

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, simultano_state, go_button, 0) PORT_NAME("Go")
INPUT_PORTS_END

static INPUT_PORTS_START( cc2150 )
	PORT_INCLUDE( simultano )

	PORT_MODIFY("IN.5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void simultano_state::cc2150(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 3_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &simultano_state::cc2150_map);
	m_maincpu->set_periodic_int(FUNC(simultano_state::irq0_line_hold), attotime::from_hz(91.6)); // measured

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));
	m_board->set_nvram_enable(true);

	// video hardware
	SED1502(config, m_lcd, 32768).write_segs().set(FUNC(simultano_state::lcd_output_w));
	PWM_DISPLAY(config, m_lcd_pwm).set_size(16, 34);
	m_lcd_pwm->set_refresh(attotime::from_hz(30));
	m_lcd_pwm->output_x().set(FUNC(simultano_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(873/2, 1080/2);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2+2, 8);
	config.set_default_layout(layout_saitek_simultano);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void simultano_state::simultano(machine_config &config)
{
	cc2150(config);

	// basic machine hardware
	M65C02(config.replace(), m_maincpu, 5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &simultano_state::simultano_map);
	m_maincpu->set_periodic_int(FUNC(simultano_state::irq0_line_hold), attotime::from_hz(76)); // approximation

	// extension rom
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_egr");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr").set_filter("egr2");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( simultano )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1h_c12_u3.u3",  0x0000, 0x8000, CRC(e7f8bae4) SHA1(82d8c6879e031b9909dd63bff692055f32236f9c) )
	ROM_LOAD("byo1h_c13_u4.u4", 0x8000, 0x8000, CRC(4f5557bc) SHA1(2fd4b1791cec4e6e33b1da644edb603ed8c9cd2e) )

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

ROM_START( simultanoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1h_c12e_u3.u3",  0x0000, 0x8000, CRC(d583fdb4) SHA1(4be242691215ab1635a5d672441d339596f719c6) ) // AMI 27256
	ROM_LOAD("byo1h_c13b_u4.u4", 0x8000, 0x8000, CRC(c607b421) SHA1(b0c784b570dfd1fcbe3da68bcfbae2dae2957a74) ) // "

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

ROM_START( cc2150 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("yo1g_418_u3.u3",  0x8000, 0x8000, CRC(612dac24) SHA1(ba318f2ba34f9eb3df76a30c455bded76617bb11) ) // AMI 27512
	ROM_CONTINUE(               0x0000, 0x8000 )

	ROM_REGION( 795951, "screen", 0 )
	ROM_LOAD("simultano.svg", 0, 795951, CRC(ac9942bb) SHA1(f9252e5bf7b8af698a403c3f8f5ea9e475e0bf0b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME        PARENT     COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, simultano,  0,         0,      simultano, simultano, simultano_state, empty_init, "Saitek / Heuristic Software", "Kasparov Simultano (ver. C)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, simultanoa, simultano, 0,      simultano, simultano, simultano_state, empty_init, "Saitek / Heuristic Software", "Kasparov Simultano (ver. B)", MACHINE_SUPPORTS_SAVE )

SYST( 1988, cc2150,     simultano, 0,      cc2150,    cc2150,    simultano_state, empty_init, "Tandy Corporation / Saitek / Heuristic Software", "Chess Champion 2150", MACHINE_SUPPORTS_SAVE )
