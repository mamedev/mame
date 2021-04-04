// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Memory map

0000-2fff ROM                 R   D0-D7
3000-37ff ROM                 R   D0-D7
37de      UART status         R/W D0-D7
37df      UART data           R/W D0-D7
37e0      for the realtime clock
37e1      select disk drive 0         W
37e2      cassette drive latch address    W
37e3      select disk drive 1         W
37e4      select which cassette unit      W   D0-D1 (D0 selects unit 1, D1 selects unit 2)
37e5      select disk drive 2         W
37e7      select disk drive 3         W
37e0-37e3 floppy motor            W   D0-D3
          or floppy head select   W   D3
37e8      send a byte to printer  W   D0-D7
37e8      read printer status     R   D7
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
- IRQ mode 1
- NMI

Has non-addressable links to set the baud rate. Receive and Transmit clocks are tied together.

Cassette baud rates:
- 500 baud @1.77MHz and 1000 baud @4MHz.

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

FE:
- bit 0 is for selecting inverse video of the whole screen on a lnw80
- bit 2 enables colour on a lnw80
- bit 3 is for selecting roms (low) or 16k hires area (high) on a lnw80

Shift and Right-arrow will enable 32 cpl, if the hardware allows it.

SYSTEM commands:
    - Press Break (End key) to quit
    - Press Enter to exit with error
    - xxxx to load program xxxx from tape.
    - / to execute last program loaded
    - /nnnnn to execute program at nnnnn (decimal)

About the RTC - The time is incremented while ever the cursor is flashing. It is stored in a series
    of bytes in the computer's work area. The bytes are in a certain order, this is:
    seconds, minutes, hours, year, day, month. The seconds are stored at 0x4041.
    A reboot always sets the time to zero.

********************************************************************************************************

To Do / Status:
--------------

- basically works
- add 1.77 / 4 MHz switch
- find out if it really did support 32-cpl mode or not
- hi-res and colour are coded but do not work
- investigate expansion-box
- none of my collection of lnw80-specific floppies will work; some crash MAME

*******************************************************************************************************/
#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "imagedev/snapquik.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
#include "machine/buffer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "formats/dmk_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/trs80_dsk.h"

class lnw80_state : public driver_device
{
public:
	lnw80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_region_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_p_gfxram(*this, "gfxram")
		, m_lnw_bank(*this, "lnw_banked_mem")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_uart(*this, "uart")
		, m_uart_clock(*this, "uart_clock")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_floppy2(*this, "fdc:2")
		, m_floppy3(*this, "fdc:3")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_io_baud(*this, "BAUD")
		, m_io_config(*this, "CONFIG")
		, m_io_keyboard(*this, "LINE%u", 0)
	{ }

