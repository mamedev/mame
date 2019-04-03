// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
// Originally written in MESS 0.1 by Juergen Buchmueller.
// Substantially rewritten by Robbbert in 2008, many new clones added
/***************************************************************************
TRS80 memory map

0000-2fff ROM                 R   D0-D7
3000-37ff ROM on Model III        R   D0-D7
      unused on Model I
37de      UART status             R/W D0-D7
37df      UART data           R/W D0-D7
37e0      interrupt latch address (lnw80 = for the realtime clock)
37e1      select disk drive 0         W
37e2      cassette drive latch address    W
37e3      select disk drive 1         W
37e4      select which cassette unit      W   D0-D1 (D0 selects unit 1, D1 selects unit 2)
37e5      select disk drive 2         W
37e7      select disk drive 3         W
37e0-37e3 floppy motor            W   D0-D3
      or floppy head select   W   D3
37e8      send a byte to printer          W   D0-D7
37e8      read printer status             R   D7
37ec-37ef FDC WD179x              R/W D0-D7
37ec      command             W   D0-D7
37ec      status              R   D0-D7
37ed      track               R/W D0-D7
37ee      sector              R/W D0-D7
37ef      data                R/W D0-D7
3800-38ff keyboard matrix         R   D0-D7
3900-3bff unused - kbd mirrored
3c00-3fff video RAM               R/W D0-D5,D7 (or D0-D7)
4000-ffff RAM

Interrupts:
IRQ mode 1
NMI

Printer: Level II usually 37e8; System80 uses port FD.

System80 has non-addressable dip switches to set the UART control register.
System80 and LNW80 have non-addressable links to set the baud rate. Receive and Transmit clocks are tied together.

Cassette baud rates:    Model I level I - 250 baud
        Model I level II and all clones - 500 baud
        LNW-80 - 500 baud @1.77MHz and 1000 baud @4MHz.

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on, enables cassette data paths on a system-80
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

FE:
- bit 0 is for selecting inverse video of the whole screen on a lnw80
- bit 2 enables colour on a lnw80
- bit 3 is for selecting roms (low) or 16k hires area (high) on a lnw80
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
    The monitor works on the radionic too.

About the ht1080z - This was made for schools in Hungary. Each comes with a BASIC extension roms
    which activated Hungarian features. To activate - start emulation - enter SYSTEM
    Enter /12288 and the extensions will be installed and you are returned to READY.
    The ht1080z is identical to the System 80, apart from the character rom.
    The ht1080z2 has a modified extension rom and character generator.

About the RTC - The time is incremented while ever the cursor is flashing. It is stored in a series
    of bytes in the computer's work area. The bytes are in a certain order, this is:
    seconds, minutes, hours, year, day, month. The seconds are stored at 0x4041.
    A reboot always sets the time to zero.

Not dumped (to our knowledge):
 TRS80 Japanese bios
 TRS80 Katakana Character Generator
 TRS80 Small English Character Generator
 TRS80 Model III old version Character Generator
 TRS80 Model II bios and boot disk

Not emulated:
 TRS80 Japanese kana/ascii switch and alternate keyboard
 TRS80 Model III/4 Hard drive, Graphics board, Alternate Character set
 LNW80 1.77 / 4.0 MHz switch (this is a physical switch)
 Radionic has 16 colours with a byte at 350B controlling the operation. See manual.

Virtual floppy disk formats are JV1, JV3, and DMK. Only the JV1 is emulated.

********************************************************************************************************

To Do / Status:
--------------

For those machines that allow it, add cass2 as an image device and hook it up.

trs80:     works

trs80l2:   works
           expansion-box to be slotified

sys80:     works
           investigate expansion-box
           add 32 / 64 cpl switch

ht1080z    works
           verify clock for AY-3-8910
           investigate expansion-box

radionic:  works
           floppy not working (@6C0, DRQ never gets set)
           add colour
           expansion-box?
           uart

lnw80:     works
           add 1.77 / 4 MHz switch
           find out if it really did support 32-cpl mode or not
           hi-res and colour are coded but do not work
           investigate expansion-box

*******************************************************************************************************/

#include "emu.h"
#include "includes/trs80.h"

#include "machine/com8116.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"

