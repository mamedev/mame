// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
// Originally written in MESS 0.1 by Juergen Buchmueller.
// Substantially rewritten by Robbbert in 2008, many new clones added
/***************************************************************************
TRS80 memory map

0000-2fff ROM                            R   D0-D7
3000-37ff ROM on EACA models             R   D0-D7
          unused on Model I
37de      UART status                    R/W D0-D7
37df      UART data                      R/W D0-D7
37e0      interrupt latch address
37e1      select disk drive 0            W
37e2      cassette drive latch address   W
37e3      select disk drive 1            W
37e4      select which cassette unit     W   D0-D1 (D0 selects unit 1, D1 selects unit 2)
37e5      select disk drive 2            W
37e7      select disk drive 3            W
37e0-37e3 floppy motor                   W   D0-D3
          or floppy head select          W   D3
37e8      send a byte to printer         W   D0-D7
37e8      read printer status            R   D7
37ec-37ef FDC FD1771                     R/W D0-D7
37ec      command                        W   D0-D7
37ec      status                         R   D0-D7
37ed      track                          R/W D0-D7
37ee      sector                         R/W D0-D7
37ef      data                           R/W D0-D7
3800-38ff keyboard matrix                R   D0-D7
3900-3bff unused - kbd mirrored
3c00-3fff video RAM                      R/W D0-D5,D7 (or D0-D7)
4000-ffff RAM

Interrupts:
IRQ mode 1
NMI

Printer: Level II usually 37e8; System80 uses port FD.

System80 has non-addressable dip switches to set the UART control register.
System80 has non-addressable links to set the baud rate. Receive and Transmit clocks are tied together.

Cassette baud rates:    Model I level I - 250 baud
        Model I level II and all clones - 500 baud

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on, enables cassette data paths on a system-80
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

FE:
- bit 4 selects internal cassette player (low) or external unit (high) on a system-80

FD:
- Read printer status on a system-80
- Write to printer on a system-80

F9:
- UART data (write) status (read) on a system-80

F8:
- UART data (read) status (write) on a system-80

Shift and Right-arrow will enable 32 cpl, if the hardware allows it.

SYSTEM commands:
    - Press Break (End key) to quit
    - Press Enter to exit with error
    - xxxx to load program xxxx from tape.
    - / to execute last program loaded
    - /nnnnn to execute program at nnnnn (decimal)

About the system80 - Asian version of trs80l2, known as EACA Video Genie. In USA called
    PMC-80, in South Africa called TRZ-80, and Dick Smith imported them to Australia and
    New Zealand as the System 80. The Hungarian version is the ht1080z.
    Inbuilt extensions:
    - SYSTEM then /12288 = enable extended keyboard and flashing block cursor
    - SYSTEM then /12299 = turn cursor back to normal
    - SYSTEM then /12294 = enable extended keyboard only
    - SYSTEM then /12710 = enter machine-language monitor
    Monitor commands:
    - B : return to Basic
    - Dnnnn : Dump hex to screen. Press down-arrow for more. Press enter to quit.
    - Mnnnn : Modify memory. Enter new byte and it increments to next address. X to quit.
    - Gnnnn : Execute program at nnnn
    - Gnnnn,tttt : as above, breakpoint at tttt
    - R : modify registers

About the ht1080z - This was made for schools in Hungary. Each comes with a BASIC extension roms
    which activated Hungarian features. To activate - start emulation - enter SYSTEM
    Enter /12288 and the extensions will be installed and you are returned to READY.
    The ht1080z is identical to the System 80, apart from the character rom.
    The ht1080z2 has a modified extension rom and character generator.

About the eg3003 - This is the original of the EACA clones, and enjoyed success in Europe,
    particularly in Germany. The normal roms would make it exactly a System-80, however we've
    added the TCS ROM extension for something different. To activate - enter SYSTEM
    Enter /12345 and the inbuilt monitor will be ready to go. To start the monitor, hold
    up-arrow and hit M. You get a # prompt. The keyboard is also now in lower-case, even
    though monitor commands are required to be in upper-case. Monitor commands:
    - A : Ascii Dump
    - D : Hex dump
    - E : Edit Memory
    - H : Hex converter
    - J : Jump (Go)
    - P : Punch
    - R : Return to BASIC
    - S : Search
    - X : Hex Calculator

About the RTC - The time is incremented while ever the cursor is flashing. It is stored in a series
    of bytes in the computer's work area. The bytes are in a certain order, this is:
    seconds, minutes, hours, year, day, month. The seconds are stored at 0x4041.
    A reboot always sets the time to zero.

Not dumped (to our knowledge):
 TRS80 Japanese bios
 TRS80 Katakana Character Generator
 TRS80 Small English Character Generator
 TRS80 Model III old version Character Generator

Not emulated:
 TRS80 Japanese kana/ascii switch and alternate keyboard
 TRS80 Model III/4 Hard drive, Graphics board, Alternate Character set
 Radionic has 16 colours with a byte at 350B controlling the operation. See manual.


********************************************************************************************************

To Do / Status:
--------------

- For those machines that allow it, add cass2 as an image device and hook it up.
- Difficulty loading real tapes.
- Writing to floppy is problematic; freezing/crashing are common issues.

trs80:     works

trs80l2:   works
           expansion-box to be slotified

sys80:     works
           investigate expansion-box

ht1080z    works
           verify clock for AY-3-8910
           investigate expansion-box

*******************************************************************************************************/