	void lnw80(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	static void floppy_formats(format_registration &fr);
	void port_ff_w(uint8_t data);
	void lnw80_fe_w(uint8_t data);
	void port_ea_w(uint8_t data);
	void port_e8_w(uint8_t data);
	uint8_t lnw80_fe_r();
	uint8_t port_ff_r();
	uint8_t port_ea_r();
	uint8_t port_e8_r();
	uint8_t irq_status_r();
	uint8_t printer_r();
	void printer_w(uint8_t data);
	void motor_w(uint8_t data);
	uint8_t keyboard_r(offs_t offset);
	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	INTERRUPT_GEN_MEMBER(rtc_interrupt);
	INTERRUPT_GEN_MEMBER(fdc_interrupt);
	TIMER_CALLBACK_MEMBER(cassette_data_callback);
	DECLARE_WRITE_LINE_MEMBER(intrq_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	void lnw80_palette(palette_device &palette) const;
	uint32_t screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lnw80_io(address_map &map);
	void lnw80_mem(address_map &map);
	void lnw_banked_mem(address_map &map);

	bool m_mode;
	uint8_t m_irq;
	uint8_t m_mask;
	bool m_reg_load;
	u8 m_lnw_mode;
	bool m_cassette_data;
	emu_timer *m_cassette_data_timer;
	double m_old_cassette_val;
	uint8_t m_size_store;
	uint16_t m_timeout;
	floppy_image_device *m_floppy;
	required_device<cpu_device> m_maincpu;
	required_memory_region m_region_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	required_shared_ptr<u8> m_p_gfxram;
	required_device<address_map_bank_device> m_lnw_bank;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_device<ay31015_device> m_uart;
	required_device<clock_device> m_uart_clock;
	required_device<fd1771_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_io_baud;
	required_ioport m_io_config;
	required_ioport_array<8> m_io_keyboard;
};


void lnw80_state::lnw80_mem(address_map &map)
{
	map(0x0000, 0x3fff).m(m_lnw_bank, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0xffff).ram();
}

void lnw80_state::lnw_banked_mem(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x37e0, 0x37e3).rw(FUNC(lnw80_state::irq_status_r), FUNC(lnw80_state::motor_w));
	map(0x37e8, 0x37eb).rw(FUNC(lnw80_state::printer_r), FUNC(lnw80_state::printer_w));
	map(0x37ec, 0x37ef).rw(FUNC(lnw80_state::fdc_r), FUNC(lnw80_state::fdc_w));
	map(0x3800, 0x3bff).r(FUNC(lnw80_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0x7fff).ram().share(m_p_gfxram);
}

void lnw80_state::lnw80_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xe8, 0xe8).rw(FUNC(lnw80_state::port_e8_r), FUNC(lnw80_state::port_e8_w));
	map(0xe9, 0xe9).portr("E9");
	map(0xea, 0xea).rw(FUNC(lnw80_state::port_ea_r), FUNC(lnw80_state::port_ea_w));
	map(0xeb, 0xeb).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xfe, 0xfe).rw(FUNC(lnw80_state::lnw80_fe_r), FUNC(lnw80_state::lnw80_fe_w));
	map(0xff, 0xff).rw(FUNC(lnw80_state::port_ff_r), FUNC(lnw80_state::port_ff_w));
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

static INPUT_PORTS_START( lnw80 )
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
	// On original lnw80, shift cancels all keys except F3 which becomes backspace.
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

#define IRQ_M1_RTC      0x80    /* RTC on Model I */
#define IRQ_M1_FDC      0x40    /* FDC on Model I */


TIMER_CALLBACK_MEMBER(lnw80_state::cassette_data_callback)
{
	double new_val = (m_cassette->input());

	/* Check for HI-LO transition */
	if ( m_old_cassette_val > -0.2 && new_val < -0.2 )
		m_cassette_data = true;

	m_old_cassette_val = new_val;
}


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


uint8_t lnw80_state::port_e8_r()
{
/* not emulated
    d7 Clear-to-Send (CTS), Pin 5
    d6 Data-Set-Ready (DSR), pin 6
    d5 Carrier Detect (CD), pin 8
    d4 Ring Indicator (RI), pin 22
    d3,d2,d0 Not used
    d1 UART Receiver Input, pin 20 (pin 20 is also DTR) */

	return 0;
}

uint8_t lnw80_state::port_ea_r()
{
/* UART Status Register
    d7 Data Received ('1'=condition true)
    d6 Transmitter Holding Register empty ('1'=condition true)
    d5 Overrun Error ('1'=condition true)
    d4 Framing Error ('1'=condition true)
    d3 Parity Error ('1'=condition true)
    d2..d0 Not used */

	uint8_t data=7;
	m_uart->write_swe(0);
	data |= m_uart->tbmt_r() ? 0x40 : 0;
	data |= m_uart->dav_r( ) ? 0x80 : 0;
	data |= m_uart->or_r(  ) ? 0x20 : 0;
	data |= m_uart->fe_r(  ) ? 0x10 : 0;
	data |= m_uart->pe_r(  ) ? 0x08 : 0;
	m_uart->write_swe(1);

	return data;
}

void lnw80_state::port_e8_w(uint8_t data)
{
	m_reg_load = BIT(data, 1);
}