#include "formats/trs80_dsk.h"
#include "formats/dmk_dsk.h"


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
	map(0x0000, 0x377f).rom(); // sys80,ht1080 needs up to 375F
	map(0x37de, 0x37de).rw(FUNC(trs80_state::sys80_f9_r), FUNC(trs80_state::sys80_f8_w));
	map(0x37df, 0x37df).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0x37e0, 0x37e3).rw(FUNC(trs80_state::irq_status_r), FUNC(trs80_state::motor_w));
	map(0x37e4, 0x37e7).w(FUNC(trs80_state::cassunit_w));
	map(0x37e8, 0x37eb).rw(FUNC(trs80_state::printer_r), FUNC(trs80_state::printer_w));
	map(0x37ec, 0x37ec).r(FUNC(trs80_state::wd179x_r));
	map(0x37ec, 0x37ec).w(m_fdc, FUNC(fd1793_device::cmd_w));
	map(0x37ed, 0x37ed).rw(m_fdc, FUNC(fd1793_device::track_r), FUNC(fd1793_device::track_w));
	map(0x37ee, 0x37ee).rw(m_fdc, FUNC(fd1793_device::sector_r), FUNC(fd1793_device::sector_w));
	map(0x37ef, 0x37ef).rw(m_fdc, FUNC(fd1793_device::data_r), FUNC(fd1793_device::data_w));
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

void trs80_state::lnw80_mem(address_map &map)
{
	map(0x0000, 0x3fff).m(m_lnw_bank, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0xffff).ram();
}

void trs80_state::lnw_banked_mem(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x37e0, 0x37e3).rw(FUNC(trs80_state::irq_status_r), FUNC(trs80_state::motor_w));
	map(0x37e8, 0x37eb).rw(FUNC(trs80_state::printer_r), FUNC(trs80_state::printer_w));
	map(0x37ec, 0x37ec).r(FUNC(trs80_state::wd179x_r));
	map(0x37ec, 0x37ec).w(m_fdc, FUNC(fd1793_device::cmd_w));
	map(0x37ed, 0x37ed).rw(m_fdc, FUNC(fd1793_device::track_r), FUNC(fd1793_device::track_w));
	map(0x37ee, 0x37ee).rw(m_fdc, FUNC(fd1793_device::sector_r), FUNC(fd1793_device::sector_w));
	map(0x37ef, 0x37ef).rw(m_fdc, FUNC(fd1793_device::data_r), FUNC(fd1793_device::data_w));
	map(0x3800, 0x3bff).r(FUNC(trs80_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0x7fff).ram().share(m_p_gfxram).region("gfx2", 0);
}

void trs80_state::lnw80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xe8, 0xe8).rw(FUNC(trs80_state::port_e8_r), FUNC(trs80_state::port_e8_w));
	map(0xe9, 0xe9).portr("E9");
	map(0xea, 0xea).rw(FUNC(trs80_state::port_ea_r), FUNC(trs80_state::port_ea_w));
	map(0xeb, 0xeb).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xfe, 0xfe).rw(FUNC(trs80_state::lnw80_fe_r), FUNC(trs80_state::lnw80_fe_w));
	map(0xff, 0xff).rw(FUNC(trs80_state::port_ff_r), FUNC(trs80_state::port_ff_w));
}

void trs80_state::radionic_mem(address_map &map)
{
	m1_mem(map);
	// Optional external RS232 module with 8251
	//map(0x3400, 0x3401).mirror(0xfe).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// Internal colour controls (need details)
	//map(0x3500, 0x35ff).w(FUNC(trs80_state::colour_w));
	// Internal interface to external slots
	map(0x3600, 0x3603).mirror(0xfc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
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
	// On the radionic, Shift works, and the symbols display correctly.
	// PORT_CHAR('_') is correct for all systems, however the others are only for radionic.
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_F1)  PORT_CHAR('[') PORT_CHAR('{')  // radionic: F1
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_F2) PORT_CHAR('\\') PORT_CHAR('}') // radionic: F2 ; sys80 mkII: F2 ; lnw80: F1
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_F3)  PORT_CHAR(']') PORT_CHAR('|')  // radionic: F3 ; sys80 mkII: F3
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_F4)  PORT_CHAR('^')                 // radionic: F4 ; sys80 mkII: F4 ; lnw80: F2
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_F5)  PORT_CHAR('_')                 // radionic: LF ; sys80 mkII: F1 ; lnw80: _

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
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL) // LNW80 only
	PORT_BIT(0xee, 0x00, IPT_UNUSED)
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

static const gfx_layout lnw80_charlayout =
{
	8, 8,           /* 8 x 8 characters */
	128,            /* 128 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 7, 5, 6, 1, 0, 2, 4, 3 },
	/* y offsets */
	{  0*8, 512*8, 256*8, 768*8, 1*8, 513*8, 257*8, 769*8 },
	8*2        /* every char takes 8 bytes */
};