#include "emu.h"
#include "trs80.h"
#include "machine/input_merger.h"
#include "sound/ay8910.h"
#include "softlist_dev.h"


void trs80_state::trs80_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x3800, 0x3bff).r(FUNC(trs80_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0x7fff).ram();
}

void trs80_state::trs80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xff, 0xff).rw(FUNC(trs80_state::port_ff_r), FUNC(trs80_state::port_ff_w));
}

void trs80_state::m1_mem(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x37de, 0x37de).rw(FUNC(trs80_state::sys80_f9_r), FUNC(trs80_state::sys80_f8_w));
	map(0x37df, 0x37df).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0x37e0, 0x37e3).rw(FUNC(trs80_state::irq_status_r), FUNC(trs80_state::motor_w));
	map(0x37e4, 0x37e7).w(FUNC(trs80_state::cassunit_w));
	map(0x37e8, 0x37eb).rw(FUNC(trs80_state::printer_r), FUNC(trs80_state::printer_w));
	map(0x37ec, 0x37ef).rw(FUNC(trs80_state::fdc_r), FUNC(trs80_state::fdc_w));
	map(0x3800, 0x3bff).r(FUNC(trs80_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0xffff).ram();
}

void trs80_state::m1_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xe8, 0xe8).rw(FUNC(trs80_state::port_e8_r), FUNC(trs80_state::port_e8_w));
	map(0xe9, 0xe9).portr("E9").w("brg", FUNC(com8116_device::stt_str_w));
	map(0xea, 0xea).rw(FUNC(trs80_state::port_ea_r), FUNC(trs80_state::port_ea_w));
	map(0xeb, 0xeb).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xff, 0xff).rw(FUNC(trs80_state::port_ff_r), FUNC(trs80_state::port_ff_w));
}

void trs80_state::sys80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xf8, 0xf8).r(m_uart, FUNC(ay31015_device::receive)).w(FUNC(trs80_state::sys80_f8_w));
	map(0xf9, 0xf9).r(FUNC(trs80_state::sys80_f9_r)).w(m_uart, FUNC(ay31015_device::transmit));
	map(0xfd, 0xfd).rw(FUNC(trs80_state::printer_r), FUNC(trs80_state::printer_w));
	map(0xfe, 0xfe).w(FUNC(trs80_state::sys80_fe_w));
	map(0xff, 0xff).rw(FUNC(trs80_state::port_ff_r), FUNC(trs80_state::port_ff_w));
}

void trs80_state::ht1080z_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	sys80_io(map);
	map(0x1e, 0x1e).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x1f, 0x1f).w("ay1", FUNC(ay8910_device::address_w));
}

