// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edström
/**********************************************************************

  Ericsson PC keyboard emulation

  TTL-level bi-directional serial matrix keyboard

  The mc6801 contains an internal ROM that handles scanning of the keyboard,
  controlling the 2 or 3 LEDs and also the programming of the scan code for
  a single programmable key.

  There are two known variants of the keyboard. The first had the Ericsson
  internal name "Sgt Pepper" where the hardware was OEMed/manufactured by
  FACIT and had two LEDs while the second variant called "Roger Moore" had
  three LEDs and was manufactured by Ericsson.

  Both keyboard hooks up directly to the port of a 6801 MCU. There are
  16 column lines driven by Port 3 and Port 4 that goes low one at a time
  during the scan process and when a key is pressed one of the six corresponding
  row lines goes low and fed to through a 74HC04 inverter into port 1, where a
  high bit means a key was pressed.

  The connector has TX/Hold, Rx and Reset. Reset is connected directly to the
  MCU so the host CPU can keep it in RESET until it needs the keyboard.
  Rx is connected to the RX line of the SCI in the MCU, P23 - bit 3 of port 2.
  Tx/Hold is bidirectional, connected to the TX line of the SCI in the MCU, P24
  bit 4 of port 2, but can also be kept low by the host CPU to temporarily inhibit
  the keyboard from sending more scan codes. This is sensed by P16 through a
  74HC04 inverter. The keyboard is specified to be able to buffer up to 20 scan codes.

  The data is exchanged in both direction asynchronously at 1200 baud, 8 databits,
  1 start and 1 stop bit. At startup the host CPU sends a $00 (zero) byte to the
  keyboard simultaneously with the MCU sending a $A5 to the CPU to ensure full
  duplex operation. If the $A5 byte is not received EPC will display a "Keyboard
  Error" message on the screen.

  P17 and P20 are connected to LEDs on Caps Lock and Num Lock keys. The latter
  keyboard variant Roger Moore also had a LED on Scroll Lock connected to P22.
  P20, P21 and P22 are pulled high to bring the MCU into MODE 7 at RESET. NMI
  and IRQ are pulled high and not connected to anything externally.

                  +--+--+--+--+--+-6x10K--o +5v
  +-------+       |  |  |  |  |  |
  |    P30|<------x--x--x--x--x--x---   COLUMNS     x = 1N4448 diod towards P3/P4
  |    P31|<------x--x--x--x--x--x---    x 16           in serie with key button
  |    P32|<------x--x--x--x--x--x---
  |    P33|<------x--x--x--x--x--x---               A pressed button pulls a P1 row
  |    P34|<------x--x--x--x--x--x---               low when its P3/P4 column is
  |    P35|<------x--x--x--x--x--x---               being scanned
  |    P36|<------x--x--x--x--x--x---
  |    P37|<------x--x--x--x--x--x---
  |    P40|<------x--x--x--x--x--x---
  |    P41|<------x--x--x--x--x--x---
  |    P42|<------x--x--x--x--x--x---
  |    P43|<------x--x--x--x--x--x---
  |    P44|<------x--x--x--x--x--x---
  |    P45|<------x--x--x--x--x--x---
  |    P46|<------x--x--x--x--x--x---
  |    P47|<------x--x--x--x--x--x---
  |       |       |  |  |  |  |  |
  | M6801 |       |  |  |  |  |  |
  |       |     6 x 74HC04 hex inverter
  |P10-P15|<------+--+--+--+--+--+      ROWS x 6
  +-------+

 Credits
 -------
 The internal ROM was dumped in collaboration with Dalby Datormuseum, whom
 also provided documentation and schematics of the keyboard

   https://sites.google.com/site/dalbydatormuseum/home
   https://github.com/MattisLind/6801reader

 **********************************************************************/

#include "emu.h"
#include "eispc_kb.h"
#include "cpu/m6800/m6801.h"

//**************************************************************************
//  CONFIGURABLE LOGGING
//**************************************************************************
#define LOG_PORTS   (1U <<  1)
#define LOG_RESET   (1U <<  2)
#define LOG_BITS    (1U <<  3)
#define LOG_UI      (1U <<  4)
#define LOG_LEDS    (1U <<  5)

//#define VERBOSE (LOG_LEDS)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)
#define LOGRST(...)   LOGMASKED(LOG_RESET, __VA_ARGS__)
#define LOGBITS(...)  LOGMASKED(LOG_BITS,  __VA_ARGS__)
#define LOGUI(...)    LOGMASKED(LOG_UI,    __VA_ARGS__)
#define LOGLEDS(...)  LOGMASKED(LOG_LEDS,  __VA_ARGS__)

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "mcu"