static const gfx_layout radionic_charlayout =
{
	8, 16,          /* 8 x 16 characters */
	256,            /* 256 characters */
	1,          /* 1 bits per pixel */
	{ 0 },          /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{  0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8        /* every char takes 16 bytes */
};

static GFXDECODE_START(gfx_trs80)
	GFXDECODE_ENTRY( "chargen", 0, trs80_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(gfx_ht1080z)
	GFXDECODE_ENTRY( "chargen", 0, ht1080z_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START(gfx_lnw80)
	GFXDECODE_ENTRY( "chargen", 0, lnw80_charlayout, 0, 4 )
GFXDECODE_END

static GFXDECODE_START(gfx_radionic)
	GFXDECODE_ENTRY( "chargen", 0, radionic_charlayout, 0, 1 )
GFXDECODE_END


FLOPPY_FORMATS_MEMBER( trs80_state::floppy_formats )
	FLOPPY_TRS80_FORMAT,
	FLOPPY_DMK_FORMAT
FLOPPY_FORMATS_END

static void trs80_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_QD);
}


void trs80_state::trs80(machine_config &config)       // the original model I, level I, with no extras
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 10.6445_MHz_XTAL / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::trs80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::trs80_io);

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
	WAVE(config, "wave", m_cassette).add_route(ALL_OUTPUTS, "mono", 0.05);

	/* devices */
	CASSETTE(config, m_cassette);
}

MACHINE_CONFIG_START(trs80_state::model1)      // model I, level II
	trs80(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::m1_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::m1_io);
	m_maincpu->set_periodic_int(FUNC(trs80_state::rtc_interrupt), attotime::from_hz(40));

	/* devices */
	m_cassette->set_formats(trs80l2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);

	MCFG_QUICKLOAD_ADD("quickload", trs80_state, trs80_cmd, "cmd", attotime::from_seconds(1))

	FD1793(config, m_fdc, 4_MHz_XTAL / 4); // todo: should be fd1771
	m_fdc->intrq_wr_callback().set(FUNC(trs80_state::intrq_w));

	FLOPPY_CONNECTOR(config, "fdc:0", trs80_floppies, "sssd", trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", trs80_floppies, "sssd", trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", trs80_floppies, "", trs80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", trs80_floppies, "", trs80_state::floppy_formats).enable_sound(true);

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
MACHINE_CONFIG_END

void trs80_state::sys80(machine_config &config)
{
	model1(config);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::sys80_io);

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
}

void trs80_state::lnw80(machine_config &config)
{
	model1(config);
	//m_maincpu->set_clock(16_MHz_XTAL / 4); // or 16MHz / 9; 4MHz or 1.77MHz operation selected by HI/LO switch
	m_maincpu->set_clock(16_MHz_XTAL / 9); // need this so cassette can work
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::lnw80_mem);
	m_maincpu->set_addrmap(AS_IO, &trs80_state::lnw80_io);

	ADDRESS_MAP_BANK(config, m_lnw_bank, 0);
	m_lnw_bank->set_addrmap(0, &trs80_state::lnw_banked_mem);
	m_lnw_bank->set_data_width(8);
	m_lnw_bank->set_addr_width(16);
	m_lnw_bank->set_stride(0x4000);

	MCFG_MACHINE_RESET_OVERRIDE(trs80_state, lnw80)

	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_lnw80);

	subdevice<palette_device>("palette")->set_entries(8).set_init(FUNC(trs80_state::lnw80_palette));
	subdevice<screen_device>("screen")->set_raw(3.579545_MHz_XTAL * 3, 682, 0, 480, 264, 0, 192); // 10.738MHz generated by tank circuit (top left of page 2 of schematics)
	// LNW80 Theory of Operations gives H and V periods as 15.750kHz and 59.66Hz, probably due to rounding the calculated ~15.7468kHz to 4 figures
	subdevice<screen_device>("screen")->set_screen_update(FUNC(trs80_state::screen_update_lnw80));

	config.device_remove("brg");
	CLOCK(config, m_uart_clock, 19200 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_tcp));
}

