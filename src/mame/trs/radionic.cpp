// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************
Komtek 1 (Radionic R1001) memory map

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
3c00-3fff static video RAM and colour ram, banked
4000-ffff dynamic main RAM

Printer: Usually 37e8, but you can use the PPI instead.

Cassette baud rate: 500 baud

I/O ports
FF:
- bits 0 and 1 are for writing a cassette
- bit 2 must be high to turn the cassette motor on, enables cassette data paths on a system-80
- bit 3 switches the display between 64 or 32 characters per line
- bit 6 remembers the 32/64 screen mode (inverted)
- bit 7 is for reading from a cassette

Shift and Right-arrow will enable 32 cpl.

SYSTEM commands:
    - Press Break (End key) to quit
    - Press Enter to exit with error
    - xxxx to load program xxxx from tape.
    - / to execute last program loaded
    - /nnnnn to execute program at nnnnn (decimal)

Inbuilt Monitor - its existence is not revealed to the user.
    - SYSTEM then /12710 = enter machine-language monitor
    Monitor commands:
    - B : return to Basic
    - Dnnnn : Dump hex to screen. Press down-arrow for more. Press enter to quit.
    - Mnnnn : Modify memory. Enter new byte and it increments to next address. X to quit.
    - Gnnnn : Execute program at nnnn
    - Gnnnn,tttt : as above, breakpoint at tttt
    - R : modify registers
    The monitor appears to end at 33D3, however the rom contains more code through to 35EB.
    But, the 34xx area is overwitten by the optional RS-232 expansion box.

About the RTC - The time is incremented while it is enabled via the Machine Configuration. The time
    is stored in a series of bytes in the computer's work area. The bytes are in a certain order,
    this being: seconds, minutes, hours, days. The seconds are stored at 0x4041. A reboot always
    sets the time to zero.

About colours - The computer has 16 colours with a byte at 350B controlling the operation.
    POKE 13579,2  : monochrome
    POKE 13579,4  : programmable colour
    POKE 13579,12 : automatic colour
    More information was discovered, this being
    - It doesn't have to be 350B (13579), anything in the 35xx range will do.
    - d0 : write of vram goes to vram (0 = yes, 1 = no)
    - d1 : write of vram goes to colour ram (0 = yes, 1 = no)
    - d2 : colour enable (0 = monochrome, 1 = colour)
    - d3 : programmable or automatic (0 = programmable, 1 = automatic)
    The colour codes and names are listed in the palette below. The descriptions are quite vague,
    and appear to be background only. The foreground is assumed to always be white.
    Automatic is a preset colour set by internal jumpers.
    - No schematic or technical info for the colour board
    - No information on the settings of the Automatic mode
    - The "User Friendly Manual" has a bunch of mistakes (page 15).
    - It's not known if colour ram can be read. But LDOS won't scroll if it can't always read vram.
    - No colour programs exist in the wild, so nothing can be verified.
    So, some guesswork has been done.

About the PPI - A selling point of this computer was the ability to sense and control external gadgets.
    For example, it could join to a temperature sensor, and when the temperature reached a certain value
    the computer could instruct a device to turn on or off. There's 4 input jacks and 6 output jacks.
    The PPI can also control a parallel printer. To enable this, enter SYSTEM then /12367 .

About the RS-232 unit - This is an external box that plugs into the expansion port. It takes over memory
    region 3400-34FF, although it only uses 3400 and 3401. It has a baud generator consisting of 2x 74LS163
    and a dipswitch block to choose one of 5 possible rates. The UART and RS-232 parts are conventional.
    There's no programming of the unit from the inbuilt roms; you need to write your own.

********************************************************************************************************

To Do / Status:
--------------

- Difficulty loading real tapes.
- Writing to floppy is problematic; freezing/crashing are common issues.
- Add fdc doubler (only some info available)

*******************************************************************************************************/

#include "emu.h"
#include "trs80.h"
#include "trs80_quik.h"

#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"

#include "softlist_dev.h"
#include "utf8.h"


namespace {

#define MASTER_XTAL 12164800


class radionic_state : public trs80_state
{
public:
	radionic_state(const machine_config &mconfig, device_type type, const char *tag)
		: trs80_state(mconfig, type, tag)
		, m_ppi(*this, "ppi")
		, m_uart2(*this, "uart2")
		, m_clock(*this, "uclock")
	{ }

