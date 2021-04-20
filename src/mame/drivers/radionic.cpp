// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Komtek 1 (Radionic) memory map

0000-37ff ROM                            R   D0-D7
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

Printer: Level II usually 37e8

Cassette baud rate: 500 baud

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on, enables cassette data paths on a system-80
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

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

Radionic has 16 colours with a byte at 350B controlling the operation. See manual.


********************************************************************************************************

To Do / Status:
--------------

- For those machines that allow it, add cass2 as an image device and hook it up.
- Difficulty loading real tapes.
- Writing to floppy is problematic; freezing/crashing are common issues.

radionic:  works
           floppy not working (@6C0, DRQ never gets set)
           add colour
           expansion-box?
           uart

*******************************************************************************************************/

#include "emu.h"
#include "includes/trs80.h"
#include "machine/i8255.h"


class radionic_state : public trs80_state
{
public:
	radionic_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80_state(mconfig, type, tag)
		, m_ppi(*this, "ppi")
	{ }

	void radionic(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	static void floppy_formats(format_registration &fr);

	required_device<i8255_device> m_ppi;
};

void radionic_state::mem_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	// Optional external RS232 module with 8251
	//map(0x3400, 0x3401).mirror(0xfe).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// Internal colour controls (need details)
	//map(0x3500, 0x35ff).w(FUNC(radionic_state::colour_w));
	// Internal interface to external slots
	map(0x3600, 0x3603).mirror(0xfc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x37de, 0x37de).rw(FUNC(radionic_state::sys80_f9_r), FUNC(radionic_state::sys80_f8_w));
	map(0x37df, 0x37df).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0x37e0, 0x37e3).rw(FUNC(radionic_state::irq_status_r), FUNC(radionic_state::motor_w));
	map(0x37e4, 0x37e7).w(FUNC(radionic_state::cassunit_w));
	map(0x37e8, 0x37eb).rw(FUNC(radionic_state::printer_r), FUNC(radionic_state::printer_w));
	map(0x37ec, 0x37ef).rw(FUNC(radionic_state::fdc_r), FUNC(radionic_state::fdc_w));
	map(0x3800, 0x3bff).r(FUNC(radionic_state::keyboard_r));
	map(0x3c00, 0x3fff).ram().share(m_p_videoram);
	map(0x4000, 0xffff).ram();
}

void radionic_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xe8, 0xe8).rw(FUNC(radionic_state::port_e8_r), FUNC(radionic_state::port_e8_w));
	map(0xe9, 0xe9).portr("E9").w("brg", FUNC(com8116_device::stt_str_w));
	map(0xea, 0xea).rw(FUNC(radionic_state::port_ea_r), FUNC(radionic_state::port_ea_w));
	map(0xeb, 0xeb).rw(m_uart, FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0xff, 0xff).rw(FUNC(radionic_state::port_ff_r), FUNC(radionic_state::port_ff_w));
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

static INPUT_PORTS_START( radionic )
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
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_F1)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_F2) PORT_CHAR('\\') PORT_CHAR('}')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_F3)  PORT_CHAR(']') PORT_CHAR('|')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_F4)  PORT_CHAR('^')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_F5)  PORT_CHAR('_') // radionic: LF

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
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	/* backspace do the same as cursor left */
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')

	PORT_START("LINE7")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0xfe, 0x00, IPT_UNUSED)

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


/**************************** F4 CHARACTER DISPLAYER ***********************************************************/
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

static GFXDECODE_START(gfx_radionic)
	GFXDECODE_ENTRY( "chargen", 0, radionic_charlayout, 0, 1 )
GFXDECODE_END

