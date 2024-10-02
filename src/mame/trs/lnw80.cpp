// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Memory map

0000-37ff ROM                 R   D0-D7
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
- bit 0 is for selecting inverse video of the whole screen
- bit 1 chooses text or graphics screen
- bit 2 enables colour
- bit 3 is for selecting roms (low) or 16k hires area (high)

Shift and Right-arrow will enable 32 cpl.

SYSTEM commands:
    - Press Break (End key) to quit
    - Press Enter to exit with error
    - xxxx to load program xxxx from tape.
    - / to execute last program loaded
    - /nnnnn to execute program at nnnnn (decimal)

About the RTC - The hardware side exists, but special software is needed to get the clock to work.
    By default, nothing happens.

********************************************************************************************************

To Do / Status:
--------------

- basically works
- hi-res and colour are coded and pass the test suite, but need some real programs to check with.
- investigate expansion-box
- none of my collection of lnw80-specific floppies will work; some crash MAME

*******************************************************************************************************/

#include "emu.h"
#include "trs80.h"
#include "trs80_quik.h"
#include "machine/input_merger.h"
#include "formats/td0_dsk.h"
#include "softlist_dev.h"
#include "utf8.h"

namespace {

class lnw80_state : public trs80_state
{
public:
	lnw80_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80_state(mconfig, type, tag)
		, m_p_gfxram(*this, "gfxram")
		, m_lnw_bank(*this, "lnw_banked_mem")
	{ }

	void lnw80(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);
	void lnw80_fe_w(u8 data);
	u8 lnw80_fe_r();
	void lnw80_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lnw80_io(address_map &map) ATTR_COLD;
	void lnw80_mem(address_map &map) ATTR_COLD;