namespace {

INPUT_PORTS_START(eispc_kb)
	PORT_START("P15")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 6")    PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) // 77
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP +")    PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')                                  // 78
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 5")    PORT_CODE(KEYCODE_5_PAD)    PORT_CHAR('5')                                  // 76
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("* PRINT") PORT_CODE(KEYCODE_TILDE)    PORT_CHAR('*') PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) // 55
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("R Shift") PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)                        // 54
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_MINUS)    PORT_CHAR('-') PORT_CHAR('_')                   // 53
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(". :")     PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR(':')                   // 52
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5")      PORT_CODE(KEYCODE_F5)       PORT_CHAR(UCHAR_MAMEKEY(F5))                    // 63
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6")      PORT_CODE(KEYCODE_F6)       PORT_CHAR(UCHAR_MAMEKEY(F6))                    // 64
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                    // no scancode is sent
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL")    PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))              // 29
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(", ;")     PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR(';')                   // 51
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_D)        PORT_CHAR('D') PORT_CHAR('d')                   // 32
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_X)        PORT_CHAR('X') PORT_CHAR('x')                   // 45
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_C)        PORT_CHAR('C') PORT_CHAR('c')                   // 46
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_J)        PORT_CHAR('J') PORT_CHAR('j')                   // 36

	PORT_START("P14")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                        // 00 - keyboard error
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BREAK")   PORT_CODE(KEYCODE_PAUSE)     PORT_CHAR(UCHAR_MAMEKEY(PAUSE))                    // 70
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 7")    PORT_CODE(KEYCODE_7_PAD)     PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(HOME))      // 71
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                        // ff - keyboard error
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('^') PORT_CHAR('~') PORT_CHAR(']')       // 27
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00e5) PORT_CHAR(0x00c5) PORT_CHAR('[') // 26 å Å
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_P)         PORT_CHAR('P') PORT_CHAR('p')                      // 25
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1")      PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))                       // 59
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2")      PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))                       // 60
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')                      // 17
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')                      // 18
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')                      // 24
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')                      // 19
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')                      // 20
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')                      // 21
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                      PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I')                      // 23

	PORT_START("P13")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_NUMLOCK)      PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))        // 69
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                // ff - keyboard error
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BS DEL") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL)) // 14
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')              // 13
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')              // 12
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')              // 11
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')              // 10
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')              // 02
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC")    PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))              // 01
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')              // 03
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')              // 04
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')              // 09
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')              // 05
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')              // 06
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')              // 07
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')              // 08

	PORT_START("P12")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 9")      PORT_CODE(KEYCODE_9_PAD)     PORT_CHAR('9') PORT_CHAR(UCHAR_MAMEKEY(PGUP))   // 73
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP -")      PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))             // 74
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 8")      PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_UP) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(UP)) // 72
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_TILDE)     PORT_CHAR('`') PORT_CHAR('~')       // 41
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_QUOTE)     PORT_CHAR('\'') PORT_CHAR('"')      // 40
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR(':')       // 39
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')       // 38
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3")        PORT_CODE(KEYCODE_F3)        PORT_CHAR(UCHAR_MAMEKEY(F3))        // 61
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4")        PORT_CODE(KEYCODE_F4)        PORT_CHAR(UCHAR_MAMEKEY(F4))        // 62
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')       // 16
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TAB")       PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)                        // 15
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')       // 37
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')       // 33
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')       // 34
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')       // 35
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                        PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')       // 22

	PORT_START("P11")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD )                     PORT_CODE(KEYCODE_DEL_PAD)       PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))        // 83
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)                              // 28
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 0")    PORT_CODE(KEYCODE_0_PAD)        PORT_CHAR('0') PORT_CHAR(UCHAR_MAMEKEY(INSERT)) // 82
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // 89 - no key
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // 86 - no key
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // 87 - no key
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // 88 - no key
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F9")         PORT_CODE(KEYCODE_F9)        PORT_CHAR(UCHAR_MAMEKEY(F9))               // 67
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F10")        PORT_CODE(KEYCODE_F10)       PORT_CHAR(UCHAR_MAMEKEY(F10))              // 68
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // scan code ff - keyboard error
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD )                         PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')                            // 43
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK")  PORT_CODE(KEYCODE_CAPSLOCK)  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))         // 58
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_LALT)                                                 // 56
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                   // 85 - no key
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                         PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')              // 47
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                         PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')                             // 57

	PORT_START("P10")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 3")     PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_PGDN)                           // 81
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                    // ff - keyboard error
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 2")     PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) // 80
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NEW LINE") PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))          // 84 (programmable, default is 28)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 1")     PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD))                   // 79
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("KP 4")     PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_LEFT) PORT_CHAR('4') PORT_CHAR(UCHAR_MAMEKEY(LEFT))     // 75
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )                                                                                                    // ff - keyboard error
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7")       PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))                 // 65
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8")       PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))                 // 66
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)                         // 42
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')                // 44
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')                // 50
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')                // 30
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')                // 31
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')                // 48
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_KEYBOARD )                       PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')                // 49
INPUT_PORTS_END


//-------------------------------------------------
//  ROM( eispc_kb )
//-------------------------------------------------

ROM_START( eispc_kb )
	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "sgtpepper-1.2.bin", 0x000, 0x800, CRC(7107b841) SHA1(a939dd50622575c31fea9c7adb7a7db5403a7aca) )
ROM_END

} // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EISPC_KB, eispc_keyboard_device, "eispc_kb", "Ericsson PC keyboard")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  eispc_keyboard_device - constructor
//-------------------------------------------------