/**************************************************************************
   w/o SHIFT                             with SHIFT
   +-------------------------------+     +-------------------------------+
   | 0   1   2   3   4   5   6   7 |     | 0   1   2   3   4   5   6   7 |
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
|0 | @ | A | B | C | D | E | F | G |  |0 | ` | a | b | c | d | e | f | g |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|1 | H | I | J | K | L | M | N | O |  |1 | h | i | j | k | l | m | n | o |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|2 | P | Q | R | S | T | U | V | W |  |2 | p | q | r | s | t | u | v | w |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|3 | X | Y | Z | [ | \ | ] | ^ | _ |  |3 | x | y | z | { | | | } | ~ |   |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  |4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|5 | 8 | 9 | : | ; | , | - | . | / |  |5 | 8 | 9 | * | + | < | = | > | ? |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|  |6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|7 |SHF|   |   |   |   |   |   |   |  |7 |SHF|   |   |   |   |   |   |   |
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+

***************************************************************************/

static INPUT_PORTS_START( trs80 )
	PORT_START("LINE0")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)          PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)          PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)          PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)          PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)          PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)          PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)          PORT_CHAR('G') PORT_CHAR('g')

	PORT_START("LINE1")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)          PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)          PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)          PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)          PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)          PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)          PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)          PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)          PORT_CHAR('O') PORT_CHAR('o')

	PORT_START("LINE2")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)          PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)          PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)          PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)          PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)          PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)          PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)          PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)          PORT_CHAR('W') PORT_CHAR('w')

	PORT_START("LINE3")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)          PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)          PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)          PORT_CHAR('Z') PORT_CHAR('z')
	// These keys produce output on all systems, the shift key having no effect. They display arrow symbols and underscore.
	// On original TRS80, shift cancels all keys except F3 which becomes backspace.
	// PORT_CHAR('_') is correct for all systems.
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_F2)   // sys80 mkII: F2
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_F3)   // sys80 mkII: F3
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_F4)  // sys80 mkII: F4
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_F5)         PORT_CHAR('_')  // sys80 mkII: F1

	PORT_START("LINE4") // Number pad: System 80 Mk II only
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)          PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME(": *") PORT_CODE(KEYCODE_MINUS)    PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("; +") PORT_CODE(KEYCODE_COLON)    PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)    PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("- =") PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP)     PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)    PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("LINE6")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)      PORT_CHAR(13)
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(F8))   // Missing from early System 80
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	/* backspace do the same as cursor left */
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))   // Missing from early System 80
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0xfe, 0x00, IPT_UNUSED)

	PORT_START("RESET") // special button
	PORT_BIT(0x01, 0x00, IPT_OTHER) PORT_NAME("Reset") PORT_CODE(KEYCODE_DEL) PORT_WRITE_LINE_DEVICE_MEMBER("nmigate", input_merger_device, in_w<0>)
INPUT_PORTS_END

static INPUT_PORTS_START(trs80l2)
	PORT_INCLUDE (trs80)
	PORT_START("CONFIG")
	PORT_CONFNAME(    0x80, 0x00,   "Floppy Disc Drives")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(   0x80, DEF_STR( On ) )
	PORT_BIT(0x7f, 0x7f, IPT_UNUSED)

	PORT_START("E9")    // these are the power-on uart settings
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x88, 0x08, "Parity")
	PORT_DIPSETTING(    0x08, DEF_STR(None))
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x80, "Even")
	PORT_DIPNAME( 0x10, 0x10, "Stop Bits")
	PORT_DIPSETTING(    0x10, "2")
	PORT_DIPSETTING(    0x00, "1")
	PORT_DIPNAME( 0x60, 0x60, "Bits")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x20, "6")
	PORT_DIPSETTING(    0x40, "7")
	PORT_DIPSETTING(    0x60, "8")
INPUT_PORTS_END

static INPUT_PORTS_START(sys80)
	PORT_INCLUDE (trs80l2)
	PORT_MODIFY("CONFIG")
	PORT_CONFNAME(    0x08, 0x00,   "Video Cut")  // Toggle switch on the back
	PORT_CONFSETTING(   0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(   0x08, DEF_STR( On ) )
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Page") PORT_CODE(KEYCODE_F6) PORT_TOGGLE  // extra keys above the main keyboard
	//PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F7) PORT_TOGGLE  // this turns on the tape motor
	PORT_START("BAUD")
	PORT_DIPNAME( 0xff, 0x06, "Baud Rate")
	PORT_DIPSETTING(    0x00, "110")
	PORT_DIPSETTING(    0x01, "300")
	PORT_DIPSETTING(    0x02, "600")
	PORT_DIPSETTING(    0x03, "1200")
	PORT_DIPSETTING(    0x04, "2400")
	PORT_DIPSETTING(    0x05, "4800")
	PORT_DIPSETTING(    0x06, "9600")
	PORT_DIPSETTING(    0x07, "19200")
INPUT_PORTS_END



/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
static const gfx_layout trs80_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8        /* every char takes 8 bytes */
};

