// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-99 keyboard emulation

*********************************************************************/

/*

Keyboard PCB Layout
-------------------

|-----------------------------------------------------------------------|
|   CN1         CN2             CPU1        ROM1                    SW1 |
|                                                                       |
|   6MHz    CPU0        ROM0        GI                                  |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|                                                                       |
|-----------------------------------------------------------------------|

Notes:
    Relevant IC's shown.

    CPU0        - STMicro ET8035N-6 8035 CPU
    CPU1        - STMicro ET8035N-6 8035 CPU
    ROM0        - Texas Instruments TMS2516JL-16 2Kx8 ROM "107268-16"
    ROM1        - STMicro ET2716Q-1 2Kx8 ROM "107268-17"
    GI          - General Instruments 321239007 keyboard chip "4=7"
    CN1         - DB15 connector, Luxor ABC R8 (3 button mouse)
    CN2         - 12 pin PCB header, keyboard data cable
    SW1         - reset switch?

*/

/*

    TODO:

    - watchdog clock
    - output leds

*/

#include "emu.h"
#include "abc99.h"

#include "speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8035_Z2_TAG "z2"
#define I8035_Z5_TAG "z5"
#define R8_TAG       "r8"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC99, abc99_device, "abc99", "Luxor ABC 99")


//-------------------------------------------------
//  ROM( abc99 )
//-------------------------------------------------

ROM_START( abc99 )
	ROM_SYSTEM_BIOS( 0, "107268", "107268-17" )
	ROM_SYSTEM_BIOS( 1, "106819", "106819-09" )
	ROM_DEFAULT_BIOS("106819")

	ROM_REGION( 0x1000, I8035_Z2_TAG, 0 )
	ROMX_LOAD( "107268-17.z3", 0x0000, 0x0800, CRC(2f60cc35) SHA1(ebc6af9cd0a49a0d01698589370e628eebb6221c), ROM_BIOS(0) ) // UP/DOWN mode is broken
	ROMX_LOAD( "106819-09.z3", 0x0000, 0x1000, CRC(ffe32a71) SHA1(fa2ce8e0216a433f9bbad0bdd6e3dc0b540f03b7), ROM_BIOS(1) ) // ABC 99 6490423-01

	ROM_REGION( 0x800, I8035_Z5_TAG, 0 )
	ROMX_LOAD( "107268-16.z6", 0x0000, 0x0800, CRC(785ec0c6) SHA1(0b261beae20dbc06fdfccc50b19ea48b5b6e22eb), ROM_BIOS(0) )
	ROMX_LOAD( "107268-64.z6", 0x0000, 0x0800, CRC(e33683ae) SHA1(0c1d9e320f82df05f4804992ef6f6f6cd20623f3), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *abc99_device::device_rom_region() const
{
	return ROM_NAME( abc99 );
}


//-------------------------------------------------
//  ADDRESS_MAP( keyboard_mem )
//-------------------------------------------------

void abc99_device::keyboard_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(I8035_Z2_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( keyboard_io )
//-------------------------------------------------

void abc99_device::keyboard_io(address_map &map)
{
	map(0x21, 0x21).w(FUNC(abc99_device::led_w));
	map(0x30, 0x3f).rw(FUNC(abc99_device::key_y_r), FUNC(abc99_device::key_x_w));
}


//-------------------------------------------------
//  ADDRESS_MAP( mouse_mem )
//-------------------------------------------------

void abc99_device::mouse_mem(address_map &map)
{
	map(0x0000, 0x07ff).rom().region(I8035_Z5_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc99_device::device_add_mconfig(machine_config &config)
{
	// keyboard CPU
	I8035(config, m_maincpu, 0); // from Z5 T0 output
	m_maincpu->set_addrmap(AS_PROGRAM, &abc99_device::keyboard_mem);
	m_maincpu->set_addrmap(AS_IO, &abc99_device::keyboard_io);
	m_maincpu->p1_out_cb().set(FUNC(abc99_device::z2_p1_w));
	m_maincpu->p2_in_cb().set(FUNC(abc99_device::z2_p2_r));
	m_maincpu->t0_in_cb().set_constant(0); // mouse connected
	m_maincpu->t1_in_cb().set(FUNC(abc99_device::z2_t1_r));

	// mouse CPU
	I8035(config, m_mousecpu, XTAL(6'000'000));
	m_mousecpu->set_addrmap(AS_PROGRAM, &abc99_device::mouse_mem);
	m_mousecpu->p1_in_cb().set(FUNC(abc99_device::z5_p1_r));
	m_mousecpu->p2_out_cb().set(FUNC(abc99_device::z5_p2_w));
	m_mousecpu->set_t0_clk_cb(I8035_Z2_TAG, FUNC(device_t::set_unscaled_clock_int));
	m_mousecpu->t1_in_cb().set(FUNC(abc99_device::z5_t1_r));

	// watchdog
	WATCHDOG_TIMER(config, m_watchdog).set_time(attotime::from_hz(0));

	// mouse
	LUXOR_R8(config, m_mouse, 0);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( keyboard_reset )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( abc99_device::keyboard_reset )
{
	if (newval)
	{
		m_mousecpu->reset();
	}
}


//-------------------------------------------------
//  INPUT_PORTS( abc99 )
//-------------------------------------------------

ioport_value abc99_device::cursor_x4_r()
{
	u8 cursor = m_cursor->read();
	u8 data = 0;
	switch (cursor & 0xf)
	{
		case 0x1: // up
		case 0x9: // up-right
			data = 0x1;
			break;
		case 0x2: // down
		case 0xa: // down-right
			data = 0x2;
			break;
		case 0x8: // right
			data = 0x3;
			break;
	}
	return data;
}

ioport_value abc99_device::cursor_x6_r()
{
	u8 cursor = m_cursor->read();
	u8 data = 0;
	switch (cursor & 0xf)
	{
		case 0x1: // up
		case 0x5: // up-left
			data = 0x1;
			break;
		case 0x2: // down
		case 0x6: // down-left
			data = 0x2;
			break;
		case 0x4: // left
			data = 0x3;
			break;
	}
	return data;
}

static INPUT_PORTS_START( abc99 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF13") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2192|") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t') // U+2192 = →
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LF") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad CE") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad RETURN") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PRINT") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad .") PORT_CODE(KEYCODE_DEL_PAD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Left SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_HOME)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF15") PORT_CODE(KEYCODE_PAUSE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(abc99_device, cursor_x4_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(U'ü') PORT_CHAR(U'Ü')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Right SHIFT") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF14") PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(abc99_device, cursor_x6_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(U'é') PORT_CHAR(U'É')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(U'å') PORT_CHAR(U'Å')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(U'ä') PORT_CHAR(U'Ä')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_END)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"|\u2190") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT)) // U+2190 = ←

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('+') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(U'ö') PORT_CHAR(U'Ö')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)

	PORT_START("X11")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("X12")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR(U'¤')

	PORT_START("X14")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('/')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("X15")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')

	PORT_START("Z14")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("Z14:1,2,3")
	PORT_DIPSETTING(    0x00, "Swedish" )
	PORT_DIPSETTING(    0x01, "US English" )
	PORT_DIPSETTING(    0x02, "Spanish" )
	PORT_DIPSETTING(    0x03, "Danish" )
	PORT_DIPSETTING(    0x04, "Norwegian" )
	PORT_DIPSETTING(    0x05, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x08, 0x08, "Keyboard Program" ) PORT_DIPLOCATION("Z14:4")
	PORT_DIPSETTING(    0x00, "Internal (8048)" )
	PORT_DIPSETTING(    0x08, "External PROM" )

	PORT_START("CURSOR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2191") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP)) // U+2191 = ↑
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2193") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN)) // U+2193 = ↓
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT)) // U+2190 = ←
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // U+2192 = →

	PORT_START("J4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keyboard Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, abc99_device, keyboard_reset, 0)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor abc99_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( abc99 );
}