/* lores characters are in the character generator. Each character is 8x16. */
uint32_t radionic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;
	uint8_t cols = BIT(m_mode, 0) ? 32 : 64;
	uint8_t skip = BIT(m_mode, 0) ? 2 : 1;

	if (m_mode != m_size_store)
	{
		m_size_store = m_mode;
		screen.set_visible_area(0, cols*8-1, 0, 16*16-1);
	}

	for (uint8_t y = 0; y < 16; y++)
	{
		for (uint8_t ra = 0; ra < 16; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma + 64; x+=skip)
			{
				uint8_t chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				uint8_t gfx = m_p_chargen[(chr<<3) | (ra & 7) | (ra & 8) << 8];

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
		ma+=64;
	}
	return 0;
}

void radionic_state::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_JV1_FORMAT);
}

// Most images are single-sided, 40 tracks or less.
// However, the default is QD to prevent MAME from
// crashing if a disk with more than 40 tracks is used.
static void radionic_floppies(device_slot_interface &device)
{
	device.option_add("35t_sd", FLOPPY_525_SSSD_35T);
	device.option_add("40t_sd", FLOPPY_525_SSSD);
	device.option_add("40t_dd", FLOPPY_525_DD);
	device.option_add("80t_qd", FLOPPY_525_QD);
}


void radionic_state::radionic(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3579000 / 2);
	//m_maincpu->set_clock(12_MHz_XTAL / 6); // or 3.579MHz / 2 (selectable?)
	m_maincpu->set_addrmap(AS_PROGRAM, &radionic_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &radionic_state::io_map);
	// Komtek I "User Friendly Manual" calls for "Z80 running at 1.97 MHz." This likely refers to an alternate NTSC version
	// whose master clock was approximately 11.8005 MHz (6 times ~1.966 MHz and 750 times 15.734 kHz). Though the schematics
	// provide the main XTAL frequency as 12 MHz, that they also include a 3.579 MHz XTAL suggests this possibility.
	m_maincpu->set_periodic_int(FUNC(radionic_state::rtc_interrupt), attotime::from_hz(40));
	m_maincpu->set_periodic_int(FUNC(radionic_state::nmi_line_pulse), attotime::from_hz(12_MHz_XTAL / 12 / 16384));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL, 768, 0, 512, 312, 0, 256);
	screen.set_screen_update(FUNC(radionic_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_radionic);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(trs80l2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "cmd", attotime::from_seconds(1)).set_load_callback(FUNC(radionic_state::quickload_cb));

	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(radionic_state::intrq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], radionic_floppies, "80t_qd", radionic_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], radionic_floppies, "80t_qd", radionic_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], radionic_floppies, nullptr, radionic_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], radionic_floppies, nullptr, radionic_state::floppy_formats).enable_sound(true);

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

ROM_START(radionic)
	ROM_REGION(0x3800, "maincpu", 0)
	ROM_LOAD("ep1.z37",        0x0000, 0x1000, CRC(e8908f44) SHA1(7a5a60c3afbeb6b8434737dd302332179a7fca59) )
	ROM_LOAD("ep2.z36",        0x1000, 0x1000, CRC(46e88fbf) SHA1(a3ca32757f269e09316e1e91ba1502774e2f5155) )
	ROM_LOAD("ep3.z35",        0x2000, 0x1000, CRC(306e5d66) SHA1(1e1abcfb5b02d4567cf6a81ffc35318723442369) )
	ROM_LOAD("ep4.z34",        0x3000, 0x0800, CRC(70f90f26) SHA1(cbee70da04a3efac08e50b8e3a270262c2440120) )
	ROM_CONTINUE(              0x3000, 0x0800)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("trschar.z58",    0x0000, 0x1000, CRC(02e767b6) SHA1(c431fcc6bd04ce2800ca8c36f6f8aeb2f91ce9f7) )
ROM_END


//    YEAR  NAME         PARENT   COMPAT   MACHINE   INPUT     CLASS           INIT           COMPANY         FULLNAME         FLAGS
COMP( 1983, radionic,    0,       trs80l2, radionic, radionic, radionic_state, empty_init,    "Komtek",       "Radionic",      MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