	void radionic(machine_config &config);

private:
	INTERRUPT_GEN_MEMBER(rtc_via_nmi);
	void radionic_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	std::unique_ptr<u8[]> m_vram;  // video ram
	std::unique_ptr<u8[]> m_cram;  // colour ram
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void cctrl_w(offs_t offset, u8 data);
	void video_w(offs_t offset, u8 data);
	u8 video_r(offs_t offset);
	void ppi_pa_w(offs_t offset, u8 data);
	void ppi_pb_w(offs_t offset, u8 data);
	void ppi_pc_w(offs_t offset, u8 data);
	u8 ppi_pc_r(offs_t offset);
	void mem_map(address_map &map) ATTR_COLD;
	static void floppy_formats(format_registration &fr);
	u8 m_cctrl = 2;
	required_device<i8255_device> m_ppi;
	required_device<i8251_device> m_uart2;
	required_device<clock_device> m_clock;
};

void radionic_state::mem_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3400, 0x3401).mirror(0xfe).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));  // optional RS-232
	map(0x3500, 0x35ff).w(FUNC(radionic_state::cctrl_w));
	map(0x3600, 0x3603).mirror(0xfc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));  // interface to external sensors
	map(0x37e0, 0x37e3).rw(FUNC(radionic_state::irq_status_r), FUNC(radionic_state::motor_w));
	map(0x37e8, 0x37eb).rw(FUNC(radionic_state::printer_r), FUNC(radionic_state::printer_w));
	map(0x37ec, 0x37ef).rw(FUNC(radionic_state::fdc_r), FUNC(radionic_state::fdc_w));
	map(0x3800, 0x3bff).r(FUNC(radionic_state::keyboard_r));
	map(0x3c00, 0x3fff).rw(FUNC(radionic_state::video_r), FUNC(radionic_state::video_w));
	map(0x4000, 0xffff).ram();
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
	PORT_CONFNAME(  0xc0, 0x00, "Floppy and RTC") // Floppy doesn't work if RTC is on, so interlink them.
	PORT_CONFSETTING(   0x00, "Both Off" )
	PORT_CONFSETTING(   0x80, "RTC off, Floppy On" )
	PORT_CONFSETTING(   0x40, "RTC on, Floppy Off" )
	PORT_CONFNAME(  0x30, 0x00, "Colour")  // 2 switches on the side
	PORT_CONFSETTING(   0x00, "Monochrome" )
	PORT_CONFSETTING(   0x10, "Auto Colour" )
	PORT_CONFSETTING(   0x30, "Programmable Colour" )
	PORT_CONFNAME(  0x07, 0x00, "Serial Port" ) // a jumper on the board? (not documented)
	PORT_CONFSETTING(   0x00, "300 baud" )
	PORT_CONFSETTING(   0x01, "600 baud" )
	PORT_CONFSETTING(   0x02, "1200 baud" )
	PORT_CONFSETTING(   0x03, "2400 baud" )
	PORT_CONFSETTING(   0x04, "4800 baud" )
INPUT_PORTS_END


/* Levels are unknown - guessing */
static constexpr rgb_t radionic_pens[] =
{
	// colour
	{ 200, 200, 200 }, // 0 off white
	{   0, 255,   0 }, // 1 light green
	{ 255,   0,   0 }, // 2 red
	{   0, 128,   0 }, // 3 dark green
	{   0,   0, 255 }, // 4 blue
	{   0, 255, 255 }, // 5 greenish blue
	{ 255,   3,  62 }, // 6 rose red
	{ 136, 155, 174 }, // 7 dusty blue
	{ 200, 200,   0 }, // 8 greenish yellow
	{ 173, 255,  47 }, // 9 light yellow (greenish)
	{ 207,  33,  33 }, // 10 golden red
	{ 128, 128, 128 }, // 11 grey
	{ 220, 255,   0 }, // 12 reddish green
	{ 255, 255, 191 }, // 13 pale yellow
	{ 247, 170,   0 }, // 14 orange
	{  90, 156,  57 }, // 15 ogre green
	// monochrome
	{ 250, 250, 250 }, // 16 white
	{   0,   0,   0 }  // 17 black
};

void radionic_state::radionic_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, radionic_pens);
}

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
u32 radionic_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;
	u8 cols = m_cpl ? 32 : 64;
	u8 skip = m_cpl ? 2 : 1;
	// colours have to be enabled both by a poke and by switches on the side of the unit
	bool col_en = BIT(m_cctrl, 2) && BIT(m_io_config->read(), 4);
	bool auto_en = BIT(m_cctrl, 3) || !BIT(m_io_config->read(), 5);
	u8 fg = 16, bg = 17;   // monochrome
	if (col_en && auto_en)
		bg = 4;    // automatic colour

	if (cols != m_cols)
	{
		m_cols = cols;
		screen.set_visible_area(0, cols*8-1, 0, 16*16-1);
	}

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 16; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x+=skip)
			{
				u8 chr = m_vram[x];
				if (col_en && !auto_en)
					bg = m_cram[x] & 15;

				/* get pattern of pixels for that character scanline */
				u8 gfx = m_p_chargen[(chr<<3) | (ra & 7) | (ra & 8) << 8];

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 0) ? fg : bg;
				*p++ = BIT(gfx, 1) ? fg : bg;
				*p++ = BIT(gfx, 2) ? fg : bg;
				*p++ = BIT(gfx, 3) ? fg : bg;
				*p++ = BIT(gfx, 4) ? fg : bg;
				*p++ = BIT(gfx, 5) ? fg : bg;
				*p++ = BIT(gfx, 6) ? fg : bg;
				*p++ = BIT(gfx, 7) ? fg : bg;
			}
		}
		ma+=64;
	}
	return 0;
}