eispc_keyboard_device::eispc_keyboard_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, EISPC_KB, tag, owner, clock)
	, m_mcu(*this, M6801_TAG)
	, m_rows(*this, "P1%u", 0)
	, m_txd_cb(*this)
	, m_led_caps_cb(*this)
	, m_led_num_cb(*this)
	, m_led_scroll_cb(*this)
	, m_rxd_high(true)
	, m_txd_high(true)
	, m_hold(true)
	, m_col_select(0)
{
}

WRITE_LINE_MEMBER(eispc_keyboard_device::rxd_w)
{
	LOGBITS("KBD bit presented: %d\n", state);
	m_rxd_high = CLEAR_LINE != state;
}

WRITE_LINE_MEMBER(eispc_keyboard_device::hold_w)
{
	m_hold = CLEAR_LINE == state;
}

WRITE_LINE_MEMBER(eispc_keyboard_device::rst_line_w)
{
	if (state == CLEAR_LINE)
	{
		m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		LOGRST("KBD: Keyboard mcu reset line is cleared\n");
	}
	else
	{
		m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		LOGRST("KBD: Keyboard mcu reset line is asserted\n");
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void eispc_keyboard_device::device_start()
{
	m_txd_cb.resolve_safe();
	m_led_caps_cb.resolve_safe();
	m_led_num_cb.resolve_safe();
	m_led_scroll_cb.resolve_safe();

	save_item(NAME(m_rxd_high));
	save_item(NAME(m_txd_high));
	save_item(NAME(m_col_select));

	m_rxd_high = true;
	m_txd_high = true;
	m_col_select = 0;
}



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void eispc_keyboard_device::device_add_mconfig(machine_config &config)
{
	M6801(config, m_mcu, XTAL(4'915'200)); // Crystal verified from schematics and visual inspection
	m_mcu->set_addrmap(AS_PROGRAM, &eispc_keyboard_device::eispc_kb_mem);

	m_mcu->in_p1_cb().set([this]
	{
		uint8_t data = 0; // Indicate what keys are pressed in selected column

		for (int i = 0; i < 6; i++) data |= ( (~m_rows[i]->read() & m_col_select) ? 0x20 >> i : 0 );

		// Update txd bit
		data &= 0x3f;
		data |= ((!m_hold || !m_txd_high) ? 0 : 0x40);

		if ((data & 0x3f) != 0 && data != m_p1)
		{
			LOGUI("Reading port 1: %02x m_col_select:%04x\n", data, m_col_select);
			m_p1 = data;
		}
		return data;
	});

	m_mcu->out_p1_cb().set([this](uint8_t data)
	{
		LOGPORTS("Writing %02x PORT 1\n", data);
		LOGLEDS("Num: %d\n", BIT(data, 7));
		m_led_num_cb(BIT(data, 7) ? 1 : 0);
	});

	m_mcu->in_p2_cb().set([this]
	{
		uint8_t data = M6801_MODE_7 | (m_rxd_high ? (1 << 3) : 0);
		LOGPORTS("Reading port 2: %02x\n", data);
		//LOGBITS("KBD: Reading rxd_high: %02x\n", m_rxd_high);
		return data;
	});

	m_mcu->out_p2_cb().set([this](uint8_t data)
	{
		LOGPORTS("Writing port 2: %02x\n", data);
		LOGLEDS("Caps: %d Scroll: %d\n", BIT(data, 0), BIT(data, 2));
		m_led_caps_cb(BIT(data, 0) ? 1 : 0);
		m_led_scroll_cb(BIT(data, 2) ? 1 : 0); // Only working with "Roger Moore" roms
		LOGBITS("KBD: writing bit: %02x\n", BIT(data, 4));
	});

	m_mcu->out_ser_tx_cb().set([this](bool state)
	{
		m_txd_high = CLEAR_LINE != state;
		LOGBITS("KBD: writing bit: %02x\n", m_txd_high);
		m_txd_cb(state);
	});

	m_mcu->in_p3_cb().set([this]
	{
		LOGPORTS("Reading Port 3\n");
		return 0x00;
	});

	m_mcu->out_p3_cb().set([this](uint8_t data)
	{
		m_col_select &= 0xff00;
		m_col_select |= ~data;
	});

	m_mcu->in_p4_cb().set([this]
	{
		LOGPORTS("Reading Port 4\n");
		return 0x00;
	});

	m_mcu->out_p4_cb().set([this](uint8_t data)
	{
		m_col_select &= 0x00ff;
		m_col_select |= (~data << 8);
	});
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor eispc_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( eispc_kb );
}

//-------------------------------------------------
//  ADDRESS_MAP( eispc_kb_mem )
//-------------------------------------------------

void eispc_keyboard_device::eispc_kb_mem(address_map &map)
{
	map(0x0000, 0x001f).m(M6801_TAG, FUNC(m6801_cpu_device::m6801_io));
	map(0x0080, 0x00ff).ram();
	map(0xf800, 0xffff).rom().region(M6801_TAG, 0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *eispc_keyboard_device::device_rom_region() const
{
	return ROM_NAME( eispc_kb );
}