void trs80_state::radionic(machine_config &config)
{
	model1(config);
	m_maincpu->set_clock(12_MHz_XTAL / 6); // or 3.579MHz / 2 (selectable?)
	// Komtek I "User Friendly Manual" calls for "Z80 running at 1.97 MHz." This likely refers to an alternate NTSC version
	// whose master clock was approximately 11.8005 MHz (6 times ~1.966 MHz and 750 times 15.734 kHz). Though the schematics
	// provide the main XTAL frequency as 12 MHz, that they also include a 3.579 MHz XTAL suggests this possibility.
	m_maincpu->set_periodic_int(FUNC(trs80_state::nmi_line_pulse), attotime::from_hz(12_MHz_XTAL / 12 / 16384));
	m_maincpu->set_addrmap(AS_PROGRAM, &trs80_state::radionic_mem);

	subdevice<screen_device>("screen")->set_raw(12_MHz_XTAL, 768, 0, 512, 312, 0, 256);
	subdevice<screen_device>("screen")->set_screen_update(FUNC(trs80_state::screen_update_radionic));
	subdevice<gfxdecode_device>("gfxdecode")->set_info(gfx_radionic);

	// Interface to external circuits
	I8255(config, m_ppi);
	//m_ppi->in_pc_callback().set(FUNC(pulsar_state::ppi_pc_r));      // Sensing from external and printer status
	//m_ppi->out_pa_callback().set(FUNC(pulsar_state::ppi_pa_w));    // Data for external plugin printer module
	//m_ppi->out_pb_callback().set(FUNC(pulsar_state::ppi_pb_w));    // Control data to external
	//m_ppi->out_pc_callback().set(FUNC(pulsar_state::ppi_pc_w));    // Printer strobe
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(trs80)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("level1.rom",   0x0000, 0x1000, CRC(70d06dff) SHA1(20d75478fbf42214381e05b14f57072f3970f765))

	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END


ROM_START(trs80l2)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_SYSTEM_BIOS(0, "level2", "Radio Shack Level II Basic")
	ROMX_LOAD("trs80.z33",   0x0000, 0x1000, CRC(37c59db2) SHA1(e8f8f6a4460a6f6755873580be6ff70cebe14969), ROM_BIOS(0))
	ROMX_LOAD("trs80.z34",   0x1000, 0x1000, CRC(05818718) SHA1(43c538ca77623af6417474ca5b95fb94205500c1), ROM_BIOS(0))
	ROMX_LOAD("trs80.zl2",   0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "rsl2", "R/S L2 Basic")
	ROMX_LOAD("trs80alt.z33",0x0000, 0x1000, CRC(be46faf5) SHA1(0e63fc11e207bfd5288118be5d263e7428cc128b), ROM_BIOS(1))
	ROMX_LOAD("trs80alt.z34",0x1000, 0x1000, CRC(6c791c2d) SHA1(2a38e0a248f6619d38f1a108eea7b95761cf2aee), ROM_BIOS(1))
	ROMX_LOAD("trs80alt.zl2",0x2000, 0x1000, CRC(55b3ad13) SHA1(6279f6a68f927ea8628458b278616736f0b3c339), ROM_BIOS(1))

	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END


ROM_START(radionic)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("ep1.bin",      0x0000, 0x1000, CRC(e8908f44) SHA1(7a5a60c3afbeb6b8434737dd302332179a7fca59))
	ROM_LOAD("ep2.bin",      0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155))
	ROM_LOAD("ep3.bin",      0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369))
	ROM_LOAD("ep4.bin",      0x3000, 0x0800, CRC(70f90f26) SHA1(cbee70da04a3efac08e50b8e3a270262c2440120))
	ROM_CONTINUE(            0x3000, 0x0800)

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD("trschar.bin",  0x0000, 0x1000, CRC(02e767b6) SHA1(c431fcc6bd04ce2800ca8c36f6f8aeb2f91ce9f7))
ROM_END


ROM_START(sys80)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("sys80rom.1",   0x0000, 0x1000, CRC(8f5214de) SHA1(d8c052be5a2d0ec74433043684791d0554bf203b))
	ROM_LOAD("sys80rom.2",   0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155))
	ROM_LOAD("trs80.zl2",    0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369))
	/* This rom turns the system80 into the "blue label" version. SYSTEM then /12288 to activate. */
	ROM_LOAD("sys80.ext",    0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715))

	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD("trs80m1.chr",  0x0000, 0x0400, CRC(0033f2b9) SHA1(0d2cd4197d54e2e872b515bbfdaa98efe502eda7))
ROM_END

#define rom_sys80p rom_sys80