	u8 m_lnw_mode = 0U;
	required_shared_ptr<u8> m_p_gfxram;
	memory_view m_lnw_bank;
};


void lnw80_state::lnw80_mem(address_map &map)
{
	map(0x0000, 0x3fff).view(m_lnw_bank);
	m_lnw_bank[0](0x0000, 0x2fff).rom().region("maincpu", 0);
	m_lnw_bank[0](0x37de, 0x37de).rw(FUNC(lnw80_state::sys80_f9_r), FUNC(lnw80_state::sys80_f8_w));
	m_lnw_bank[0](0x37e0, 0x37e3).rw(FUNC(lnw80_state::irq_status_r), FUNC(lnw80_state::motor_w));
	m_lnw_bank[0](0x37e4, 0x37e7).w(FUNC(lnw80_state::cassunit_w));
	m_lnw_bank[0](0x37e8, 0x37eb).rw(FUNC(lnw80_state::printer_r), FUNC(lnw80_state::printer_w));
	m_lnw_bank[0](0x37ec, 0x37ef).rw(FUNC(lnw80_state::fdc_r), FUNC(lnw80_state::fdc_w));
	m_lnw_bank[0](0x3800, 0x3bff).r(FUNC(lnw80_state::keyboard_r));
	m_lnw_bank[0](0x3c00, 0x3fff).ram().share(m_p_videoram);
	m_lnw_bank[1](0x0000, 0x3fff).ram().share(m_p_gfxram);
	map(0x4000, 0xffff).ram();
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
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)        PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)        PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('_')
	PORT_BIT(0x28, 0x00, IPT_UNUSED)

	PORT_START("LINE4")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, 0x00, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, 0x00, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, 0x00, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, 0x00, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, 0x00, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE5")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, 0x00, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
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
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, 0x00, IPT_KEYBOARD) PORT_NAME("Control") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xee, 0x00, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x01, 0x00, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_DEL) PORT_CHAR(UCHAR_MAMEKEY(PAUSE)) PORT_WRITE_LINE_DEVICE_MEMBER("nmigate", input_merger_device, in_w<0>)

	PORT_START("CONFIG")
	PORT_CONFNAME(    0x80, 0x00,   "Floppy Disc Drives")
	PORT_CONFSETTING(   0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(   0x80, DEF_STR( On ) )
	PORT_CONFNAME(    0x40, 0x00,   "CPU Speed")
	PORT_CONFSETTING(   0x00, "1.77 MHz" )
	PORT_CONFSETTING(   0x40, "4 MHz" )

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


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


u8 lnw80_state::lnw80_fe_r()
{
	return m_lnw_mode;
}


/* lnw80 can switch out all the devices, roms and video ram to be replaced by graphics ram. */
void lnw80_state::lnw80_fe_w(u8 data)
{
/* lnw80 video options
    d3 bankswitch lower 16k between roms and hires ram (1=hires)
    d2 enable colour    \
    d1 hres             /   these 2 are the bits from the MODE command of LNWBASIC
    d0 inverse video (entire screen) */

	m_lnw_mode = data;

	m_lnw_bank.select(BIT(data, 3));
}


/*************************************
 *  Machine              *
 *************************************/

void lnw80_state::machine_start()
{
	save_item(NAME(m_cpl));
	save_item(NAME(m_irq));
	save_item(NAME(m_mask));
	save_item(NAME(m_reg_load));
	save_item(NAME(m_lnw_mode));
	save_item(NAME(m_cassette_data));
	save_item(NAME(m_old_cassette_val));
	save_item(NAME(m_cols));
	save_item(NAME(m_timeout));

	m_reg_load=1;

	m_cassette_data_timer = timer_alloc(FUNC(lnw80_state::cassette_data_callback), this);
	m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
}

void lnw80_state::machine_reset()
{
	m_cpl = 0;
	m_cols = 0xff;
	m_cassette_data = false;
	const u16 s_bauds[8]={ 110, 300, 600, 1200, 2400, 4800, 9600, 19200 };
	u16 s_clock = s_bauds[m_io_baud->read()] << 4;
	m_uart_clock->set_unscaled_clock(s_clock);

	m_maincpu->set_unscaled_clock(BIT(m_io_config->read(), 6) ? (16_MHz_XTAL / 4) : (16_MHz_XTAL / 9));  // HI-LO switch
	m_reg_load = 1;
	lnw80_fe_w(0);
}

/* 8-bit video, 64/80 characters per line = lnw80 */
u32 lnw80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const u16 rows[] = { 0, 0x200, 0x100, 0x300, 1, 0x201, 0x101, 0x301 };
	u16 sy=0,ma=0;
	bool inv = BIT(m_lnw_mode, 0);
	u8 mode = BIT(m_lnw_mode, 1, 2);
	u8 cols = BIT(mode, 0) ? 80 : 64;
	u8 skip = 1;
	if (mode == 0)
	{
		skip = m_cpl ? 2 : 1;
		if (skip == 2)
			cols >>= 1;
	}

	if (cols != m_cols)
	{
		m_cols = cols;
		screen.set_visible_area(0, cols*6-1, 0, 16*12-1);
	}

	u8 bg=7,fg=0;
	if (inv)
	{
		bg = 0;
		fg = 7;
	}

	switch (mode)
	{
		case 0:                 // MODE 0
			for (u16 y = 0; y < 16; y++)
			{
				for (u16 ra = 0; ra < 12; ra++)
				{
					u16 *p = &bitmap.pix(sy++);

					for (u16 x = ma; x < ma + 64; x+=skip)
					{
						u8 chr = m_p_videoram[x];

						if (chr & 0x80)
						{
							u8 gfxbit = (ra & 0x0c)>>1;
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
							u8 gfx;
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

		case 1:                  // MODE 1
			for (u16 y = 0; y < 0x400; y+=0x40)
			{
				for (u16 ra = 0; ra < 0x3000; ra+=0x400)
				{
					u16 *p = &bitmap.pix(sy++);

					for (u16 x = 0; x < 0x40; x++)
					{
						u8 gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (u16 x = 0; x < 0x10; x++)
					{
						u8 gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
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

		case 2:                  // MODE 2
			/* it seems the text video ram can have an effect in this mode,
			    not explained clearly, so not emulated */
			for (u16 y = 0; y < 0x400; y+=0x40)
			{
				for (u16 ra = 0; ra < 0x3000; ra+=0x400)
				{
					u16 *p = &bitmap.pix(sy++);

					for (u16 x = 0; x < 0x40; x++)
					{
						u8 gfx = m_p_gfxram[ y | x | ra];
						/* Display 6 pixels in normal region */
						fg = BIT(gfx, 3, 3);
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
						fg = BIT(gfx, 0, 3);
						*p++ = fg;
						*p++ = fg;
						*p++ = fg;
					}
				}
			}
			break;

		case 3:                  // MODE 3
			/* the manual does not explain at all how colour is determined
			    for the extended area. Further, the background colour
			    is not mentioned anywhere. Black is assumed. */
			for (u16 y = 0; y < 0x400; y+=0x40)
			{
				for (u16 ra = 0; ra < 0x3000; ra+=0x400)
				{
					u16 *p = &bitmap.pix(sy++);

					for (u16 x = 0; x < 0x40; x++)
					{
						u8 gfx = m_p_gfxram[ y | x | ra];
						fg = BIT(m_p_videoram[ x | y ], 3, 3);
						/* Display 6 pixels in normal region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = BIT(m_p_videoram[ 0x3c00 | x | y ], 0, 3);
						*p++ = BIT(gfx, 3) ? fg : bg;
						*p++ = BIT(gfx, 4) ? fg : bg;
						*p++ = BIT(gfx, 5) ? fg : bg;
					}

					for (u16 x = 0; x < 0x10; x++)
					{
						u8 gfx = m_p_gfxram[ 0x3000 | x | (ra & 0xc00) | ((ra & 0x3000) >> 8)];
						fg = BIT(m_p_gfxram[ 0x3c00 | x | y ], 3, 3);
						/* Display 6 pixels in extended region */
						*p++ = BIT(gfx, 0) ? fg : bg;
						*p++ = BIT(gfx, 1) ? fg : bg;
						*p++ = BIT(gfx, 2) ? fg : bg;
						fg = BIT(m_p_gfxram[ 0x3c00 | x | y ], 0, 3);
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
	fr.add(FLOPPY_JV1_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
}

static void lnw80_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_QD); // QD allows the 80-track boot disks to work.
}


void lnw80_state::lnw80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &lnw80_state::lnw80_mem);
	m_maincpu->set_addrmap(AS_IO, &lnw80_state::lnw80_io);
	m_maincpu->halt_cb().set("nmigate", FUNC(input_merger_device::in_w<1>));

	input_merger_device &nmigate(INPUT_MERGER_ANY_HIGH(config, "nmigate"));
	nmigate.output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI); // TODO: also causes SYSRES on expansion bus

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// LNW80 Theory of Operations gives H and V periods as 15.750kHz and 59.66Hz, probably due to rounding the calculated ~15.7468kHz to 4 figures
	screen.set_raw(3.579545_MHz_XTAL * 3, 682, 0, 480, 264, 0, 192); // 10.738MHz generated by tank circuit (top left of page 2 of schematics)
	screen.set_screen_update(FUNC(lnw80_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", FUNC(lnw80_state::lnw80_palette), 8);
	GFXDECODE(config, "gfxdecode", "palette", gfx_lnw80);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	CASSETTE(config, m_cassette);
	m_cassette->set_formats(trs80l2_cassette_formats);
	m_cassette->set_default_state(CASSETTE_PLAY);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("trs80_cass");

	TRS80_QUICKLOAD(config, "quickload", m_maincpu, attotime::from_seconds(1));

	FD1771(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(lnw80_state::intrq_w));

	FLOPPY_CONNECTOR(config, m_floppy[0], lnw80_floppies, "sssd", lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], lnw80_floppies, "sssd", lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], lnw80_floppies, nullptr, lnw80_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], lnw80_floppies, nullptr, lnw80_state::floppy_formats).enable_sound(true);

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

	SOFTWARE_LIST(config, "cass_list").set_original("trs80_cass").set_filter("1"); // L
	SOFTWARE_LIST(config, "quik_list").set_original("trs80_quik").set_filter("1"); // L
	SOFTWARE_LIST(config, "flop_list").set_original("trs80_flop").set_filter("1"); // L
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START(lnw80)
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("lnw_a.u78",      0x0000, 0x0800, CRC(e09f7e91) SHA1(cd28e72efcfebde6cf1c7dbec4a4880a69e683da) )
	ROM_LOAD("lnw_a1.u75",     0x0800, 0x0800, CRC(ac297d99) SHA1(ccf31d3f9d02c3b68a0ee3be4984424df0e83ab0) )
	ROM_LOAD("lnw_b.u79",      0x1000, 0x0800, CRC(c4303568) SHA1(13e3d81c6f0de0e93956fa58c465b5368ea51682) )
	ROM_LOAD("lnw_b1.u76",     0x1800, 0x0800, CRC(3a5ea239) SHA1(8c489670977892d7f2bfb098f5df0b4dfa8fbba6) )
	ROM_LOAD("lnw_c.u80",      0x2000, 0x0800, CRC(2ba025d7) SHA1(232efbe23c3f5c2c6655466ebc0a51cf3697be9b) )
	ROM_LOAD("lnw_c1.u77",     0x2800, 0x0800, CRC(ed547445) SHA1(20102de89a3ee4a65366bc2d62be94da984a156b) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("lnw_chr.u100",   0x0000, 0x0800, CRC(c89b27df) SHA1(be2a009a07e4378d070002a558705e9a0de59389) )

	ROM_REGION(0x0020, "proms", 0)
	ROM_LOAD_OPTIONAL("lnw_ntsc.u130",  0x0000, 0x0020, CRC(b990a207) SHA1(1a1cc3150cbfed76b1c88c0d561f9bee954f3234) )
ROM_END

} // anonymous namespace

//    YEAR  NAME         PARENT    COMPAT    MACHINE   INPUT    CLASS        INIT        COMPANY          FULLNAME    FLAGS
COMP( 1981, lnw80,       0,        trs80l2,  lnw80,    lnw80,   lnw80_state, empty_init, "LNW Research",  "LNW-80",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
