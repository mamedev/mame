// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

    Novag generic 68000 based chess computer driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - verify irq/beeper timing
    - RS232 port

******************************************************************************

Diablo 68000:
- M68000 @ 16MHz, IRQ ~256Hz
- 2*8KB RAM TC5565 battery-backed, 2*32KB hashtable RAM TC55257 3*32KB ROM
- HD44780 LCD controller (16x1)
- R65C51P2 ACIA @ 1.8432MHz, RS232
- magnetic sensors, 8*8 chessboard leds

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "cpu/m68000/m68000.h"
#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_diablo68k.lh" // clickable


class novag68k_state : public novagbase_state
{
public:
	novag68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: novagbase_state(mconfig, type, tag)
	{ }

	// Diablo 68000
	DECLARE_WRITE8_MEMBER(diablo68k_control_w);
	DECLARE_WRITE8_MEMBER(diablo68k_lcd_data_w);
	DECLARE_WRITE8_MEMBER(diablo68k_leds_w);
	DECLARE_READ8_MEMBER(diablo68k_input1_r);
	DECLARE_READ8_MEMBER(diablo68k_input2_r);
	void diablo68k_map(address_map &map);
	void diablo68k(machine_config &config);
};



// Devices, I/O

/******************************************************************************
    Diablo 68000
******************************************************************************/

// TTL

WRITE8_MEMBER(novag68k_state::diablo68k_control_w)
{
	// d1: HD44780 RS
	// other: ?
	m_lcd_control = data & 7;

	// d7: enable beeper
	m_beeper->set_state(data >> 7 & 1);

	// d4-d6: input mux, led select
	m_inp_mux = 1 << (data >> 4 & 0x7) & 0xff;
	display_matrix(8, 8, m_led_data, m_inp_mux);
	m_led_data = 0; // ?
}

WRITE8_MEMBER(novag68k_state::diablo68k_lcd_data_w)
{
	// d0-d7: HD44780 data
	m_lcd->write(space, m_lcd_control >> 1 & 1, data);
}

WRITE8_MEMBER(novag68k_state::diablo68k_leds_w)
{
	// d0-d7: chessboard leds
	m_led_data = data;
}

READ8_MEMBER(novag68k_state::diablo68k_input1_r)
{
	// d0-d7: multiplexed inputs (chessboard squares)
	return ~read_inputs(8) & 0xff;
}

READ8_MEMBER(novag68k_state::diablo68k_input2_r)
{
	// d0-d2: multiplexed inputs (side panel)
	// other: ?
	return ~read_inputs(8) >> 8 & 7;
}



/******************************************************************************
    Address Maps
******************************************************************************/

// Diablo 68000

ADDRESS_MAP_START(novag68k_state::diablo68k_map)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x280000, 0x28ffff) AM_RAM
	AM_RANGE(0x300000, 0x300007) AM_DEVREADWRITE8("acia", mos6551_device, read, write, 0xff00)
	AM_RANGE(0x380000, 0x380001) AM_WRITE8(diablo68k_leds_w, 0xff00) AM_READNOP
	AM_RANGE(0x3a0000, 0x3a0001) AM_WRITE8(diablo68k_lcd_data_w, 0xff00)
	AM_RANGE(0x3c0000, 0x3c0001) AM_READWRITE8(diablo68k_input2_r, diablo68k_control_w, 0xff00)
	AM_RANGE(0x3e0000, 0x3e0001) AM_READ8(diablo68k_input1_r, 0xff00)
	AM_RANGE(0xff8000, 0xffbfff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( diablo68k )
	PORT_INCLUDE( novag_cb_magnets )

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



/******************************************************************************
    Machine Drivers
******************************************************************************/

MACHINE_CONFIG_START(novag68k_state::diablo68k)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16_MHz_XTAL)
	MCFG_CPU_PERIODIC_INT_DRIVER(novag68k_state, irq2_line_hold, 256) // guessed
	MCFG_CPU_PROGRAM_MAP(diablo68k_map)

	MCFG_DEVICE_ADD("acia", MOS6551, 0)
	MCFG_MOS6551_XTAL(1.8432_MHz_XTAL)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60) // arbitrary
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(6*16+1, 10)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*16, 0, 10-1)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(novagbase_state, novag_lcd)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 8)
	MCFG_HD44780_PIXEL_UPDATE_CB(novagbase_state, novag_lcd_pixel_update)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", novagbase_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_novag_diablo68k)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 1024) // guessed
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( diablo68 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("evenurom.bin", 0x00000, 0x8000, CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9) )
	ROM_LOAD16_BYTE("oddlrom.bin",  0x00001, 0x8000, CRC(e182dbdd) SHA1(24dacbef2173fa737636e4729ff22ec1e6623ca5) )
	ROM_LOAD16_BYTE("book.bin", 0x10000, 0x8000, CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108) ) // no odd rom
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE    INPUT      STATE        INIT  COMPANY, FULLNAME, FLAGS
CONS( 1991, diablo68, 0,      0, diablo68k, diablo68k, novag68k_state, 0, "Novag", "Diablo 68000", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
