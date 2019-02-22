// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************
*
* novag_diablo.cpp, subdriver of machine/novagbase.cpp, machine/chessbase.cpp

TODO:
- hook up RS232 port (when connected, I'm only getting "New Game")

*******************************************************************************

Novag Diablo 68000 overview:
- M68000 @ 16MHz, IPL1 256Hz, IPL2 from ACIA IRQ(always high)
- 2*8KB RAM TC5565 battery-backed, 2*32KB hashtable RAM TC55257 3*32KB ROM
- HD44780 LCD controller (16x1)
- R65C51P2 ACIA @ 1.8432MHz, RS232
- magnetic sensors, 8*8 chessboard leds

Scorpio 68000 hardware is very similar, but with chessboard buttons and side leds.

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_diablo68k.lh" // clickable
#include "novag_scorpio68k.lh" // clickable


namespace {

class diablo_state : public novagbase_state
{
public:
	diablo_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag),
		m_screen(*this, "screen"),
		m_acia(*this, "acia"),
		m_rs232(*this, "rs232")
	{ }

	// machine drivers
	void diablo68k(machine_config &config);
	void scorpio68k(machine_config &config);

private:
	// devices/pointers
	required_device<screen_device> m_screen;
	required_device<mos6551_device> m_acia;
	required_device<rs232_port_device> m_rs232;

	// address maps
	void diablo68k_map(address_map &map);
	void scorpio68k_map(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(lcd_data_w);
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_READ8_MEMBER(input1_r);
	DECLARE_READ8_MEMBER(input2_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL

WRITE8_MEMBER(diablo_state::control_w)
{
	// d0: HD44780 E
	// d1: HD44780 RS
	if (m_lcd_control & ~data & 1)
		m_lcd->write(m_lcd_control >> 1 & 1, m_lcd_data);
	m_lcd_control = data & 3;

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);

	// d2,d3: side leds(scorpio)
	u8 leds2 = ~data >> 2 & 3;

	// d4-d6: input mux, led select
	m_inp_mux = 1 << (data >> 4 & 0x7) & 0xff;
	display_matrix(8+2, 8, m_led_data | leds2 << 8, m_inp_mux);
	m_led_data = 0; // ?
}

WRITE8_MEMBER(diablo_state::lcd_data_w)
{
	// d0-d7: HD44780 data
	m_lcd_data = data;
}

WRITE8_MEMBER(diablo_state::leds_w)
{
	// d0-d7: chessboard leds
	m_led_data = data;
}

READ8_MEMBER(diablo_state::input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(diablo_state::input2_r)
{
	// d0-d2: multiplexed inputs (side panel)
	// other: ?
	return ~read_inputs(8) >> 8 & 7;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void diablo_state::diablo68k_map(address_map &map)
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

void diablo_state::scorpio68k_map(address_map &map)
{
	diablo68k_map(map);
	map(0x380000, 0x380000).w(FUNC(diablo_state::control_w));
	map(0x3c0000, 0x3c0001).nopw();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( diablo68k_sidepanel )
	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Take Back / Analyze Games")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("->")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Set Level")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Flip Display / Time Control")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("<-")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint / Next Best")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Priority / Tournament Book / Pawn")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Yes/Start / Start of Game")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Trace Forward / AutoPlay")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-Op / Restore Game / Rook")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("No/End / End of Game")

	PORT_MODIFY("IN.4")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Clear Board / Delete Pro-Op")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Best Move/Random / Review / Knight")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Print Book / Store Game")

	PORT_MODIFY("IN.5")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Change Color")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Sound / Info / Bishop")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Print Moves / Print Evaluations")

	PORT_MODIFY("IN.6")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Verify/Set Up / Pro-Op Book/Both Books")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Solve Mate / Infinite / Queen")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Print List / Acc. Time")

	PORT_MODIFY("IN.7")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Player/Player / Gambit Book / King")
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Print Board / Interface")
INPUT_PORTS_END

static INPUT_PORTS_START( diablo68k )
	PORT_INCLUDE( generic_cb_magnets )
	PORT_INCLUDE( diablo68k_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( scorpio68k )
	PORT_INCLUDE( generic_cb_buttons )
	PORT_INCLUDE( diablo68k_sidepanel )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void diablo_state::diablo68k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &diablo_state::diablo68k_map);

	const attotime irq_period = attotime::from_hz(32.768_kHz_XTAL/128); // 256Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(diablo_state::irq_on<M68K_IRQ_2>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(1100)); // active for 1.1us
	TIMER(config, "irq_off").configure_periodic(FUNC(diablo_state::irq_off<M68K_IRQ_2>), irq_period);

	MOS6551(config, m_acia).set_xtal(1.8432_MHz_XTAL);
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60); // arbitrary
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(6*16+1, 10);
	m_screen->set_visarea(0, 6*16, 0, 10-1);
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(diablo_state::novag_lcd_palette), 3);

	HD44780(config, m_lcd, 0);
	m_lcd->set_lcd_size(2, 8);
	m_lcd->set_pixel_update_cb(FUNC(diablo_state::novag_lcd_pixel_update), this);

	TIMER(config, "display_decay").configure_periodic(FUNC(diablo_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_diablo68k);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 32.768_kHz_XTAL/32); // 1024Hz
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void diablo_state::scorpio68k(machine_config &config)
{
	diablo68k(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &diablo_state::scorpio68k_map);
	config.set_default_layout(layout_novag_scorpio68k);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( diablo68 )
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("evenurom.bin", 0x00000, 0x8000, CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9) )
	ROM_LOAD16_BYTE("oddlrom.bin",  0x00001, 0x8000, CRC(e182dbdd) SHA1(24dacbef2173fa737636e4729ff22ec1e6623ca5) )
	ROM_LOAD16_BYTE("book.bin",     0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END


ROM_START( scorpio68 )
	ROM_REGION16_BE( 0x20000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("s_evn_904.u3", 0x00000, 0x8000, CRC(a8f63245) SHA1(0ffdc6eb8ecad730440b0bfb2620fb00820e1aea) )
	ROM_LOAD16_BYTE("s_odd_c18.u2", 0x00001, 0x8000, CRC(4f033319) SHA1(fce228b1705b7156d4d01ef92b22a875d0f6f321) )
	ROM_LOAD16_BYTE("502.u4",       0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT CMP MACHINE     INPUT       CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1991, diablo68,  0,      0, diablo68k,  diablo68k,  diablo_state, empty_init, "Novag", "Diablo 68000", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )

CONS( 1991, scorpio68, 0,      0, scorpio68k, scorpio68k, diablo_state, empty_init, "Novag", "Scorpio 68000", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