static const gfx_layout ht1080z_charlayout =
{
	5, 12,          /* 5 x 12 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16           /* every char takes 16 bytes */
};

static GFXDECODE_START(gfx_trs80)
	GFXDECODE_ENTRY( "chargen", 0, trs80_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(gfx_ht1080z)
	GFXDECODE_ENTRY( "chargen", 0, ht1080z_charlayout, 0, 1 )
GFXDECODE_END


void trs80_state::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_JV1_FORMAT);
}

// Most images are single-sided, 40 tracks or less.
// However, the default is QD to prevent MAME from
// crashing if a disk with more than 40 tracks is used.
static void trs80_floppies(device_slot_interface &device)
{
	device.option_add("35t_sd", FLOPPY_525_SSSD_35T);
	device.option_add("40t_sd", FLOPPY_525_SSSD);
	device.option_add("40t_dd", FLOPPY_525_DD);
	device.option_add("80t_qd", FLOPPY_525_QD);
}


void trs80_state::level1(machine_config &config)      // the original model I, level I, with no extras
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 10.6445_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::trs80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::trs80_io);
	m_maincpu->halt_cb().set("nmigate", FUNC(input_merger_device::in_w<1>));

	input_merger_device &nmigate(INPUT_MERGER_ANY_HIGH(config, "nmigate"));
	nmigate.output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI); // TODO: also causes SYSRES on expansion bus

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.6445_MHz_XTAL, 672, 0, 384, 264, 0, 192);
	screen.set_screen_update(FUNC(trs80_state::screen_update_trs80));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_trs80);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(trs80l1_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("trs80_cass");

	/* software lists */
	SOFTWARE_LIST(config, "cass_list").set_original("trs80_cass").set_filter("0");
}

void trs80_state::level2(machine_config &config)      // model I, level II
{
	level1(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::m1_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::m1_io);
	m_maincpu->set_periodic_int(FUNC(trs80_state::rtc_interrupt), attotime::from_hz(40));

	/* devices */
	m_cassette->set_formats(trs80l2_cassette_formats);

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "cmd", attotime::from_seconds(1)));
	quickload.set_load_callback(FUNC(trs80_state::quickload_cb));
	quickload.set_interface("trs80_quik");

	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(trs80_state::intrq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], trs80_floppies, "80t_qd", trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], trs80_floppies, "80t_qd", trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], trs80_floppies, nullptr, trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], trs80_floppies, nullptr, trs80_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit6));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit5));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit4));

	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	com8116_device &brg(COM8116(config, "brg", 5.0688_MHz_XTAL));   // BR1941L
	brg.fr_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	brg.ft_handler().set(m_uart, FUNC(ay31015_device::write_tcp));

	AY31015(config, m_uart);
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	//MCFG_AY31015_WRITE_DAV_CB(WRITELINE( , , ))
	m_uart->set_auto_rdav(true);
	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);

	SOFTWARE_LIST(config.replace(), "cass_list").set_original("trs80_cass").set_filter("1");
	SOFTWARE_LIST(config, "quik_list").set_original("trs80_quik").set_filter("1");
	SOFTWARE_LIST(config, "flop_list").set_original("trs80_flop").set_filter("1");
}

void trs80_state::sys80(machine_config &config)
{
	level2(config);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::sys80_io);
	m_maincpu->halt_cb().set_nop(); // TODO: asserts HLTA on expansion bus instead

	subdevice<screen_device>("screen")->set_screen_update(FUNC(trs80_state::screen_update_sys80));

	config.device_remove("brg");
	CLOCK(config, m_uart_clock, 19200 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_tcp));
}

void trs80_state::sys80p(machine_config &config)
{
	sys80(config);
	m_maincpu->set_clock(10.48_MHz_XTAL / 6);
	subdevice<screen_device>("screen")->set_raw(10.48_MHz_XTAL, 672, 0, 384, 312, 0, 192);
}