void lnw80_state::port_ea_w(uint8_t data)
{
	if (m_reg_load)

/* d2..d0 not emulated
    d7 Even Parity Enable ('1'=even, '0'=odd)
    d6='1',d5='1' for 8 bits
    d6='0',d5='1' for 7 bits
    d6='1',d5='0' for 6 bits
    d6='0',d5='0' for 5 bits
    d4 Stop Bit Select ('1'=two stop bits, '0'=one stop bit)
    d3 Parity Inhibit ('1'=disable; No parity, '0'=parity enabled)
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Request-to-Send (RTS), pin 4
    d0 Data-Terminal-Ready (DTR), pin 20 */

	{
		m_uart->write_cs(0);
		m_uart->write_nb1(BIT(data, 6));
		m_uart->write_nb2(BIT(data, 5));
		m_uart->write_tsb(BIT(data, 4));
		m_uart->write_eps(BIT(data, 7));
		m_uart->write_np(BIT(data, 3));
		m_uart->write_cs(1);
	}
	else
	{
/* not emulated
    d7,d6 Not used
    d5 Secondary Unassigned, pin 18
    d4 Secondary Transmit Data, pin 14
    d3 Secondary Request-to-Send, pin 19
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Data-Terminal-Ready (DTR), pin 20
    d0 Request-to-Send (RTS), pin 4 */

	}
}

uint8_t lnw80_state::lnw80_fe_r()
{
	return m_lnw_mode;
}

uint8_t lnw80_state::port_ff_r()
{
/* ModeSel and cassette data
    d7 cassette data from tape
    d6 modesel setting */

	return (m_mode ? 0 : 0x40) | (m_cassette_data ? 0x80 : 0) | 0x3f;
}

/* lnw80 can switch out all the devices, roms and video ram to be replaced by graphics ram. */
void lnw80_state::lnw80_fe_w(uint8_t data)
{
/* lnw80 video options
    d3 bankswitch lower 16k between roms and hires ram (1=hires)
    d2 enable colour    \
    d1 hres             /   these 2 are the bits from the MODE command of LNWBASIC
    d0 inverse video (entire screen) */

	m_lnw_mode = data;

	m_lnw_bank->set_bank(BIT(data, 3));
}

void lnw80_state::port_ff_w(uint8_t data)
{
/* Standard output port of Model I
    d3 ModeSel bit
    d2 Relay
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, 1.0, -1.0, 0.0 };

	m_cassette->change_state(BIT(data, 2) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR );
	m_cassette->output(levels[data & 3]);
	m_cassette_data = false;

	m_mode = BIT(data, 3);

	static const double speaker_levels[4] = { 0.0, -1.0, 0.0, 1.0 };
	m_speaker->set_levels(4, speaker_levels);
	m_speaker->level_w(data & 3);
}

/*************************************
 *
 *      Interrupt handlers.
 *
 *************************************/

INTERRUPT_GEN_MEMBER(lnw80_state::rtc_interrupt)
{
/* This enables the processing of interrupts for the clock and the flashing cursor.
    The OS counts one tick for each interrupt. It is called 40 times per second. */

	m_irq |= IRQ_M1_RTC;
	m_maincpu->set_input_line(0, HOLD_LINE);

	// While we're here, let's countdown the motor timeout too.
	// Let's not... LDOS often freezes
//  if (m_timeout)
//  {
//      m_timeout--;
//      if (m_timeout == 0)
//          if (m_floppy)
//              m_floppy->mon_w(1);  // motor off
//  }
}


WRITE_LINE_MEMBER(lnw80_state::intrq_w)
{
	if (state)
	{
		m_irq |= IRQ_M1_FDC;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else
		m_irq &= ~IRQ_M1_FDC;
}


/*************************************
 *                                   *
 *      Memory handlers              *
 *                                   *
 *************************************/

u8 lnw80_state::fdc_r(offs_t offset)
{
	if ((offset == 0) && (!BIT(m_io_config->read(), 7)))
		return 0xff;
	else
		return m_fdc->read(offset) ^ 0xff;
}

void lnw80_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset, data ^ 0xff);
}

uint8_t lnw80_state::printer_r()
{
	return m_cent_status_in->read();
}