ROM_START(lnw80)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("lnw_a.bin",    0x0000, 0x0800, CRC(e09f7e91) SHA1(cd28e72efcfebde6cf1c7dbec4a4880a69e683da))
	ROM_LOAD("lnw_a1.bin",   0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0))
	ROM_LOAD("lnw_b.bin",    0x1000, 0x0800, CRC(c4303568) SHA1(13e3d81c6f0de0e93956fa58c465b5368ea51682))
	ROM_LOAD("lnw_b1.bin",   0x1800, 0x0800, CRC(3a5ea239) SHA1(8c489670977892d7f2bfb098f5df0b4dfa8fbba6))
	ROM_LOAD("lnw_c.bin",    0x2000, 0x0800, CRC(2ba025d7) SHA1(232efbe23c3f5c2c6655466ebc0a51cf3697be9b))
	ROM_LOAD("lnw_c1.bin",   0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("lnw_chr.bin",  0x0000, 0x0800, CRC(c89b27df) SHA1(be2a009a07e4378d070002a558705e9a0de59389))

	ROM_REGION(0x4000, "gfx2", ROMREGION_ERASEFF) // for trs80_gfxram
ROM_END


ROM_START(ht1080z)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("ht1080z.rom",  0x0000, 0x3000, CRC(2bfef8f7) SHA1(7a350925fd05c20a3c95118c1ae56040c621be8f))
	ROM_LOAD("sys80.ext",    0x3000, 0x0800, CRC(2a851e33) SHA1(dad21ec60973eb66e499fe0ecbd469118826a715))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("ht1080z.chr",  0x0000, 0x0800, CRC(e8c59d4f) SHA1(a15f30a543e53d3e30927a2e5b766fcf80f0ae31))
ROM_END


ROM_START(ht1080z2)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("ht1080z.rom",  0x0000, 0x3000, CRC(2bfef8f7) SHA1(7a350925fd05c20a3c95118c1ae56040c621be8f))
	ROM_LOAD("ht1080z2.ext", 0x3000, 0x0800, CRC(07415ac6) SHA1(b08746b187946e78c4971295c0aefc4e3de97115))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("ht1080z2.chr", 0x0000, 0x0800, CRC(6728f0ab) SHA1(1ba949f8596f1976546f99a3fdcd3beb7aded2c5))
ROM_END


ROM_START(ht108064)
	ROM_REGION(0x3800, "maincpu",0)
	ROM_LOAD("ht108064.rom", 0x0000, 0x3000, CRC(48985a30) SHA1(e84cf3121f9e0bb9e1b01b095f7a9581dcfaaae4))
	ROM_LOAD("ht108064.ext", 0x3000, 0x0800, CRC(fc12bd28) SHA1(0da93a311f99ec7a1e77486afe800a937778e73b))

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD("ht108064.chr", 0x0000, 0x0800, CRC(e76b73a4) SHA1(6361ee9667bf59d50059d09b0baf8672fdb2e8af))
ROM_END


void trs80_state::init_trs80()
{
	m_mode = 0;
}

void trs80_state::init_trs80l2()
{
	m_mode = 2;
}


//    YEAR  NAME         PARENT    COMPAT  MACHINE   INPUT    CLASS        INIT           COMPANY                        FULLNAME                           FLAGS
COMP( 1977, trs80,       0,        0,      trs80,    trs80,   trs80_state, init_trs80,    "Tandy Radio Shack",           "TRS-80 Model I (Level I Basic)",  0 )
COMP( 1978, trs80l2,     0,        0,      model1,   trs80l2, trs80_state, init_trs80l2,  "Tandy Radio Shack",           "TRS-80 Model I (Level II Basic)", 0 )
COMP( 1983, radionic,    trs80l2,  0,      radionic, trs80l2, trs80_state, init_trs80,    "Komtek",                      "Radionic",                        0 )
COMP( 1980, sys80,       trs80l2,  0,      sys80,    sys80,   trs80_state, init_trs80l2,  "EACA Computers Ltd",          "System-80 (60 Hz)",               0 )
COMP( 1980, sys80p,      trs80l2,  0,      sys80p,   sys80,   trs80_state, init_trs80l2,  "EACA Computers Ltd",          "System-80 (50 Hz)",               0 )
COMP( 1981, lnw80,       trs80l2,  0,      lnw80,    sys80,   trs80_state, init_trs80,    "LNW Research",                "LNW-80",                          0 )
COMP( 1983, ht1080z,     trs80l2,  0,      ht1080z,  sys80,   trs80_state, init_trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series I",               0 )
COMP( 1984, ht1080z2,    trs80l2,  0,      ht1080z,  sys80,   trs80_state, init_trs80l2,  "Hiradastechnika Szovetkezet", "HT-1080Z Series II",              0 )
COMP( 1985, ht108064,    trs80l2,  0,      ht1080z,  sys80,   trs80_state, init_trs80,    "Hiradastechnika Szovetkezet", "HT-1080Z/64",                     0 )
