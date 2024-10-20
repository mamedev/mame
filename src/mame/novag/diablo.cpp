// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard, Berger
/*******************************************************************************

Novag Diablo 68000 (model 908)
Novag Scorpio 68000 (model 909)

Hardware notes (Diablo 68000):
- M68000 @ 16MHz, IPL1 256Hz, IPL2 from ACIA IRQ(always high)
- 2*8KB RAM TC5565 battery-backed, 2*32KB hashtable RAM TC55257
- 3*32KB ROM (27C256 or equivalent)
- HD44780A00 LCD controller (16x1)
- R65C51P2 ACIA @ 1.8432MHz, RS232 for Novag Super System
- magnetic sensors, 8*8 chessboard leds

Scorpio 68000 hardware is very similar, but with chessboard buttons and side leds.

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/hd44780.h"
#include "video/pwm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_diablo68.lh"
#include "novag_scorpio68.lh"


namespace {

class diablo_state : public driver_device
{
public:
	diablo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_display(*this, "display"),
		m_lcd(*this, "hd44780"),
		m_board(*this, "board"),
		m_acia(*this, "acia"),
		m_rs232(*this, "rs232"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// machine configs
	void diablo68(machine_config &config);
	void scorpio68(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<m68000_base_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<pwm_display_device> m_display;
	required_device<hd44780_device> m_lcd;
	required_device<sensorboard_device> m_board;
	required_device<mos6551_device> m_acia;
	required_device<rs232_port_device> m_rs232;
	required_device<beep_device> m_beeper;
	required_ioport_array<8> m_inputs;

	u8 m_inp_mux = 0;
	u8 m_lcd_control = 0;
	u8 m_lcd_data = 0;

	// address maps
	void diablo68_map(address_map &map) ATTR_COLD;
	void scorpio68_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void control_w(u8 data);
	void control2_w(u8 data);
	void lcd_data_w(u8 data);
	void leds_w(u8 data);
	u8 input1_r();
	u8 input2_r();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void lcd_palette(palette_device &palette) const;
};

void diablo_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_control));
	save_item(NAME(m_lcd_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void diablo_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xff, 0xff, 0xff)); // background
	palette.set_pen_color(1, rgb_t(0x00, 0x00, 0x00)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(0xe8, 0xe8, 0xe8)); // lcd pixel off
}

HD44780_PIXEL_UPDATE(diablo_state::lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (line < 2 && pos < 8)
	{
		// internal: (8+8)*1, external: 1*16
		bitmap.pix(1 + y, 1 + line*8*6 + pos*6 + x) = state ? 1 : 2;
	}
}


// misc

void diablo_state::control_w(u8 data)
{
	// d0: HD44780 E
	// d1: HD44780 RS
	if (m_lcd_control & ~data & 1)
		m_lcd->write(m_lcd_control >> 1 & 1, m_lcd_data);
	m_lcd_control = data & 3;

	// d7: enable beeper
	m_beeper->set_state(BIT(data, 7));

	// d4-d6: input mux, led select
	m_inp_mux = data >> 4 & 7;
	m_display->write_my(1 << m_inp_mux);
}

void diablo_state::control2_w(u8 data)
{
	control_w(data);

	// d2,d3: side leds (scorpio68)
	m_display->write_mx(~data >> 2 & 3);
}

void diablo_state::lcd_data_w(u8 data)
{
	// d0-d7: HD44780 data
	m_lcd_data = data;
}

void diablo_state::leds_w(u8 data)
{
	// d0-d7: chessboard leds (diablo68)
	m_display->write_mx(data);
}

u8 diablo_state::input1_r()
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~m_board->read_rank(m_inp_mux, true);
}

u8 diablo_state::input2_r()
{
	// d0-d2: multiplexed inputs (side panel)
	// other: ?
	return ~m_inputs[m_inp_mux]->read() & 7;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void diablo_state::diablo68_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x200000, 0x20ffff).rom().region("maincpu", 0x10000);
	map(0x280000, 0x28ffff).ram();
	map(0x300000, 0x300007).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write)).umask16(0xff00);
	map(0x380000, 0x380001).nopr();
	map(0x380000, 0x380000).w(FUNC(diablo_state::leds_w));
	map(0x3a0000, 0x3a0000).w(FUNC(diablo_state::lcd_data_w));
	map(0x3c0000, 0x3c0000).rw(FUNC(diablo_state::input2_r), FUNC(diablo_state::control_w));
	map(0x3e0000, 0x3e0000).r(FUNC(diablo_state::input1_r));
	map(0xff8000, 0xffbfff).ram().share("nvram");
}

void diablo_state::scorpio68_map(address_map &map)
{
	diablo68_map(map);
	map(0x380000, 0x380000).w(FUNC(diablo_state::control2_w));
	map(0x3c0000, 0x3c0001).nopw();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( diablo68 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Take Back / Analyze Games")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Right")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Level")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Flip Display / Time Control")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Left")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint / Next Best")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Priority / Tournament Book / Pawn")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Yes/Start / Start of Game")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Trace Forward / AutoPlay")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-Op / Restore Game / Rook")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("No/End / End of Game")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Clear Board / Delete Pro-Op")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Best Move/Random / Review / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Print Book / Store Game")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Sound / Info / Bishop")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Print Moves / Print Evaluations")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Verify/Set Up / Pro-Op Book/Both Books")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Solve Mate / Infinite / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Print List / Acc. Time")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Player/Player / Gambit Book / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Print Board / Interface")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void diablo_state::diablo68(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &diablo_state::diablo68_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 32.768_kHz_XTAL/128)); // 256Hz
	irq_clock.set_pulse_width(attotime::from_nsec(1380)); // active for 1.38us
	irq_clock.signal_handler().set_inputline(m_maincpu, M68K_IRQ_IPL1);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));
	m_board->set_nvram_enable(true);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60); // arbitrary
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(6*16+1, 10);
	m_screen->set_visarea_full();
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(diablo_state::lcd_palette), 3);

	HD44780(config, m_lcd, 270'000); // OSC = 91K resistor
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(diablo_state::lcd_pixel_update));

	PWM_DISPLAY(config, m_display).set_size(8, 8);
	config.set_default_layout(layout_novag_diablo68);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);

	// uart (configure after video)
	MOS6551(config, m_acia).set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set_inputline("maincpu", M68K_IRQ_IPL2);
	m_acia->rts_handler().set("acia", FUNC(mos6551_device::write_cts));
	m_acia->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	m_rs232->dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
}

void diablo_state::scorpio68(machine_config &config)
{
	diablo68(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &diablo_state::scorpio68_map);

	m_board->set_type(sensorboard_device::BUTTONS);
	m_board->set_delay(attotime::from_msec(150));

	m_display->set_width(2);
	config.set_default_layout(layout_novag_scorpio68);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( diablo68 ) // ID = D 1.08, serial 90999x
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("d_904.u3", 0x00000, 0x8000, CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9) )
	ROM_LOAD16_BYTE("d_a09.u2", 0x00001, 0x8000, CRC(bcd27647) SHA1(2de5bccd4e7b9a32957811c0028dbca1a407b9b0) )
	ROM_LOAD16_BYTE("502.u4",   0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END

ROM_START( diablo68a ) // ID = D 1.08, serial 90984x
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("d_904.u3", 0x00000, 0x8000, CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9) )
	ROM_LOAD16_BYTE("d_924.u2", 0x00001, 0x8000, CRC(e182dbdd) SHA1(24dacbef2173fa737636e4729ff22ec1e6623ca5) ) // only 4 bytes different from diablo68
	ROM_LOAD16_BYTE("502.u4",   0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END

ROM_START( diablo68b ) // ID = D 1.08, serial 90973x
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("d_evn_904.u3", 0x00000, 0x8000, CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9) )
	ROM_LOAD16_BYTE("d_odd_904.u2", 0x00001, 0x8000, CRC(d46fcc7a) SHA1(8ed69cd0fec07bf5451eaa882c87cf7cf70c87eb) ) // only 2 bytes different from diablo68a
	ROM_LOAD16_BYTE("ds_bk.u4",     0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END


ROM_START( scorpio68 ) // ID = S 1.08
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("s_evn_904.u3", 0x00000, 0x8000, CRC(a8f63245) SHA1(0ffdc6eb8ecad730440b0bfb2620fb00820e1aea) )
	ROM_LOAD16_BYTE("s_odd_c18.u2", 0x00001, 0x8000, CRC(4f033319) SHA1(fce228b1705b7156d4d01ef92b22a875d0f6f321) )
	ROM_LOAD16_BYTE("502.u4",       0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT      CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1991, diablo68,  0,        0,      diablo68,  diablo68,  diablo_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Diablo 68000 (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1991, diablo68a, diablo68, 0,      diablo68,  diablo68,  diablo_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Diablo 68000 (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1991, diablo68b, diablo68, 0,      diablo68,  diablo68,  diablo_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Diablo 68000 (set 3)", MACHINE_SUPPORTS_SAVE )

SYST( 1991, scorpio68, 0,        0,      scorpio68, diablo68,  diablo_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Scorpio 68000", MACHINE_SUPPORTS_SAVE )