void trs80_state::ht1080z(machine_config &config)
{
	sys80p(config);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::ht1080z_io);

	subdevice<screen_device>("screen")->set_screen_update(FUNC(trs80_state::screen_update_ht1080z));
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_ht1080z);

	AY8910(config, "ay1", 1'500'000).add_route(ALL_OUTPUTS, "mono", 0.25); // guess of clock
	//ay1.port_a_read_callback(FUNC(trs80_state::...);  // ports are some kind of expansion slot
	//ay1.port_b_read_callback(FUNC(trs80_state::...);

	SOFTWARE_LIST(config.replace(), "cass_list").set_original("trs80_cass").set_filter("H");
	SOFTWARE_LIST(config.replace(), "quik_list").set_original("trs80_quik").set_filter("H");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(trs80)
	ROM_REGION(0x3800, "maincpu", ROMREGION_ERASEFF)
	// These roms had many names due to multiple suppliers
	// Memory   Location    Maker and type                Label
	// 000-7FF  Z33         Intel 2716                    ROM-A
	// 800-FFF  Z34         Intel 2716                    ROM-B
	// 000-7FF  Z33         National Semiconductor 2316   MM2316_R/D
	// 800-FFF  Z34         National Semiconductor 2316   MM2316_S/D
	// 000-7FF  Z33         National Semiconductor 2316   M2316E_R/N
	// 800-FFF  Z34         National Semiconductor 2316   M2316E_S/N
	// 000-7FF  Z33         Motorola                      7807
	// 800-FFF  Z34         Motorola                      7804
	// 000-FFF  Z33         Motorola                      7809_BASIC I
	ROM_LOAD("level1.rom",     0x0000, 0x1000, CRC(70d06dff) SHA1(20d75478fbf42214381e05b14f57072f3970f765) )

	ROM_REGION(0x0400, "chargen", 0)
	ROM_LOAD("mcm6670p.z29",   0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7) )
ROM_END


ROM_START(trs80l2)
	ROM_REGION(0x3800, "maincpu", ROMREGION_ERASEFF)
	// There's no space for these roms on a Model 1 board, so an extra ROM board was created to hold them.
	// This board plugs into either Z33 or Z34. Confusingly, the locations on this board are also Z numbers.
	// The last version of the board only holds 2 roms - Z1 as 8K (ROM A/B), and Z2 as 4K (ROM C).
	ROM_SYSTEM_BIOS(0, "level2", "Radio Shack Level II Basic")
	ROMX_LOAD("rom-a.z1",      0x0000, 0x1000, CRC(37c59db2) SHA1(e8f8f6a4460a6f6755873580be6ff70cebe14969), ROM_BIOS(0) )
	ROMX_LOAD("rom-b.z2",      0x1000, 0x1000, CRC(05818718) SHA1(43c538ca77623af6417474ca5b95fb94205500c1), ROM_BIOS(0) )
	ROMX_LOAD("rom-c.z3",      0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "rsl2", "R/S L2 Basic")
	ROMX_LOAD("rom-a_alt.z1",  0x0000, 0x1000, CRC(be46faf5) SHA1(0e63fc11e207bfd5288118be5d263e7428cc128b), ROM_BIOS(1) )
	ROMX_LOAD("rom-b_alt.z2",  0x1000, 0x1000, CRC(6c791c2d) SHA1(2a38e0a248f6619d38f1a108eea7b95761cf2aee), ROM_BIOS(1) )
	ROMX_LOAD("rom-c_alt.z3",  0x2000, 0x1000, CRC(55b3ad13) SHA1(6279f6a68f927ea8628458b278616736f0b3c339), ROM_BIOS(1) )

	ROM_REGION(0x0400, "chargen", 0)
	ROM_LOAD("mcm6670p.z29",   0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7) )
ROM_END


// From here are EACA-made clones