void radionic_state::machine_start()
{
	trs80_state::machine_start();
	save_item(NAME(m_cctrl));

	// video ram
	m_vram = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_vram), 0x0800);

	// colour
	m_cram = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_cram), 0x0800);
}

void radionic_state::machine_reset()
{
	trs80_state::machine_reset();
	m_cctrl = 2;

	u8 sw = m_io_config->read() & 7;
	u16 baud = 300;
	for (u8 i = 0; i < sw; i++)
		baud <<= 1;
	m_clock->set_unscaled_clock(baud*16); // It's designed on the assumption that the uart will divide by 16
	//printf("%d\n",baud);
}

INTERRUPT_GEN_MEMBER(radionic_state::rtc_via_nmi)
{
	if (BIT(m_io_config->read(), 6))
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::from_usec(100));
}

u8 radionic_state::video_r(offs_t offset)
{
	return m_vram[offset];
}

void radionic_state::video_w(offs_t offset, u8 data)
{
	if (!BIT(m_cctrl, 0))
		m_vram[offset] = data;
	if (!BIT(m_cctrl, 1))
		m_cram[offset] = data;
}

void radionic_state::cctrl_w(offs_t offset, u8 data)
{
	m_cctrl = data & 15;
}

void radionic_state::ppi_pa_w(offs_t offset, u8 data)
{
	// d0-7: Data to extra printer
}

void radionic_state::ppi_pb_w(offs_t offset, u8 data)
{
	// d0-7: Outputs to control jacks (only 6 connected up by default)
}

void radionic_state::ppi_pc_w(offs_t offset, u8 data)
{
	// Printer control
	// d0: Strobe
}

u8 radionic_state::ppi_pc_r(offs_t offset)
{
	// Printer Status
	// d1: Busy
	// d2: out of paper
	// d3: Unit select

	// Sensor Status
	// d4-7: sensing inputs
	return 0xFF;
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
	// Photos from Incog show 12.1648, and 3.579545 xtals. The schematic seems to just approximate these values.
	Z80(config, m_maincpu, 3.579545_MHz_XTAL / 2);
	//m_maincpu->set_clock(MASTER_XTAL / 6); // early machines only, before the floppy interface was added
	m_maincpu->set_addrmap(AS_PROGRAM, &radionic_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &radionic_state::trs80_io);
	m_maincpu->set_periodic_int(FUNC(radionic_state::rtc_via_nmi), attotime::from_hz(MASTER_XTAL / 12 / 16384));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_XTAL, 768, 0, 512, 312, 0, 256);
	screen.set_screen_update(FUNC(radionic_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_radionic);
	PALETTE(config, "palette", FUNC(radionic_state::radionic_palette), 18);   // 16 colours + monochrome

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

	FD1771(config, m_fdc, 16_MHz_XTAL / 16);
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

	CLOCK(config, m_clock, 4'800);
	m_clock->signal_handler().set(m_uart2, FUNC(i8251_device::write_txc));
	m_clock->signal_handler().set(m_uart2, FUNC(i8251_device::write_rxc));

	// RS232 port: the schematic is missing most of the info, so guessing
	I8251(config, m_uart2, 3.579545_MHz_XTAL / 2 );
	m_uart2->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart2->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart2->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart2, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(m_uart2, FUNC(i8251_device::write_dsr));

	// Interface to external circuits
	I8255(config, m_ppi);
	m_ppi->in_pc_callback().set(FUNC(radionic_state::ppi_pc_r));      // Sensing from external and printer status
	m_ppi->out_pa_callback().set(FUNC(radionic_state::ppi_pa_w));    // Data for external plugin printer module
	m_ppi->out_pb_callback().set(FUNC(radionic_state::ppi_pb_w));    // Control data to external
	m_ppi->out_pc_callback().set(FUNC(radionic_state::ppi_pc_w));    // Printer strobe

	SOFTWARE_LIST(config, "cass_list").set_original("trs80_cass").set_filter("1"); // R
	SOFTWARE_LIST(config, "quik_list").set_original("trs80_quik").set_filter("1"); // R
	SOFTWARE_LIST(config, "flop_list").set_original("trs80_flop").set_filter("1"); // R
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

} // anonymous namespace


//    YEAR  NAME         PARENT   COMPAT   MACHINE   INPUT     CLASS           INIT           COMPANY     FULLNAME                    FLAGS
COMP( 1983, radionic,    0,       trs80l2, radionic, radionic, radionic_state, empty_init,    "Komtek",   "Radionic R1001/Komtek 1",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