void lnw80_state::printer_w(uint8_t data)
{
	m_cent_data_out->write(data);
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

uint8_t lnw80_state::irq_status_r()
{
/* (trs80l2) Whenever an interrupt occurs, 37E0 is read to see what devices require service.
    d7 = RTC
    d6 = FDC
    d2 = Communications (not emulated)
    All interrupting devices are serviced in a single interrupt. There is a mask byte,
    which is dealt with by the DOS. We take the opportunity to reset the cpu INT line. */

	u8 result = m_irq;
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq = 0;
	return result;
}


void lnw80_state::motor_w(uint8_t data)
{
	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->mon_w(0);
		m_floppy->ss_w(BIT(data, 4));
		m_timeout = 200;
	}

	// switch to fm
	m_fdc->dden_w(1);
}

/*************************************
 *      Keyboard         *
 *************************************/
uint8_t lnw80_state::keyboard_r(offs_t offset)
{
	u8 i, result = 0;

	for (i = 0; i < 8; i++)
		if (BIT(offset, i))
			result |= m_io_keyboard[i]->read();

	return result;
}


/*************************************
 *  Machine              *
 *************************************/

void lnw80_state::machine_start()
{
	save_item(NAME(m_mode));
	save_item(NAME(m_irq));
	save_item(NAME(m_mask));
	save_item(NAME(m_reg_load));
	save_item(NAME(m_lnw_mode));
	save_item(NAME(m_cassette_data));
	save_item(NAME(m_old_cassette_val));
	save_item(NAME(m_size_store));
	save_item(NAME(m_timeout));

	m_size_store = 0xff;
	m_reg_load=1;

	m_cassette_data_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(lnw80_state::cassette_data_callback),this));
	m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
}

void lnw80_state::machine_reset()
{
	m_mode = 0;
	m_cassette_data = false;
	const uint16_t s_bauds[8]={ 110, 300, 600, 1200, 2400, 4800, 9600, 19200 };
	u16 s_clock = s_bauds[m_io_baud->read()] << 4;
	m_uart_clock->set_unscaled_clock(s_clock);

	m_reg_load = 1;
	m_lnw_mode = 0;
	lnw80_fe_w(0);
}


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

#define CMD_TYPE_OBJECT_CODE                            0x01
#define CMD_TYPE_TRANSFER_ADDRESS                       0x02
#define CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER     0x04
#define CMD_TYPE_LOAD_MODULE_HEADER                     0x05
#define CMD_TYPE_PARTITIONED_DATA_SET_HEADER            0x06
#define CMD_TYPE_PATCH_NAME_HEADER                      0x07
#define CMD_TYPE_ISAM_DIRECTORY_ENTRY                   0x08
#define CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY            0x0a
#define CMD_TYPE_PDS_DIRECTORY_ENTRY                    0x0c
#define CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY             0x0e
#define CMD_TYPE_YANKED_LOAD_BLOCK                      0x10
#define CMD_TYPE_COPYRIGHT_BLOCK                        0x1f

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

QUICKLOAD_LOAD_MEMBER(lnw80_state::quickload_cb)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint8_t type, length;
	uint8_t data[0x100];
	uint8_t addr[2];
	void *ptr;

	while (!image.image_feof())
	{
		image.fread( &type, 1);
		image.fread( &length, 1);

		switch (type)
		{
			case CMD_TYPE_OBJECT_CODE:  // 01 - block of data
			{
				length -= 2;
				u16 block_length = length ? length : 256;
				image.fread( &addr, 2);
				u16 address = (addr[1] << 8) | addr[0];
				if (LOG) logerror("/CMD object code block: address %04x length %u\n", address, block_length);
				if (address < 0x3c00)
				{
					image.message("Attempting to write outside of RAM");
					return image_init_result::FAIL;
				}
				ptr = program.get_write_ptr(address);
				image.fread( ptr, block_length);
			}
			break;

			case CMD_TYPE_TRANSFER_ADDRESS: // 02 - go address
			{
				image.fread( &addr, 2);
				u16 address = (addr[1] << 8) | addr[0];
				if (LOG) logerror("/CMD transfer address %04x\n", address);
				m_maincpu->set_state_int(Z80_PC, address);
			}
			return image_init_result::PASS;

		case CMD_TYPE_LOAD_MODULE_HEADER: // 05 - name
			image.fread( &data, length);
			if (LOG) logerror("/CMD load module header '%s'\n", data);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK: // 1F - copyright info
			image.fread( &data, length);
			if (LOG) logerror("/CMD copyright block '%s'\n", data);
			break;

		default:
			image.fread( &data, length);
			logerror("/CMD unsupported block type %u!\n", type);
			image.message("Unsupported or invalid block type");
			return image_init_result::FAIL;
		}
	}

	return image_init_result::PASS;
}