ROM_START(eg3003)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("3001.z10",       0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b) )
	ROM_LOAD("3002.z11",       0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155) )
	ROM_LOAD("3003.z12",       0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	ROM_LOAD("tcs-ext.z13",    0x3000, 0x0800, CRC(8f2ac112) SHA1(be0c2a5fb9cb01173c4da6dc8c71ca5975f441bb) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("tcs-ext.z25",    0x0000, 0x0800, CRC(150c5f1f) SHA1(afbce73ab0360108b32e75eb75a3966eb5c503e7) )
ROM_END


ROM_START(sys80)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("3001.z10",       0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b) )
	ROM_LOAD("3002.z11",       0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155) )
	ROM_LOAD("3003.z12",       0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	/* This rom turns the system80 into the "blue label" version. SYSTEM then /12288 to activate. */
	ROM_LOAD("sys80.z13",      0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715) )

	ROM_REGION(0x0400, "chargen", 0)
	// Z25 could be 2513 (early version) or 52116 (later version)
	// This rom is Z25 on the video board, not Z25 on the CPU board.
	ROM_LOAD("2513.z25",       0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7) )
ROM_END

#define rom_sys80p rom_sys80

// Although I don't have schematics for the HT-series, it would be reasonable to expect the board locations to be the same
ROM_START(ht1080z)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("3001.z10",       0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b) )
	ROM_LOAD("3002.z11",       0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155) )
	ROM_LOAD("3003.z12",       0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	ROM_LOAD("sys80.z13",      0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("ht1080z.z25",    0x0000, 0x0800, CRC(e8c59d4f) SHA1(a15f30a543e53d3e30927a2e5b766fcf80f0ae31) )
ROM_END


ROM_START(ht1080z2)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("3001.z10",       0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b) )
	ROM_LOAD("3002.z11",       0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155) )
	ROM_LOAD("3003.z12",       0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	ROM_LOAD("ht1080z2.z13",   0x3000, 0x0800, CRC(07415ac6) SHA1(b08746b187946e78c4971295c0aefc4e3de97115) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("ht1080z2.z25",   0x0000, 0x0800, CRC(6728f0ab) SHA1(1ba949f8596f1976546f99a3fdcd3beb7aded2c5) )
ROM_END


ROM_START(ht108064)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("3001_64.z10",    0x0000, 0x1000, CRC(59ec132e) SHA1(232c04827e494ea49931d7ab9a5b87b76c81aef1) )
	ROM_LOAD("3002_64.z11",    0x1000, 0x1000, CRC(a7a73e8c) SHA1(6e0f232b8666744328853cef6bb72b8e44b4c184) )
	ROM_LOAD("3003.z12",       0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	ROM_LOAD("ht108064.z13",   0x3000, 0x0800, CRC(fc12bd28) SHA1(0da93a311f99ec7a1e77486afe800a937778e73b) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("ht108064.z25",   0x0000, 0x0800, CRC(e76b73a4) SHA1(6361ee9667bf59d50059d09b0baf8672fdb2e8af) )
ROM_END


void trs80_state::init_trs80l2()
{
	m_7bit = true;
}


//    YEAR  NAME         PARENT    COMPAT  MACHINE   INPUT    CLASS        INIT           COMPANY                        FULLNAME                           FLAGS
COMP( 1977, trs80,       0,        0,       level1,   trs80,   trs80_state, empty_init,    "Tandy Radio Shack",           "TRS-80 Model I (Level I Basic)",  MACHINE_SUPPORTS_SAVE )
COMP( 1978, trs80l2,     0,        0,       level2,   trs80l2, trs80_state, init_trs80l2,  "Tandy Radio Shack",           "TRS-80 Model I (Level II Basic)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, eg3003,      0,        trs80l2, sys80,    sys80,   trs80_state, init_trs80l2,  "EACA Computers Ltd",          "Video Genie EG3003",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, sys80,       eg3003,   0,       sys80,    sys80,   trs80_state, init_trs80l2,  "EACA Computers Ltd",          "System-80 (60 Hz)",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, sys80p,      eg3003,   0,       sys80p,   sys80,   trs80_state, init_trs80l2,  "EACA Computers Ltd",          "System-80 (50 Hz)",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1983, ht1080z,     eg3003,   0,       ht1080z,  sys80,   trs80_state, init_trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series I",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1984, ht1080z2,    eg3003,   0,       ht1080z,  sys80,   trs80_state, init_trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series II",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1985, ht108064,    eg3003,   0,       ht1080z,  sys80,   trs80_state, empty_init,    "Hiradastechnika Szovetkezet", "HT-1080Z/64",                     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