//-------------------------------------------------
//  serial_input -
//-------------------------------------------------

void abc99_device::serial_input()
{
	m_maincpu->set_input_line(MCS48_INPUT_IRQ, (m_si || m_si_en) ? CLEAR_LINE : ASSERT_LINE);
	m_mousecpu->set_input_line(MCS48_INPUT_IRQ, m_si ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  serial_clock -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(abc99_device::serial_clock)
{
	m_slot->trxc_w(1);
	m_slot->trxc_w(0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc99_device - constructor
//-------------------------------------------------

abc99_device::abc99_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC99, tag, owner, clock),
	abc_keyboard_interface(mconfig, *this),
	m_serial_timer(nullptr),
	m_maincpu(*this, I8035_Z2_TAG),
	m_mousecpu(*this, I8035_Z5_TAG),
	m_watchdog(*this, "watchdog"),
	m_speaker(*this, "speaker"),
	m_mouse(*this, R8_TAG),
	m_x(*this, "X%u", 0),
	m_z14(*this, "Z14"),
	m_cursor(*this, "CURSOR"),
	m_leds(*this, "led%u", 0U),
	m_keylatch(0),
	m_si(1),
	m_si_en(1),
	m_so_z2(1),
	m_so_z5(1),
	m_t1_z2(0),
	m_t1_z5(0),
	m_led_en(1),
	m_reset(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc99_device::device_start()
{
	m_leds.resolve();

	// allocate timers
	m_serial_timer = timer_alloc(FUNC(abc99_device::serial_clock), this);
	attotime serial_clock = MCS48_ALE_CLOCK(m_mousecpu->get_t0_clock()); // 8333 bps
	m_serial_timer->adjust(serial_clock, 0, serial_clock);

	// state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_si));
	save_item(NAME(m_si_en));
	save_item(NAME(m_so_z2));
	save_item(NAME(m_so_z5));
	save_item(NAME(m_t1_z2));
	save_item(NAME(m_t1_z5));
	save_item(NAME(m_led_en));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc99_device::device_reset()
{
	// external access
	m_maincpu->set_input_line(MCS48_INPUT_EA, ASSERT_LINE);
	m_mousecpu->set_input_line(MCS48_INPUT_EA, ASSERT_LINE);

	m_slot->write_rx(1);
}


//-------------------------------------------------
//  txd_w -
//-------------------------------------------------

void abc99_device::txd_w(int state)
{
	if (m_si != state)
	{
		m_si = state;
		serial_input();
	}
}


//-------------------------------------------------
//  reset_w -
//-------------------------------------------------

void abc99_device::reset_w(int state)
{
	m_mousecpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  key_x_w -
//-------------------------------------------------

void abc99_device::led_w(uint8_t data)
{
	if (m_led_en) return;

	m_leds[LED_1] = BIT(data, 0);
	m_leds[LED_2] = BIT(data, 1);
	m_leds[LED_3] = BIT(data, 2);
	m_leds[LED_4] = BIT(data, 3);
	m_leds[LED_5] = BIT(data, 4);
	m_leds[LED_6] = BIT(data, 5);
	m_leds[LED_7] = BIT(data, 6);
	m_leds[LED_8] = BIT(data, 7);
}


//-------------------------------------------------
//  key_y_r -
//-------------------------------------------------

uint8_t abc99_device::key_y_r()
{
	return m_x[m_keylatch]->read();
}


//-------------------------------------------------
//  key_x_w -
//-------------------------------------------------

void abc99_device::key_x_w(offs_t offset, uint8_t data)
{
	m_keylatch = offset & 0x0f;

	if (m_keylatch == 14)
	{
		m_watchdog->watchdog_reset();
	}
}


//-------------------------------------------------
//  z2_p1_w -
//-------------------------------------------------

void abc99_device::z2_p1_w(uint8_t data)
{
	/*

	    bit     description

	    P10     serial output
	    P11     KEY DOWN
	    P12     transmit -> Z5 T1
	    P13     INS led
	    P14     ALT led
	    P15     CAPS LOCK led
	    P16     speaker output
	    P17     Z8 enable

	*/

	// serial output
	m_so_z2 = BIT(data, 0);
	m_slot->write_rx(m_so_z2 && m_so_z5);

	// key down
	m_slot->keydown_w(!BIT(data, 1));

	// master T1
	m_t1_z5 = BIT(data, 2);

	// key LEDs
	m_leds[LED_INS] = !BIT(data, 3);
	m_leds[LED_ALT] = !BIT(data, 4);
	m_leds[LED_CAPS_LOCK] = !BIT(data, 5);

	// speaker output
	m_speaker->level_w(!BIT(data, 6));

	// Z8 enable
	m_led_en = BIT(data, 7);
}


//-------------------------------------------------
//  z2_p2_r -
//-------------------------------------------------

uint8_t abc99_device::z2_p2_r()
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24
	    P25     DIP0
	    P26     DIP1
	    P27     DIP2

	*/

	uint8_t data = m_z14->read() << 5;

	return data;
}


//-------------------------------------------------
//  z5_p1_r -
//-------------------------------------------------

uint8_t abc99_device::z5_p1_r()
{
	/*

	    bit     description

	    P10     XA
	    P11     XB
	    P12     YA
	    P13     YB
	    P14     LB
	    P15     MB
	    P16     RB
	    P17     input from host

	*/

	uint8_t data = 0;

	// mouse
	data |= m_mouse->read() & 0x7f;

	// serial input
	data |= m_si << 7;

	return data;
}


//-------------------------------------------------
//  z5_p2_w -
//-------------------------------------------------

void abc99_device::z5_p2_w(uint8_t data)
{
	/*

	    bit     description

	    P20
	    P21
	    P22
	    P23
	    P24     Z2 serial input enable
	    P25     Z2 RESET
	    P26     serial output
	    P27     Z2 T1

	*/

	// serial input enable
	int si_en = BIT(data, 4);

	if (m_si_en != si_en)
	{
		m_si_en = si_en;
		serial_input();
	}

	// Z2 reset
	int reset = BIT(data, 5);

	if (m_reset != reset)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, reset ? CLEAR_LINE : ASSERT_LINE);
	}

	m_reset = reset;

	// serial output
	m_so_z5 = BIT(data, 6);
	m_slot->write_rx(m_so_z2 && m_so_z5);

	// keyboard CPU T1
	m_t1_z2 = BIT(data, 7);
}