/* 8-bit video, 64/80 characters per line = lnw80 */
uint32_t lnw80_state::screen_update_lnw80(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const uint16_t rows[] = { 0, 0x200, 0x100, 0x300, 1, 0x201, 0x101, 0x301 };
	uint16_t sy=0,ma=0;
	uint8_t cols = BIT(m_lnw_mode, 1) ? 80 : 64;

	/* Although the OS can select 32-character mode, it is not supported by hardware */
	if (m_lnw_mode != m_size_store)
	{
		m_size_store = m_lnw_mode;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	uint8_t bg=7,fg=0;
	if (BIT(m_lnw_mode, 1))
	{
		bg = 0;
		fg = 7;
	}

	switch (m_lnw_mode & 0x06)
	{
		case 0:                 // MODE 0
			for (uint16_t y = 0; y < 16; y++)
			{
				for (uint16_t ra = 0; ra < 12; ra++)
				{
					uint16_t *p = &bitmap.pix(sy++);

					for (uint16_t x = ma; x < ma + 64; x++)
					{
						uint8_t chr = m_p_videoram[x];

						if (chr & 0x80)
						{
							uint8_t gfxbit = (ra & 0x0c)>>1;
							/* Display one line of a lores character (6 pixels) */
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							gfxbit++;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
							*p++ = BIT(chr, gfxbit) ? fg : bg;
						}
						else
						{
							/* get pattern of pixels for that character scanline */
							uint8_t gfx;
							if (ra < 8)
								gfx = m_p_chargen[(chr<<1) | rows[ra] ];
							else
								gfx = 0;

							/* Display a scanline of a character (6 pixels) */
							*p++ = BIT(gfx, 2) ? fg : bg;
							*p++ = BIT(gfx, 1) ? fg : bg;
							*p++ = BIT(gfx, 6) ? fg : bg;
							*p++ = BIT(gfx, 7) ? fg : bg;
							*p++ = BIT(gfx, 5) ? fg : bg;
							*p++ = BIT(gfx, 3) ? fg : bg;
						}
					}
				}
				ma+=64;
			}
			break;

		case 0x02:                  // MODE 1
			for (uint16_t y = 0; y < 0x400; y+=0x40)
			{
				for (uint16_t ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix(sy++);

					for (uint16_t x = 0; x < 0x40; x++)
					{
						uint8_t gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (uint16_t x = 0; x < 0x10; x++)
					{
						uint8_t gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
						/* Display 6 pixels in extended region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}
				}
			}
			break;

		case 0x04:                  // MODE 2
			/* it seems the text video ram can have an effect in this mode,
			    not explained clearly, so not emulated */
			for (uint16_t y = 0; y < 0x400; y+=0x40)
			{
				for (uint16_t ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix(sy++);

					for (uint16_t x = 0; x < 0x40; x++)
					{
						uint8_t gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						fg = (gfx & 0x38) >> 3;
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
						fg = gfx & 0x07;
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
					}
				}
			}
			break;

		case 0x06:                  // MODE 3
			/* the manual does not explain at all how colour is determined
			    for the extended area. Further, the background colour
			    is not mentioned anywhere. Black is assumed. */
			for (uint16_t y = 0; y < 0x400; y+=0x40)
			{
				for (uint16_t ra = 0; ra < 0x3000; ra+=0x400)
				{
					uint16_t *p = &bitmap.pix(sy++);

					for (uint16_t x = 0; x < 0x40; x++)
					{
						uint8_t gfx = m_p_gfxram[ y | x | ra];
						fg = (m_p_videoram[ x | y ] & 0x38) >> 3;
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = m_p_videoram[ 0x3c00 | x | y ] & 0x07;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (uint16_t x = 0; x < 0x10; x++)
					{
						uint8_t gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
						fg = (m_p_gfxram[ 0x3c00 | x | y ] & 0x38) >> 3;
						/* Display 6 pixels in extended region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = m_p_gfxram[ 0x3c00 | x | y ] & 0x07;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}
				}
			}
			break;
	}
	return 0;
}

/***************************************************************************
  Palettes
***************************************************************************/

/* Levels are unknown - guessing */
static constexpr rgb_t lnw80_pens[] =
{
	{ 220, 220, 220 }, // white
	{   0, 175,   0 }, // green
	{ 200, 200,   0 }, // yellow
	{ 255,   0,   0 }, // red
	{ 255,   0, 255 }, // magenta
	{   0,   0, 175 }, // blue
	{   0, 255, 255 }, // cyan
	{   0,   0,   0 }  // black
};

void lnw80_state::lnw80_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, lnw80_pens);
}

/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
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

static GFXDECODE_START(gfx_lnw80)
	GFXDECODE_ENTRY( "chargen", 0, lnw80_charlayout, 0, 4 )
GFXDECODE_END


void lnw80_state::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_TRS80_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
}

static void lnw80_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_QD); // QD allows the 80-track boot disks to work.
}


void lnw80_state::lnw80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 9);
	//m_maincpu->set_clock(16_MHz_XTAL / 4); // or 16MHz / 9; 4MHz or 1.77MHz operation selected by HI/LO switch
	m_maincpu->set_addrmap(AS_PROGRAM, &lnw80_state::lnw80_mem);
	m_maincpu->set_addrmap(AS_IO, &lnw80_state::lnw80_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// LNW80 Theory of Operations gives H and V periods as 15.750kHz and 59.66Hz, probably due to rounding the calculated ~15.7468kHz to 4 figures
	screen.set_raw(3.579545_MHz_XTAL * 3, 682, 0, 480, 264, 0, 192); // 10.738MHz generated by tank circuit (top left of page 2 of schematics)
	screen.set_screen_update(FUNC(lnw80_state::screen_update_lnw80));
	screen.set_palette("palette");
	PALETTE(config, "palette", FUNC(lnw80_state::lnw80_palette), 8);
	GFXDECODE(config, "gfxdecode", "palette", gfx_lnw80);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_default_state(CASSETTE_PLAY);

	QUICKLOAD(config, "quickload", "cmd", attotime::from_seconds(1)).set_load_callback(FUNC(lnw80_state::quickload_cb));

	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(lnw80_state::intrq_w));

	FLOPPY_CONNECTOR(config, "fdc:0", lnw80_floppies, "sssd", lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", lnw80_floppies, "sssd", lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", lnw80_floppies, nullptr, lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", lnw80_floppies, nullptr, lnw80_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit6));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit5));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit4));

	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	CLOCK(config, m_uart_clock, 19200 * 16);
	m_uart_clock->signal_handler().set(m_uart, FUNC(ay31015_device::write_rcp));
	m_uart_clock->signal_handler().append(m_uart, FUNC(ay31015_device::write_tcp));

	AY31015(config, m_uart);
	m_uart->read_si_callback().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_uart->write_so_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	//MCFG_AY31015_WRITE_DAV_CB(WRITELINE( , , ))
	m_uart->set_auto_rdav(true);
	RS232_PORT(config, "rs232", default_rs232_devices, nullptr);

	ADDRESS_MAP_BANK(config, m_lnw_bank, 0);
	m_lnw_bank->set_addrmap(0, &lnw80_state::lnw_banked_mem);
	m_lnw_bank->set_data_width(8);
	m_lnw_bank->set_addr_width(16);
	m_lnw_bank->set_stride(0x4000);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

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
ROM_END


//    YEAR  NAME         PARENT    COMPAT    MACHINE   INPUT    CLASS        INIT        COMPANY          FULLNAME    FLAGS
COMP( 1981, lnw80,       0,        trs80l2,  lnw80,    lnw80,   lnw80_state, empty_init, "LNW Research",  "LNW-80",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
